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

#ifndef __SERVER_MODULE_H__
#define __SERVER_MODULE_H__

#include <string>

#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/common.h>
#include <ndn.cxx/fields/name.h>
#include <ndn.cxx/interest.h>

#include <sqlite3.h>

#define DB_ENTRY_TYPE_DIR	0
#define DB_ENTRY_TYPE_FIL	1
//#define DB_ENTRY_TYPE_VER	2
//#define DB_ENTRY_TYPE_SEG	3

extern const char *db_name;
extern sqlite3 *db;

// Global prefix for NDNFS
extern std::string global_prefix;


// callbalck on receiving incoming interest.
// respond proper content object and comsumes the interest. or simple ignore
// the interest if no content object found.
void OnInterest(ndn::Ptr<ndn::Interest> interest);

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
void ndnName2String(ndn::Name name, uint64_t &version, int &seg, string &path);

// ndn name selector. deriving NDNFS name that specifies a content object from
// the NDN name given in an interest. return the NDNFS name on success; NULL 
// on failure (no match found).
//const std::string NameSelector(ndn::Ptr<ndn::Interest> interest);

int ProcessName(Ptr<Interest> interest,uint64_t &version, int &seg, string &path);
// search mongo db specified by c from entry specified by cursor for 
// possible matches. whenever finding a possible match, check if it suffices
// the selectors.
/*const std::string 
Search4PossibleMatch_Rec(
		mongo::ScopedDbConnection* c, 
		mongo::BSONObj current_entry, 
		ndn::Ptr<ndn::Interest> interest);*/

// check if the directory/content object specified by cursor suffices 
// the min/max suffix components selector specified in interest. 
// note that if and only if cursor points to a segment entry can a match be 
// found. skip checking if cursor points to some other type entry.
//bool CheckSuffix(mongo::BSONObj current_entry, ndn::Ptr<ndn::Interest> interest);
bool CompareComponent(char* a, char* b);
// fetch raw data as binary from the segment specified by ndnfs_name
// number of bytes fetch stored in len
const char* FetchData(uint64_t version, int seg, string path  int& len);

// fetch data as string from the segment specified by ndnfs_name
std::string FetchData(uint64_t version, int seg, string path  int& len);
	
#endif // __SERVER_MODULE_H__
