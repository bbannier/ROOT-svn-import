// request symbols
#include "cling/Interpreter/Interpreter.h"
#include "cling/UserInterface/UserInterface.h"

namespace cling {
void libcling__symbol_requester() {
   cling::Interpreter I;
   cling::UserInterface U(I);
}
}
