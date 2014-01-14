/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Wentao Shang <wentao@cs.ucla.edu>
 *         Qiuhan Ding <dingqiuhan@gmail.com>
 */

#ifndef NDNFS_SEGMENT_H
#define NDNFS_SEGMENT_H

#include "ndnfs.h"

inline int seek_segment(int doff)
{
    return (doff >> ndnfs::seg_size_shift);
}

inline int segment_to_size(int seg)
{
    return (seg << ndnfs::seg_size_shift);
}

int read_segment(const char* path, const int ver, const int seg, char *output, const int limit, const int offset);

int make_segment(const char* path, const int ver, const int seg, const char *data, const int len);

void remove_segments(const char* path, const int ver, const int start = 0);

void truncate_segment(const char* path, const int ver, const int seg, const off_t length);

#endif
