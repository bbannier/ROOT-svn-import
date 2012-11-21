#include "utWeightHandling.h"

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

utWeightHandling::utWeightHandling(const char* /*theOption*/)
   : UnitTest(string("WeightHandling"))
{

}
utWeightHandling::~utWeightHandling(){ }
TTree* utWeightHandling::create_Tree(const char* opt)
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
   tree->Branch("wght",&weight,"wght/F");
   tree->Branch("origweight",&weight,"origweight/F");
   TRandom3 R( 99 );

   do {
      for (i=0; i<nvar; i++)  var[i]  = 2.*R.Rndm()-1.;
      for (i=0; i<nivar; i++) ivar[i] = (int) (20.*(R.Rndm()-0.5));
      for (i=0; i<nfval; i++)  fval[i]  = 5.*(R.Rndm()-0.5);
      Float_t xout = var[0]+var[1]+var[2]*var[1]-var[0]*var[1]*var[1]+2.*(R.Rndm()-1.);
      weight = R.Gaus(1.,0.1);
      if (xout < -1.) iclass=2;
      else if (xout > 0) iclass=0; // signal
      else iclass=1;               // background
      if (nevt[iclass]<nmax+iclass*10){
         nevt[iclass]++;
         //std::cout << xout<< " " <<nevt[0]<< " " << nevt[1]<< " " << nevt[2]<< std::endl;
         ievt++;
         tree->Fill();
      }
   } while ( TMath::Min(nevt[0],TMath::Min(nevt[1],nevt[2])) < nmax);
   //tree->Print();
   return tree;
}

bool utWeightHandling::operateSingleFactory(const char* factoryname, const char* opt, float ratio0, float ratio1, float ratio2)
{
   if (TString(factoryname)=="") factoryname = "TMVAWeightTest";
#ifdef COUTDEBUG
   std::cout <<"operateSingleFactory option="<<opt<<std::endl;
#endif
   TString option=opt;
   TTree* tree(0);
   TFile* inFile(0);
   inFile = TFile::Open( "weights/input.root", "RECREATE" );
   tree = create_Tree();
   inFile->Write();
   inFile->Close();
   if (inFile) delete inFile;
   inFile = TFile::Open( "weights/input.root");
   tree = (TTree*) inFile->Get("Tree");

   TString _methodTitle="LD",_methodOption="!H:!V"; // fix me
   TString prepareString="";
#ifdef VERBOSE
   string factoryOptions( "V:!Silent:Transformations=I,D:AnalysisType=Classification:!Color:!DrawProgressBar" );
#else
   string factoryOptions( "!V:Silent:Transformations=I,D:AnalysisType=Classification:!Color:!DrawProgressBar" );   
#endif
   TString outfileName( "weights/TMVA.root" );
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );

   Factory* factory = new Factory(factoryname,outputFile,factoryOptions);
   factory->AddVariable( "var0",  "Variable 0", 'F' );
   factory->AddVariable( "var1",  "Variable 1", 'F' );
   if (option.Contains("var2"))  factory->AddVariable( "var2",  "Var 2", 'F' );
   if (option.Contains("ivar0")) factory->AddVariable( "ivar0",  "Var i0", 'I' );
   if (option.Contains("ivar1")) factory->AddVariable( "ivar1",  "Var i1", 'I' );

   factory->AddSpectator( "ievt", 'I' );
   factory->AddSpectator( "iclass", 'I' );
   factory->AddSpectator( "origweight", 'F' );
   factory->AddTree(tree,"Signal"    ,  0.3, "iclass==0");
   factory->AddTree(tree,"Background",  1.5, "iclass==1");
   factory->AddTree(tree,"Background", 10.3, "iclass==2");
   factory->SetSignalWeightExpression(    "wght" );
   factory->SetBackgroundWeightExpression("wght" );

   TString normmode="None";
   if (option.Contains("UseNumEvents"))      normmode = "NumEvents"; 
   if (option.Contains("UseEqualNumEvents")) normmode = "EqualNumEvents";
 
      if (prepareString=="") prepareString = Form("nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=%s:!V",normmode.Data()) ;

   factory->PrepareTrainingAndTestTree( "iclass==0", "iclass>0", prepareString);

   if (option.Contains("StringMethodBooking")) factory->BookMethod("LD","LD","!H:!V");
   else factory->BookMethod(TMVA::Types::kLD,"LD","!H:!V");

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
   // check weights in test file
   TFile* testFile = new TFile("weights/TMVA.root");
   TTree* testTree = (TTree*)(testFile->Get("TestTree"));
   float weight,origweight,fclass;
   testTree->SetBranchAddress("weight",&weight);
   testTree->SetBranchAddress("origweight",&origweight);
   testTree->SetBranchAddress("iclass",&fclass);
   float ratio=0.;
   int nevt = testTree->GetEntriesFast();
   for (Long64_t ievt=0;ievt<nevt;ievt++) {
      testTree->GetEntry(ievt);
      
      if (TMath::Abs(fclass-0.)<1.e-5) ratio=ratio0;
      if (TMath::Abs(fclass-1.)<1.e-5) ratio=ratio1;
      if (TMath::Abs(fclass-2.)<1.e-5) ratio=ratio2;
      //std::cout << "class="<<fclass<<" ratio0="<<ratio0<<" ratio1="<<ratio1<<" ratio2="<<ratio2<<" ratio="<<ratio<<std::endl;
      test_(roughFloatCompare(weight/origweight,ratio));
   }
   //
   return ret;
}

void utWeightHandling::run()
{
   // create directory weights if necessary 
   FileStat_t stat;   
   if(gSystem->GetPathInfo("./weights",stat)) {// FIXME:: give the filename of the sample somewhere else?
      gSystem->MakeDirectory("weights"); 
#ifdef COUTDEBUG
      std::cout << "creating directory weights"<<std::endl;
#endif
   }

   test_(operateSingleFactory("TMVAWeightTest3VarF2VarI","var2:ivar0:ivar1:useweights:UseNone",0.3,1.5,10.3));
   test_(operateSingleFactory("TMVAWeightTest3VarF2VarI","var2:ivar0:ivar1:useweights:UseNumEvents",1.02267,0.249389,1.71247)); // odd weights due to weight renormalization initiated by NumEvents option
   test_(operateSingleFactory("TMVAWeightTest3VarF2VarI","var2:ivar0:ivar1:useweights:UseEqualNumEvents",1.02267,0.10843,0.744552)); // odd weights due to weight renormalization initiated by EqualNumEvents option

}

