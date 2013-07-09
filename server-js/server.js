var NDN = require('ndn-on-node').NDN;
var Name = require('ndn-on-node').Name;
var Interest = require('ndn-on-node').Interest;
var ContentObject = require('ndn-on-node').ContentObject;
var MongoClient = require('mongodb').MongoClient;

if (process.argv.length != 3)
    throw new Error('must specify global prefix as a command-line parameter.');

var ndn = new NDN();
ndn.setDefaultKey('./non.pub', './non.pem');

var global_prefix = new Name(process.argv[2]);
var collection = null;

var DIR_ENTRY = 0;
var FILE_ENTRY = 1;
var VER_ENTRY = 2;
var SEG_ENTRY = 3;

// Main event handler
var onInterest = function (interest) {
    console.log('Interest received in callback.');

    var query = interest.name.getSuffix(global_prefix.size());
    // The last two components may be version or segment number
    // Convert them into ascii-encoded integer strings
    for (var i = query.size() - 1; i >= query.size() - 2 && i >= 0; i--) {
	var component = query.components[i];
	if (component[0] == 0 || component[0] == 0xFD) {
            // this component is version of segment number
	    component[0] = 0;  // reset 0xfd to 0, in case of a version number
	    var num = parseInt(component.toString('hex'), 16);
	    query.components[i] = new Buffer(num.toString());
	}
    }
    var str = query.to_uri();
    console.log('Asking for Name ' + str);

    // Closure for recursive database query.
    var recQuery = function (err, item) {
	if (!item) {
	    console.log('Cannot find entry for ' + str);
	    return;
	}

	switch (item.type) {
	case DIR_ENTRY:
	console.log('Directory entry found for ' + str);
	str = str + '/' + item.data[0];
	collection.findOne({_id:str}, recQuery);
	break;

	case FILE_ENTRY:
	console.log('File entry found for ' + str);
	str = str + '/' + item.data;
	collection.findOne({_id:str}, recQuery);
	break;

	case VER_ENTRY:
	console.log('Version entry found for ' + str);
	str = str + '/' + item.data[0];
	collection.findOne({_id:str}, recQuery);
	break;

	case SEG_ENTRY:
	console.log('Segment entry found for ' + str);
	var co = ContentObject.parse(item.data.buffer);
	try {
	    ndn.send(co);
	} catch (e) {
	    console.log(e.toString());
	}
	break;

	default:
	break;
	}
    };

    collection.findOne({_id:str}, recQuery);
};

ndn.onopen = function () {
    console.log('Connected to local ccnd.');
    ndn.registerPrefix(global_prefix, onInterest);
    console.log('NDNFS prefix registered. Start listening for Interests...');
};

MongoClient.connect("mongodb://localhost:27017/ndnfs", function(err, db) {
	if (err) { console.log('Cannot connect to MongoDB.'); return; }
	
	console.log('Connected to NDNFS database in MongoDB.');

	collection = db.collection('root');

	collection.findOne({_id:'/'}, function(err, item) {
		if (!item) {
		    console.log('Cannot find root directory in database.');
		    db.close();
		    return;
		}
		console.log('Root directory found in database.');

		ndn.connect();
	    });
    });

