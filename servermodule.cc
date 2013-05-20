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

#define SERVERMODULE_INF	100

#include <iostream>
#include <string>
#include <vector>

#include <ndn.cxx.h>
#include <mongo/client/dbclient.h>
//#include <boost/function.hpp>
//#include <boost/bind.hpp>
//#include <boost/lexical_cast.hpp>

#include "servermodule.h"

using namespace std;

extern const char* db_name;
extern mongo::ScopedDbConnection* c;

// callbalck on receiving incoming interest.
// respond proper content object and comsumes the interest. or simple ignore
// the interest if no content object found.
void OnInterest(ndn::InterestPtr interest) {
	// 1.convert ndn name requested by interest to specific content object 
	// name that can be fetched in NDNFS. this may be done by following the
	// selector rules.
	// 2.call NDNFS API to fetch content object
	// 3.publish content object and consume the interest	
	cout << "OnInterest(): interest name: " << interest->getName() << endl;

	const string ndnfsName = NameSelector(interest);
}

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
const string ndnName2string(ndn::Name name) {
	string str_name("");
	string slash("/");

	ndn::Name::iterator iter = name.begin();
	for (; iter != name.end(); iter++) {
		cout << "ndnName2ndnfsName(): interest name component: " << ndn::Name::asString(*iter) << endl;
		str_name += (slash + ndn::Name::asString(*iter));
	}

	return str_name;
}

// ndn name selector. deriving NDNFS name that specifies a content object from
// the NDN name given in an interest. return the NDNFS name on success; NULL 
// oin failure (no match found).
// this function looks up the underlying NDNFS directly searching for proper 
// content object name that matches what the interest requires.
const string NameSelector(ndn::InterestPtr interest) {
	string ndn_name("");
	ndn_name = ndnName2string(interest->getName());

	string ndnfs_name("");
	// TODO: derive ndnfs name from above string style ndn_name
	// connect to db and mount the directory represented by ndn_name
	auto_ptr<mongo::DBClientCursor> cursor = 
		c->conn().query(db_name, QUERY("_id" << ndn_name));
	if (!cursor->more()) {
		// query failed, no entry found
		cout << "NameSelector(): no such prefix/name found in ndnfs: " << ndn_name << endl;
		return string("");
	}

	mongo::BSONObj entry = cursor->next();
	// search for a match in db specified by c starting from entry 
	// specified by cursor.
	cout << "NameSelector(): searching for: " << ndn_name << "..." << endl;
	ndnfs_name = Search4PossibleMatch_Rec(c, entry, interest);

	if (ndnfs_name.empty()) {
		cout << "NameSelector(): no match found for: " << ndn_name << endl;
	}
	return ndnfs_name;
}

// search mongo db specified by c from entry specified by cursor for 
// possible matches. whenever finding a possible match, check if it suffices
// the selectors.
const string Search4PossibleMatch_Rec(mongo::ScopedDbConnection* c, 
		mongo::BSONObj current_entry, 
		ndn::InterestPtr interest) {
	string ndnfs_name("");
	// ASSERT: for each _id specified as absolute path, there is only 1 entry
	// TODO: check current entry. if it is a possible match, check for 
	// selectors also to see if it can be returned.
//	boost::function<bool (mongo::BSONObj, ndn::InterestPtr)> check_selectors;
//	check_selectors = boost::bind(CheckSelectors, current_entry, interest);
	// now check_selectors() can be called directly to check current entry.

	cout << "Search4PossibleMatch_Rec(): now searching: " << current_entry["_id"].String() << endl;

	mongo::BSONObj next_entry;
	vector<mongo::BSONElement> subdir_list;
	vector<mongo::BSONElement>::iterator iter;
	// TODO: traverse this entire directory recursively
	switch (current_entry["type"].Int()) {
		case DB_ENTRY_TYPE_DIR:
		case DB_ENTRY_TYPE_FIL:
		case DB_ENTRY_TYPE_VER:
			subdir_list = current_entry["data"].Array();
			for (iter = subdir_list.begin(); iter != subdir_list.end(); iter++) {
				string current_name(ndnfs_name);
				if (current_entry["_id"].String() == "/") {
					// if searching under root directory, construct next level name
					// by simply appending
					current_name += (*iter).String();
				}
				else {
					// if searching under other directory, construct next level
					// name by appending separator (/) and subdir/file name
					current_name += ("/" + (*iter).String());
				}
				auto_ptr<mongo::DBClientCursor> current_cursor = 
					c->conn().query(db_name, QUERY("_id" << current_name));
				if (current_cursor->more()) {
					next_entry = current_cursor->next();
					ndnfs_name = 
						Search4PossibleMatch_Rec(c, next_entry, interest);
					if (!ndnfs_name.empty()) return ndnfs_name;
				}
			}
			break;
		case DB_ENTRY_TYPE_SEG:
			if (CheckSelectors(current_entry, interest)) {
				ndnfs_name = current_entry["_id"].String();
				cout << "Search4PossibleMatch_Rec(): found a match: " << ndnfs_name << endl;
				return ndnfs_name;
			}
			break;
		default: cerr << "Search4PossibleMatch_Rec(): unidentified entry type: " << current_entry["type"].Int() << endl;
	}
		
	// TODO: if any match is found, return its name (content object name)
	return ndnfs_name;
}

// check if the directory/content object specified by cursor suffices 
// selectors specified by interest. note if and only if cursor points to 
// a segment entry can a match be found. skip checking if cursor points to 
// some other type entry.
bool CheckSelectors(mongo::BSONObj current_entry, ndn::InterestPtr interest) {
	int entry_type = current_entry["type"].Int();

	// we only check segment entries to see if it suffices the selectors
	assert(entry_type == DB_ENTRY_TYPE_SEG);

	uint32_t min_suffix_components = ndn::Interest::ncomps;
	uint32_t max_suffix_components = ndn::Interest::ncomps;

	// minSuffixComponents
	if (interest->getMinSuffixComponents() != ndn::Interest::ncomps) {
		cout << "NameSelector(): MinSuffixComponents set to: " << min_suffix_components << endl;
		min_suffix_components = interest->getMinSuffixComponents();
	}
	else min_suffix_components = 0;
	// maxSuffixComponents
	if (interest->getMaxSuffixComponents() != ndn::Interest::ncomps) {
		cout << "NameSelector(): MaxSuffixComponents set to: " << max_suffix_components << endl;
	}
	else max_suffix_components = SERVERMODULE_INF;
	// TODO: publisherPublicKeyDigest
	// TODO: exclude
	
	return false; // REPLACE THIS
}
