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
#include <ndn.cxx/data.h>
#include <ndn.cxx/common.h>
#include <ndn.cxx/security/exception.h>

#include <boost/lexical_cast.hpp>

#include <ndn.cxx/helpers/uri.h>

#include <iostream>

using namespace std;
using namespace boost;
using namespace ndn;

int read_segment(const string& ver_path, const int seg, char *output, const int limit, const int offset)
{
#ifdef NDNFS_DEBUG
  cout << "read_segment: " << ver_path << ", " << seg << ", " << limit << ", " << offset << endl;
#endif

    string segment = lexical_cast<string> (seg);
    string seg_path = ver_path + "/" + segment;

    const char*co_raw;
    int co_size;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }

    co_raw = (const char*) sqlite3_column_blob(stmt,1);
    co_size = sqlite3_column_bytes(stmt,1);

    Ptr<Blob> data_Blob = Create<Blob>(co_raw,co_size);////
    Ptr<const Blob> data = data_Blob;////
    Blob & data_content = Data::decodeFromWire(data)->content();///////
    const char *content = (const char*)data_content.buf();//////
    for (int i = 0; i < co_size; i++) {
      cout << co_raw[i];
    }
    cout << endl;
    cout << co_size << endl;
    cout << content[0] << endl;

    int copy_len = data_content.size();///////
    cout << copy_len << endl;
    if (copy_len > limit)  // Don't write across the limit
	copy_len = limit;

    memcpy(output, content + offset, copy_len);/////
    
    sqlite3_finalize(stmt);
    
    return copy_len;
}


int make_segment(const string& file_path, const uint64_t ver, const int seg, const bool final, const char *data, const int len)
{
#ifdef NDNFS_DEBUG
    cout << "make_segment: " << file_path << ", " << ver << ", " << seg <<", " << endl;
#endif

    assert(len > 0);

    string version = lexical_cast<string> (ver);
    string segment = lexical_cast<string> (seg);
    string ver_path = file_path + "/" + version;
    string full_path = ver_path + "/" + segment;
    string full_name = ndnfs::global_prefix + file_path;
    string escaped_name;
    Uri::toEscaped(full_name.begin(), full_name.end(), back_inserter(escaped_name));

    Name seg_name(escaped_name);
    seg_name.appendVersion(ver);
    seg_name.appendSeqNum(seg);
#ifdef NDNFS_DEBUG
    cout << "seg_name is " << seg_name.toUri() << endl;
#endif

    Content co(data,len);
    Data data0;
    data0.setName(seg_name);
    data0.setContent(co);
    try{
      keychain->sign(data0,signer);////XXXXXXXXXXX
    }catch(security::SecException & e){
      cerr << e.Msg() << endl;
      cerr << data0.getName() << endl;
    }
    Ptr<Blob> wire_data = data0.encodeToWire();
    char* co_raw = wire_data->buf();
    int co_size = wire_data->size();

    for (int i = 0; i < co_size; i++) {
      cout << co_raw[i];
    }
    cout << endl;
    cout << co_size << endl;

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "INSERT INTO file_segments (path,data,offset) VALUES (?,?,?);", -1, &stmt, 0);
    sqlite3_bind_text(stmt,1,full_path.c_str(),-1,SQLITE_STATIC);
    sqlite3_bind_blob(stmt,2,co_raw,co_size,SQLITE_STATIC);
    sqlite3_bind_int(stmt,3,segment_to_size(seg));
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    return 0;
}

void remove_segments(const string& path, uint64_t version, const int start/* = 0 */)
{
#ifdef NDNFS_DEBUG
  cout << "remove_segments: " << path << ", " << version << ", " << start << endl;
#endif

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT totalSegments FROM file_versions WHERE path = ? AND version = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, version);
    int res = sqlite3_step(stmt);
    if (res != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return;
    }
    int segs = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    for (int i = start; i < segs; i++) {
        string seg_path = path + "/" + lexical_cast<string> (version) + "/" + lexical_cast<string> (i);
        sqlite3_prepare_v2(db, "DELETE FROM file_segments WHERE path = ?;", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
    }
}

void truncate_segment(const string& ver_path, const int seg, const off_t length)
{
#ifdef NDNFS_DEBUG
  cout << "truncate_segment: " << ver_path << ", " << seg << endl;
#endif

    string seg_path = ver_path + "/" + lexical_cast<string> (seg);
    
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ?;", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, seg_path.c_str(), -1, SQLITE_STATIC);
    if(sqlite3_step(stmt) == SQLITE_ROW) {
        if (length == 0) {
	  sqlite3_finalize(stmt);
	  sqlite3_prepare_v2(db, "UPDATE file_segments SET data = ? WHERE path = ?;", -1, &stmt, 0);
	  sqlite3_bind_blob(stmt, 1, NULL, -1, SQLITE_STATIC);
	  sqlite3_bind_text(stmt, 2, seg_path.c_str(), -1, SQLITE_STATIC);
	  sqlite3_step(stmt);
	  sqlite3_finalize(stmt);
	} else {
	  const char* co_raw = (const char *)sqlite3_column_blob(stmt,1);
	  int co_size = sqlite3_column_bytes(stmt,1);

	  assert(co_size > (int)length);

	  Ptr<Blob> data_Blob = Create<Blob>(co_raw,co_size);////
	  Ptr<const Blob> data_blob = data_Blob;////
	  Ptr<Data> data = Data::decodeFromWire(data_blob);////
	  Blob & data_content = data->content();////
	  const char *content = (const char*)data_content.buf();////

	  Content co(content,length);////
	  Data trunc_data;////
	  trunc_data.setName(data->getName());////
	  trunc_data.setContent(co);////
	  keychain->sign(trunc_data,signer);////XXXXXX
	  Ptr<Blob> wire_data = trunc_data.encodeToWire();////
	  char *trunc_co_raw = wire_data->buf();////
	  int trunc_co_size = wire_data->size();////

	  sqlite3_finalize(stmt);
	  sqlite3_prepare_v2(db, "UPDATE file_segments SET data = ? WHERE path = ?;", -1, &stmt, 0);
	  sqlite3_bind_blob(stmt, 1, trunc_co_raw, trunc_co_size, SQLITE_STATIC);
	  sqlite3_bind_text(stmt, 2, seg_path.c_str(), -1, SQLITE_STATIC);
	  sqlite3_step(stmt);
	  sqlite3_finalize(stmt);
	}
    }
}
