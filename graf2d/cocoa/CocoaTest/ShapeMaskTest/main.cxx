#include <iostream>

#include "TApplication.h"
#include "TVirtualX.h"
#include "TGPicture.h"
#include "TGClient.h"
#include "TSystem.h"
#include "TImage.h"

#include "testframe.h"

#define TEST9

int main(int argc, char ** argv)
{
   //All test cases are described in

   using namespace ROOT::CocoaTest;

   TApplication app("test_app", &argc, argv);

   TImage *image = TImage::Open("speedo.gif");
   if (!image || !image->IsValid()) {
      std::cout<<"'speedo.gif' was not found\n";
      return 0;
   }
   
   const TGPicture * pic = gClient->GetPicturePool()->GetPicture(gSystem->BaseName("speedo.gif"), image->GetPixmap(), image->GetMask());
   if (!pic) {
      std::cout<<"Could not create TGPicture from speedo.gif\n";
      return 0;
   }

   TestFrame * mainFrame = new TestFrame(0, pic->GetWidth(), pic->GetHeight(), kMainFrame, 0xff0000);
   gVirtualX->ShapeCombineMask(mainFrame->GetId(), 0, 0, pic->GetMask());
   gVirtualX->MapSubwindows(mainFrame->GetId());
   gVirtualX->MapRaised(mainFrame->GetId());

   app.Run();
}
