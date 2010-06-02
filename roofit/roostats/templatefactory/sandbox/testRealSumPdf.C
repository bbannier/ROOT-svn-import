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


void testRealSumPdf()
{
  // C r e a t e   p d f   f o r   s a m p l i n g 
  // ---------------------------------------------

  RooRealVar x("x","x",0,-10,20) ;
  RooRealVar m1("m1","m1",0, -10,10) ;
  RooRealVar m2("m2","m2",5, -10,10) ;
  RooRealVar s1("s1","s1",1, 0,20) ;
  RooGaussian g1("g1","g1",x,m1,s1);
  RooGaussian g2("g2","g2",x,m2,s1);
  m1.setConstant();
  m2.setConstant();
  s1.setConstant();

  // normalizations
  RooRealVar a("a","a",100, 0,2000) ;
  RooRealVar b("b","b",200, 0,2000) ;

  x.setBins(50);
  // make histograms for nominal/up/down
  //    channel 1
  RooDataHist* data1 = g1.generateBinned(x,a.getVal()) ;
  m1.setVal(1.);
  RooDataHist* data1u = g1.generateBinned(x,a.getVal()) ;
  m1.setVal(-1.);
  RooDataHist* data1d = g1.generateBinned(x,a.getVal()) ;

  //    channel 2
  RooRealVar alpha2("alpha2","alpha2",0, -3,3) ;
  RooDataHist* data2 = g2.generateBinned(x,b.getVal()) ;
  s1.setVal(1.2);
  RooDataHist* data2u = g2.generateBinned(x,b.getVal()) ;
  s1.setVal(.9);
  RooDataHist* data2d = g2.generateBinned(x,b.getVal()) ;
  s1.setVal(1.);

  // convert histograms to functions
  //    channel 1
  RooHistFunc histFunc1("histFunc1","histFunc1",x,*data1,0) ;
  RooHistFunc histFunc1u("histFunc1u","histFunc1",x,*data1u,0) ;
  RooHistFunc histFunc1d("histFunc1d","histFunc1",x,*data1d,0) ;
  //    channel 2
  RooHistFunc histFunc2("histFunc2","histFunc2",x,*data2,0) ;
  RooHistFunc histFunc2u("histFunc2u","histFunc2",x,*data2u,0) ;
  RooHistFunc histFunc2d("histFunc2d","histFunc2",x,*data2d,0) ;

  // make interpolations
  RooRealVar alpha1("alpha1","alpha1",0, -3,3) ;
  PiecewiseInterpolation g1Syst("g1Syst","g1Syst",histFunc1, 
				RooArgSet(histFunc1d),RooArgSet(histFunc1u),RooArgSet(alpha1));
  RooRealVar alpha2("alpha2","alpha2",0, -3,3) ;
  PiecewiseInterpolation g2Syst("g2Syst","g2Syst",histFunc2, 
				RooArgSet(histFunc2d),RooArgSet(histFunc2u),RooArgSet(alpha2));

  // need to remove bin width effect from histFunc normalization
  RooConstVar binWidth("binWidth","bin width", x.numBins()/(x.getMax()-x.getMin())) ;
  RooProduct scaledSyst1("scaledSyst1","",RooArgSet(g1Syst, binWidth));
  RooProduct scaledSyst2("scaledSyst2","",RooArgSet(g2Syst, binWidth));

  // histogram normalization is sensitive to the binning.  gets factor of binwidth
  cout << "hist pdf 1 integral = " << scaledSyst1.createIntegral(x)->getVal() << endl;
  cout << "hist pdf 2 integral = " << scaledSyst2.createIntegral(x)->getVal() << endl;

  // make model out of function, using norm as expected events in extended term
  // extended term should be only thing driving alpha term
  a.setVal(1.);
  b.setVal(1.);
  alpha1.setVal(.5);
  alpha2.setVal(-.5);
  RooRealSumPdf model("model","model",RooArgSet(scaledSyst1,scaledSyst2), RooArgSet(a,b),kTRUE);
  RooDataSet* data = model.generate(x,300) ;

  model.graphVizTree("model.dot");

  cout << "model integral = " << model.createIntegral(x)->getVal() << endl;
  model.expectedEvents(x);
  cout << "expected events = " << model.expectedEvents(x) << endl;
  model.fitTo(*data);
  cout << "expected events = " << model.expectedEvents(x) << endl;

  // Plot unbinned data and histogram pdf overlaid
  RooPlot* frame1 = x.frame() ;
  data->plotOn(frame1) ;  
  model.plotOn(frame1, LineColor(kBlue)) ;  
  //  model.plotOn(frame1, LineColor(kGreen),Components(scaledSyst1)) ;  
  //  model.plotOn(frame1, LineColor(kGreen),Components(scaledSyst2)) ;  

  TCanvas* can = new TCanvas("testPieceWise","testPieceWise",800,400) ;  
  can->cd(1) ; frame1->Draw() ;


}
