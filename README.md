NDNFS
=====

NDN-friendly file system (based on FUSE)

To test on Mac:

./ndnfs /tmp/ndnfs

umount /tmp/ndnfs


Use '-f' to run in foreground and see printf debug info

If '-f' is used, NDNFS is unmounted automatically when you kill 'ndnfs' process
