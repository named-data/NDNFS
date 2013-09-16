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
 * Author: Zhe Wen <wenzhe@cs.ucla.edu>
 */
//#define DEBUG

#include <iostream>
#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "servermodule.h"
#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/common.h>

using namespace std;
using namespace ndn;
using namespace boost;

extern Ptr<Wrapper> handler;
extern bool child_selector_set;

// callbalck on receiving incoming interest.
// respond proper content object and comsumes the interest. or simple ignore
// the interest if no content object found.
void OnInterest(Ptr<Interest> interest) {
    static int interest_cnt = 0;
#ifdef DEBUG
    cout << interest_cnt++ << "------------------------------------------" << endl;
    cout << "OnInterest(): interest name: " << interest->getName() << endl;
#endif
	string path;
	uint64_t version;
	int seg;
	int res = ProcessName(interest, version, seg, path);
    if (res == -1) {
		cout << "OnInterest(): no match found for prefix: " << interest->getName() << endl;
    }
    else {
      //#ifdef DEBUG
		cout << "OnInterest(): a match has been found for prefix: " << interest->getName() << endl;
		cout << "OnInterest(): fetching content object ..." << endl;

	int len;
	const char* data = FetchData(version,seg,path, len);
	ndn::Blob bin_data;
	for (int i = 0; i < len; i++) {
	    bin_data.push_back(data[i]);
	}
	handler->putToCcnd(bin_data);
	cout << "OnInterest(): content object returned and interest consumed" << endl;
    }
    cout << "OnInterest(): Done" << endl;
    cout << "------------------------------------------------------------" << endl;
}

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
void ndnName2String(ndn::Name name, uint64_t &version, int &seg, string &path) {
    path = "";
    string slash("/");
    version = -1;
    seg = -1;
    ndn::Name::const_iterator iter = name.begin();
    for (; iter != name.end(); iter++) {
#ifdef DEBUG
	cout << "ndnName2String(): interest name component: " << comp << endl;
#endif
		const uint8_t marker = *(iter->buf());
		// cout << (unsigned int)marker << endl;
		if (marker == 0xFD) {
			version = iter->toVersion(); 
		} 
		else if (marker == 0x00) {
			seg = iter->toSeqNum();
		}
		else {
			comp = iter->toUri();
			path += (slash + comp);
		}
	
    }
#ifdef DEBUG
    cout << "ndnName2String(): interest name: " << path << endl;
#endif
    path = path.substr(global_prefix.length());
    if (path == "")
		path = string("/");
#ifdef DEBUG
    cout << "ndnName2String(): file path after trimming: " << path << endl;
#endif
}


int ProcessName(Ptr<Interest> interest,uint64_t &version, int &seg, string &path){
	ndnName2String(interest->getName(),version,seg,path);
	if(version>0 && seg>0){
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1 &stmt,0);
		sqlite3_bind_text(stmt, 1, path.c_str(),SQLITE_STATIC);
		sqlite3_bind_int64(stmt,2,version);
		sqlite3_bind_int(stmt,3,seg);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
			#ifdef DEBUG
			cout << "ProcessName(): no such prefix/name found in ndnfs: " << path << endl;
			#endif
			sqlite3_finalize(stmt);
			return -1;
		}
		sqlite3_finalize(stmt);
		return 1;
	}
	else if(version>0){
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1 &stmt,0);
		sqlite3_bind_text(stmt, 1, path.c_str(),SQLITE_STATIC);
		sqlite3_bind_int64(stmt,2,version);
		sqlite3_bind_int(stmt,3,0);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
			#ifdef DEBUG
			cout << "ProcessName(): no such prefix/name found in ndnfs: " << ndn_name << endl;
			#endif
			sqlite3_finalize(stmt);
			return -1;
		}
		seg = 0;
		sqlite3_finalize(stmt);
		return 1;
	}
	else{
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?", -1 &stmt,0);
		sqlite3_bind_text(stmt, 1, path.c_str(),SQLITE_STATIC);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
			#ifdef DEBUG
			cout << "ProcessName(): no such prefix/name found in ndnfs: " << path << endl;
			#endif
			sqlite3_finalize(stmt);
			return -1;
		}
		//recursively finding path
		int type = sqlite3_column_int(stmt,2);
		if(type == 1){
			#ifdef DEBUG
			cout << "ProcessName(): find file: " << path << endl;
			#endif
			version = sqlite3_column_int64(stmt,7);
			seg = 0;
			sqlite3_finalize(stmt);
			return 1;
		}
		sqlite3_finalize(stmt);
		#ifdef DEBUG
			cout << "ProcessName(): recursively find file: " << path << endl;
		#endif
		int res = MatchFile(path, version, seg);
		return res;
	}
}

bool CompareComponent(char* a, char* b){
	Name path1 = Name(string(a));
	Name path2 = Name(string(b));
	len1 = path1.size();
	len2 = path2.size();
	comp1 = path1.get(len1);
	comp2 = path2.get(len2);
	return comp1<comp2;}

int MatchFile(string &path, uint64_t& version, int& seg){
	//finding the relevant file recursively
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE parent = ?", -1 &stmt,0);
	sqlite3_bind_text(stmt, 1, path.c_str(), SQLITE_STATIC);
	vector <char*> paths;
	Name path_t;
	while(sqlite3_step(stmt) == SQLITE_ROW){
		paths.push_back(sqlite3_column(stmt, 0));
	}
	if(paths.capacity()!=0){
		sort(paths.begin(), paths.end(), CompareComponent);
		//can add selector here
		path = paths[0];
		sqlite3_finalize(stmt);
		sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?", -1 &stmt,0);
		sqlite3_bind_text(stmt, 1, path.c_str(), SQLITE_STATIC);
		if(sqlite3_step(stmt) == SQLITE_ROW){
			int type = sqlite3_column_int(stmt,2);
			if (type == 1){
				#ifdef DEBUG
					cout << "MatchFile(): find file: " << path << endl;
				#endif
				version = sqlite3_column_int64(stmt,7);
				seg = 0;
				sqlite3_finalize(stmt);
				return 1;
			}
			else
				sqlite3_finalize(stmt);
				return MatchFile(path, version, seg);
		}
		else
			#ifdef DEBUG
			cout << "MatchFile(): no such prefix/name found in ndnfs: " << path << endl;
			#endif
			return -1;
	}
	else
		#ifdef DEBUG
			cout << "MatchFile(): no such file found in path: " << path << endl;
		#endif
		return -1;
}


// fetch raw data as binary from the segment specified by ndnfs_name
// number of bytes fetch stored in len
const char* FetchData(uint64_t version, int seg, string path  int& len) {
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int64(stmt, 2, version);
	sqlite3_bind_int(stmt, 3, seg);
	if(sqlite3_step(stmt) == SQLITE_ROW){
		char * data = (char *)sqlite3_column_blob(stmt,3)
		len = strlen(data) + 1;
		sqlite3_finalize(stmt);
		return data;
	}
	else {
	// query failed, no entry found
		cerr << "FetchData(): error locating data: " << path << endl;
		sqlite3_finalize(stmt);
		return NULL;
    }
}

// TODO: publisherPublicKeyDigest
// related implementation not available currently in lib ccnx-cpp
// TODO: exclude
// related implementation not available currently in lib ccnx-cpp


