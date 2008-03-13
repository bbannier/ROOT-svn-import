# $Header$

LD       = ${CXX}
LDEXT    = so

MAKEDEPEND = ${ROOTSYS}/bin/rmkdepend -Y -w 666 -f-
MKDEPINC  := make_depend.inc
ROOTCINT   = ${ROOTSYS}/bin/rootcint

CPPFLAGS = -I. -I${ROOTSYS}/include
CXXFLAGS = -g -fPIC -Wall -Woverloaded-virtual
LDFLAGS  = -shared

ROOTCINTFLAGS = -I.

#--------------------------------------------------------------

SRCS := $(wildcard *.cxx)
HDRS := $(SRCS:.cxx=.h)
OBJS := $(SRCS:.cxx=.o)
LIBS := libQuakeViz.${LDEXT}
EXES :=

ALLHDRS := ${HDRS} LinkDef.h
ALLOBJS := ${OBJS} QuakeViz_Dict.o

#--------------------------------------------------------------

.SUFFIXES:  .cxx .cc .h .o
.SECONDARY: ${ALLOBJS} 

.SILENT: ${ALLOBJS} 

%.o: %.cc
	@echo --- Compiling $<
	${CXX} ${CPPFLAGS} -c -o $@ ${CXXFLAGS} $<

%.o: %.cxx
	@echo --- Compiling $<
	${CXX} ${CPPFLAGS} -c -o $@ ${CXXFLAGS} $<

lib%.so: ${ALLOBJS}
	${LD} ${LDFLAGS} -o $@ $^ ${ROOTLIBS} ${EXTLIBS}

all: ${LIBS} ${EXES}

QuakeViz_Dict.cc: ${ALLHDRS}
	rootcint -f $@ -c -p ${ROOTCINTFLAGS} ${ALLHDRS}

#--------------------------------------------------------------

depend:
	@rm -f ${MKDEPINC}

${MKDEPINC}: ${HDRS} ${SRCS}
	@echo "Creating dependencies."
	@${MAKEDEPEND} -- -- ${SRCS} > ${MKDEPINC} 2>/dev/null

clean:
	@rm -f ${ALLOBJS} *_Dict.* ${LIBS} ${EXES}

distclean: clean
	@rm -f ${MKDEPINC}
	@(rm -f $(notdir ${LIBS}))
	@(rm -f $(notdir ${EXES}))

#--------------------------------------------------------------

echo_hdrs:
	@echo ${HDRS}

echo_srcs:
	@echo ${SRCS}

echo_objs:
	@echo ${OBJS}

echo_libs:
	@echo ${LIBS}

#--------------------------------------------------------------

-include ${MKDEPINC}
