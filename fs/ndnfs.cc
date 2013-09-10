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
 * Author: Wentao Shang <wentao@cs.ucla.edu>
 *         Qiuhan Ding <dingqiuhan@gmail.com>
 */

#include "ndnfs.h"
#include "directory.h"
#include "file.h"
#include "attribute.h"

#include <unistd.h>
#include <sys/types.h>

using namespace std;
using namespace boost;

const char *db_name = "/tmp/ndnfs.db";
sqlite3 *db;

ndn::Name signer("/ndn/ucla.edu/qiuhan");
string ndnfs::global_prefix;

ndn::Ptr<ndn::security::OSXPrivatekeyStore> privateStoragePtr = ndn::Ptr<ndn::security::OSXPrivatekeyStore>::Create();
ndn::Ptr<ndn::security::Keychain> keychain = ndn::Ptr<ndn::security::Keychain>(new ndn::security::Keychain(privateStoragePtr, "/Users/ndn/qiuhan/policy", "/tmp/encryption.db"));
//////policy needs to be changed

const int ndnfs::dir_type = 0;
const int ndnfs::file_type = 1;

const int ndnfs::seg_size = 4096;  // size of the content in each content object segment counted in bytes
const int ndnfs::seg_size_shift = 12;

int ndnfs::user_id = 0;
int ndnfs::group_id = 0;

static void create_fuse_operations(struct fuse_operations *fuse_op)
{
    fuse_op->getattr  = ndnfs_getattr;
    fuse_op->chmod    = ndnfs_chmod;
    fuse_op->setxattr = ndnfs_setxattr;
    fuse_op->open     = ndnfs_open;
    fuse_op->read     = ndnfs_read;
    fuse_op->readdir  = ndnfs_readdir;
    fuse_op->create   = ndnfs_create;
    fuse_op->write    = ndnfs_write;
    fuse_op->truncate = ndnfs_truncate;
    fuse_op->release  = ndnfs_release;
    fuse_op->unlink   = ndnfs_unlink;
    fuse_op->mkdir    = ndnfs_mkdir;
    fuse_op->rmdir    = ndnfs_rmdir;
}

static struct fuse_operations ndnfs_fs_ops;

struct ndnfs_config {
    char *prefix;
};

#define NDNFS_OPT(t, p, v) { t, offsetof(struct ndnfs_config, p), v }

static struct fuse_opt ndnfs_opts[] = {
    NDNFS_OPT("prefix=%s", prefix, 0),
    FUSE_OPT_END
};

int main(int argc, char **argv)
{
    assert((1 << ndnfs::seg_size_shift) == ndnfs::seg_size);
    ndnfs::global_prefix = "/ndn/ucla.edu/qiuhan/dummy/ndnfs";
    
    cout << "main: NDNFS version 0.2" << endl;
    
    // uid and gid will be set to that of the user who starts the fuse process
    ndnfs::user_id = getuid();
    ndnfs::group_id = getgid();

    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct ndnfs_config conf;
    memset(&conf, 0, sizeof(conf));
    fuse_opt_parse(&args, &conf, ndnfs_opts, NULL);

    if (conf.prefix != NULL) {
	ndn::Name InterestBaseName(conf.prefix);
	ndnfs::global_prefix = InterestBaseName.toUri();
    }
    cout << "main: global prefix is " << ndnfs::global_prefix << endl;

    cout << "main: test sqlite connection..." << endl;

    if (sqlite3_open(db_name, &db) == SQLITE_OK) {
        cout << "main: ok" << endl;
    } else {
        cout << "main: cannot connect to sqlite db, quit" << endl;
	sqlite3_close(db);
        return -1;
    }
    
    cout << "main: init tables in sqlite db..." << endl;
    
    // Init tables in database
    const char* INIT_FS_TABLE = "\
CREATE TABLE IF NOT EXISTS                        \n\
  file_system(                                    \n\
    path                 TEXT NOT NULL,           \n\
    parent               TEXT NOT NULL,           \n\
    type                 INTEGER,                 \n\
    mode                 INTEGER,                 \n\
    atime                INTEGER,                 \n\
    mtime                INTEGER,                 \n\
    size                 INTEGER,                 \n\
    current_version      INTEGER,                 \n\
    temp_version         INTEGER,                 \n\
    PRIMARY KEY (path)                            \n\
  );                                              \n\
CREATE INDEX id_fs ON file_system (path);         \n\
";

    if (sqlite3_exec(db, INIT_FS_TABLE, NULL, NULL, NULL) != SQLITE_OK) {
	cout << "Cannot create file_system table in sqlite db" << endl;
	sqlite3_close(db);
        return -1;
    }

    const char* INIT_VER_TABLE = "\
CREATE TABLE IF NOT EXISTS                                   \n\
  file_versions(                                             \n\
    path          TEXT NOT NULL,                             \n\
    version       INTEGER,                                   \n\
    size          INTEGER,                                   \n\
    totalSegments INTEGER,                                   \n\
    PRIMARY KEY (path)                                       \n\
  );                                                         \n\
CREATE INDEX id_ver ON file_versions (path, version);        \n\
";

    if (sqlite3_exec(db, INIT_VER_TABLE, NULL, NULL, NULL) != SQLITE_OK) {
	cout << "Cannot create file_versions table in sqlite db" << endl;
	sqlite3_close(db);
        return -1;
    }

    const char* INIT_SEG_TABLE = "\
CREATE TABLE IF NOT EXISTS                        \n\
  file_segments(                                  \n\
    name        TEXT NOT NULL,                    \n\
    data        BLOB NOT NULL,                    \n\
    offset      INTEGER,                          \n\
    PRIMARY KEY (name)                            \n\
  );                                              \n\
CREATE INDEX id_seg ON file_segments (name);      \n\
";

    if (sqlite3_exec(db, INIT_SEG_TABLE, NULL, NULL, NULL) != SQLITE_OK) {
	cout << "Cannot create file_segments table in sqlite db" << endl;
	sqlite3_close(db);
        return -1;
    }

    cout << "main: ok" << endl;

    create_fuse_operations(&ndnfs_fs_ops);
    
    cout << "main: enter FUSE main loop" << endl << endl;
    return fuse_main(args.argc, args.argv, &ndnfs_fs_ops, NULL);
}
