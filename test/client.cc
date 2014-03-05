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


#include <ndn-cpp/common.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/face.hpp>

#include "dir.pb.h"
#include "file.pb.h"

#include <iostream>
#include <getopt.h>
#include <unistd.h>

using namespace std;
using namespace ndn;
//using namespace boost;

Face handler("localhost");

void onData(const ptr_lib::shared_ptr<const Interest>&, const ptr_lib::shared_ptr<Data>&);
void onTimeout(const ptr_lib::shared_ptr<const Interest>&);

bool done = false;

void onData(const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data) {
    const Blob& content = data->getContent();
    const Name& data_name = data->getName();
    const Name::Component& comp = data_name.get(data_name.size() - 2);
    string marker = comp.toEscapedString();
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
    else {
        cout << "data: " << string((char*)content.buf(), content.size()) << endl;
        cout << "fbi: " << data->getMetaInfo().getFinalBlockID().toSegment() << endl;
    }

    done = true;
}

void onTimeout(const ptr_lib::shared_ptr<const Interest>& origInterest) {
    cout << "TIMEOUT!" << endl;
    //handler->expressInterest(*origInterest, onData, onTimeout);
    done = true;
}

void Usage() {
    fprintf(stderr, "usage: ./client [-n name]\n");
    exit(1);
}

int main (int argc, char **argv) {
    ptr_lib::shared_ptr<Interest> interestPtr(new Interest());
    interestPtr->setScope(ndn_Interest_ANSWER_CONTENT_STORE);
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

    handler.expressInterest(*interestPtr, onData, onTimeout);
    cout << "Interest sent" << endl;

    while (!done) {
        handler.processEvents();
        usleep (10000);
    }

    return 0;
}
