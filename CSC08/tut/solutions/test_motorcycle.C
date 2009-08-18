
#include "Riostream.h"
#include "TROOT.h"
#include "TCanvas.h"
#ifndef __CINT__
// We already load the library for CINT
#include "Motorcycle.h"
#else
class TMotorcycle;
#endif

void setproperties(TMotorcycle *mybike, const char *brand, const char *model, Int_t cap, Float_t power)
{
   // function setting (changing) parameters of the TMotorcycle class
   // passed in argument

   // set the brand
   mybike->SetBrand(brand);
   // set the model
   mybike->SetModel(model);
   // set the cubic capacity
   mybike->SetCubicCap(cap);
   // set the power
   mybike->SetPower(power);
}

void printinfos(TMotorcycle *mybike)
{
   // function printing parameters of the TMotorcycle class
   // pointer passed in argument

   // dump infos on the terminal
   cout << "Brand: " << mybike->GetBrand();
   cout << " - Model: " << mybike->GetModel();
   cout << " - Cubic Capacity: " << mybike->GetCubicCap();
   cout << " - Power: " << mybike->GetPower() << endl;
}

void test_motorcycle()
{
   // main function

   // load the library containing the TMotorcycle class 
   // implementation
   gROOT->ProcessLine(".L Motorcycle.C+");
   // create a canvas to display pictures of the motorcycles
   TCanvas *c1 = new TCanvas("c1", "Pictures", 1200, 600);
   // divide the canvas
   c1->Divide(3, 2);
   // create a new TMotorcycle class with some values
   TMotorcycle *mybike = new TMotorcycle("Honda", "VTX1300S", 1312, 55);
   // verify the class member values
   printinfos(mybike);
   // go to the 1st pad
   c1->cd(1);
   // display picture of the motorcycle
   mybike->ShowPicture();
   // change the values
   setproperties(mybike, "Suzuki", "M800Z", 805, 39);
   // and verify the values have been changed
   printinfos(mybike);
   // go to the 2d pad
   c1->cd(2);
   // display picture of the motorcycle
   mybike->ShowPicture();
   // change values
   setproperties(mybike, "Yamaha", "XVS1300A", 1304, 53.5);
   // verify
   printinfos(mybike);
   // go to the 3rd pad
   c1->cd(3);
   // display picture of the motorcycle
   mybike->ShowPicture();
   // change values
   setproperties(mybike, "Kawasaki", "VN900C", 903, 37);
   // verify
   printinfos(mybike);
   // go to the 4th pad
   c1->cd(4);
   // display picture of the motorcycle
   mybike->ShowPicture();
   // change values
   setproperties(mybike, "Harley-Davidson", "FXCWC", 1584, 49.236);
   // verify
   printinfos(mybike);
   // go to the 5th pad
   c1->cd(5);
   // display picture of the motorcycle
   mybike->ShowPicture();
   c1->cd();
}

#endif
