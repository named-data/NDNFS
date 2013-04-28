#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>

#include <fuse.h>
#include "mongo/client/dbclient.h"

using namespace std;
using namespace mongo;
using namespace boost;


static const char *db_name = "ndnfs.root";

static const int dir_type = 0;
static const int file_type = 1;
static const int version_type = 2;
static const int segment_type = 3;

static int
ndnfs_getattr(const char *path, struct stat *stbuf)
{
    cout << "ndnfs_getattr: called with path " << path << endl;
    
    memset(stbuf, 0, sizeof(struct stat));

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();  // There should be no two entries with the same id (absolute path)
    int type = entry.getIntField("type");
    int mode = entry.getIntField("mode");
    
    if (type == dir_type) {
        stbuf->st_mode = S_IFDIR | mode;
        stbuf->st_nlink = 1;
    } else if (type == file_type) {
        stbuf->st_mode = S_IFREG | mode;
        stbuf->st_nlink = 1;
        int file_size = entry.getIntField("size");
        stbuf->st_size = file_size;
    }
    
    c->done();
    delete c;
    return 0;
}

static int
ndnfs_open(const char *path, struct fuse_file_info *fi)
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

    //TODO: check mode
    
    //BSONObj entry = cursor->next();
    //if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //    return -EACCES;

    c->done();
    delete c;
    return 0;
}

static int
ndnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
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

static int
ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
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
    int file_size = entry.getIntField("size");

    if ((size_t)offset >= file_size) {/* Trying to read past the end of file. */
        c->done();
        delete c;
        return 0;
    }

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    string content = entry.getStringField("data");
    memcpy(buf, content.substr(offset, size).c_str(), size); /* Provide the content. */

    c->done();
    delete c;
    return size;
}

static int
ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    cout << "ndnfs_create: called with path " << path << endl;
    cout << "ndnfs_create: create file with flag " << fi->flags << " and mode " << mode << endl;
    
    string abs_path(path);
    size_t last_comp_pos = abs_path.rfind('/');
    string dir_path = abs_path.substr(0, last_comp_pos + 1);
    string file_name = abs_path.substr(last_comp_pos + 1);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    // Cannot create file that has conflicting file name
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
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
    BSONObj file_entry = BSONObjBuilder().append("_id", path).append("type", file_type).append("mode", 0666)
                            .append("data", "").append("size", 0).obj();
    c->conn().insert(db_name, file_entry);
    // Append to existing BSON array
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << file_name ) ));
    
    c->done();
    delete c;
    return 0;
}

static int
ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    cout << "ndnfs_write: called with path " << path << endl;
    cout << "ndnfs_write: start write at offset " << offset << " with size " << size << endl;
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
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
    
    string old_content = entry.getStringField("data");
    string buf_content(buf, size);
    
    string content = old_content.substr(0, offset) + buf_content;
    
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "data" << content ) ));
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "size" << (int)content.size() ) ));
    
    c->done();
    delete c;
    return size;  // return the number of tyes written on success
}


static void
create_fuse_operations(struct fuse_operations *fuse_op)
{
    fuse_op->getattr = ndnfs_getattr;
    fuse_op->open    = ndnfs_open;
    fuse_op->read    = ndnfs_read;
    fuse_op->readdir = ndnfs_readdir;
    //fuse_op->create  = ndnfs_create;
    fuse_op->write   = ndnfs_write;
}

struct fuse_operations ndnfs_fs_ops;

int
main(int argc, char **argv)
{
    cout << "main: NDNFS version beta 0.1" << endl;
    cout << "main: test mongodb connection..." << endl;
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    if (c->ok()) {
        cout << "main: ok" << endl;
    } else {
        cout << "main: cannot connect to local mongodb, quit..." << endl;
        c->done();
        delete c;
        return -1;
    }
    
    // Check database first
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << "/"));
    if (!cursor->more()) {
        // Create root directory as an empty folder if database is empty
        BSONObj root_dir = BSONObjBuilder().append("_id", "/").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, root_dir);
        
        /* For test use, should remove later */
        // Add a file
        BSONObj hello_file = BSONObjBuilder().append("_id", "/hello.txt").append("type", file_type).append("mode", 0666)
                            .append("data", "Hello World\n").append("size", 12).obj();
        c->conn().insert(db_name, hello_file);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello.txt" ) ) );
        // Add a folder
        BSONObj hello_dir = BSONObjBuilder().append("_id", "/hello").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, hello_dir);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello" ) ) );
    }
    cout << "main: Root directory mounted from database" << endl;
    
    c->done();
    delete c;
    
    create_fuse_operations(&ndnfs_fs_ops);
    
    cout << "main: Enter FUSE main loop" << endl;
    return fuse_main(argc, argv, &ndnfs_fs_ops, NULL);
}
