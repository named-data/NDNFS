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


inline int split_last_component(const char *path, std::string &prefix, std::string &name)
{
    std::string abs_path(path);
    size_t last_comp_pos = abs_path.rfind('/');
    if (last_comp_pos == std::string::npos)
        return -1;
    
    prefix = abs_path.substr(0, last_comp_pos);
    if (prefix.empty())
        prefix = "/";
    name = abs_path.substr(last_comp_pos + 1);
    
    return 0;
}

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

#endif
