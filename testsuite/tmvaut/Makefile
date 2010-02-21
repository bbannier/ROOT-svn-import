include $(ROOTSYS)/test/Makefile.arch

.SUFFIXES: .$(SrcSuf)

SOURCES    = $(wildcard *.cxx)
OBJECTS    = $(SOURCES:.cxx=.o)

all: $(OBJECTS)

.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS) -c $<


clean:
	rm -f $(OBJECTS)

distclean:
	rm -f $(OBJECTS)

.PHONY : all
