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


#include <ndn.cxx/common.h>
#include <ndn.cxx/data.h>
#include <ndn.cxx/interest.h>
#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/wrapper/closure.h>
#include <ndn.cxx/security/keychain.h>
#include <ndn.cxx/security/identity/osx-privatekey-store.h>
#include <boost/bind.hpp>


#include <iostream>

using namespace std;
using namespace ndn;
using namespace boost;

Ptr<security::OSXPrivatekeyStore> privateStoragePtr = Ptr<security::OSXPrivatekeyStore>::Create();
Ptr<security::Keychain> keychain = Ptr<security::Keychain>(new security::Keychain(privateStoragePtr, "/Users/ndn/qiuhan/policy", "/tmp/encryption.db"));
Ptr<Wrapper> handler = Ptr<Wrapper>(new Wrapper(keychain));////////////////////////

void OnData(Ptr<Data> data);
void OnTimeout(Ptr<Closure> closure, Ptr<Interest> origInterest);

void OnData(Ptr<Data> data) {
    Blob & content = data->content();
    cout << "data: " << string((char*)content.buf(), content.size()) << endl;
}

void OnTimeout(Ptr<Closure> closure, Ptr<Interest> origInterest) {
    // re-express interest
    cout << "TIME OUT :(" << endl;
    handler->sendInterest(origInterest, closure);
}

void Usage() {
    fprintf(stderr, "usage: ./client [-n name][-i minsuffix][-a maxfuffix][-c childeselector]\n");
    exit(1);
}

void verifiedError(Ptr<Interest> interest)
{
    cout << "unverified" << endl;
}


int main (int argc, char **argv) {
    Ptr<Interest> interestPtr = Ptr<Interest>(new Interest());
    interestPtr->setScope(Interest::SCOPE_LOCAL_HOST);
    interestPtr->setAnswerOriginKind(0);

    const char* name = "";
    uint32_t min_suffix_comps = Interest::ncomps;
    uint32_t max_suffix_comps = Interest::ncomps;
    uint8_t child_selector = Interest::CHILD_DEFAULT;

    int opt;
    while ((opt = getopt(argc, argv, "n:i:a:c:")) != -1) {
        switch (opt) {
        case 'n': 
            name = optarg;
            cout << "main(): set name: " << name << endl;
            interestPtr->setName(ndn::Name(name));
            break;
        case 'i': 
            min_suffix_comps = atoi(optarg);
            cout << "main(): set min suffix components: " << min_suffix_comps << endl;
            interestPtr->setMinSuffixComponents(min_suffix_comps);
            break;
        case 'a': 
            max_suffix_comps = atoi(optarg);
            cout << "main(): set max suffix components: " << max_suffix_comps << endl;
            interestPtr->setMaxSuffixComponents(max_suffix_comps);
            break;
        case 'c': 
            child_selector = atoi(optarg);
            cout << "main(): set child selector: " << (uint32_t)child_selector << endl;
            interestPtr->setChildSelector(child_selector);
            break;
        default: 
            Usage(); 
            break;
        }
    }
    Ptr<Closure> closure = Ptr<Closure> (new Closure(boost::bind(OnData, _1),
                                                     boost::bind(OnTimeout, _1, _2),
                                                     boost::bind(verifiedError, _1),
                                                     Closure::UnverifiedDataCallback()
                                             )
        );
    handler->sendInterest(interestPtr, closure);
    cout << "Interest sent" << endl;

    sleep(3);


    return 0;
}
