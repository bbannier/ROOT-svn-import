//  root -l
//  .L createComparisonData.C
// CreateDataForInputTests()

#include "TROOT.h"
#include "TMath.h"
#include "TTree.h"
#include "TArrayD.h"
#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TRandom.h"
#include "Riostream.h"
#include "TCanvas.h"
#include "TMatrixD.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TBranch.h"
#include <vector>

void CreateDataForInputTests(Int_t nmax = 10000, Int_t nmax2=150000, const char* filename = "TMVAInputData.root" ){
   // create testtreeS1, S2, SSum, treeSFalse  var1 --> -var1, all effis should be zero
   // create traintreeS1, S2, SSum, 
   // same for Background tree labelling S-->B

   // vars 1,2,3,4, weight, flag I am signal, flag I am training,
   bool debug = false;
   // output file
   TFile* dataFile = TFile::Open( filename, "RECREATE" );
   int nvar = 4+1;
   int nvarInt = 0;
   int nsig = 0, nbgd=0;
   int n1=nmax/5;
   Float_t xvar[100], xin[100],xgaus[100];
   TRandom R( 100 );
   TTree* treeS1Train = new TTree( "TreeS1Train", "TreeS1Train", 1 );
   TTree* treeS2Train = new TTree( "TreeS2Train", "TreeS2Train", 1 );
   TTree* treeSSumTrain = new TTree( "TreeSSumTrain", "TreeSSumTrain", 1 );
   TTree* treeS1Test = new TTree( "TreeS1Test", "TreeS1Test", 1 );
   TTree* treeS2Test = new TTree( "TreeS2Test", "TreeS2Test", 1 );
   TTree* treeSSumTest = new TTree( "TreeSSumTest", "TreeSSumTest", 1 );
   TTree* treeSFake = new TTree( "TreeSFake", "TreeSFake", 1 );

   TTree* treeB1Train = new TTree( "TreeB1Train", "TreeB1Train", 1 );
   TTree* treeB2Train = new TTree( "TreeB2Train", "TreeB2Train", 1 );
   TTree* treeBSumTrain = new TTree( "TreeBSumTrain", "TreeBSumTrain", 1 );
   TTree* treeB1Test = new TTree( "TreeB1Test", "TreeB1Test", 1 );
   TTree* treeB2Test = new TTree( "TreeB2Test", "TreeB2Test", 1 );
   TTree* treeBSumTest = new TTree( "TreeBSumTest", "TreeBSumTest", 1 );
   TTree* treeBFake = new TTree( "TreeBFake", "TreeBFake", 1 );

   TTree* treeSBSumTest = new TTree( "TreeSBSumTest", "TreeSBSumTest", 1 );
   TTree* treeSBSumTrain = new TTree( "TreeSBSumTrain", "TreeSBSumTrain", 1 );
   TTree* treeSBLarge = new TTree( "TreeSBLarge", "TreeSBLarge", 1 );

   vector<TTree*> trees;
   trees.push_back(treeS1Train); //0
   trees.push_back(treeS2Train);
   trees.push_back(treeSSumTrain);
   trees.push_back(treeS1Test); //3
   trees.push_back(treeS2Test);
   trees.push_back(treeSSumTest);
   trees.push_back(treeSFake); //6

   trees.push_back(treeB1Train); //7
   trees.push_back(treeB2Train);
   trees.push_back(treeBSumTrain);
   trees.push_back(treeB1Test); //10
   trees.push_back(treeB2Test);
   trees.push_back(treeBSumTest);
   trees.push_back(treeBFake); //13

   trees.push_back(treeSBSumTest); //14
   trees.push_back(treeSBSumTrain); //15
   trees.push_back(treeSBLarge); //16

   ofstream fileSSumTrain,fileSSumTest,fileBSumTrain,fileBSumTest;
   fileSSumTrain.open("fileSSumTrain.dat");
   fileSSumTrain<<"var1/F:var2/F:var3/F:var4/F:weight/F:istest/F:isfake/F:issig/F:evtno/F"<<endl;
   fileSSumTest.open("fileSSumTest.dat");
   fileSSumTest<<"var1/F:var2/F:var3/F:var4/F:weight/F:istest/F:isfake/F:issig/F:evtno/F"<<endl;
   fileBSumTrain.open("fileBSumTrain.dat");
   fileBSumTrain<<"var1/F:var2/F:var3/F:var4/F:weight/F:istest/F:isfake/F:issig/F:evtno/F"<<endl;
   fileBSumTest.open("fileBSumTest.dat");
   fileBSumTest<<"var1/F:var2/F:var3/F:var4/F:weight/F:istest/F:isfake/F:issig/F:evtno/F"<<endl;
   Float_t weight, istest, isfake, issig, evtno;
   for (Int_t ivar=1; ivar<nvar; ivar++) {
      for (Int_t itree=0; itree<trees.size(); itree++) {
         trees[itree]->Branch( TString(Form( "var%i", ivar )).Data(), &xvar[ivar], TString(Form( "var%i/F", ivar )).Data() );
         trees[itree]->Branch("weight",&weight,"weight/F" );
         trees[itree]->Branch("istest",&istest,"istest/F" ); // 1=true
         trees[itree]->Branch("isfake",&isfake,"isfake/F" );
         trees[itree]->Branch("issig",&issig,"issig/F" );
         trees[itree]->Branch("evtno",&evtno,"evtno/F" );
      }
   }
   do {
      for (Int_t ivar=0; ivar<=nvar+nvarInt; ivar++) { 
         xin[ivar]=R.Rndm();
         xgaus[ivar]=R.Gaus(0.,1.);
      }

      bool isSignal = (R.Rndm()>0.5);      
      issig  = isSignal? 1. : 0.;
      weight = 1.;
      // flat displaced
      if (isSignal) xvar[1]= TMath::Abs(xgaus[1]);         
      else          xvar[1]= TMath::Abs(xgaus[1])+0.5;
      if (isSignal) xvar[2]= xgaus[2];         
      else          xvar[2]= xgaus[2]+0.5;
      if (isSignal) xvar[3]= xgaus[3];         
      else          xvar[3]= xgaus[3]+0.5;
      if (isSignal) xvar[4]= xgaus[4];         
      else          xvar[4]= xgaus[4]+0.5;
      //-----------------------     
      if (isSignal) evtno=nsig;
      else evtno = nbgd;

      if (isSignal && nsig < nmax){
         isfake=0.;
         if (nsig<n1){ // train 1
            istest=0.;
            treeS1Train->Fill();
            treeSSumTrain->Fill();
            treeSBSumTrain->Fill();
            fileSSumTrain << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nsig<2*n1){ //train2
            istest=0.;
            treeS2Train->Fill();
            treeSSumTrain->Fill();
            treeSBSumTrain->Fill();
            fileSSumTrain << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nsig<3*n1){ // test1
            istest=1.;
            xvar[2]+=0.1;
            treeS1Test->Fill();
            treeSSumTest->Fill();
            treeSBSumTest->Fill();
            fileSSumTest << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nsig<4*n1){ // test2
            istest=1.;
            xvar[2]+=0.1;
            treeS2Test->Fill();
            treeSSumTest->Fill();
            treeSBSumTest->Fill();
            fileSSumTest << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nsig<5*n1){ //fake test
            istest=1.;
            isfake=1.;
            xvar[1]=-xvar[1];
            treeSFake->Fill();
         }
         nsig++;
      }
      else if (isSignal && nsig < nmax2){
         isfake=0.;
         istest=0.;
         treeSBLarge->Fill();
         nsig++;
      }

      if (!isSignal && nbgd < nmax){
         isfake=0.;
         if (nbgd<n1){ // train 1
            istest=0.;
            treeB1Train->Fill();
            treeBSumTrain->Fill();
            treeSBSumTrain->Fill();
            fileBSumTrain << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nbgd<2*n1){ //train2
            istest=0.;
            treeB2Train->Fill();
            treeBSumTrain->Fill();
            treeSBSumTrain->Fill();
            fileBSumTrain << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nbgd<3*n1){ // test1
            istest=1.;
            treeB1Test->Fill();
            treeBSumTest->Fill();
            treeSBSumTest->Fill();
            fileBSumTest << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;   
         }
         else if (nbgd<4*n1){ // test2
            istest=1.;
            treeB2Test->Fill();
            treeBSumTest->Fill();
            treeSBSumTest->Fill();
            fileBSumTest << xvar[1] << " " <<xvar[2] << " " <<xvar[3] << " " <<xvar[4] << " " << weight<< " " << istest<< " " << isfake<<" " <<issig<<" " <<evtno<<endl;  
         }
         else if (nbgd<5*n1){ //fake test
            istest=1.;
            isfake=1.;
            xvar[1]=-xvar[1];
            treeBFake->Fill();
         }
         nbgd++;
      }
      else if (!isSignal && nbgd < nmax2){
         isfake=0.;
         istest=0.;
         treeSBLarge->Fill();
         nbgd++;
      }

      if ((nsig+nbgd)%100==0) cout << "event="<<nsig <<" " <<nbgd<<endl;
   } while ( nsig < nmax2 || nbgd < nmax2);
   dataFile->Write();
   dataFile->Close();
   fileSSumTrain.close();
   fileSSumTest.close();
   fileBSumTrain.close();
   fileBSumTest.close();
}

