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

#ifndef VERSION_H
#define VERSION_H

#include "ndnfs.h"
#include "segment.h"

inline long long generate_version() {
    //std::cout << "generate_version: called" << std::endl;
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (long long)(tv.tv_sec * (uint64_t)1000000 + tv.tv_usec);
}


//inline long long get_temp_version(mongo::BSONObj& file_entry)
//{
//    return  file_entry.getField("temp").Long();
//}


//inline long long get_current_version(mongo::BSONObj& file_entry)
//{
//    return  file_entry.getField("data").Long();
//}


//inline int get_version_size(mongo::BSONObj& ver_entry)
//{
//    return  ver_entry.getIntField("size");
//}

//int get_version_size(const std::string& path, mongo::ScopedDbConnection *c, const long long ver);

long long create_temp_version(const std::string& path, mongo::ScopedDbConnection *c);

//int get_current_version_size(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry);

int read_version(const std::string& ver_path, char *output, size_t size, off_t offset);

inline int read_current_version(const std::string& path, char *output, size_t size, off_t offset)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(db, 1, path.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != ROW){
        return -1;
    }
    uint64_t curr_ver = sqlite3_column_int64(stmt, 1);
    //long long curr_ver = get_current_version(file_entry);
    if (curr_ver == -1)
	return -1;
    
    std::string version = boost::lexical_cast<std::string> (curr_ver);
    std::string ver_path = path + "/" + version;
    return read_version(ver_path, output, size, offset);
    sqlite3_finalize(stmt);
}


int write_temp_version(const std::string& path, const char *buf, size_t size, off_t offset);

int truncate_temp_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, off_t length);

void remove_version(const std::string& ver_path, mongo::ScopedDbConnection *c);

void remove_versions(const std::string& file_path, mongo::ScopedDbConnection *c);

#endif
