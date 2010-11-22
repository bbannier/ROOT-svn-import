//////////////////////////////////////////////////////////////////////////
//
// 'SPECIAL PDFS' RooFit tutorial macro #706
// 
// Histogram based p.d.f.s and functions
//
//
//
// 07/2008 - Wouter Verkerke 
// 
/////////////////////////////////////////////////////////////////////////

#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooRealVar.h"
#include "RooDataSet.h"
#include "RooGaussian.h"
#include "RooPolynomial.h"
#include "RooHistPdf.h"
#include "RooHistFunc.h"
#include "TCanvas.h"
#include "RooPlot.h"
using namespace RooFit ;


void testPieceWise()
{

  RooRealVar x("x","x",0,-10,20) ;
  RooRealVar m1("m1","m1",0, -10,10) ;
  RooRealVar m2("m2","m2",5, -10,10) ;
  RooRealVar s1("s1","s1",1, 0,20) ;
  RooRealVar a("a","a",1, 0,20) ;
  RooRealVar b("b","b",1, 0,20) ;
  RooRealVar c("c","c",10, 0,100) ;
  RooRealVar d("d","d",10, 0,100) ;
  RooGaussian g1("g1","g1",x,m1,s1);
  RooGaussian g2("g2","g2",x,m2,s1);
  RooProduct g1s("g1s","g1s",RooArgSet(g1,c));
  RooProduct g2s("g2s","g2s",RooArgSet(g2,d));
  m1.setConstant();
  m2.setConstant();
  s1.setConstant();

  RooRealSumPdf rsum("rsum","rsum",RooArgList(g1,g2), RooArgList(a,b));
  cout << "rsum.expectedEvents = " << rsum.expectedEvents(x) <<endl;

  return;
  //  RooAbsReal* rsumInt = rsum.createIntegral(x,NormSet(x));
  RooAbsReal* rsumInt = rsum.createCdf(x);
  cout << "rsumInt.getValue = " << rsumInt->getVal() <<endl;
  //  RooAbsReal* g1Int = g1.createIntegral(x,NormSet(x));
  RooAbsReal* g1Int = g1.createCdf(x);
  cout << "g1Int.getValue = " << g1Int->getVal() <<endl;
  RooAbsReal* g1sInt = g1s.createIntegral(x,NormSet(x));
  //  RooAbsReal* g1sInt = g1s.createCdf(x);
  cout << "g1sInt.getValue = " << g1sInt->getVal() <<endl;


  // C r e a t e   l o w   s t a t s   h i s t o g r a m
  // ---------------------------------------------------

  // Sample 500 events from p
  x.setBins(20) ;
  RooDataSet* data1 = p.generate(x,5000) ;
  a.setVal(.012);
  b.setVal(-.0107);
  RooDataSet* dataD = p.generate(x,5000) ;
  a.setVal(.008);
  b.setVal(-.009);
  RooDataSet* dataU = p.generate(x,5000) ;
  
  // Create a binned dataset with 20 bins and 500 events
  RooDataHist* hist1 = data1->binnedClone() ;
  RooDataHist* histD = dataD->binnedClone() ;
  RooDataHist* histU = dataU->binnedClone() ;

  // Represent data in dh as pdf in x
  RooHistFunc histpdf1("histpdf1","histpdf1",x,*hist1,0) ;
  RooHistFunc histpdfD("histpdfD","histpdfD",x,*histD,0) ;
  RooHistFunc histpdfU("histpdfU","histpdfU",x,*histU,0) ;

  cout << "hist pdf 1 integral = " << histpdf1.analyticalIntegral(histpdf1.getAnalyticalIntegral(x,x)) << endl;

  // do linear interpolation
  RooRealVar alpha("alpha","alpha",-1,1);
  RooRealVar beta("beta","beta",1,0,10);
  //  beta.setConstant();
  PiecewiseInterpolation modelFunc("modelFunc","model",histpdf1, RooArgSet(histpdfD),RooArgSet(histpdfU),RooArgSet(alpha));
  // make model out of function, using norm as expected events in extended term
  RooRealSumPdf model("model","model",RooArgSet(modelFunc), RooArgSet(beta),kTRUE);

  alpha.setVal(.5);
  //RooAbsData* newDS = model.generateBinned(x,2000);
  RooAbsData* newDS = model.generate(x,300); // expected events = 1 with this.
  alpha.setVal(1.);
  model.expectedEvents(x);
  cout << "expected events = " << model.expectedEvents(x) << endl;
  model.fitTo(*newDS);
  cout << "expected events = " << model.expectedEvents(x) << endl;

  // Plot unbinned data and histogram pdf overlaid
  RooPlot* frame1 = x.frame(Title("Low statistics histogram pdf"),Bins(100)) ;
  newDS->plotOn(frame1);
  /*
  histpdf1.plotOn(frame1, LineColor(kBlack)) ;  
  histpdfD.plotOn(frame1, LineColor(kRed)) ;  
  histpdfU.plotOn(frame1, LineColor(kGreen)) ;  
  */
  model.plotOn(frame1, LineColor(kMagenta), LineStyle(kDotted)) ;
  /*
  alpha.setVal(0);
  modelFunc.plotOn(frame1, LineColor(kMagenta), LineStyle(kDotted)) ;
  alpha.setVal(1);
  modelFunc.plotOn(frame1, LineColor(kYellow), LineStyle(kDotted)) ;
  alpha.setVal(-1);
  modelFunc.plotOn(frame1, LineColor(kOrange), LineStyle(kDotted)) ;
  alpha.setVal(.8);
  modelFunc.plotOn(frame1, LineColor(kBlue), LineStyle(kSolid)) ;
  */
  
  /////////////////////////////

  cout << "ok, here we go" << endl;
  TH1* hh=0;
  //  TH1* hh = model.createHistogram("hh",x,YVar(alpha,Binning(20)));
  cout << "made hist" << hh << endl;

  TCanvas* can = new TCanvas("testPieceWise","testPieceWise",800,400) ;  
  can->Divide(2) ;
  can->cd(1) ; frame1->Draw() ;
  //  can->cd(2) ; hh->Draw("surf") ;


}
