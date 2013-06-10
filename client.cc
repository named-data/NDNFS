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
#include <iostream>

using namespace std;

ndn::Wrapper handler;

void OnData(ndn::Name name, ndn::PcoPtr pco);
void OnTimeout(ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest);

void OnData(ndn::Name name, ndn::PcoPtr pco) {
	ndn::BytesPtr content = pco->contentPtr();
	cout << "data: " << string((char*)ndn::head(*content), content->size()) << endl;
}

void OnTimeout(ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest) {
	// re-express interest
	cout << "TIME OUT :(" << endl;
	handler.sendInterest(*origInterest, closure);
}

void Usage() {
	fprintf(stderr, "usage: ./client [-n name][-i minsuffix][-a maxfuffix][-c childeselector]\n");
	exit(1);
}

int main (int argc, char **argv) {
	ndn::Interest interest = ndn::Interest();
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	interest.setAnswerOriginKind(0);

	const char* name = "";
	uint32_t min_suffix_comps = ndn::Interest::ncomps;
	uint32_t max_suffix_comps = ndn::Interest::ncomps;
	uint8_t child_selector = ndn::Interest::CHILD_DEFAULT;

	int opt;
	while ((opt = getopt(argc, argv, "n:i:a:c:")) != -1) {
		switch (opt) {
			case 'n': 
				name = optarg;
				cout << "main(): set name: " << name << endl;
				interest.setName(ndn::Name(name));
				break;
			case 'i': 
				min_suffix_comps = atoi(optarg);
				cout << "main(): set min suffix components: " << min_suffix_comps << endl;
				interest.setMinSuffixComponents(min_suffix_comps);
				break;
			case 'a': 
				max_suffix_comps = atoi(optarg);
				cout << "main(): set max suffix components: " << max_suffix_comps << endl;
				interest.setMaxSuffixComponents(max_suffix_comps);
				break;
			case 'c': 
				child_selector = atoi(optarg);
				cout << "main(): set child selector: " << (uint32_t)child_selector << endl;
				interest.setChildSelector(child_selector);
				break;
			default: 
				Usage(); 
				break;
		 }
	}

	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Interest sent" << endl;

	sleep(3);


	return 0;
}
