#include "utFactory.h"

#include <string>
#include <iostream>
#include <cassert>
#include <vector>

#include "TTree.h"
#include "TFile.h"
#include "TString.h"

#include "TMVA/Factory.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Types.h"

#include "UnitTest.h"

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

utFactory::utFactory(const char* /*theOption*/) 
   : UnitTest(string("Factory"))
{ 

}
utFactory::~utFactory(){ }
TTree* utFactory::create_Tree(const char* opt)
{
   TString option = opt;
   const int nmax=100; 
   const int nvar=3, nfval=2, nivar=1, nclass=3;
   float weight=1.;
   vector<float> var(nvar), fval(nfval); // fix me hardcoded nvar
   vector<int> ivar(nivar), nevt(nclass);
   Int_t iclass, ievt=0,i=0;

   TTree* tree = new TTree( "Tree", "Tree");
   for (i=0; i<nvar ; i++) tree->Branch(Form( "var%i", i), &var[i],
                                        Form( "var%i/F", i ));
   for (i=0; i<nfval; i++) tree->Branch(Form( "fval%i", i), &fval[i],
                                        Form( "fval%i/F", i ));
   for (i=0; i<nivar; i++) tree->Branch(Form( "ivar%i", i), &ivar[i],
                                        Form( "ivar%i/I", i ));
   tree->Branch("iclass",&iclass,"iclass/I");
   tree->Branch("ievt",&ievt,"ievt/I");
   tree->Branch("weight",&weight,"weight/F");
   TRandom3 R( 99 );

   do {
      for (i=0; i<nvar; i++)  var[i]  = 2.*R.Rndm()-1.;
      for (i=0; i<nivar; i++) ivar[i] = (int) (20.*(R.Rndm()-0.5));
      for (i=0; i<nfval; i++)  fval[i]  = 5.*(R.Rndm()-0.5);
      Float_t xout = var[0]+var[1]+var[2]*var[1]-var[0]*var[1]*var[1]+2.*(R.Rndm()-1.);
      if (xout < -2.) iclass=2;
      else if (xout > 0) iclass=0; // signal
      else iclass=1;               // background
      nevt[iclass]++;
      //std::cout << xout<< " " <<nevt[0]<< " " << nevt[1]<< " " << nevt[2]<< std::endl;
      ievt++;
      tree->Fill();
   } while ( TMath::Min(nevt[0],TMath::Min(nevt[1],nevt[2])) < nmax);
   //tree->Print();
   return tree;
}

void utFactory::operateSingleFactory(const char* opt)
{
   TMVA::Types::EMVA _methodType = TMVA::Types::kLD;
   TString _methodTitle="LD",_methodOption="!H:!V"; // fix me
   TString prepareString="";
   string factoryOptions( "!V:!Silent:Transformations=I;D;P;G,D:AnalysisType=Classification:!Color:!DrawProgressBar" );
   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   TTree* tree = create_Tree();
   //TTree* tree2 = create_Tree();
   Factory* factory = new Factory("TMVATest",outputFile,factoryOptions);
   factory->AddVariable( "var0",  "Variable 0", 'F' );
   factory->AddVariable( "var1",  "Variable 1", 'F' );
   factory->AddSpectator( "ievt", 'I' );
   factory->AddSignalTree(tree);
   factory->AddBackgroundTree(tree);
   if (prepareString=="") prepareString = "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   // this crashes "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   factory->PrepareTrainingAndTestTree( "iclass==0", "iclass==1", prepareString);

   //factory->BookMethod(TMVA::Types::kLD,"LD","!H:!V");
   factory->BookMethod("LD","LD","!H:!V");

   //factory->BookMethod(_methodType, _methodTitle, _methodOption);

   factory->TrainAllMethods();
   factory->TestAllMethods();
   factory->EvaluateAllMethods();
   MethodBase* theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));
   double ROCValue = theMethod->GetROCIntegral();
   test_(ROCValue>0.6);
   delete factory;
   outputFile->Close(); // ???
}
void utFactory::run()
{
   // create three factories with two methods each
   operateSingleFactory("test");
   // operate them in parallel

}

