#include <string>
#include <iostream>
#include "inuit/drivers/TerminalDriver.hxx"

int main(int, char*[]) {
   using namespace Inuit;
   TerminalDriver::Instance().Clear();
   return 0;
}
