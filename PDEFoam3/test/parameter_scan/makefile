
# set tmva directory here
DIRTMVA = /afs/cern.ch/user/path/to/tmva

PROGR   = analysis

LIBS    = -L$(DIRTMVA)/lib -lTMVA.1 \
	  $(shell root-config --libs) -lMLP -lTreePlayer -lMinuit -lXMLIO

INCLUDE = -I$(DIRTMVA)/ \
	  $(shell root-config --cflags) 

$(PROGR): % : %.cxx
	g++ -O2 -Wall $< $(INCLUDE) $(LIBS) -o $@

clean:
	rm -f *.o
	rm -f *.pyc
	rm -f $(PROGR)
