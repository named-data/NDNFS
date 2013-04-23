TARGETS = ndnfs

CC = g++
CXXFLAGS_OSXFUSE = -D__FreeBSD__=10 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I/usr/local/include/osxfuse/fuse
CXXFLAGS_MONGODB = -I/opt/local/include/
CXXFLAGS_EXTRA = -Wall -g
LIBS = -losxfuse
LIBS_MONGODB = -pthread -lmongoclient -lboost_thread-mt -lboost_filesystem-mt -lboost_program_options-mt -lboost_system-mt
LDFLAGS = -L/opt/local/lib/

.cc:
	$(CC) $(CXXFLAGS_OSXFUSE) $(CXXFLAGS_MONGODB) $(CXXFLAGS_EXTRA) -o ndnfs $< $(LIBS) $(LIBS_MONGODB) $(LDFLAGS)

all: $(TARGETS)

ndnfs: ndnfs.cc

clean:
	rm -f $(TARGETS) *.o
