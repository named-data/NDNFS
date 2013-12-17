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
 *         Qiuhan Ding <dingqiuhan@gmail.com>
 */

#include "attribute.h"

using namespace std;
//using namespace boost;

int ndnfs_getattr(const char *path, struct stat *stbuf)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_getattr: path=" << path << endl;
#endif

    memset(stbuf, 0, sizeof(struct stat));
    
    int ret = 0;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
	int type = sqlite3_column_int(stmt, 2);
	
	if (type == ndnfs::dir_type) {
	    stbuf->st_mode = S_IFDIR | sqlite3_column_int(stmt, 3);
	    stbuf->st_atime = sqlite3_column_int(stmt, 4);
	    stbuf->st_mtime = sqlite3_column_int(stmt, 5);
	    stbuf->st_nlink = 1;
	} else if (type == ndnfs::file_type) {
	    stbuf->st_mode = S_IFREG | sqlite3_column_int(stmt, 3);
	    stbuf->st_atime = sqlite3_column_int(stmt, 4);
	    stbuf->st_mtime = sqlite3_column_int(stmt, 5);
	    stbuf->st_nlink = 1;
	    stbuf->st_size = sqlite3_column_int(stmt, 6);
	} else {
	    ret = -ENOENT;
	}
    
	// Use the same id for all files and dirs
	stbuf->st_uid = ndnfs::user_id;
	stbuf->st_gid = ndnfs::group_id;
    } else {
        ret = -ENOENT;
    }
    
    sqlite3_finalize(stmt);

    return ret;
}


int ndnfs_chmod(const char *path, mode_t mode)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_chmod: path=" << path << ", change mode to " << std::oct << mode << endl;
#endif
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "UPDATE file_system SET mode = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, mode);
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

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

    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
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
