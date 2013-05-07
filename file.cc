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
    
    if (entry.getIntField("size") == 0) {
        c->done();
        delete c;
        return 0;
    }
    
    int co_size;
    const char *co_raw = get_latest_version_data(path, c, entry, co_size);
    if (co_size == -1) {
	c->done();
        delete c;
        return -ENOENT;
    } else if (co_size == 0) {
	c->done();
        delete c;
        return 0;
    }

    ndn::ParsedContentObject pco((const unsigned char *)co_raw, co_size);
    ndn::BytesPtr co_content = pco.contentPtr();
    
    int file_size = co_content->size();
    const char *content = (const char *)ndn::head(*co_content);

    if ((size_t)offset >= file_size) {/* Trying to read past the end of file. */
        c->done();
        delete c;
        return 0;
    }

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    if (file_size > 0)
        memcpy(buf, content + offset, size);
    
    c->done();
    delete c;
    return size;
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
    
    // Create a version for the new file; add both file and the new version into database
    create_version(file_path, c);
    
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
    
    int old_co_size;
    const char *old_co_raw = get_latest_version_data(file_path, c, entry, old_co_size);
    
    int old_file_size = 0;
    const char *old_content = NULL;
    if (old_co_size > 0) {
        ndn::ParsedContentObject pco((const unsigned char *)old_co_raw, old_co_size);
        ndn::BytesPtr old_co_content = pco.contentPtr();
    
        old_file_size = old_co_content->size();
        old_content = (const char *)ndn::head(*old_co_content);
        
        if (old_file_size < offset) {
            c->done();
            delete c;
            return -EINVAL;
        }
    }
    
    int file_size = offset + size;
    char *content = new char[file_size];
    
    if (old_file_size > 0 && offset > 0)
        memcpy(content, old_content, offset);
    
    memcpy(content + offset, buf, size);
    
    add_version_with_data(file_path, c, content, file_size);
    
    delete content;
    
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
    // XXX: low performance!!!
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
    
    // Then remove file entry
    cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -EEXIST;
    }

    BSONObj file_entry = cursor->next();
    
    // Remove file entry with all the version entries
    remove_versions_and_file(file_path, c, file_entry);

    c->done();
    delete c;
    return 0;
}
