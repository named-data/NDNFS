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

inline uint64_t generate_version() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint64_t)(tv.tv_sec * (uint64_t)1000000 + tv.tv_usec);
}


inline long long get_temp_version(mongo::BSONObj& file_entry)
{
    return  file_entry.getField("temp").Long();
}


inline long long get_current_version(mongo::BSONObj& file_entry)
{
    return  file_entry.getField("data").Long();
}


inline int get_version_size(mongo::BSONObj& ver_entry)
{
    return  ver_entry.getIntField("size");
}

int get_version_size(const std::string& path, mongo::ScopedDbConnection *c, const long long ver);

long long create_temp_version(const std::string& path, mongo::ScopedDbConnection *c);

int get_current_version_size(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry);

int read_version(const std::string& ver_path, mongo::ScopedDbConnection *c, char *output, size_t size, off_t offset);

inline int read_current_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, char *output, size_t size, off_t offset)
{
    long long curr_ver = get_current_version(file_entry);
    if (curr_ver == -1)
	return -1;
    
    std::string version = boost::lexical_cast<std::string> (curr_ver);
    std::string ver_path = path + "/" + version;
    return read_version(ver_path, c, output, size, offset);
}


int write_temp_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, const char *buf, size_t size, off_t offset);

int truncate_temp_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, off_t length);

void remove_version(const std::string& ver_path, mongo::ScopedDbConnection *c);

void remove_versions(const std::string& file_path, mongo::ScopedDbConnection *c);

#endif
