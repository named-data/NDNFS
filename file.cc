/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Wentao Shang <wentao@cs.ucla.edu>
 */

#include "file.h"

using namespace std;
using namespace boost;
using namespace mongo;

int ndnfs_open(const char *path, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_open: called with path " << path << endl;
    cout << "ndnfs_open: flag is 0x" << std::hex << fi->flags << endl;
#endif

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }

    BSONObj file_entry = cursor->next();
    if (file_entry.getIntField("type") != ndnfs::file_type) {
        c->done();
	delete c;
	return -EISDIR;
    }

    // TODO: check file access mode against uid and gid

    // Check open flags
    switch (fi->flags & O_ACCMODE) {
    case O_RDONLY:
	// Nothing to be done for read only access
	break;
    case O_WRONLY:
    case O_RDWR:
	// Create temporary version for file editing

	// If there is already a temp version there, it means that
	// some one is writing to the file now. We should reject this 
	// open request.
	if (get_temp_version(file_entry) != -1) {
	    c->done();
	    delete c;
	    return -EACCES;
	}
	
	c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "temp" << generate_version() ) ));
	
	break;
    default:
	break;
    }
    
    c->done();
    delete c;
    return 0;
}


int ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_create: called with path " << path << endl;
    cout << "ndnfs_create: create file with flag 0x" << std::hex << fi->flags << " and mode 0" << std::oct << mode << endl;
#endif

    string file_path(path);
    string dir_path, file_name;
    split_last_component(file_path, dir_path, file_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << file_path));
    if (cursor->more()) {
        // Cannot create file that has conflicting file name
        c->done();
        delete c;
        return -EEXIST;
    }
    
    // Cannot create file without creating necessary folders in advance
    cursor = c->conn().query(db_name, QUERY("_id" << dir_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    // Generate temparary version for the new file
    long long tmp_ver = generate_version();
    
    // Create new file entry with empty version list and a temp version
    int now = time(0);
    BSONObj file_entry = BSONObjBuilder().append("_id", file_path).append("type", ndnfs::file_type).append("mode", mode).append("temp", tmp_ver)
	.append("size", 0).append("atime", now).append("mtime", now).append("data", (long long)-1).obj();

    // Add the file entry to database
    c->conn().insert(db_name, file_entry);

    // Append file name to existing BSON array of the parent folder
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << file_name ) ));
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$set" << BSON( "mtime" << now ) ));
    
    c->done();
    delete c;
    return 0;
}


int ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_read: called with path " << path << endl;
    cout << "ndnfs_read: start read at offset " << std::dec << offset << " with size " << size << endl;
#endif

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();
    if (entry.getIntField("type") != ndnfs::file_type) {
        c->done();
        delete c;
        return -EINVAL;
    }

    int size_read = read_current_version(path, c, entry, buf, size, offset);
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "atime" << (int)time(0) ) ));
    
    c->done();
    delete c;
    return size_read;
}


int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_write: called with path " << path << endl;
    cout << "ndnfs_write: start write at offset " << std::dec << offset << " with size " << size << endl;
#endif

    string file_path(path);
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << file_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -EINVAL;
    }
    
    BSONObj entry = cursor->next();
    if (entry.getIntField("type") != ndnfs::file_type) {
        c->done();
        delete c;
        return -EINVAL;
    }

    // Write data to a new version of the file
    int written = write_temp_version(file_path, c, entry, buf, size, offset);
    if (written < 0) {
	c->done();
	delete c;
	return -EINVAL;
    }

    c->conn().update(db_name, BSON("_id" << file_path), BSON( "$set" << BSON( "mtime" << (int)time(0) ) ));
    
    c->done();
    delete c;
    return written;  // return the number of tyes written on success
}


int ndnfs_truncate(const char *path, off_t length)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_truncate: called with path " << path << endl;
    cout << "ndnfs_truncate: truncate to length " << std::dec << length << endl;
#endif

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;     
	return -ENOENT;
    }
    
    BSONObj file_entry = cursor->next();
    if (file_entry.getIntField("type") != ndnfs::file_type) {
        c->done();
	delete c;
	return -EISDIR;
    }

    int ret = truncate_temp_version(path, c, file_entry, length);
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "mtime" << (int)time(0) ) ));

    c->done();
    delete c;   
    return ret;
}


int ndnfs_unlink(const char *path)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_unlink: called with path " << path << endl;
#endif

    string file_path(path);
    string dir_path, file_name;
    split_last_component(file_path, dir_path, file_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    
    // First remove pointer in the folder that holds the file
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$pull" << BSON( "data" << file_name ) ));
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$set" << BSON( "mtime" << (int)time(0) ) ));
    
    // Then remove all the versions under the file entry
    remove_versions(file_path, c);
    
    // Finally, remove file entry
    c->conn().remove(db_name, QUERY("_id" << path));

    c->done();
    delete c;
    return 0;
}

int ndnfs_release(const char *path, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_release: called with path " << path << " and flag " << std::hex << fi->flags << endl;
#endif

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
	c->done();
        delete c;
        return -EEXIST;
    }
    
    BSONObj file_entry = cursor->next();
    if (file_entry.getIntField("type") != ndnfs::file_type) {
        c->done();
	delete c;
	return -1;
    }

/*
  In FUSE design, if the file is created with create() call,
  the corresponding release() call always has fi->flags = 0.
  So we need to check the current version number to tell if
  the file is just created or opened in read only mode.
 */

    long long curr_ver_num = get_current_version(file_entry);
    // Check open flags
    if (((fi->flags & O_ACCMODE) != O_RDONLY) || curr_ver_num == -1) {
        // This file is either opened with write access or just created
        long long tmp_ver_num = get_temp_version(file_entry);
        if (tmp_ver_num == -1) {
	    c->done();
	    delete c;
	    return -1;
	}
	
	string file_path(path);
	string tmp_ver_path = file_path + "/" + lexical_cast<string> (tmp_ver_num);
	cursor = c->conn().query(db_name, QUERY("_id" << tmp_ver_path));
	if (!cursor->more()) {
	    // Nothing is written into this file so the temp version does not exist
	    // In this case we simply keep the current version as it is and reset temp version number
	    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "temp" << (long long)-1) ));
	} else {
	    // Update version number and remove old version
	    int size = get_version_size(path, c, tmp_ver_num);
	    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "size" << size << "data" << tmp_ver_num << "temp" << (long long)-1) ));
	    string old_ver_path = file_path + "/" + lexical_cast<string> (curr_ver_num);
	    remove_version(old_ver_path, c);
	    //c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "mtime" << (int)time(0) ) ));
	}
    }

    c->done();
    delete c;
    return 0;
}
