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

#include "version.h"

#include <ndn-cpp/data.hpp>
#include <ndn-cpp/common.hpp>

using namespace std;
using namespace ndn;

int read_version(const char *path, const int ver, char *output, size_t size, off_t offset)
{
#ifdef NDNFS_DEBUG
    cout << "read_version: path=" << path << std::dec << ", ver=" << ver << ", size=" << size << ", offset=" << offset << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, ver);

    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    int file_size = sqlite3_column_int(stmt, 2);
    sqlite3_finalize(stmt);

    if (file_size <= (size_t)offset || file_size == 0)
        return 0;

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    int seg_off = seek_segment(offset);

    // Read first segment starting from some offset
    int total_read = read_segment(path, ver, seg_off, output, size, (offset - segment_to_size(seg_off)));
    if (total_read == -1) {
        return 0;
    }
    size -= total_read;
    seg_off++;

    int seg_read = 0;
    while (size > 0) {
        // Read the rest of the segments starting at zero offset
        seg_read = read_segment(path, ver, seg_off++, output + total_read, size, 0);
        if (seg_read == -1) {
            // If anything is wrong when reading segments, just return what we have got already
            break;
        }
        total_read += seg_read;
        size -= seg_read;
    }

    return total_read;
}


////////////////////////////////////////////////////////
// Caution: the following function does the hack to 
// add "final block id" to the first segment of file 
// data. It is so hacky that the author does not even
// bother to write any comment for it.
////////////////////////////////////////////////////////
void update_fbi (const char *path, const int ver, int fbi)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, ver);
    sqlite3_bind_int(stmt, 3, 0);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return;
    }
    const char * data = (const char *)sqlite3_column_blob(stmt, 3);
    int len = sqlite3_column_bytes(stmt, 3);
    Data seg;
    seg.wireDecode((const uint8_t*)data, len);
    seg.getMetaInfo().setFinalBlockID(Name().appendSegment(fbi).get(0).getValue());
    keychain->signByIdentity(seg, ndnfs::signer);
    SignedBlob wire_data = seg.wireEncode();
    const char* co_raw = (const char*)wire_data.buf();
    int co_size = wire_data.size();
    sqlite3_finalize(stmt);
    sqlite3_prepare_v2(db, "UPDATE file_segments SET data = ? WHERE path = ? AND version = ?  AND segment = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt,2,path,-1,SQLITE_STATIC);
    sqlite3_bind_int(stmt,3,ver);
    sqlite3_bind_int(stmt,4,0);
    sqlite3_bind_blob(stmt,1,co_raw,co_size,SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}


int duplicate_version (const char *path, const int from_ver, const int to_ver)
{
#ifdef NDNFS_DEBUG
  cout << "duplicate_version: path=" << path << std::dec << ", from=" << from_ver << ", to=" << to_ver << endl;
#endif

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2 (db, "SELECT size, totalSegments FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
  sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
  sqlite3_bind_int (stmt, 2, from_ver);
  if (sqlite3_step (stmt) != SQLITE_ROW)
    {
      sqlite3_finalize (stmt);
      return -1;
    }
  int ver_size = sqlite3_column_int (stmt, 0);
  int total_seg = sqlite3_column_int (stmt, 1);
  
  int seg_len;
  const char *seg_raw;
  int data_len;
  const char *data;
  for (int i = 0; i < total_seg; i++)
    {
      // Copy each segment
      sqlite3_finalize (stmt);
      sqlite3_prepare_v2 (db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?;", -1, &stmt, 0);
      sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
      sqlite3_bind_int (stmt, 2, from_ver);
      sqlite3_bind_int (stmt, 3, i);
      if (sqlite3_step (stmt) != SQLITE_ROW)
	{
	  sqlite3_finalize (stmt);
	  return -1;
	}
		
      seg_raw = (const char *) sqlite3_column_blob (stmt, 3);
      seg_len = sqlite3_column_bytes (stmt, 3);
		
      Data seg;
      seg.wireDecode ((const uint8_t*) seg_raw, seg_len);
      const Blob& seg_content = seg.getContent ();
      data_len = seg_content.size ();
      data = (const char*) seg_content.buf ();
		
      make_segment (path, to_ver, i, data, data_len);
    }

  // Insert "to" version entry
  sqlite3_finalize (stmt);
  sqlite3_prepare_v2 (db, "INSERT INTO file_versions (path, version, size, totalSegments) VALUES (?,?,?,?);", -1, &stmt, 0);
  sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
  sqlite3_bind_int (stmt, 2, to_ver);
  sqlite3_bind_int (stmt, 3, ver_size);
  sqlite3_bind_int (stmt, 4, total_seg);
  int res = sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  if (res != SQLITE_OK && res != SQLITE_DONE)
    return -1;

  return 0;
}


// Write data to the specified version and return the new file size for that version
int write_version (const char* path, const int ver, const char *buf, size_t size, off_t offset)
{
#ifdef NDNFS_DEBUG
  cout << "write_version: path=" << path << std::dec << ", ver=" << ver << ", size=" << size << ", offset=" << offset << endl;
  //cout << "write_version: content to write is " << endl;
  //for (int i = 0; i < size; i++)
  // {
  //   cout << buf[i];
  // }
  //cout << endl;
#endif
  
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2 (db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
  sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
  sqlite3_bind_int (stmt, 2, ver);
  
  if (sqlite3_step (stmt) != SQLITE_ROW)
    {
      // Should not happen
      sqlite3_finalize (stmt);
      return -1;
    }

  int old_ver_size = sqlite3_column_int (stmt, 2);
  int old_total_seg = sqlite3_column_int (stmt, 3);
  const char *buf_pos = buf;
  size_t size_left = size;
  int seg_off = seek_segment (offset);
  int tail = offset - segment_to_size (seg_off);
  
  if (tail > 0)
    {
      // Special handling for the boundary segment
      // Keep what is remaining in that segment before 'offset' and override the data after 'offset'
      sqlite3_finalize (stmt);
      sqlite3_prepare_v2 (db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?;", -1, &stmt, 0);
      sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
      sqlite3_bind_int (stmt, 2, ver);
      sqlite3_bind_int (stmt, 3, seg_off);
      if (sqlite3_step (stmt) != SQLITE_ROW)
	{
	  // Should not happen
	  sqlite3_finalize (stmt);
	  return -1;
	}
      
      int seg_len = sqlite3_column_bytes (stmt, 3);
      const char *seg_raw = (const char *) sqlite3_column_blob (stmt, 3);
      
      Data seg;
      seg.wireDecode ((const uint8_t*) seg_raw, seg_len);
      const Blob& seg_content = seg.getContent ();
      int seg_content_len = seg_content.size ();
      assert (tail <= seg_content_len);
      const uint8_t *old_data = seg_content.buf ();
	    
      int copy_len = ndnfs::seg_size - tail;  // This is what we can copy at most in this segment
      bool final = false;
      if (copy_len > size)
	{
	  // The data we want to write may not fill out the rest of the segment
	  copy_len = size;
	  final = true;
	}

      char *data = new char[ndnfs::seg_size];
      if (data == NULL)
	{
	  sqlite3_finalize (stmt);
	  return -1;
	}
	    
      memcpy (data, old_data, seg_content_len);  // Copy everything from old data
      memcpy (data + tail, buf, copy_len);  // Overwrite the part we want to write to

      // The final size of this segment content is the maximum of the old size and the new size
      int updated_seg_len = tail + copy_len;
      updated_seg_len = seg_content_len > updated_seg_len ? seg_content_len : updated_seg_len;
      make_segment (path, ver, seg_off++, data, updated_seg_len);
      delete data;
	    
      if (final)
	{
	  // Then we are done
	  goto out;
	}	
      // Else, move pointer forward
      buf_pos += copy_len;
      size_left -= copy_len;
    }
    
  while (size_left > 0)
    {
      int copy_len = ndnfs::seg_size;
      if (copy_len > size_left)
	copy_len = size_left;

      make_segment (path, ver, seg_off++, buf_pos, copy_len);
      buf_pos += copy_len;
      size_left -= copy_len;
    }

out:
  int total_seg = seg_off > old_total_seg ? seg_off : old_total_seg;
  
  update_fbi (path, ver, total_seg - 1);

  int ver_size = (int) (offset + size);
  // The new total size should be the maximum of old size and (size + offset)
  ver_size = ver_size > old_ver_size ? ver_size : old_ver_size;

  // Update temp version entry
  sqlite3_finalize (stmt);
  sqlite3_prepare_v2 (db, "UPDATE file_versions SET size = ?, totalSegments = ? WHERE path = ? AND version = ?;", -1, &stmt, 0);
  sqlite3_bind_int (stmt, 1, ver_size);
  sqlite3_bind_int (stmt, 2, total_seg);
  sqlite3_bind_text (stmt, 3, path, -1, SQLITE_STATIC);
  sqlite3_bind_int (stmt, 4, ver);
  int res = sqlite3_step (stmt);
  sqlite3_finalize (stmt);
  if (res != SQLITE_OK && res != SQLITE_DONE)
    return -1;
  
  return ver_size;
}


int truncate_version(const char* path, const int ver, off_t length)
{
#ifdef NDNFS_DEBUG
  cout << "truncate_version: path=" << path << std::dec << ", ver=" << ver << ", length=" << length << endl;
#endif

  sqlite3_stmt *stmt;
  sqlite3_prepare_v2 (db, "SELECT * FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
  sqlite3_bind_text (stmt, 1, path, -1, SQLITE_STATIC);
  sqlite3_bind_int (stmt, 2, ver);

  if (sqlite3_step (stmt) != SQLITE_ROW)
    {
      // Should not happen
      sqlite3_finalize (stmt);
      return -1;
    }
  
  int size = sqlite3_column_int (stmt, 2);
  sqlite3_finalize (stmt);
  
  if ((size_t) length == size)
    {
      return 0;
    }
  else if ((size_t) length < size)
    {
      // Truncate to length
      int seg_end = seek_segment (length);

      sqlite3_prepare_v2 (db, "UPDATE file_versions SET size = ?, totalSegments = ? WHERE path = ? and version = ?;", -1, &stmt, 0);
      sqlite3_bind_int (stmt, 1, (int) length);
      sqlite3_bind_int (stmt, 2, seg_end);
      sqlite3_bind_text (stmt, 3, path, -1, SQLITE_STATIC);
      sqlite3_bind_int (stmt, 4, ver);
      int res = sqlite3_step (stmt);
      sqlite3_finalize (stmt);
      if (res != SQLITE_OK && res != SQLITE_DONE)
	return -1;

      if (length > 0)
	update_fbi (path, ver, seg_end);

      // Update version size and segment list
      int tail = length - segment_to_size (seg_end);
      truncate_segment (path, ver, seg_end, tail);
      remove_segments (path, ver, seg_end + 1);
      
      return 0;
    }
  else
    {
      // TODO: pad with zeros
      return -1;
    }
}


void remove_version(const char* path, const int ver)
{
#ifdef NDNFS_DEBUG
    cout << "remove_version: path=" << path << ", ver=" << std::dec << ver << endl;
#endif

    remove_segments(path, ver);
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "DELETE FROM file_versions WHERE path = ? and version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, ver);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}


void remove_versions(const char* path)
{
#ifdef NDNFS_DEBUG
    cout << "remove_versions: path=" << path << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT current_version, temp_version FROM file_system WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path, -1, SQLITE_STATIC);
    int res = sqlite3_step(stmt);
    if(res == SQLITE_ROW){
        int curr_ver = sqlite3_column_int(stmt, 0);
        if (curr_ver != -1) {
            remove_version(path, curr_ver);
        }

        int tmp_ver = sqlite3_column_int(stmt, 1);
        if (tmp_ver != -1) {
            remove_version(path, tmp_ver);
        }
    }
    sqlite3_finalize(stmt);
}
