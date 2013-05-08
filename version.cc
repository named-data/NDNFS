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

#include "version.h"

#include <sys/time.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace boost;
using namespace mongo;


static inline uint64_t timestamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

string create_empty_version(const string& path, ScopedDbConnection *c)
{
    uint64_t now = timestamp();
    string version = lexical_cast<string> (now);
    cout << "create_version: version " << version << " created for file " << path << endl;

    string full_path = path + "/" + version;

    // Create empty segment for this version
    create_empty_segment(full_path, c);

    BSONObj ver_entry = BSONObjBuilder().append("_id", full_path).append("type", version_type).append("mode", 0666)
	.append("data", BSONArrayBuilder().append("0").arr()).append("size", 0).append("final", 0).obj();

    // Add verion entry to database
    c->conn().insert(db_name, ver_entry);
    
    return version;
}


static inline string get_latest_version_name(BSONObj& file_entry)
{
    vector< BSONElement > data = file_entry["data"].Array();
    return data[ data.size() - 1 ].String();
}


static inline int get_version_info(BSONObj& ver_entry, int& mode, int& size)
{
    size = ver_entry.getIntField("size");
    mode = ver_entry.getIntField("mode");
    return 0;
}

int get_latest_version_info(const string& path, ScopedDbConnection *c, BSONObj& file_entry, int& mode, int& size)
{
    string version = get_latest_version_name(file_entry);
    string full_path = path + "/" + version;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << full_path));
    if (!cursor->more()) {
        return -1;
    }

    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
        return -1;
    }
    
    return get_version_info(ver_entry, mode, size);
}

int read_latest_version(const string& path, ScopedDbConnection *c, BSONObj& file_entry, char *output, size_t size, off_t offset)
{
    string version = get_latest_version_name(file_entry);
    string ver_path = path + "/" + version;
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << ver_path));
    if (!cursor->more()) {
	return -ENOENT;
    }
    
    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
	return -EINVAL;
    }

    int file_size = ver_entry.getIntField("size");
    if (file_size <= (size_t)offset || file_size == 0) {
	return 0;
    }

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    int seg_off = seek_segment(offset);

    // Read first segment starting from some offset
    int total_read = read_segment(ver_path, c, seg_off, output, size, (offset - segment_to_size(seg_off)));
    if (total_read == -1) {
	return 0;
    }
    size -= total_read;
    seg_off++;

    int seg_read = 0;
    while (size > 0) {
	// Read the rest of the segments starting at zero offset
	seg_read = read_segment(ver_path, c, seg_off++, output + total_read, size, 0);
	if (seg_read == -1) {
	    // If anything is wrong when reading segments, just return what we have got already
	    break;
	}
	total_read += seg_read;
	size -= seg_read;
    }

    return total_read;
}

string add_version(const string& path, ScopedDbConnection *c, BSONObj& file_entry, const char *buf, size_t size, off_t offset)
{
    uint64_t ver_num = timestamp();
    string version = lexical_cast<string> (ver_num);
    cout << "add_version_with_data: version " << version << " created for file " << path << endl;

    const char *buf_pos = buf;
    size_t size_left = size;
    bool final = false;
    int seg_off = seek_segment(offset);

    if (offset > 0) {
	// Copy data from last version
	string last_ver = get_latest_version_name(file_entry);
	string last_ver_path = path + "/" + last_ver;
	auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << last_ver_path));
	if (!cursor->more()) {
	    return string();
	}
    
	BSONObj last_ver_entry = cursor->next();
	if (last_ver_entry.getIntField("type") != version_type) {
	    return string();
	}

	int old_file_size = last_ver_entry.getIntField("size");
	if (old_file_size < (size_t)offset) {
	    // Trying to read after file end
	    return string();
	}
	
	// Copy old data before the offset
	int old_seg_size;
	const char *old_seg_raw;
	int old_data_len;
	const char *old_data;
	for (int i = 0; i < seg_off; i++) {
	    string last_seg_path = last_ver_path + "/" + lexical_cast<string> (i);

	    cursor = c->conn().query(db_name, QUERY("_id" << last_seg_path));
	    if (!cursor->more()) {
		return string();
	    }

	    BSONObj seg_entry = cursor->next();
	    if (seg_entry.getIntField("type") != segment_type) {
		return string();
	    }
    
	    get_segment_data_raw(seg_entry, old_seg_raw, old_seg_size);

	    if (old_seg_size > 0) {
		ndn::ParsedContentObject pco((const unsigned char *)old_seg_raw, old_seg_size);
		ndn::BytesPtr old_seg_content = pco.contentPtr();
    
		old_data_len = old_seg_content->size();
		old_data = (const char *)ndn::head(*old_seg_content);
                
		make_segment(path, c, ver_num, i, false, old_data, old_data_len);
	    }
	}

	// Special handling for the 'seg_off' segment
	string last_seg_path = last_ver_path + "/" + lexical_cast<string> (seg_off);

	cursor = c->conn().query(db_name, QUERY("_id" << last_seg_path));
	if (!cursor->more()) {
	    return string();
	}

	BSONObj seg_entry = cursor->next();
	if (seg_entry.getIntField("type") != segment_type) {
	    return string();
	}
    
	get_segment_data_raw(seg_entry, old_seg_raw, old_seg_size);

	if (old_seg_size > 0) {
	    ndn::ParsedContentObject pco((const unsigned char *)old_seg_raw, old_seg_size);
	    ndn::BytesPtr old_seg_content = pco.contentPtr();

	    old_data_len = offset - segment_to_size(seg_off);
	    assert(old_data_len <= old_seg_content->size());
	    old_data = (const char *)ndn::head(*old_seg_content);

	    int copy_len = seg_size - old_data_len;
	    if (copy_len > size) {
		copy_len = size;
		final = true;
	    }

	    char *data = new char[old_data_len + copy_len];
	    if (data == NULL) {
		return string();
	    }
	    
	    memcpy(data, old_data, old_data_len);
	    memcpy(data + old_data_len, buf, copy_len);
	    
	    make_segment(path, c, ver_num, seg_off++, final, data, old_data_len + copy_len);
	    delete data;
	    
	    if (final) {
		// Then we are done
		goto out;
	    }
	    
	    // Else, move pointer forward
	    buf_pos += copy_len;
	    size_left -= copy_len;
	}
    }
    
    while (size_left > 0) {
	int copy_len = seg_size;
	if (copy_len > size_left) {
	    copy_len = size_left;
	    final = true;
	}

	make_segment(path, c, ver_num, seg_off++, final, buf_pos, copy_len);
	buf_pos += copy_len;
	size_left -= copy_len;
    }

out:
    // Create new version entry
    BSONArrayBuilder bab;
    for (int i = 0; i < seg_off; i++) {
	bab.append( lexical_cast<string> (i) );
    }
    BSONObj ver_entry = BSONObjBuilder().append("_id", path + "/" + version).append("type", version_type).append("mode", 0666)
	.append("data", bab.arr()).append("size", (int)(offset + size)).append("final", (seg_off - 1)).obj();

    // Add verion entry to database
    c->conn().insert(db_name, ver_entry);

    return version;
}


void remove_versions(const string& path, ScopedDbConnection *c)
{
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << path));
    if (!cursor->more()) {
        return;
    }

    BSONObj file_entry = cursor->next();
    
    vector< BSONElement > vers = file_entry["data"].Array();
    for (int i = 0; i < vers.size(); i++) {
        string ver_path = path + "/" + vers[i].String();
	remove_segments(ver_path, c);
	c->conn().remove(db_name, QUERY("_id" << ver_path));
    }
}
