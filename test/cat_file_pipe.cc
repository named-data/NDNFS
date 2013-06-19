/*
 * Copyright (c) 2013 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhe Wen <wenzhe@cs.ucla.edu>, Wentao Shang <wentao@cs.ucla.edu>
 */


#include <ndn.cxx.h>
#include <iostream>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;

namespace ndn {

typedef boost::posix_time::ptime Time;
typedef boost::posix_time::time_duration TimeInterval;

namespace time
{
    inline TimeInterval Seconds (int secs) { return boost::posix_time::seconds (secs); }
    inline TimeInterval Milliseconds (int msecs) { return boost::posix_time::milliseconds (msecs); }
    inline TimeInterval Microseconds (int musecs) { return boost::posix_time::microseconds (musecs); }

    inline TimeInterval Seconds (double fractionalSeconds)
    {
	double seconds, microseconds;
	seconds = std::modf (fractionalSeconds, &microseconds);
	microseconds *= 1000000;

	return time::Seconds (seconds) + time::Microseconds (microseconds);
    }

    inline Time Now () { return boost::posix_time::microsec_clock::universal_time (); }

    const Time UNIX_EPOCH_TIME = Time (boost::gregorian::date (1970, boost::gregorian::Jan, 1));
    inline TimeInterval NowUnixTimestamp ()
    {
	return TimeInterval (time::Now () - UNIX_EPOCH_TIME);
    }
} // time
} // ndn

ndn::Name filename;
ndn::Wrapper handler;

int interest_pipe_size = 8;

int retrans = 0;
int totalsize = 0;
int totalseq = 0;  // total number of unique seq num received
int next_seq = 0;  // max of seq num requested

ndn::Time start;

void OnData(ndn::Name name, ndn::PcoPtr pco);
void OnTimeout(ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest);

void OnData(ndn::Name name, ndn::PcoPtr pco) {
    ndn::BytesPtr content = pco->contentPtr();
//    cout << "data: " << string((char*)ndn::head(*content), content->size()) << endl;
    uint64_t seqnum = name.rbegin()->toSeqNum();
    totalsize += content->size();
//    cout << seqnum << endl;
    totalseq++;
    if (seqnum == 0) {
	filename.append( *(++name.rbegin()) );
	cout << filename.toUri() << endl;
//	sleep(1);
	for (int i = 0; i < interest_pipe_size; i++) {
     	    ndn::Interest interest = ndn::Interest();
	    interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	    //interest.setAnswerOriginKind(0);
    
	    next_seq++;
	    interest.setName(ndn::Name(filename).appendSeqNum(next_seq));
	    handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	}
	return;
    }
    
    if (totalseq == 23952 + 1) {
//    if (totalseq == 55 + 1) {
	// We can stop now
	ndn::Time stop = ndn::time::Now();
	cout << "Total run time: " << (stop - start) << endl;
	cout << "Total segment fetched: " << totalseq << endl;
	cout << "Max seq num requested: " << next_seq << endl;
	cout << "Total size fetched: " << totalsize << endl;
	cout << "Total number of retrans: " << retrans << endl;
	return;
    }
    
    // When we receive a new segment, request the next unrequested segment 
    // so that there is always fixed number of outstanding interests
    ndn::Interest interest = ndn::Interest();
    interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
    //interest.setAnswerOriginKind(0);
    
    next_seq++;
    interest.setName(ndn::Name(filename).appendSeqNum(next_seq));
    handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
    return;
}

void OnTimeout(ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest) {
    uint64_t seqnum = name.rbegin()->toSeqNum();
    cout << "Timeout on seqnum " << seqnum << endl;
    if (totalseq >= 23952 + 1) {
//    if (totalseq >= 55 + 1) {
	// We already finished, so don't retrans anymore
	return;
    }
    // otherwise, always re-express interest on timeout
    retrans++;
    handler.sendInterest(*origInterest, closure);
}

void Usage() {
	fprintf(stderr, "usage: ./client [-n name][-i minsuffix][-a maxfuffix][-c childeselector]\n");
	exit(1);
}

int main (int argc, char **argv) {
	ndn::Interest interest = ndn::Interest();
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	//interest.setAnswerOriginKind(0);

	const char* name = "";
	uint32_t min_suffix_comps = ndn::Interest::ncomps;
	uint32_t max_suffix_comps = ndn::Interest::ncomps;
	uint8_t child_selector = ndn::Interest::CHILD_DEFAULT;

	int opt;
	while ((opt = getopt(argc, argv, "n:i:a:c:")) != -1) {
		switch (opt) {
			case 'n': 
				name = optarg;
				cout << "main(): set name: " << name << endl;
				filename = ndn::Name(name);
				interest.setName(filename);
				break;
			case 'i': 
				min_suffix_comps = atoi(optarg);
				cout << "main(): set min suffix components: " << min_suffix_comps << endl;
				interest.setMinSuffixComponents(min_suffix_comps);
				break;
			case 'a': 
				max_suffix_comps = atoi(optarg);
				cout << "main(): set max suffix components: " << max_suffix_comps << endl;
				interest.setMaxSuffixComponents(max_suffix_comps);
				break;
			case 'c': 
				child_selector = atoi(optarg);
				cout << "main(): set child selector: " << (uint32_t)child_selector << endl;
				interest.setChildSelector(child_selector);
				break;
			default: 
				Usage(); 
				break;
		 }
	}

	start = ndn::time::Now();
	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Started..." << endl;

	sleep(10);


	return 0;
}
