NDNFS Server Module
===================

Server Module for NDNFS-network interaction

Compilation on Ubuntu 12.04
---------------------------

Dependencies:

* mongodb 2.4.3 (including its cpp library version 2.4.3)
* ccnx-cpp library
* ccnx library
* boost library

To compile the source code:

	./make

To run on Ubuntu:
	
	./server [-p prefix] [-d dbname]

By default, prefix = "/", dbname = 'ndnfs.root'. This will start the server module that serves incoming interest with name under prefix "prefix" for possible matching content objects stored in database on localhost specified by "dbname"
	./client [-n prefix] [-i minsuffixcomps] [-a maxsuffixcomps] [-c childselector]

Must at least specify prefix with -n option. This will issue an interest with speficied name selectors and send it to localhost for data fectching.

Compilation on Mac OS
---------------------

This should be exactly the same as that on Ubuntu 12.04
