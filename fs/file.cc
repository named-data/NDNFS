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

#include "file.h"

using namespace std;
//using namespace boost;

int ndnfs_open(const char *path, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_open: path=" << path << ", flag=0x" << std::hex << fi->flags << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT type, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    
    int type = sqlite3_column_int(stmt, 0);
    int tmp_ver = sqlite3_column_int(stmt, 1);
    if (type != ndnfs::file_type) {
        sqlite3_finalize(stmt);
        return -EISDIR;
    }

    sqlite3_finalize(stmt);

    // TODO: check file access mode against uid and gid

    // Check open flags
    switch (fi->flags & O_ACCMODE) {
    case O_RDONLY:
        // Nothing to be done for read only access
        break;
    case O_WRONLY:
    case O_RDWR:
        // Create temporary version for file editing
        
        // If there is already a temp version there, it means that
        // some one is writing to the file now. We should reject this 
        // open request.
        if (tmp_ver != -1) {
            return -EACCES;
        }
	
        tmp_ver = time(0);
        sqlite3_prepare_v2(db, "UPDATE file_system SET temp_version = ? WHERE path = ?;", -1, &stmt, 0);
        sqlite3_bind_int(stmt, 1, tmp_ver);
        sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
        res = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (res != SQLITE_OK && res != SQLITE_DONE)
            return -EACCES;

        break;
    default:
        break;
    }

    return 0;
}


int ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_create: path=" << path << ", flag=0x" << std::hex << fi->flags << ", mode=0" << std::oct << mode << endl;
#endif

    string dir_path, file_name;
    split_last_component(path, dir_path, file_name);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res == SQLITE_ROW) {
        // Cannot create file that has conflicting file name
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    
    sqlite3_finalize(stmt);

    //XXX: We don't check this for now.
    // // Cannot create file without creating necessary folders in advance
    // cursor = c->conn().query(db_name, QUERY("_id" << dir_path));
    // if (!cursor->more()) {
    //     c->done();
    //     delete c;
    //     return -ENOENT;
    // }
    
    // Generate temparary version for the new file
    int tmp_ver = time(0);
    
    // Create temp version for the new file
    sqlite3_prepare_v2(db, "INSERT INTO file_versions (path, version, size, totalSegments) VALUES (?, ?, ?, ?);", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, tmp_ver);
    sqlite3_bind_int(stmt, 3, 0);
    sqlite3_bind_int(stmt, 4, 0);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Add the file entry to database
    sqlite3_prepare_v2(db, 
                       "INSERT INTO file_system (path, parent, type, mode, atime, mtime, size, current_version, temp_version) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);", 
                       -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dir_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, ndnfs::file_type);
    sqlite3_bind_int(stmt, 4, mode);
    sqlite3_bind_int(stmt, 5, tmp_ver);
    sqlite3_bind_int(stmt, 6, tmp_ver);
    sqlite3_bind_int(stmt, 7, 0);  // size
    sqlite3_bind_int(stmt, 8, -1);  // current version
    sqlite3_bind_int(stmt, 9, tmp_ver);  // temp version
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    // Update mtime for parent folder
    sqlite3_prepare_v2(db, "UPDATE file_system SET mtime = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, tmp_ver);
    sqlite3_bind_text(stmt, 2, dir_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return 0;
}


int ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_read: path=" << path << ", offset=" << std::dec << offset << ", size=" << size << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT type, current_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    
    int type = sqlite3_column_int(stmt, 0);
    int curr_ver = sqlite3_column_int(stmt, 1);
    if (type != ndnfs::file_type) {
        sqlite3_finalize(stmt);
        return -EINVAL;
    }

    sqlite3_finalize(stmt);

    int size_read = read_version(path, curr_ver, buf, size, offset);

    sqlite3_prepare_v2(db, "UPDATE file_system SET atime = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, (int)time(0));
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return size_read;
}


int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_write: path=" << path << std::dec << ", size=" << size << ", offset=" << offset << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT type, current_version, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    
    int type = sqlite3_column_int(stmt, 0);
    int curr_ver = sqlite3_column_int(stmt, 1);
    int temp_ver = sqlite3_column_int(stmt, 2);
    if (type != ndnfs::file_type) {
        sqlite3_finalize(stmt);
        return -EINVAL;
    }

    sqlite3_finalize(stmt);

    // Write data to a new version of the file
    int written = write_temp_version(path, curr_ver, temp_ver, buf, size, offset);
    if (written < 0) {
        return -EINVAL;
    }

    sqlite3_prepare_v2(db, "UPDATE file_system SET size = ?, mtime = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, (int)(offset + size));
    sqlite3_bind_int(stmt, 1, (int)time(0));
    sqlite3_bind_text(stmt, 2, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return written;  // return the number of tyes written on success
}


int ndnfs_truncate(const char *path, off_t length)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_truncate: path=" << path << ", truncate to length " << std::dec << length << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT type, current_version, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }
    
    int type = sqlite3_column_int(stmt, 0);
    int curr_ver = sqlite3_column_int(stmt, 1);
    int temp_ver = sqlite3_column_int(stmt, 2);
    if (type != ndnfs::file_type) {
        sqlite3_finalize(stmt);
        return -EINVAL;
    }

    sqlite3_finalize(stmt);

    int ret = truncate_temp_version(path, curr_ver, temp_ver, length);

    sqlite3_prepare_v2(db, "UPDATE file_system SET size = ?, mtime = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, (int)length);
    sqlite3_bind_int(stmt, 2, (int)time(0));
    sqlite3_bind_text(stmt, 3, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return ret;
}


int ndnfs_unlink(const char *path)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_unlink: path=" << path << endl;
#endif

    string dir_path, file_name;
    split_last_component(path, dir_path, file_name);

    // First, remove all the versions under the file entry (both current and temp)
    remove_versions(path);    

    // Then, remove file entry
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM file_system WHERE type=1 AND path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    // Finally, update parent directory mtime
    sqlite3_prepare_v2(db, "UPDATE file_system SET mtime = ? WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, (int)time(0));
    sqlite3_bind_text(stmt, 2, dir_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;
}

int ndnfs_release(const char *path, struct fuse_file_info *fi)
{
#ifdef NDNFS_DEBUG
    cout << "ndnfs_release: path=" << path << ", flag=0x" << std::hex << fi->flags << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT type, current_version, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -ENOENT;
    }

    int type = sqlite3_column_int(stmt, 0);
    int curr_ver = sqlite3_column_int(stmt, 1);
    int temp_ver = sqlite3_column_int(stmt, 2);
    if (type != ndnfs::file_type) {
        sqlite3_finalize(stmt);
        return -EINVAL;
    }

    sqlite3_finalize(stmt);

    /*
      In FUSE design, if the file is created with create() call,
      the corresponding release() call always has fi->flags = 0.
      So we need to check the current version number to tell if
      the file is just created or opened in read only mode.
    */

    // Check open flags
    if (((fi->flags & O_ACCMODE) != O_RDONLY) || curr_ver == -1) {
        // This file is either opened with write access or just created
        if (temp_ver == -1) {
            return -1;
        }
	
        sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, temp_ver);
        int res = sqlite3_step(stmt);

        if (res != SQLITE_ROW) {
            // Nothing is written into this file so the temp version does not exist
            // In this case we simply keep the current version as it is and reset temp version number
            sqlite3_stmt *stmt2;
            sqlite3_prepare_v2(db, "UPDATE file_system SET temp_version = ? WHERE path = ?;", -1, &stmt2, 0);
            sqlite3_bind_int(stmt2, 1, -1);
            sqlite3_bind_text(stmt2, 2, path, -1, SQLITE_STATIC);
            sqlite3_step(stmt2);
            sqlite3_finalize(stmt2);
        } else {
            // Update version number and remove old version
            int size = sqlite3_column_int(stmt, 2);

            sqlite3_stmt *stmt2;
            sqlite3_prepare_v2(db, "UPDATE file_system SET size = ?, current_version = ?, temp_version = ? WHERE path = ?;", -1, &stmt2, 0);
            sqlite3_bind_int(stmt2, 1, size);
            sqlite3_bind_int(stmt2, 2, temp_ver);  // set current_version to the original temp_version
            sqlite3_bind_int(stmt2, 3, -1);  // set temp_version to -1
            sqlite3_bind_text(stmt2, 4, path, -1, SQLITE_STATIC);
            sqlite3_step(stmt2);
            sqlite3_finalize(stmt2);

            if (curr_ver != -1)
                remove_version(path, curr_ver);
        }
        sqlite3_finalize(stmt);
    }

    return 0;
}
