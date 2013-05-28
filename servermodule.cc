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

// #define DEBUG

#include <iostream>
#include <string>
#include <vector>

#include <ndn.cxx.h>
#include <mongo/client/dbclient.h>
#include <boost/lexical_cast.hpp>

#include "servermodule.h"

using namespace std;

extern ndn::Wrapper handler;
extern const char* db_name;
extern mongo::ScopedDbConnection* c;
extern bool child_selector_set;

// callbalck on receiving incoming interest.
// respond proper content object and comsumes the interest. or simple ignore
// the interest if no content object found.
void OnInterest(ndn::InterestPtr interest) {
	// 1.convert ndn name requested by interest to specific content object 
	// name that can be fetched in NDNFS. this may be done by following the
	// selector rules.
	// 2.call NDNFS API to fetch content object
	// 3.publish content object and consume the interest
	static int interest_cnt = 0;
	cout << interest_cnt++ << "------------------------------------------" << endl;
	cout << "OnInterest(): interest name: " << interest->getName() << endl;

	const string ndnfs_name = NameSelector(interest);

	if (ndnfs_name.empty()) {
		cout << "OnInterest(): no match found for prefix: " << interest->getName() << endl;
	}
	else {
		cout << "OnInterest(): a match has been found for prefix: " << interest->getName() << endl;
		cout << "OnInterest(): fetching content object ..." << endl;
		// fetch the content object from mongo db
		int len;
		/**********************************************
		 * use this part to fecth data as binary
		const char* data = FetchData(ndnfs_name, len);
		handler.publishData(interest->getName(), data, len);
		***********************************************/
		// test example: string data
		string string_data = FetchStringData(ndnfs_name, len);
		cout << "OnInterest(): string data: " << string_data << endl;
		// TODO: seems that client receives nothing ...
		handler.publishData(interest->getName(), string_data.c_str(), len);
		cout << "OnInterest(): content object returned and interest consumed" << endl;
	}
	cout << "OnInterest(): Done" << endl;
	cout << "------------------------------------------------------------" << endl;
}

// ndn-ndnfs name converter. converting name from ndn::Name representation to
// string representation.
const string ndnName2String(ndn::Name name) {
	string str_name("");
	string slash("/");

	ndn::Name::const_iterator iter = name.begin();
	for (; iter != name.end(); iter++) {
		string comp = iter->toUri ();
		// cout << "ndnName2String(): interest name component: " << comp << endl;
		if (comp[0] == '%') {
			ostringstream os;
			os << iter->toNumber ();
			comp = os.str();
		}
		str_name += (slash + comp);
	}
	// cout << "ndnName2String(): interest name: " << str_name << endl;

	return str_name;
}

// ndn name selector. deriving NDNFS name that specifies a content object from
// the NDN name given in an interest. return the NDNFS name on success; NULL 
// oin failure (no match found).
// this function looks up the underlying NDNFS directly searching for proper 
// content object name that matches what the interest requires.
const string NameSelector(ndn::InterestPtr interest) {
	string ndn_name("");
	ndn_name = ndnName2String(interest->getName());

	string ndnfs_name("");
	// derive ndnfs name from above string style ndn_name
	// connect to db and mount the directory represented by ndn_name
	auto_ptr<mongo::DBClientCursor> cursor = 
		c->conn().query(db_name, QUERY("_id" << ndn_name));
	if (!cursor->more()) {
		// query failed, no entry found
#ifdef DEBUG
		cout << "NameSelector(): no such prefix/name found in ndnfs: " << ndn_name << endl;
#endif
		return string("");
	}

	mongo::BSONObj entry = cursor->next();
	// search for a match in db specified by c starting from entry 
	// specified by cursor.
#ifdef DEBUG
	cout << "NameSelector(): searching for: " << ndn_name << endl;
#endif
	child_selector_set = false;
	ndnfs_name = Search4PossibleMatch_Rec(c, entry, interest);

	if (ndnfs_name.empty()) {
#ifdef DEBUG
		cout << "NameSelector(): no match found for: " << ndn_name << endl;
#endif
	}
	return ndnfs_name;
}


struct BSONElementLessThan {
	inline bool operator()(const mongo::BSONElement a, const mongo::BSONElement b) {
		if (a.type() == mongo::NumberInt)
			return (a.Int() < b.Int());
		else
			return (a.String() < b.String());
	}
};

// search mongo db specified by c from entry specified by cursor for 
// possible matches. whenever finding a possible match, check if it suffices
// the selectors.
const string Search4PossibleMatch_Rec(mongo::ScopedDbConnection* c, 
		mongo::BSONObj current_entry, 
		ndn::InterestPtr interest) {
	string original_name(current_entry.getStringField("_id"));
	// ASSERT: for each _id specified as absolute path, there is only 1 entry
	// check current entry. if it is a possible match, check for 
	// selectors also to see if it can be returned.
	// now check_selectors() can be called directly to check current entry.

#ifdef DEBUG
	cout << "Search4PossibleMatch_Rec(): now searching: " << original_name << endl;
#endif
	mongo::BSONObj next_entry;
	vector<mongo::BSONElement> subdir_list;
	// traverse this entire directory recursively
	assert(current_entry.hasField("type"));
	int type = current_entry.getIntField("type"); // error calling Int()
	int first_index = 0;
	int last_index = 0;
	int current_index;
	uint8_t child_selector;
	string ndnfs_name("");
	switch (type) {
		case DB_ENTRY_TYPE_DIR:
		case DB_ENTRY_TYPE_FIL:
		case DB_ENTRY_TYPE_VER:
			subdir_list = current_entry["data"].Array();
			// do sort here
			sort(subdir_list.begin(), subdir_list.end(), BSONElementLessThan());
			// childeselector check. use global variable child_selector_set to 
			// guarantee that the selector is applied to first level name after
			// prefix only
			// TODO: canonical ccnx ordering???
			first_index = 0;
			last_index = subdir_list.size();
			// TODO: the following function returns nothing and the child 
			// selector switch() always goes to CHILD_LEFT
			child_selector = interest->getChildSelector();
			if (!child_selector_set && child_selector) {
#ifdef DEBUG
				cout << "Search4PossibleMatch_Rec(): checking ChildSelector" << child_selector << endl;
#endif
				child_selector_set = true;
				switch (child_selector) {
					case ndn::Interest::CHILD_LEFT:	// CHILD_LEFT
						first_index = 0;
						last_index = 1;
						break;
					case ndn::Interest::CHILD_RIGHT:	// CHILD_RIGHT
						first_index = subdir_list.size() - 1;
						last_index = subdir_list.size();
						break;
					case ndn::Interest::CHILD_DEFAULT:	// CHILD_DEFAULT
						first_index = 0;
						last_index = subdir_list.size();
						break;
					default:
						cerr << "Search4PossibleMatch_Rec(): unidentified child selector" << endl;
						cerr << "Search4PossibleMatch_Rec(): use default settings" << endl;
						first_index = 0;
						last_index = subdir_list.size();
				}
			}
			for (current_index = first_index; 
				 current_index < last_index; 
				 current_index++) {
				string current_name(original_name);
				if (current_name[current_name.length()-1] == '/') {
					// if searching under root directory, construct next level name
					// by simply appending
					if (type == DB_ENTRY_TYPE_DIR)
						current_name += subdir_list[current_index].String();
					else 
						current_name += 
							boost::lexical_cast<string>(subdir_list[current_index].Int());
				}
				else {
					// if searching under other directory, construct next level
					// name by appending separator (/) and subdir/file name
					current_name += "/";
					if (type == DB_ENTRY_TYPE_DIR)
						current_name += subdir_list[current_index].String();
					else {
						current_name += 
							boost::lexical_cast<string>(subdir_list[current_index].Int());
					}
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
			if (CheckSuffix(current_entry, interest)) {
				ndnfs_name = current_entry.getStringField("_id");
#ifdef DEBUG
				cout << "Search4PossibleMatch_Rec(): find a match: " << ndnfs_name << endl;
#endif
				return ndnfs_name;
			}
			break;
		default: 
			cerr << "Search4PossibleMatch_Rec(): unidentified entry type: " << current_entry.getIntField("type") << endl;
	}
		
	return ndnfs_name;
}

// check if the directory/content object specified by cursor suffices 
// selectors specified by interest. note if and only if cursor points to 
// a segment entry can a match be found. skip checking if cursor points to 
// some other type entry.
bool CheckSuffix(mongo::BSONObj current_entry, ndn::InterestPtr interest) {
	assert(current_entry.hasField("type"));
#ifdef DEBUG
	cout << "CheckSuffix(): checking min/maxSuffixComponents" << endl;
#endif
	int entry_type = current_entry.getIntField("type");

	// we only check segment entries to see if it suffices the selectors
	// cout << "CheckSelectors(): checking assertion: is segment type" << endl;
	assert(entry_type == DB_ENTRY_TYPE_SEG);

	// min/max suffix components
	uint32_t min_suffix_components = interest->getMinSuffixComponents();
	uint32_t max_suffix_components = interest->getMaxSuffixComponents();
#ifdef DEBUG
	cout << "CheckSuffix(): MinSuffixComponents set to: " << min_suffix_components << endl;
	cout << "CheckSuffix(): MaxSuffixComponents set to: " << max_suffix_components << endl;
#endif

	// do suffix components check
	uint32_t prefix_len = interest->getName().size();
	string match = current_entry.getStringField("_id");
	uint32_t match_len = ndn::Name(match).size();
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

// fetch raw data as binary from the segment specified by ndnfs_name
// number of bytes fetch stored in len
const char* FetchData(string ndnfs_name, int& len) {
	auto_ptr<mongo::DBClientCursor> cursor = 
	c->conn().query(db_name, QUERY("_id" << ndnfs_name));
	if (!cursor->more()) {
		// query failed, no entry found
		cerr << "FetchData(): error locating data: " << ndnfs_name << endl;
		return NULL;
	}

	mongo::BSONObj entry = cursor->next();
	assert(entry.getIntField("type") == DB_ENTRY_TYPE_SEG);
	assert(entry.hasField("size"));
	if (entry.getIntField("size") == 0) {
		len = 0;
		return NULL;
	}
	else {
		len = entry.getIntField("size");
		return entry.getField("data").binData(len);
	}
}

// fetch data as string from the segment specified by ndnfs_name
string FetchStringData(string ndnfs_name, int& len) {
	auto_ptr<mongo::DBClientCursor> cursor = 
	c->conn().query(db_name, QUERY("_id" << ndnfs_name));
	if (!cursor->more()) {
		// query failed, no entry found
		cerr << "FetchStringData(): error locating data: " << ndnfs_name << endl;
		return NULL;
	}

	mongo::BSONObj entry = cursor->next();
	assert(entry.getIntField("type") == DB_ENTRY_TYPE_SEG);
	len = strlen(entry.getStringField("data")) + 1; // count tail NULL in
	return entry.getStringField("data");
}

// TODO: publisherPublicKeyDigest
// related implementation not available currently in lib ccnx-cpp
// TODO: exclude
// related implementation not available currently in lib ccnx-cpp


