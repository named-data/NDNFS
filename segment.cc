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

#include "segment.h"

#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace mongo;

int create_empty_segment(const string& ver_path, ScopedDbConnection *c)
{
    int seg_num = 0;
    string segment = lexical_cast<string> (seg_num);
    string full_path = ver_path + "/" + segment;
    BSONObj seg_entry = BSONObjBuilder().append("_id", full_path).append("type", segment_type)
	.appendBinData("data", 0, BinDataGeneral, NULL).append("offset", 0).obj();

    // Add segment entry to database
    c->conn().insert(db_name, seg_entry);
    
    return 0;
}

int read_segment(const string& ver_path, ScopedDbConnection *c, const int seg, char *output, const int limit, const int offset)
{
    string segment = lexical_cast<string> (seg);
    string seg_path = ver_path + "/" + segment;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
    if (!cursor->more()) {
	return -1;
    }
    
    BSONObj seg_entry = cursor->next();
    if (seg_entry.getIntField("type") != segment_type) {
	return -1;
    }

    int co_size;
    const char *co_raw = get_segment_data_raw(seg_entry, co_size);
    if (co_size == 0) {
	// This should not happen usually
	return -1;
    }

    ndn::ParsedContentObject pco((const unsigned char *)co_raw, co_size);
    ndn::BytesPtr co_content = pco.contentPtr();    
    const char *content = (const char *)ndn::head(*co_content);

    int copy_len = co_content->size();
    if (copy_len > limit)  // Don't write across the limit
	copy_len = limit;

    memcpy(output, content + offset, copy_len);
    
    return copy_len;
}

int make_segment(const string& file_path, ScopedDbConnection *c, const uint64_t ver, const int seg, const bool final, const char *data, const int len)
{
    string version = lexical_cast<string> (ver);
    string segment = lexical_cast<string> (seg);
    string full_path = file_path + "/" + version + "/" + segment;

    ndn::Name seg_name(file_path);
    seg_name.appendVersion(ver);
    seg_name.appendSeqNum(seg);

    ndn::Bytes co = ndn_wrapper.createContentObject(seg_name, data, len);
    unsigned char *co_raw = ndn::head(co);
    int co_size = co.size();

    BSONObj seg_entry = BSONObjBuilder().append("_id", full_path).append("type", segment_type)
	.appendBinData("data", co_size, BinDataGeneral, co_raw).append("offset", segment_to_size(seg)).obj();

    // Add segment entry to database
    c->conn().insert(db_name, seg_entry);

    return 0;
}

void remove_segments(const string& ver_path, ScopedDbConnection *c)
{
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << ver_path));
    if (!cursor->more()) {
        return;
    }

    BSONObj ver_entry = cursor->next();
    
    vector< BSONElement > segs = ver_entry["data"].Array();
    for (int i = 0; i < segs.size(); i++) {
        string seg_path = ver_path + "/" + segs[i].String();
	c->conn().remove(db_name, QUERY("_id" << seg_path));
    }
}
