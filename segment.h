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

#ifndef NDNFS_SEGMENT_H
#define NDNFS_SEGMENT_H

#include "ndnfs.h"


inline int seek_segment(int doff)
{
    return (doff >> seg_size_shift);
}

inline int segment_to_size(int seg)
{
    return (seg << seg_size_shift);
}

inline const char* get_segment_data_raw(mongo::BSONObj& seg_entry, int& data_len)
{
    if (seg_entry.getIntField("size") == 0) {
	data_len = 0;
	return NULL;
    } else {
	return seg_entry.getField("data").binData(data_len);
    }
}

int read_segment(const std::string& ver_path, mongo::ScopedDbConnection *c, const int seg, char *output, const int limit, const int offset);

int make_segment(const std::string& file_path, mongo::ScopedDbConnection *c, const uint64_t ver, const int seg, const bool final, const char *data, const int len);

void remove_segments(const std::string& ver_path, mongo::ScopedDbConnection *c, const int start = 0);

void truncate_segment(const std::string& ver_path, mongo::ScopedDbConnection *c, const int seg, const off_t length);

#endif
