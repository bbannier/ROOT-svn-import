#include "TApplication.h"
#include "TGFrame.h"

int main(int argc, char ** argv)
{
   TApplication app("test_app", &argc, argv);
   TGFrame frame(0, 400, 400);
   
   frame.MapRaised();
   app.Run();
}
