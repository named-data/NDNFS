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
 * Author: Qiuhan Ding <dingqiuhan@gmail.com>, Wentao Shang <wentao@cs.ucla.edu>
 */

#include <iostream>

#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/fields/name.h>
#include <ndn.cxx/common.h>
#include <boost/lexical_cast.hpp>
#include <ndn.cxx/security/keychain.h>
#include <ndn.cxx/security/identity/osx-privatekey-store.h>
#include <ndn.cxx/common.h>
#include <ndn.cxx/security/certificate/certificate.h>
#include <sqlite3.h>
#include <boost/bind.hpp>
#include <unistd.h>

#include "servermodule.h"

using namespace std;
using namespace ndn;

const char *db_name = "/tmp/ndnfs.db";
sqlite3 *db;

// create a global handler
ndn::Name signer("/ndn/ucla.edu/qiuhan");
Ptr<security::OSXPrivatekeyStore> privateStoragePtr = Ptr<security::OSXPrivatekeyStore>::Create();
Ptr<security::Keychain> keychain = Ptr<security::Keychain>(new security::Keychain(privateStoragePtr, "/Users/ndn/qiuhan/policy", "/tmp/encryption.db"));//////policy needs to be changed
Ptr<Wrapper> handler = Ptr<Wrapper>(new Wrapper(keychain));

string global_prefix;

void
publishAllCert(Ptr<Wrapper> wrapper)
{
    cerr << "push all cert!" << endl;
    sqlite3 * fakeDB;
    int res = sqlite3_open("/Users/ndn/qiuhan/NDNFS/fake-data.db", &fakeDB);
  
    if (res != SQLITE_OK)
        cerr << "damn it!" << endl;

    sqlite3_stmt *stmt;
    sqlite3_prepare_v2 (fakeDB, "SELECT data_blob FROM data", -1, &stmt, 0);

    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        Blob dataBlob(sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));    
        cerr << "publish certificate" << endl;
        wrapper->putToCcnd(dataBlob);
    }

    sqlite3_close (fakeDB);
}

int main(int argc, char **argv) {
    const char* prefix = "/ndn/ucla.edu/qiuhan/dummy/ndnfs";
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
    
    publishAllCert(handler);
    Name certificatedsk_name("/ndn/ucla.edu/qiuhan/DSK-1378423613/ID-CERT/1378423803");
    Ptr<security::Certificate> certdsk = keychain->getAnyCertificate(certificatedsk_name);
    Ptr<Blob> cert0 = certdsk->encodeToWire();
    cerr << "publish DSK: " << cert0->size() << endl;
    handler->putToCcnd(*cert0);

    Name certificateksk_name("/ndn/ucla.edu/qiuhan/KSK-1378422677/ID-CERT/1378423300");
    Ptr<security::Certificate> certksk = keychain->getAnyCertificate(certificateksk_name);
    Ptr<Blob> cert1 = certksk->encodeToWire();
    cerr << "publish KSK: " << cert1->size() << endl;
    handler->putToCcnd(*cert1);
    
    cout << "main: open sqlite database" << endl;

    if (sqlite3_open(db_name, &db) == SQLITE_OK) {
        cout << "main: ok" << endl;
    } else {
        cout << "main: cannot connect to sqlite db, quit" << endl;
        sqlite3_close(db);
        return -1;
    }

    cout << "serving prefix: " << prefix << endl;
    Name InterestBaseName(prefix);
    global_prefix = InterestBaseName.toUri();
    cout << "global prefix for NDNFS: " << global_prefix << endl;

    handler->setInterestFilter(InterestBaseName, OnInterest);
    while (true) {
        sleep (1);
    }
	
    cout << "main(): ServerModule exiting ..." << endl;

    return 0;
}

