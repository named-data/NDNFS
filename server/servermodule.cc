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
#define DEBUG

#include <cstdio>
#include <iostream>
#include <fstream>
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
#ifdef DEBUG
    cout << "OnInterest(): extracted version=" << (int64_t)version << ", segment=" << seg << ", path=" << path << endl;
#endif
    if (res == -1) {
		cout << "OnInterest(): no match found for prefix: " << interest->getName() << endl;
    }
    else {
        bool suffix = CheckSuffix(interest, path);
        if(suffix == true){
            cout << "OnInterest(): a match has been found for prefix: " << interest->getName() << endl;
            cout << "OnInterest(): fetching content object from database" << endl;

            int len = -1;
            const char* data = NULL;
            sqlite3_stmt *stmt;
            sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 2, version);
            sqlite3_bind_int(stmt, 3, seg);
            if(sqlite3_step(stmt) == SQLITE_ROW){
                const char * data = (const char *)sqlite3_column_blob(stmt, 3);
                len = sqlite3_column_bytes(stmt, 3);
#ifdef DEBUG
                cout << "OnInterest(): blob length=" << len << endl;
                cout << "OnInterest(): blob data is " << endl;
                ofstream ofs("/tmp/blob", ios_base::binary);
                for (int i = 0; i < len; i++) {
                    printf("%02x", (unsigned char)data[i]);
                    ofs << data[i];
                }
                cout << endl;
#endif
                ndn::Blob bin_data(data, len);
                handler->putToCcnd(bin_data);
                cout << "OnInterest(): content object returned and interest consumed" << endl;
            }
            else {
            // query failed, no entry found
                cerr << "OnInterest(): error locating data: " << path << endl;
            }
            sqlite3_finalize(stmt);
	}
    }
    cout << "OnInterest(): Done" << endl;
    cout << "------------------------------------------------------------" << endl;
}

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
void ndnName2String(const ndn::Name& name, uint64_t &version, int &seg, string &path) {
    path = "";
    string slash("/");
    version = -1;
    seg = -1;
    ndn::Name::const_iterator iter = name.begin();
    for (; iter != name.end(); iter++) {
#ifdef DEBUG
        cout << "ndnName2String(): interest name component: " << iter->toUri() << endl;
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
			string comp = iter->toUri();
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


int ProcessName(Ptr<Interest> interest, uint64_t &version, int &seg, string &path){
	ndnName2String(interest->getName(), version, seg, path);
#ifdef DEBUG
    cout << "ProcessName(): version=" << (int64_t)version << ", segment=" << seg << ", path=" << path << endl;
#endif
	child_selector = interest->getChildSelector();
	child_selector_set = false;
	if(version != -1 && seg != -1){
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 2, version);
		sqlite3_bind_int(stmt, 3, seg);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
#ifdef DEBUG
			cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
#endif
			sqlite3_finalize(stmt);
			return -1;
		}
		sqlite3_finalize(stmt);
		return 1;
	}
	else if(version != -1){
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
		sqlite3_bind_int64(stmt, 2, version);
		sqlite3_bind_int(stmt,3, 0);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
#ifdef DEBUG
			cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
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
		sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
		if(sqlite3_step(stmt)!= SQLITE_ROW){
#ifdef DEBUG
			cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
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
			version = sqlite3_column_int64(stmt, 7);
			seg = 0;
			sqlite3_finalize(stmt);
			return 1;
		}
		sqlite3_finalize(stmt);
#ifdef DEBUG
        cout << "ProcessName(): recursively find file: " << path << endl;
#endif
		int res = MatchFile(path, version, seg, child_selector);
		return res;
	}
}

bool CompareComponent(const string& a, const string& b){
    ndn::Name path1(a);
    ndn::Name path2(b);
	int len1 = path1.size();
	int len2 = path2.size();
    ndn::name::Component& comp1 = path1.get(len1 - 1);
    ndn::name::Component& comp2 = path2.get(len2 - 1);
	return comp1<comp2;
}

int MatchFile(string &path, uint64_t& version, int& seg, uint8_t child_selector){
	//finding the relevant file recursively
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE parent = ?", -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
	vector<string> paths;
	Name path_t;
	while(sqlite3_step(stmt) == SQLITE_ROW){
		paths.push_back(string((const char *)sqlite3_column_text(stmt, 0)));
	}
	if(paths.size()!=0){
		sort(paths.begin(), paths.end(), CompareComponent);
		//can add selector here
		if(!child_selector_set && child_selector){
			if(child_selector == Interest::CHILD_RIGHT)
				path = paths[paths.size()-1];
			else
				path = paths[0];
			child_selector_set = false;
		}
		sqlite3_finalize(stmt);
		sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?", -1, &stmt, 0);
		sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
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

bool CheckSuffix(Ptr<Interest> interest, string path) {
#ifdef DEBUG
    cout << "CheckSuffix(): checking min/maxSuffixComponents" << endl;
#endif
    // min/max suffix components
    uint32_t min_suffix_components = interest->getMinSuffixComponents();
    uint32_t max_suffix_components = interest->getMaxSuffixComponents();
#ifdef DEBUG
    cout << "CheckSuffix(): MinSuffixComponents set to: " << min_suffix_components << endl;
    cout << "CheckSuffix(): MaxSuffixComponents set to: " << max_suffix_components << endl;
#endif

    // do suffix components check
    uint32_t prefix_len = interest->getName().size();
    string match = global_prefix + path;
    uint32_t match_len = ndn::Name(match).size() + 2;
    // digest considered one component implicitly
    uint32_t suffix_len = match_len - prefix_len + 1;
    if (max_suffix_components != ndn::Interest::ncomps &&
	suffix_len > max_suffix_components) {
#ifdef DEBUG
	cout << "CheckSuffix(): max suffix mismatch" << endl;
#endif
	return false;
    }
    if (min_suffix_components != ndn::Interest::ncomps &&
	suffix_len < min_suffix_components) {
#ifdef DEBUG
	cout << "CheckSuffix(): min suffix mismatch" << endl;
#endif
	return false;
    }

    return true;
}

// TODO: publisherPublicKeyDigest
// related implementation not available currently in lib ccnx-cpp
// TODO: exclude
// related implementation not available currently in lib ccnx-cpp


