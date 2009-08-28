CXX=g++
all: test

%.o: %.cxx
	$(CXX) -g -c -I$(ROOTSYS)/include $<

symdep: main.cxx $(wildcard T*.h T*.cxx)
	$(CXX) -g $(filter %.cxx,$^) -o $@

usrlibs.txt:
	find /usr/ -name '*.so' | sed -r 's,/usr/lib/libpulsedsp.so,,' > $@

test: symdep ao.o usrlibs.txt
	time ./symdep ao.o `ls $(ROOTSYS)/lib/*.so | sed -r 's,'$(ROOTSYS)'/lib/lib(XrdPosixPreload|New).so,,'` `cat usrlibs.txt`

clean:
	rm -f symdep *.o usrlibs.txt

