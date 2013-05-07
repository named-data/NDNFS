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

#include <sys/time.h>
#include <boost/lexical_cast.hpp>

#include "version.h"

using namespace std;
using namespace boost;
using namespace mongo;


static inline uint64_t timestamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}

void create_version(const string &path, ScopedDbConnection *c)
{
    uint64_t now = timestamp();
    string version = lexical_cast<string> (now);
    cout << "create_version: version " << version << " created for file " << path << endl;

    string full_path = path + "/" + version;
    BSONObj ver_entry = BSONObjBuilder().append("_id", full_path).append("type", version_type).append("mode", 0666)
	                .appendBinData("data", 0, BinDataGeneral, NULL).append("size", 0).obj();

    // Add verion entry to database
    c->conn().insert(db_name, ver_entry);
     
    // Create new file entry with empty content, "data" field contains the version just created
    BSONObj file_entry = BSONObjBuilder().append("_id", path).append("type", file_type)
	                 .append("data", BSONArrayBuilder().append(version).arr()).obj();

    // Add the file entry to database
    c->conn().insert(db_name, file_entry);

    return;
}

void add_version_with_data(const string &path, ScopedDbConnection *c, const char *file_data, int file_size)
{
    ndn::Name file_name(path);
    uint64_t now = timestamp();
    string version = lexical_cast<string> (now);
    cout << "add_version_with_data: version " << version << " created for file " << path << endl;

    file_name.appendVersion(now);    
    string full_path = path + "/" + version;
    ndn::Bytes co = ndn_wrapper.createContentObject(file_name, file_data, file_size);
    unsigned char *co_raw = ndn::head(co);
    int co_size = co.size();

    BSONObj ver_entry = BSONObjBuilder().append("_id", full_path).append("type", version_type).append("mode", 0666)
                        .appendBinData("data", co_size, BinDataGeneral, co_raw).append("size", file_size).obj();

    // Add verion entry to database
    c->conn().insert(db_name, ver_entry);
    // Append version code to existing BSON array of the parent file
    c->conn().update(db_name, BSON("_id" << path), BSON( "$push" << BSON( "data" << version ) ));
    
    return;
}

static inline string get_latest_version_name(BSONObj &entry)
{
    vector< BSONElement > data = entry["data"].Array();
    return data[ data.size() - 1 ].String();
}

int get_latest_version_info(const string &path, ScopedDbConnection *c, BSONObj &entry, int &mode, int& size)
{
    string version = get_latest_version_name(entry);
    string full_path = path + "/" + version;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << full_path));
    if (!cursor->more()) {
        return -1;
    }

    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
        return -1;
    }
    
    size = ver_entry.getIntField("size");
    mode = ver_entry.getIntField("mode");
    
    return 0;
}

const char *get_latest_version_data(const string &path, ScopedDbConnection *c, BSONObj &entry, int &data_length)
{
    string version = get_latest_version_name(entry);
    string full_path = path + "/" + version;

    auto_ptr<DBClientCursor> cursor = c->conn().query(db_name, QUERY("_id" << full_path));
    if (!cursor->more()) {
	data_length = -1;
        return NULL;
    }

    BSONObj ver_entry = cursor->next();
    if (ver_entry.getIntField("type") != version_type) {
        data_length = -1;
        return NULL;
    }
    
    if (ver_entry.getIntField("size") == 0) {
        data_length = 0;
        return NULL;
    }
    
    return ver_entry.getField("data").binData(data_length);
}

void remove_versions_and_file(const string &path, ScopedDbConnection *c, BSONObj &entry)
{
    vector< BSONElement > vers = entry["data"].Array();
    for (int i = 0; i < vers.size(); i++) {
        string full_path = path + "/" + vers[i].String();
	c->conn().remove(db_name, QUERY("_id" << full_path));
    }    

    c->conn().remove(db_name, QUERY("_id" << path));
}
