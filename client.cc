
#include <ndn.cxx.h>
#include <iostream>

using namespace std;

const char *FILENAME = NULL;
ndn::Name InterestBaseName;

// create a global handler
ndn::Wrapper handler;

void OnData (ndn::Name name, ndn::PcoPtr pco);
void OnTimeout (ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest);

void OnData (ndn::Name name, ndn::PcoPtr pco) {
	ndn::BytesPtr content = pco->contentPtr ();
	cout << "data: " << string ((char*)ndn::head (*content), content->size ()) << endl;
}

void OnTimeout (ndn::Name name, const ndn::Closure &closure, ndn::InterestPtr origInterest) {
	// re-express interest
	// handler.sendInterest (*origInterest, closure);
	cout << "TIME OUT :(" << endl;
}

int main (int argc, char **argv) {
	ndn::Interest interest;

	cout << "--------------------------------------------------" << endl;
	interest = ndn::Interest();
	interest.setName(ndn::Name("/ucla.edu/cs/CS217B"));
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	cout << ">>>> default interest: " << interest.getName() << endl;
	cout << "Expect: /ucla.edu/cs/CS217B/presentation.pdf/10/1: data" << endl;
	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Interest sent" << endl;

	usleep (200);
	cout << "--------------------------------------------------" << endl;
	interest = ndn::Interest();
	interest.setName(ndn::Name("/ucla.edu/cs/CS217B"));
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	interest.setMinSuffixComponents(5);
	cout << ">>>> min suffix interest: " << interest.getName() << endl;
	cout << "Expect: /ucla.edu/cs/CS217B/project/final.tar.gz/1000/0: data" << endl;
	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Interest sent" << endl;

	usleep (200);
	cout << "--------------------------------------------------" << endl;
	interest = ndn::Interest();
	interest.setName(ndn::Name("/ucla.edu/cs"));
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	interest.setChildSelector(ndn::Interest::CHILD_RIGHT);
	cout << ">>>> child selector interest: " << interest.getName() << endl;
	cout << "Expect: /ucla.edu/cs/CS217B/README.txt/100/0: data" << endl;
	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Interest sent" << endl;

	usleep (200);
	cout << "--------------------------------------------------" << endl;
	interest = ndn::Interest();
	interest.setName(ndn::Name("/cs.ucla.edu"));
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	cout << ">>>> non-existing interest: " << interest.getName() << endl;
	cout << "Expect: NULL" << endl;
	handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	cout << "Interest sent" << endl;

	// TODO: ccnx-cpp aborts sending max suffix interest.
	// not sure what is wrong
	usleep (200);
	cout << "--------------------------------------------------" << endl;
	interest = ndn::Interest();
	interest.setName(ndn::Name("/ucla.edu/cs/CS217B"));
	interest.setScope(ndn::Interest::SCOPE_LOCAL_HOST);
	interest.setMaxSuffixComponents(3);
	cout << ">>>> max suffix interest: " << interest.getName() << endl;
	cout << "Expect: NULL" << endl;
	try {
		handler.sendInterest(interest, ndn::Closure(OnData, OnTimeout));
	}
	catch (boost::exception &e) {
		cerr << boost::diagnostic_information(e) << endl;
	}
	cout << "Interest sent" << endl;




	return 0;
}
