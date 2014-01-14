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

#include "directory.h"

using namespace std;

int ndnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_readdir: path=" << path << endl;
#endif    

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    sqlite3_finalize(stmt);
    
    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE parent = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string path((const char *)sqlite3_column_text(stmt, 0));
        size_t last_comp_pos = path.rfind('/');
        if (last_comp_pos == std::string::npos)
            continue;
        
        string name = path.substr(last_comp_pos + 1);
        filler(buf, name.c_str(), NULL, 0);
    }

    sqlite3_finalize(stmt);

    return 0;
}

int ndnfs_mkdir(const char *path, mode_t mode)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_mkdir: path=" << path << ", mode=0" << std::oct << mode << endl;
#endif

    string dir_path, dir_name;
    split_last_component(path, dir_path, dir_name);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (res == SQLITE_ROW) {
        // Cannot create file that has conflicting file name
        return -EEXIST;
    }
    
    // Add new file entry with empty content
    int now = time(0);
    sqlite3_prepare_v2(db, "INSERT INTO file_system (path, parent, type, mode, atime, mtime, size, current_version, temp_version) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dir_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, ndnfs::dir_type);
    sqlite3_bind_int(stmt, 4, mode);
    sqlite3_bind_int(stmt, 5, now);
    sqlite3_bind_int(stmt, 6, now);
    sqlite3_bind_int(stmt, 7, -1);  // size
    sqlite3_bind_int64(stmt, 8, -1); // current_ver
    sqlite3_bind_int64(stmt, 9, -1); // temp_ver
    res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (res == SQLITE_OK || res == SQLITE_DONE)
        return 0;
    else
        return -EACCES;
}


/*
 * For rmdir, we don't need to implement recursive remove,
 * because 'rm -r' will iterate all the sub-entries (dirs or
 * files) for us and remove them one-by-one.   ---SWT
 */
int ndnfs_rmdir(const char *path)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_rmdir: path=" << path << endl;
#endif

    if (strcmp(path, "/") == 0) {
        // Cannot remove root dir.
        // This should not happen in the real world.
        return -EINVAL;
    }
    
    string parent_dir_path, dir_name;
    split_last_component(path, parent_dir_path, dir_name);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (res != SQLITE_ROW) {
        // Cannot remove non-existing data
        return -EEXIST;
    }
    
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE parent = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (res == SQLITE_ROW) {
	// Cannot remove non-empty dir
        return -ENOTEMPTY;
    }
    
    // Remove dir entry
    sqlite3_prepare_v2(db, "DELETE FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;
}
