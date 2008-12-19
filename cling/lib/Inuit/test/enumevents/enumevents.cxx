#include <string>
#include <iostream>
#include "inuit/drivers/TerminalDriver.hxx"
#include "inuit/drivers/InputDriver.hxx"

int main(int, char*[]) {
   using namespace Inuit;
   TerminalDriver& td = TerminalDriver::Instance();
   InputDriver& ed = InputDriver::Instance(td);
   bool goon = true;
   while (goon) {
      std::list<Input> events;
      ed.ProcessAvailableInputs(events, true);
      for (std::list<Input>::iterator iE = events.begin(); iE != events.end(); ++iE) {
         if (iE->GetType() == Input::kTypePrintable) {
            std::cout << "!" << iE->GetPrintable();
            if (iE->GetPrintable() == 'Q')
               goon = false;
         } else if (iE->GetType() == Input::kTypeNonPrintable) {
            Pos pos = td.GetCursorPos();
            td.Goto(Pos(0, td.GetSize().fY - 2));
            std::cout << " type: " << iE->GetENonPrintableName() << std::endl;
            td.Goto(pos);
         }
         Pos pos = td.GetCursorPos();
         td.Goto(Pos(0, td.GetSize().fY - 1));
         std::string modName;
         iE->GetEModifierName(modName);
         std::cout << " type: " << iE->GetETypeName()
            << " mod: " << modName
            << " repeat: " << iE->GetRepeat() << std::endl;
         td.Goto(pos);
      }
   }
   return 0;
}
