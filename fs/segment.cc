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
using namespace mongo;
using namespace ndn;

int read_segment(const string& ver_path, ScopedDbConnection *c, const int seg, char *output, const int limit, const int offset)
{
    string segment = lexical_cast<string> (seg);
    string seg_path = ver_path + "/" + segment;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
    if (!cursor->more()) {
	return -1;
    }
    
    BSONObj seg_entry = cursor->next();
    if (seg_entry.getIntField("type") != ndnfs::segment_type) {
	return -1;
    }

    int co_size;
    const char *co_raw = get_segment_data_raw(seg_entry, co_size);
    if (co_size == 0) {
	// This should not happen usually
	return -1;
    }
    
    Ptr<Blob> data_Blob = Create<Blob>(co_raw,co_size);////
    Ptr<const Blob> data = data_Blob;////
    Blob & data_content = Data::decodeFromWire(data)->content();///////
    const char *content = (const char*)data_content.buf();//////
    
    //////ndn::ParsedContentObject pco((const unsigned char *)co_raw, co_size);//////
    //////ndn::BytesPtr co_content = pco.contentPtr();    //////
    //////const char *content = (const char *)ndn::head(*co_content);//////

    int copy_len = data_content.size();///////
    if (copy_len > limit)  // Don't write across the limit
	copy_len = limit;

    memcpy(output, content + offset, copy_len);/////
    
    return copy_len;
}


int make_segment(const string& file_path, ScopedDbConnection *c, const uint64_t ver, const int seg, const bool final, const char *data, const int len)
{
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
    cout << "make_segment: seg_name is " << seg_name.toUri() << endl;
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
    //ndn::Bytes co = ndn_wrapper.createContentObject(seg_name, data, len);///////////////////////
    //unsigned char *co_raw = ndn::head(co);////////////////////////////////
    //int co_size = co.size();/////////////////////////

    BSONObj seg_entry = BSONObjBuilder().append("_id", full_path).append("type", ndnfs::segment_type)
	.appendBinData("data", co_size, BinDataGeneral, co_raw).append("offset", segment_to_size(seg)).obj();

    // Add segment entry to database
    c->conn().insert(db_name, seg_entry);

    return 0;
}

void remove_segments(const string& ver_path, ScopedDbConnection *c, const int start/* = 0 */)
{
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << ver_path));
    if (!cursor->more()) {
        return;
    }

    BSONObj ver_entry = cursor->next();
    
    vector< BSONElement > segs = ver_entry["data"].Array();
    for (int i = start; i < segs.size(); i++) {
        string seg_path = ver_path + "/" + lexical_cast<string> (segs[i].Int());
	c->conn().remove(db_name, QUERY("_id" << seg_path));
    }
}

void truncate_segment(const string& ver_path, ScopedDbConnection *c, const int seg, const off_t length)
{
    string seg_path = ver_path + "/" + lexical_cast<string> (seg);
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << seg_path));
    if (!cursor->more()) {
        return;
    }

    if (length == 0) {
	BSONObj bin_data = BSONObjBuilder().appendBinData("data", 0, BinDataGeneral, NULL).obj();
	c->conn().update(db_name, BSON("_id" << seg_path), BSON( "$set" << BSON( "data" << bin_data ) ));
    } else {
	BSONObj seg_entry = cursor->next();

	int co_size;
	const char *co_raw = get_segment_data_raw(seg_entry, co_size);
	assert(co_size > (int)length);

    Ptr<Blob> data_Blob = Create<Blob>(co_raw,co_size);////
    Ptr<const Blob> data_blob = data_Blob;////
    Ptr<Data> data = Data::decodeFromWire(data_blob);////
    Blob & data_content = data->content();////
    const char *content = (const char*)data_content.buf();////
	////ndn::ParsedContentObject pco((const unsigned char *)co_raw, co_size);
	////ndn::BytesPtr co_content = pco.contentPtr();    
	////const char *content = (const char *)ndn::head(*co_content);
	Content co(content,length);////
    Data trunc_data;////
    trunc_data.setName(data->getName());////
    trunc_data.setContent(co);////
    keychain->sign(trunc_data,signer);////XXXXXX
    Ptr<Blob> wire_data = trunc_data.encodeToWire();////
    char *trunc_co_raw = wire_data->buf();////
    int trunc_co_size = wire_data->size();////
	////ndn::Bytes trunc_co = ndn_wrapper.createContentObject(pco.name(), content, length);
	////unsigned char *trunc_co_raw = ndn::head(trunc_co);
	////int trunc_co_size = trunc_co.size();

	BSONObj bin_data = BSONObjBuilder().appendBinData("data", trunc_co_size, BinDataGeneral, trunc_co_raw).obj();
	c->conn().update(db_name, BSON("_id" << seg_path), BSON( "$set" << BSON( "data" << bin_data ) ));
    }
}
