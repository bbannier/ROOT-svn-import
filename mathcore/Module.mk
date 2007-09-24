# Module.mk for mathcore module
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 20/6/2005

MODDIR       := mathcore
MODDIRS      := $(MODDIR)/src
MODDIRI      := $(MODDIR)/inc

MATHCOREDIR  := $(MODDIR)
MATHCOREDIRS := $(MATHCOREDIR)/src
MATHCOREDIRI := $(MATHCOREDIR)/inc

##### libMathCore #####
MATHCOREL    := $(MODDIRI)/Math/LinkDef.h
MATHCOREL2   := $(MODDIRI)/Math/LinkDef2.h
MATHCOREL32  := $(MODDIRI)/Math/LinkDef_GenVector32.h
MATHCORELINC := $(MODDIRI)/Math/LinkDef_Func.h \
                $(MODDIRI)/Math/LinkDef_GenVector.h \
                $(MODDIRI)/Math/LinkDef_Point3D.h \
                $(MODDIRI)/Math/LinkDef_Vector3D.h \
                $(MODDIRI)/Math/LinkDef_Vector4D.h \
                $(MODDIRI)/Math/LinkDef_Rotation.h
MATHCOREDS   := $(MODDIRS)/G__MathCore.cxx
MATHCOREDS32 := $(MODDIRS)/G__MathCore32.cxx
MATHCOREDO   := $(MATHCOREDS:.cxx=.o)
MATHCOREDO32 := $(MATHCOREDS32:.cxx=.o)
MATHCOREDH   := $(MATHCOREDS:.cxx=.h)


#LF
MATHCORETMPDS    := $(MODDIRS)/G__MathCoreTmp.cxx
MATHCORETMPDO    := $(MATHCORETMPDS:.cxx=.o)
MATHCORETMPDH    := $(MATHCORETMPDS:.cxx=.h)
MATHCORETMP2DS   := $(MODDIRS)/G__MathCoreTmp2.cxx
MATHCORETMP2DO   := $(MATHCORETMP2DS:.cxx=.o)
MATHCORETMP2DH   := $(MATHCORETMP2DS:.cxx=.h)

#LF
MATHCORETMPDS32    := $(MODDIRS)/G__MathCore32Tmp.cxx
MATHCORETMPDO32    := $(MATHCORETMPDS32:.cxx=.o)
MATHCORETMPDH32    := $(MATHCORETMPDS32:.cxx=.h)
MATHCORETMP2DS32    := $(MODDIRS)/G__MathCore32Tmp2.cxx
MATHCORETMP2DO32    := $(MATHCORETMP2DS32:.cxx=.o)
MATHCORETMP2DH32    := $(MATHCORETMP2DS32:.cxx=.h)


MATHCOREDH1  :=  $(MODDIRI)/Math/Vector3D.h \
                 $(MODDIRI)/Math/Point3D.h \
                 $(MODDIRI)/Math/Vector4D.h \
                 $(MODDIRI)/Math/Vector2D.h \
                 $(MODDIRI)/Math/Point2D.h \
                 $(MODDIRI)/Math/Rotation3D.h \
                 $(MODDIRI)/Math/RotationZYX.h \
                 $(MODDIRI)/Math/RotationX.h \
                 $(MODDIRI)/Math/RotationY.h \
                 $(MODDIRI)/Math/RotationZ.h \
                 $(MODDIRI)/Math/LorentzRotation.h \
                 $(MODDIRI)/Math/Boost.h    \
                 $(MODDIRI)/Math/BoostX.h    \
                 $(MODDIRI)/Math/BoostY.h    \
                 $(MODDIRI)/Math/BoostZ.h    \
                 $(MODDIRI)/Math/EulerAngles.h \
                 $(MODDIRI)/Math/AxisAngle.h \
                 $(MODDIRI)/Math/Quaternion.h \
                 $(MODDIRI)/Math/Transform3D.h \
                 $(MODDIRI)/Math/Translation3D.h \
                 $(MODDIRI)/Math/Plane3D.h \
                 $(MODDIRI)/Math/SpecFuncMathCore.h \
                 $(MODDIRI)/Math/DistFuncMathCore.h \
                 $(MODDIRI)/Math/IParamFunction.h \
                 $(MODDIRI)/Math/IFunction.h \
                 $(MODDIRI)/Math/VectorUtil_Cint.h

MATHCOREDH2  :=  $(MODDIRI)/Math/Vector3D.h \
                 $(MODDIRI)/Math/Point3D.h \
                 $(MODDIRI)/Math/Vector4D.h \
                 $(MODDIRI)/Math/Vector2D.h \
                 $(MODDIRI)/Math/Point2D.h \
                 $(MODDIRI)/Math/Rotation3D.h \
                 $(MODDIRI)/Math/RotationZYX.h \
                 $(MODDIRI)/Math/RotationX.h \
                 $(MODDIRI)/Math/RotationY.h \
                 $(MODDIRI)/Math/RotationZ.h \
                 $(MODDIRI)/Math/LorentzRotation.h \
                 $(MODDIRI)/Math/Boost.h    \
                 $(MODDIRI)/Math/BoostX.h    \
                 $(MODDIRI)/Math/BoostY.h    \
                 $(MODDIRI)/Math/BoostZ.h

MATHCOREDH132:=  $(MODDIRI)/Math/Vector3D.h \
                 $(MODDIRI)/Math/Point3D.h \
                 $(MODDIRI)/Math/Vector4D.h \



MATHCOREAH   := $(filter-out $(MODDIRI)/Math/LinkDef%, $(wildcard $(MODDIRI)/Math/*.h))
MATHCOREGVH  := $(filter-out $(MODDIRI)/Math/GenVector/LinkDef%, $(wildcard $(MODDIRI)/Math/GenVector/*.h))
MATHCOREH    := $(MATHCOREAH) $(MATHCOREGVH)
MATHCORES    := $(filter-out $(MODDIRS)/G__%,$(wildcard $(MODDIRS)/*.cxx))
MATHCOREO    := $(MATHCORES:.cxx=.o)

MATHCOREDEP  := $(MATHCOREO:.o=.d)  $(MATHCOREDO:.o=.d) $(MATHCOREDO32:.o=.d)

#LF
MATHCORETMPDEP   := $(MATHCORETMPDO:.o=.d)  $(MATHCORETMP2DO:.o=.d) $(MATHCORETMPDO32:.o=.d) $(MATHCORETMP2DO32:.o=.d)

MATHCORELIB  := $(LPATH)/libMathCore.$(SOEXT)

#LF
MATHCOREMAP  := $(MATHCORELIB:.$(SOEXT)=.rootmap)
#MATHCOREDICTLIB  := $(LPATH)/libMathCoreDict.$(SOEXT)
#MATHCOREDICTMAP  := $(MATHCOREDICTLIB:.$(SOEXT)=.rootmap)

#LF
MATHCORENM       := $(MATHCORELIB:.$(SOEXT)=.nm)

# used in the main Makefile
ALLHDRS      += $(patsubst $(MODDIRI)/Math/%.h,include/Math/%.h,$(MATHCOREH))
ALLLIBS      += $(MATHCORELIB) $(MATHCOREDICTLIB)
ALLMAPS      += $(MATHCOREMAP)

# include all dependency files
INCLUDEFILES += $(MATHCOREDEP)

##### local rules #####
include/Math/%.h: $(MATHCOREDIRI)/Math/%.h
		@(if [ ! -d "include/Math/GenVector" ]; then   \
		   mkdir -p include/Math/GenVector;       \
		fi)
		cp $< $@

#LF
$(MATHCORELIB): $(MATHCOREO) $(MATHCORETMPDO) $(MATHCORETMP2DO) $(MATHCOREDO) \
		$(MATHCORETMPDO32) $(MATHCORETMP2DO32) $(MATHCOREDO32) $(ORDER_) $(MAINLIBS)
		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
		"$(SOFLAGS)" libMathCore.$(SOEXT) $@ "$(MATHCOREO) $(MATHCORETMPDO) $(MATHCORETMP2DO) $(MATHCOREDO) $(MATHCORETMPDO32) $(MATHCORETMP2DO32) $(MATHCOREDO32)" \
		"$(MATHCORELIBEXTRA)"

#LF
# There are no inline functions so we ignore Tmp2
#$(MATHCOREDICTLIB): $(MATHCOREDO) $(ORDER_) $(MAINLIBS)
#		@$(MAKELIB) $(PLATFORM) $(LD) "$(LDFLAGS)" \
#		"$(SOFLAGS)" libMathCoreDict.$(SOEXT) $@ "$(MATHCOREDO)"\
#		"$(MATHCOREDICTLIBEXTRA)"

#LF
$(MATHCORETMPDS):   $(MATHCOREDH2) $(MATHCOREL2) $(MATHCORELINC) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MATHCOREDH2) $(MATHCOREL2)

#LF
$(MATHCORETMP2DS):  $(MATHCOREDH2) $(MATHCOREL2) $(MATHCORELINC) $(MATHCORELINC) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MATHCOREDH2) $(MATHCOREL2)

#LF
$(MATHCOREDS):      $(MATHCOREDH1) $(MATHCOREL) $(MATHCORELINC) $(ROOTCINTTMPEXE) $(MATHCORENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MATHCORENM) -. 3 -c $(MATHCOREDH1) $(MATHCOREL)



#LF
$(MATHCORETMPDS32):   $(MATHCOREDH132) $(MATHCOREL32) $(MATHCORELINC) $(ROOTCINTTMPEXE)
		@echo "Generating first dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 1 -c $(MATHCOREDH132) $(MATHCOREL32)

#LF
$(MATHCORETMP2DS32):  $(MATHCOREDH132) $(MATHCOREL32) $(MATHCORELINC) $(ROOTCINTTMPEXE)
		@echo "Generating second dictionary $@..."
		$(ROOTCINTTMP) -f $@ -. 2 -c $(MATHCOREDH132) $(MATHCOREL32)

#LF
$(MATHCOREDS32):      $(MATHCOREDH132) $(MATHCOREL32) $(MATHCORELINC) $(ROOTCINTTMPEXE) $(MATHCORENM)
		@echo "Generating third dictionary $@..."
		$(ROOTCINTTMP) -f $@ -L $(ROOTSYS)/$(MATHCORENM) -. 3 -c $(MATHCOREDH132) $(MATHCOREL32)

#LF
$(MATHCOREMAP): $(RLIBMAP) $(MAKEFILEDEP) $(MATHCOREL) $(MATHCORELINC) $(MATHCOREL32)
		$(RLIBMAP) -o $(MATHCOREMAP) -l $(MATHCORELIB) \
		-d $(MATHCORELIBDEPM) -c $(MATHCOREL) $(MATHCORELINC) $(MATHCOREL32)
#LF
$(MATHCORENM):      $(MATHCOREO) $(MATHCORETMPDO) $(MATHCORETMPDO32) $(MATHCORETMP2DO) $(MATHCORETMP2DO32)
		@echo "Generating symbols file $@..."
		nm -g -p --defined-only $(MATHCORETMPDO) | awk '{printf("%s\n", $$3)'} > $(MATHCORENM)
		nm -g -p --defined-only $(MATHCORETMPDO32) | awk '{printf("%s\n", $$3)'} >> $(MATHCORENM)
		nm -g -p --defined-only $(MATHCORETMP2DO) | awk '{printf("%s\n", $$3)'} >> $(MATHCORENM)
		nm -g -p --defined-only $(MATHCORETMP2DO32) | awk '{printf("%s\n", $$3)'} >> $(MATHCORENM)
		nm -g -p --defined-only $(MATHCOREO) | awk '{printf("%s\n", $$3)'} >> $(MATHCORENM)

all-mathcore:   $(MATHCORELIB) $(MATHCOREMAP)

clean-mathcore:
		@rm -f $(MATHCOREO) $(MATHCOREDO) clean-pds-mathcore

clean::         clean-mathcore

#LF
clean-pds-mathcore:	
		rm -f $(MATHCORETMPDS) $(MATHCORETMPDO) $(MATHCORETMPDH) \
		$(MATHCORETMPDEP) $(MATHCORETMP2DS) $(MATHCORETMP2DO) $(MATHCORETMP2DH) $(MATHNM) \
		$(MATHCORETMPDO32) $(MATHCORETMPDH32) $(MATHCORETMP2DS32) $(MATHCORETMP2DO32) $(MATHCORETMP2DH32) 

distclean-mathcore: clean-mathcore
		@rm -f $(MATHCOREDEP) $(MATHCOREDS) $(MATHCOREDS32) $(MATHCOREDH) \
		   $(MATHCORELIB) $(MATHCOREMAP)
		@rm -rf include/Math

distclean::     distclean-mathcore

test-mathcore:	all-mathcore
		@cd $(MATHCOREDIR)/test; make


