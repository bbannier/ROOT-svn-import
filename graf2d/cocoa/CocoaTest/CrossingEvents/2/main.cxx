#include <fstream>

#include "TApplication.h"
#include "testframe.h"

#include "guibuilder.h"

int main(int argc, char ** argv)
{
   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);
   
   GuiBuilder bld;
//   std::ifstream inputFile("test1.gui");
//   std::ifstream inputFile("test2.gui");
   std::ifstream inputFile("test3.gui");
//   std::ifstream inputFile("test4.gui");


   try 
   {
      bld.BuildGUI(inputFile);
   } 
   catch (const std::exception &)
   {
      return 0;
   }

   app.Run();
}
