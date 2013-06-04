import sys
import pyccn

class DataClosure(pyccn.Closure):
    def __init__(self, file):
        self.name = pyccn.Name(file)
        self.versioned = False
        self.totalsize = 0

    def upcall(self, kind, upcallInfo):
        if kind == pyccn.UPCALL_CONTENT or kind == pyccn.UPCALL_CONTENT_UNVERIFIED:
            co = upcallInfo.ContentObject

            print co.name
            print len(co.content)
            self.totalsize += len(co.content)

            if self.versioned == False:
                self.name = self.name.append(co.name[-2])
                self.versioned = True
                print self.name

            segnum = pyccn.Name.seg2num(co.name[-1])
            print segnum
            if segnum >= 55:
                print "Total size fetched: " + str(self.totalsize)
                return pyccn.RESULT_OK;

            handler.expressInterest(self.name.appendSegment(segnum + 1), self)

        elif kind == pyccn.UPCALL_INTEREST_TIMED_OUT:
            return pyccn.RESULT_OK

        return pyccn.RESULT_OK

if len(sys.argv) != 2:
    print "Error: need to specify prefix of the file (without trailing '/')"
    exit(-1)

file = sys.argv[1]
closure = DataClosure(file)
handler = pyccn.CCN()

handler.expressInterest(closure.name, closure)
handler.run(2000)
