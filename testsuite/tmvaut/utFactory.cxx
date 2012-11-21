#include "utFactory.h"

#include <string>
#include <iostream>
#include <cassert>
#include <vector>
#include <exception>

#include "TMath.h"
#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
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

bool utFactory::addEventsToFactoryByHand(const char* factoryname, const char* opt)
{
#ifdef COUTDEBUG
   std::cout <<"addEventsToFactoryByHand option="<<opt<<std::endl;
#endif
   TString option=opt;
   bool useWeights = option.Contains("useWeights");
   bool useNegWeights = option.Contains("useNegWeights");
   TString _methodTitle,_methodOption="!H:!V"; // fix me
   if (option.Contains("useBDT")) _methodTitle="BDT";
   else if (option.Contains("useMLP")) _methodTitle="MLP";
   else _methodTitle="LD";

   TString prepareString="";
#ifdef VERBOSE
   string factoryOptions( "V:!Silent:Transformations=I:AnalysisType=Classification:!Color:!DrawProgressBar" );
#else
   string factoryOptions( "!V:Silent:Transformations=I:AnalysisType=Classification:!Color:!DrawProgressBar" );
#endif
   TString outfileName( "weights/ByHand.root" );
   TFile* outputFile = new TFile( outfileName, "RECREATE" );
   outputFile->cd();
   Factory* factory = new Factory(factoryname,outputFile,factoryOptions);
   factory->AddVariable( "var0",  "Variable 0", 'F' );
   factory->AddVariable( "var1",  "Variable 1", 'F' );

   vector <double> vars(2);
   TRandom3 r(99);
   double weight = 1.;
   for (int i=0;i<100;i++){
      if (useWeights){
         vars[0]= 4. * (r.Rndm()-0.5);
         vars[1]= 4. * (r.Rndm()-0.5);
         weight = TMath::Gaus(vars[0],1.,1.)*TMath::Gaus(vars[1],0.,1.);
         if (useNegWeights && i>90) weight = -weight;
      }
      else {
         vars[0]=r.Gaus(1.,1.);
         vars[1]=r.Gaus(0.,1.);
      }
      factory->AddSignalTrainingEvent( vars, weight );
      factory->AddSignalTestEvent( vars, weight );
   }
   for (int i=0;i<100;i++){
      vars[0]= 4. * (r.Rndm()-0.5);
      vars[1]= 4. * (r.Rndm()-0.5);
      weight = 1.;
      factory->AddBackgroundTrainingEvent( vars, weight);
      factory->AddBackgroundTestEvent( vars, weight);
   }
   if (prepareString=="") prepareString = "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   factory->PrepareTrainingAndTestTree( "", "", prepareString);

   factory->BookMethod(_methodTitle,_methodTitle, "!H:!V");
   factory->TrainAllMethods();
   factory->TestAllMethods();
   factory->EvaluateAllMethods();
   MethodBase* theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));
   double ROCValue = theMethod->GetROCIntegral();
   //cout << "ROC="<<ROCValue<<endl;
   delete factory;
   outputFile->Close();
   if (outputFile) delete outputFile;
   return (ROCValue>0.6);
}

bool utFactory::operateSingleFactory(const char* factoryname, const char* opt)
{
   if (TString(factoryname)=="") factoryname = "TMVATest";
#ifdef COUTDEBUG
   std::cout <<"operateSingleFactory option="<<opt<<std::endl;
#endif
   TString option=opt;
   TTree* tree(0);
   TFile* inFile(0);
   if (!(option.Contains("MemoryResidentTree"))){
      inFile = TFile::Open( "weights/input.root", "RECREATE" );
      tree = create_Tree();
      inFile->Write();
      inFile->Close();
      if (inFile) delete inFile;
      inFile = TFile::Open( "weights/input.root");
      tree = (TTree*) inFile->Get("Tree");
   }
   else if (! (option.Contains("LateTreeBooking"))) tree = create_Tree();

   TString prepareString="";
   string factoryOptions( "!V:Silent:Transformations=I,D:AnalysisType=Classification:!Color:!DrawProgressBar" );
   TString outfileName( "weights/TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );
   if (option.Contains("LateTreeBooking") && option.Contains("MemoryResidentTree")) tree = create_Tree();

   Factory* factory = new Factory(factoryname,outputFile,factoryOptions);
   factory->AddVariable( "var0",  "Variable 0", 'F' );
   factory->AddVariable( "var1",  "Variable 1", 'F' );
   if (option.Contains("var2"))  factory->AddVariable( "var2",  "Var 2", 'F' );
   if (option.Contains("ivar0")) factory->AddVariable( "ivar0",  "Var i0", 'I' );
   if (option.Contains("ivar1")) factory->AddVariable( "ivar1",  "Var i1", 'I' );

   factory->AddSpectator( "ievt", 'I' );
   if (option.Contains("reverseSB")){
      //std::cout << "reverse SB!"<<std::endl;
      factory->AddBackgroundTree(tree);
      factory->AddSignalTree(tree);
   } else {
      factory->AddSignalTree(tree);
      factory->AddBackgroundTree(tree);
   }

   if (prepareString=="") prepareString = "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   // this crashes "nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V" ;
   factory->PrepareTrainingAndTestTree( "iclass==0", "iclass==1", prepareString);

   TString _methodTitle,_methodOption="!H:!V";
   if (option.Contains("TypeMethodBooking")){
      //std::cout << "TypeMethodBooking"<<std::endl;
      _methodTitle="LD";
      factory->BookMethod(TMVA::Types::kLD,_methodTitle,_methodOption);
   }
   else {
      _methodTitle="LD";
      if (option.Contains("BDT")) _methodTitle="BDT";
      if (option.Contains("MLP")) _methodTitle="MLP";
      factory->BookMethod(_methodTitle,_methodTitle,_methodOption);
   }

   factory->TrainAllMethods();
   factory->TestAllMethods();
   factory->EvaluateAllMethods();
   MethodBase* theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));
   double ROCValue = theMethod->GetROCIntegral();   
   delete tree;
   delete factory;
   outputFile->Close(); 
   if (option.Contains("InputFile")){
      inFile->Close();
   }
   if (outputFile) delete outputFile;
   if (inFile) delete inFile;

   Bool_t ret = (ROCValue>0.6);
   if (!ret) {
      std::cout <<"FAILURE with operateSingleFactory option="<<opt<< ",  bad value ROC="<<ROCValue<<std::endl;
   }
   return ret;
}

void utFactory::run()
{
   // create directory weights if necessary 
   FileStat_t stat;   
   if(gSystem->GetPathInfo("./weights",stat)) {// FIXME:: give the filename of the sample somewhere else?
      gSystem->MakeDirectory("weights"); 
#ifdef COUTDEBUG
      std::cout << "creating directory weights"<<std::endl;
#endif
   }
   // create three factories with two methods each
   test_(addEventsToFactoryByHand("ByHand","")); // uses Factory::AddSignalTrainingEvent
   test_(addEventsToFactoryByHand("ByHand2","useWeights")); 

   test_(addEventsToFactoryByHand("ByHand","useMLP")); // uses Factory::AddSignalTrainingEvent
   test_(addEventsToFactoryByHand("ByHand2","useWeights:useMLP")); 


   test_(addEventsToFactoryByHand("ByHand","useBDT")); // uses Factory::AddSignalTrainingEvent
   test_(addEventsToFactoryByHand("ByHand2","useWeights:useNegWeights:useBDT")); 
   test_(addEventsToFactoryByHand("ByHand2","useWeights:useBDT")); 



   test_(operateSingleFactory("TMVATest","TypeMethodBooking"));
   test_(operateSingleFactory("TMVATest","BDT"));
   test_(operateSingleFactory("TMVATest","BDT:reverseSB"));
   test_(operateSingleFactory("TMVATest","MLP"));
   test_(operateSingleFactory("TMVATest","MLP:reverseSB"));
   test_(operateSingleFactory("TMVATest","reverseSB"));
   test_(operateSingleFactory("TMVATest",""));
   test_(operateSingleFactory("TMVATest3Var","var2"));
   test_(operateSingleFactory("TMVATest3VarF2VarI","var2:ivar0:ivar1"));


// uses Factory::AddSignalTrainingEvent

   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:StringMethodBooking"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:LateTreeBooking"));
   //creates crash test_(operateSingleFactory("TMVATest","MemoryResidentTree:LateTreeBooking:StringMethodBooking"));



}

