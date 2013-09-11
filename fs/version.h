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

#ifndef VERSION_H
#define VERSION_H

#include "ndnfs.h"
#include "segment.h"

inline uint64_t generate_version() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (uint64_t)(tv.tv_sec * (uint64_t)1000000 + tv.tv_usec);
}

int read_version(const char* path, const uint64_t version, char *output, size_t size, off_t offset);

int write_temp_version(const char* path, const uint64_t current_ver, const uint64_t temp_ver, const char *buf, size_t size, off_t offset);

int truncate_temp_version(const char* path, const uint64_t current_ver, const uint64_t temp_ver, off_t length);

void remove_version(const char* path, const uint64_t version);

void remove_versions(const char* path);

#endif
