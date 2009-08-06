default: libEveGNode.so

CXX      := $(shell root-config --cxx)
CXXFLAGS := $(shell root-config --cflags)
LDFLAGS  := $(shell root-config --ldflags)

SRCS := $(wildcard *.cxx)
HDRS := $(SRCS:.cxx=.h)
OBJS := $(SRCS:.cxx=.o)

test:
	echo ${SRCS}
	echo ${HDRS}
	echo ${OBJS}

DICT    := G__EveGraphNode
DICT_CC := ${DICT}.cc
DICT_H  := ${DICT}.h
DICT_O  := ${DICT}.o

TEveGraphNode.o: TEveGraphNode.cxx TEveGraphNode.h
	${CXX} -c ${CXXFLAGS} TEveGraphNode.cxx

%.o: %.cxx
	${CXX} -c $< ${CXXFLAGS} -o $@

${DICT_H} ${DICT_CC}: ${HDRS} LinkDef.h
	rootcint  -f ${DICT_CC} -c -I${ROOTSYS}/include ${HDRS} LinkDef.h

${DICT_O}: ${DICT_H} ${DICT_CC}
	${CXX} -c ${CXXFLAGS} ${DICT_CC}

libEveGNode.so: ${OBJS} ${DICT_O}
	${CXX} -shared ${LDFLAGS} ${OBJS} ${DICT_O} -o libEveGNode.so

# Tells make that clean should not check if file 'clean' exists
.PHONY: clean

clean:
	rm -rf libEveGNode.so ${OBJS} ${DICT}.*
