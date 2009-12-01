#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"

/*
 * Author: Eckhard von Toerne, U. of Bonn
 * Test classes for the DataInput tests
 */

class TestItem{
public:
   int     fNTrainSig, fNTrainBg, fNTestSig, fNTestBg, fNTrain, fNTest;
   double  fFracTrain,fFracTest;
   TString fFake;

   TestItem(){}
   TestItem(int NTrainSig, int NTrainBg, int NTestSig, int NTestBg, const char* Fake) : fNTrainSig(NTrainSig), fNTrainBg(NTrainBg), fNTestSig(NTestSig), fNTestBg(NTestBg), fFake(Fake), fNTrain(NTrainSig+NTrainBg),fNTest(NTestSig+NTestBg){
      fFracTrain = ((double) fNTrainSig)/fNTrain;
      fFracTest  = ((double) fNTestSig )/fNTest;
      fFake.ToLower();
   } 

   ~TestItem(){}

};

class DataInputTest {
private: 
   TTree* fTrainTree;
   TTree* fTestTree;
   std::vector<TestItem> fList;
   //static const TString fFakeName("isfake");
   //static const TString fSigName("issig");
   //static const TString fTestName("istest");
   static const double fEps = 1.e-4;

public:
   DataInputTest() : fTrainTree(0), fTestTree(0) {}

   ~DataInputTest(){fList.clear();}

   void SetTrees(TTree* ttrain, TTree* ttest){fTrainTree=ttrain; fTestTree=ttest;}

   void RegisterAssertion(int ntrainSig, int ntrainBg, int ntestSig, int ntestBg, const char* fakes="hasnofakes")
   {
      fList.push_back(TestItem(ntrainSig, ntrainBg, ntestSig, ntestBg, fakes));
   }

   void CheckAssertions()
   {
      assert(fList.size()==1);
      for (int i=0;i<fList.size();i++){
         assert(GetTreeValue(fTrainTree,"","ENTRIES")==fList[i].fNTrain);
         assert(TMath::Abs(GetTreeValue(fTrainTree,"issig","MEAN")-fList[i].fFracTrain)<fEps);
         assert(GetTreeValue(fTestTree,"","ENTRIES")==fList[i].fNTest);
         assert(TMath::Abs(GetTreeValue(fTestTree,"issig","MEAN")-fList[i].fFracTest)<fEps);
         if (fList[i].fFake!=TString("testtrainmixed")){
               assert(GetTreeValue(fTrainTree,"istest","MAX")<fEps);
               assert(GetTreeValue(fTestTree,"istest","MIN")>1.-fEps);
         }
         if (fList[i].fFake==TString("hasnofakes") || 
             fList[i].fFake==TString("testtrainmixed")){
            assert(GetTreeValue(fTrainTree,"isfake","MAX")<fEps);
            assert(GetTreeValue(fTestTree,"isfake","MAX")<fEps);
         }
         else if (fList[i].fFake==TString("hastrainfakes")){
            assert(GetTreeValue(fTrainTree,"isfake","MAX")>1.-fEps);
            assert(GetTreeValue(fTestTree,"isfake","MAX")<fEps);            
         }
         else if (fList[i].fFake==TString("hastestfakes")){
            assert(GetTreeValue(fTrainTree,"isfake","MAX")<fEps);
            assert(GetTreeValue(fTestTree,"isfake","MAX")>1.-fEps);            
         }
         else if (fList[i].fFake==TString("hastraintestfakes")){
            assert(GetTreeValue(fTrainTree,"isfake","MAX")>1.-fEps);
            assert(GetTreeValue(fTestTree,"isfake","MAX")>1.-fEps);            
         }
         else {
            std::cout <<" unknown fake option "<<fList[i].fFake<<std::endl;
         }
      }
   }

   double GetTreeValue(TTree* tree, const char* varname, const char* type)
   {
      TString typ(type);
      typ.ToLower();
      if (typ=="min")     return tree->GetMinimum(varname);
      if (typ=="max")     return  tree->GetMaximum(varname);
      if (typ=="entries") return tree->GetEntries();      
      TH1D* htemp = new TH1D("htemp","htemp", 5000, 
                             tree->GetMinimum(varname),
                             tree->GetMaximum(varname)+1.e-5);
      tree->Draw(Form("%s>>htemp",varname), "", "q");
      double val=0.;
      if (typ=="mean"){ 
         val = htemp->GetMean();
         std::cout << "mean="<<val<<std::endl;
      }
      if (typ=="rms") val= htemp->GetRMS();
      delete htemp;
      return val;
   }

};
