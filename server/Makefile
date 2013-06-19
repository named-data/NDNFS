# * Copyright (c) 2013 University of California, Los Angeles
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License version 2 as
# * published by the Free Software Foundation;
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# *
# * Author: Zhe Wen <wenzhe@cs.ucla.edu>

CXX = g++

CFLAGS = -c -Wall

LDFLAGS =

# lbooster lmongoclient lboost_thread lboost_filesystem lboost_program_options
# lboost_system
LIBRARIES =	-pthread \
			-lndn.cxx \
			-lmongoclient \
			-lboost_thread \
			-lboost_filesystem \
			-lboost_program_options \
			-lboost_system
INCLUDE_PATHS = -I/usr/local/include \
				-I/usr/include/boost \
				-I/usr/local/include/mongo
LIBRARY_PATHS = -L/usr/lib \
				-L/usr/local/lib

SOURCES = server.cc servermodule.cc 

all: server client

server: server.o servermodule.o
	$(CXX) $(LDFLAGS) server.o servermodule.o $(LIBRARY_PATHS) $(LIBRARIES) -o server

server.o: server.cc
	$(CXX) server.cc $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -o server.o

servermodule.o: servermodule.cc
	$(CXX) servermodule.cc $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -o servermodule.o

client: client.o
	$(CXX) $(LDFLAGS) client.o $(LIBRARY_PATHS) $(LIBRARIES) -o client

client.o: client.cc
	$(CXX) client.cc $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) -o client.o

clean:
	    rm -f *.o
		rm -f server
		rm -f client
