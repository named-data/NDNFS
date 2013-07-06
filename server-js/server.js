var NDN = require('./ndn.js').NDN;
var Name = require('./ndn.js').Name;
var Interest = require('./ndn.js').Interest;
var ContentObject = require('./ndn.js').ContentObject;
var MongoClient = require('mongodb').MongoClient;

var ndn = new NDN();
ndn.setDefaultKey('./non.pub', './non.pem');

var global_prefix = null;
var collection = null;

var DIR_ENTRY = 0;
var FILE_ENTRY = 1;
var VER_ENTRY = 2;
var SEG_ENTRY = 3;

// Main event handler
var onInterest = function (interest) {
    console.log('Interest received in callback.');

    var query = interest.name.getSuffix(global_prefix.size());
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
    global_prefix = new Name('/wentao.shang/ndnfs');
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

