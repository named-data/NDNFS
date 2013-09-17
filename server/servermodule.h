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
void ndnName2String(const ndn::Name& name, uint64_t &version, int &seg, std::string &path);

// ndn name selector. deriving NDNFS name that specifies a content object from
// the NDN name given in an interest. return the NDNFS name on success; NULL 
// on failure (no match found).
int ProcessName(ndn::Ptr<ndn::Interest> interest, uint64_t &version, int &seg, std::string &path);

// search mongo db specified by c from entry specified by cursor for 
// possible matches. whenever finding a possible match, check if it suffices
// the selectors.
int MatchFile(std::string &path, uint64_t& version, int& seg);

// check if the directory/content object specified by cursor suffices 
// the min/max suffix components selector specified in interest. 
// note that if and only if cursor points to a segment entry can a match be 
// found. skip checking if cursor points to some other type entry.
bool CompareComponent(const std::string& a, const std::string& b);

bool CheckSuffix(ndn::Ptr<ndn::Interest> interest, std::string path);
#endif // __SERVER_MODULE_H__
