.PHONY: all clean clean_dolookup clean_do_class_lookup clean_do_func_lookup_with_overload

all : dolookup

clean : clean_dolookup clean_do_class_lookup clean_do_func_lookup_with_overload

dolookup : dolookup.o
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -o $@ $< `llvm-config --ldflags` -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangEdit -lclangAnalysis -lclangIndex -lclangRewrite -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMX86Info -lLLVMMCDisassembler -lLLVMInterpreter -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport

dolookup.o : dolookup.cpp
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -c -o $@ $<

do_class_lookup : do_class_lookup.o
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -o $@ $< `llvm-config --ldflags` -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangEdit -lclangAnalysis -lclangIndex -lclangRewrite -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMX86Info -lLLVMMCDisassembler -lLLVMInterpreter -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport

do_class_lookup.o : do_class_lookup.cpp
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -c -o $@ $<

do_func_lookup_with_overload : do_func_lookup_with_overload.o
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -o $@ $< `llvm-config --ldflags` -lclangFrontendTool -lclangFrontend -lclangDriver -lclangSerialization -lclangCodeGen -lclangParse -lclangSema -lclangEdit -lclangAnalysis -lclangIndex -lclangRewrite -lclangAST -lclangLex -lclangBasic -lLLVMAsmParser -lLLVMInstrumentation -lLLVMLinker -lLLVMArchive -lLLVMBitReader -lLLVMDebugInfo -lLLVMJIT -lLLVMipo -lLLVMBitWriter -lLLVMTableGen -lLLVMX86Disassembler -lLLVMX86AsmParser -lLLVMX86CodeGen -lLLVMX86Desc -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMX86Info -lLLVMMCDisassembler -lLLVMInterpreter -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCParser -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMTransformUtils -lLLVMipa -lLLVMAnalysis -lLLVMMCJIT -lLLVMRuntimeDyld -lLLVMExecutionEngine -lLLVMTarget -lLLVMMC -lLLVMObject -lLLVMCore -lLLVMSupport

do_func_lookup_with_overload.o : do_func_lookup_with_overload.cpp
	g++ `llvm-config --cppflags` -g -O0 -fexceptions -c -o $@ $<

clean_dolookup :
	rm -f dolookup.o
	rm -f dolookup

clean_do_class_lookup :
	rm -f do_class_lookup.o
	rm -f do_class_lookup

clean_do_func_lookup_with_overload :
	rm -f do_func_lookup_with_overload.o
	rm -f do_func_lookup_with_overload

