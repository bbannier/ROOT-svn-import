
#MAKEFLAGS = --no-print-directory -r -s -j2

LIBRARY=MakeModelAndMeasurements
OBJDIR:=$(PWD)/obj
INCDIR:=$(PWD)/inc
SRCDIR:=$(PWD)/src
LIBDIR:=$(PWD)/lib
BINDIR:=$(PWD)/bin
DEPDIR:=$(PWD)/dep


ARCH_LOC_1 := $(wildcard $(shell root-config --prefix)/test/Makefile.arch)
ARCH_LOC_2 := $(wildcard $(shell root-config --prefix)/share/root/test/Makefile.arch)
ARCH_LOC_3 := $(wildcard $(shell root-config --prefix)/share/doc/root/test/Makefile.arch)


ifneq ($(strip $(ARCH_LOC_1)),)
  $(info Using $(ARCH_LOC_1))
  include $(ARCH_LOC_1)
else
  ifneq ($(strip $(ARCH_LOC_2)),)
    $(info Using $(ARCH_LOC_2))
    include $(ARCH_LOC_2)
  else
    ifneq ($(strip $(ARCH_LOC_3)),)
      $(info Using $(ARCH_LOC_3))
      include $(ARCH_LOC_3)
    else
      $(error Could not find Makefile.arch!)
    endif
  endif
endif

VPATH    += $(OBJDIR) $(SRCDIR)
INCLUDES += -I./ -I$(INCDIR)
CXXFLAGS += -Wall -Wno-overloaded-virtual -Wno-unused

# Set the locations of some files
DICTHEAD  = $(SRCDIR)/$(LIBRARY)_Dict.h
DICTFILE  = $(SRCDIR)/$(LIBRARY)_Dict.$(SrcSuf)
DICTOBJ   = $(OBJDIR)/$(LIBRARY)_Dict.$(ObjSuf)
DICTLDEF  = $(INCDIR)/$(LIBRARY)_LinkDef.h
SKIPCPPLIST = $(DICTFILE)
SKIPHLIST   = $(DICTHEAD) $(DICTLDEF)
LIBFILE   = $(LIBDIR)/lib$(LIBRARY).a
SHLIBFILE = $(LIBDIR)/lib$(LIBRARY).$(DllSuf)
UNAME = $(shell uname)
ADDLIBS = -lRooFitCore -lRooFit -lRooStats -lFoam -lHtml -lMinuit -lThread -lXMLParser

# Set up the default targets
all: shlib $(BINDIR)/$(LIBRARY) 

$(BINDIR)/$(LIBRARY): $(SHLIBFILE)
	@echo "Linking " $@
	$(LD) $(LDFLAGS) -O2 $(OBJDIR)/MakeModelAndMeasurements.o -L$(LIBDIR) -lMakeModelAndMeasurements \
	  $(ROOTLIBS) $(ADDLIBS) -o $@

# List of all header and source files to build
HLIST   = $(filter-out $(SKIPHLIST),$(wildcard $(INCDIR)/*.h))
CPPLIST = $(filter-out $(SKIPCPPLIST),$(wildcard $(SRCDIR)/*.$(SrcSuf)))

# List of all object files to build
OLIST = $(patsubst %.$(SrcSuf),%.o,$(notdir $(CPPLIST)))

# Implicit rule to compile all sources
%.o : %.$(SrcSuf)
	@echo "Compiling $<"
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -O2 -c $< -o $(OBJDIR)/$(notdir $@) $(INCLUDES)

# Rule to create the dictionary
$(DICTFILE): $(HLIST) $(DICTLDEF)
	@echo "Generating dictionary $@" 
	@$(shell root-config --exec-prefix)/bin/rootcint -f $(DICTFILE) -c -p $(INCLUDES) $^

# Rule to comile the dictionary
$(DICTOBJ): $(DICTFILE)
	@echo "Compiling $<"
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -O2 -c $(INCLUDES) -o $@ $<

# Rule that creates various symbolic links
#setlinks: $(SHLIBFILE)
#	if [ `root-config --platform` = "macosx" -a \
#	      ! -e $(lib)/lib$(LIBRARY).so ]; then \
#	  echo "Linking $(SHLIBFILE) to $(LIBDIR)/lib$(LIBRARY).so"; \
#	  ln -s $(SHLIBFILE) $(LIBDIR)/lib$(LIBRARY).so; \
#	fi

##############################
# The dependencies section   
# - the purpose of the .d files is to keep track of the
#   header file dependence
# - this can be achieved using the makedepend command 
##############################
# .d tries to pre-process .cc
-include $(foreach var,$(notdir $(CPPLIST:.$(SrcSuf)=.d)),$(DEPDIR)/$(var))

$(DEPDIR)/%.d: %.$(SrcSuf)
	mkdir -p $(DEPDIR)
	if test -f $< ; then \
	  echo "Making $(@F)"; \
	  $(SHELL) -ec '$(CPP) -MM $(CXXFLAGS) $(INCLUDES) $< | sed '\''/Cstd\/rw/d'\'' > $@'; \
	fi

# Rule to combine objects into a unix shared library
$(SHLIBFILE): $(OLIST) $(DICTOBJ)
	mkdir -p $(LIBDIR)
	@echo "Making shared library: $(SHLIBFILE)"
	@rm -f $(SHLIBFILE)
ifneq (,$(findstring macosx,$(ARCH)))
	$(LD) $(LDFLAGS) -dynamiclib -single_module -undefined dynamic_lookup -O2 $(addprefix $(OBJDIR)/,$(OLIST)) $(DICTOBJ) -o $(SHLIBFILE)
else
	$(LD) $(LDFLAGS) $(SOFLAGS) -O2 $(addprefix $(OBJDIR)/,$(OLIST)) $(DICTOBJ) -o $(SHLIBFILE)
endif

# Useful build targets
shlib: $(SHLIBFILE)

clean:
	rm -f $(DICTFILE) $(DICTHEAD)
	rm -f $(OBJDIR)/*.o
	rm -f $(SHLIBFILE)
	rm -f $(LIBDIR)/lib$(LIBRARY).so

distclean: clean_result
	rm -rf $(OBJDIR)
	rm -rf $(DEPDIR)
	rm -f $(DICTFILE) $(DICTHEAD)
	rm -f $(SHLIBFILE)
	rm -f $(LIBDIR)/lib$(LIBRARY).so
	rm -f $(LIBDIR)/$(LIBRARY).par

clean_result:
	@rm -f results/*.eps  results/*.tex results/*.table results/*.dot results/*.root

.SUFFIXES: .$(SrcSuf)

###

.$(SrcSuf).$(ObjSuf):
	$(CXX) -g $(CXXFLAGS) -c $<
