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

#include "attribute.h"

using namespace std;
using namespace boost;
using namespace mongo;

int ndnfs_getattr(const char *path, struct stat *stbuf)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_getattr: called with path " << path << endl;
#endif

    memset(stbuf, 0, sizeof(struct stat));

    mongo::ScopedDbConnection *c = mongo::ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }
    
    BSONObj entry = cursor->next();  // There should be no two entries with the same id (absolute path)
    int type = entry.getIntField("type");
    
    if (type == ndnfs::dir_type) {
	stbuf->st_mode = S_IFDIR | entry.getIntField("mode");
	stbuf->st_atime = entry.getIntField("atime");
	stbuf->st_mtime = entry.getIntField("mtime");
        stbuf->st_nlink = 1;
    } else if (type == ndnfs::file_type) {
        stbuf->st_mode = S_IFREG | entry.getIntField("mode");
	stbuf->st_atime = entry.getIntField("atime");
	stbuf->st_mtime = entry.getIntField("mtime");
        stbuf->st_nlink = 1;
        stbuf->st_size = entry.getIntField("size");
    } else {
	c->done();
	delete c;
	return -ENOENT;
    }

    // Use the same id for all files and dirs
    stbuf->st_uid = ndnfs::user_id;
    stbuf->st_gid = ndnfs::group_id;
    
    c->done();
    delete c;
    return 0;
}


int ndnfs_chmod(const char *path, mode_t mode)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_chmod: called with path " << path << endl;
    cout << "ndnfs_chmod: change mode to 0" << std::oct << mode << endl;
#endif

    mongo::ScopedDbConnection *c = mongo::ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }

    c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( "mode" << (int)mode ) ));

    return 0;
}


// Dummy function to stop commands such as 'cp' from complaining

#ifdef NDNFS_OSXFUSE
int ndnfs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags, uint32_t position)
#elif NDNFS_FUSE
int ndnfs_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
#endif
{
    /*
    cout << "ndnfs_setxattr: called with path " << path << ", flag " << std::dec << flags << ", position " << position << endl;
    //cout << "ndnfs_setxattr: set attr " << name << " to " << value << endl;
    cout << "ndnfs_setxattr: set attr " << name << endl;

    mongo::ScopedDbConnection *c = mongo::ScopedDbConnection::getScopedDbConnection("localhost");
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        c->done();
        delete c;
        return -ENOENT;
    }

    // TODO: need to escape 'name' string
    //c->conn().update(db_name, BSON("_id" << path), BSON( "$set" << BSON( name << value ) ));
    */
    return 0;
}
