# Module.mk for gui module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

MODDIR       := gui
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

GUIDIR       := $(MODDIR)
GUIDIRS      := $(GUIDIR)/src
GUIDIRI      := $(GUIDIR)/inc

##### libGui #####
GUIL1        := $(MODDIRI)/LinkDef1.h
GUIL2        := $(MODDIRI)/LinkDef2.h
GUIL3        := $(MODDIRI)/LinkDef3.h
GUIDS1       := $(MODDIRS)/G__Gui1.cxx
GUIDS2       := $(MODDIRS)/G__Gui2.cxx
GUIDS3       := $(MODDIRS)/G__Gui3.cxx
GUIDO1       := $(GUIDS1:.cxx=.o)
GUIDO2       := $(GUIDS2:.cxx=.o)
GUIDO3       := $(GUIDS3:.cxx=.o)
GUIL         := $(GUIL1) $(GUIL2) $(GUIL3)
GUIDS        := $(GUIDS1) $(GUIDS2) $(GUIDS3)
GUIDO        := $(GUIDO1) $(GUIDO2) $(GUIDO3)
GUIDH        := $(GUIDS:.cxx=.h)

#LF
GUITMPDS1    := $(MODDIRS)/G__Gui1Tmp.cxx
GUITMPDO1    := $(GUITMPDS1:.cxx=.o)
GUITMPDH1    := $(GUITMPDS1:.cxx=.h)
GUITMP2DS1   := $(MODDIRS)/G__Gui1Tmp2.cxx
GUITMP2DO1   := $(GUITMP2DS1:.cxx=.o)
GUITMP2DH1   := $(GUITMP2DS1:.cxx=.h)

#LF
GUITMPDS2    := $(MODDIRS)/G__Gui2Tmp.cxx
GUITMPDO2    := $(GUITMPDS2:.cxx=.o)
GUITMPDH2    := $(GUITMPDS2:.cxx=.h)
GUITMP2DS2   := $(MODDIRS)/G__Gui2Tmp2.cxx
GUITMP2DO2   := $(GUITMP2DS2:.cxx=.o)
GUITMP2DH2   := $(GUITMP2DS2:.cxx=.h)

#LF
GUITMPDS3    := $(MODDIRS)/G__Gui3Tmp.cxx
GUITMPDO3    := $(GUITMPDS3:.cxx=.o)
GUITMPDH3    := $(GUITMPDS3:.cxx=.h)
GUITMP2DS3   := $(MODDIRS)/G__Gui3Tmp2.cxx
GUITMP2DO3   := $(GUITMP2DS3:.cxx=.o)
GUITMP2DH3   := $(GUITMP2DS3:.cxx=.h)


GUIH1        := TGObject.h TGClient.h TGWindow.h TGPicture.h TGDimension.h \
                TGFrame.h TGLayout.h TGString.h TGWidget.h TGIcon.h TGLabel.h \
                TGButton.h TGTextBuffer.h TGTextEntry.h TGMsgBox.h TGMenu.h \
                TGGC.h TGShutter.h TG3DLine.h TGProgressBar.h TGButtonGroup.h \
                TGNumberEntry.h TGTableLayout.h WidgetMessageTypes.h \
                TGIdleHandler.h TGInputDialog.h
GUIH2        := TGObject.h TGScrollBar.h TGCanvas.h TGListBox.h TGComboBox.h \
                TGTab.h TGSlider.h TGPicture.h TGListView.h TGMimeTypes.h \
                TGFSContainer.h TGFileDialog.h TGStatusBar.h TGToolTip.h \
                TGToolBar.h TGListTree.h TGText.h TGView.h TGTextView.h \
                TGTextEdit.h TGTextEditDialogs.h TGDoubleSlider.h TGSplitter.h \
                TGFSComboBox.h TGImageMap.h TGApplication.h TGXYLayout.h \
                TGResourcePool.h TGFont.h TGTripleSlider.h
GUIH3        := TRootGuiFactory.h TRootApplication.h TRootCanvas.h \
                TRootBrowser.h TRootContextMenu.h TRootDialog.h \
                TRootControlBar.h TRootHelpDialog.h TRootEmbeddedCanvas.h \
                TGColorDialog.h TGColorSelect.h TGFontDialog.h \
                TGDockableFrame.h TGMdi.h TGMdiFrame.h TGMdiMainFrame.h \
                TGMdiDecorFrame.h TGMdiMenu.h TVirtualDragManager.h \
                TGuiBuilder.h TGRedirectOutputGuard.h TGPasswdDialog.h \
                TGTextEditor.h TGSpeedo.h TGDNDManager.h

GUIH4        := HelpText.h
GUIH1        := $(patsubst %,$(MODDIRI)/%,$(GUIH1))
GUIH2        := $(patsubst %,$(MODDIRI)/%,$(GUIH2))
GUIH3        := $(patsubst %,$(MODDIRI)/%,$(GUIH3))
GUIH4        := $(patsubst %,$(MODDIRI)/%,$(GUIH4))
GUIH         := $(GUIH1) $(GUIH2) $(GUIH3) $(GUIH4)
GUIS         := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
GUIO         := $(GUIS:.cxx=.o)

#LF
GUITMPDS       := $(GUITMPDS1) $(GUITMPDS2) $(GUITMPDS3)
GUITMPDO       := $(GUITMPDO1) $(GUITMPDO2) $(GUITMPDO3)
GUITMP2DS      := $(GUITMP2DS1) $(GUITMP2DS2) $(GUITMP2DS3)
GUITMP2DO      := $(GUITMP2DO1) $(GUITMP2DO2) $(GUITMP2DO3)

GUIDEP       := $(GUIO:.o=.d) $(GUIDO:.o=.d)

#LF
GUITMPDEP   := $(GUITMPDO:.o=.d)

#LF
GUILIB       := $(LPATH)/libGui.$(SOEXT)
GUIDICTLIB   := $(LPATH)/libGuiDict.$(SOEXT)
GUIMAP      := $(GUILIB:.$(SOEXT)=.rootmap)
GUIDICTMAP   := $(GUIDICTLIB:.$(SOEXT)=.rootmap)
GUINM        := $(GUILIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS     += $(patsubst $(MODDIRI)/%.h,include/%.h,$(GUIH))
ALLLIBS     += $(GUILIB)
ALLMAPS     += $(GUIMAP)

# include all dependency files
INCLUDEFILES += $(GUIDEP)

##### local rules #####
include/%.h:    $(GUIDIRI)/%.h
		cp $< $@

#LF
$(GUILIB):      $(GUIO) $(GUITMPDO) $(GUITMP2DO) $(GUIDO) $(ORDER_) $(MAINLIBS) $(GUILIBDEP)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libGui.$(SOEXT) $@ "$(GUIO) $(GUITMPDO) $(GUITMP2DO) $(GUIDO)"\
		"$(GUILIBEXTRA)"
#LF
#$(GUIDICTLIB):  $(GUIDO) $(ORDER_) $(MAINLIBS) $(GUIDICTLIBDEP) $(GUITMP2DO)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libGuiDict.$(SOEXT) $@ "$(GUIDO) $(GUITMP2DO)"\
#		"$(GUIDICTLIBEXTRA)"

#LF
$(GUITMPDS1):   $(GUIH1) $(GUIL1) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GUIH1) $(GUIL1)
#LF
$(GUITMP2DS1):  $(GUIH1) $(GUIL1) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GUIH1) $(GUIL1)
#LF
$(GUIDS1):      $(GUIH1) $(GUIL1) $(ROOTCINTTMPEXE) $(GUINM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GUINM) -. 3 -c $(GUIH1) $(GUIL1)

#LF
$(GUITMPDS2):   $(GUIH2) $(GUIL2) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GUIH2) $(GUIL2)
#LF
$(GUITMP2DS2):  $(GUIH2) $(GUIL2) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GUIH2) $(GUIL2)
#LF
$(GUIDS2):     $(GUIH2) $(GUIL2) $(ROOTCINTTMPEXE) $(GUINM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GUINM) -. 3 -c $(GUIH2) $(GUIL2)

#LF
$(GUITMPDS3):   $(GUIH3) $(GUIL3) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(GUIH3) $(GUIL3)
#LF
$(GUITMP2DS3):  $(GUIH3) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(GUIH3) $(GUIL3)
#LF
$(GUIDS3):     $(GUIH3) $(GUIL3) $(ROOTCINTTMPEXE) $(GUINM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(GUINM) -. 3 -c $(GUIH3) $(GUIL3)

#LF
#$(GUIDICTMAP):  $(RLIBMAP) $(MAKEFILEDEP) $(GUIL)
#		$(RLIBMAP) -o $(GUIDICTMAP) -l $(GUIDICTLIB) \
#		-d $(GUILIB) $(GUILIBDEPM) -c $(GUIL)

#LF
$(GUINM):      $(GUIO) $(GUITMPDO) $(GUITMP2DO) 
		@echo "Generating symbols file $@..."
		nm -p --defined-only $(GUITMPDO1) | awk '{printf("%s\n", $$3)'} > $(GUINM)
		nm -p --defined-only $(GUITMPDO2) | awk '{printf("%s\n", $$3)'} >> $(GUINM)
		nm -p --defined-only $(GUITMPDO3) | awk '{printf("%s\n", $$3)'} >> $(GUINM)
		nm -p --defined-only $(GUITMP2DO1) | awk '{printf("%s\n", $$3)'} >> $(GUINM)
		nm -p --defined-only $(GUITMP2DO2) | awk '{printf("%s\n", $$3)'} >> $(GUINM)
		nm -p --defined-only $(GUITMP2DO3) | awk '{printf("%s\n", $$3)'} >> $(GUINM)
		nm -p --defined-only $(GUIO) | awk '{printf("%s\n", $$3)'} >> $(GUINM)

$(GUIMAP):      $(RLIBMAP) $(MAKEFILEDEP) $(GUIL)
		$(RLIBMAP) -o $(GUIMAP) -l $(GUILIB) \
		   -d $(GUILIBDEPM) -c $(GUIL)

all-gui:        $(GUILIB) $(GUIMAP)

clean-gui:
		@rm -f $(GUIO) $(GUIDO)

clean::         clean-gui clean-pds-gui

#LF
clean-pds-gui:	
		rm -f $(GUITMPDS) $(GUITMPDO) $(GUITMPDH) \
		$(GUITMPDEP) $(GUITMP2DS) $(GUITMP2DO) $(GUITMP2DH) $(GUINM)

distclean-gui:  clean-gui
		@rm -f $(GUIDEP) $(GUIDS) $(GUIDH) $(GUILIB) $(GUIMAP)

distclean::     distclean-gui
