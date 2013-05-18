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

#include <ndn.cxx.h>
#include <mongo/client/dbclient.h>

#define DB_ENTRY_TYPE_DIR	0
#define DB_ENTRY_TYPE_FIL	1
#define DB_ENTRY_TYPE_VER	2
#define DB_ENTRY_TYPE_SEG	3

using namespace std;

// callbalck on receiving incoming interest.
// respond proper content object and comsumes the interest. or simple ignore
// the interest if no content object found.
void OnInterest(ndn::InterestPtr interest);

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
const string ndnName2string(ndn::Name name);

// ndn name selector. deriving NDNFS name that specifies a content object from
// the NDN name given in an interest. return the NDNFS name on success; NULL 
// on failure (no match found).
const string NameSelector(ndn::InterestPtr interest);

// search mongo db specified by c from entry specified by cursor for 
// possible matches. whenever finding a possible match, check if it suffices
// the selectors.
const string 
Search4PossibleMatch_Rec(
		mongo::ScopedDbConnection* c, 
		mongo::BSONObj current_entry, 
		ndn::InterestPtr interest);

// check if the directory/content object specified by cursor suffices 
// selectors specified by interest. note if and only if cursor points to 
// a segment entry can a match be found. skip checking if cursor points to 
// some other type entry.
bool CheckSelectors(mongo::BSONObj current_entry, ndn::InterestPtr interest);


#endif // __SERVER_MODULE_H__
