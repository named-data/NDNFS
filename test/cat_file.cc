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


#include <ndn-cpp/common.hpp>
#include <ndn-cpp/data.hpp>
#include <ndn-cpp/interest.hpp>
#include <ndn-cpp/face.hpp>

#include "dir.pb.h"
#include "file.pb.h"

#include <iostream>
//#include <fstream>
#include <boost/chrono.hpp>


using namespace std;
using namespace ndn;

using namespace boost::chrono;

Face handler("localhost");

ndn::Name file_name;
int total_size = 0;
int total_seg = 0;
int current_seg = 0;

bool done = false;

typedef high_resolution_clock::time_point stdtime;
typedef high_resolution_clock::duration stdduration;

stdtime start;

//ofstream ofs("/tmp/file1", ios_base::binary | ios_base::trunc);

void onFileData (const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data);
void onTimeout (const ptr_lib::shared_ptr<const Interest>& origInterest);

void onMetaData (const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data) {
    const Blob& content = data->getContent();
    const Name& data_name = data->getName();
    const Name::Component& comp = data_name.get(data_name.size() - 2);
    string marker = comp.toEscapedString();
    if (marker == "%C1.FS.dir") {
        cerr << "Requested name correspondes to a directory." << endl;
    } else if (marker == "%C1.FS.file") {
        ndnfs::FileInfo infof;
        if (infof.ParseFromArray(content.buf(), content.size()) && infof.IsInitialized()) {
            cout << "File metadata received." << endl;
            cout << "  name:  " << data_name.toUri() << endl;
            cout << "  size:  " << infof.size() << endl;
            cout << "  version:   " << infof.version() << endl;
            cout << "  total segments: " << infof.totalseg() << endl;

            total_size = infof.size();
            total_seg = infof.totalseg();
            file_name = data_name.getPrefix(data_name.size() - 2);
            file_name.appendVersion((uint64_t)infof.version());
            cout << "File prefix with version is: " << file_name.toUri() << endl;

            cout << "Start to fetch file segments..." << endl;

            ptr_lib::shared_ptr<Interest> interestPtr(new Interest());
            interestPtr->setScope(ndn_Interest_ANSWER_CONTENT_STORE);
            interestPtr->setName(Name(file_name).appendSegment((uint64_t)current_seg));
            //interestPtr->setAnswerOriginKind(0);

            start = high_resolution_clock::now();
            handler.expressInterest(*interestPtr, onFileData, onTimeout);
        } else {
            cerr << "protobuf error" << endl;
        }
    } else
        cerr << "Unknown metadata marker: " << marker << endl;

    return;
}

void onFileData (const ptr_lib::shared_ptr<const Interest>& interest, const ptr_lib::shared_ptr<Data>& data) {
    //const Blob& content = data->getContent();
    const Name& data_name = data->getName();
    //cout << "Segment received: " << data_name.toUri() << endl;
    //cout << "    data: " << string((char*)content.buf(), content.size()) << endl;
    //for (int i = 0; i < content.size(); i++) {
    //    ofs << content[i];
    //}

    current_seg = (int)(data_name.rbegin()->toSegment());
    //cout << current_seg << endl;
    current_seg++;  // segments are zero-indexed
    if (current_seg == total_seg) {
        stdtime stop = high_resolution_clock::now();
        //stdduration td = stop - start;
        cout << "Last segment received." << endl;
        cout << "Total run time: " << duration_cast<milliseconds>(stop - start).count() << " ms" << endl;
        cout << "Total segment fetched: " << current_seg << endl;
        cout << "Throughput: " << (double) total_size / (double) duration_cast<milliseconds>(stop - start).count() / 1024 * 8 << " kbps" << endl;
        done = true;
    } else {
        ptr_lib::shared_ptr<Interest> interestPtr(new Interest());
        interestPtr->setScope(ndn_Interest_ANSWER_CONTENT_STORE);
        interestPtr->setName(Name(file_name).appendSegment((uint64_t)current_seg));
        //interestPtr->setAnswerOriginKind(0);    

        handler.expressInterest(*interestPtr, onFileData, onTimeout);
    }
}

void onTimeout (const ptr_lib::shared_ptr<const Interest>& origInterest) {
    cout << "Timeout!" << current_seg << endl;
    done = true;
}

void Usage () {
	fprintf(stderr, "usage: ./cat_file [-n name]\n");
	exit(1);
}


int main (int argc, char **argv) {
    ptr_lib::shared_ptr<Interest> interestPtr(new Interest());
    interestPtr->setScope(ndn_Interest_ANSWER_CONTENT_STORE);

	const char* name = NULL;
    bool repo_mode = false;

	int opt;
	while ((opt = getopt(argc, argv, "n:r")) != -1) {
		switch (opt) {
        case 'n': 
            name = optarg;
            cout << "main(): set name: " << name << endl;
            break;
        case 'r':
            // Fetch file from repo (unversioned but sequenced)
            repo_mode = true;
            break;
        default: 
            Usage(); 
            break;
        }
	}
    
    if (repo_mode) {
        file_name = Name(name);
        total_seg = 12654;
        total_size = 103655871;
        
        interestPtr->setName(Name(file_name).appendSegment(0));
        //interestPtr->setAnswerOriginKind(0);

        start = high_resolution_clock::now();
        handler.expressInterest(*interestPtr, onFileData, onTimeout);
    } else {
        interestPtr->setName(Name(name).append("%C1.FS.file"));
        //interestPtr->setAnswerOriginKind(0);
        handler.expressInterest(*interestPtr, onMetaData, onTimeout);
	}

	cout << "Started..." << endl;

	while (!done) {
        handler.processEvents();
        usleep (10);
    }

    //ofs.close();

	return 0;
}
