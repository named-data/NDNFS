#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/algorithm/string.hpp>

#include <fuse.h>
#include <mongo/client/dbclient.h>
#include <ccnx-cpp.h>

using namespace std;
using namespace boost;
using namespace mongo;

static Ccnx::Wrapper ndn_wrapper;
static const char *db_name = "ndnfs.root";

static const int dir_type = 0;
static const int file_type = 1;
static const int version_type = 2;
static const int segment_type = 3;


static inline int
split_last_component(const char *path, string &prefix, string &name)
{
    string abs_path(path);
    size_t last_comp_pos = abs_path.rfind('/');
    if (last_comp_pos == string::npos)
        return -1;
    
    prefix = abs_path.substr(0, last_comp_pos);
    if (prefix.empty())
        prefix = "/";
    name = abs_path.substr(last_comp_pos + 1);
    
    return 0;
}


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
        stbuf->st_size = entry.getIntField("size");
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
    const char *co_raw = entry.getField("data").binData(co_size);
    Ccnx::ParsedContentObject pco((const unsigned char *)co_raw, co_size);
    Ccnx::BytesPtr co_content = pco.contentPtr();
    
    int file_size = entry.getIntField("size");
    assert(co_content->size() == file_size);
    const char *content = (const char *)Ccnx::head(*co_content);

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

static int
ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    cout << "ndnfs_create: called with path " << path << endl;
    cout << "ndnfs_create: create file with flag " << fi->flags << " and mode " << mode << endl;
    
    string dir_path, file_name;
    split_last_component(path, dir_path, file_name);
    
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
    
    // Add new file entry with empty content, "data" field is not set
    BSONObj file_entry = BSONObjBuilder().append("_id", path).append("type", file_type).append("mode", 0666)
                        .appendBinData("data", 0, BinDataGeneral, NULL).append("size", 0).obj();
    c->conn().insert(db_name, file_entry);
    // Append to existing BSON array
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$push" << BSON( "data" << file_name ) ));
    
    c->done();
    delete c;
    return 0;
}


static int
ndnfs_mkdir(const char *path, mode_t mode)
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
    
    int old_co_size;
    const char *old_co_raw = entry.getField("data").binData(old_co_size);
    
    int old_file_size = 0;
    const char *old_content = NULL;
    if (old_co_size > 0) {
        Ccnx::ParsedContentObject pco((const unsigned char *)old_co_raw, old_co_size);
        Ccnx::BytesPtr old_co_content = pco.contentPtr();
    
        old_file_size = entry.getIntField("size");
        assert(old_co_content->size() == old_file_size);
        old_content = (const char *)Ccnx::head(*old_co_content);
        
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
    
    Ccnx::Bytes co = ndn_wrapper.createContentObject(Ccnx::Name (path),
                                                           content,
                                                           file_size);
    unsigned char *co_raw = Ccnx::head(co);
    int co_size = co.size();
    BSONObj data_obj = BSONObjBuilder().appendBinData("data", co_size, BinDataGeneral, co_raw).obj();
    
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << data_obj ));
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "size" << (int)(file_size) ) ));
    
    delete content;
    
    c->done();
    delete c;
    return size;  // return the number of tyes written on success
}

static int
ndnfs_unlink(const char *path)
{
    cout << "ndnfs_unlink: called with path " << path << endl;
    
    string dir_path, file_name;
    split_last_component(path, dir_path, file_name);
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        // Cannot remove non-existing data
        c->done();
        delete c;
        return -EEXIST;
    }
    
    // Remove file entry
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
        if (s != file_name) {
            bab.append(s);
        }
    }
    
    c->conn().update(db_name, BSON("_id" << dir_path), BSON( "$set" << BSON( "data" << bab.arr() ) ));
    
    c->done();
    delete c;
    return 0;
}

static int
ndnfs_rmdir(const char *path)
{
    cout << "ndnfs_rmdir: called with path " << path << endl;
    
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
    
    // Remove file entry
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


static void
create_fuse_operations(struct fuse_operations *fuse_op)
{
    fuse_op->getattr = ndnfs_getattr;
    fuse_op->open    = ndnfs_open;
    fuse_op->read    = ndnfs_read;
    fuse_op->readdir = ndnfs_readdir;
    fuse_op->create  = ndnfs_create;
    fuse_op->write   = ndnfs_write;
    fuse_op->unlink  = ndnfs_unlink;
    fuse_op->mkdir   = ndnfs_mkdir;
    //fuse_op->rmdir   = ndnfs_unlink;
    fuse_op->rmdir   = ndnfs_rmdir;
}

static struct fuse_operations ndnfs_fs_ops;

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
    
    cout << "main: mount root directory..." << endl;
    // For test purpose, clear database first
    c->conn().dropCollection(db_name);
    c->conn().createCollection(db_name);
    
    // Check database first
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << "/"));
    if (!cursor->more()) {
        // Create root directory as an empty folder if database is empty
        BSONObj root_dir = BSONObjBuilder().append("_id", "/").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, root_dir);
        
        /* For test use, should remove later */
        // Add a file
        const char *hello = "Hello World!\n";
        Ccnx::Bytes hello_co = ndn_wrapper.createContentObject(Ccnx::Name ("/hello.txt"),
                                                         hello,
                                                         13);
        unsigned char *co_raw = Ccnx::head(hello_co);
        int co_len = hello_co.size();
        BSONObj hello_file = BSONObjBuilder().append("_id", "/hello.txt").append("type", file_type).append("mode", 0666)
                            .appendBinData("data", co_len, BinDataGeneral, co_raw).append("size", 13).obj();
        c->conn().insert(db_name, hello_file);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello.txt" ) ) );
        // Add an empty file
        BSONObj empty_file = BSONObjBuilder().append("_id", "/empty.txt").append("type", file_type).append("mode", 0666)
                            .appendBinData("data", 0, BinDataGeneral, NULL).obj();
        c->conn().insert(db_name, empty_file);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "empty.txt" ) ) );
        // Add a folder
        BSONObj hello_dir = BSONObjBuilder().append("_id", "/hello").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, hello_dir);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello" ) ) );
        
        /*
        // Test parsing content object
        auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << "/hello.txt"));
        BSONObj entry = cursor->next();
        int co_size;
        const char *co = entry.getField("data").binData(co_size);
        Ccnx::ParsedContentObject pco((const unsigned char *)co, co_size);
        
        Ccnx::BytesPtr content = pco.contentPtr();
        cout << "parsed content is: " << Ccnx::head(*content) << endl;
         */
        
        /* End of test */
    }
    cout << "main: ok" << endl;
    
    c->done();
    delete c;
    
    create_fuse_operations(&ndnfs_fs_ops);
    
    cout << "main: enter FUSE main loop" << endl;
    return fuse_main(argc, argv, &ndnfs_fs_ops, NULL);
}
