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

#include "directory.h"

using namespace std;
using namespace boost;
using namespace mongo;

int ndnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
    cout << "ndnfs_readdir: called with path " << path << endl;
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();
    int type = entry.getIntField("type");
    if (type != dir_type) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    vector< BSONElement > data = entry["data"].Array();

    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
    for (int i = 0; i < data.size(); i++) {
        filler(buf, data[i].String().c_str(), NULL, 0);
    }
    
    c->done();
    delete c;
    return 0;
}

int ndnfs_mkdir(const char *path, mode_t mode)
{
    cout << "ndnfs_mkdir: called with path " << path << endl;
    cout << "ndnfs_mkdir: make directory with mode 0" << std::oct << mode << endl;

    string dir_path, dir_name;
    split_last_component(path, dir_path, dir_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (cursor->more()) {
        // Cannot create file that has conflicting file name
        c->done();
        delete c;
        return -EEXIST;
    }
    
    // Cannot create file without creating necessary folders
    cursor = c->conn().query(db_name, QUERY("_id" << dir_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();
    
    // Add new file entry with empty content
    int now = time(0);
    BSONObj dir_entry = BSONObjBuilder().append("_id", path).append("type", dir_type).append("mode", mode)
	.append("atime", now).append("mtime", now).append("data", BSONArrayBuilder().arr()).obj();
    c->conn().insert(db_name, dir_entry);
    // Append to existing BSON array
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << dir_name ) ));
    
    c->done();
    delete c;
    return 0;
}


/*
 * For rmdir, we don't need to implement recursive remove,
 * because 'rm -r' will iterate all the sub-entries (dirs or
 * files) for us and remove them one-by-one.   ---SWT
 */
int ndnfs_rmdir(const char *path)
{
    cout << "ndnfs_rmdir: called with path " << path << endl;
    
    if (strcmp(path, "/") == 0) {
        // Cannot remove root dir.
        // This should not happen in the real world.
        return -EINVAL;
    }
    
    string parent_dir_path, dir_name;
    split_last_component(path, parent_dir_path, dir_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        // Cannot remove non-existing data
        c->done();
        delete c;
        return -EEXIST;
    }
    
    // Cannot create non-empty dir
    BSONObj dir_entry = cursor->next();
    assert(dir_entry.getIntField("type") == dir_type);
    vector< BSONElement > dir_data = dir_entry["data"].Array();
    if (dir_data.size() != 0) {
        c->done();
        delete c;
        return -ENOTEMPTY;
    }
    
    // Remove dir entry
    c->conn().remove(db_name, QUERY("_id" << path));
    
    // Remove pointer in the parent folder
    // XXX: low performance!!!
    cursor = c->conn().query(db_name, QUERY("_id" << parent_dir_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -EEXIST;
    }
    
    BSONObj parent_entry = cursor->next();
    vector< BSONElement > parent_data = parent_entry["data"].Array();
    
    BSONArrayBuilder bab;
    for (int i = 0; i < parent_data.size(); i++) {
        string s = parent_data[i].String();
        if (s != dir_name) {
            bab.append(s);
        }
    }
    
    c->conn().update(db_name, BSON("_id" << parent_dir_path), BSON( "$set" << BSON( "data" << bab.arr() << "mtime" << (int)time(0) ) ));

    c->done();
    delete c;
    return 0;
}
