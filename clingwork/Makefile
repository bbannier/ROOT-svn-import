c1 : c1.cpp
	g++ -g -Wall -W -Woverloaded-virtual -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $< -L/local2/russo/llvm/lib -lclangFrontend -lclangDriver -lclangCodeGen -lclangSema -lclangChecker -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangLex -lclangBasic `llvm-config --libs` -pthread -ldl

c3 : c3.cpp
	g++ -g -Wall -W -Woverloaded-virtual -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $< -L/local2/russo/llvm/lib -lclangFrontend -lclangDriver -lclangCodeGen -lclangSema -lclangChecker -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangLex -lclangBasic `llvm-config --libs` -pthread -ldl

c4 : c4.o c4_tools.o
	g++ -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $^ -L/local2/russo/llvm/lib -lclangFrontend -lclangDriver -lclangCodeGen -lclangSema -lclangChecker -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangLex -lclangBasic `llvm-config --libs` -ludis86 -pthread -ldl

c4.o : c4.cpp
	g++ -c -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $<

c4_tools.o : c4_tools.cpp c4_tools.h
	g++ -c -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $<

c5 : c5.o c5_tools.o
	g++ -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $^ -L/local2/russo/llvm/lib -lclangFrontend -lclangDriver -lclangCodeGen -lclangSema -lclangChecker -lclangAnalysis -lclangRewrite -lclangAST -lclangParse -lclangLex -lclangBasic `llvm-config --libs` -ludis86 -pthread -ldl

c5.o : c5.cpp
	g++ -c -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $<

c5_tools.o : c5_tools.cpp c5_tools.h
	g++ -c -g -D_DEBUG -D_GNU_SOURCE -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -I/local2/russo -I/local2/russo/llvm/include -I/local2/russo/llvm-head/tools -o $@ $<

