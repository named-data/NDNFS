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
    
    if (type == dir_type) {
	stbuf->st_mode = S_IFDIR | entry.getIntField("mode");
	stbuf->st_atime = entry.getIntField("atime");
	stbuf->st_mtime = entry.getIntField("mtime");
        stbuf->st_nlink = 1;
    } else if (type == file_type) {
	int mode, size;
	if (get_latest_version_info(path, c, entry, mode, size) == -1) {
	    c->done();
	    delete c;
	    return -ENOENT;
	}
	
        stbuf->st_mode = S_IFREG | entry.getIntField("mode");
	stbuf->st_atime = entry.getIntField("atime");
	stbuf->st_mtime = entry.getIntField("mtime");
        stbuf->st_nlink = 1;
        stbuf->st_size = size;
    } else {
	c->done();
	delete c;
	return -ENOENT;
    }

    // Use the same id for all files and dirs
    stbuf->st_uid = user_id;
    stbuf->st_gid = group_id;
    
    c->done();
    delete c;
    return 0;
}
