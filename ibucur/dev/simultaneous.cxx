
#include <string>
#include "test.h"

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "TRandom.h"
#include "TBenchmark.h"
#include "RooRealVar.h"
#include "RooBernstein.h"
#include "RooStringVar.h"
#include "TMath.h"
#include "RooDataHist.h"
#include "RooCategory.h"
#include "RooPoisson.h"
#include "RooGaussian.h"
#include "RooUniform.h"
#include "RooExponential.h"
#include "RooPlot.h"
#include "TCanvas.h"
#include "RooRandom.h"
#include "RooDataSet.h"
#include "RooBreitWigner.h"
#include "RooAbsDataStore.h"

using namespace RooFit;

void test() {

   gRandom->SetSeed(12345);
   RooRandom::randomGenerator()->SetSeed(12346);

   RooRealVar varA("a", "a", 0, 10);
   RooRealVar varB("b", "b", 0, 10);
   RooRealVar varC("c", "c", 0, 10);
   RooRealVar p("p", "p", 6);
   RooRealVar m("m", "m", 0);
   RooRealVar s("s", "s", 1, 5);
   RooRealVar m2("m2", "m2", 5);
   RooRealVar w("w","w",5);

   varA.setBins(10);
   varB.setBins(10);
   varC.setBins(10);


   RooGaussian gaus("gaus","gaus", varA, m, s);
   RooPoisson pois("pois", "pois", varB, p);
   RooBreitWigner bw("bw", "bw", varC, m2, w);
   
   gaus.setAttribute("bin");
   pois.setAttribute("bin");
   bw.setAttribute("bin");


   RooDataHist *histA = gaus.generateBinned(varA, 30);
   RooDataHist *histB = pois.generateBinned(varB, 30);
   RooDataHist *histC = bw.generateBinned(varC, 30);

//   RooDataHist *histA2 = gaus.generateBinned(varA, 30);
//   RooDataHist *histA3 = gaus.generateBinned(varA, 30);


   RooDataSet *dataA = gaus.generate(varA, 30, GenBinned("bin"));
   RooDataSet *dataB = pois.generate(varB, 30, AllBinned());
   RooDataSet *dataC = bw.generate(varC, 30, GenBinned("bin"));


   std::cout << "Entries ABC: " << dataA->numEntries()
             << " " << dataB->numEntries()
             << " " << dataC->numEntries() << std::endl;

   std::cout << "Entries hist ABC: " << histA->numEntries()
             << " " << histB->numEntries()
             << " " << histC->numEntries() << std::endl;
   for(Int_t i = 0; i < dataA->numEntries(); i++) {
//      dataA->store()->get(i)->Print("v"); std::cout << "weight " << dataA->weight() << std::endl;
      //histA->get(i)->Print("v"); std::cout << "weight " << histA->weight() << std::endl;
      //dataB->get(i)->Print("v"); std::cout << "weight " << dataB->weight() << std::endl;
      //dataC->get(i)->Print("v"); std::cout << "weight " << dataC->weight() << std::endl;
   }


   RooCategory cat("cat","cat");
   cat.defineType("sampleA");
   cat.defineType("sampleB");
   cat.defineType("sampleC");

   std::map<std::string, RooDataSet *> mapData;
   mapData[ "sampleA" ] = dataA;
   mapData[ "sampleB" ] = dataB;
   mapData[ "sampleC" ] = dataC;

   std::map<std::string, RooDataHist *> mapHist;
   mapHist[ "sampleA" ] = histA;
   mapHist[ "sampleB" ] = histB;
   mapHist[ "sampleC" ] = histC;

//   RooDataHist dataset("data","data", RooArgSet(varA, varB, varC), cat, mapData);

   RooDataSet dataset("data","data", RooArgSet(varA, varB, varC), Index(cat),
      Import("sampleA", *dataA), Import("sampleB", *dataB), Import("sampleC", *dataC));


   TCanvas *ca = new TCanvas("ca","ca",1600, 1200);
   ca->Divide(3,2);

   RooPlot *pA = varA.frame();
   dataA->plotOn(pA);
   ca->cd(1);
   pA->Draw("");

   RooPlot *pB = varB.frame();
   dataB->plotOn(pB);
   ca->cd(2);
   pB->Draw("");

   RooPlot *pC = varC.frame();
   dataC->plotOn(pC);
   ca->cd(3);
   pC->Draw("");



   RooPlot *pAll = varA.frame();
   dataset.plotOn(pAll, Cut("cat==cat::sampleA"));
   ca->cd(4);
   pAll->Draw("");

   RooPlot *pBll = varB.frame();
   dataset.plotOn(pBll, Cut("cat==cat::sampleB"));
   ca->cd(5);
   pBll->Draw("");
     
   RooPlot *pCll = varC.frame();
   dataset.plotOn(pCll, Cut("cat==cat::sampleC"));
   ca->cd(6);
   pCll->Draw("");


   Int_t totalEntries = dataset.numEntries();
   std::cout << "Entries combined " << totalEntries << std::endl;

   for(Int_t i = 0; i < totalEntries; i++) {
//      RooRealVar *a = (RooRealVar *)dataset.get(i)->find("a");
  //    RooRealVar *b = (RooRealVar *)dataset.get(i)->find("b");
    //  RooRealVar *c = (RooRealVar *)dataset.get(i)->find("c");
      //RooCategory *category = (RooCategory *)dataset.get(i)->find("cat");

//      category->Print();
      dataset.get(i);
      
  //    dataset.get(i)->Print("");
//   std::cout << "dataset.get(i) " << dataset.get(i) << std::endl;
          std::cout << "  weight(" << i << "): " << dataset.weight() << std::endl;
//          std::cout << "a " << a->getVal() << std::endl; 
//          std::cout << "b " << b->getVal() << std::endl; 
//          std::cout << "c " << c->getVal() << std::endl; 
   }



}


int main() {
   test();
   return 0;
}
