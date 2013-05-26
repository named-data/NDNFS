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
