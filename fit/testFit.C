#include "TH1.h"
#include "TF1.h"
#include "TSystem.h"
#include "TRandom3.h"

#include "Fit/DataVector.h"
#include "Fit/BinPoint.h"
#include "Fit/TH1Interface.h"
#include "Fit/Fitter.h"
#include "Fit/WrappedTF1.h"

#include <string>
#include <iostream>

// print the data
template <class T> 
void printData(const T & data) {
   for (typename T::const_iterator itr = data.begin(); itr != data.end(); ++itr) { 
      std::cout << itr->Coords()[0] << "   " << itr->Value() << "   " << itr->Error() << std::endl; 
   }
   std::cout << "\ndata size is " << data.Size() << std::endl;
}    

int testFit() { 



   std::string fname("gaus");
   TF1 * func = (TF1*)gROOT->GetFunction(fname.c_str());
   func->SetParameter(0,10);
   func->SetParameter(1,0);
   func->SetParameter(2,3.0);

   TRandom3 rndm;

   // fill an histogram 
   TH1D * h1 = new TH1D("h1","h1",30,-5.,5.);
//      h1->FillRandom(fname.c_str(),100);
   for (int i = 0; i <100; ++i) 
      h1->Fill( rndm.Gaus(0,3) );

   h1->Print();
   //h1->Draw();

//    gSystem->Load("libMinuit2");
//    gSystem->Load("libFit");


   // ROOT::Fit::DataVector<ROOT::Fit::BinPoint> dv; 
   
   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d,h1,func);


   printData(d);

   // create the function
   ROOT::Fit::WrappedTF1 f(func); 
   double p[3] = {100,1,2}; 
   f.SetParameters(p); 

   // create the fitter 

   ROOT::Fit::Fitter fitter; 
   bool ret = fitter.Fit(d, f);
   if (ret)  
      fitter.Result().Print(std::cout); 
   else {
      std::cout << " Fit Failed " << std::endl;
      return -1; 
   }
   return 0; 
}

int main() { 

   return testFit(); 
}
   
