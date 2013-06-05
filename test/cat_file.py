import sys, time
import pyccn

class DataClosure(pyccn.Closure):
    def __init__(self, file):
        self.name = pyccn.Name(file)
        self.versioned = False
        self.totalsize = 0
        self.segnum = 0
        self.retrans = 0

    def upcall(self, kind, upcallInfo):
        if kind == pyccn.UPCALL_CONTENT or kind == pyccn.UPCALL_CONTENT_UNVERIFIED:
            co = upcallInfo.ContentObject

#            print co.name
#            print len(co.content)
            self.totalsize += len(co.content)

            if self.versioned == False:
                self.name = self.name.append(co.name[-2])
                self.versioned = True
#                print self.name

            self.segnum = pyccn.Name.seg2num(co.name[-1])
#            print self.segnum
            if self.segnum == 9405:
#            if self.segnum == 55:
                stop = time.time()
                print stop
                print "Total running time: " + str(stop - start)
                print "Total segments fetched: " + str(self.segnum)
                print "Total size fetched: " + str(self.totalsize)
                print "Total number of retransmission: " + str(self.retrans)
                return pyccn.RESULT_OK

            nextname = self.name.appendSegment(self.segnum + 1)
#            print nextname
            handler.expressInterest(nextname, self, tmpl)

        elif kind == pyccn.UPCALL_INTEREST_TIMED_OUT:
#            print "Timeout"
#            print "Total segments fetched: " + str(self.segnum)
#            print "Total size fetched: " + str(self.totalsize)
#            return pyccn.RESULT_OK
            self.retrans = self.retrans + 1
            return pyccn.RESULT_REEXPRESS

        return pyccn.RESULT_OK

if len(sys.argv) != 2:
    print "Error: need to specify prefix of the file (without trailing '/')"
    exit(-1)

file = sys.argv[1]
closure = DataClosure(file)
tmpl = pyccn.Interest(answerOriginKind = 0, interestLifetime = 1000.0)
handler = pyccn.CCN()

start = time.time()
print start
handler.expressInterest(closure.name, closure, tmpl)
handler.run(20000)
