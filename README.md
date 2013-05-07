NDNFS
=====

NDN-friendly file system (based on FUSE)

Compilation dependency (on Mac):

OXSFUSE 2.5.5;
mongodb 2.4.2 (including its cpp library ver. 2.4.2);
ndn.cxx library;
boost;

To test on Mac:

mkdir /tmp/ndnfs
./build/ndnfs /tmp/ndnfs

umount /tmp/ndnfs


Use '-f' to run in foreground and see printf debug info

./build/ndnfs -f /tmp/ndnfs

If '-f' is used, NDNFS is unmounted automatically when you kill 'ndnfs' process
