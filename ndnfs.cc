#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>

#include <fuse.h>
#include "mongo/client/dbclient.h"

using namespace std;;
using namespace mongo;;

static const char  *file_path      = "/hello.txt";
static const char   file_content[] = "Hello World!\n";
static const size_t file_size      = sizeof(file_content)/sizeof(char) - 1;

static DBClientConnection c;

static int
ndnfs_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 3;
    } else if (strcmp(path, file_path) == 0) { /* The only file we have. */
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = file_size;
    } else /* We reject everything else. */
        return -ENOENT;

    return 0;
}

static int
ndnfs_open(const char *path, struct fuse_file_info *fi)
{
    if (strcmp(path, file_path) != 0) /* We only recognize one file. */
        return -ENOENT;

    if ((fi->flags & O_ACCMODE) != O_RDONLY) /* Only reading allowed. */
        return -EACCES;

    return 0;
}

static int
ndnfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
              off_t offset, struct fuse_file_info *fi)
{
    if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
        return -ENOENT;
    
    auto_ptr<DBClientCursor> cursor = c.query("test.ndnfs", QUERY("name" << "/"));

    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
    while (cursor->more()) {
        BSONObj p = cursor->next();
        BSONElement dir = p["subdir"];
        BSONObj dir_obj = dir.Obj();
        filler(buf, dir_obj.getStringField("name"), NULL, 0);
    }
    return 0;
}

static int
ndnfs_read(const char *path, char *buf, size_t size, off_t offset,
           struct fuse_file_info *fi)
{
    if (strcmp(path, file_path) != 0)
        return -ENOENT;

    if ((size_t)offset >= file_size) /* Trying to read past the end of file. */
        return 0;

    if (offset + size > file_size) /* Trim the read to the file size. */
        size = file_size - offset;

    memcpy(buf, file_content + offset, size); /* Provide the content. */

    return size;
}

static void
create_fuse_operations(struct fuse_operations *fuse_op)
{
    fuse_op->getattr = ndnfs_getattr; /* To provide size, permissions, etc. */
    fuse_op->open    = ndnfs_open;    /* To enforce read-only access.       */
    fuse_op->read    = ndnfs_read;    /* To provide file content.           */
    fuse_op->readdir = ndnfs_readdir; /* To provide directory listing.      */
}


int
main(int argc, char **argv)
{
    try {
        c.connect("localhost");
        std::cout << "connected ok" << std::endl;
    } catch( const mongo::DBException &e ) {
        std::cout << "caught " << e.what() << std::endl;
        return -1;
    }
    
    BSONObj file_mapping = BSON( "name" << "hello.txt" << "content" << "Hello World!\n");
    BSONObj root_dir = BSON( "name" << "/" << "subdir" << file_mapping);
    c.insert("test.ndnfs", root_dir);
    
    struct fuse_operations ndnfs_fs_ops;
    create_fuse_operations(&ndnfs_fs_ops);
    
    return fuse_main(argc, argv, &ndnfs_fs_ops, NULL);
}
