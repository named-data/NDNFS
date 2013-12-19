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
 * Author: Qiuhan Ding <dingqiuhan@gmail.com>, Wentao Shang <wentao@cs.ucla.edu>
 */

//#define NDNFS_DEBUG

#include <cstdio>
#include <iostream>
//#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "servermodule.h"
#include <ndn-cpp/face.hpp>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/common.hpp>

using namespace std;
using namespace ndn;

extern ptr_lib::shared_ptr<Face> handler;

void OnInterest(const ptr_lib::shared_ptr<const Name>& prefix, const ptr_lib::shared_ptr<const Interest>& interest, Transport& transport, uint64_t registeredPrefixId) {
#ifdef NDNFS_DEBUG
    cout << "------------------------------------------------------------" << endl;
    cout << "OnInterest(): interest name: " << interest->getName() << endl;
#endif
    ProcessName(interest->getName(), transport);
#ifdef NDNFS_DEBUG
    cout << "OnInterest(): Done" << endl;
    cout << "------------------------------------------------------------" << endl;
#endif
}

void OnRegisterFailed(const ptr_lib::shared_ptr<const Name>& prefix) {
  
}

void ndnName2String(const ndn::Name& name, int &version, int &seg, string &path) {
    version = -1;
    seg = -1;
    ostringstream oss;
    ndn::Name::const_iterator iter = name.begin();
    for (; iter != name.end(); iter++) {
#ifdef NDNFS_DEBUG
        cout << "ndnName2String(): interest name component: " << iter->toEscapedString() << endl;
#endif
        const uint8_t marker = *(iter->getValue().buf());
        //cout << (unsigned int)marker << endl;
        if (marker == 0xFD) {
            version = iter->toVersion(); 
        }
        else if (marker == 0x00) {
            seg = iter->toSegment();
        }
        else if (marker == 0xC1) {
            continue;
        }
        else {
            string comp = iter->toEscapedString();
            oss << "/" << comp;
        }
    }
    path = oss.str();
#ifdef NDNFS_DEBUG
    cout << "ndnName2String(): full path: " << path << endl;
#endif
    path = path.substr(global_prefix.length());
    if (path == "")
        path = string("/");
#ifdef NDNFS_DEBUG
    cout << "ndnName2String(): file path after removing global prefix: " << path << endl;
#endif
}

void ProcessName(const Name& interest_name, Transport& transport) {
    string path;
    int version;
    int seg;
    ndnName2String(interest_name, version, seg, path);
#ifdef NDNFS_DEBUG
    cout << "ProcessName(): version=" << version << ", segment=" << seg << ", path=" << path << endl;
#endif
    if(version != -1 && seg != -1){
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT * FROM file_segments WHERE path = ? AND version = ? AND segment = ?", -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, version);
        sqlite3_bind_int(stmt, 3, seg);
        if(sqlite3_step(stmt) != SQLITE_ROW){
#ifdef NDNFS_DEBUG
            cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
#endif
            sqlite3_finalize(stmt);
            return;
        }
#ifdef NDNFS_DEBUG
        cout << "ProcessName(): a match has been found for prefix: " << interest_name << endl;
        cout << "ProcessName(): fetching content object from database" << endl;
#endif       
        const char * data = (const char *)sqlite3_column_blob(stmt, 3);
        int len = sqlite3_column_bytes(stmt, 3);
#ifdef NDNFS_DEBUG
        cout << "ProcessName(): blob length=" << len << endl;
        cout << "ProcessName(): blob data is " << endl;
        //ofstream ofs("/tmp/blob", ios_base::binary);
        for (int i = 0; i < len; i++) {
            printf("%02x", (unsigned char)data[i]);
            //ofs << data[i];
        }
        cout << endl;
#endif
        transport.send((uint8_t*)data, len);
#ifdef NDNFS_DEBUG
        cout << "ProcessName(): content object returned and interest consumed" << endl;
#endif
        sqlite3_finalize(stmt);
    }
    else if (version != -1 && seg == -1) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ? ", -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, version);
        if(sqlite3_step(stmt) != SQLITE_ROW){
#ifdef NDNFS_DEBUG
            cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
#endif
            sqlite3_finalize(stmt);
            return;
        }
		
        SendFile(path, version, sqlite3_column_int(stmt,2), sqlite3_column_int(stmt,3), transport);
        sqlite3_finalize(stmt);
    }
    else if (version == -1 && seg == -1) {
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE path = ?", -1, &stmt, 0);
        sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
        if(sqlite3_step(stmt) != SQLITE_ROW){
#ifdef NDNFS_DEBUG
            cout << "ProcessName(): no such file/directory found in ndnfs: " << path << endl;
#endif
            sqlite3_finalize(stmt);
            return;
        }
        
        int type = sqlite3_column_int(stmt,2);
        if(type == 1){
#ifdef NDNFS_DEBUG
            cout << "ProcessName(): found file: " << path << endl;
#endif
            version = sqlite3_column_int(stmt, 7);
            sqlite3_finalize(stmt);
            sqlite3_prepare_v2(db, "SELECT * FROM file_versions WHERE path = ? AND version = ? ", -1, &stmt, 0);
            sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 2, version);
            if(sqlite3_step(stmt) != SQLITE_ROW){
#ifdef NDNFS_DEBUG
                cout << "ProcessName(): no such file version found in ndnfs: " << path << endl;
#endif
                sqlite3_finalize(stmt);
                return;
            }
            
            SendFile(path, version, sqlite3_column_int(stmt,2), sqlite3_column_int(stmt,3), transport);
            sqlite3_finalize(stmt);
        } else {
#ifdef NDNFS_DEBUG
            cout << "ProcessName(): found dir: " << path << endl;
#endif
            int mtime = sqlite3_column_int(stmt, 5);
            sqlite3_finalize(stmt);
            SendDir(path, mtime, transport);
        }
    }
}

void SendFile(const string& path, int version, int sizef, int totalseg, Transport& transport) {
    ndnfs::FileInfo infof;
    infof.set_size(sizef);
    infof.set_totalseg(totalseg);
    infof.set_version(version);
    int size = infof.ByteSize();
    char *wireData = new char[size];
    infof.SerializeToArray(wireData, size);
    Name name(global_prefix + path);
    name.append("%C1.FS.file").appendVersion(version);
    Data data0;
    data0.setName(name);
    data0.setContent((uint8_t*)wireData, size);
    data0.getMetaInfo().setTimestampMilliseconds(time(NULL) * 1000.0);
    keychain->signByIdentity(data0, signer);
    transport.send(*data0.wireEncode());
    return;
}

void SendDir(const string& path, int mtime, Transport& transport) {
    //finding the relevant file recursively
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "SELECT * FROM file_system WHERE parent = ?", -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_STATIC);
    
    ndnfs::DirInfoArray infoa;
    int count = 0;
    while(sqlite3_step(stmt) == SQLITE_ROW){
        ndnfs::DirInfo *info = infoa.add_di();
        info->set_type(sqlite3_column_int(stmt, 2));
        info->set_path((const char *)sqlite3_column_text(stmt, 0));
        count++;
    }
    sqlite3_finalize(stmt);
    //return packet
    if(count!=0){
        int size = infoa.ByteSize();
        char *wireData = new char[size];
        infoa.SerializeToArray(wireData, size);
        Name name(global_prefix + path);
        name.append("%C1.FS.dir").appendVersion(mtime);
        Data data0;
        data0.setName(name);
        data0.setContent((uint8_t*)wireData, size);
        data0.getMetaInfo().setTimestampMilliseconds(time(NULL) * 1000.0);
        keychain->signByIdentity(data0, signer);
        transport.send(*data0.wireEncode());
        delete wireData;
    }
    else {
#ifdef NDNFS_DEBUG
        cout << "MatchFile(): no such file found in path: " << path << endl;
#endif
    }
    return;
}
