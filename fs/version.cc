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

#include <ndn.cxx/data.h>
#include <ndn.cxx/common.h>

using namespace std;
using namespace boost;
using namespace ndn;

int read_version(const char *path, uint64_t version, char *output, size_t size, off_t offset)
{
#ifdef NDNFS_DEBUG
  cout << "read_version: " << path << ", " << version << ", " << size << ", " << offset << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, version);

    string ver_path(path);
    ver_path += "/" + lexical_cast<string>(version);

    int res = sqlite3_step(stmt);

    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    int file_size = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);

    if (file_size <= (size_t)offset || file_size == 0) {
	return 0;
    }

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    int seg_off = seek_segment(offset);

    // Read first segment starting from some offset
    int total_read = read_segment(ver_path, seg_off, output, size, (offset - segment_to_size(seg_off)));
    cout << total_read << endl;
    if (total_read == -1) {
	return 0;
    }
    size -= total_read;
    seg_off++;

    int seg_read = 0;
    while (size > 0) {
	// Read the rest of the segments starting at zero offset
	seg_read = read_segment(ver_path, seg_off++, output + total_read, size, 0);
	if (seg_read == -1) {
	    // If anything is wrong when reading segments, just return what we have got already
	    break;
	}
	total_read += seg_read;
	size -= seg_read;
    }

    return total_read;
}


int write_temp_version(const string& path, uint64_t current_ver, uint64_t temp_ver, const char *buf, size_t size, off_t offset)
{
#ifdef NDNFS_DEBUG
  cout << "write_temp_version: " << path << ", " << current_ver << ", " << temp_ver << ", " << size << ", " << offset << endl;
#endif

  for (int i = 0; i < size; i++) {
      cout << buf[i];
    }
  cout << "haha" << endl;

    int seg_off = seek_segment(offset);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, temp_ver);

    string tmp_ver_path = path + "/" + lexical_cast<string>(temp_ver);
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
#ifdef NDNFS_DEBUG
        cout << "write_temp_version: copy-on-write" << endl;
#endif
	// *copy-on-write* strategy:
	// If temp version is not inserted, create a new entry for the temp version.
	// This only happens on the first write() after open()

        int total_segment = 0;
	if (offset > 0) {
	    // Copy data from current version 
	    sqlite3_finalize(stmt);
	    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
	    sqlite3_bind_int64(stmt,2, current_ver);
	    sqlite3_bind_text(stmt, 1,path.c_str(), -1, SQLITE_STATIC);
	    if (sqlite3_step(stmt) != SQLITE_ROW){
	        sqlite3_finalize(stmt);
	        return -1;
	    }
	    
	    if(sqlite3_column_int(stmt,2) < (size_t)offset) {
		// Trying to read after file end
	        sqlite3_finalize(stmt);
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
	        string seg_path = path + "/" + lexical_cast<string>(current_ver) + "/" + lexical_cast<string> (i);
		sqlite3_finalize(stmt);
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ?;", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
		if(sqlite3_step(stmt) != SQLITE_ROW){
		    sqlite3_finalize(stmt);
		    return -1;
		}
		
		seg_raw = (const char *)sqlite3_column_blob(stmt, 1);
		seg_len = sqlite3_column_bytes(stmt,1);
		
		Ptr<Blob> seg_Blob = Create<Blob>(seg_raw, seg_len);////
		Ptr<const Blob> seg_blob = seg_Blob;////
		Ptr<Data> seg = Data::decodeFromWire(seg_blob);////
		Blob & seg_content = seg->content();////
		data_len = (seg_content.size() > tail) ? tail : seg_content.size();////
		data = (const char*)seg_content.buf();////
		
		make_segment(path, temp_ver, i, false, data, data_len);
		    
		tail -= data_len;
		i++;
	    }
	    total_segment = i;
	    assert(i <= seg_off);
	}

        // Insert temp version entry
	sqlite3_finalize(stmt);
	sqlite3_prepare_v2(db, "INSERT INTO file_versions (path, version, size, totalSegments) VALUES (?,?,?,?);", -1, &stmt, 0);
	sqlite3_bind_text(stmt,1,path.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_int64(stmt,2,temp_ver);
	sqlite3_bind_int(stmt,3,offset);
	sqlite3_bind_int(stmt,4,total_segment);
	int res = sqlite3_step(stmt);
	cout << "res: " << std::dec << res << endl;
	if(res != SQLITE_OK && res != SQLITE_DONE){
	    sqlite3_finalize(stmt);
	    return -1;
	}
    }
	
    // From now on, we only work on the temp version
    const char *buf_pos = buf;
    size_t size_left = size;
    bool final = false;
    int tail = offset - segment_to_size(seg_off);

    cout << "tail: " << tail << endl;
    
    if (tail > 0) {	
        // Special handling for the 'seg_off' segment
        string seg_path = path + "/" + lexical_cast<string> (temp_ver) + "/" + lexical_cast<string> (seg_off);
	cout << seg_path << endl;
	sqlite3_finalize(stmt);
	sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ?;", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
	if (sqlite3_step(stmt) != SQLITE_ROW) {
	    sqlite3_finalize(stmt);
	    return -1;
	}
    	
	int seg_len = sqlite3_column_bytes(stmt,1);
	const char *seg_raw = (const char *)sqlite3_column_blob(stmt,1);
	
	if (seg_len > 0) {
	    Ptr<Blob> seg_Blob = Create<Blob>(seg_raw, seg_len);////
	    Ptr<const Blob> seg_blob = seg_Blob;////
	    Ptr<Data> seg = Data::decodeFromWire(seg_blob);////
	    Blob & seg_content = seg->content();////
	    
	    assert(tail < seg_content.size());
	    const char *old_data = (const char*)seg_content.buf();
	    
	    int copy_len = ndnfs::seg_size - tail;
	    if (copy_len > size) {
	        // The data we want to write may not fill out the rest of the segment
	        copy_len = size;
		final = true;
	    }

	    char *data = new char[tail + copy_len];
	    if (data == NULL) {
	        sqlite3_finalize(stmt);
		return -1;
	    }
	    
	    memcpy(data, old_data, tail);
	    memcpy(data + tail, buf, copy_len);
	    
	    make_segment(path, temp_ver, seg_off++, final, data, tail + copy_len);
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
    
    cout << "size_left: " << size_left << endl;
    while (size_left > 0) {
        int copy_len = ndnfs::seg_size;
	if (copy_len > size_left) {
	    copy_len = size_left;
	    final = true;
	}
	cout << "copy_len: " << copy_len << endl;
	make_segment(path, temp_ver, seg_off++, final, buf_pos, copy_len);
	buf_pos += copy_len;
	size_left -= copy_len;
    }

out:
    // Remove old segments after seg_off
    remove_segments(path, temp_ver, seg_off);

    // Update temp version entry
    sqlite3_finalize(stmt);
    sqlite3_prepare_v2(db, "UPDATE file_versions SET size = ?, totalSegments = ? WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, (int)(offset+size));
    sqlite3_bind_int(stmt, 2, seg_off);
    sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 4, temp_ver);
    int res = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (res != SQLITE_OK && res != SQLITE_DONE)
        return -1;
    else
        return size;
}


int truncate_temp_version(const string& path, uint64_t current_ver, uint64_t temp_ver, off_t length)
{
#ifdef NDNFS_DEBUG
  cout << "truncate_temp_version: " << path << ", " << current_ver << ", " << temp_ver << endl;
#endif

    int seg_off = seek_segment(length);

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, temp_ver);

    string tmp_ver_path = path + "/" + lexical_cast<string>(temp_ver);
    
    if(sqlite3_step(stmt) != SQLITE_ROW){
	// *copy-on-write* strategy:
	// If temp version is not inserted, create a new entry for the temp version.
	// This only happens on the first write() after open()

        int total_segment = 0;
	if (length > 0) {
	    // Copy data from current version 
	    sqlite3_finalize(stmt);
	    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
	    sqlite3_bind_int64(stmt,2, current_ver);
	    sqlite3_bind_text(stmt, 1,path.c_str(), -1, SQLITE_STATIC);
	    if (sqlite3_step(stmt) != SQLITE_ROW){
	        return -1;
	    }
	    
	    if (sqlite3_column_int(stmt,6)  < (size_t)length) {
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
	        string seg_path = path + "/"+ lexical_cast<string>(current_ver) + "/" + lexical_cast<string> (i);

		sqlite3_finalize(stmt);
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ?;", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
		if(sqlite3_step(stmt) != SQLITE_ROW){
		    return -1;
		}

		seg_raw = (const char *)sqlite3_column_blob(stmt, 1);
		seg_len = sqlite3_column_bytes(stmt,1);
		
		Ptr<Blob> seg_Blob = Create<Blob>(seg_raw, seg_len);////
		Ptr<const Blob> seg_blob = seg_Blob;////
		Ptr<Data> seg = Data::decodeFromWire(seg_blob);////
		Blob & seg_content = seg->content();////
        
		data_len = (seg_content.size() > tail) ? tail : seg_content.size();////
		data = (const char*)seg_content.buf();////
        
				////ndn::ParsedContentObject pco((const unsigned char *)seg_raw, seg_len);
				////ndn::BytesPtr seg_content = pco.contentPtr();
    
				////data_len = (seg_content->size() > tail) ? tail : seg_content->size();
				////data = (const char *)ndn::head(*seg_content);
		
		make_segment(path, temp_ver, i, false, data, data_len);
		
		tail -= data_len;
		i++;
	    }
	    total_segment = i;
	    assert(i <= seg_off);
	}
	
	sqlite3_finalize(stmt);
	sqlite3_prepare_v2(db, "INSERT INTO file_versions (path, version, size, totalSegments) VALUES (?,?,?,?);", -1, &stmt, 0);
	sqlite3_bind_text(stmt,1,path.c_str(),-1,SQLITE_STATIC);
	sqlite3_bind_int64(stmt,2,temp_ver);
	sqlite3_bind_int(stmt,3,length);
	sqlite3_bind_int(stmt,4,total_segment);
	int res = sqlite3_step(stmt);
	if(res != SQLITE_OK && res != SQLITE_DONE){
	    return -1;
	}
	return 0;
    }
    
    int size = sqlite3_column_int(stmt,2);

    if ((size_t)length == size) {
	return 0;
    } else if ((size_t)length < size) {
	// Truncate to length
	int seg_end = seek_segment(length);

	sqlite3_finalize(stmt);
	sqlite3_prepare_v2(db, "UPDATE file_versions SET size = ?, totalSegments = ? WHERE path = ? and version = ?;", -1, &stmt, 0);
    sqlite3_bind_int(stmt,1,(int)(length));
	sqlite3_bind_int(stmt, 2, seg_end);
    sqlite3_bind_text(stmt, 3, path.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 4, temp_ver);
    if (sqlite3_step(stmt) != SQLITE_ROW){
        return -1;
    }
    sqlite3_finalize(stmt);
	// Update version size and segment list
	int tail = length - segment_to_size(seg_end);
	truncate_segment(tmp_ver_path, seg_end, tail);
	remove_segments(path, temp_ver, seg_end + 1);
	
	return 0;
    } else {
	// TODO: pad with zeros
	return -1;
    }
}


void remove_version(const char* path, uint64_t version)
{
#ifdef NDNFS_DEBUG
  cout << "remove_version: " << path << ", " << version << endl;
#endif

    remove_segments(path, version);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM file_versions WHERE path = ? and version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, version);
    if(sqlite3_step(stmt) == SQLITE_OK){
    }
    sqlite3_finalize(stmt);
}


void remove_versions(const char* path)
{
#ifdef NDNFS_DEBUG
  cout << "remove_versions: " << path << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT current_version, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    cout << std::dec << res << endl;
    if(res == SQLITE_ROW){
        uint64_t curr_ver = sqlite3_column_int64(stmt, 0);
	if (curr_ver != -1) {
	  remove_version(path, curr_ver);
	}

	uint64_t tmp_ver = sqlite3_column_int64(stmt, 1);
	if (tmp_ver != -1) {
	    remove_version(path, tmp_ver);
	}
    }
    sqlite3_finalize(stmt);
}
