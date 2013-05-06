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
 */

#include "ndnfs.h"
#include "directory.h"
#include "file.h"
#include "attribute.h"

using namespace std;
using namespace boost;
using namespace mongo;

ndn::Wrapper ndn_wrapper;
const char *db_name = "ndnfs.root";

const int dir_type = 0;
const int file_type = 1;
const int version_type = 2;
const int segment_type = 3;


static void create_fuse_operations(struct fuse_operations *fuse_op)
{
    fuse_op->getattr = ndnfs_getattr;
    fuse_op->open    = ndnfs_open;
    fuse_op->read    = ndnfs_read;
    fuse_op->readdir = ndnfs_readdir;
    fuse_op->create  = ndnfs_create;
    fuse_op->write   = ndnfs_write;
    fuse_op->unlink  = ndnfs_unlink;
    fuse_op->mkdir   = ndnfs_mkdir;
    fuse_op->rmdir   = ndnfs_rmdir;
}

static struct fuse_operations ndnfs_fs_ops;

int main(int argc, char **argv)
{
    cout << "main: NDNFS version beta 0.1" << endl;
    cout << "main: test mongodb connection..." << endl;
    
    ScopedDbConnection *c = ScopedDbConnection::getScopedDbConnection("localhost");
    if (c->ok()) {
        cout << "main: ok" << endl;
    } else {
        cout << "main: cannot connect to local mongodb, quit..." << endl;
        c->done();
        delete c;
        return -1;
    }
    
    cout << "main: mount root directory..." << endl;
    // For test purpose, clear database first
    c->conn().dropCollection(db_name);
    c->conn().createCollection(db_name);
    
    // Check database first
    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << "/"));
    if (!cursor->more()) {
        // Create root directory as an empty folder if database is empty
        BSONObj root_dir = BSONObjBuilder().append("_id", "/").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, root_dir);
        
        /* For test use, should remove later */
        // Add a file
        const char *hello = "Hello World!\n";
        ndn::Bytes hello_co = ndn_wrapper.createContentObject(ndn::Name ("/hello.txt"),
                                                         hello,
                                                         13);
        unsigned char *co_raw = ndn::head(hello_co);
        int co_len = hello_co.size();
        BSONObj hello_file = BSONObjBuilder().append("_id", "/hello.txt").append("type", file_type).append("mode", 0666)
                            .appendBinData("data", co_len, BinDataGeneral, co_raw).append("size", 13).obj();
        c->conn().insert(db_name, hello_file);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello.txt" ) ) );
        // Add an empty file
        BSONObj empty_file = BSONObjBuilder().append("_id", "/empty.txt").append("type", file_type).append("mode", 0666)
                            .appendBinData("data", 0, BinDataGeneral, NULL).append("size", 0).obj();
        c->conn().insert(db_name, empty_file);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "empty.txt" ) ) );
        // Add a folder
        BSONObj hello_dir = BSONObjBuilder().append("_id", "/hello").append("type", dir_type).append("mode", 0777)
                            .append("data", BSONArrayBuilder().arr()).obj();
        c->conn().insert(db_name, hello_dir);
        c->conn().update(db_name, BSON( "_id" << "/" ), BSON( "$push" << BSON( "data" << "hello" ) ) );
        
        /*
        // Test parsing content object
        auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << "/hello.txt"));
        BSONObj entry = cursor->next();
        int co_size;
        const char *co = entry.getField("data").binData(co_size);
        Ccnx::ParsedContentObject pco((const unsigned char *)co, co_size);
        
        Ccnx::BytesPtr content = pco.contentPtr();
        cout << "parsed content is: " << Ccnx::head(*content) << endl;
         */
        
        /* End of test */
    }
    cout << "main: ok" << endl;
    
    c->done();
    delete c;
    
    create_fuse_operations(&ndnfs_fs_ops);
    
    cout << "main: enter FUSE main loop" << endl;
    return fuse_main(argc, argv, &ndnfs_fs_ops, NULL);
}
