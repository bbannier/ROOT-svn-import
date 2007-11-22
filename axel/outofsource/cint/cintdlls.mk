# cintdlls.mk for cintdlls target
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Axel Naumann, 2006-09-14

.PHONY: cintdlls distclean-cintdlls clean-cintdlls

# no: iterator pair
# already in libCore (base/inc/Linkdef2.h): string 
CINTSTLDLLNAMES = vector vectorbool list deque map map2 set \
                  multimap multimap2 multiset \
                  stack queue exception stdexcept climits complex
CINTINCDLLNAMES = stdfunc stdcxxfunc
ifneq ($(PLATFORM),win32)
# FIX THEM!
  CINTSTLDLLNAMES += valarray
  CINTINCDLLNAMES += posix ipc
endif
# ".dll", not ".$(SOEXT)"!
CINTDLLS = $(addsuffix .dll,$(addprefix $(CINTDIRSTL)/,$(CINTSTLDLLNAMES)) \
                            $(addprefix $(CINTDIRDLLS)/,$(CINTINCDLLNAMES)))

CINTDLLNAMES = $(CINTSTLDLLNAMES) $(CINTINCDLLNAMES)

# these need dictionaries
CINTDICTDLLS =
ifneq ($(findstring vector,$(CINTDLLS)),)
CINTDICTDLLS += lib/libvectorDict.$(SOEXT)
$(CINTDIRSTL)/vector.dll: metautils/src/stlLoader_vector.o
endif
ifneq ($(findstring list,$(CINTDLLS)),)
CINTDICTDLLS += lib/liblistDict.$(SOEXT)
$(CINTDIRSTL)/list.dll: metautils/src/stlLoader_list.o
endif
ifneq ($(findstring deque,$(CINTDLLS)),)
CINTDICTDLLS += lib/libdequeDict.$(SOEXT)
$(CINTDIRSTL)/deque.dll: metautils/src/stlLoader_deque.o
endif
ifneq ($(findstring map,$(CINTDLLS)),)
CINTDICTDLLS += lib/libmapDict.$(SOEXT)
CINTDICTDLLS += lib/libmap2Dict.$(SOEXT)
$(CINTDIRSTL)/map.dll: metautils/src/stlLoader_map.o
$(CINTDIRSTL)/map.dll: metautils/src/stlLoader_map2.o
endif
ifneq ($(findstring set,$(CINTDLLS)),)
CINTDICTDLLS += lib/libsetDict.$(SOEXT)
$(CINTDIRSTL)/set.dll: metautils/src/stlLoader_set.o
endif
ifneq ($(findstring multimap,$(CINTDLLS)),)
CINTDICTDLLS += lib/libmultimapDict.$(SOEXT)
CINTDICTDLLS += lib/libmultimap2Dict.$(SOEXT)
$(CINTDIRSTL)/multimap.dll: metautils/src/stlLoader_multimap.o
$(CINTDIRSTL)/multimap.dll: metautils/src/stlLoader_multimap2.o
endif
ifneq ($(findstring multiset,$(CINTDLLS)),)
CINTDICTDLLS += lib/libmultisetDict.$(SOEXT)
$(CINTDIRSTL)/multiset.dll: metautils/src/stlLoader_multiset.o
endif
ifneq ($(findstring valarray,$(CINTDLLS)),)
CINTDICTDLLS += lib/libvalarrayDict.$(SOEXT)
$(CINTDIRSTL)/valarray.dll: metautils/src/stlLoader_valarray.o
endif

ALLCINTDLLS = $(CINTDLLS) $(CINTDICTDLLS)

# used in the main Makefile
ALLLIBS    += $(ALLCINTDLLS)

INCLUDEFILES += $(addsuffix .d,$(addprefix metautils/src/stlLoader_,$(CINTSTLDLLNAMES)))\
   $(CINTDIRL_)/posix/mktypes.d $(CINTDIRL_)/posix/exten.d

cintdlls: $(ALLCINTDLLS)

$(CINTDIRDLLSTL)/G__cpp_string.cxx:	$(CINTDIRL)/dll_stl/str.h
$(CINTDIRDLLSTL)/G__cpp_vector.cxx:	$(CINTDIRL)/dll_stl/vec.h
$(CINTDIRDLLSTL)/G__cpp_vectorbool.cxx: $(CINTDIRL)/dll_stl/vecbool.h
$(CINTDIRDLLSTL)/G__cpp_list.cxx:	$(CINTDIRL)/dll_stl/lst.h
$(CINTDIRDLLSTL)/G__cpp_deque.cxx:	$(CINTDIRL)/dll_stl/dqu.h
$(CINTDIRDLLSTL)/G__cpp_map.cxx:	$(CINTDIRL)/dll_stl/mp.h
$(CINTDIRDLLSTL)/G__cpp_map2.cxx:	$(CINTDIRL)/dll_stl/mp.h
$(CINTDIRDLLSTL)/G__cpp_set.cxx:	$(CINTDIRL)/dll_stl/st.h
$(CINTDIRDLLSTL)/G__cpp_multimap.cxx:	$(CINTDIRL)/dll_stl/multmp.h
$(CINTDIRDLLSTL)/G__cpp_multimap2.cxx:	$(CINTDIRL)/dll_stl/multmp.h
$(CINTDIRDLLSTL)/G__cpp_multiset.cxx:	$(CINTDIRL)/dll_stl/multst.h
$(CINTDIRDLLSTL)/G__cpp_stack.cxx:	$(CINTDIRL)/dll_stl/stk.h
$(CINTDIRDLLSTL)/G__cpp_queue.cxx:	$(CINTDIRL)/dll_stl/que.h
$(CINTDIRDLLSTL)/G__cpp_valarray.cxx:	$(CINTDIRL)/dll_stl/vary.h
$(CINTDIRDLLSTL)/G__cpp_exception.cxx:	$(CINTDIRL)/dll_stl/cinteh.h
$(CINTDIRDLLSTL)/G__cpp_stdexcept.cxx:	$(CINTDIRL)/dll_stl/se.h
$(CINTDIRDLLSTL)/G__cpp_climits.cxx:	$(CINTDIRL)/dll_stl/clim.h
$(CINTDIRDLLSTL)/G__cpp_complex.cxx:	$(CINTDIRL)/dll_stl/cmplx.h
$(CINTDIRDLLSTL)/G__cpp_iterator.cxx:	$(CINTDIRL)/dll_stl/iter.h
$(CINTDIRDLLSTL)/G__cpp_pair.cxx:	$(CINTDIRL)/dll_stl/pr.h

$(CINTDIRL_)/G__cpp_stdcxxfunc.cxx: 	$(CINTDIRL)/stdstrct/stdcxxfunc.h
$(CINTDIRL_)/G__c_stdfunc.c:		$(CINTDIRL)/stdstrct/stdfunc.h
$(CINTDIRL_)/G__c_posix.c:		$(CINTDIRL)/posix/exten.h
$(CINTDIRL_)/G__c_ipc.c:		$(CINTDIRL)/ipc/ipcif.h

FAVOR_SYSINC := -I-
ifeq ($(PLATFORM),sgi)
   FAVOR_SYSINC =
endif
CINTDLLINCDIRS := -I. -I$(CINTDIRDLLSTL) $(FAVOR_SYSINC)
ifeq ($(GCC_MAJOR),4)
  CINTDLLINCDIRS := -iquote. -iquote$(CINTDIRDLLSTL)
endif
$(CINTDLLS): CINTCXXFLAGS += $(CINTDLLINCDIRS)

##### all cintdlls end on .dll
ifneq ($(SOEXT),dll)
CINTDLLSOEXTCMD = mv $(@:.dll=.$(SOEXT)) $@
ifeq ($(PLATFORM),macosx)
ifeq ($(subst $(MACOSX_MINOR),,456789),456789)
# MACOSX_MINOR < 4
  CINTDLLSOEXTCMD += ;mv $(@:.dll=.so) $@
else
  CINTDLLSOEXTCMD += ;rm -f $(@:.dll=.so)
endif
endif # macosx
endif # need to mv to .dll
##### all cintdlls end on .dll - END

# Filter out the explicit link flag
ifneq ($(subst build/unix/makelib.sh,,$(MAKELIB)),$(MAKELIB))
  $(CINTDLLS): MAKELIB := $(subst -x,,$(MAKELIB))
endif

$(CINTDIRSTL)/%.dll: $(CINTDIRDLLSTL)/G__cpp_%.o $(ORDER_) $(MAINLIBS)
	@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" "$(SOFLAGS)" $(notdir $(@:.dll=.$(SOEXT))) $(@:.dll=.$(SOEXT)) $(filter-out $(MAINLIBS),$^)
	$(CINTDLLSOEXTCMD)
ifneq ($(subst win,,$(ARCH)),$(ARCH))
	@rm -f $(@:.dll=.lib) $(@:.dll=.exp) # remove import libs
endif

$(CINTDIRDLLS)/%.dll: $(CINTDIRL_)/G__c_%.o $(ORDER_) $(MAINLIBS)
	@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" "$(SOFLAGS)" $(notdir $(@:.dll=.$(SOEXT))) $(@:.dll=.$(SOEXT)) $(filter-out $(MAINLIBS),$^)
	$(CINTDLLSOEXTCMD)

metautils/src/stlLoader_%.cc: $(SRCDIR)/metautils/src/stlLoader.cc
	cp -f $< $@

metautils/src/stlLoader_%.o: metautils/src/stlLoader_%.cc
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- $(CINTCXXFLAGS) -D__cplusplus -- $<
	$(CXX) $(OPT) $(CINTCXXFLAGS) $(INCDIRS) -DWHAT=\"$*\" $(CXXOUT)$@ -c $<

$(CINTDIRDLLSTL)/G__cpp_%.cxx: $(CINTTMP) $(IOSENUM)
	$(CINTTMP) -w1 -z$(notdir $*) -n$@ $(subst $*,,$(patsubst %map2,-DG__MAP2,$*)) \
	   -D__MAKECINT__ -DG__MAKECINT -I$(CINTDIRDLLSTL) -I$(CINTDIRL) \
	   -c-1 -A -Z0 $(filter-out $(IOSENUM),$(filter %.h,$^))

$(CINTDIRDLLINC)/G__cpp_%.cxx: $(CINTTMP) $(IOSENUM)
	$(CINTTMP) -w1 -z$(notdir $*) -n$@ $(subst $*,,$(patsubst %map2,-DG__MAP2,$*)) \
	   -D__MAKECINT__ -DG__MAKECINT -I$(CINTDIRDLLSTL) -I$(CINTDIRL) \
	   -c-1 -A -Z0 $(filter-out $(IOSENUM),$(filter %.h,$^))

$(CINTDIRL_)/G__c_%.c: $(CINTTMP) $(IOSENUM)
	$(CINTTMP) -K -w1 -z$(notdir $*) -n$@ -D__MAKECINT__ -DG__MAKECINT \
	   $(MACOSX_UNIX03) -c-2 -Z0 $(filter-out $(IOSENUM),$(filter %.h,$^))

ifeq ($(subst $(MACOSX_MINOR),,1234),1234)
# MACOSX_MINOR > 4
$(CINTDIRL_)/G__c_posix.c: MACOSX_UNIX03 = -D__DARWIN_UNIX03
endif

$(CINTDIRL_)/G__c_%.o: CINTCFLAGS += -I. -DG__SYSTYPES_H
$(CINTDIRDLLSTL)/G__cpp_%.o: $(CINTDIRDLLSTL)/G__cpp_%.cxx
	mkdir -p $(dir $@)
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- \
	  $(CXXFLAGS) -D__cplusplus -I$(CINTDIR)/lib/prec_stl \
	  -I$(CINTDIR)/stl -- $<
	$(CXX) $(NOOPT) $(CXXFLAGS) -I. $(CXXOUT)$@ -c $<
$(CINTDIRDLLINC)/G__cpp_%.o: $(CINTDIRDLLINC)/G__cpp_%.cxx
	mkdir -p $(dir $@)
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- \
	  $(CXXFLAGS) -D__cplusplus -I$(CINTDIR)/lib/prec_stl \
	  -I$(CINTDIR)/stl -- $<
	$(CXX) $(NOOPT) $(CXXFLAGS) -I. $(CXXOUT)$@ -c $<

$(CINTDIRL_)/G__c_stdfunc.o $(CINTDIRL_)/G__c_posix.o $(CINTDIRL_)/G__c_ipc.o: \
$(CINTDIRL_)/G__c_%.o: $(CINTDIRL_)/G__c_%.c
	mkdir -p $(dir $@)
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- \
	  $(CINTCFLAGS) -I$(CINTDIR)/lib/prec_stl \
	  -I$(CINTDIR)/stl -- $<
	$(CC) $(OPT) $(CINTCFLAGS) $(CXXOUT)$@ -c $<

##### posix special treatment
$(CINTDIRL_)/posix/exten.o: $(CINTDIRL)/posix/exten.c
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- $(CINTCFLAGS) -- $<
	$(CC) $(OPT) $(CINTCFLAGS) -I. $(CXXOUT)$@ -c $<

$(CINTDIRL_)/G__c_posix.c: $(CINTDIRDLLS)/sys/types.h $(CINTDIRL)/posix/exten.h $(CINTDIRL)/posix/posix.h

$(CINTDIRDLLS)/posix.dll: $(CINTDIRL_)/G__c_posix.o $(CINTDIRL_)/posix/exten.o

$(CINTDIRDLLS)/sys/types.h: $(CINTDIRL_)/posix/mktypes$(EXEEXT)
	(cd $(dir $<) && \
	./$(notdir $<))
	cp -f $(CINTDIRDLLS)/systypes.h $@

$(CINTDIRL_)/posix/mktypes$(EXEEXT): $(CINTDIRL)/posix/mktypes.c
	mkdir -p $(dir $@)
	$(MAKEDEP) -R -f$@.d -Y -w 1000 -- $(CINTCFLAGS) -- $< 
	$(CC) $(OPT) $(CXXOUT)$@ $<
##### posix special treatment - END

##### ipc special treatment
$(CINTDIRDLLS)/sys/ipc.dll: $(CINTDIRL_)/G__c_ipc.o

##### ipc special treatment - END

##### dictionaries
$(patsubst lib/lib%Dict.$(SOEXT),$(CINTDIRDLLSTL)/rootcint_%.cxx,$(CINTDICTDLLS)): \
$(CINTDIRDLLSTL)/rootcint_%.cxx: $(ROOTCINTTMPEXE)
	$(ROOTCINTTMP) -f $@ -c \
	   $(subst $*,,$(patsubst %map2,-DG__MAP2,$*)) $(subst multi,,${*:2=}) \
	   $(SRCDIR)/metautils/src/${*:2=}Linkdef.h

$(patsubst lib/lib%Dict.$(SOEXT),$(CINTDIRDLLSTL)/rootcint_%.o,$(CINTDICTDLLS)): CINTCXXFLAGS += -I.

rootcint_%.o: rootcint_%.cxx
	$(MAKEDIR)
	$(MAKEDEP) -R -f$(patsubst %.o,%.d,$@) -Y -w 1000 -- \
	   $(CXXFLAGS) -D__cplusplus -I$(CINTDIR)/lib/prec_stl \
	   -I$(CINTDIR)/stl -- $<
	$(CXX) $(NOOPT) $(CXXFLAGS) -I. $(CXXOUT)$@ -c $<


$(CINTDICTDLLS): lib/lib%Dict.$(SOEXT): $(CINTDIRDLLSTL)/rootcint_%.o $(ORDER_) $(MAINLIBS)
	@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" "$(SOFLAGS)" $(notdir $@) $@ $(filter-out $(MAINLIBS),$^)
##### dictionaries - END

##### clean

# remove only .o, .dll, .$(SOEXT)
clean-cintdlls:
	@(for cintdll in $(CINTDLLNAMES); do \
	  rm -f $(CINTDIRDLLSTL)/rootcint_$${cintdll}.o \
	  $(CINTDIRDLLSTL)/G__cpp_$${cintdll}.o \
	  $(CINTDIRL_)/G__c_$${cintdll}.o \
	  metautils/src/stlLoader_$${cintdll}.o; done)
	@rm -f $(ALLCINTDLLS) \
	  $(CINTDIRL_)/posix/exten.o $(CINTDIRDLLS)/posix.* \
	  $(CINTDIRDLLS)/ipc.*

clean:: clean-cintdlls

# remove generated code, too.
distclean-cintdlls: clean-cintdlls
	@(for cintdll in $(CINTDLLNAMES); do \
	  rm -f $(CINTDIRDLLSTL)/rootcint_$${cintdll}.* \
	  $(CINTDIRDLLSTL)/G__cpp_$${cintdll}.* \
	  $(CINTDIRL_)/G__c_$${cintdll}.* \
	  metautils/src/stlLoader_$${cintdll}.*; done)
	@rm -f $(ALLCINTDLLS) \
	  $(CINTDIRL_)/posix/mktypes$(EXEEXT)
ifeq ($(PLATFORM),macosx)
	@rm -f  $(CINTDIRSTL)/*.so
	@rm -rf $(CINTDIRL_)/posix/mktypes.dSYM
endif

distclean:: distclean-cintdlls

##### clean - END
