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

#ifndef NDNFS_H
#define NDNFS_H

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>

#include <fuse.h>
#include <mongo/client/dbclient.h>
#include <ndn.cxx.h>

extern ndn::Wrapper ndn_wrapper;
extern const char *db_name;

extern const int dir_type;
extern const int file_type;
extern const int version_type;
extern const int segment_type;


inline int split_last_component(const std::string& path, std::string &prefix, std::string &name)
{
    size_t last_comp_pos = path.rfind('/');
    if (last_comp_pos == std::string::npos)
        return -1;
    
    prefix = path.substr(0, last_comp_pos);
    if (prefix.empty())
        prefix = "/";
    name = path.substr(last_comp_pos + 1);
    
    return 0;
}

inline int split_last_component(const char *path, std::string &prefix, std::string &name)
{
    std::string path_string(path);
    return split_last_component(path_string, prefix, name);
}

#endif
