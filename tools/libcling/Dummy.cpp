// request symbols
#include "cling/Interpreter/Interpreter.h"
#include "cling/UserInterface/UserInterface.h"

namespace cling {
void libcling__symbol_requester() {
   const char* const argv[] = {"libcling__symbol_requester", 0};
   cling::Interpreter I(1, argv);
   cling::UserInterface U(I);
}
}
