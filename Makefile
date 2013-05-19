CXX = g++

TARGET = server

FLAGS = -c -Wall

# lbooster lmongoclient lboost_thread lboost_filesystem lboost_program_options
# lboost_system
LIBRARIES = -lndn.cxx \
			-lmongoclient \
			-lboost_thread-mt \
			-lboost_filesystem \
			-lboost_program_options \
			-lboost_system
INCLUDE_PATHS = -I/usr/local/include \
				-I/usr/local/include/boost \
				-I/usr/local/include/mongo \
				-I/home/nathanw/devel/_include
LIBRARY_PATHS = -L/usr/lib \
				-L/usr/local/lib

SOURCES = servermodule.cc server.cc

OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	    $(CXX) $(OBJECTS) $(LIBRARY_PATHS) $(LIBRARIES) -o $(TARGET)

%.o: %.cpp
	    $(CXX) $(FLAGS) $(INCLUDE_PATHS) $(LIBRARY_PATHS) $(LIBRARIES) $< -o $@

clean:
	    rm -f $(TARGET) $(OBJECTS)
