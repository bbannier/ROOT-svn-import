# Module.mk for treeviewer module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := treeviewer
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

TREEVIEWERDIR  := $(MODDIR)
TREEVIEWERDIRS := $(TREEVIEWERDIR)/src
TREEVIEWERDIRI := $(TREEVIEWERDIR)/inc

##### libTreeViewer #####
TREEVIEWERL  := $(MODDIRI)/LinkDef.h
TREEVIEWERDS := $(MODDIRS)/G__TreeViewer.cxx
TREEVIEWERDO := $(TREEVIEWERDS:.cxx=.o)
TREEVIEWERDH := $(TREEVIEWERDS:.cxx=.h)

#TREEVIEWERH  := $(filter-out $(MODDIRI)/LinkDef%,$(wildcard $(MODDIRI)/*.h))
#TREEVIEWERS  := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
ifeq ($(ARCH),win32old)
TREEVIEWERL  := $(MODDIRI)/LinkDefWin32.h
TREEVIEWERH  := TTreeViewerOld.h TPaveVar.h
TREEVIEWERS  := TTreeViewerOld.cxx TPaveVar.cxx
else
TREEVIEWERH  := TTreeViewer.h TTVSession.h TTVLVContainer.h HelpTextTV.h TSpider.h TSpiderEditor.h TParallelCoord.h \
                TParallelCoordVar.h TParallelCoordRange.h TParallelCoordEditor.h
TREEVIEWERS  := TTreeViewer.cxx TTVSession.cxx TTVLVContainer.cxx HelpTextTV.cxx TSpider.cxx TSpiderEditor.cxx \
                TParallelCoord.cxx TParallelCoordVar.cxx TParallelCoordRange.cxx TParallelCoordEditor.cxx
endif
TREEVIEWERH  := $(patsubst %,$(MODDIRI)/%,$(TREEVIEWERH))
TREEVIEWERS  := $(patsubst %,$(MODDIRS)/%,$(TREEVIEWERS))

TREEVIEWERO  := $(TREEVIEWERS:.cxx=.o)


#LF
TREEVIEWERTMPDS    := $(MODDIRS)/G__TreeViewerTmp.cxx
TREEVIEWERTMPDO    := $(TREEVIEWERTMPDS:.cxx=.o)
TREEVIEWERTMPDH    := $(TREEVIEWERTMPDS:.cxx=.h)
TREEVIEWERTMP2DS   := $(MODDIRS)/G__TreeViewerTmp2.cxx
TREEVIEWERTMP2DO   := $(TREEVIEWERTMP2DS:.cxx=.o)
TREEVIEWERTMP2DH   := $(TREEVIEWERTMP2DS:.cxx=.h)

TREEVIEWERDEP := $(TREEVIEWERO:.o=.d) $(TREEVIEWERDO:.o=.d)

#LF
TREEVIEWERTMPDEP  := $(TREEVIEWERTMPDO:.o=.d)

TREEVIEWERLIB := $(LPATH)/libTreeViewer.$(SOEXT)
TREEVIEWERMAP := $(TREEVIEWERLIB:.$(SOEXT)=.rootmap)

#LF
TREEVIEWERNM       := $(TREEVIEWERLIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS       += $(patsubst $(MODDIRI)/%.h,include/%.h,$(TREEVIEWERH))
ALLLIBS       += $(TREEVIEWERLIB)
ALLMAPS       += $(TREEVIEWERMAP)

# include all dependency files
INCLUDEFILES += $(TREEVIEWERDEP)

##### local rules #####
include/%.h:    $(TREEVIEWERDIRI)/%.h
		cp $< $@

#LF
$(TREEVIEWERLIB):   $(TREEVIEWERO) $(TREEVIEWERTMPDO) $(TREEVIEWERTMP2DO) $(TREEVIEWERDO) $(ORDER_) \
			$(MAINLIBS) $(TREEVIEWERLIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		   "$(SOFLAGS)" libTreeViewer.$(SOEXT) $@ "$(TREEVIEWERO) \
			$(TREEVIEWERTMPDO) $(TREEVIEWERTMP2DO) $(TREEVIEWERDO)" \
		   "$(TREEVIEWERLIBEXTRA)"

#LF
$(TREEVIEWERTMPDS):   $(TREEVIEWERH) $(TREEVIEWERL) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(TREEVIEWERH) $(TREEVIEWERL)

#LF
$(TREEVIEWERTMP2DS):  $(TREEVIEWERH) $(TREEVIEWERL) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(TREEVIEWERH) $(TREEVIEWERL)

#LF
$(TREEVIEWERDS):    $(TREEVIEWERH) $(TREEVIEWERL) $(ROOTCINTTMPEXE) $(TREEVIEWERNM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(TREEVIEWERNM) -. 3 -c $(TREEVIEWERH) $(TREEVIEWERL)

#LF
$(TREEVIEWERNM):      $(TREEVIEWERO) $(TREEVIEWERTMPDO) $(TREEVIEWERTMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(TREEVIEWERTMPDO) | awk '{printf("%s\n", $$3)'} > $(TREEVIEWERNM)
		nm -p --defined-only $(TREEVIEWERTMP2DO) | awk '{printf("%s\n", $$3)'} >> $(TREEVIEWERNM)
		nm -p --defined-only $(TREEVIEWERO) | awk '{printf("%s\n", $$3)'} >> $(TREEVIEWERNM)

$(TREEVIEWERMAP): $(RLIBMAP) $(MAKEFILEDEP) $(TREEVIEWERL)
		$(RLIBMAP) -o $(TREEVIEWERMAP) -l $(TREEVIEWERLIB) \
		   -d $(TREEVIEWERLIBDEPM) -c $(TREEVIEWERL)

all-treeviewer: $(TREEVIEWERLIB) $(TREEVIEWERMAP)

clean-treeviewer:
		@rm -f $(TREEVIEWERO) $(TREEVIEWERDO)

clean::         clean-treeviewer clean-pds-treeviewer

#LF
clean-pds-treeviewer:	
		rm -f $(TREEVIEWERTMPDS) $(TREEVIEWERTMPDO) $(TREEVIEWERTMPDH) \
		$(TREEVIEWERTMPDEP) $(TREEVIEWERTMP2DS) $(TREEVIEWERTMP2DO) $(TREEVIEWERTMP2DH) $(TREEVIEWERNM)

distclean-treeviewer: clean-treeviewer
		@rm -f $(TREEVIEWERDEP) $(TREEVIEWERDS) $(TREEVIEWERDH) \
		   $(TREEVIEWERLIB) $(TREEVIEWERMAP)

distclean::     distclean-treeviewer
