#ifndef NDNFS_DIRECTORY_H
#define NDNFS_DIRECTORY_H

#include "ndnfs.h"

int ndnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi);

int ndnfs_mkdir(const char *path, mode_t mode);

int ndnfs_rmdir(const char *path);


#endif
