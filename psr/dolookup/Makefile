.PHONY: all clean clean_dolookup

all : dolookup

clean : clean_dolookup

dolookup : dolookup.o
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -o $@ $< `llvm-config --ldflags` -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangAnalysis -lclangIndex -lclangRewrite -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMX86Info -lLLVMMCDisassembler -lLLVMInterpreter -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport

dolookup.o : dolookup.cpp
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -c -o $@ $<

clean_dolookup :
	rm -f dolookup.o
	rm -f dolookup
