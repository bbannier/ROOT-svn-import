// $Id$
// Author: Joerg Stelzer   11/2007 
///////////////////////////////////////////////////////////////////////////////////
//
//  TMVA functionality test suite
//  =============================
//
//  This program performs tests of TMVA
//
//   To run the program do: 
//   stressTMVA          : run standard test
//
///////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <iomanip>
#include <ostream>
#include <fstream>
#include <assert.h>
#include <string>
#include <sstream>

#include "TString.h"
#include "TFile.h"
#include "TMath.h"
#include "TH1.h"
#include "TSystem.h"
#include "TRandom3.h"
#include "TMath.h"
#include "TMatrixD.h"


#include "TMVA/Factory.h"
#include "TMVA/Event.h"
#include "TMVA/PDF.h"
#include "TMVA/Reader.h"
#include "TMVA/Types.h"
#include "TMVA/VariableDecorrTransform.h"
#include "TMVA/VariableIdentityTransform.h"
#include "TMVA/VariableGaussTransform.h"
#include "TMVA/VariableNormalizeTransform.h"
#include "TMVA/VariablePCATransform.h"
#include "TMVA/DataSetInfo.h"
#include "TMVA/MethodFisher.h"
#include "TMVA/MethodFDA.h"
#include "TMVA/MethodPDEFoam.h"
#include "TMVA/MethodSVM.h"
#include "TMVA/MethodLD.h"
#include "TMVA/MethodPDERS.h"
#include "TMVA/MethodMLP.h"
#include "TMVA/Types.h"

namespace TMVA {

class TMVATestBenchRegression {



   public:
   
      TMVATestBenchRegression() {}
      ~TMVATestBenchRegression() {}

      void prepareData();
      void getGaussRnd( TArrayD& v, const TMatrixD& sqrtMat, TRandom& R );
      TMatrixD* produceSqrtMat( const TMatrixD& covMat );


      DataSetInfo* createDataSetFromTree();
      DataSetInfo* addRegressionTargets( DataSetInfo* dsi );

   private:

};



void TMVATestBenchRegression::prepareData(){
   std::cout << "---------- prepare data -----START " << std::endl;
   // create the data
   Int_t N = 2000;
   const Int_t nvar = 4;
   Float_t xvar[nvar];

   // output flie
   TFile* dataFile = TFile::Open( "data.root", "RECREATE" );

   // create signal and background trees
   TTree* treeS = new TTree( "TreeS", "TreeS", 1 );   
   TTree* treeB = new TTree( "TreeB", "TreeB", 1 );   
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      treeS->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
      treeB->Branch( TString(Form( "var%i", ivar+1 )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar+1 )).Data() );
   }
      
   TRandom R( 100 );
   Float_t xS[nvar] = {  0.2,  0.3,  0.5,  0.9 };
   Float_t xB[nvar] = { -0.2, -0.3, -0.5, -0.6 };
   Float_t dx[nvar] = {  1.0,  1.0, 1.0, 1.0 };
   TArrayD* v = new TArrayD( nvar );
   Float_t rho[20];
   rho[1*2] = 0.4;
   rho[1*3] = 0.6;
   rho[1*4] = 0.9;
   rho[2*3] = 0.7;
   rho[2*4] = 0.8;
   rho[3*4] = 0.93;

   // create covariance matrix
   TMatrixD* covMatS = new TMatrixD( nvar, nvar );
   TMatrixD* covMatB = new TMatrixD( nvar, nvar );
   for (Int_t ivar=0; ivar<nvar; ivar++) {
      (*covMatS)(ivar,ivar) = dx[ivar]*dx[ivar];
      (*covMatB)(ivar,ivar) = dx[ivar]*dx[ivar];
      for (Int_t jvar=ivar+1; jvar<nvar; jvar++) {
         (*covMatS)(ivar,jvar) = rho[(ivar+1)*(jvar+1)]*dx[ivar]*dx[jvar];
         (*covMatS)(jvar,ivar) = (*covMatS)(ivar,jvar);

         (*covMatB)(ivar,jvar) = rho[(ivar+1)*(jvar+1)]*dx[ivar]*dx[jvar];
         (*covMatB)(jvar,ivar) = (*covMatB)(ivar,jvar);
      }
   }
   std::cout << "signal covariance matrix: " << std::endl;
   covMatS->Print();
   std::cout << "background covariance matrix: " << std::endl;
   covMatB->Print();

   // produce the square-root matrix
   TMatrixD* sqrtMatS = produceSqrtMat( *covMatS );
   TMatrixD* sqrtMatB = produceSqrtMat( *covMatB );

   // loop over species
   for (Int_t itype=0; itype<2; itype++) {

      Float_t*  x;
      TMatrixD* m;
      if (itype == 0) { x = xS; m = sqrtMatS; std::cout << "- produce signal" << std::endl; }
      else            { x = xB; m = sqrtMatB; std::cout << "- produce background" << std::endl; }

      // event loop
      TTree* tree = (itype==0) ? treeS : treeB;
      for (Int_t i=0; i<N; i++) {

         if (i%100 == 0) std::cout << "... event: " << i << " (" << N << ")" << std::endl;
         getGaussRnd( *v, *m, R );

         for (Int_t ivar=0; ivar<nvar; ivar++) xvar[ivar] = (*v)[ivar] + x[ivar];
         
         tree->Fill();
      }
   }

   // write trees
   treeS->Write();
   treeB->Write();

   treeS->Show(0);
   treeB->Show(1);

   dataFile->Close();
   std::cout << "created data file: " << dataFile->GetName() << std::endl;

   std::cout << "---------- prepare data -----END " << std::endl;
}



TMatrixD* TMVATestBenchRegression::produceSqrtMat( const TMatrixD& covMat )
{
   Double_t sum = 0;
   Int_t size = covMat.GetNrows();;
   TMatrixD* sqrtMat = new TMatrixD( size, size );

   for (Int_t i=0; i< size; i++) {
      
      sum = 0;
      for (Int_t j=0;j< i; j++) sum += (*sqrtMat)(i,j) * (*sqrtMat)(i,j);

      (*sqrtMat)(i,i) = TMath::Sqrt(TMath::Abs(covMat(i,i) - sum));

      for (Int_t k=i+1 ;k<size; k++) {

         sum = 0;
         for (Int_t l=0; l<i; l++) sum += (*sqrtMat)(k,l) * (*sqrtMat)(i,l);

         (*sqrtMat)(k,i) = (covMat(k,i) - sum) / (*sqrtMat)(i,i);

      }
   }
   return sqrtMat;
}

void TMVATestBenchRegression::getGaussRnd( TArrayD& v, const TMatrixD& sqrtMat, TRandom& R ) 
{
   // generate "size" correlated Gaussian random numbers

   // sanity check
   const Int_t size = sqrtMat.GetNrows();
   if (size != v.GetSize()) 
      std::cout << "<getGaussRnd> too short input vector: " << size << " " << v.GetSize() << std::endl;

   Double_t* tmpVec = new Double_t[size];

   for (Int_t i=0; i<size; i++) {
      Double_t x, y, z;
      y = R.Rndm();
      z = R.Rndm();
      x = 2*TMath::Pi()*z;
      tmpVec[i] = TMath::Sin(x) * TMath::Sqrt(-2.0*TMath::Log(y));
   }

   for (Int_t i=0; i<size; i++) {
      v[i] = 0;
      for (Int_t j=0; j<=i; j++) v[i] += sqrtMat(i,j) * tmpVec[j];
   }

   delete tmpVec;
}



















DataSetInfo* TMVATestBenchRegression::createDataSetFromTree(){
   std::cout << "---------- createDataSetFromTree -----START " << std::endl;
   TFile *f = TFile::Open( "data.root" );
   TTree *treeS = (TTree*)f->Get( "TreeS" );
   TTree *treeB = (TTree*)f->Get( "TreeB" );

   DataInputHandler *dih = new DataInputHandler();
   DataSetManager::createInstance(*dih);

   TString dsiName = "testDataSet";
   DataSetInfo* dsi = DataSetManager::Instance().GetDataSetInfo(dsiName);
   if(dsi!=0){
      std::cout << "dataset with name " << dsiName << " already present." << std::endl;
      return 0;
   }
   std::cout << "no dataset with name " << dsiName << " found. A new one will be created." << std::endl;


   dsi = new DataSetInfo(dsiName);
   DataSetManager::Instance().AddDataSetInfo(*dsi);

   dsi->AddVariable( "var1" );
   dsi->AddVariable( "var2" );
   dsi->AddVariable( "var3" );

   dsi->AddTarget( "var4" );


   dih->AddSignalTree( treeS );
   dih->AddBackgroundTree( treeB );

   std::cout << "     datasets for signal and background created ==> loop through events" << std::endl;

   DataSet* ds = DataSetManager::Instance().CreateDataSet( dsiName );

//    ds = dsi->GetDataSet(); 
   UInt_t n = 0;
   std::cout << "number of events=" << ds->GetNEvents() << std::endl;
   for( UInt_t ievt=0; ievt<ds->GetNEvents(); ievt++ ){
      const Event *ev = ds->GetEvent(ievt);
//       Float_t val = ev->GetVal(ievt);
      if( n%500== 0 ){
    std::cout << "<event=" << n << ":vars=" << ev->GetNVariables(); std::cout.flush();
    for( UInt_t ivar = 0; ivar < ev->GetNVariables(); ivar++ ){
       std::cout << "|" << ivar << "=" << ev->GetVal(ivar); std::cout.flush();
    }
    std::cout << ">"; std::cout.flush();
      }
      n++;
   }
   std::cout << std::endl;

   std::cout << "---------- createDataSetFromTree -----END " << std::endl;
   return dsi;
}


DataSetInfo* TMVATestBenchRegression::addRegressionTargets( DataSetInfo* dsi ){
   std::cout << "---------- add regression values -----START " << std::endl;

   TString dsiName = "testDataSet";
//   DataSetInfo* dsi = DataSetManager::Instance().GetDataSetInfo(dsiName);
   if(dsi==0){
      std::cout << "no dataset with name " << dsiName << " found. ERROR." << std::endl;
      return 0;
   }
//      std::cout << "dataset with name " << dsiName << " already present." << std::endl;


//   DataSet* ds = DataSetManager::Instance().CreateDataSet( dsiName );
   DataSet* ds = dsi->GetDataSet(); 

   UInt_t n = 0;
   std::cout << "number of events=" << ds->GetNEvents() << std::endl;
   for( UInt_t ievt=0; ievt<ds->GetNEvents(Types::kTraining); ievt++ ){
      Event *ev = ds->GetEvent(ievt);

// --------------------------- turn this on for regression
// Float_t regVal = 6.0*ev->GetValue(0) + 1.0/3.0*ev->GetValue(1) + 0.5*ev->GetValue(2) +3.0;
     Float_t regVal = ev->GetValue(0) * ev->GetValue(1) - ev->GetValue(2);
     ev->SetTarget( 0, regVal );
// ---------------------------

      if( n%500== 0 ){
    std::cout << "<event=" << n << ":vars=" << ev->GetNVariables(); std::cout.flush();
    for( UInt_t ivar = 0; ivar < ev->GetNVariables(); ivar++ ){
       std::cout << "|" << ivar << "=" << ev->GetVal(ivar); std::cout.flush();
    }
    for( UInt_t itar = 0; itar < (ev->GetTargets()).size(); itar++ ){
       std::cout << "|t" << itar << "=" << ev->GetTarget(itar); std::cout.flush();
    }
    std::cout << ">" << std::endl; std::cout.flush();
      }
      n++;
   }
   std::cout << std::endl;
   n = 0;
   std::cout << "number of test events=" << ds->GetNEvents(Types::kTesting) << std::endl;
   for( UInt_t ievt=0; ievt<ds->GetNEvents(Types::kTesting); ievt++ ){
      Event *ev = ds->GetEvent(ievt, Types::kTesting);

// --------------------------- turn this on for regression
     Float_t regVal = ev->GetValue(0) * ev->GetValue(1) - ev->GetValue(2);
// Float_t regVal = 6.0*ev->GetValue(0) + 1.0/3.0*ev->GetValue(1) + 0.5*ev->GetValue(2) +3.0;
     ev->SetTarget( 0, regVal );
// ---------------------------

      if( n%500== 0 ){
    std::cout << "<event=" << n << ":vars=" << ev->GetNVariables(); std::cout.flush();
    for( UInt_t ivar = 0; ivar < ev->GetNVariables(); ivar++ ){
       std::cout << "|" << ivar << "=" << ev->GetVal(ivar); std::cout.flush();
    }
    for( UInt_t itar = 0; itar < (ev->GetTargets()).size(); itar++ ){
       std::cout << "|t" << itar << "=" << ev->GetTarget(itar); std::cout.flush();
    }
    std::cout << ">" << std::endl; std::cout.flush();
      }
      n++;
   }
   std::cout << std::endl;


   std::cout << "---------- add regression values -----END " << std::endl;
   return dsi;
}








}// namespace TMVA

int counter = 0;
std::vector< TMVA::MethodBase* > v;
std::vector< double > sqsum;
void addmethod( TMVA::MethodBase* method )
{
   v.push_back( method );
   sqsum.push_back(0);
   counter++;
}


int main() {
   TMVA::TMVATestBenchRegression testsuite;

   std::cout << "******************************************************************" << std::endl;
   std::cout << "* TMVA - regression test-bench suite *" << std::endl;
   std::cout << "******************************************************************" << std::endl;

   bool back = true;

   testsuite.prepareData();

   TMVA::DataSetInfo* dsi = testsuite.createDataSetFromTree();
   
   testsuite.addRegressionTargets( dsi );
   TDirectory *target_dir = new TDirectory("weights", "target_dir", "", 0);
   
   TMVA::DataSet* ds = dsi->GetDataSet(); 







   
//add methods here--------------------------------------------------------------------------------------------
//it's testing on the training data
//   addmethod(new TMVA::MethodSVM( "SVM_Gauss", "SVM_Gauss", *dsi,"Sigma=2.5:C=1.3:Tol=0.000001:Kernel=Gauss:VarTransform=N" ));
   addmethod(new TMVA::MethodLD( "LD", "LD", *dsi ));
   //   addmethod(new TMVA::MethodFDA( "FDA", "FDA", *dsi, "H:!V:Formula=(0)+(1)*x0+(2)*x1+(3)*x2+(4)*x0*x1+(5)*x0*x2+(6)*x1*x2+(7)*x1*x2:ParRanges=(-10,10);(-10,10);(-10,10);(-10,10);(-10,10);(-10,10);(-10,10);(-10,10):FitMethod=MC:Converger=MINUIT:ErrorLevel=1:PrintLevel=-1:FitStrategy=0:!UseImprove:!UseMinos:SetBatch:SampleSize=20"));
   //   addmethod(new TMVA::MethodPDERS("PDERS","PDERS",*dsi,"!H:!V:NormTree=T:VolumeRangeMode=Adaptive:KernelEstimator=Gauss:GaussSigma=0.2:NEventsMin=50:NEventsMax=200"));
   addmethod(new TMVA::MethodPDEFoam( "PDEFoam","PDEFoam",*dsi,TString("!H:!V:MultiTargetRegression=F:TargetSelection=Mpv:TailCut=0.001:VolFrac=0.0333:nCells=1000:nSampl=2000:nBin=5:Compress=T:Kernel=None:CutNmin=T:Nmin=100"),target_dir));
   //   addmethod(new TMVA::MethodMLP("MLP","MLP",*dsi, "H:!V:!Normalise:NeuronType=tanh:NCycles=200:HiddenLayers=N+1,N:TestRate=6:TrainingMethod=BFGS"));
   //   addmethod(new TMVA::MethodMLP("CFMlpANN","CFMlpANN",*dsi, "!H:!V:NCycles=300:HiddenLayers=N+1,N"));



	
   for (std::vector< TMVA::MethodBase* >::iterator it = v.begin(); it!=v.end(); it++)
   {
      (*it)->SetAnalysisType(TMVA::Types::kRegression);
      (*it)->Train();
   }

   ds->SetCurrentType(TMVA::Types::kTesting);
   std::cout << std::fixed;   
   for( UInt_t ievt=0; ievt<ds->GetNEvents(TMVA::Types::kTesting); ievt++ ){
           
      const TMVA::Event *ev = ds->GetEvent(ievt);
      Float_t r = ev->GetTarget(0);
      std::cout <<"\033[1;31mEv["<<ievt<<"]\033[0m = "<< r <<" ";
      for (int i=0; i<counter; i++)
      {
         double tmp = (double)v[i]->GetRegressionValues()[0];         
         sqsum[i] += (tmp-r)*(tmp-r);
         std::cout << "\t\033[1;3"<<i%4+2<<"m"<<v[i]->GetMethodName()<<"\033[0m = "<< std::setprecision(6) << tmp;
      }
      std::cout <<std::endl;
   }
   std::cout <<std::endl;
   std::cout <<"\033[1;31mSum of squares\033[0m\t";
   for (int i=0; i<counter; i++)
      std::cout << "\t\033[1;3"<<i%4+2<<"m"<<v[i]->GetMethodName()<<"\033[0m = "<< std::setprecision(4) << sqsum[i];
   std::cout <<std::endl;

   std::cout <<"\033[1;31mStandard Deviation\033[0m";
   for (int i=0; i<counter; i++)
      std::cout << "\t\033[1;3"<<i%4+2<<"m"<<v[i]->GetMethodName()<<"\033[0m = "<< std::setprecision(6) << sqrt(sqsum[i]/ds->GetNEvents());
   std::cout <<std::endl;

   return 1;
}








/* For the emacs weenies in the crowd.
Local Variables:
   c-basic-offset: 3
End:
*/
