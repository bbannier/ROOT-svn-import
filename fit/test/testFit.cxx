#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TF2.h"
#include "TSystem.h"
#include "TRandom3.h"
#include "TROOT.h"

#include "Fit/DataVector.h"
//#include "Fit/BinPoint.h"
#include "THFitInterface.h"
#include "Fit/Fitter.h"

#include "Math/WrappedMultiTF1.h"
#include "Math/WrappedParamFunction.h"
#include "Math/WrappedTF1.h"
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

   ROOT::Fit::Fitter fitter; 

   bool ret = fitter.Fit(d, f);
   if (ret)  
      fitter.Result().Print(std::cout); 
   else {
      std::cout << "Chi2 Fit Failed " << std::endl;
      return -1; 
   }

   // test using binned likelihood 
   ret = fitter.LikelihoodFit(d, f);
   if (ret)  
      fitter.Result().Print(std::cout); 
   else {
      std::cout << "Binned Likelihood Fit Failed " << std::endl;
      return -1; 
   }


   return 0;
}


class Func1D : public ROOT::Math::IParamFunction { 
public:
   void SetParameters(const double *p) { std::copy(p,p+NPar(),fp);}
   const double * Parameters() const { return fp; }
   Func1D * Clone() const { 
      Func1D * f =  new Func1D(); 
      f->SetParameters(fp);
      return f;
   };
   unsigned int NPar() const { return 3; }
private:
   double DoEval( double x) const { 
      return fp[0]*x*x + fp[1]*x + fp[2]; 
   }
   double fp[3];
   
};

// gradient 2D function
class GradFunc2D : public ROOT::Math::IParamMultiGradFunction { 
public:
   void SetParameters(const double *p) { std::copy(p,p+NPar(),fp);}
   const double * Parameters() const { return fp; }
   GradFunc2D * Clone() const { 
      GradFunc2D * f =  new GradFunc2D(); 
      f->SetParameters(fp);
      return f;
   };
   unsigned int NDim() const { return 2; }
   unsigned int NPar() const { return 5; }
private:
   double DoEval( const double *x) const { 
      return fp[0]*x[0]*x[0] + fp[1]*x[0] + fp[2]*x[1]*x[1] + fp[3]*x[1] + fp[4]; 
   }
   void DoParameterGradient( const double * x, double * grad) const { 
      grad[0] = x[0]*x[0]; 
      grad[1] = x[0];
      grad[2] = x[1]*x[1]; 
      grad[3] = x[1];
      grad[4] = 0; 
   }
   double DoDerivative(const double *x, unsigned int icoord = 0) const { 
      assert(icoord <= 1); 
      if (icoord == 0) 
         return 2. * fp[0] * x[0] + fp[1];
      else 
         return 2. * fp[2] * x[1] + fp[3];
   }
   double fp[5];
   
};

int testHisto1DPolFit() { 



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

   // fill fit data
   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d,h2,func);


   printData(d);

   // create the function
   //ROOT::Math::Polynomial f(100,0,3); 
   Func1D f; 

   //ROOT::Math::WrappedTF1 f(*func); 
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

int testHisto2DFit() { 
   // fit using a 2d parabola (test also gradient)


   std::string fname("pol2");
   TF2 * func = new TF2("f2d",ROOT::Math::ParamFunctor(GradFunc2D() ), -5.,5.,-5,5,5);
   double p0[5] = { 1.,2.,0.5,1.,3. }; 
   func->SetParameters(p0);
   assert(func->GetNpar() == 5); 

   TRandom3 rndm;

   // fill an histogram 
   TH2D * h2 = new TH2D("h2d","h2d",30,-5.,5.,30,-5.,5.);
//      h1->FillRandom(fname.c_str(),100);
   for (int i = 0; i <1000; ++i) {
      double x,y = 0;
      func->GetRandom2(x,y);
      h2->Fill(x,y);
   }
   // fill fit data
   ROOT::Fit::BinData d; 
   ROOT::Fit::FillData(d,h2,func);


   //printData(d);

   // create the function
   GradFunc2D f; 

   double p[5] = { 2.,1.,1,2.,100. }; 
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

   // test without gradient
   std::cout <<"\ntest result without using gradient" << std::endl;
   ROOT::Math::WrappedParamFunction<GradFunc2D *> f2(&f,2,5,p);
   ret = fitter.Fit(d, f2);
   if (ret)  
      fitter.Result().Print(std::cout); 
   else {
      std::cout << " Fit Failed " << std::endl;
      return -1; 
   }

   // test binned likelihood gradient
   std::cout <<"\ntest result using gradient and binned likelihood" << std::endl;
   ret = fitter.LikelihoodFit(d, f);
   f.SetParameters(p); 
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
   iret |= testFit( testHisto1DPolFit, "Histogram1D Polynomial Fit");
   iret |= testFit( testHisto2DFit, "Histogram2D Gradient Fit");
   iret |= testFit( testUnBin1DFit, "Unbin 1D Fit");
   return iret; 
}
   
