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
#include <ndn-cpp/security/key-chain.hpp>
#include <ndn-cpp/security/identity/osx-private-key-storage.hpp>
#include <ndn-cpp/security/identity/memory-private-key-storage.hpp>
#include <ndn-cpp/security/identity/basic-identity-storage.hpp>
#include <ndn-cpp/security/policy/no-verify-policy-manager.hpp>
#include <ndn-cpp/security/certificate/certificate.hpp>
#include <sqlite3.h>
#include <unistd.h>

#include "servermodule.h"

using namespace std;
using namespace ndn;

const char *db_name = "/tmp/ndnfs.db";
sqlite3 *db;

Name signer("/ndn/edu/ucla/cs/irl/imac");

ptr_lib::shared_ptr<OSXPrivateKeyStorage> privateStoragePtr(new OSXPrivateKeyStorage());
ndn::ptr_lib::shared_ptr<ndn::BasicIdentityStorage> identityStoragePtr(new ndn::BasicIdentityStorage());
KeyChain keychain(
    ptr_lib::make_shared<IdentityManager>(identityStoragePtr, privateStoragePtr), 
    ptr_lib::make_shared<NoVerifyPolicyManager>());

Face handler("localhost");

string global_prefix;

int main(int argc, char **argv) {
    const char* prefix = "/ndn/ucla.edu/cs/irl/imac/ndnfs";
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

    handler.registerPrefix(InterestBaseName, ::OnInterest, ::OnRegisterFailed);
    while (true) {
        handler.processEvents();
        usleep (10);
    }
	
    cout << "main(): ServerModule exiting ..." << endl;

    return 0;
}

