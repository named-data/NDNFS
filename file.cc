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
    cout << "ndnfs_open: called with path " << path << endl;
    cout << "ndnfs_open: flag is " << fi->flags << endl;
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }

    // TODO: check flags with file mode    
    //BSONObj entry = cursor->next();
//     switch (fi->flags & O_ACCMODE) {
//     case O_RDONLY:
// 	cout << "read only" << endl;
// 	break;
//     case O_WRONLY:
// 	cout << "write only" << endl;
// 	break;
//     case O_RDWR:
// 	cout << "read write" << endl;
// 	break;
//     default:
// 	break;
//     }
    
    c->done();
    delete c;
    return 0;
}

int ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    cout << "ndnfs_read: called with path " << path << endl;
    cout << "ndnfs_read: start read at offset " << offset << " with size " << size << endl;

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();
    if (entry.getIntField("type") != file_type) {
        c->done();
        delete c;
        return -EINVAL;
    }

    int size_read = read_latest_version(path, c, entry, buf, size, offset);
    
    c->done();
    delete c;
    return size_read;
}

int ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    cout << "ndnfs_create: called with path " << path << endl;
    cout << "ndnfs_create: create file with flag " << fi->flags << " and mode " << mode << endl;
    
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
    
    // Create empty version for the new file
    string version = create_empty_version(file_path, c);
    
    // Create new file entry with empty content, "data" field contains the version string
    BSONObj file_entry = BSONObjBuilder().append("_id", file_path).append("type", file_type)
	                 .append("data", BSONArrayBuilder().append(version).arr()).obj();

    // Add the file entry to database
    c->conn().insert(db_name, file_entry);

    // Append to existing BSON array of the parent folder
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << file_name ) ));
    
    c->done();
    delete c;
    return 0;
}

int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    cout << "ndnfs_write: called with path " << path << endl;
    cout << "ndnfs_write: start write at offset " << offset << " with size " << size << endl;

    string file_path(path);
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << file_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -EINVAL;
    }
    
    BSONObj entry = cursor->next();
    if (entry.getIntField("type") != file_type) {
        c->done();
        delete c;
        return -EINVAL;
    }

    // Write data to a new version of the file
    string version = add_version(file_path, c, entry, buf, size, offset);
    if (version.empty()) {
	c->done();
	delete c;
	return -EINVAL;
    }

    // Append version string to existing BSON array of the parent file
    c->conn().update(db_name, BSON("_id" << file_path), BSON( "$push" << BSON( "data" << version ) ));
    
    c->done();
    delete c;
    return size;  // return the number of tyes written on success
}

int ndnfs_unlink(const char *path)
{
    cout << "ndnfs_unlink: called with path " << path << endl;
    
    string file_path(path);
    string dir_path, file_name;
    split_last_component(file_path, dir_path, file_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    
    // First remove pointer in the folder that holds the file
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << dir_path));
    if (!cursor->more()) {
	c->done();
        delete c;
        return -EEXIST;
    }
    
    BSONObj dir_entry = cursor->next();
    vector< BSONElement > dir_data = dir_entry["data"].Array();
    
    BSONArrayBuilder bab;
    for (int i = 0; i < dir_data.size(); i++) {
        string s = dir_data[i].String();
        if (s != file_name) {
            bab.append(s);
        }
    } 
   
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$set" << BSON( "data" << bab.arr() ) ));
    
    // Then remove all the versions under the file entry
    remove_versions(file_path, c);
    
    // Finally, remove file entry
    c->conn().remove(db_name, QUERY("_id" << path));

    c->done();
    delete c;
    return 0;
}
