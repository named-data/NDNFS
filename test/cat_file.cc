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
 * Author: Wentao Shang <wentao@cs.ucla.edu>, Qiuhan Ding <dingqiuhan@gmail.com>
 */


#include <ndn.cxx/common.h>
#include <ndn.cxx/data.h>
#include <ndn.cxx/interest.h>
#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/wrapper/closure.h>
#include <ndn.cxx/security/keychain.h>
#include <ndn.cxx/security/identity/osx-privatekey-store.h>
#include <boost/bind.hpp>

#include "dir.pb.h"
#include "file.pb.h"

#include <iostream>
#include <fstream>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace ndn;
using namespace boost;

Ptr<security::OSXPrivatekeyStore> privateStoragePtr = Ptr<security::OSXPrivatekeyStore>::Create();
Ptr<security::Keychain> keychain = Ptr<security::Keychain>(new security::Keychain(privateStoragePtr, "/Users/ndn/qiuhan/policy", "/tmp/encryption.db"));
Ptr<Wrapper> handler = Ptr<Wrapper>(new Wrapper(keychain));

void OnMetaData(Ptr<Data> data);
void OnFileData(Ptr<Data> data);
void OnTimeout(Ptr<Closure> closure, Ptr<Interest> origInterest);
void verifiedError (Ptr<Interest> interest);

ndn::Name file_name;
int retrans = 0;
int total_size = 0;
int total_seg = 0;
int current_seg = 0;

ndn::Time start;

ofstream ofs("/tmp/file1", ios_base::binary | ios_base::trunc);

void OnMetaData (Ptr<Data> data) {
    Blob& content = data->content();
    Name& data_name = data->getName();
    name::Component& comp = data_name.get(data_name.size() - 2);
    string marker = comp.toUri();
    if (marker == "%C1.FS.dir") {
        cerr << "Requested name correspondes to a directory." << endl;
    } else if (marker == "%C1.FS.file") {
        ndnfs::FileInfo infof;
        if (infof.ParseFromArray(content.buf(), content.size()) && infof.IsInitialized()) {
            cout << "File metadata received." << endl;
            cout << "    name:  " << data_name.toUri() << endl;
            cout << "    size:  " << infof.size() << endl;
            cout << "    version number:   " << infof.version() << endl;
            cout << "    total segments: " << infof.totalseg() << endl;

            total_size = infof.size();
            total_seg = infof.totalseg();
            file_name = data_name.getPrefix(data_name.size() - 2);
            file_name.appendVersion((uint64_t)infof.version());
            cout << "File prefix with version is: " << file_name.toUri() << endl;

            cout << "Start to fetch file segments..." << endl;

            Ptr<Interest> interestPtr = Ptr<Interest>(new Interest());
            interestPtr->setScope(Interest::SCOPE_LOCAL_HOST);
            interestPtr->setName(Name(file_name).appendSeqNum((uint64_t)current_seg));
            //interestPtr->setAnswerOriginKind(0);

            Ptr<Closure> closure = Ptr<Closure> (
                new Closure(boost::bind(OnFileData, _1),
                            boost::bind(OnTimeout, _1, _2),
                            boost::bind(verifiedError, _1),
                            Closure::UnverifiedDataCallback()
                    )
                );

            start = ndn::time::Now();
            handler->sendInterest(interestPtr, closure);
        } else {
            cerr << "protobuf error" << endl;
        }
    } else
        cerr << "Unknown metadata marker: " << marker << endl;

    return;
}

void OnFileData (Ptr<Data> data) {
    Blob& content = data->content();
    Name& data_name = data->getName();
    cout << "Segment received: " << data_name.toUri() << endl;
    //cout << "    data: " << string((char*)content.buf(), content.size()) << endl;
    for (int i = 0; i < content.size(); i++) {
        ofs << content[i];
    }

    current_seg = (int)(data_name.rbegin()->toSeqNum());
    cout << current_seg << endl;
    current_seg ++;
    if (current_seg == total_seg) {
        ndn::Time stop = ndn::time::Now();
        cout << "Last segment received." << endl;
        cout << "Total run time: " << (stop - start) << endl;
        cout << "Total segment fetched: " << current_seg << endl;
        cout << "Total size fetched: " << total_size << endl;
        cout << "Total number of retrans: " << retrans << endl;
    } else {
        Ptr<Interest> interestPtr = Ptr<Interest>(new Interest());
        interestPtr->setScope(Interest::SCOPE_LOCAL_HOST);
        interestPtr->setName(Name(file_name).appendSeqNum((uint64_t)current_seg));
        //interestPtr->setAnswerOriginKind(0);    

        Ptr<Closure> closure = Ptr<Closure> (
            new Closure(boost::bind(OnFileData, _1),
                        boost::bind(OnTimeout, _1, _2),
                        boost::bind(verifiedError, _1),
                        Closure::UnverifiedDataCallback()
                )
            );
        
        handler->sendInterest(interestPtr, closure);
    }
    return;
}

void OnTimeout (Ptr<Closure> closure, Ptr<Interest> origInterest) {
    // re-express interest
    retrans++;
    cout << "Timeout on segment #" << current_seg << endl;
    if (retrans < 50)
        handler->sendInterest(origInterest, closure);
}

void Usage () {
	fprintf(stderr, "usage: ./client [-n name]\n");
	exit(1);
}

void verifiedError (Ptr<Interest> interest) {
    cout << "unverified" << endl;
}


int main (int argc, char **argv) {
    Ptr<Interest> interestPtr = Ptr<Interest>(new Interest());
    interestPtr->setScope(Interest::SCOPE_LOCAL_HOST);
	//interestPtr->setAnswerOriginKind(0);

	const char* name = NULL;

	int opt;
	while ((opt = getopt(argc, argv, "n:")) != -1) {
		switch (opt) {
			case 'n': 
				name = optarg;
				cout << "main(): set name: " << name << endl;
				interestPtr->setName(ndn::Name(name).append("%C1.FS.file"));
				break;
			default: 
				Usage(); 
				break;
		 }
	}

    Ptr<Closure> closure = Ptr<Closure> (
        new Closure(boost::bind(OnMetaData, _1),
                    boost::bind(OnTimeout, _1, _2),
                    boost::bind(verifiedError, _1),
                    Closure::UnverifiedDataCallback()
            )
        );
    handler->sendInterest(interestPtr, closure);
    
	start = ndn::time::Now();
	
	cout << "Started..." << endl;

	sleep(10);

    ofs.close();

	return 0;
}
