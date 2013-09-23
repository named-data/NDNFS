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

#ifndef __SERVER_MODULE_H__
#define __SERVER_MODULE_H__

#include <string>

#include <ndn.cxx/wrapper/wrapper.h>
#include <ndn.cxx/common.h>
#include <ndn.cxx/fields/name.h>
#include <ndn.cxx/interest.h>
#include <ndn.cxx/security/identity/osx-privatekey-store.h>
#include "dir.pb.h"
#include "file.pb.h"

#include <sqlite3.h>

extern const char *db_name;
extern sqlite3 *db;
extern ndn::Ptr<ndn::security::OSXPrivatekeyStore> privateStoragePtr;
extern ndn::Ptr<ndn::security::Keychain> keychain;
extern ndn::Name signer;

// Global prefix for NDNFS
extern std::string global_prefix;

void OnInterest(ndn::Ptr<ndn::Interest> interest);

void ndnName2String(const ndn::Name& name, uint64_t &version, int &seg, std::string &path);

void ProcessName(ndn::Name& interest_name);

void SendDir(const std::string& path, int mtime);

//bool CompareComponent(const std::string& a, const std::string& b);
void SendFile(const std::string& path, uint64_t version, int sizef, int totalseg);

#endif // __SERVER_MODULE_H__
