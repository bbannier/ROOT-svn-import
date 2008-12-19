#include <string>
#include <iostream>
#include "inuit/drivers/TerminalDriver.hxx"

int main(int, char*[]) {
   using namespace Inuit;
   TerminalDriver& drv = TerminalDriver::Instance();
   drv.PageFeed();
   drv.Goto(Pos(1, 1));
   std::cout << "Press enter:";
   std::string s;
   std::cin >> s;
   return 0;
}
