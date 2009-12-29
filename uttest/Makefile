SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .cxx .o

CXX = g++
CFLAGS=-c -Wall -I../../tmva $(shell root-config --cflags) -I../../tmva/inc
LIBS    = -L../../tmva/lib/ $(shell root-config --libs) -lTMVA.1 -lMLP -lTreePlayer -lMinuit

SOURCES    = $(wildcard *.cxx)
OBJECTS    = $(SOURCES:.cxx=.o)
EXECUTABLE = testTMVA

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LIBS) $(OBJECTS) -o $@

.cxx.o:
	@echo -n "Building $@ ... "
	$(CXX) $(CFLAGS) $< -o $@
	@echo "Done"


clean:
	rm -f $(EXECUTABLE) $(OBJECTS)

.PHONY : all
