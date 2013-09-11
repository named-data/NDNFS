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

#ifndef NDNFS_FILE_H
#define NDNFS_FILE_H

#include "ndnfs.h"
#include "version.h"

int ndnfs_open(const char *path, struct fuse_file_info *fi);

int ndnfs_create(const char *path, mode_t mode, struct fuse_file_info *fi);

int ndnfs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int ndnfs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int ndnfs_truncate(const char *path, off_t offset);

int ndnfs_unlink(const char *path);

int ndnfs_release(const char *path, struct fuse_file_info *fi);

#endif
