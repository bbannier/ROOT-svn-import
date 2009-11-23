######################################################################
# Project: TMVA - Toolkit for Multivariate Data Analysis             #
# Code   : Example usage for TMVA analyses                           #
###################################################################### 

MAKEFLAGS = --no-print-directory -r -s

include Makefile.arch

INCLUDE = -I../include $(shell root-config --cflags)
LIBS    = -L../lib/ $(shell root-config --libs) -lTMVA.1 -lMLP -lTreePlayer -lMinuit

TMVALIB = ../lib/libTMVA.1.so

BINS = TMVAClassificationApplication397 


all: $(BINS)

$(TMVALIB): ../src/*.cxx ../src/*.h
	$(MAKE) -C ../src/

$(BINS): % : %.cxx ../src/*.h
	@echo -n "Building $@ ... "
	$(CXX) $(CCFLAGS) $< $(INCLUDE) $(LIBS) -g -o $@
	@echo "Done"

clean:
	rm -f $(BINS)

.PHONY : all clean

