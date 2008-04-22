###############################################################################

include $(ROOTSYS)/test/Makefile.arch

###############################################################################

ifeq ($(PLATFORM),win32)
EXTRAFLAGS   =
EXTRALIBS    = "$(ROOTSYS)/lib/libRGL.lib" "$(ROOTSYS)/lib/libGed.lib" \
               "$(ROOTSYS)/lib/libEve.lib" OpenGL32.lib Glu32.lib
else
EXTRAFLAGS   =
EXTRALIBS    = 
endif

###############################################################################

#SRCS := $(wildcard *.$(SrcSuf))
SRCS := $(filter-out QuakeVizDict.$(SrcSuf),$(wildcard *.$(SrcSuf)))
HDRS := $(SRCS:.$(SrcSuf)=.h)
OBJS := $(SRCS:.$(SrcSuf)=.$(ObjSuf))
QUAKEVIZSO := libQuakeViz.$(DllSuf)
ifeq ($(PLATFORM),win32)
QUAKEVIZLIB  := libQuakeViz.lib
else
QUAKEVIZLIB  := $(QUAKEVIZSO)
endif

ALLHDRS := ${HDRS} LinkDef.h
ALLOBJS := ${OBJS} QuakeVizDict.$(ObjSuf)

###############################################################################

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)

all: $(QUAKEVIZSO)

$(QUAKEVIZSO): $(ALLOBJS)
ifeq ($(PLATFORM),win32)
	bindexplib $* $^ > $*.def
	lib -nologo -MACHINE:IX86 $^ -def:$*.def \
	   $(OutPutOpt)$(QUAKEVIZLIB)
	$(LD) $(SOFLAGS) $(LDFLAGS) $^ $*.exp $(ROOTGLIBS) \
	   $(EXTRALIBS) $(OutPutOpt)$@
	$(MT_DLL)
else
	$(LD) $(SOFLAGS) $(LDFLAGS) $^ $(ROOTGLIBS) $(EXTRALIBS) $(OutPutOpt)$@
endif
	@echo "$@ done"

###############################################################################

QuakeVizDict.$(SrcSuf): $(ALLHDRS)
		@echo "Generating dictionary $@..."
		@rootcint -f $@ -c $^

clean:
		rm -f *.o *.obj *.res *.pdb *.def *.exp \
		      *.lib *.ilk *.manifest *.d *.def *Dict.*

distclean:  clean
		@rm -f $(QUAKEVIZSO)

###############################################################################

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) $(EXTRAFLAGS) -c $<

###############################################################################


