// author Eckhard von Toerne, U. of Bonn 2011
#include "ReaderTest.h"
#include "TFile.h"
#include "TSystem.h"
#include "TROOT.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Reader.h"
#include <cstdlib>

using namespace std;
using namespace UnitTesting;
using namespace TMVA;


ReaderTest::ReaderTest(TString methodtitle, 
                       TString weightdir, bool isRegression) :
   UnitTest(string((isRegression? "RegRead_" : "ClasRead_"))+string(methodtitle.Data())+string("_")+string(weightdir),__FILE__),
   fIsRegression(isRegression),
   fWeightFile(""),
   fTreeFilename(""), 
   fTreeName("TestTree"), 
   fMethodTitle(methodtitle),
   fVariableNames(0),
   fBranchNames(0)
{
   std::vector<TString>* vars=  new vector<TString>();
   std::vector<TString>* branchvars=  new vector<TString>();
   std::vector<TString>* specs =  new vector<TString>();
   if (fIsRegression){
      fWeightFile=weightdir + "/TMVARegression_"
         + fMethodTitle + ".weights.xml";
      fTreeFilename=weightdir + "/TMVAReg.root";
      vars->push_back("var1");
      vars->push_back("var2");      
      specs->push_back("var1*2");
      specs->push_back("var1*3");
   }
   else {
      fWeightFile=weightdir + "/TMVAClassification_"
         + fMethodTitle + ".weights.xml";
      fTreeFilename=weightdir + "/TMVA.root";
      vars->push_back("var1+var2");
      vars->push_back("var1-var2");      
      vars->push_back("var3");      
      vars->push_back("var4");
      branchvars->push_back("myvar1");
      branchvars->push_back("myvar2");
      branchvars->push_back("var3");
      branchvars->push_back("var4");
      specs->push_back("var1*2");
      specs->push_back("var1*3");
   }
   if (branchvars->size()!=0) fBranchNames = branchvars;
   else fBranchNames =vars;
   fVariableNames=vars;
   fSpecNames = specs;
   OpenFiles();
   if (!fTestFile) return;
   fTestTree = (TTree*)(fTestFile->Get(fTreeName));
   fNVar = fVariableNames->size();
   fNSpecs = fSpecNames->size();
}

ReaderTest::ReaderTest(TString methodtitle, TString weightfile, TString tmvafile, TString treename ,const vector<TString>* varlist, bool isRegression) 
   : UnitTest(string((isRegression? "RegRead_" : "ClasRead_"))+string(methodtitle.Data())+string("_")+string(weightfile),__FILE__),
     fIsRegression(isRegression),
     fWeightFile(weightfile),
     fTreeFilename(tmvafile), 
     fTreeName(treename), 
     fMethodTitle(methodtitle), 
     fVariableNames(varlist),
     fBranchNames(varlist),
     fSpecNames(new vector<TString>())  
{
   OpenFiles();
   if (!fTestFile) return;
   fTestTree = (TTree*)(fTestFile->Get(fTreeName));
   fNVar = fVariableNames->size();
   fNSpecs = fSpecNames->size();
}

void ReaderTest::OpenFiles()
{
   if (gSystem->AccessPathName( fWeightFile ))
      gSystem->Exec(Form("wget http://www.uni-bonn.de/~etoerne/tmva/weightfiles/%s -O %s",fWeightFile.Data(),fWeightFile.Data()));
   if (fWeightFile.Contains("PDEFoam")){
      TString FoamRootFile = fWeightFile;
      FoamRootFile.ReplaceAll(".xml","_foams.root");
      if (gSystem->AccessPathName(FoamRootFile)) {
         gSystem->Exec(Form("wget http://www.uni-bonn.de/~etoerne/tmva/weightfiles/%s -O %s",FoamRootFile.Data(),FoamRootFile.Data()));
      }
   }
   if (gSystem->AccessPathName( fTreeFilename ))  
      gSystem->Exec(Form("wget http://www.uni-bonn.de/~etoerne/tmva/weightfiles/%s -O %s",fTreeFilename.Data(),fTreeFilename.Data()));
   fTestFile = new TFile(fTreeFilename);
   test_(fTestFile);
}

ReaderTest::~ReaderTest()
{
   fTestFile->Close();
   if (fTestFile) delete fTestFile;
}

void ReaderTest::run()
{
   test_(fTestFile && fTestTree);
   if (!(fTestFile && fTestTree)) return;
   const int nTest=1; // ToDo 3 reader usages
   const float effS=0.301;
   float testTarget,readerVal=0.;
   
   // setup test tree access
   vector<float>  testvar(fNVar);
   vector<float>  dummy(fNVar);
   vector<float>  dummy2(fNVar);
   vector<float>  testvarFloat(fNVar);
   vector<double> testvarDouble(fNVar);
   vector<float>  specvar(fNSpecs);
   for (UInt_t i=0;i<fNVar;i++){
      fTestTree->SetBranchAddress(fBranchNames->at(i),&testvar[i]);
   }
   bool isCuts = fMethodTitle.Contains("Cuts");
   //cout << "val="<<fMethodTitle.Data()<< (isCuts? " _cut_":"")<<endl;
   bool method_was_booked = !(fTestTree->SetBranchAddress(fMethodTitle.Data(),&testTarget));
   test_(method_was_booked);
   if (!method_was_booked) return;
   TString readerName = fMethodTitle ;
   double diff, maxdiff = 0., sumdiff=0., previousVal=0.;
   int stuckCount=0;
   int nevt= TMath::Min((int) fTestTree->GetEntries(),fMaxEvt);

   std::vector< TMVA::Reader* > reader(nTest);
   for (int iTest=0;iTest<nTest;iTest++){
      //std::cout << "iTest="<<iTest<<std::endl;
#ifdef VERBOSE
      reader[iTest] = new TMVA::Reader( "!Color:!Silent" );
#else
      reader[iTest] = new TMVA::Reader( "!Color:Silent" );
#endif
      for (UInt_t i=0;i<fNVar;i++)
         reader[iTest]->AddVariable( fVariableNames->at(i),&testvar[i]);
      for (UInt_t j=0;j<fNSpecs;j++) reader[iTest]->AddSpectator( fSpecNames->at(j) ,&specvar[j]); 

      reader[iTest] ->BookMVA( readerName, fWeightFile) ;

      // run the reader application and compare to test tree
      for (Long64_t ievt=0;ievt<nevt;ievt++) {
         //if (ievt<3) std::cout << "ievt="<<ievt<<std::endl;
         fTestTree->GetEntry(ievt);
         for (UInt_t i=0;i<fNVar;i++){
            testvarDouble[i]= testvar[i];
            testvarFloat[i]= testvar[i];
         }

         if (fIsRegression){
            if (iTest==0){ readerVal=(reader[iTest]->EvaluateRegression( readerName))[0];}
            else if (iTest==1){ readerVal=(reader[iTest]->EvaluateRegression( readerName)).at(0);}
            else if (iTest==2){ readerVal=reader[iTest]->EvaluateRegression( 0, readerName);}
            else {
               std::cout << "ERROR, undefined iTest value "<<iTest<<endl;
               exit(1);
            }
         }
         else {
            if (iTest==0){
               if (isCuts) 
                  readerVal = reader[iTest]->EvaluateMVA( readerName, effS );
               else readerVal=reader[iTest]->EvaluateMVA( readerName);  
            } 
            else if (iTest==1){
               if (isCuts)
                  readerVal = reader[iTest]->EvaluateMVA( testvarFloat, readerName, effS );
               else readerVal=reader[iTest]->EvaluateMVA( testvarFloat, readerName);  
            }
            else if (iTest==2){
               if (isCuts) 
                  readerVal = reader[iTest]->EvaluateMVA( testvarDouble, readerName, effS );
               else readerVal=reader[iTest]->EvaluateMVA( testvarDouble, readerName);  
            }
            else {
               std::cout << "ERROR, undefined iTest value "<<iTest<<endl;
               exit(1);
            }
         }
         diff = TMath::Abs(readerVal-testTarget);
         maxdiff = diff > maxdiff ? diff : maxdiff;
         sumdiff += diff;
         if (ievt>0 && iTest ==0 && TMath::Abs(readerVal-previousVal)<1.e-6) stuckCount++;
         //if (ievt<5) std::cout << "i="<<iTest<<", readerVal="<<readerVal<<" testTarget"<<testTarget<<" diff="<<diff<<" stuckcount="<<stuckCount<<std::endl;

         if (iTest ==0 ) previousVal=readerVal;
      }

   }

   sumdiff=sumdiff/nevt;

   test_(maxdiff <1.e-4);
   test_(sumdiff <1.e-5);
   if (!isCuts) test_(stuckCount<nevt/10);

   for (int i=0;i<nTest;i++) delete reader[i];

#ifdef COUTDEBUG
   cout << "end of reader test maxdiff="<<maxdiff<<", sumdiff="<<sumdiff<<" stuckcount="<<stuckCount<<endl;
#endif
}

