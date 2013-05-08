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
#include "segment.h"

std::string create_empty_version(const std::string& path, mongo::ScopedDbConnection *c);

int get_latest_version_info(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, int& mode, int& size);

int read_latest_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, char *output, size_t size, off_t offset);

std::string add_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, const char *buf, size_t size, off_t offset);

void remove_versions(const std::string& path, mongo::ScopedDbConnection *c);

int truncate_latest_version(const std::string& path, mongo::ScopedDbConnection *c, mongo::BSONObj& file_entry, off_t length);

#endif
