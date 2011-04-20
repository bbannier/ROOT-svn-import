// RUN: %cling -l %s\(\"%s\"\)
// RUN: %cling -l %s\(\"%s\"\) | FileCheck %s

#include <typeinfo>
extern "C" int printf(const char*,...);
void globalinit(const std::string& location) {
   gCling->loadFile(location + ".h", 0, false); // CHECK: A::S()
   gCling->loadFile(location + "2.h", 0, false); // CHECK: B::S()
}
// CHECK: B::~S()
// CHECK: A::~S()
