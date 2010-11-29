#include "utFactory.h"

#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <exception>

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
   const int nvar=3, nfval=2, nivar=2, nclass=3;
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

bool utFactory::operateSingleFactory(const char* factoryname, const char* opt)
{
   if (TString(factoryname)=="") factoryname = "TMVATest";
   std::cout <<"operateSingleFactory option="<<opt<<std::endl;
   //try {
   TString option=opt;
   TTree* tree(0);
   TFile* inFile(0);
   if (!(option.Contains("MemoryResidentTree"))){
      inFile = TFile::Open( "input.root", "RECREATE" );
      tree = create_Tree();
      inFile->Write();
      inFile->Close();
      if (inFile) delete inFile;
      inFile = TFile::Open( "input.root");
      tree = (TTree*) inFile->Get("Tree");
   }
   else if (! (option.Contains("LateTreeBooking"))) tree = create_Tree();

   TMVA::Types::EMVA _methodType = TMVA::Types::kLD;
   TString _methodTitle="LD",_methodOption="!H:!V"; // fix me
   TString prepareString="";
   string factoryOptions( "!V:Silent:Transformations=I;D;P;G,D:AnalysisType=Classification:!Color:!DrawProgressBar" );
   TString outfileName( "TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   if (option.Contains("LateTreeBooking") && option.Contains("MemoryResidentTree")) tree = create_Tree();

   Factory* factory = new Factory(factoryname,outputFile,factoryOptions);
   factory->AddVariable( "var0",  "Variable 0", 'F' );
   factory->AddVariable( "var1",  "Variable 1", 'F' );
   if (option.Contains("var2"))  factory->AddVariable( "var2",  "Var 2", 'F' );
   if (option.Contains("ivar0")) factory->AddVariable( "ivar0",  "Var i0", 'I' );
   if (option.Contains("ivar1")) factory->AddVariable( "ivar1",  "Var i1", 'I' );

   factory->AddSpectator( "ievt", 'I' );
   factory->AddSignalTree(tree);
   factory->AddBackgroundTree(tree);
   if (prepareString=="") prepareString = "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   // this crashes "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   factory->PrepareTrainingAndTestTree( "iclass==0", "iclass==1", prepareString);

   if (option.Contains("StringMethodBooking")) factory->BookMethod("LD","LD","!H:!V");
   else factory->BookMethod(TMVA::Types::kLD,"LD","!H:!V");

   //factory->BookMethod(_methodType, _methodTitle, _methodOption);

   factory->TrainAllMethods();
   factory->TestAllMethods();
   factory->EvaluateAllMethods();
   MethodBase* theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));
   double ROCValue = theMethod->GetROCIntegral();   
   delete tree;
   delete factory;
   outputFile->Close(); // ???
   if (option.Contains("InputFile")){
      inFile->Close();
   }
   if (outputFile) delete outputFile;
   if (inFile) delete inFile;

   return (ROCValue>0.6);
   //}
   //catch (...) { return false;}

}
void utFactory::run()
{
   // create three factories with two methods each
   test_(operateSingleFactory("TMVATest","StringMethodBooking"));
   test_(operateSingleFactory("TMVATest",""));
   test_(operateSingleFactory("TMVATest3Var","var2"));
   test_(operateSingleFactory("TMVATest3VarF2VarI","var2:ivar0:ivar1"));

   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:StringMethodBooking"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:LateTreeBooking"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:LateTreeBooking:StringMethodBooking"));



}

