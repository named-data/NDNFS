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
 * Author: Qiuhan Ding <dingqiuhan@gmail.com>
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

using namespace std;
using namespace ndn;
using namespace boost;

Ptr<security::OSXPrivatekeyStore> privateStoragePtr = Ptr<security::OSXPrivatekeyStore>::Create();
Ptr<security::Keychain> keychain = Ptr<security::Keychain>(new security::Keychain(privateStoragePtr, "/Users/ndn/qiuhan/policy", "/tmp/encryption.db"));
Ptr<Wrapper> handler = Ptr<Wrapper>(new Wrapper(keychain));

void OnData(Ptr<Data> data);
void OnTimeout(Ptr<Closure> closure, Ptr<Interest> origInterest);

void OnData(Ptr<Data> data) {
    Blob& content = data->content();
    Name& data_name = data->getName();
    name::Component& comp = data_name.get(data_name.size() - 2);
    string marker = comp.toUri();
    if(marker == "%C1.FS.dir"){
        ndnfs::DirInfoArray infoa;
        if(infoa.ParseFromArray(content.buf(),content.size()) && infoa.IsInitialized()){
            cout << "This is a directory:" << endl;
            int n = infoa.di_size();
            for(int i = 0; i<n; i++){
                const ndnfs::DirInfo &info = infoa.di(i);
                cout << info.path();
                if(info.type() == 0)
                    cout <<":    DIR"<<endl;
                else
                    cout <<":    FILE"<<endl;
            }
        }
        else{
            cerr << "protobuf error" << endl;
        }
    }
    else if(marker == "%C1.FS.file"){
        ndnfs::FileInfo infof;
        if(infof.ParseFromArray(content.buf(),content.size()) && infof.IsInitialized()){
            cout << "This is a file" << endl;
            cout << "name:  " << data->getName().toUri() << endl;
            cout << "size:  " << infof.size() << endl;
            cout << "version:   " << infof.version() << endl;
            cout << "total segments: " << infof.totalseg() << endl;
        }
        else{
            cerr << "protobuf error" << endl;
        }
    }
    else
        cout << "data: " << string((char*)content.buf(), content.size()) << endl;
}

void OnTimeout(Ptr<Closure> closure, Ptr<Interest> origInterest) {
    // re-express interest
    cout << "TIME OUT :(" << endl;
    handler->sendInterest(origInterest, closure);
}

void Usage() {
    fprintf(stderr, "usage: ./client [-n name]\n");
    exit(1);
}

void verifiedError(Ptr<Interest> interest)
{
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
            interestPtr->setName(ndn::Name(name));
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
