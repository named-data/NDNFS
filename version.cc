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

using namespace std;
using namespace boost;
using namespace mongo;


int get_version_size(const string& path, ScopedDbConnection *c, const long long ver)
{
    string ver_path = path + "/" + lexical_cast<string> (ver);

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << ver_path));
    if (!cursor->more()) {
        return -1;
    }

    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
        return -1;
    }
    
    return get_version_size(ver_entry);
}

int get_current_version_size(const string& path, ScopedDbConnection *c, BSONObj& file_entry)
{
    string version = lexical_cast<string> (get_current_version(file_entry));
    string ver_path = path + "/" + version;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << ver_path));
    if (!cursor->more()) {
        return -1;
    }

    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
        return -1;
    }
    
    return get_version_size(ver_entry);
}


int read_version(const string& ver_path, ScopedDbConnection *c, char *output, size_t size, off_t offset)
{
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


int write_temp_version(const string& path, ScopedDbConnection *c, BSONObj& file_entry, const char *buf, size_t size, off_t offset)
{
    int seg_off = seek_segment(offset);

    long long tmp_ver_num = get_temp_version(file_entry);
    string tmp_ver = lexical_cast<string> (tmp_ver_num);
    string tmp_ver_path = path + "/" + tmp_ver;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << tmp_ver_path));
    if (!cursor->more()) {
	// *copy-on-write* strategy:
	// If temp version is not inserted, create a new entry for the temp version.
	// This only happens on the first write() after open()
	BSONArrayBuilder bab;

	if (offset > 0) {
	    // Copy data from current version
	    string curr_ver_path = path + "/" + lexical_cast<string> (get_current_version(file_entry));
	    cursor = c->conn().query(db_name, QUERY("_id" << curr_ver_path));
	    if (!cursor->more()) {
		return -1;
	    }
	
	    BSONObj curr_ver_entry = cursor->next();
	    if (curr_ver_entry.getIntField("type") != version_type) {
		return -1;
	    }

	    if (curr_ver_entry.getIntField("size") < (size_t)offset) {
		// Trying to read after file end
		return -1;
	    }
	
	    // Copy old data before the offset
	    int seg_len;
	    const char *seg_raw;
	    int data_len;
	    const char *data;
	    int i = 0;
	    int tail = offset;
	    while (tail > 0) {
		string seg_path = curr_ver_path + "/" + lexical_cast<string> (i);

		cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
		if (!cursor->more()) {
		    return -1;
		}

		BSONObj seg_entry = cursor->next();
		if (seg_entry.getIntField("type") != segment_type) {
		    return -1;
		}

		seg_raw = get_segment_data_raw(seg_entry, seg_len);

		ndn::ParsedContentObject pco((const unsigned char *)seg_raw, seg_len);
		ndn::BytesPtr seg_content = pco.contentPtr();
    
		data_len = (seg_content->size() > tail) ? tail : seg_content->size();
		data = (const char *)ndn::head(*seg_content);
		
		make_segment(path, c, tmp_ver_num, i, false, data, data_len);
		    
		bab.append(i);
		
		tail -= data_len;
		i++;
	    }
	    assert(i <= seg_off);
	}

        // Insert temp version entry
	BSONObj ver_entry = BSONObjBuilder().append("_id", tmp_ver_path).append("type", version_type)
	    .append("data", bab.arr()).append("size", offset).obj();

        // Add verion entry to database
	c->conn().insert(db_name, ver_entry);
    
	// Update cursor
	cursor = c->conn().query(db_name, QUERY("_id" << tmp_ver_path));
	if (!cursor->more()) {
	    // If this fails, then we have something really bad
	    return -1;
	}
    }

    // From now on, we only work on the temp version
    BSONObj tmp_ver_entry = cursor->next();
    if (tmp_ver_entry.getIntField("type") != version_type) {
	// This should never happen
	return -1;
    }

    const char *buf_pos = buf;
    size_t size_left = size;
    bool final = false;
    int tail = offset - segment_to_size(seg_off);

    if (tail > 0) {	
	// Special handling for the 'seg_off' segment
	string seg_path = tmp_ver_path + "/" + lexical_cast<string> (seg_off);

	cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
	if (!cursor->more()) {
	    return -1;
	}

	BSONObj seg_entry = cursor->next();
	if (seg_entry.getIntField("type") != segment_type) {
	    return -1;
	}
    	
	int seg_len;
	const char *seg_raw = get_segment_data_raw(seg_entry, seg_len);

	if (seg_len > 0) {
	    ndn::ParsedContentObject pco((const unsigned char *)seg_raw, seg_len);
	    ndn::BytesPtr seg_content = pco.contentPtr();

	    assert(tail < seg_content->size());
	    const char *old_data = (const char *)ndn::head(*seg_content);

	    int copy_len = seg_size - tail;
	    if (copy_len > size) {
		// The data we want to write may not fill out the rest of the segment
		copy_len = size;
		final = true;
	    }

	    char *data = new char[tail + copy_len];
	    if (data == NULL) {
		return -1;
	    }
	    
	    memcpy(data, old_data, tail);
	    memcpy(data + tail, buf, copy_len);
	    
	    make_segment(path, c, tmp_ver_num, seg_off++, final, data, tail + copy_len);
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

	make_segment(path, c, tmp_ver_num, seg_off++, final, buf_pos, copy_len);
	buf_pos += copy_len;
	size_left -= copy_len;
    }

out:
    // Update temp version entry
    BSONArrayBuilder bab;
    for (int i = 0; i < seg_off; i++) {
	bab.append( lexical_cast<string> (i) );
    }
    c->conn().update(db_name, BSON("_id" << tmp_ver_path), BSON( "$set" << BSON( "data" << bab.arr() << "size" << (int)(offset + size) ) ));

    return size;
}


int truncate_temp_version(const string& path, ScopedDbConnection *c, BSONObj& file_entry, off_t length)
{
    int seg_off = seek_segment(length);

    long long tmp_ver_num = get_temp_version(file_entry);
    string tmp_ver = lexical_cast<string> (tmp_ver_num);
    string tmp_ver_path = path + "/" + tmp_ver;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << tmp_ver_path));
    if (!cursor->more()) {
        // *copy-on-write* strategy:
	// If temp version is not inserted, create a new entry for the temp version.
	// This only happens on the first truncate() after open()
	BSONArrayBuilder bab;

	if (length > 0) {
	    // Copy data from current version
	    string curr_ver_path = path + "/" + lexical_cast<string> (get_current_version(file_entry));
	    cursor = c->conn().query(db_name, QUERY("_id" << curr_ver_path));
	    if (!cursor->more()) {
		return -1;
	    }
	
	    BSONObj curr_ver_entry = cursor->next();
	    if (curr_ver_entry.getIntField("type") != version_type) {
		return -1;
	    }

	    if (curr_ver_entry.getIntField("size") < (size_t)length) {
		// TODO: pad zeros
		return -1;
	    }
	
	    // Copy old data before the offset
	    int seg_len;
	    const char *seg_raw;
	    int data_len;
	    const char *data;
	    int i = 0;
	    int tail = length;
	    while (tail > 0) {
		string seg_path = curr_ver_path + "/" + lexical_cast<string> (i);

		cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
		if (!cursor->more()) {
		    return -1;
		}

		BSONObj seg_entry = cursor->next();
		if (seg_entry.getIntField("type") != segment_type) {
		    return -1;
		}

		seg_raw = get_segment_data_raw(seg_entry, seg_len);

		ndn::ParsedContentObject pco((const unsigned char *)seg_raw, seg_len);
		ndn::BytesPtr seg_content = pco.contentPtr();
    
		data_len = (seg_content->size() > tail) ? tail : seg_content->size();
		data = (const char *)ndn::head(*seg_content);
		
		make_segment(path, c, tmp_ver_num, i, false, data, data_len);
		    
		bab.append(i);
		
		tail -= data_len;
		i++;
	    }
	    assert(i <= seg_off);
	}

        // Insert temp version entry
	BSONObj ver_entry = BSONObjBuilder().append("_id", tmp_ver_path).append("type", version_type)
	    .append("data", bab.arr()).append("size", length).obj();

        // Add verion entry to database
	c->conn().insert(db_name, ver_entry);
    
	return 0;
    }
    
    BSONObj tmp_ver_entry = cursor->next();
    if (tmp_ver_entry.getIntField("type") != version_type) {
        return -EINVAL;
    }

    int size = tmp_ver_entry.getIntField("size");

    if ((size_t)length == size) {
	return 0;
    } else if ((size_t)length < size) {
	// Truncate to length
	int seg_end = seek_segment(length);

	// Update version size and segment list
	c->conn().update(db_name, BSON("_id" << tmp_ver_path), BSON( "$set" << BSON( "size" << (int)length ) ));
	BSONArrayBuilder bab;
	for (int i = 0; i <= seg_end; i++) {
	    bab.append( lexical_cast<string> (i) );
	}
	c->conn().update(db_name, BSON("_id" << tmp_ver_path), BSON( "$set" << BSON( "data" << bab.arr() ) ));

	int tail = length - segment_to_size(seg_end);
	truncate_segment(tmp_ver_path, c, seg_end, tail);
	remove_segments(tmp_ver_path, c, seg_end + 1);
	
	return 0;
    } else {
	// TODO: pad with zeros
	return -1;
    }
}


void remove_version(const string& ver_path, ScopedDbConnection *c)
{
    remove_segments(ver_path, c);
    c->conn().remove(db_name, QUERY("_id" << ver_path));
}


void remove_versions(const string& file_path, ScopedDbConnection *c)
{
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << file_path));
    if (!cursor->more()) {
        return;
    }

    BSONObj file_entry = cursor->next();
    long long curr_ver = get_current_version(file_entry);
    if (curr_ver != -1) {
	string curr_ver_path = file_path + "/" + lexical_cast<string> (curr_ver);
	remove_version(curr_ver_path, c);
    }

    long long tmp_ver = get_temp_version(file_entry);
    if (tmp_ver != -1) {
	string tmp_ver_path = file_path + "/" + lexical_cast<string> (tmp_ver);
	remove_version(tmp_ver_path, c);
    }
}
