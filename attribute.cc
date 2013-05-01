#include "attribute.h"

using namespace std;
using namespace boost;
using namespace mongo;

int ndnfs_getattr(const char *path, struct stat *stbuf)
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
