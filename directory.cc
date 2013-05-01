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
    cout << "ndnfs_mkdir: create file with mode " << mode << endl;
    
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
    BSONObj dir_entry = BSONObjBuilder().append("_id", path).append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
    c->conn().insert(db_name, dir_entry);
    // Append to existing BSON array
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << dir_name ) ));
    
    c->done();
    delete c;
    return 0;
}

int ndnfs_rmdir(const char *path)
{
    cout << "ndnfs_rmdir: called with path " << path << endl;
    
    if (strcmp(path, "/") == 0) {
        // Cannot remove root dir.
        // This should not happen in the real world.
        return -EINVAL;
    }
    
    string dir_path, dir_name;
    split_last_component(path, dir_path, dir_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        // Cannot remove non-existing data
        c->done();
        delete c;
        return -EEXIST;
    }
    
    // Remove dir entry
    c->conn().remove(db_name, QUERY("_id" << path));
    
    // Remove pointer in the folder that holds the file
    // XXX: low performance!!!
    cursor = c->conn().query(db_name, QUERY("_id" << dir_path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -EEXIST;
    }
    
    BSONObj entry = cursor->next();
    vector< BSONElement > data = entry["data"].Array();
    
    BSONArrayBuilder bab;
    for (int i = 0; i < data.size(); i++) {
        string s = data[i].String();
        if (s != dir_name) {
            bab.append(s);
        }
    }
    
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$set" << BSON( "data" << bab.arr() ) ));
    
    c->done();
    delete c;
    return 0;
}
