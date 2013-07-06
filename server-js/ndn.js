/**
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 * This class contains all CCNx tags
 */

/**
 * Ported to node.js by Wentao Shang
 */


var CCNProtocolDTags = {

	/**
	 * Note if you add one of these, add it to the reverse string map as well.
	 * Emphasize getting the work done at compile time over trying to make something
	 * flexible and developer error-proof.
	 */

	 Any : 13,
	 Name : 14,
	 Component : 15,
	 Certificate : 16,
	 Collection : 17,
	 CompleteName : 18,
	 Content : 19,
	 SignedInfo : 20,
	 ContentDigest : 21,
	 ContentHash : 22,
	 Count : 24,
	 Header : 25,
	 Interest : 26,	/* 20090915 */
	 Key : 27,
	 KeyLocator : 28,
	 KeyName : 29,
	 Length : 30,
	 Link : 31,
	 LinkAuthenticator : 32,
	 NameComponentCount : 33,	/* DeprecatedInInterest */
	 RootDigest : 36,
	 Signature : 37,
	 Start : 38,
	 Timestamp : 39,
	 Type : 40,
	 Nonce : 41,
	 Scope : 42,
	 Exclude : 43,
	 Bloom : 44,
	 BloomSeed : 45,
	 AnswerOriginKind : 47,
	 InterestLifetime : 48,
	 Witness : 53,
	 SignatureBits : 54,
	 DigestAlgorithm : 55,
	 BlockSize : 56,
	 FreshnessSeconds : 58,
	 FinalBlockID : 59,
	 PublisherPublicKeyDigest : 60,
	 PublisherCertificateDigest : 61,
	 PublisherIssuerKeyDigest : 62,
	 PublisherIssuerCertificateDigest : 63,
	 ContentObject : 64,	/* 20090915 */
	 WrappedKey : 65,
	 WrappingKeyIdentifier : 66,
	 WrapAlgorithm : 67,
	 KeyAlgorithm : 68,
	 Label : 69,
	 EncryptedKey : 70,
	 EncryptedNonceKey : 71,
	 WrappingKeyName : 72,
	 Action : 73,
	 FaceID : 74,
	 IPProto : 75,
	 Host : 76,
	 Port : 77,
	 MulticastInterface : 78,
	 ForwardingFlags : 79,
	 FaceInstance : 80,
	 ForwardingEntry : 81,
	 MulticastTTL : 82,
	 MinSuffixComponents : 83,
	 MaxSuffixComponents : 84,
	 ChildSelector : 85,
	 RepositoryInfo : 86,
	 Version : 87,
	 RepositoryVersion : 88,
	 GlobalPrefix : 89,
	 LocalName : 90,
	 Policy : 91,
	 Namespace : 92,
	 GlobalPrefixName : 93,
	 PolicyVersion : 94,
	 KeyValueSet : 95,
	 KeyValuePair : 96,
	 IntegerValue : 97,
	 DecimalValue : 98,
	 StringValue : 99,
	 BinaryValue : 100,
	 NameValue : 101,
	 Entry : 102,
	 ACL : 103,
	 ParameterizedName : 104,
	 Prefix : 105,
	 Suffix : 106,
	 Root : 107,
	 ProfileName : 108,
	 Parameters : 109,
	 InfoString : 110,
	// 111 unallocated
	 StatusResponse : 112,
	 StatusCode : 113,
	 StatusText : 114,

	// Sync protocol
	 SyncNode : 115,
	 SyncNodeKind : 116,
	 SyncNodeElement : 117,
	 SyncVersion : 118,
	 SyncNodeElements : 119,
	 SyncContentHash : 120,
	 SyncLeafCount : 121,
	 SyncTreeDepth : 122,
	 SyncByteCount : 123,
	 ConfigSlice : 124,
	 ConfigSliceList : 125,
	 ConfigSliceOp : 126,

	// Remember to keep in sync with schema/tagnames.csvsdict
	 CCNProtocolDataUnit : 17702112,
	 CCNPROTOCOL_DATA_UNIT : "CCNProtocolDataUnit"
};

var CCNProtocolDTagsStrings = [
    null, null, null, null, null, null, null, null, null, null, null, null, null,
    "Any", "Name", "Component", "Certificate", "Collection", "CompleteName",
    "Content", "SignedInfo", "ContentDigest", "ContentHash", null, "Count", "Header",
    "Interest", "Key", "KeyLocator", "KeyName", "Length", "Link", "LinkAuthenticator",
    "NameComponentCount", null, null, "RootDigest", "Signature", "Start", "Timestamp", "Type",
    "Nonce", "Scope", "Exclude", "Bloom", "BloomSeed", null, "AnswerOriginKind",
    "InterestLifetime", null, null, null, null, "Witness", "SignatureBits", "DigestAlgorithm", "BlockSize",
    null, "FreshnessSeconds", "FinalBlockID", "PublisherPublicKeyDigest", "PublisherCertificateDigest",
    "PublisherIssuerKeyDigest", "PublisherIssuerCertificateDigest", "ContentObject",
    "WrappedKey", "WrappingKeyIdentifier", "WrapAlgorithm", "KeyAlgorithm", "Label",
    "EncryptedKey", "EncryptedNonceKey", "WrappingKeyName", "Action", "FaceID", "IPProto",
    "Host", "Port", "MulticastInterface", "ForwardingFlags", "FaceInstance",
    "ForwardingEntry", "MulticastTTL", "MinSuffixComponents", "MaxSuffixComponents", "ChildSelector",
    "RepositoryInfo", "Version", "RepositoryVersion", "GlobalPrefix", "LocalName",
    "Policy", "Namespace", "GlobalPrefixName", "PolicyVersion", "KeyValueSet", "KeyValuePair",
    "IntegerValue", "DecimalValue", "StringValue", "BinaryValue", "NameValue", "Entry",
    "ACL", "ParameterizedName", "Prefix", "Suffix", "Root", "ProfileName", "Parameters", "InfoString", null,
    "StatusResponse", "StatusCode", "StatusText", "SyncNode", "SyncNodeKind", "SyncNodeElement",
    "SyncVersion", "SyncNodeElements", "SyncContentHash", "SyncLeafCount", "SyncTreeDepth", "SyncByteCount",
    "ConfigSlice", "ConfigSliceList", "ConfigSliceOp" ];
/**
 * @author: Wentao Shang
 * See COPYING for copyright and distribution information.
 * This class represents CCNTime Objects
 */

/**
 * Ported to node.js by Wentao Shang
 */

/**
 * Create CCNTime object. If input is null, set input to current JavaScript time
 */
var CCNTime = function CCNTime(input) {
    if (input == null) {
	var d = new Date()
	input = d.getTime();
    } else if (typeof input != 'number')
	throw new NONError('CCNTimeError', 'invalid input type ' + (typeof input));
    
    this.msec = input;  // in milliseconds
    this.binary = DataUtils.unsignedIntToBigEndian((input / 1000) * 4096);
};


CCNTime.prototype.encodeToBinary = function() {
    return this.binary;
};


CCNTime.prototype.getJavascriptDate = function () {
    var d = new Date();
    d.setTime( this.msec );
    return d
};
/**
 * This class contains utilities to help parse the data
 * author: Meki Cheraoui, Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

/**
 * Ported to node.js by Wentao Shang
 */


var DataUtils = function DataUtils () {};

/*
 * arrays is an array of Buffers. Return a new Buffer which is the concatenation of all.
 */
DataUtils.concatArrays = function (arrays) {
    return Buffer.concat(arrays);
}

/**
 * Return true if a1 and a2 are the same length with equal elements.
 */
DataUtils.arraysEqual = function (a1, a2) {
    if (a1.length != a2.length)
        return false;
    
    for (var i = 0; i < a1.length; ++i) {
        if (a1[i] != a2[i])
            return false;
    }

    return true;
};

/*
 * Convert the big endian Buffer to an unsigned int.
 */
DataUtils.bigEndianToUnsignedInt = function (/*Buffer*/ bytes) {
    return parseInt(bytes.toString('hex'), 16);
};

/*
 * Convert the int value to a new big endian Buffer and return.
 * Throw Error ff value is negative. 
 */
DataUtils.unsignedIntToBigEndian = function (value) {
    if (value < 0)
        throw new NoNError('DataUtilsError', 'require unsigned int but get negative value: ' + value);

    var hex = Math.round(value).toString(16);
    if (hex.length % 2 == 1)
	hex = '0' + hex;
    
    return new Buffer(hex, 'hex');
};
/**
 * @author: Wentao Shang
 * See COPYING for copyright and distribution information.
 * This class represents JS exception objects
 */

var NoNError = function NoNError(name, msg) {
    this.name = name;
    this.message = msg;
};

NoNError.prototype = new Error();

NoNError.prototype.toString = function () {
    var name = this.name;
    var msg = this.message;
    if (name == null) name = "NoNError";
    if (msg == null) msg = "an error is thrown.";
    return name + ': ' + msg;
};
/**
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 * Encapsulate an Uint8Array and support dynamic reallocation.
 */

/**
 * Ported to node.js by Wentao Shang
 */


/*
 * Create a DynamicBuffer where this.array is a Buffer of size length.
 * If length is not supplied, use a default initial length.
 * The methods will update this.length.
 * To access the array, use this.array or call subarray.
 */
var DynamicBuffer = function DynamicBuffer(length) {
    if (!length)
        length = 16;

    this.array = new Buffer(length);
    this.length = length;
};

/*
 * Ensure that this.array has the length, reallocate and copy if necessary.
 * Update this.length which may be greater than length.
 */
DynamicBuffer.prototype.ensureLength = function(length) {
    if (this.array.length >= length)
        return;
    
    // See if double is enough.
    var newLength = this.array.length * 2;
    if (length > newLength)
        // The needed length is much greater, so use it.
        newLength = length;
    
    var newArray = new Buffer(newLength);
    this.array.copy(newArray);
    this.array = newArray;
    this.length = newLength;
};

/*
 * Call this.array.set(value, offset), reallocating if necessary. 
 */
DynamicBuffer.prototype.set = function(value, offset) {
    this.ensureLength(value.length + offset);
    value.copy(this.array, offset);
};

/*
 * Return this.array.subarray(begin, end);
 */
DynamicBuffer.prototype.subarray = function(begin, end) {
    return this.array.slice(begin, end);
}

/*
 * The same as subarray()
 */
DynamicBuffer.prototype.slice = function(begin, end) {
    return this.array.slice(begin, end);
}
/**
 * This class is used to encode ccnb binary elements (blob, type/value pairs).
 * 
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 */

/**
 * Ported to node.js by Wentao Shang
 */

var XML_EXT = 0x00; 	
var XML_TAG = 0x01; 
var XML_DTAG = 0x02; 
var XML_ATTR = 0x03; 
var XML_DATTR = 0x04; 
var XML_BLOB = 0x05; 
var XML_UDATA = 0x06; 
var XML_CLOSE = 0x0;
var XML_SUBTYPE_PROCESSING_INSTRUCTIONS = 16; 

var XML_TT_BITS = 3;
var XML_TT_MASK = ((1 << XML_TT_BITS) - 1);
var XML_TT_VAL_BITS = XML_TT_BITS + 1;
var XML_TT_VAL_MASK = ((1 << (XML_TT_VAL_BITS)) - 1);
var XML_REG_VAL_BITS = 7;
var XML_REG_VAL_MASK = ((1 << XML_REG_VAL_BITS) - 1);
var XML_TT_NO_MORE = (1 << XML_REG_VAL_BITS); // 0x80
var BYTE_MASK = 0xFF;
var LONG_BYTES = 8;
var LONG_BITS = 64;
	
var bits_11 = 0x0000007FF;
var bits_18 = 0x00003FFFF;
var bits_32 = 0x0FFFFFFFF;


var BinaryXMLEncoder = function BinaryXMLEncoder(){
    this.ostream = new DynamicBuffer(100);
    this.offset = 0;
};

exports.CcnbEncoder = BinaryXMLEncoder;

/*
 * Encode string in utf8.
 */
BinaryXMLEncoder.prototype.writeUString = function(/*String*/ utf8str) {
    this.encodeUString(utf8str, XML_UDATA);
};


BinaryXMLEncoder.prototype.writeBlob = function(/*Buffer*/ blob) {
    if (LOG > 4) console.log("writeBlob: ");
    if (LOG > 4) console.log(blob);
    this.encodeBlob(blob, blob.length);
};


BinaryXMLEncoder.prototype.writeStartElement = function (tag) {
    var dictionaryVal = tag;
    
    if (null == dictionaryVal) {
	this.encodeUString(tag, XML_TAG);
    } else {
	this.encodeTypeAndVal(XML_DTAG, dictionaryVal);
    }
};


BinaryXMLEncoder.prototype.writeEndElement = function() {
    if (LOG>4) console.log('Write end element at OFFSET ' + this.offset);
    this.ostream.ensureLength(this.offset + 1);
    this.ostream.array[this.offset] = XML_CLOSE;
    this.offset += 1;
};


BinaryXMLEncoder.prototype.writeElement = function(tag, data) {
    this.writeStartElement(tag);
    
    if (typeof data == 'number') {
	// Encode non-negative integer as decimal string
	if(LOG>4) console.log('Going to write a number ' + data);
	
	this.writeUString(data.toString());
    }
    else if (typeof data == 'string') {
	if(LOG>4) console.log('Going to write a string ' + data);
	
	this.writeUString(data);
    } else {
	if(LOG>4) console.log('Going to write a blob ' + data.toString('hex') );

	this.writeBlob(data);
    }
    
    this.writeEndElement();
};


var TypeAndVal = function TypeAndVal(_type, _val) {
    this.type = _type;
    this.val = _val;
};


BinaryXMLEncoder.prototype.encodeTypeAndVal = function (type, val) {
    if(LOG>4) console.log('Encoding type ' + type + ' and value '+ val);
    if(LOG>4) console.log('Offset is ' + this.offset);
	
    if ((type > XML_UDATA) || (type < 0) || (val < 0)) {
	throw new NoNError('EncodeError', "tag and value must be positive, and tag valid.");
    }
	
    // Encode backwards. Calculate how many bytes we need:
    var numEncodingBytes = this.numEncodingBytes(val);
    this.ostream.ensureLength(this.offset + numEncodingBytes);

    // Bottom 4 bits of val go in last byte with tag.
    this.ostream.array[this.offset + numEncodingBytes - 1] = 
    (BYTE_MASK &
     (((XML_TT_MASK & type) | 
       ((XML_TT_VAL_MASK & val) << XML_TT_BITS))) |
     XML_TT_NO_MORE); // set top bit for last byte
    val = val >>> XML_TT_VAL_BITS;
    
    // Rest of val goes into preceding bytes, 7 bits per byte, top bit
    // is "more" flag.
    var i = this.offset + numEncodingBytes - 2;
    while ((0 != val) && (i >= this.offset)) {
	this.ostream.array[i] = (BYTE_MASK & (val & XML_REG_VAL_MASK)); // leave top bit unset
	val = val >>> XML_REG_VAL_BITS;
	--i;
    }
    if (val != 0) {
	throw new NoNError('EncodeError', "this should not happen: miscalculated encoding");
    }
    this.offset += numEncodingBytes;
    
    return numEncodingBytes;
};


BinaryXMLEncoder.prototype.encodeUString = function (ustring, type) {
    if (null == ustring)
	throw new NoNError('EncodeError', 'cannot encode null string.');

    if (type == XML_TAG || type == XML_ATTR && ustring.length == 0)
	throw new NoNError('EncodeError', 'cannot encode empty string');
    
    if(LOG>4) console.log("The string to write is: " + ustring);

    var strBytes = new Buffer(ustring, 'ascii');
    
    this.encodeTypeAndVal(type, 
			  (((type == XML_TAG) || (type == XML_ATTR)) ?
			   (strBytes.length-1) :
			   strBytes.length));
    
    if(LOG>4) console.log("THE string buffer to write is:");
    if(LOG>4) console.log(strBytes);
    
    this.writeString(strBytes);
    this.offset += strBytes.length;
};


BinaryXMLEncoder.prototype.encodeBlob = function (blob, length) {
    if (null == blob)
	throw new NoNError('EncodeError', 'cannot encode null blob.');
    
    if(LOG>4) console.log('Length of blob is ' + length);

    this.encodeTypeAndVal(XML_BLOB, length);

    this.writeBlobArray(blob);
    this.offset += length;
};

var ENCODING_LIMIT_1_BYTE = ((1 << (XML_TT_VAL_BITS)) - 1);
var ENCODING_LIMIT_2_BYTES = ((1 << (XML_TT_VAL_BITS + XML_REG_VAL_BITS)) - 1);
var ENCODING_LIMIT_3_BYTES = ((1 << (XML_TT_VAL_BITS + 2 * XML_REG_VAL_BITS)) - 1);

BinaryXMLEncoder.prototype.numEncodingBytes = function (x) {
    if (x <= ENCODING_LIMIT_1_BYTE) return (1);
    if (x <= ENCODING_LIMIT_2_BYTES) return (2);
    if (x <= ENCODING_LIMIT_3_BYTES) return (3);
    
    var numbytes = 1;
    
    // Last byte gives you XML_TT_VAL_BITS
    // Remainder each give you XML_REG_VAL_BITS
    x = x >>> XML_TT_VAL_BITS;
    while (x != 0) {
        numbytes++;
	x = x >>> XML_REG_VAL_BITS;
    }
    return (numbytes);
};

BinaryXMLEncoder.prototype.writeDateTime = function (tag, time) {    
    if(LOG>4)console.log('Encoding CCNTime: ' + time.msec);
    if(LOG>4)console.log('Encoding CCNTime in binary value:' + time.encodeToBinary().toString('hex'));
    
    this.writeElement(tag, time.encodeToBinary());
};

// This does not update this.offset.
BinaryXMLEncoder.prototype.writeString = function (input) {	
    if(typeof input === 'string'){
    	if(LOG>4) console.log('Going to write a string: ' + input);
        
        this.ostream.ensureLength(this.offset + input.length);
	this.writeBlobArray(new Buffer(input, 'ascii'));
    }
    else{
	if(LOG>4) console.log('Going to write a string in binary form: ');
	if(LOG>4) console.log(input);
	
	this.writeBlobArray(input);
    }
};


BinaryXMLEncoder.prototype.writeBlobArray = function (/* Buffer */ blob) {	
    this.ostream.set(blob, this.offset);
};


BinaryXMLEncoder.prototype.getReducedOstream = function () {
    return this.ostream.subarray(0, this.offset);
};
/**
 * This class is used to decode ccnb binary elements (blob, type/value pairs).
 * 
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 */

/**
 * Ported to node.js by Wentao Shang
 */

var XML_EXT = 0x00; 
var XML_TAG = 0x01; 
var XML_DTAG = 0x02; 
var XML_ATTR = 0x03; 
var XML_DATTR = 0x04; 
var XML_BLOB = 0x05; 
var XML_UDATA = 0x06; 
var XML_CLOSE = 0x0;
var XML_SUBTYPE_PROCESSING_INSTRUCTIONS = 16; 

var XML_TT_BITS = 3;
var XML_TT_MASK = ((1 << XML_TT_BITS) - 1);
var XML_TT_VAL_BITS = XML_TT_BITS + 1;
var XML_TT_VAL_MASK = ((1 << (XML_TT_VAL_BITS)) - 1);
var XML_REG_VAL_BITS = 7;
var XML_REG_VAL_MASK = ((1 << XML_REG_VAL_BITS) - 1);
var XML_TT_NO_MORE = (1 << XML_REG_VAL_BITS); // 0x80
var BYTE_MASK = 0xFF;
var LONG_BYTES = 8;
var LONG_BITS = 64;
	
var bits_11 = 0x0000007FF;
var bits_18 = 0x00003FFFF;
var bits_32 = 0x0FFFFFFFF;

var CCNB_DECODE_MAX_LEN = 32768;  // Stop decoding if field length is longer than a threshold

var BinaryXMLDecoder = function BinaryXMLDecoder(istream) {	
    this.istream = istream;  // Buffer
    this.offset = 0;
};

exports.CcnbDecoder = BinaryXMLDecoder;


BinaryXMLDecoder.prototype.readStartElement = function (startTag) {
    tv = this.decodeTypeAndVal();
    		
    if (null == tv) {
	throw new NoNError('DecodeError', "expected start element " + startTag + ", got something not a tag.");
    }
    
    var decodedTag = null;
			
    if (tv.type() == XML_TAG) {
	// Tag value represents length-1 as tags can never be empty.
	decodedTag = this.decodeUString(tv.val());
    } else if (tv.type() == XML_DTAG) {
	decodedTag = tv.val();
    }
    
    if ((null ==  decodedTag) || decodedTag != startTag ) {
	throw new NoNError('DecodeError', "expecting start element " + startTag + " but got " + decodedTag);
    }
};


BinaryXMLDecoder.prototype.peekStartElementAsString = function () {
    var decodedTag = null;
    var previousOffset = this.offset;
    try {
	// Have to distinguish genuine errors from wrong tags. Could either use
	// a special exception subtype, or redo the work here.
	//this.TypeAndVal 
	var tv = this.decodeTypeAndVal();

	if (null != tv) {
	    if (tv.type() == XML_TAG) {
		if (tv.val()+1 > CCNB_DECODE_MAX_LEN) {
		    throw new NoNError("DecodeError", "length " + (tv.val() + 1) + " longer than expected maximum length.");
		}
		decodedTag = this.decodeUString(tv.val());
	    } else if (tv.type() == XML_DTAG) {
		decodedTag = tagToString(tv.val());					
	    }
	} // else, not a type and val, probably an end element. rewind and return false.
    }
    finally {
	this.offset = previousOffset;
    }
    return decodedTag;
};

BinaryXMLDecoder.prototype.peekStartElement = function (startTag) {
    if (typeof startTag == 'string') {
	var decodedTag = this.peekStartElementAsString();
	
	if ((null !=  decodedTag) && decodedTag == startTag) {
	    return true;
	}
	return false;
    } else if (typeof startTag == 'number') {
	var decodedTag = this.peekStartElementAsLong();
	if ((null !=  decodedTag) && decodedTag == startTag) {
	    return true;
	}
	return false;
    } else {
	throw new NoNError('DecodeError', "should be string or number.");
    }
}


BinaryXMLDecoder.prototype.peekStartElementAsLong = function () {
    var decodedTag = null;
    var previousOffset = this.offset;
		
    try {
	// Have to distinguish genuine errors from wrong tags. Could either use
	// a special exception subtype, or redo the work here.
	//this.TypeAndVal
	var tv = this.decodeTypeAndVal();

	if (null != tv) {
	    if (tv.type() == XML_TAG) {
		if (tv.val()+1 > CCNB_DECODE_MAX_LEN) {
		    throw new NoNError('DecodeError', "length " + (tv.val() + 1) + " longer than expected maximum length!");
		}
		// Tag value represents length-1 as tags can never be empty.
		var strTag = this.decodeUString(tv.val());
		decodedTag = stringToTag(strTag);
	    } else if (tv.type() == XML_DTAG) {
		decodedTag = tv.val();		
	    }
	} // else, not a type and val, probably an end element. rewind and return false.
    }
    finally {
	this.offset = previousOffset;
    }
    return decodedTag;
};


BinaryXMLDecoder.prototype.readBinaryElement = function (startTag){
    var blob = null;
    this.readStartElement(startTag);
    blob = this.readBlob();
    return blob;
};


BinaryXMLDecoder.prototype.readEndElement = function () {
    var next = this.istream[this.offset]; 
    		
    this.offset++;
    
    if (next != XML_CLOSE) {
	throw new NoNError('DecodeError', "expected end element, got " + next);
    }
};


BinaryXMLDecoder.prototype.readUString = function () {
    var ustring = this.decodeUString();	
    this.readEndElement();
    return ustring;
};


BinaryXMLDecoder.prototype.readBlob = function () {
    var blob = this.decodeBlob();	
    this.readEndElement();
    return blob; // Buffer
};



BinaryXMLDecoder.prototype.readDateTime = function (startTag)  {
    var byteTimestamp = this.readBinaryElement(startTag);

    byteTimestamp = byteTimestamp.toString('hex');
    
    byteTimestamp = parseInt(byteTimestamp, 16);

    var lontimestamp = (byteTimestamp/ 4096) * 1000;

    //if(lontimestamp<0) lontimestamp =  - lontimestamp;

    if(LOG>4) console.log('DECODED DATE WITH VALUE');
    if(LOG>4) console.log(lontimestamp);

    var timestamp = new CCNTime(lontimestamp);
    //timestamp.setDateBinary(byteTimestamp);
    
    if (null == timestamp) {
	throw new NoNError('DecodeError', "cannot parse timestamp: " + byteTimestamp.toString('hex'));
    }		
    return timestamp;
};

BinaryXMLDecoder.prototype.decodeTypeAndVal = function() {
    var type = -1;
    var val = 0;
    var more = true;

    do {	
	var next = this.istream[this.offset];
		
	if (next < 0) {
	    return null; 
	}

	if ((0 == next) && (0 == val)) {
	    return null;
	}
		
	more = (0 == (next & XML_TT_NO_MORE));
		
	if  (more) {
	    val = val << XML_REG_VAL_BITS;
	    val |= (next & XML_REG_VAL_MASK);
	} else {
	    type = next & XML_TT_MASK;
	    val = val << XML_TT_VAL_BITS;
	    val |= ((next >>> XML_TT_BITS) & XML_TT_VAL_MASK);
	}
		
	this.offset++;
		
    } while (more);
	
    if(LOG>4) console.log('TYPE is '+ type + ' VAL is '+ val);

    return new TypeAndVal(type, val);
};


BinaryXMLDecoder.peekTypeAndVal = function() {
    var tv = null;    
    var previousOffset = this.offset;
    
    try {
	tv = this.decodeTypeAndVal();
    } finally {
	this.offset = previousOffset;
    }
    
    return tv;
};


BinaryXMLDecoder.prototype.decodeBlob = function (blobLength) {
    if (null == blobLength) {
	var tv = this.decodeTypeAndVal();
	blobLength = tv.val();
    }
    
    var bytes = this.istream.slice(this.offset, this.offset + blobLength);
    this.offset += blobLength;
    
    return bytes;
};


BinaryXMLDecoder.prototype.decodeUString = function (byteLength) {
    if (null == byteLength) {
	var tempStreamPosition = this.offset;
	
	var tv = this.decodeTypeAndVal();
	
	if ((null == tv) || (XML_UDATA != tv.type())) { // if we just have closers left, will get back null
	    this.offset = tempStreamPosition;
	    return "";
	}
			
	byteLength = tv.val();
    }

    var stringBytes = this.decodeBlob(byteLength);
    return stringBytes.toString();
};


//OBject containg a pair of type and value
var TypeAndVal = function TypeAndVal(_type, _val) {
    this.t = _type; // byte
    this.v = _val;  // byte
};

TypeAndVal.prototype.type = function () {
    return this.t;
};

TypeAndVal.prototype.val = function () {
    return this.v;
};


BinaryXMLDecoder.prototype.readIntegerElement = function (startTag) {
    if(LOG>4) console.log('READING INTEGER '+ startTag);
    if(LOG>4) console.log('TYPE OF '+ typeof startTag);
    
    var strVal = this.readUTF8Element(startTag);
    
    return parseInt(strVal);
};


BinaryXMLDecoder.prototype.readUTF8Element = function (startTag) {
    this.readStartElement(startTag);
    var strElementText = this.readUString();
    return strElementText;
};


/* 
 * Set the offset into the input, used for the next read.
 */
BinaryXMLDecoder.prototype.seek = function (offset) {
    this.offset = offset;
};
/**
 * This class uses BinaryXMLDecoder to follow the structure of a ccnb binary element to 
 * determine its end.
 * 
 * @author: Jeff Thompson
 * See COPYING for copyright and distribution information.
 */

var BinaryXMLStructureDecoder = function BinaryXMLDecoder() {
    this.gotElementEnd = false;
    this.offset = 0;
    this.level = 0;
    this.state = BinaryXMLStructureDecoder.READ_HEADER_OR_CLOSE;
    this.headerLength = 0;
    this.useHeaderBuffer = false;
    this.headerBuffer = new DynamicBuffer(5);
    this.nBytesToRead = 0;
};

BinaryXMLStructureDecoder.READ_HEADER_OR_CLOSE = 0;
BinaryXMLStructureDecoder.READ_BYTES = 1;

/*
 * Continue scanning input starting from this.offset.  If found the end of the element
 *   which started at offset 0 then return true, else false.
 * If this returns false, you should read more into input and call again.
 * You have to pass in input each time because the array could be reallocated.
 * This throws an exception for badly formed ccnb.
 */
BinaryXMLStructureDecoder.prototype.findElementEnd = function(input) {
    if (this.gotElementEnd)
        // Someone is calling when we already got the end.
        return true;
    
    var decoder = new BinaryXMLDecoder(input);
    
    while (true) {
        if (this.offset >= input.length)
            // All the cases assume we have some input.
            return false;
        
        switch (this.state) {
            case BinaryXMLStructureDecoder.READ_HEADER_OR_CLOSE:               
                // First check for XML_CLOSE.
                if (this.headerLength == 0 && input[this.offset] == XML_CLOSE) {
                    ++this.offset;
                    // Close the level.
                    --this.level;
                    if (this.level == 0)
                        // Finished.
                        return true;
                    if (this.level < 0)
                        throw new NoNError("BinaryXMLStructureDecoderError", "unexepected close tag at offset " + (this.offset - 1));
                    
                    // Get ready for the next header.
                    this.startHeader();
                    break;
                }
                
                var startingHeaderLength = this.headerLength;
                while (true) {
                    if (this.offset >= input.length) {
                        // We can't get all of the header bytes from this input. Save in headerBuffer.
                        this.useHeaderBuffer = true;
                        var nNewBytes = this.headerLength - startingHeaderLength;
                        this.headerBuffer.set
                            (input.subarray(this.offset - nNewBytes, nNewBytes), startingHeaderLength);
                        
                        return false;
                    }
                    var headerByte = input[this.offset++];
                    ++this.headerLength;
                    if (headerByte & XML_TT_NO_MORE)
                        // Break and read the header.
                        break;
                }
                
                var typeAndVal;
                if (this.useHeaderBuffer) {
                    // Copy the remaining bytes into headerBuffer.
                    nNewBytes = this.headerLength - startingHeaderLength;
                    this.headerBuffer.set
                        (input.subarray(this.offset - nNewBytes, nNewBytes), startingHeaderLength);

                    typeAndVal = new BinaryXMLDecoder(this.headerBuffer.array).decodeTypeAndVal();
                }
                else {
                    // We didn't have to use the headerBuffer.
                    decoder.seek(this.offset - this.headerLength);
                    typeAndVal = decoder.decodeTypeAndVal();
                }
                
                if (typeAndVal == null)
                    throw new NoNError("BinaryXMLStructureDecoderError", "can't read header starting at offset " + (this.offset - this.headerLength));
                
                // Set the next state based on the type.
                var type = typeAndVal.t;
                if (type == XML_DATTR)
                    // We already consumed the item. READ_HEADER_OR_CLOSE again.
                    // ccnb has rules about what must follow an attribute, but we are just scanning.
                    this.startHeader();
                else if (type == XML_DTAG || type == XML_EXT) {
                    // Start a new level and READ_HEADER_OR_CLOSE again.
                    ++this.level;
                    this.startHeader();
                }
                else if (type == XML_TAG || type == XML_ATTR) {
                    if (type == XML_TAG)
                        // Start a new level and read the tag.
                        ++this.level;
                    // Minimum tag or attribute length is 1.
                    this.nBytesToRead = typeAndVal.v + 1;
                    this.state = BinaryXMLStructureDecoder.READ_BYTES;
                    // ccnb has rules about what must follow an attribute, but we are just scanning.
                }
                else if (type == XML_BLOB || type == XML_UDATA) {
                    this.nBytesToRead = typeAndVal.v;
                    this.state = BinaryXMLStructureDecoder.READ_BYTES;
                }
                else
                    throw new NoNError("BinaryXMLStructureDecoderError", "unrecognized header type " + type);
                break;
            
            case BinaryXMLStructureDecoder.READ_BYTES:
                var nRemainingBytes = input.length - this.offset;
                if (nRemainingBytes < this.nBytesToRead) {
                    // Need more.
                    this.offset += nRemainingBytes;
                    this.nBytesToRead -= nRemainingBytes;
                    return false;
                }
                // Got the bytes.  Read a new header or close.
                this.offset += this.nBytesToRead;
                this.startHeader();
                break;
            
            default:
                // We don't expect this to happen.
                throw new NoNError("BinaryXMLStructureDecoderError", "unrecognized state " + this.state);
        }
    }
};

/*
 * Set the state to READ_HEADER_OR_CLOSE and set up to start reading the header
 */
BinaryXMLStructureDecoder.prototype.startHeader = function () {
    this.headerLength = 0;
    this.useHeaderBuffer = false;
    this.state = BinaryXMLStructureDecoder.READ_HEADER_OR_CLOSE;    
};

/*
 *  Set the offset into the input, used for the next read.
 */
BinaryXMLStructureDecoder.prototype.seek = function (offset) {
    this.offset = offset;
};
/**
 * @author: Meki Cheraoui, Jeff Thompson
 * See COPYING for copyright and distribution information.
 * This class represents a Name as an array of components where each is a byte array.
 */

/**
 * Ported to node.js by Wentao Shang
 */
 
/**
 * Create a new Name from _components.
 * If _components is a string, parse it as a URI.  Otherwise it is an array of components
 * where each is a string, byte array, ArrayBuffer or Uint8Array. 
 * Convert and store as an array of Uint8Array.
 * If a component is a string, encode as utf8.
 */
var Name = function Name(_components) {
    if (_components == null)
	this.components = [];
    else if (typeof _components == 'string') {
	this.components = Name.createComponentArray(_components);
    } else if (typeof _components == 'object') {
	this.components = [];
        if (_components instanceof Name)
            this.add(_components);
        else {
            for (var i = 0; i < _components.length; i++)
                this.add(_components[i]);
        }
    }
};

exports.Name = Name;

/**
 * Get the number of components in the Name object
 */
Name.prototype.size = function () {
    return this.components.length;
};

/**
 * Add a component at the end of the Name.
 * 'component' is a string, Array, Buffer or Name.
 * Convert to Buffer and add to this Name.
 * If a component is a string, encode as utf8.
 * Return 'this' Name object to allow chaining calls to add.
 */
Name.prototype.add = function (component) {
    var result;
    if(typeof component == 'string')
        result = Name.stringComponentToBuffer(component);
    else if(typeof component == 'object' && component instanceof Buffer)
        result = component;
    else if (typeof component == 'object' && component instanceof Name) {
        var components;
        if (component == this)
            // special case, when we need to create a copy
            components = this.components.slice(0, this.components.length);
        else
            components = component.components;
        
        for (var i = 0; i < components.length; ++i)
            this.components.push(new Buffer(components[i]));

        return this;
    } else 
	throw new NoNError('NameError', "cannot add Name element at index " + this.components.length + ": Invalid type");
    
    this.components.push(result);
    return this;
};

// Alias for Name.add()
Name.prototype.append = function (component) {
    return this.add(component);
};

Name.prototype.appendVersion = function () {
    var d = new Date();
    var time = d.getTime().toString(16);

    if (time.length % 2 == 1)
	time = '0' + time;

    time = 'fd' + time;
    var binTime = new Buffer(time, 'hex');
    return this.add(binTime);
};

Name.prototype.appendSegment = function (seg) {
    if (seg == null || seg == 0)
	return this.add(new Buffer('00', 'hex'));

    var segStr = seg.toString(16);

    if (segStr.length % 2 == 1)
	segStr = '0' + segStr;

    segStr = '00' + segStr;
    return this.add(new Buffer(segStr, 'hex'));
};

Name.prototype.appendKeyID = function (/*Key*/ key) {
    var cmd = 'c12e4d2e4b00';  // '%C1.M.K%00'
    var digest = key.publicKeyDigest.toString('hex');
    var keyID = cmd + digest;
    return this.add(new Buffer(keyID, 'hex'));
};


// Convert URI string to Buffer. Handles special characters such as '%00'
// new Buffer(component, 'ascii') will encode '%00' as 0x20 (space)
// new Buffer(component, 'utf8') will encode '%C1' as a two-byte unicode character
// Both won't help in this case
Name.stringComponentToBuffer = function (component) {
    var buf = new Buffer(component.length);  // at least this length
    var pos = 0;  // # of bytes encoded into the Buffer
    var i = 0;
    while (i < component.length) {
	if (component[i] == '%') {
	    var hex = component.substr(i+1, 2);
	    buf[pos] = parseInt(hex, 16);
	    i += 2;
	} else
	    buf[pos] = component.charCodeAt(i);

	i++;
	pos++;
    }
    return buf.slice(0, pos);
};

/**
 * Parse name as a URI string and return an array of Buffer components.
 */
Name.createComponentArray = function (name) {
    name = name.trim();
    if (name.length <= 0)
        return [];

    var iColon = name.indexOf(':');
    if (iColon >= 0) {
        // Make sure the colon came before a '/'.
        var iFirstSlash = name.indexOf('/');
        if (iFirstSlash < 0 || iColon < iFirstSlash)
            // Omit the leading protocol such as ndn:
            name = name.substr(iColon + 1, name.length - iColon - 1).trim();
    }
    
    if (name[0] == '/') {
        if (name.length >= 2 && name[1] == '/') {
            // Strip the authority following "//".
            var iAfterAuthority = name.indexOf('/', 2);
            if (iAfterAuthority < 0)
                // Unusual case: there was only an authority.
                return [];
            else
                name = name.substr(iAfterAuthority + 1, name.length - iAfterAuthority - 1).trim();
        }
        else
            name = name.substr(1, name.length - 1).trim();
    }

    var array = name.split('/');
    
    for (var i = 0; i < array.length; ++i) {
        var component = array[i].trim();
        
        if (component.match(/[^.]/) == null) {
            // Special case for component of only periods.  
            if (component.length <= 2) {
                // Zero, one or two periods is illegal.  Ignore this componenent to be
                //   consistent with the C implmentation.
                // This also gets rid of a trailing '/'.
                array.splice(i, 1);
                --i;  
                continue;
            }
            else
                // Remove 3 periods.
                array[i] = component.substr(3, component.length - 3);
        }
        else
            array[i] = component;
        
        // Change the component to Buffer now.
        array[i] = Name.stringComponentToBuffer(array[i]);
    }

    return array;
};


Name.prototype.from_ccnb = function(/*XMLDecoder*/ decoder) {
    if (LOG>4) console.log('--------Start decoding Name...');

    decoder.readStartElement(this.getElementLabel());
		
    this.components = new Array();

    while (decoder.peekStartElement(CCNProtocolDTags.Component)) {
	this.add(decoder.readBinaryElement(CCNProtocolDTags.Component));
    }
		
    decoder.readEndElement();

    if (LOG>4) console.log('--------Finish decoding Name.');
};

Name.prototype.to_ccnb = function(/*XMLEncoder*/ encoder) {
    if (LOG>4) console.log('--------Encoding Name...');

    if (this.components == null)
	throw new NoNError('NameError', "cannot encode empty content name");

    encoder.writeStartElement(this.getElementLabel());
    var count = this.components.length;
    for (var i = 0; i < count; i++) {
	encoder.writeElement(CCNProtocolDTags.Component, this.components[i]);
    }
    encoder.writeEndElement();

    if (LOG>4) console.log('--------Finish encoding Name.');
};

Name.prototype.encodeToBinary = function () {
    var enc = new BinaryXMLEncoder();
    this.to_ccnb(enc);
    return enc.getReducedOstream();
};

Name.prototype.getElementLabel = function(){
    return CCNProtocolDTags.Name;
};


/**
 * Return component as an escaped string according to "CCNx URI Scheme".
 * We can't use encodeURIComponent because that doesn't encode all the characters we want to.
 */
Name.toEscapedString = function(component) {
    var result = "";
    var gotNonDot = false;
    for (var i = 0; i < component.length; ++i) {
        if (component[i] != 0x2e) {
            gotNonDot = true;
            break;
        }
    }
    if (!gotNonDot) {
        // Special case for component of zero or more periods.  Add 3 periods.
        result = "...";
        for (var i = 0; i < component.length; ++i)
            result += ".";
    }
    else {
        for (var i = 0; i < component.length; ++i) {
            var value = component[i];
            // Check for 0-9, A-Z, a-z, (+), (-), (.), (_)
            if (value >= 0x30 && value <= 0x39 || value >= 0x41 && value <= 0x5a ||
                value >= 0x61 && value <= 0x7a || value == 0x2b || value == 0x2d || 
                value == 0x2e || value == 0x5f)
                result += String.fromCharCode(value);
            else
                result += "%" + (value < 16 ? "0" : "") + value.toString(16).toUpperCase();
        }
    }
    return result;
};

// Return the escaped name string according to "CCNx URI Scheme".
Name.prototype.to_uri = function () {
    if (this.components.length == 0)
        return "/";
    
    var result = "";
	
    for(var i = 0; i < this.components.length; ++i)
	result += "/"+ Name.toEscapedString(this.components[i]);
    
    return result;
};


Name.is_text_encodable = function (/*Buffer*/ blob) {
    if (blob.length == 0) return false;

    for (var i = 0; i < blob.length; i++) {
	var c = blob[i];
	if (c < 0x20 || c > 0x7E) return false;
	if (c == 0x3C || c == 0x3E || c == 0x26) return false;
    }
    return true;
};

/**
 * Return a string of XML representation of the Name object
 * encoding can be 'hex' or 'base64'. if null, will use 'hex' by default
 */
Name.prototype.to_xml = function (encoding) {
    var xml = '<Name>';

    if (encoding == null)
	encoding = 'hex';

    for(var i = 0; i < this.components.length; i++) {
	var blob = this.components[i];
	if (Name.is_text_encodable(blob))
	    xml += '<Component ccnbencoding="text">' + blob.toString() + '</Component>';
	else 
	    xml += '<Component ccnbencoding="' + encoding + 'Binary">' + blob.toString(encoding).toUpperCase() + '</Component>';
    }
    xml += '</Name>';
    return xml;
};

/**
 * Return a new Name with the first nComponents components of this Name.
 */
Name.prototype.getPrefix = function (nComponents) {
    return new Name(this.components.slice(0, nComponents));
};

/**
 * Return a new Name with the suffix starting at the p-th component of this Name.
 */
Name.prototype.getSuffix = function (p) {
    return new Name(this.components.slice(p));
};

/**
 * Return a new Buffer of the component at i.
 */
Name.prototype.getComponent = function (i) {
    var result = new Buffer(this.components[i].length);
    this.components[i].copy(result);
    return result;
};


/**
 * Return true if this Name has the same components as name.
 */
Name.prototype.equalsName = function(name) {
    if (this.components.length != name.components.length)
        return false;
    
    // Start from the last component because they are more likely to differ.
    for (var i = this.components.length - 1; i >= 0; --i) {
        if (!DataUtils.arraysEqual(this.components[i], name.components[i]))
            return false;
    }
    
    return true;
};


/**
 * Returns true if 'this' is a prefix of 'name'
 */
Name.prototype.match = function (name) {
    var i_name = this.components;
    var o_name = name.components;

    // The intrest name is longer than the name we are checking it against.
    if (i_name.length > o_name.length)
	return false;

    // Check if at least one of given components doesn't match.
    for (var i = 0; i < i_name.length; ++i) {
        if (!DataUtils.arraysEqual(i_name[i], o_name[i]))
            return false;
    }

    return true;
};
/**
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 * This class represents ContentObject Objects
 */

/**
 * Ported to node.js by Wentao Shang
 */

/**
 * When creating ContentObject in JS code, _name and _content must be specified while _signedinfo is optional.
 */
var ContentObject = function ContentObject(_name, _content, _signedInfo) {	
    if (_name == null) {
	this.name = null;
    } else if (typeof _name == 'string') {
	this.name = new Name(_name);
    } else if (_name instanceof Name) {
	this.name = _name;
    } else
	throw new NoNError('ContentObjectError', 'unknown name type ' + _name.constructor.name);
    
    if (_content == null) {
	this.content = null;
    } else if (typeof _content == 'string') {
	this.content = new Buffer(_content);
    } else if (_content instanceof Buffer) {
	this.content = _content;
    } else
	throw new NoNError('ContentObjectError', 'unknown content type ' + _content.constructor.name);
    
    this.signedInfo = _signedInfo;  // may be null

    this.signature = null;

    this.startSIG = null;
    this.endSIG = null;
    this.signedData = null;
};

exports.ContentObject = ContentObject;

/**
 * The 'key' parameter is a mandatory Key object
 * The optional 'param' is a JSON object that may contain the following fields:
 * { keyName: the name of the 'key'; if this field is present, the KeyLocator of this ContentObject will be filled with KeyName rather than Key bits
 *   contentType: the type of the ContentObject; if this filed is null, the default ContentType.DATA is used
 *   freshness: a number in seconds
 *   finalBlockID: a integer indicating the segment number of the final block
 * }
 * This 'param' is a high-level interface to set SignedInfo fields through ContentObject methods
 */
ContentObject.prototype.sign = function (key, param) {
    if (key == null || key.privateKeyPem == null) {
	throw new NoNError('ConteotObjectError', "cannot sign data without a private key.");
    }

    // Set SignedInfo if it's empty
    if (this.signedInfo == null) {
	this.signedInfo = new SignedInfo();
	this.signedInfo.setFields(key, param);
    }

    var n1 = this.encodeObject(this.name);
    var n2 = this.encodeObject(this.signedInfo);
    var n3 = this.encodeContent();

    var rsa = require("crypto").createSign('RSA-SHA256');
    rsa.update(n1);
    rsa.update(n2);
    rsa.update(n3);
    
    var sig = new Buffer(rsa.sign(key.privateKeyPem));

    this.signature = new Signature();
    this.signature.signature = sig;
};

ContentObject.prototype.verify = function (/*Key*/ key) {
    if (key == null || key.publicKeyPem == null) {
	throw new NoNError('ConteotObjectError', "cannot verify data without a public key.");
    }

    var verifier = require('crypto').createVerify('RSA-SHA256');
    verifier.update(this.signedData);
    return verifier.verify(key.publicKeyPem, this.signature.signature);
};

ContentObject.prototype.encodeObject = function encodeObject(obj) {
    var enc = new BinaryXMLEncoder(); 
    obj.to_ccnb(enc);
    return enc.getReducedOstream();
};

ContentObject.prototype.encodeContent = function encodeContent(obj) {
    var enc = new BinaryXMLEncoder();
    enc.writeElement(CCNProtocolDTags.Content, this.content);
    return enc.getReducedOstream();	
};

ContentObject.prototype.saveSignedData = function (bytes) {
    var sig = bytes.slice(this.startSIG, this.endSIG);
    this.signedData = sig;
};

ContentObject.prototype.from_ccnb = function (decoder) {
    if (LOG > 4) console.log('--------Start decoding ContentObject...');

    decoder.readStartElement(this.getElementLabel());

    if (decoder.peekStartElement(CCNProtocolDTags.Signature)) {
	this.signature = new Signature();
	this.signature.from_ccnb(decoder);
    }

    this.startSIG = decoder.offset;

    this.name = new Name();
    this.name.from_ccnb(decoder);
    
    if (decoder.peekStartElement(CCNProtocolDTags.SignedInfo)) {
	this.signedInfo = new SignedInfo();
	this.signedInfo.from_ccnb(decoder);
    }
    
    this.content = decoder.readBinaryElement(CCNProtocolDTags.Content);

    this.endSIG = decoder.offset;
    	
    decoder.readEndElement();
    
    this.saveSignedData(decoder.istream);

    if (LOG > 4) console.log('--------Finish decoding ContentObject.');
};

ContentObject.prototype.to_ccnb = function (encoder) {
    if (LOG > 4) console.log('--------Encoding ContentObject...');

    encoder.writeStartElement(this.getElementLabel());

    if (null != this.signature)
	this.signature.to_ccnb(encoder);
    
    this.startSIG = encoder.offset;
    
    if (null != this.name)
	this.name.to_ccnb(encoder);
    
    if (null != this.signedInfo)
	this.signedInfo.to_ccnb(encoder);

    encoder.writeElement(CCNProtocolDTags.Content, this.content);

    this.endSIG = encoder.offset;
    
    encoder.writeEndElement();
    
    //this.saveSignedData(encoder.ostream);

    if (LOG > 4) console.log('--------Finish encoding ContentObject');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
ContentObject.prototype.to_xml = function (encoding) {
    var xml = '<ContentObject>';

    if (encoding == null)
	encoding = 'hex';  // default choice

    if (null != this.signature)
	xml += this.signature.to_xml(encoding);
    
    if (null != this.name)
	xml += this.name.to_xml(encoding);
    
    if (null != this.signedInfo)
	xml += this.signedInfo.to_xml(encoding);

    xml += '<Content ccnbencoding="' + encoding + 'Binary">' + this.content.toString(encoding).toUpperCase() + '</Content>';

    xml += '</ContentObject>';
    return xml;
};


ContentObject.prototype.encodeToBinary = function () {
    var enc = new BinaryXMLEncoder();
    this.to_ccnb(enc);
    return enc.getReducedOstream();
};

/**
 * Static method to parse a Buffer containing ccnb-formated ContentObject bytes.
 * Return a parsed ContentObject object.
 */
ContentObject.parse = function (buf) {
    var dec = new BinaryXMLDecoder(buf);
    var co = new ContentObject();
    co.from_ccnb(dec);
    return co;
};

ContentObject.prototype.getElementLabel = function () { return CCNProtocolDTags.ContentObject; };

/**
 * Signature
 */
var Signature = function Signature(_witness, _signature, _digestAlgorithm) {
    this.Witness = _witness;//byte [] _witness;
    this.signature = _signature;//byte [] _signature;
    this.digestAlgorithm = _digestAlgorithm//String _digestAlgorithm;
};

exports.Signature = Signature;

Signature.prototype.from_ccnb = function (decoder) {
    if(LOG>4) console.log('--------Start decoding Signature...');

    decoder.readStartElement(this.getElementLabel());	
		
    if (decoder.peekStartElement(CCNProtocolDTags.DigestAlgorithm)) {
	if(LOG>4)console.log('DIGIEST ALGORITHM FOUND');
	this.digestAlgorithm = decoder.readUTF8Element(CCNProtocolDTags.DigestAlgorithm); 
    }
    if (decoder.peekStartElement(CCNProtocolDTags.Witness)) {
	if(LOG>4)console.log('WITNESS FOUND');
	this.Witness = decoder.readBinaryElement(CCNProtocolDTags.Witness); 
    }
		
    //FORCE TO READ A SIGNATURE

    if(LOG>4)console.log('SIGNATURE FOUND');
    this.signature = decoder.readBinaryElement(CCNProtocolDTags.SignatureBits);

    decoder.readEndElement();

    if(LOG>4) console.log('--------Finish decoding Signature.');
};


Signature.prototype.to_ccnb = function (encoder) {
    if (LOG > 4) console.log('--------Encoding Signature...');

    if (!this.validate()) {
	throw new Error("Cannot encode: field values missing.");
    }
	
    encoder.writeStartElement(this.getElementLabel());

    if ((null != this.digestAlgorithm) && (!this.digestAlgorithm.equals(CCNDigestHelper.DEFAULT_DIGEST_ALGORITHM))) {
	encoder.writeElement(CCNProtocolDTags.DigestAlgorithm, OIDLookup.getDigestOID(this.DigestAlgorithm));
    }
	
    if (null != this.Witness) {
	// needs to handle null witness
	encoder.writeElement(CCNProtocolDTags.Witness, this.Witness);
    }

    encoder.writeElement(CCNProtocolDTags.SignatureBits, this.signature);

    encoder.writeEndElement();

    if (LOG > 4) console.log('--------Finish encoding Signature.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
Signature.prototype.to_xml = function (encoding) {
    var xml = '<Signature>';
    if (encoding == null)
	encoding = 'hex';
    // Currently we only encode signature bits
    if (this.signature != null)
	xml += '<SignatureBits ccnbencoding="' + encoding + 'Binary">' + this.signature.toString(encoding).toUpperCase() + '</SignatureBits>';
    xml += '</Signature>';
    return xml;
};

Signature.prototype.getElementLabel = function() { return CCNProtocolDTags.Signature; };

Signature.prototype.validate = function() {
    return null != this.signature;
};


/**
 * SignedInfo
 */

// ContentType blob represented as base64 string
var ContentType = {DATA:'DATA', ENCR:'ENCR', GONE:'GONE', KEY:'KEY/', LINK:'LINK', NACK:'NACK'};

exports.ContentType = ContentType;

var SignedInfo = function SignedInfo(_publisher, _timestamp, _type, _locator, _freshnessSeconds, _finalBlockID) {
    this.publisher = _publisher; //publisherPublicKeyDigest
    this.timestamp = _timestamp; // CCN Time
    this.type = _type; // ContentType
    this.locator = _locator; //KeyLocator
    this.freshnessSeconds = _freshnessSeconds; // Integer
    this.finalBlockID = _finalBlockID; //byte array
};

exports.SignedInfo = SignedInfo;

/**
 * Initialize SignedInfo with a Key and a Signing Paramter object
 * 
 * The mandatory 'key' paramter is the signing Key object
 * The optional 'param' is a JSON object that may contain the following fields:
 * { keyName: the name of the 'key'; if this field is present, the KeyLocator of this ContentObject will be filled with KeyName rather than Key bits
 *   contentType: the type of the ContentObject; if this filed is null, the default ContentType.DATA is used
 *   freshness: a number in seconds
 *   finalBlockID: a integer indicating the segment number of the final block
 * }
 */
SignedInfo.prototype.setFields = function (key, param) {
    if (key == null)
	throw new NoNError('SignedInfoError', 'cannot set SignedInfo without key info.');

    this.publisher = new PublisherPublicKeyDigest(key.getKeyID());
    
    this.timestamp = new CCNTime();

    if (param == null) {
	this.type = ContentType.DATA;
	this.locator = new KeyLocator(key, KeyLocatorType.KEY);
    } else {
	if (param.contentType == null)
	    this.type = ContentType.DATA;  // default
	else
	    this.type = param.contentType;

	if (param.keyName == null)
	    this.locator = new KeyLocator(key, KeyLocatorType.KEY);
	else 
	    this.locator = new KeyLocator(param.keyName, KeyLocatorType.KEYNAME);

	this.freshnessSeconds = param.freshness;  // may be null
	this.finalBlockID = param.finalBlockID;   // may be null
    }
};

SignedInfo.prototype.from_ccnb = function (decoder) {
    if (LOG > 4) console.log('--------Start decoding SignedInfo...');

    decoder.readStartElement( this.getElementLabel() );
		
    if (decoder.peekStartElement(CCNProtocolDTags.PublisherPublicKeyDigest)) {
	if(LOG>4)console.log('DECODING PUBLISHER KEY');
	this.publisher = new PublisherPublicKeyDigest();
	this.publisher.from_ccnb(decoder);
    }

    if (decoder.peekStartElement(CCNProtocolDTags.Timestamp)) {
	if(LOG>4)console.log('DECODING TIMESTAMP');
	this.timestamp = decoder.readDateTime(CCNProtocolDTags.Timestamp);
    }

    if (decoder.peekStartElement(CCNProtocolDTags.Type)) {
	binType = decoder.readBinaryElement(CCNProtocolDTags.Type);

	this.type = binType.toString('base64');

	if(LOG>4)console.log('ContentType in SignedInfo is '+ this.type);
    } else {
	this.type = ContentType.DATA; // default
    }
    
    if (decoder.peekStartElement(CCNProtocolDTags.FreshnessSeconds)) {
	this.freshnessSeconds = decoder.readIntegerElement(CCNProtocolDTags.FreshnessSeconds);
	if(LOG>4)console.log('FRESHNESS IN SECONDS IS '+ this.freshnessSeconds);
    }
		
    if (decoder.peekStartElement(CCNProtocolDTags.FinalBlockID)) {
	if(LOG>4)console.log('DECODING FINAL BLOCKID');
	this.finalBlockID = decoder.readBinaryElement(CCNProtocolDTags.FinalBlockID);
    }
    
    if (decoder.peekStartElement(CCNProtocolDTags.KeyLocator)) {
	if(LOG>4)console.log('DECODING KEY LOCATOR');
	this.locator = new KeyLocator();
	this.locator.from_ccnb(decoder);
    }
    
    decoder.readEndElement();

    if (LOG > 4) console.log('--------Finish decoding SignedInfo.');
};

SignedInfo.prototype.to_ccnb = function (encoder) {
    if (LOG > 4) console.log('--------Encoding SignedInfo...');

    if (!this.validate()) {
	throw new NoNError('SignedInfoError', "cannot encode because field values missing.");
    }
    
    encoder.writeStartElement(this.getElementLabel());
    	
    if (null != this.publisher) {
	this.publisher.to_ccnb(encoder);
    }
    
    if (null != this.timestamp) {
	encoder.writeDateTime(CCNProtocolDTags.Timestamp, this.timestamp);
    }

    if (null != this.type && this.type != ContentType.DATA) {
	encoder.writeElement(CCNProtocolDTags.Type, new Buffer(this.type, 'base64'));
    }

    if (null != this.freshnessSeconds) {
	encoder.writeElement(CCNProtocolDTags.FreshnessSeconds, this.freshnessSeconds);
    }

    if (null != this.finalBlockID) {
	encoder.writeElement(CCNProtocolDTags.FinalBlockID, this.finalBlockID);
    }

    if (null != this.locator) {
	this.locator.to_ccnb(encoder);
    }

    encoder.writeEndElement();

    if (LOG > 4) console.log('--------Finish encoding SignedInfo.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
SignedInfo.prototype.to_xml = function (encoding) {
    var xml = '<SignedInfo>';

    if (encoding == null)
	encoding = 'hex';

    if (null != this.publisher)
	xml += this.publisher.to_xml(encoding);

    if (null != this.timestamp)
	xml += '<Timestamp ccnbencoding="' + encoding + 'Binary">' 
	    + this.timestamp.encodeToBinary().toString(encoding).toUpperCase()
	    + '</Timestamp>';

    if (null != this.type && this.type != ContentType.DATA)
	// Use base64 encoding for ContentType regardless of 'encoding' parameter
	xml += '<Type ccnbencoding="base64Binary"' + this.type + '</Type>';
    
    if (null != this.freshnessSeconds)
	xml += '<FreshnessSeconds>' + this.freshnessSeconds + '</FreshnessSeconds>';

    if (null != this.finalBlockID)
	xml += '<FinalBlockID ccnbencoding="' + encoding + 'Binary">' + this.finalBlockID.toString(encoding).toUpperCase() + '</FinalBlockID>'

    if (null != this.locator)
	xml += this.locator.to_xml(encoding);

    xml += '</SignedInfo>';
    return xml;
};

SignedInfo.prototype.getElementLabel = function () { 
    return CCNProtocolDTags.SignedInfo;
};


SignedInfo.prototype.validate = function () {
    // We don't do partial matches any more, even though encoder/decoder
    // is still pretty generous.
    if (null == this.publisher || null == this.timestamp || null == this.locator)
	return false;
    else
	return true;
};
/**
 * @author: Meki Cheraoui, Jeff Thompson
 * See COPYING for copyright and distribution information.
 * This class represents Interest Objects
 */

/**
 * Ported to node.js by Wentao Shang
 */

// _interestLifetime is in milliseconds.
var Interest = function Interest
   (_name, _faceInstance, _minSuffixComponents, _maxSuffixComponents, _publisherPublicKeyDigest, _exclude, 
    _childSelector, _answerOriginKind, _scope, _interestLifetime, _nonce) {		
    this.name = _name;
    this.faceInstance = _faceInstance;
    this.maxSuffixComponents = _maxSuffixComponents;
    this.minSuffixComponents = _minSuffixComponents;
	
    this.publisherPublicKeyDigest = _publisherPublicKeyDigest;
    this.exclude = _exclude;
    this.childSelector = _childSelector;
    this.answerOriginKind = _answerOriginKind;
    this.scope = _scope;
    this.interestLifetime = _interestLifetime;  // milli seconds
    this.nonce = _nonce;
};

exports.Interest = Interest;

Interest.RECURSIVE_POSTFIX = "*";

Interest.CHILD_SELECTOR_LEFT = 0;
Interest.CHILD_SELECTOR_RIGHT = 1;

Interest.ANSWER_NO_CONTENT_STORE = 0;
Interest.ANSWER_CONTENT_STORE = 1;
Interest.ANSWER_GENERATED = 2;
Interest.ANSWER_STALE = 4;		// Stale answer OK
Interest.MARK_STALE = 16;		// Must have scope 0.  Michael calls this a "hack"

Interest.DEFAULT_ANSWER_ORIGIN_KIND = Interest.ANSWER_CONTENT_STORE | Interest.ANSWER_GENERATED;


Interest.prototype.from_ccnb = function (/*XMLDecoder*/ decoder) {
    if (LOG>4) console.log('--------Start decoding Interest...');
    decoder.readStartElement(CCNProtocolDTags.Interest);

    this.name = new Name();
    this.name.from_ccnb(decoder);

    if (decoder.peekStartElement(CCNProtocolDTags.MinSuffixComponents))
	this.minSuffixComponents = decoder.readIntegerElement(CCNProtocolDTags.MinSuffixComponents);

    if (decoder.peekStartElement(CCNProtocolDTags.MaxSuffixComponents)) 
	this.maxSuffixComponents = decoder.readIntegerElement(CCNProtocolDTags.MaxSuffixComponents);
			
    if (decoder.peekStartElement(CCNProtocolDTags.PublisherPublicKeyDigest)) {
	this.publisherPublicKeyDigest = new PublisherPublicKeyDigest();
	this.publisherPublicKeyDigest.from_ccnb(decoder);
    }

    if (decoder.peekStartElement(CCNProtocolDTags.Exclude)) {
	this.exclude = new Exclude();
	this.exclude.from_ccnb(decoder);
    }
		
    if (decoder.peekStartElement(CCNProtocolDTags.ChildSelector))
	this.childSelector = decoder.readIntegerElement(CCNProtocolDTags.ChildSelector);
		
    if (decoder.peekStartElement(CCNProtocolDTags.AnswerOriginKind))
	this.answerOriginKind = decoder.readIntegerElement(CCNProtocolDTags.AnswerOriginKind);
		
    if (decoder.peekStartElement(CCNProtocolDTags.Scope))
	this.scope = decoder.readIntegerElement(CCNProtocolDTags.Scope);

    if (decoder.peekStartElement(CCNProtocolDTags.InterestLifetime))
	this.interestLifetime = 1000.0 * DataUtils.bigEndianToUnsignedInt
	    (decoder.readBinaryElement(CCNProtocolDTags.InterestLifetime)) / 4096;
		
    if (decoder.peekStartElement(CCNProtocolDTags.Nonce))
	this.nonce = decoder.readBinaryElement(CCNProtocolDTags.Nonce);
		
    decoder.readEndElement();

    if (LOG>4) console.log('--------Finish decoding Interest.');
};

Interest.prototype.to_ccnb = function (/*XMLEncoder*/ encoder) {
    if (LOG>4) console.log('--------Encoding Interest....');
    
    encoder.writeStartElement(CCNProtocolDTags.Interest);
    
    if (null != this.name)
	this.name.to_ccnb(encoder);
    
    if (null != this.minSuffixComponents) 
	encoder.writeElement(CCNProtocolDTags.MinSuffixComponents, this.minSuffixComponents);	
    
    if (null != this.maxSuffixComponents) 
	encoder.writeElement(CCNProtocolDTags.MaxSuffixComponents, this.maxSuffixComponents);
    
    if (null != this.publisherPublicKeyDigest)
	this.publisherPublicKeyDigest.to_ccnb(encoder);
    
    if (null != this.exclude)
	this.exclude.to_ccnb(encoder);
    
    if (null != this.childSelector) 
	encoder.writeElement(CCNProtocolDTags.ChildSelector, this.childSelector);
    
    if (this.DEFAULT_ANSWER_ORIGIN_KIND != this.answerOriginKind && this.answerOriginKind!=null) 
	encoder.writeElement(CCNProtocolDTags.AnswerOriginKind, this.answerOriginKind);
    
    if (null != this.scope) 
	encoder.writeElement(CCNProtocolDTags.Scope, this.scope);
    
    if (null != this.interestLifetime) 
	encoder.writeElement(CCNProtocolDTags.InterestLifetime, 
			     DataUtils.unsignedIntToBigEndian((this.interestLifetime / 1000.0) * 4096));
    
    if (null != this.nonce)
	encoder.writeElement(CCNProtocolDTags.Nonce, this.nonce);
    
    encoder.writeEndElement();
    
    if (LOG>4) console.log('--------Finish encoding Interest.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
Interest.prototype.to_xml = function (encoding) {
    var xml = '<Interest>';

    if (encoding == null)
	encoding = 'hex';

    if (null != this.name)
	xml += this.name.to_xml(encoding);
	
    if (null != this.minSuffixComponents) 
	xml += '<MinSuffixComponents>' + this.minSuffixComponents + '</MinSuffixComponents>';

    if (null != this.maxSuffixComponents) 
	xml += '<MaxSuffixComponents>' + this.maxSuffixComponents + '</MaxSuffixComponents>';

    if (null != this.publisherPublicKeyDigest)
	xml += this.publisherPublicKeyDigest.to_xml(encoding);
		
    if (null != this.exclude)
	xml += this.exclude.to_xml(encoding);
		
    if (null != this.childSelector)
	xml += '<ChildSelector>' + this.childSelector + '</ChildSelector>';

    if (this.DEFAULT_ANSWER_ORIGIN_KIND != this.answerOriginKind && this.answerOriginKind!=null) 
	xml += '<AnswerOriginKind>' + this.answerOriginKind + '</AnswerOriginKind>';
		
    if (null != this.scope) 
	xml += '<Scope>' + this.scope + '</Scope>';
		
    if (null != this.interestLifetime)
	xml += '<InterestLifetime ccnbencoding="' + encoding + 'Binary">' 
	    + DataUtils.unsignedIntToBigEndian((this.interestLifetime / 1000.0) * 4096).toString(encoding).toUpperCase()
	    + '</InterestLifetime>';
    
    if (null != this.nonce)
	xml += '<Nonce ccnbencoding="' + encoding + 'Binary">' + this.nonce.toString(encoding).toUpperCase() + '</Nonce>';
    
    xml += '</Interest>';

    return xml;
};


/*
 * Return encoded Buffer containing the ccnd-formated interest packet
 */
Interest.prototype.encodeToBinary = function () {
    var enc = new BinaryXMLEncoder();
    this.to_ccnb(enc);
    return enc.getReducedOstream();
};


/*
 * Return true if this.name.match(name) and the name conforms to the interest selectors.
 */
Interest.prototype.matches_name = function (/*Name*/ name) {
    if (!this.name.match(name))
        return false;
    
    if (this.minSuffixComponents != null &&
        // Add 1 for the implicit digest.
        !(name.components.length + 1 - this.name.components.length >= this.minSuffixComponents))
        return false;
    if (this.maxSuffixComponents != null &&
        // Add 1 for the implicit digest.
        !(name.components.length + 1 - this.name.components.length <= this.maxSuffixComponents))
        return false;
    if (this.exclude != null && name.components.length > this.name.components.length &&
        this.exclude.matches(name.components[this.name.components.length]))
        return false;
    
    return true;
};

/*
 * Return a new Interest with the same fields as this Interest.  
 * Note: This does NOT make a deep clone of the name, exclue or other objects.
 */
Interest.prototype.clone = function () {
    return new Interest(
	this.name, this.faceInstance, this.minSuffixComponents, this.maxSuffixComponents, 
	this.publisherPublicKeyDigest, this.exclude, this.childSelector, this.answerOriginKind, 
	this.scope, this.interestLifetime, this.nonce);
};

/*
 * Handle the interest Exclude element.
 * _values is an array where each element is either Uint8Array component or Exclude.ANY.
 */
var Exclude = function Exclude(_values) { 
    this.values = (_values || []);
    
    // Check the type of the input
    for (var i = 0; i < this.values.length; i++) {
	var component = this.values[i];
	if (typeof component == 'string')
	    // Convert string to Buffer
	    this.values[i] = Name.stringComponentToBuffer(component);
	else if (!(component instanceof Buffer))
	    // Otherwise it should be a Buffer
	    throw new NoNError('ExcludeError', 'unknown type of input component.');
    }
};

exports.Exclude = Exclude;

Exclude.ANY = "*";

Exclude.prototype.from_ccnb = function (/*XMLDecoder*/ decoder) {
    decoder.readStartElement(CCNProtocolDTags.Exclude);

    while (true) {
        if (decoder.peekStartElement(CCNProtocolDTags.Component))
            this.values.push(decoder.readBinaryElement(CCNProtocolDTags.Component));
        else if (decoder.peekStartElement(CCNProtocolDTags.Any)) {
            decoder.readStartElement(CCNProtocolDTags.Any);
            decoder.readEndElement();
            this.values.push(Exclude.ANY);
        }
        else if (decoder.peekStartElement(CCNProtocolDTags.Bloom)) {
            // Skip the Bloom and treat it as Any.
            decoder.readBinaryElement(CCNProtocolDTags.Bloom);
            this.values.push(Exclude.ANY);
        }
        else
            break;
    }
    
    decoder.readEndElement();
};

Exclude.prototype.to_ccnb = function (/*XMLEncoder*/ encoder) {
    if (this.values == null || this.values.length == 0)
	return;

    encoder.writeStartElement(CCNProtocolDTags.Exclude);
    
    // TODO: Do we want to order the components (except for ANY)?
    for (var i = 0; i < this.values.length; ++i) {
        if (this.values[i] == Exclude.ANY) {
            encoder.writeStartElement(CCNProtocolDTags.Any);
            encoder.writeEndElement();
        }
        else
            encoder.writeElement(CCNProtocolDTags.Component, this.values[i]);
    }

    encoder.writeEndElement();
};

Exclude.prototype.to_xml = function (encoding) {
    if (this.values == null || this.values.length == 0)
	return "";

    if (encoding == null)
	encoding = 'hex';
    
    var xml = '<Exclude>';
    
    // TODO: Do we want to order the components (except for ANY)?
    for (var i = 0; i < this.values.length; ++i) {
        if (this.values[i] == Exclude.ANY) {
            xml += '<Any></Any>';
        }
        else {
	    var blob = this.values[i];
	    if (Name.is_text_encodable(blob))
		xml += '<Component ccnbencoding="text">' + blob.toString() + '</Component>';
	    else 
		xml += '<Component ccnbencoding="' + encoding + 'Binary">' + blob.toString(encoding).toUpperCase() + '</Component>';
	}
    }

    xml += '</Exclude>';

    return xml;
};


/*
 * Return a string with elements separated by "," and Exclude.ANY shown as "*". 
 */
Exclude.prototype.to_uri = function () {
    if (this.values == null || this.values.length == 0)
	return "";

    var result = "";
    for (var i = 0; i < this.values.length; ++i) {
        if (i > 0)
            result += ",";
        
        if (this.values[i] == Exclude.ANY)
            result += "*";
        else
            result += Name.toEscapedString(this.values[i]);
    }
    return result;
};

/*
 * Return true if the component matches any of the exclude criteria.
 */
Exclude.prototype.matches = function (/*Buffer*/ component) {
    for (var i = 0; i < this.values.length; ++i) {
        if (this.values[i] == Exclude.ANY) {
            var lowerBound = null;
            if (i > 0)
                lowerBound = this.values[i - 1];
            
            // Find the upper bound, possibly skipping over multiple ANY in a row.
            var iUpperBound;
            var upperBound = null;
            for (iUpperBound = i + 1; iUpperBound < this.values.length; ++iUpperBound) {
                if (this.values[iUpperBound] != Exclude.ANY) {
                    upperBound = this.values[iUpperBound];
                    break;
                }
            }
            
            // If lowerBound != null, we already checked component equals lowerBound on the last pass.
            // If upperBound != null, we will check component equals upperBound on the next pass.
            if (upperBound != null) {
                if (lowerBound != null) {
                    if (Exclude.compareComponents(component, lowerBound) > 0 &&
                        Exclude.compareComponents(component, upperBound) < 0)
                        return true;
                }
                else {
                    if (Exclude.compareComponents(component, upperBound) < 0)
                        return true;
                }
                
                // Make i equal iUpperBound on the next pass.
                i = iUpperBound - 1;
            }
            else {
                if (lowerBound != null) {
                    if (Exclude.compareComponents(component, lowerBound) > 0)
                        return true;
                }
                else
                    // this.values has only ANY.
                    return true;
            }
        }
        else {
            if (DataUtils.arraysEqual(component, this.values[i]))
                return true;
        }
    }
    
    return false;
};

/**
 * Compare two Buffer components.
 * Return -1 if component1 is less than component2, 1 if greater or 0 if equal.
 * A component is less if it is shorter, otherwise if equal length do a byte comparison.
 */
Exclude.compareComponents = function (component1, component2) {
    if (component1.length < component2.length)
        return -1;
    if (component1.length > component2.length)
        return 1;
    
    for (var i = 0; i < component1.length; ++i) {
        if (component1[i] < component2[i])
            return -1;
        if (component1[i] > component2[i])
            return 1;
    }

    return 0;
};
/**
 * @author: Meki Cheraoui, Wentao Shang
 * See COPYING for copyright and distribution information.
 * This class represents Key and KeyLocator Objects
 */

/**
 * Ported to node.js by Wentao Shang
 */

/**
 * Key
 */
var Key = function Key() {
    this.publicKeyDer = null;     // Buffer
    this.publicKeyDigest = null;  // Buffer
    this.publicKeyPem = null;     // String
    this.privateKeyPem = null;    // String
};


/**
 * Helper functions to read Key fields
 * TODO: generateRSA()
 */

Key.prototype.publicToDER = function () {
    return this.publicKeyDer;  // Buffer
};

Key.prototype.privateToDER = function () {
    // Remove the '-----XXX-----' from the beginning and the end of the key
    // and also remove any \n in the key string
    var lines = this.privateKeyPem.split('\n');
    priKey = "";
    for (var i = 1; i < lines.length - 1; i++)
	priKey += lines[i];
    return new Buffer(priKey, 'base64');    
};

Key.prototype.publicToPEM = function () {
    return this.publicKeyPem;
};

Key.prototype.privateToPEM = function () {
    return this.privateKeyPem;
};

Key.prototype.getKeyID = function () {
    return this.publicKeyDigest;
};

exports.Key = Key;

Key.prototype.readDerPublicKey = function (/*Buffer*/pub_der) {
    if (LOG > 4) console.log("Encode DER public key:\n" + pub_der.toString('hex'));

    this.publicKeyDer = pub_der;

    var hash = require("crypto").createHash('sha256');
    hash.update(this.publicKeyDer);
    this.publicKeyDigest = new Buffer(hash.digest());
    
    var keyStr = pub_der.toString('base64');
    var keyPem = "-----BEGIN PUBLIC KEY-----\n";
    for (var i = 0; i < keyStr.length; i += 64)
	keyPem += (keyStr.substr(i, 64) + "\n");
    keyPem += "-----END PUBLIC KEY-----";

    this.publicKeyPem = keyPem;

    if (LOG > 4) console.log("Convert public key to PEM format:\n" + this.publicKeyPem);
};

Key.prototype.fromPemFile = function (pub, pri) {
    if (pub == null || pri == null) {
	throw new NoNError('KeyError', 'cannot create Key object without file name.');
    }

    // Read public key

    var pubpem = require('fs').readFileSync(pub).toString();
    if (LOG>4) console.log("Content in public key PEM file: \n" + pubpem);

    var pub_pat = /-----BEGIN\sPUBLIC\sKEY-----[\s\S]*-----END\sPUBLIC\sKEY-----/;
    pubKey = pub_pat.exec(pubpem).toString();
    this.publicKeyPem = pubKey;
    if (LOG>4) console.log("Key.publicKeyPem: \n" + this.publicKeyPem);

    // Remove the '-----XXX-----' from the beginning and the end of the public key
    // and also remove any \n in the public key string
    var lines = pubKey.split('\n');
    pubKey = "";
    for (var i = 1; i < lines.length - 1; i++)
	pubKey += lines[i];
    this.publicKeyDer = new Buffer(pubKey, 'base64');
    if (LOG>4) console.log("Key.publicKeyDer: \n" + this.publicKeyDer.toString('hex'));

    var hash = require("crypto").createHash('sha256');
    hash.update(this.publicKeyDer);
    this.publicKeyDigest = new Buffer(hash.digest());
    if (LOG>4) console.log("Key.publicKeyDigest: \n" + this.publicKeyDigest.toString('hex'));

    // Read private key

    var pem = require('fs').readFileSync(pri).toString();
    if (LOG>4) console.log("Content in private key PEM file: \n" + pem);

    var pri_pat = /-----BEGIN\sRSA\sPRIVATE\sKEY-----[\s\S]*-----END\sRSA\sPRIVATE\sKEY-----/;
    this.privateKeyPem = pri_pat.exec(pem).toString();
    if (LOG>4) console.log("Key.privateKeyPem: \n" + this.privateKeyPem);
};

/**
 * KeyLocator
 */
var KeyLocatorType = {
    KEY:1,
    CERTIFICATE:2,
    KEYNAME:3
};

exports.KeyLocatorType = KeyLocatorType;

var KeyLocator = function KeyLocator(_input, _type) { 
    this.type = _type;
    
    if (_type == KeyLocatorType.KEYNAME) {
    	if (LOG>3) console.log('KeyLocator: Set KEYNAME to ' + _input.to_uri());
    	this.keyName = new KeyName(_input);  // KeyName
    }
    else if (_type == KeyLocatorType.KEY) {
    	if (LOG>3) console.log('KeyLocator: Set KEY to ' + _input.publicKeyPem);
    	this.publicKey = _input;  // Key
    }
    else if (_type == KeyLocatorType.CERTIFICATE) {
    	if (LOG>3) console.log('KeyLocator: Set CERTIFICATE to ' + input.toString('hex'));
    	this.certificate = _input;  // Buffer
    }
};

exports.KeyLocator = KeyLocator;

KeyLocator.prototype.from_ccnb = function(decoder) {
    if (LOG>4) console.log('--------Start decoding KeyLocator...');

    decoder.readStartElement(this.getElementLabel());

    if (decoder.peekStartElement(CCNProtocolDTags.Key)) {
	try {
	    encodedKey = decoder.readBinaryElement(CCNProtocolDTags.Key);

	    this.publicKey = new Key();
	    this.publicKey.readDerPublicKey(encodedKey);
	    this.type = KeyLocatorType.KEY;
	    
	    if(LOG>4) console.log('Public key in PEM format: '+ this.publicKey.publicKeyPem);
	} catch (e) {
	    throw new NoNError('KeyError', "cannot parse key");
	}
    } else if (decoder.peekStartElement(CCNProtocolDTags.Certificate)) {
	try {
	    encodedCert = decoder.readBinaryElement(CCNProtocolDTags.Certificate);
			
	    /*
	     * Certificates not yet working
	     */
			
	    //CertificateFactory factory = CertificateFactory.getInstance("X.509");
	    //this.certificate = (X509Certificate) factory.generateCertificate(new ByteArrayInputStream(encodedCert));

	    this.certificate = encodedCert;
	    this.type = KeyLocatorType.CERTIFICATE;

	    if(LOG>4) console.log('CERTIFICATE FOUND: '+ this.certificate);
	} catch (e) {
	    throw new NoNError('KeyError', "cannot decode certificate.");
	}
    } else  {
	this.type = KeyLocatorType.KEYNAME;		
	this.keyName = new KeyName();
	this.keyName.from_ccnb(decoder);
    }
    decoder.readEndElement();

    if (LOG>4) console.log('--------Finish decoding KeyLocator.');
};


KeyLocator.prototype.to_ccnb = function (encoder) {
    if(LOG>4) console.log('--------Encoding KeyLocator...');
    if(LOG>4) console.log('KeyLocator type is is ' + this.type);

    if (!this.validate()) {
	throw new NoNError('KeyError', "cannot encode KeyLocator because field values missing.");
    }

    encoder.writeStartElement(this.getElementLabel());
	
    if (this.type == KeyLocatorType.KEY) {
	if(LOG>4) console.log('About to encode a public key:\n' + this.publicKey.publicKeyDer.toString('hex'));
	encoder.writeElement(CCNProtocolDTags.Key, this.publicKey.publicKeyDer);
    } else if (this.type == KeyLocatorType.CERTIFICATE) {
	try {
	    encoder.writeElement(CCNProtocolDTags.Certificate, this.certificate);
	} catch (e) {
	    throw new NoNError('KeyError', "certificate encoding error");
	}
    } else if (this.type == KeyLocatorType.KEYNAME) {
	this.keyName.to_ccnb(encoder);
    }
    encoder.writeEndElement();

    if (LOG>4) console.log('--------Finish encoding KeyLocator.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
KeyLocator.prototype.to_xml = function (encoding) {
    var xml = '<KeyLocator>';

    if (encoding == null)
	encoding = 'hex';

    if (this.type == KeyLocatorType.KEY) {
	xml += '<Key ccnbencoding="' + encoding + 'Binary">' + this.publicKey.publicKeyDer.toString(encoding).toUpperCase() + '</Key>';
    } else if (this.type == KeyLocatorType.CERTIFICATE) {
	throw new NoNError('KeyError', "don't know how to encode certificate into XML.");
    } else if (this.type == KeyLocatorType.KEYNAME) {
	xml += this.keyName.to_xml(encoding);
    }
    xml += '</KeyLocator>';
    return xml;
};

KeyLocator.prototype.getElementLabel = function() {
    return CCNProtocolDTags.KeyLocator;
};

KeyLocator.prototype.validate = function() {
    return ((null != this.keyName) || (null != this.publicKey) || (null != this.certificate));
};


/**
 * KeyName is only used by KeyLocator.
 * Currently publisherID is never set by NDN.JS
 */
var KeyName = function KeyName(name, id) {
    this.name = name;  // Name
    this.publisherId = id;  // PublisherID
};

KeyName.prototype.from_ccnb = function (decoder) {
    if (LOG>4) console.log('--------Start decoding KeyName...');

    decoder.readStartElement(this.getElementLabel());

    this.name = new Name();
    this.name.from_ccnb(decoder);
    
    if ( PublisherID.peek(decoder) ) {
	this.publisherID = new PublisherID();
	this.publisherID.from_ccnb(decoder);
    }
    
    decoder.readEndElement();

    if (LOG>4) console.log('--------Finish decoding KeyName.');
};

KeyName.prototype.to_ccnb = function (encoder) {
    if (LOG>4) console.log('--------Encoding KeyName...');

    if (!this.validate()) {
	throw new NoNError('KeyError', "cannot encode because field values missing.");
    }
	
    encoder.writeStartElement(this.getElementLabel());
	
    this.name.to_ccnb(encoder);

    if (null != this.publisherID)
	this.publisherID.to_ccnb(encoder);

    encoder.writeEndElement();

    if (LOG>4) console.log('--------Finish encoding KeyName.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
KeyName.prototype.to_xml = function (encoding) {
    var xml = '<KeyName>';

    if (encoding == null)
	encoding = 'hex';

    xml += this.name.to_xml(encoding);

    if (this.publisherID != null)
	xml += this.publisherID.to_xml(encoding);

    xml += '</KeyName>';
    return xml;
};
	
KeyName.prototype.getElementLabel = function () { return CCNProtocolDTags.KeyName; };

KeyName.prototype.validate = function () {
    // DKS -- do we do recursive validation?
    // null publisherID is ok
    return (null != this.name);
};

/**
 * @author: Meki Cheraoui, Wentao Shang
 * See COPYING for copyright and distribution information.
 * This class represents PublisherID object
 */

/**
 * Ported to node.js by Wentao Shang
 */

var isPublisherIDTag = function (tagVal) {
    if ((tagVal == CCNProtocolDTags.PublisherPublicKeyDigest) ||
	(tagVal == CCNProtocolDTags.PublisherCertificateDigest) ||
	(tagVal == CCNProtocolDTags.PublisherIssuerKeyDigest) ||
	(tagVal == CCNProtocolDTags.PublisherIssuerCertificateDigest)) {
	return true;
    }
    return false;
};

/**
 * id is the SHA-256 hash of the publisher public key Buffer
 * type can be either of CCNProtocolDtags.PublisherPublicKeyDigest | PublisherCertificateDigest | PublisherIssuerKeyDigest | PublisherIssuerCertificateDigest
 * while the latter three are usually not used
 */
var PublisherID = function PublisherID(id, type) {
    this.id = id;  // Buffer
    this.type = type;
};


PublisherID.prototype.from_ccnb = function (decoder) {
    // We have a choice here of one of 4 binary element types.
    var nextTag = decoder.peekStartElementAsLong();
		
    if (null == nextTag) {
	throw new NoNError('PublisherIDError', "cannot parse publisher ID.");
    } 
		
    if (!isPublisherIDTag(nextTag)) {
	throw new NoNError('PublisherIDError', "invalid PublisherID tag " + nextTag);
    }
    this.id = decoder.readBinaryElement(nextTag);
    if (null == this.publisherID) {
	throw new NoNError('PublisherIDError', "cannot read PublisherID of type " + nextTag + ".");
    }

    this.type = nextTag;
};

PublisherID.prototype.to_ccnb = function (encoder) {
    if (!this.validate()) {
	throw new NoNError('PublisherIDError', "cannot encode PublisherID because field value is  missing.");
    }
    
    encoder.writeElement(this.getElementLabel(), this.id);
};


PublisherID.prototype.to_xml = function (encoding) {
    if (encoding == null)
	encoding = 'hex';

    var xml = '<' + CCNProtocolDTagsStrings[this.type] + ' ccnbencoding="' + encoding + 'Binary">' 
    + this.id.toString(encoding).toUpperCase() + '</' + CCNProtocolDTagsStrings[this.type] + '>';
    return xml;
};

// Check if the next component is a PublisherID
PublisherID.peek = function (decoder) {
    var nextTag = decoder.peekStartElementAsLong();
		
    if (null == nextTag) {
	return false;
    }
    return (isPublisherIDTag(nextTag));
};

PublisherID.prototype.getElementLabel = function () { 
    return this.type;
};

PublisherID.prototype.validate = function () {
    return ((null != this.id && (null != this.type)));
};
/**
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 * This class represents PublisherPublicKeyDigest Objects
 */

/**
 * Ported to node.js by Wentao Shang
 */


var PublisherPublicKeyDigest = function PublisherPublicKeyDigest(/* Buffer */ pkd) {
    this.PUBLISHER_ID_LEN = 256/8;    
    this.publisherPublicKeyDigest = pkd;
    if (LOG>3 && pkd != null) console.log('PublisherPublicKeyDigest set to ' + pkd.toString('hex'));
};

PublisherPublicKeyDigest.prototype.from_ccnb = function (decoder) {
    this.publisherPublicKeyDigest = decoder.readBinaryElement(this.getElementLabel());
    
    if(LOG>4) console.log('Publisher public key digest is ' + this.publisherPublicKeyDigest.toString('hex'));

    if (null == this.publisherPublicKeyDigest) {
	throw new NoNError('PKDError', "Cannot parse publisher key digest.");
    }

    if (this.publisherPublicKeyDigest.length != this.PUBLISHER_ID_LEN) {
	console.log('LENGTH OF PUBLISHER ID IS WRONG! Expected ' + this.PUBLISHER_ID_LEN + ", got " + this.publisherPublicKeyDigest.length);
    }
};

PublisherPublicKeyDigest.prototype.to_ccnb = function (encoder) {
    if (LOG>4) console.log('Encoding PublisherPublicKeyDigest...');
    //TODO Check that the ByteArray for the key is present
    if (!this.validate()) {
	throw new NoNError('PKDError', "cannot encode : field values missing.");
    }
    if(LOG>4) console.log('PUBLISHER KEY DIGEST IS ' + this.publisherPublicKeyDigest.toString('hex'));
    encoder.writeElement(this.getElementLabel(), this.publisherPublicKeyDigest);
    if(LOG>4) console.log('Finish encoding PublisherPublicKeyDigest.');
};

/**
 * encoding can be 'hex' or 'base64'; if null, will use 'hex' by default
 */
PublisherPublicKeyDigest.prototype.to_xml = function (encoding) {
    if (encoding == null)
	encoding = 'hex';

    var xml = '<PublisherPublicKeyDigest ccnbencoding="' + encoding + 'Binary">';
    if (this.publisherPublicKeyDigest != null)
	xml += this.publisherPublicKeyDigest.toString(encoding).toUpperCase();
    xml += '</PublisherPublicKeyDigest>';
    return xml;
};

PublisherPublicKeyDigest.prototype.getElementLabel = function () { return CCNProtocolDTags.PublisherPublicKeyDigest; };

PublisherPublicKeyDigest.prototype.validate = function () {
    return (null != this.publisherPublicKeyDigest);
};
/**
 * @author: Meki Cheraoui
 * See COPYING for copyright and distribution information.
 * This class represents Forwarding Entries
 */

/**
 * Ported to node.js by Wentao Shang
 */

var ForwardingEntry = function ForwardingEntry(_action, _prefixName, _ccndId, _faceID, _flags, _lifetime) {
    //String
    this.action = _action;
    //Name
    this.prefixName = _prefixName;
    //PublisherPublicKeyDigest 
    this.ccndID = _ccndId;
    //Integer
    this.faceID = _faceID;
    //Integer
    this.flags = _flags;
    //Integer
    this.lifetime = _lifetime;  // in seconds
};

ForwardingEntry.prototype.from_ccnb = function (/*XMLDecoder*/ decoder) {
    if (LOG > 4) console.log('Start decoding ForwardingEntry...');
    decoder.readStartElement(this.getElementLabel());
    if (decoder.peekStartElement(CCNProtocolDTags.Action)) {
	this.action = decoder.readUTF8Element(CCNProtocolDTags.Action); 
    }
    if (decoder.peekStartElement(CCNProtocolDTags.Name)) {
	this.prefixName = new Name();
	this.prefixName.from_ccnb(decoder) ;
    }
    if (decoder.peekStartElement(CCNProtocolDTags.PublisherPublicKeyDigest)) {
	this.CcndId = new PublisherPublicKeyDigest();
	this.CcndId.from_ccnb(decoder);
    }
    if (decoder.peekStartElement(CCNProtocolDTags.FaceID)) {
	this.faceID = decoder.readIntegerElement(CCNProtocolDTags.FaceID); 
    }
    if (decoder.peekStartElement(CCNProtocolDTags.ForwardingFlags)) {
	this.flags = decoder.readIntegerElement(CCNProtocolDTags.ForwardingFlags); 
    }
    if (decoder.peekStartElement(CCNProtocolDTags.FreshnessSeconds)) {
	this.lifetime = decoder.readIntegerElement(CCNProtocolDTags.FreshnessSeconds); 
    }
    decoder.readEndElement();
    if (LOG > 4) console.log('Finish decoding ForwardingEntry.');
};

/**
 * Used by NetworkObject to encode the object to a network stream.
 */
ForwardingEntry.prototype.to_ccnb =function(/*XMLEncoder*/ encoder) {
    if (LOG > 4) console.log('--------Encoding ForwardingEntry...');
    encoder.writeStartElement(this.getElementLabel());
    if (null != this.action && this.action.length != 0)
	encoder.writeElement(CCNProtocolDTags.Action, this.action);	
    if (null != this.prefixName) {
	this.prefixName.to_ccnb(encoder);
    }
    if (null != this.CcndId) {
	this.CcndId.to_ccnb(encoder);
    }
    if (null != this.faceID) {
	encoder.writeElement(CCNProtocolDTags.FaceID, this.faceID);
    }
    if (null != this.flags) {
	encoder.writeElement(CCNProtocolDTags.ForwardingFlags, this.flags);
    }
    if (null != this.lifetime) {
	encoder.writeElement(CCNProtocolDTags.FreshnessSeconds, this.lifetime);
    }
    encoder.writeEndElement();
    if (LOG > 4) console.log('--------Finish encoding ForwardingEntry.');
};

ForwardingEntry.prototype.encodeToBinary = function () {
    var enc = new BinaryXMLEncoder();
    this.to_ccnb(enc);
    return enc.getReducedOstream();
};

ForwardingEntry.prototype.getElementLabel = function() { return CCNProtocolDTags.ForwardingEntry; };
/**
 * @author: Wentao Shang
 * See COPYING for copyright and distribution information.
 */

var DataClosure = function DataClosure(onData, onTimeout) {
    this.onData = onData;
    this.onTimeout = onTimeout;
};

var InterestClosure = function InterestClosure(onInterest) {
    this.onInterest = onInterest;
};/** 
 * @author: Wentao Shang
 * See COPYING for copyright and distribution information.
 */

var TcpTransport = function TcpTransport() {    
    this.socket = null;
    this.sock_ready = false;
    this.elementReader = null;
};

TcpTransport.prototype.connect = function(ndn) {
    if (this.socket != null)
	delete this.socket;

    this.elementReader = new BinaryXmlElementReader(ndn);

    // Connect to local ccnd via TCP
    var net = require('net');
    this.socket = new net.Socket();
    
    var self = this;

    this.socket.on('data', function(data) {			
	    if (typeof data == 'object') {
		// Make a copy of data (maybe a Buffer or a String)
		var buf = new Buffer(data);
		// Find the end of the binary XML element and call ndn.onReceivedElement.
		self.elementReader.onReceivedData(buf);
	    }
	});
    
    this.socket.on('connect', function() {
	    if (LOG > 3) console.log('socket.onopen: TCP connection opened.');
	    
	    self.sock_ready = true;

	    // Fetch ccndid now
	    var interest = new Interest(NDN.ccndIdFetcher);
	    interest.interestLifetime = 4000; // milliseconds
	    self.send(interest.encodeToBinary());
	});
    
    this.socket.on('error', function() {
	    if (LOG > 3) console.log('socket.onerror: TCP socket error');
	});
    
    this.socket.on('close', function() {
	    if (LOG > 3) console.log('socket.onclose: TCP connection closed.');

	    self.socket = null;
	    
	    // Close NDN when TCP Socket is closed
	    ndn.ready_status = NDN.CLOSED;
	    ndn.onclose();
	});

    this.socket.connect({host: 'localhost', port: 9695});
};

/**
 * Send data.
 */
TcpTransport.prototype.send = function(/*Buffer*/ data) {
    if (this.sock_ready) {
        this.socket.write(data);
    } else
	console.log('TCP connection is not established.');
};

/**
 * Close transport
 */
TcpTransport.prototype.close = function () {
    this.socket.end();
    if (LOG > 3) console.log('TCP connection closed.');
};/**
 * @author: Meki Cherkaoui, Jeff Thompson, Wentao Shang
 * See COPYING for copyright and distribution information.
 * This class represents the top-level object for communicating with an NDN host.
 */

/**
 * Ported to node.js by Wentao Shang
 */

var LOG = 0;

/**
 * NDN wrapper
 */
var NDN = function NDN() {
    this.transport = new TcpTransport();
    this.ready_status = NDN.UNOPEN;

    // Event handler
    this.onopen = function () { console.log("NDN connection established."); };
    this.onclose = function () { console.log("NDN connection closed."); };

    this.ccndid = null;
    this.default_key = null;
};

NDN.prototype.setDefaultKey = function (pubfile, prifile) {
    this.default_key = new Key();
    this.default_key.fromPemFile(pubfile, prifile);
};

NDN.prototype.getDefaultKey = function () {
    return this.default_key;
};

exports.NDN = NDN;

NDN.UNOPEN = 0;  // created but not opened yet
NDN.OPENED = 1;  // connection to ccnd opened
NDN.CLOSED = 2;  // connection to ccnd closed

NDN.ccndIdFetcher = new Name('/%C1.M.S.localhost/%C1.M.SRV/ccnd/KEY');

// Connect NDN wrapper to local ccnd
NDN.prototype.connect = function () {
    if (this.ready_status == NDN.OPENED)
	throw new NoNError('NDNError', 'cannot connect because connection is already opened.');

    this.transport.connect(this);
};

// Send packet through NDN wrapper
NDN.prototype.send = function (packet) {
    if (this.ready_status != NDN.OPENED)
	throw new NoNError('NDNError', 'cannot send because connection is not opened.');
    
    if (packet instanceof Buffer)
	this.transport.send(packet);
    else if (packet instanceof Interest || packet instanceof ContentObject)
	this.transport.send(packet.encodeToBinary());
    else
	throw new NoNError('NDNError', 'cannot send object of type ' + packet.constructor.name);
};

// Close NDN wrapper
NDN.prototype.close = function () {
    if (this.ready_status != NDN.OPENED)
	throw new NoNError('NDNError', 'cannot close because connection is not opened.');

    this.ready_status = NDN.CLOSED;
    this.transport.close();
};

// For fetching data
var PITTable = new Array();

var PITEntry = function PITEntry(interest, closure) {
    this.interest = interest;  // Interest
    this.closure = closure;    // Closure
    this.timerID = -1;  // Timer ID
};

// Return the longest entry from PITTable that matches name.
var getEntryForExpressedInterest = function (/*Name*/ name) {
    var result = null;
    
    for (var i = 0; i < PITTable.length; i++) {
	if (PITTable[i].interest.matches_name(name)) {
            if (result == null || 
                PITTable[i].interest.name.components.length > result.interest.name.components.length)
                result = PITTable[i];
        }
    }
    
    return result;
};

// For publishing data
var CSTable = new Array();

var CSEntry = function CSEntry(name, closure) {
    this.name = name;        // Name
    this.closure = closure;  // Closure
};

var getEntryForRegisteredPrefix = function (name) {
    for (var i = 0; i < CSTable.length; i++) {
	if (CSTable[i].name.match(name) != null)
	    return CSTable[i];
    }
    return null;
};


// Verification status
NDN.CONTENT = 0; // content verified
NDN.CONTENT_UNVERIFIED = 1; // content that has not been verified
NDN.CONTENT_BAD = 2; // verification failed

/**
 * Prototype of 'onData': function (interest, contentObject, verification_status) {}
 * Prototype of 'onTimeOut': function (interest) {}
 */
NDN.prototype.expressInterest = function (name, template, onData, onTimeOut) {
    if (this.ready_status != NDN.OPENED) {
	throw new NoNError('NDNError', 'connection is not established.');
    }

    var interest = new Interest(name);
    if (template != null) {
	interest.minSuffixComponents = template.minSuffixComponents;
	interest.maxSuffixComponents = template.maxSuffixComponents;
	interest.publisherPublicKeyDigest = template.publisherPublicKeyDigest;
	interest.exclude = template.exclude;
	interest.childSelector = template.childSelector;
	interest.answerOriginKind = template.answerOriginKind;
	interest.scope = template.scope;
	interest.interestLifetime = template.interestLifetime;
    }
    else
        interest.interestLifetime = 4000;   // default interest timeout value in milliseconds.

    var closure = new DataClosure(onData, onTimeOut);
    var pitEntry = new PITEntry(interest, closure);
    PITTable.push(pitEntry);

    if (interest.interestLifetime == null)
	// Use default timeout value
	interest.interestLifetime = 4000;
	
    if (interest.interestLifetime > 0) {
	pitEntry.timerID = setTimeout(function() {
		if (LOG > 3) console.log("Interest time out.");
					
		// Remove PIT entry from PITTable.
		var index = PITTable.indexOf(pitEntry);
		if (index >= 0)
		    PITTable.splice(index, 1);
					
		// Raise timeout callback
		closure.onTimeout(pitEntry.interest);
	    }, interest.interestLifetime);  // interestLifetime is in milliseconds.
	//console.log(closure.timerID);
    }

    this.transport.send(interest.encodeToBinary());
};

/**
 * Prototype of 'onInterest': function (interest) {}
 */
NDN.prototype.registerPrefix = function(prefix, onInterest) {
    if (this.ready_status != NDN.OPENED) {
	throw new NoNError('NDNError', 'connection is not established.');
    }

    if (this.ccndid == null) {
	throw new NoNError('NDNError', 'ccnd node ID unkonwn. Cannot register prefix.');
    }

    if (this.default_key == null) {
	throw new NoNError('NDNError', 'cannot register prefix without default key');
    }
    
    var fe = new ForwardingEntry('selfreg', prefix, null, null, 3, 2147483647);
    var feBytes = fe.encodeToBinary();

    var co = new ContentObject(new Name(), feBytes);
    co.sign(this.default_key);  // Use default key to sign registration packet
    var coBinary = co.encodeToBinary();

    var interestName = new Name(['ccnx', this.ccndid, 'selfreg', coBinary]);
    var interest = new Interest(interestName);
    interest.scope = 1;
    
    var closure = new InterestClosure(onInterest);
    var csEntry = new CSEntry(prefix, closure);
    CSTable.push(csEntry);

    var data = interest.encodeToBinary();
    this.transport.send(data);

    if (LOG > 3) console.log('Prefix registration packet sent.');
};

/*
 * This is called when an entire binary XML element is received, such as a ContentObject or Interest.
 * Look up in the PITTable and call the closure callback.
 */
NDN.prototype.onReceivedElement = function(element) {
    if (LOG > 4) console.log('Complete element received. Length ' + element.length + '. Start decoding.');
    
    var decoder = new BinaryXMLDecoder(element);
    // Dispatch according to packet type
    if (decoder.peekStartElement(CCNProtocolDTags.Interest)) {  // Interest packet
	var interest = new Interest();
	interest.from_ccnb(decoder);
	
	if (LOG > 3) console.log('Interest name is ' + interest.name.to_uri());
	
	var entry = getEntryForRegisteredPrefix(interest.name);
	if (entry != null) {
	    //console.log(entry);
	    entry.closure.onInterest(interest);
	}				
    } else if (decoder.peekStartElement(CCNProtocolDTags.ContentObject)) {  // Content packet
	var co = new ContentObject();
	co.from_ccnb(decoder);

	if (LOG > 3) console.log('ContentObject name is ' + co.name.to_uri());

	if (this.ccndid == null && NDN.ccndIdFetcher.match(co.name)) {
	    // We are in starting phase, record publisherPublicKeyDigest in ccndid
	    if(!co.signedInfo || !co.signedInfo.publisher 
	       || !co.signedInfo.publisher.publisherPublicKeyDigest) {
		console.log("Cannot contact router, close NDN now.");
		
		// Close NDN if we fail to connect to a ccn router
		this.ready_status = NDN.CLOSED;
		this.transport.close();
	    } else {
		if (LOG>3) console.log('Connected to local ccnd.');
		this.ccndid = co.signedInfo.publisher.publisherPublicKeyDigest;
		if (LOG>3) console.log('Local ccnd ID is ' + this.ccndid.toString('hex'));
		
		// Call NDN.onopen after success
		this.ready_status = NDN.OPENED;
		this.onopen();
	    }
	} else {
	    var pitEntry = getEntryForExpressedInterest(co.name);
	    if (pitEntry != null) {
		//console.log(pitEntry);
		// Remove PIT entry from PITTable
		var index = PITTable.indexOf(pitEntry);
		if (index >= 0)
		    PITTable.splice(index, 1);

		var cl = pitEntry.closure;
		
		// Cancel interest timer
		clearTimeout(pitEntry.timerID);

		// Key verification
		// We only verify the signature when the KeyLocator contains KEY bits

		if (co.signedInfo && co.signedInfo.locator && co.signature && co.signature.signature) {
		    if (co.signature.Witness != null) {
			// Bypass verification if Witness is present
			cl.onData(pitEntry.interest, co, NDN.CONTENT_UNVERIFIED);
			return;
		    }
		    
		    var keylocator = co.signedInfo.locator;
		    if (keylocator.type == KeyLocatorType.KEY) {
			if (LOG > 3) console.log("Keylocator contains KEY:\n" + keylocator.publicKey.publicKeyDer.toString('hex'));

			var flag = (co.verify(keylocator.publicKey) == true) ? NDN.CONTENT : NDN.CONTENT_BAD;
			cl.onData(pitEntry.interest, co, flag);
		    } else {
			if (LOG > 3) console.log("KeyLocator does not contain KEY. Leave for user to verify data.");
			cl.onData(pitEntry.interest, co, NDN.CONTENT_UNVERIFIED);
		    }
		}
	    }
	}
    }
};

/*
 * A BinaryXmlElementReader lets you call onReceivedData multiple times which uses a
 *   BinaryXMLStructureDecoder to detect the end of a binary XML element and calls
 *   elementListener.onReceivedElement(element) with the element. 
 * This handles the case where a single call to onReceivedData may contain multiple elements.
 */
var BinaryXmlElementReader = function BinaryXmlElementReader(elementListener) {
    this.elementListener = elementListener;
    this.dataParts = [];
    this.structureDecoder = new BinaryXMLStructureDecoder();
};


BinaryXmlElementReader.prototype.onReceivedData = function(/* Buffer */ rawData) {
    // Process multiple objects in the data.
    while(true) {
        // Scan the input to check if a whole ccnb object has been read.
        this.structureDecoder.seek(0);
        if (this.structureDecoder.findElementEnd(rawData)) {
            // Got the remainder of an object.  Report to the caller.
            this.dataParts.push(rawData.slice(0, this.structureDecoder.offset));
            this.elementListener.onReceivedElement(DataUtils.concatArrays(this.dataParts));
        
            // Need to read a new object.
            rawData = rawData.slice(this.structureDecoder.offset, rawData.length);
            this.dataParts = [];
            this.structureDecoder = new BinaryXMLStructureDecoder();
            if (rawData.length == 0)
                // No more data in the packet.
                return;
            
            // else loop back to decode.
        }
        else {
            // Save for a later call to concatArrays so that we only copy data once.
            this.dataParts.push(rawData);
	    if (LOG>4) console.log('Incomplete packet received. Length ' + rawData.length + '. Wait for more input.');
            return;
        }
    }
};