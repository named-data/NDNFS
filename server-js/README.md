NDNFS Server in JavaScript
==========================

A simple NDNFS server implemented in JavaScript, using NDN-on-Node library. Currently it does not support interest selectors.

To run the server, install NDN-on-Node first:

* Copy 'ndn.js' lib file into the folder that contains 'server.js'.
* Create public key pair files under the name 'non.pub' and 'non.pem' and put them in the same folder. (You may use different names but remember to change server.js code accordingly)

Then run the server with node.js

     node server.js

Currently the global prefix is hard-coded. Need to change accordingly.