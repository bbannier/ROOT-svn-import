#include "TH1.h"
#include "TF1.h"
#include "TSystem.h"
#include "TRandom3.h"
#include "TROOT.h"

#include "Fit/DataVector.h"
//#include "Fit/BinPoint.h"
#include "THFitInterface.h"
#include "Fit/Fitter.h"

#include "Math/WrappedMultiTF1.h"
#include "Math/Polynomial.h"

#include <string>
#include <iostream>

// print the data
void printData(const ROOT::Fit::BinData & data) {
   for (unsigned int i = 0; i < data.Size(); ++i) { 
      std::cout << data.Coords(i)[0] << "   " << data.Value(i) << "   " << data.Error(i) << std::endl; 
   }
   std::cout << "\ndata size is " << data.Size() << std::endl;
}    
void printData(const ROOT::Fit::UnBinData & data) {
   for (unsigned int i = 0; i < data.Size(); ++i) { 
      std::cout << data.Coords(i)[0] << "\t"; 
   }
   std::cout << "\ndata size is " << data.Size() << std::endl;
}    

int testHisto1DFit() { 


   std::string fname("gaus");
   TF1 * func = (TF1*)gROOT->GetFunction(fname.c_str());
   func->SetParameter(0,10);
   func->SetParameter(1,0);
   func->SetParameter(2,3.0);

   TRandom3 rndm;

   // fill an histogram 
   TH1D * h1 = new TH1D("h1","h1",30,-5.,5.);
//      h1->FillRandom(fname.c_str(),100);
   for (int i = 0; i <1000; ++i) 
      h1->Fill( rndm.Gaus(0,1) );

   h1->Print();
   //h1->Draw();

//    gSystem->Load("libMinuit2");
//    gSystem->Load("libFit");


   // ROOT::Fit::DataVector<ROOT::Fit::BinPoint> dv; 
   
   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d,h1,func);


   printData(d);

   // create the function
   ROOT::Math::WrappedMultiTF1 f(*func); 
   double p[3] = {100,0,3.}; 
   f.SetParameters(p); 

   // create the fitter 
   std::cout << "Fit parameter 2  " << f.Parameters()[2] << std::endl;

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

int testHisto1DGradFit() { 



   std::string fname("pol2");
   TF1 * func = (TF1*)gROOT->GetFunction(fname.c_str());
   func->SetParameter(0,1.);
   func->SetParameter(1,2.);
   func->SetParameter(2,3.0);

   TRandom3 rndm;

   // fill an histogram 
   TH1D * h2 = new TH1D("h2","h2",30,-5.,5.);
//      h1->FillRandom(fname.c_str(),100);
   for (int i = 0; i <1000; ++i) 
      h2->Fill( func->GetRandom() );

   h2->Print();
   //h1->Draw();

//    gSystem->Load("libMinuit2");
//    gSystem->Load("libFit");


   // ROOT::Fit::DataVector<ROOT::Fit::BinPoint> dv; 
   
   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d,h2,func);


   printData(d);

   // create the function
   ROOT::Math::Polynomial f(100,0,3); 
   // double p[3] = {100,0,3.}; 
//    f.SetParameters(p); 

   // create the fitter 
   std::cout << "Fit parameter 2  " << f.Parameters()[2] << std::endl;

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

int testUnBin1DFit() { 

   std::string fname("gausn");
   TF1 * func = (TF1*)gROOT->GetFunction(fname.c_str());

   TRandom3 rndm;

   int n = 100;
   ROOT::Fit::UnBinData d(n); 

   for (int i = 0; i <n; ++i) 
      d.Add( rndm.Gaus(0,1) );
  

   printData(d);

   // create the function
   ROOT::Math::WrappedMultiTF1 f(*func); 
   double p[3] = {1,2,10.}; 
   f.SetParameters(p); 

   // create the fitter 
   //std::cout << "Fit parameters  " << f.Parameters()[2] << std::endl;

   ROOT::Fit::Fitter fitter; 
   fitter.SetFunction(f);
   std::cout << "fix parameter 0 " << " to value " << f.Parameters()[0] << std::endl;
   fitter.Config().ParSettings(0).Fix();
   //fitter.Config().MinimizerOptions().SetPrintLevel(3);

   bool ret = fitter.Fit(d);
   if (ret)  
      fitter.Result().Print(std::cout); 
   else {
      std::cout << " Fit Failed " << std::endl;
      return -1; 
   }
   return 0; 
}

template<typename Test> 
int testFit(Test t, std::string name) { 
   std::cout << name << "\n\t\t";  
   int iret = t();
   std::cout << "\n" << name << ":\t\t";  
   if (iret == 0) 
      std::cout << "OK" << std::endl;  
   else 
      std::cout << "Failed" << std::endl;  
   return iret; 
}

int main() { 

   int iret = 0; 
   iret |= testFit( testHisto1DFit, "Histogram1D Fit");
   iret |= testFit( testHisto1DGradFit, "Histogram1D Gradient Fit");
   iret |= testFit( testUnBin1DFit, "Unbin 1D Fit");
   return iret; 
}
   
