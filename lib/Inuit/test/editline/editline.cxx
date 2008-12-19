#include "inuit/widgets/UI.hxx"
#include "inuit/widgets/EditLine.hxx"
#include "inuit/drivers/TerminalDriver.hxx"
#include "inuit/drivers/InputDriver.hxx"

using namespace Inuit;

bool InputHandler(UI& ui, const Input& e) {
   if (e.GetType() == Input::kTypePrintable
      && e.GetPrintable() == 'Q')
      ui.RequestQuit();
   else return false;
   return true;
}

int main() {
   TerminalDriver& td = TerminalDriver::Instance();
   InputDriver& ed = InputDriver::Instance(td);
   UI ui(Pos(td.GetSize().fX, 1), ed);
   ui.AddInputHandler(InputHandler);
   ui.AddElement(new EditLine(&ui, Pos(0, 0), td.GetSize().fX));
   ui.ProcessInputs();
   return 0;
}
