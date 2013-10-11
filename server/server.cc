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

#include <ndn-cpp/face.hpp>
#include <ndn-cpp/name.hpp>
#include <ndn-cpp/common.hpp>
#include <boost/lexical_cast.hpp>
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/security/identity/osx-private-key-storage.hpp>
#include <ndn-cpp/security/identity/basic-identity-storage.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>
#include <ndn-cpp/security/certificate/certificate.hpp>
#include <sqlite3.h>
#include <boost/bind.hpp>
#include <unistd.h>

#include "servermodule.h"

using namespace std;
using namespace ndn;

const char *db_name = "/tmp/ndnfs.db";
sqlite3 *db;

// create a global handler
Name signer("/ndn/ucla.edu/qiuhan");
ptr_lib::shared_ptr<OSXPrivateKeyStorage> privateStoragePtr(new OSXPrivateKeyStorage());
ptr_lib::shared_ptr<KeyChain> keychain(new KeyChain
  (ptr_lib::make_shared<IdentityManager>(ptr_lib::make_shared<BasicIdentityStorage>(), privateStoragePtr), 
   ptr_lib::make_shared<NoVerifyPolicyManager>()));//////policy needs to be changed
ptr_lib::shared_ptr<Transport> ndnTransport(new TcpTransport());
ptr_lib::shared_ptr<Face> handler(new Face(ndnTransport, ptr_lib::make_shared<TcpTransport::ConnectionInfo>("localhost")));

string global_prefix;

void
publishAllCert()
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
        cerr << "publish certificate" << endl;
        ndnTransport->send((const uint8_t*)sqlite3_column_blob(stmt, 0), sqlite3_column_bytes(stmt, 0));
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
    
    publishAllCert();
    Name certificatedsk_name("/ndn/ucla.edu/qiuhan/DSK-1378423613/ID-CERT/1378423803");
    SignedBlob cert0 = keychain->getAnyCertificate(certificatedsk_name)->wireEncode();
    cerr << "publish DSK: " << cert0->size() << endl;
    ndnTransport->send(*cert0);

    Name certificateksk_name("/ndn/ucla.edu/qiuhan/KSK-1378422677/ID-CERT/1378423300");
    SignedBlob cert1 = keychain->getAnyCertificate(certificateksk_name)->wireEncode();
    cerr << "publish KSK: " << cert1->size() << endl;
    ndnTransport->send(*cert1);
    
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

    handler->registerPrefix(InterestBaseName, ::OnInterest, ::OnRegisterFailed);
    while (true) {
        sleep (1);
    }
	
    cout << "main(): ServerModule exiting ..." << endl;

    return 0;
}

