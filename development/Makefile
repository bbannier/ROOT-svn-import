######################################################################
# Project: Examples of how to use TMVA                               #
###################################################################### 

MAKEFLAGS = --no-print-directory -r -s

INCLUDE = -I../include $(shell root-config --cflags)
LIBS    = -L../lib/ $(shell root-config --libs) -lTMVA.1 -lMLP -lTreePlayer

BINS = testBenchRegression #devTMVA MakeClassTester

CLASSES = $(patsubst weights/%.C,%.o,$(wildcard weights/*.class.C))

#dev: devTMVA

default: $(CLASSES)

%.o : weights/%.C 
	@echo -n "Building $@ ... "
	@$(CXX) $(INCLUDES) $(CXXFLAGS) -g -c $< -o $(notdir $@)
	@echo "Done"


all: $(BINS)

$(BINS): % : %.C ../src/*.h
	@echo -n "Building $@ ... "
	@echo	$(CXX) $(CCFLAGS) $< $(INCLUDE) $(LIBS) -o $@
	$(CXX) $(CCFLAGS) $< $(INCLUDE) $(LIBS) -o $@
	@echo "Done"

clean:
	rm -f $(BINS)




