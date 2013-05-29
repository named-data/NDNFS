NDNFS
=====

NDN-friendly file system (based on FUSE)

Compilation on Mac OSX 10.8.3
-----------------------------

Dependencies:

* OXSFUSE 2.5.6;
* mongodb 2.4.3 (including its cpp library ver. 2.4.3);
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

If '-f' is used, NDNFS is unmounted automatically when you kill 'ndnfs' process


Compilation on Ubuntu 12.04
---------------------------

Dependencies:

* libfuse-dev (can be installed with apt-get)
* libboost-all-dev (can be installed with apt-get)
* mongodb library 2.4.3
* ndn.cxx library

Note: the default mongodb package in the Ubuntu package pool is too old. Follow http://docs.mongodb.org/manual/tutorial/install-mongodb-on-ubuntu/ to install MongoDB 2.4.3 and then download and install MongoDB C++ Driver 2.4.3 from http://dl.mongodb.org/dl/cxx-driver.

Usage is the same as on Mac OSX.