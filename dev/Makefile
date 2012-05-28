RC := root-config
ROOTSYS = ../trunk
include $(ROOTSYS)/etc/Makefile.arch
MATHLIBS = -lRooFit -lRooFitCore -lRooStats

#------------------------------------------------------------------------------

ifeq ($(shell $(RC) --has-roofit),yes)
TESTOBJ = test.$(ObjSuf)
TESTSRC = test.$(SrcSuf)
TEST    = test$(ExeSuf)

STRESSROOSTATSOBJ = stressRooStats.$(ObjSuf)
STRESSROOSTATSSRC = stressRooStats.$(SrcSuf)
STRESSROOSTATS    = stressRooStats$(ExeSuf)

MODELOBJ = model.$(ObjSuf)
MODELSRC = model.$(SrcSuf)
MODEL    = model$(ExeSuf)

PVALUEOBJ = pvalue.$(ObjSuf)
PVALUESRC = pvalue.$(SrcSuf)
PVALUE    = pvalue$(ExeSuf)

RFOBJ = rf501.$(ObjSuf)
RFSRC = rf501.$(SrcSuf)
RF = rf501$(ExeSuf)

endif

#----------------------------------------------------------------------------

PROGRAMS = $(TEST) $(STRESSROOSTATS) $(MODEL) $(PVALUE)
OBJS = $(TESTOBJ) $(STRESSROOSTATSOBJ) $(MODELOBJ) $(PVALUEOBJ)

$(TEST):	$(TESTOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(MODEL):    $(MODELOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(STRESSROOSTATS):	$(STRESSROOSTATSOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(PVALUE): $(PVALUEOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
	@echo "$@ done"

$(RF) : $(RFOBJ)
	$(LD) $(LDFLAGS) $^ $(LIBS) $(MATHLIBS) $(OutPutOpt)$@
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


