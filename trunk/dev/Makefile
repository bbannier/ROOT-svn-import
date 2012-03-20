RC := root-config
ROOTSYS = ..
include $(ROOTSYS)/etc/Makefile.arch

#------------------------------------------------------------------------------

ifeq ($(shell $(RC) --has-roofit),yes)
TESTOBJ = test.$(ObjSuf)
TESTSRC = test.$(SrcSuf)
TEST    = test$(ExeSuf)

STRESSROOSTATSOBJ = stressRooStats.$(ObjSuf)
STRESSROOSTATSSRC = stressRooStats.$(SrcSuf)
STRESSROOSTATS    = stressRooStats$(ExeSuf)

endif

#----------------------------------------------------------------------------

PROGRAMS = $(TEST) $(STRESSROOSTATS)
OBJS = $(TESTOBJ) $(STRESSROOSTATSOBJ)

$(TEST):	$(TESTOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) -lRooFit -lRooFitCore -lRooStats $(OutPutOpt)$@
	@echo "$@ done"

$(STRESSROOSTATS):	$(STRESSROOSTATSOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
	@echo "$@ done"

#----------------------------------------------------------------------------

all:	$(PROGRAMS)

print:
	@echo "$(OBJS)"   

clean:
	@rm -f $(OBJS) core TrackDict.$(SrcSuf) TrackDict.h

distclean:      clean
	@rm -f $(PROGRAMS) libTrackDict.*

.SUFFIXES: .$(SrcSuf)

.$(SrcSuf).$(ObjSuf):
	$(CXX) $(CXXFLAGS) -c $<


