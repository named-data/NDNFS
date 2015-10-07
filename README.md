NDNFS
=====

NDN-friendly file system (based on FUSE)

Update Oct, 2015 (Zhehao)
-----------------------------

An updated version of the implementation (that works with the ndn-cpp (tested with 0.7)) can be found [here](https://github.com/remap/ndnfs-port). More later.

Compilation on Mac OSX 10.8.3
-----------------------------

Dependencies:

* OXSFUSE 2.5.6;
* MongoDB 2.4.5 (including its cpp library ver. 2.4.5);
* ndn.cxx library;
* boost library;

To compile the source code:

    ./waf configure (--debug)
    ./waf

To run on Mac:

    mkdir /tmp/ndnfs
    ./build/ndnfs /tmp/ndnfs

This will mount the file system to local folder '/tmp/ndnfs/'. To unmount NDNFS, simply type:

    umount /tmp/ndnfs

Use '-f' to run in foreground and see debug info (if you compiled with --debug option):

    ./build/ndnfs -f /tmp/ndnfs

If '-f' is used, NDNFS is unmounted automatically when you kill 'ndnfs' process.

To specify a global prefix for all the files stored in NDNFS:

    ./build/ndnfs -f /tmp/ndnfs -o prefix=/ndn/ucla.edu/cs/wentao/ndnfs

In this case, the NDN Content Object name is the global prefix + absolute file path.

Compilation on Ubuntu 12.04
---------------------------

Dependencies:

* libfuse-dev (can be installed with apt-get)
* libboost-all-dev (can be installed with apt-get)
* MongoDB library 2.4.3
* ndn.cxx library

Note: the MongoDB package in the Ubuntu default package pool is too old. Follow http://docs.mongodb.org/manual/tutorial/install-mongodb-on-ubuntu/ to install MongoDB 2.4.3 and then download and install MongoDB C++ Driver 2.4.3 from http://dl.mongodb.org/dl/cxx-driver.

Usage is the same as on Mac OSX.