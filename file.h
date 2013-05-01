#ifndef NDNFS_FILE_H
#define NDNFS_FILE_H

#include "ndnfs.h"

int ndnfs_open(const char *path, struct fuse_file_info *fi);

int ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);

int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int ndnfs_unlink(const char *path);

#endif
