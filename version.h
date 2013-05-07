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

#ifndef VERSION_H
#define VERSION_H

#include "ndnfs.h"

void create_version(const char *path, mongo::ScopedDbConnection *c);

void add_version_with_data(const char *path, mongo::ScopedDbConnection *c, const char *file_data, int file_size);

int get_latest_version_info(const char *path, mongo::ScopedDbConnection *c, mongo::BSONObj &entry, int &mode, int& size);

const char *get_latest_version_data(const char *path, mongo::ScopedDbConnection *c, mongo::BSONObj &entry, int &data_length);

void remove_versions_and_file(const char *path, mongo::ScopedDbConnection *c, mongo::BSONObj &entry);

#endif
