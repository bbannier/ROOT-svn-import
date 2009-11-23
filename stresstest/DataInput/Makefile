######################################################################
# Project: TMVA - Toolkit for Multivariate Data Analysis             #
# Code   : Test different ways of DataInput                          #
###################################################################### 

MAKEFLAGS = --no-print-directory -r -s

INCLUDE = -I../../include $(shell root-config --cflags)
LIBS    = -L../../lib/ $(shell root-config --libs) -lTMVA.1 -lMLP -lTreePlayer -lMinuit

BINS = TMVATestDataInput

ROOTFILES = TMVAInputData.root

TMVALIB = ../lib/libTMVA.1.so

all: $(BINS) TMVAInputData.root

TMVAInputData.root: createComparisonData.C run.C
	root -l run.C

tests:
	python runTestDataInput.py

clean:
	rm -f $(BINS) $(ROOTFILES)

fullclean: clean
	rm -rf results weights

$(BINS): % : %.cxx ../../src/*.h *.h
	@echo -n "Building $@ ... "
	$(CXX) $(CCFLAGS) $< $(INCLUDE) $(LIBS) -g -o $@
	@echo "Done"

.PHONY : all clean

