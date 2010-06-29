all : testdict

#srcdir = .
#top_srcdir = ..

#builddir = .
#top_builddir = ..

#BUILD_DIR := $(shell /bin/pwd)
#TOP_BUILD_DIR := $(shell cd .. ; /bin/pwd)

DEPS_MAGIC := $(shell mkdir .deps > /dev/null 2>&1)

-include .deps/*.pp

ROOT_CFLAGS := $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOT_LDFLAGS := $(shell $(ROOTSYS)/bin/root-config --glibs)
#ROOT_LDFLAGS += -Wl,--rpath -Wl,$(shell $(ROOTSYS)/bin/root-config --libdir)

LLVM_CONFIG := $(LLVMDIR)/bin/llvm-config

LLVM_CFLAGS := $(shell $(LLVM_CONFIG) --cflags)

LLVM_COMPONENTS := $(shell $(LLVM_CONFIG) --targets-built)
LLVM_COMPONENTS += bitreader bitwriter codegen ipo selectiondag

LLVM_LIBS := $(shell $(LLVM_CONFIG) --ldflags) -lclangFrontend -lclangCodeGen -lclangSema -lclangChecker -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangLex -lclangBasic -lclangDriver $(shell $(LLVM_CONFIG) --libs $(LLVM_COMPONENTS))

vpath %.h ../lib
vpath %.cpp ../lib

testdict_OBJECTS := clr-reg.o clr-scan.o dict.o testdict.o
testdict_LDFLAGS := $(ROOT_LDFLAGS) -lReflex $(LLVM_LIBS)

CLEANFILES := .deps testdict $(testdict_OBJECTS)

testdict : $(testdict_OBJECTS)
	g++ -o $@ $(ROOT_CFLAGS) -I$(ROOTSYS) $(LLVM_CFLAGS) $^ $(testdict_LDFLAGS)

$(testdict_OBJECTS) : %.o : %.cpp
	g++ -c -o $@ $(ROOT_CFLAGS) -I$(ROOTSYS) $(LLVM_CFLAGS) $< -Wp,-MMD,.deps/$(*F).pp

clean :
	rm -rf $(CLEANFILES)

#COMPLETE_CLEAN_FILES := Makefile

#complete-clean: clean
#	test -x ./autoclean && ./autoclean ; \
#	rm -f "$(COMPLETE_CLEAN_FILES)"

