#include "MethodUnitTestWithROCLimits.h"
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TMath.h"
#include "TMVA/MethodBase.h"
#include "TMVA/Reader.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define USETESTTREE

using namespace std;
using namespace UnitTesting;
using namespace TMVA;

MethodUnitTestWithROCLimits::MethodUnitTestWithROCLimits(const Types::EMVA& theMethod, const TString& methodTitle, const TString& theOption,
														double lowLimit, double upLimit,
                                                         const std::string & /* xname */ ,const std::string & /* filename */ , std::ostream* /* sptr */ ) :
   UnitTest((string)methodTitle, __FILE__), _methodType(theMethod) , _methodTitle(methodTitle), _methodOption(theOption), _upROCLimit(upLimit), _lowROCLimit(lowLimit), _VariableNames(0), _TreeVariableNames(0)
{
   _VariableNames  = new std::vector<TString>(0);
   _TreeVariableNames = new std::vector<TString>(0);
   _VariableNames->push_back("var1+var2");
   _VariableNames->push_back("var1-var2");
   _VariableNames->push_back("var3");
   _VariableNames->push_back("var4");
   _TreeVariableNames->push_back("myvar1");
   _TreeVariableNames->push_back("myvar2");
   _TreeVariableNames->push_back("var3");
   _TreeVariableNames->push_back("var4");
}


MethodUnitTestWithROCLimits::~MethodUnitTestWithROCLimits()
{
   delete  _VariableNames;
   delete _TreeVariableNames;
}

bool MethodUnitTestWithROCLimits::ROCIntegralWithinInterval()
{
   return (_ROCValue <= _upROCLimit) && (_ROCValue >= _lowROCLimit);
}

void MethodUnitTestWithROCLimits::run()
{
   TString outfileName = Form("weights/TMVA_%s.root",_methodTitle.Data());
   TFile* outputFile = TFile::Open( outfileName, "RECREATE" );         

// FIXME:: if file can't be created do something more?
  if(!outputFile)
    return;    

// FIXME:: make the factory option mutable?
// absolute silence options:
#ifdef VERBOSE
  string factoryOptions( "V:!Silent:AnalysisType=Classification:!Color:!DrawProgressBar" );
#else
  string factoryOptions( "!V:Silent:AnalysisType=Classification:!Color:!DrawProgressBar" );
#endif

  if (_methodOption.Contains("VarTransform")) factoryOptions+=":Transformations=I;D;P;G;N;U";
  Factory* factory = new Factory( "TMVAUnitTesting", outputFile, factoryOptions );
  //  factory->AddVariable( "myvar1 := var1+var2", 'F' );
  //  factory->AddVariable( "myvar2 := var1-var2", "Expression 2", "", 'F' );
  factory->AddVariable( Form("%s  := %s",_TreeVariableNames->at(0).Data(), _VariableNames->at(0).Data()), 'F' );
  factory->AddVariable( Form("%s  := %s",_TreeVariableNames->at(1).Data(), _VariableNames->at(1).Data()), "Expression 2", "",'F' );
  factory->AddVariable( _VariableNames->at(2),                "Variable 3", "units", 'F' );
  factory->AddVariable( _VariableNames->at(3),                "Variable 4", "units", 'F' );
  
  TFile* input(0);

  FileStat_t stat;
  
  TString fname = "../tmva/test/data/toy_sigbkg.root"; //tmva_example.root"; 
  if(!gSystem->GetPathInfo(fname,stat)) {
     input = TFile::Open( fname );
  } else if(!gSystem->GetPathInfo("../"+fname,stat)) {
     input = TFile::Open( "../"+fname );
  } else {
     input = TFile::Open( "http://root.cern.ch/files/tmva_example.root" );
  }
  if (input == NULL) {
     cerr << "broken/inaccessible input file" << endl;
  }
  
  TTree *signal     = (TTree*)input->Get("TreeS");
  TTree *background = (TTree*)input->Get("TreeB");
  
  factory->AddSignalTree(signal);  
  factory->AddBackgroundTree(background);
  
  factory->SetBackgroundWeightExpression("weight");
  
  TCut mycuts = "";
  TCut mycutb = "";
  
  // FIXME:: make options string mutable?
  factory->PrepareTrainingAndTestTree( mycuts, mycutb,
				       "nTrain_Signal=1000:nTrain_Background=1000:nTest_Signal=5000:nTest_Background=5000:SplitMode=Random:NormMode=NumEvents:!V" );
 
  factory->BookMethod(_methodType, _methodTitle, _methodOption);

  factory->TrainAllMethods();

  factory->TestAllMethods();

  factory->EvaluateAllMethods();

  _theMethod = dynamic_cast<TMVA::MethodBase*> (factory->GetMethod(_methodTitle));

  if (_methodType == TMVA::Types::kCuts) {
     // ToDo make class variable _theEffi
     Double_t err=0.;
     Double_t effi = _theMethod->GetEfficiency("Efficiency:0.1", Types::kTesting,err);
#ifdef COUTDEBUG
     std::cout << "Cuts Signal effi at for Background effi of 0.1 = " << effi<<" low limit="<<_lowROCLimit<<" high limit="<<_upROCLimit<<std::endl;
#endif
     test_(effi <= _upROCLimit && effi>=_lowROCLimit);
  }
  else {
     _ROCValue = _theMethod->GetROCIntegral();
#ifdef COUTDEBUG
     std::cout << "ROC integral = "<<_ROCValue <<" low limit="<<_lowROCLimit<<" high limit="<<_upROCLimit<<std::endl;
#endif     
     if (!ROCIntegralWithinInterval()){
        if (_ROCValue < _lowROCLimit) 
           std::cout << "failure in "       << _methodTitle
                     << ", ROC integral = " << _ROCValue 
                     <<" low limit="        << _lowROCLimit
                     <<" high limit="       << _upROCLimit << std::endl;
        else  
           std::cout << "Improvement of method beyond upper limit (counted as \"failure\"): " 
                     << _methodTitle
                     << ", ROC integral = " << _ROCValue  
                     << " low limit="       << _lowROCLimit
                     << " high limit="      << _upROCLimit << std::endl;
     }     
     test_(ROCIntegralWithinInterval());
  }
  outputFile->Close();
  delete factory;
  if (outputFile) delete outputFile;
  input->Close();
  if (input) delete input;

  // Reader tests
  const int nTest=6; // 3 reader usages + 3 tests with additional readers
  float testTreeVal,readerVal=0.;
  vector<float>  testvar(_VariableNames->size());
  vector<float>  dummy(_VariableNames->size());
  vector<float>  dummy2(_VariableNames->size());
  vector<float>  testvarFloat(_VariableNames->size());
  vector<double> testvarDouble(_VariableNames->size());

  // setup test tree access
  TFile* testFile = new TFile(Form("weights/TMVA_%s.root",_methodTitle.Data()));
#ifdef USETESTTREE
  TTree* testTree = (TTree*)(testFile->Get("TestTree"));
#else
  TTree* testTree = (TTree*)(testFile->Get("TrainTree"));
#endif
  for (UInt_t i=0;i<_VariableNames->size();i++)
     testTree->SetBranchAddress(_TreeVariableNames->at(i),&testvar[i]);
  testTree->SetBranchAddress(_methodTitle.Data(),&testTreeVal);

  std::vector<TString> variableNames2;
  variableNames2.push_back("var0");
  variableNames2.push_back("var1");
  TFile* testFile2 = new TFile("weights/ByHand.root");
  TTree* testTree2 = (TTree*)(testFile2->Get("TestTree"));
  testTree2->SetBranchAddress("var0",&dummy[0]);
  testTree2->SetBranchAddress("var1",&dummy[1]);

  TString readerName = _methodTitle + TString(" method");
  TString readerOption="!Color:Silent";
  TString dir    = "weights/TMVAUnitTesting_";
  TString weightfile=dir+_methodTitle+".weights.xml";
  TString weightfile2="weights/ByHand_BDT.weights.xml"; //TMVATest3VarF2VarI_BDT.weights.xml
  TString readerName2 = "BDT method";
  double diff=-1.f, maxdiff = 0., sumdiff=0., previousVal=0.;
  int stuckCount=0, nevt= TMath::Min((int) testTree->GetEntries(),100);
  const float effS=0.301;

  TMVA::Reader* reader2=0;
  std::vector< TMVA::Reader* > reader(nTest);
  for (int iTest=0;iTest<nTest;iTest++){
     //std::cout << "iTest="<<iTest<<std::endl;
     if (iTest==0){
        reader[iTest] = new TMVA::Reader( readerOption );
        for (UInt_t i=0;i<_VariableNames->size();i++)
           reader[iTest]->AddVariable( _VariableNames->at(i),&testvar[i]);
        reader[iTest] ->BookMVA( readerName, weightfile) ;
     }
     else if (iTest==1 || iTest ==2) {
        reader[iTest] = new TMVA::Reader( *_VariableNames, readerOption );
        reader[iTest] ->BookMVA( readerName, weightfile) ;
     }
     else if (iTest==3) { // multiple reader
        reader[iTest] = new TMVA::Reader( *_VariableNames, readerOption );
        reader2 = new TMVA::Reader( variableNames2, readerOption );
        reader[iTest]->BookMVA( readerName, weightfile) ;
        reader2->BookMVA( readerName2, weightfile2) ;
     }
     else if (iTest==4) { // multiple reader
        reader[iTest] = new TMVA::Reader( readerOption );
        for (UInt_t i=0;i<_VariableNames->size();i++)
           reader[iTest]->AddVariable( _VariableNames->at(i),&testvar[i]);
        reader[iTest] ->BookMVA( readerName, weightfile) ;
        reader2 = new TMVA::Reader( readerOption );
        for (UInt_t j=0;j<variableNames2.size();j++) reader2->AddVariable( variableNames2.at(j),&dummy[j]);
        reader2->BookMVA( readerName2, weightfile2) ;
     }
     else if (iTest==5) { // multiple reader
        reader2 = new TMVA::Reader( readerOption );
        for (UInt_t j=0;j<variableNames2.size();j++) reader2->AddVariable( variableNames2.at(j),&dummy[j]);
        reader[iTest] = new TMVA::Reader( readerOption );
        for (UInt_t i=0;i<_VariableNames->size();i++)
           reader[iTest]->AddVariable( _VariableNames->at(i),&testvar[i]);
        reader[iTest] ->BookMVA( readerName, weightfile) ;
        reader2->BookMVA( readerName2, weightfile2) ;
     }
     else {
        std::cout <<"error, itest not known"<<std::endl;
        std::exit(1);
     }

     // run the reader application and compare to test tree  
     for (Long64_t ievt=0;ievt<nevt;ievt++) {
        testTree->GetEntry(ievt);
        if (testTree2) testTree2->GetEntry(ievt);

        for (UInt_t i=0;i<_VariableNames->size();i++){
           testvarDouble[i]= testvar[i];
           testvarFloat[i]= testvar[i];
        }

        if (iTest==0){
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( readerName);  
        } 
        else if (iTest==1){
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( testvarFloat, readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( testvarFloat, readerName);  
        }
        else if (iTest==2){
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( testvarDouble, readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( testvarDouble, readerName);  
        }
        else if (iTest==3 ){
           double dummy3 = reader2->EvaluateMVA( testvarDouble, readerName2);
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( testvarDouble, readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( testvarDouble, readerName);  
           dummy3 += reader2->EvaluateMVA( testvarDouble, readerName2);
        }
        else if (iTest==4){
           double dummy4 = reader2->EvaluateMVA( testvarDouble, readerName2);
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( testvarDouble, readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( testvarDouble, readerName);  
           dummy4 += reader2->EvaluateMVA( testvarDouble, readerName2);
        }
        else if (iTest==5){
           double dummy5 = reader2->EvaluateMVA( readerName2);
           if (_methodType==Types::kCuts) 
              readerVal = reader[iTest]->EvaluateMVA( readerName, effS );
           else readerVal=reader[iTest]->EvaluateMVA( readerName);  
           dummy5 += reader2->EvaluateMVA( readerName2);
        }
        else {
           std::cout << "ERROR, undefined iTest value "<<iTest<<endl;
           exit(1);
        }
        if (_methodType!=Types::kCuts){
           diff = TMath::Abs(readerVal-testTreeVal);
           //if (diff > 1.e-5) std::cout << "diff in reader test" <<readerVal <<" " <<testTreeVal<<std::endl;
           //else std::cout<<"diff ok"<<std::endl;
           maxdiff = diff > maxdiff ? diff : maxdiff;
           sumdiff += diff;
        }
        if (ievt>0 && iTest ==0 && TMath::Abs(readerVal-previousVal)<1.e-6) stuckCount++; 
        if (iTest ==0 ) previousVal=readerVal;
     }

     if (diff>0.f){
	std::cout << "#test: " << iTest << "  diff=" << diff << " | ";

        if (iTest==0)
	   std::cout << "reader->EvaluateMVA( \"" << readerName << "\" );  ";
        else if (iTest==1)
	   std::cout << "reader->EvaluateMVA( testvarFloat, readerName );  ";
        else if (iTest==2)
	   std::cout << "reader->EvaluateMVA( testvarDouble, readerName );  ";
        else if (iTest==3 ){
	   std::cout << "dummy3=reader->EvaluateMVA( testvarDouble, readerName2 );  ";
	   std::cout << "reader[3]=reader->EvaluateMVA( testvarDouble, readerName );  ";
	   std::cout << "dummy3+=reader->EvaluateMVA( testvarDouble, readerName2 );  ";
        }
        else if (iTest==4){
	   std::cout << "dummy4=reader->EvaluateMVA( testvarDouble, readerName2 );  ";
	   std::cout << "reader[4]=reader->EvaluateMVA( testvarDouble, readerName );  ";
	   std::cout << "dummy4+=reader->EvaluateMVA( testvarDouble, readerName2 );  ";
        }
        else if (iTest==5){
	   std::cout << "dummy5=reader->EvaluateMVA( readerName2 );  ";
	   std::cout << "reader[5]=reader->EvaluateMVA( readerName );  ";
	   std::cout << "dummy5+=reader->EvaluateMVA( readerName2 );  ";
        }
	std::cout << endl;
     }
  }
  Bool_t ok=false;
  sumdiff=sumdiff/nevt;
  if (_methodType!=Types::kCuts){
     test_(maxdiff <1.e-4);
     test_(sumdiff <1.e-5);
     test_(stuckCount<nevt/2);
     if (maxdiff <1.e-4 && sumdiff <1.e-5 && stuckCount<nevt/2) ok=true;
  }
  if (_methodType==Types::kCuts){
     test_(stuckCount<nevt-20);
     test_(sumdiff <0.005);
     if (stuckCount<nevt-20 && sumdiff <0.005) ok=true;
  }
  testFile->Close();
  if (testFile) delete testFile;
  testFile2->Close();
  if (testFile2) delete testFile2;
  for (int i=0;i<nTest;i++) delete reader[i]; 
  if (reader2) delete reader2;

  if (!ok){
     cout << "Failure in reader test "<< _methodTitle <<": maxdiff="<<maxdiff<<", sumdiff="<<sumdiff<<" stuckcount="<<stuckCount<<endl;
  } 
#ifdef COUTDEBUG
  if (ok){
     cout << "end of reader test maxdiff="<<maxdiff<<", sumdiff="<<sumdiff<<" stuckcount="<<stuckCount<<endl;
  }
#endif

  bool _DoTestCCode=true; 

  // use: grep -A5  'MakeClassSpecific' ../tmva/src/Method*.cxx

  if (_methodType==Types::kCuts  // non-implemented makeclass methods BayesClassifier CFMlpANN Committee Cuts KNN PDERS RuleFit SVM
      || _methodType==Types::kBayesClassifier
      || _methodType==Types::kCFMlpANN
      || _methodType==Types::kCommittee
      || _methodType==Types::kCuts
      || _methodType==Types::kKNN
      || _methodType==Types::kPDERS
      || _methodType==Types::kRuleFit
      || _methodType==Types::kPDEFoam
      || _methodTitle.Contains("Boosted")
      ) _DoTestCCode=false;

#ifndef FULL
  _DoTestCCode=false;
#endif

  if (_DoTestCCode){
     // create generic macro
     TString macroName=Form("testmakeclass_%s",_methodTitle.Data());
     TString macroFileName=TString("weights/")+macroName+TString(".C");
     TString methodTypeName = Types::Instance().GetMethodName(_methodType);
#ifdef COUTDEBUG
     cout << "starting standalone c-code test for type="<<methodTypeName<<endl;
#endif
     FileStat_t stat2;
     if(!gSystem->GetPathInfo(macroFileName.Data(),stat2)) {
        gSystem->Unlink(macroFileName.Data());
        //gSystem->Exec(Form("rm %s",macroFileName.Data()));
     }
     ofstream fout( macroFileName );
     fout << "// generic macro file to test TMVA reader and standalone C code " << std::endl;
     fout << "#include \"TFile.h\""<<std::endl;
     fout << "#include \"TTree.h\""<<std::endl;
     fout << "#include <vector>"<<std::endl;
     fout << "#include <iostream>"<<std::endl;
     if (methodTypeName != "TMlpANN") fout << Form("#include \"weights/TMVAUnitTesting_%s.class.C\"",_methodTitle.Data()) << std::endl;
     else fout << Form("#include \"weights/TMVAUnitTesting_%s.cxx\"",_methodTitle.Data()) << std::endl;
     fout << Form("bool %s(){",macroName.Data()) << std::endl;
     fout << Form("std::cout << \"%s\" <<std::endl;",_methodTitle.Data())<<std::endl;
     fout << "std::vector<std::string> vars(4);" << std::endl; // fix me 4
     fout << "std::vector<double> val(4);" << std::endl;  // fix me 4
     fout << "bool ok=true;" << std::endl;  // fix me 4
     for (UInt_t i=0;i<_VariableNames->size();i++)
        fout << Form("vars[%d]=\"%s\";",i,_VariableNames->at(i).Data()) << std::endl;  
     if (methodTypeName != "TMlpANN") fout << Form("Read%s  aa(vars);", _methodTitle.Data()) << std::endl;
     else fout << Form("TMVAUnitTesting_%s aa;", _methodTitle.Data()) << std::endl;
     fout << Form("TFile* testFile = new TFile(\"weights/TMVA_%s.root\");",_methodTitle.Data()) << std::endl; // fix me hardcode TMVA.root
     fout << " TTree* testTree = (TTree*)(testFile->Get(\"TestTree\"));" << std::endl;
     fout << Form("std::vector<float> testvar(%d);",(Int_t) _VariableNames->size()) << std::endl;
     fout << Form("std::vector<double> testvarDouble(%d);", (Int_t) _VariableNames->size()) << std::endl;
     for (UInt_t j=0;j<_VariableNames->size();j++)
        fout << Form("testTree->SetBranchAddress(\"%s\",&testvar[%d]);",_TreeVariableNames->at(j).Data(),j) << std::endl;
     fout << "float testTreeVal,diff,nrm,maxdiff=0.,sumdiff=0.;" << std::endl;
     fout << Form("testTree->SetBranchAddress(\"%s\",&testTreeVal);",_methodTitle.Data()) << std::endl;
     fout << "Long64_t nevt= TMath::Min((int) testTree->GetEntries(),100);" << std::endl;
     fout << "  for (Long64_t ievt=0; ievt<nevt;ievt++) {" << std::endl;
     fout << "    testTree->GetEntry(ievt);" << std::endl;
     fout << Form("for (UInt_t i=0;i<%d;i++) testvarDouble[i]= testvar[i];",(Int_t) _VariableNames->size()) << std::endl;
     if (methodTypeName != "TMlpANN") fout << "double ccode_val = aa.GetMvaValue(testvarDouble);" << std::endl;
     else fout << "double ccode_val = aa.Value(0,testvarDouble[0],testvarDouble[1],testvarDouble[2],testvarDouble[3]);" << std::endl;
     fout << "diff = TMath::Abs(ccode_val-testTreeVal);" << std::endl;
     fout << "nrm = TMath::Max(TMath::Abs(ccode_val),1.);" << std::endl;
     fout << "diff = diff/nrm;" << std::endl;
     fout << "if (diff>1.2) std::cout << \"ccode_val=\" << ccode_val <<\"testval=\" << testTreeVal <<std::endl;"<<std::endl;
     fout << "maxdiff = diff > maxdiff ? diff : maxdiff;" << std::endl;
     fout << "sumdiff += diff;" << std::endl;
     fout << "}" << std::endl;
     fout << "sumdiff=sumdiff/testTree->GetEntries();" << std::endl;
     fout << "if (maxdiff >1.e-2) std::cout << \"maxdiff=\"<<maxdiff<< \", sumdiff=\"<<sumdiff<<std::endl;" << std::endl;
     fout << "if (sumdiff >2.e-4) ok=false;" << std::endl;
     fout << "testFile->Close();" << std::endl;
     fout << "if (testFile) delete testFile;" << std::endl;
     fout << "if (!ok) {" << std::endl;
     fout << "std::cout << \"maxdiff=\"<<maxdiff<< \", sumdiff=\"<<sumdiff<<\" not ok\"<<std::endl;}" << std::endl;
     fout << "return ok;" << std::endl;
     fout << "}" << std::endl;
     
     gROOT->ProcessLine(Form(".L %s+",macroFileName.Data()));
     test_(gROOT->ProcessLine(Form("%s()",macroName.Data())));
  }

}
