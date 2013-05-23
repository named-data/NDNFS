CXX = g++

CFLAGS = -c -Wall

LDFLAGS =

# lbooster lmongoclient lboost_thread lboost_filesystem lboost_program_options
# lboost_system
LIBRARIES =	-pthread \
			-lndn.cxx \
			-lmongoclient \
			-lboost_thread \
			-lboost_filesystem \
			-lboost_program_options \
			-lboost_system
INCLUDE_PATHS = -I/usr/local/include \
				-I/usr/include/boost \
				-I/usr/local/include/mongo
LIBRARY_PATHS = -L/usr/lib \
				-L/usr/local/lib

SOURCES = server.cc servermodule.cc 

OBJECTS = $(SOURCES:.cc=.o)
EXECUTABLE = server

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	    $(CXX) $(LDFLAGS) $(OBJECTS) $(LIBRARY_PATHS) $(LIBRARIES) -o $@
.cc.o:
	$(CXX) $(CFLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) $< -o $@ 

clean:
	    rm -f $(EXECUTABLE) $(OBJECTS)
