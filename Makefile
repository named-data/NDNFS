TARGETS = ndnfs

CC = g++
CFLAGS_OSXFUSE = -D__FreeBSD__=10 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I/usr/local/include/osxfuse/fuse
CFLAGS_EXTRA = -Wall -g
LIBS = -losxfuse

.c:
	$(CC) $(CFLAGS_OSXFUSE) $(CFLAGS_EXTRA) -o ndnfs $< $(LIBS)

all: $(TARGETS)

ndnfs: ndnfs.c

clean:
	rm -f $(TARGETS) *.o
