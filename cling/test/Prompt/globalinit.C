// RUN: %cling -nologo %s\(\"%s\"\) 
// RUN: %cling -nologo %s\(\"%s\"\) | FileCheck %s

#include <typeinfo>

#include "cling/Interpreter/Interpreter.h"

void globalinit(const std::string& location) {
  gCling->loadFile(location + ".h", 0, false); // CHECK: A::S()
  gCling->loadFile(location + "2.h", 0, false); // CHECK: B::S()
}
// CHECK: B::~S()
// CHECK: A::~S()
