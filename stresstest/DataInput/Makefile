######################################################################
# Project: TMVA - Toolkit for Multivariate Data Analysis             #
# Code   : Test different ways of DataInput                          #
###################################################################### 

MAKEFLAGS = --no-print-directory -r -s

INCLUDE = -I../../../tmva $(shell root-config --cflags)

BINS = TMVATestDataInput

ROOTFILES = TMVAInputData.root 

TMVALIB =../../../tmva/lib/libTMVA.1.so
LIBS    = -L../../../tmva/lib $(shell root-config --libs) -lTMVA.1 -lMLP -lTreePlayer -lMinuit

all: $(BINS) TMVAInputData.root

TMVAInputData.root: createComparisonData.C run.C
	root -q -l run.C

tests:
	python runTestDataInput.py

clean:
	rm -f $(BINS) $(ROOTFILES) file*.dat

fullclean: clean
	rm -rf results weights

$(TMVALIB):
	echo make sure that the local TMVA lib can be found here: $(TMVALIB)

$(BINS): % : %.cxx *.cxx *.h $(TMVALIB) 
	echo make sure that LD_LIBRARY_PATH is set via tmva/test/setup.sh
	@echo -n "Building $@ ... "
	echo $(CXX) $(CCFLAGS) $(INCLUDE) $(LIBS) -g -o 
	$(CXX) $(CCFLAGS) $< $(INCLUDE) $(LIBS) -g -o $@
	@echo "Done"

.PHONY : all clean

