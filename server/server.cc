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

#include <iostream>

#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/fields/name.h>
#include <ndn.cxx/common.h>

#include "servermodule.h"

using namespace std;
using namespace ndn;

const char* db_name = "ndnfs.root";
mongo::ScopedDbConnection* c;
bool child_selector_set;

// create a global handler
Ptr<Wrapper> handler = Ptr<Wrapper>(new Wrapper());

string global_prefix;


void
publishAllCert(Ptr<Wrapper> wrapper)
{
  sqlite3 * fakeDB;
  int res = sqlite3_open("./fake-data.db", &fakeDB);
  
  sqlite3_stmt *stmt;
  sqlite3_prepare_v2 (fakeDB, "SELECT data_blob FROM data", -1, &stmt, 0);

  while(sqlite3_step(stmt) == SQLITE_ROW)
    {
      Blob dataBlob(sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));    
      wrapper->putToCcnd(dataBlob);
    }

  sqlite3_close (fakeDB);
}

int main(int argc, char **argv) {
    const char* prefix = "/";
    int opt;

    while ((opt = getopt(argc, argv, "p:d:")) != -1) {
	switch (opt) {
	case 'p':
	    prefix = optarg;
	    break;
	case 'd':
	    db_name = optarg;
	    break;
	default:
	    break;
	}
    }
    
    c = mongo::ScopedDbConnection::getScopedDbConnection("localhost");
    if (c->ok())
	cout << "main(): connected to local mongo db" << endl;
    else {
	cerr << "main(): cannot connect to local mongo db" << endl;
	c->done();
	delete c;
	exit(EXIT_FAILURE);
    }

    cout << "serving prefix: " << prefix << endl;
    Name InterestBaseName = Name(prefix);
    global_prefix = InterestBaseName.toUri();
    cout << "global prefix for NDNFS: " << global_prefix << endl;

    handler->setInterestFilter(InterestBaseName, OnInterest);
    while (true) {
	sleep (1);
    }
	
    cout << "main(): ServerModule exiting ..." << endl;
    c->done();
    delete c;

    return 0;
}

