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

#include <ndn.cxx.h>
#include <servermodule.h>
#include <iostream>

using namespace std;

const char* db_name = "ndnfs.root";
mongo::ScopedDbConnection* c;

// create a global handler
ndn::Wrapper handler;

int main(int argc, char **argv) {
	c = mongo::ScopedDbConnection::getScopedDbConnection("localhost");
	if (c->ok())
		cout << "main(): connected to local mongo db" << endl;
	else {
		cerr << "main(): cannot connect to local mongo db" << endl;
		c->done();
		delete c;
		exit(EXIT_FAILURE);
	}
	
	ndn::Name InterestBaseName = ndn::Name("/ucla.edu/cs");
	handler.setInterestFilter(InterestBaseName, OnInterest);
	while (true) {
		sleep (1);
	}
	
	cout << "main(): ServerModule exiting ..." << endl;
	c->done();
	delete c;

	return 0;
}

