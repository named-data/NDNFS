#ifndef NDNFS_ATTRIBUTE_H
#define NDNFS_ATTRIBUTE_H

#include "ndnfs.h"

int ndnfs_getattr(const char *path, struct stat *stbuf);

#endif
