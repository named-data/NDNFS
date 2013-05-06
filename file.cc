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

    //TODO: check mode
    
    //BSONObj entry = cursor->next();
    //if ((fi->flags & O_ACCMODE) != O_RDONLY)
    //    return -EACCES;

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
    const char *co_raw = entry.getField("data").binData(co_size);
    ndn::ParsedContentObject pco((const unsigned char *)co_raw, co_size);
    ndn::BytesPtr co_content = pco.contentPtr();
    
    int file_size = entry.getIntField("size");
    assert(co_content->size() == file_size);
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

int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
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
        ndn::ParsedContentObject pco((const unsigned char *)old_co_raw, old_co_size);
        ndn::BytesPtr old_co_content = pco.contentPtr();
    
        old_file_size = entry.getIntField("size");
        assert(old_co_content->size() == old_file_size);
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
    
    ndn::Bytes co = ndn_wrapper.createContentObject(ndn::Name (path),
                                                           content,
                                                           file_size);
    unsigned char *co_raw = ndn::head(co);
    int co_size = co.size();
    BSONObj data_obj = BSONObjBuilder().appendBinData("data", co_size, BinDataGeneral, co_raw).obj();
    
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << data_obj ));
    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "size" << (int)(file_size) ) ));
    
    delete content;
    
    c->done();
    delete c;
    return size;  // return the number of tyes written on success
}

int ndnfs_unlink(const char *path)
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
