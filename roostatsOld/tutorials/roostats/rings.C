#ifndef __CINT__
#include "RooGlobalFunc.h"
#endif
#include "RooStats/MCMCCalculator.h"
#include "RooStats/MCMCInterval.h"
#include "RooStats/MetropolisHastings.h"
#include "RooStats/MarkovChain.h"
#include "RooStats/ProfileLikelihoodCalculator.h"
#include "RooStats/PdfProposal.h"
#include "RooStats/ProposalHelper.h"
#include "RooStats/UniformProposal.h"
#include "RooStats/ProposalFunction.h"
#include "RooStats/ConfInterval.h"
#include "RooStats/PointSetInterval.h"
#include "RooStats/LikelihoodInterval.h"
#include "RooStats/MCMCInterval.h"
#include "RooStats/LikelihoodIntervalPlot.h"
#include "RooStats/MCMCIntervalPlot.h"
#include "RooStats/FeldmanCousins.h"
#include "RooProfileLL.h"
#include "RooAbsPdf.h"
#include "RooProdPdf.h"
#include "RooAddPdf.h"
#include "RooGaussian.h"
#include "RooRealVar.h"
#include "RooConstVar.h"
#include "RooFormulaVar.h"
#include "RooAddition.h"
#include "RooPlot.h"
#include "RooDataSet.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TStopwatch.h"
#include "TH1.h"
#include "TH2F.h"

#include "RooStats/RooStatsUtils.h"

// use this order for safety on library loading
using namespace RooFit ;
using namespace RooStats ;

void Draw(RooAbsReal* var, RooRealVar* xvar, RooRealVar* yvar,
      const char* name, const Option_t* options = "")
{
   TH2F* hist = (TH2F*)var->createHistogram(name, *xvar, RooFit::YVar(*yvar),
         RooFit::Scaling(kFALSE));
   hist->Draw(options);
}

void Draw(RooAbsData* data, RooRealVar* xvar, RooRealVar* yvar,
      const char* name, const Option_t* options = "")
{
   TH2F* hist = (TH2F*)data->createHistogram(name, *xvar, RooFit::YVar(*yvar));
   hist->Draw(options);
}

void rings()
{
   // to time the macro
   TStopwatch t;
   t.Start();

   RooRealVar x("x", "x", 0, -4, 8);
   RooRealVar y("y", "y", 0, -6, 6);
   RooArgSet obs(x,y);

   RooRealVar c11("c11", "c11", -2, -4, 8);
   RooRealVar c12("c12", "c12",  0, -6, 6);
   RooRealVar r1("r1", "r1",  1, 0, 10);
   RooRealVar w1("w1", "w1", .1, 0, 10);
   RooFormulaVar v1("v1", "sqrt((@0-@1)*(@0-@1)+(@2-@3)*(@2-@3))", RooArgList(x, c11, y, c12));
   RooGaussian g1 ("gauss1", "gauss1", v1, r1, w1);

   RooRealVar c21("c21", "c21",  4, -4, 8);
   RooRealVar c22("c22", "c22",  0, -6, 6);
   RooRealVar r2("r2", "r2",  2, 0, 10);
   RooRealVar w2("w2", "w2", .1, 0, 10);
   RooFormulaVar v2("v2", "sqrt((@0-@1)*(@0-@1)+(@2-@3)*(@2-@3))", RooArgList(x, c21, y, c22));
   RooGaussian g2("gauss2", "gauss2", v2, r2, w2);

   RooAddPdf pdf("rings", "Rings", g1, g2, RooConst(0.5));

   TCanvas* control = new TCanvas("control");
   Draw(&pdf, &x, &y, "Control", "");
   control->Update();

   RooArgList axes(x, y);

   // construct a UniformProposal
   UniformProposal up;

   // construct a PdfProposal
   RooRealVar mupgx("mupgx", "", 0, -4, 8);
   RooRealVar mupgy("mupgy", "", 0, -6, 6);
   RooRealVar sigmapgx("sigmapgx", "", 3, 0, 10);
   RooRealVar sigmapgy("sigmapgy", "", 3, 0, 10);
   RooGaussian pgx("pgx", "Proposal Gaussian x", x, mupgx, sigmapgx);
   RooGaussian pgy("pgy", "Proposal Gaussian y", y, mupgy, sigmapgy);
   RooProdPdf propProduct("propProduct", "propProduct", RooArgSet(pgx, pgy));
   PdfProposal pdfProposal(propProduct);
   pdfProposal.AddMapping(mupgx, x);
   pdfProposal.AddMapping(mupgy, y);
   pdfProposal.SetCacheSize(100);

   RooDataSet clueBank("clueBank", "Bank of Clues", obs);

   x.setVal(5);
   y.setVal(1.732);
   clueBank.add(obs);

   x.setVal(4);
   y.setVal(2);
   clueBank.add(obs);

   x.setVal(4);
   y.setVal(-2);
   clueBank.add(obs);

   x.setVal(-1);
   y.setVal(0);
   clueBank.add(obs);

   x.setVal(-2);
   y.setVal(-1);
   clueBank.add(obs);

   ProposalHelper ph;
   ph.SetVariables(axes);
   //ph.SetClues(clueBank);
   //ph.SetCluesFraction(.20);
   //ph.SetUniformFraction(.10);
   //ph.SetCluesOptions("");
   ph.SetUpdateProposalParameters(kTRUE);
   ph.SetCacheSize(100);
   ph.SetWidthRangeDivisor(5);
   ProposalFunction* pdfProp = ph.GetProposalFunction();

   ProposalHelper ph2;
   ph2.SetVariables(axes);
   ph2.SetClues(clueBank);
   ph2.SetCluesFraction(.20);
   //ph2.SetUniformFraction(.10);
   ph2.SetCluesOptions("");
   ph2.SetUpdateProposalParameters(kTRUE);
   ph2.SetCacheSize(100);
   ph2.SetWidthRangeDivisor(5);
   ProposalFunction* clueBankProp = ph2.GetProposalFunction();

   //TCanvas* propFuncCanvas = new TCanvas("propFuncCanvas");
   //TH2F* propFuncHist = (TH2F*)((PdfProposal*)pdfProp)->GetPdf()->createHistogram(
   //      "propFuncHist", x, RooFit::YVar(y), RooFit::Scaling(kFALSE));
   //propFuncHist->SetTitle("Clues MVG Proposal Function");
   //propFuncHist->Draw("lego");
   //propFuncCanvas->Update();

   //Int_t k = 0;
   //printf("> ");
   //scanf("%d", &k);
   //if (!k) return;

   // make our MetropolisHastings object
   MetropolisHastings mh;
   mh.SetFunction(pdf);
   mh.SetType(MetropolisHastings::kRegular);
   mh.SetSign(MetropolisHastings::kPositive);
   mh.SetParameters(obs);

   // construct the markov chains
   Int_t iters[] = {1000, 10000, 1000000};
   Int_t size = 3;
   char title[64];

   Int_t burnInSteps = 20;
   MCMCInterval interval("interval", "MCMCInterval");
   interval.SetParameters(obs);
   interval.SetAxes(axes);
   interval.SetNumBurnInSteps(burnInSteps);
   MCMCIntervalPlot plot(interval);

   TCanvas* ringCanvas = new TCanvas("ringCanvas", "", 800, 800);
   ringCanvas->Divide(3,3);
   TCanvas* walkCanvas = new TCanvas("walkCanvas", "", 800, 800);
   walkCanvas->Divide(3,3);

   for (Int_t i = 0; i < size; i++) {
      mh.SetNumIters(iters[i]);
      printf("%d iters\n", iters[i]);

      // Uniform proposal
      mh.SetProposalFunction(up);
      printf("Constructing chain with up\n");
      MarkovChain* upChain = mh.ConstructChain();
      interval.SetChain(*upChain);

      RooDataSet* upData = upChain->GetAsDataSet(&obs);
      ringCanvas->cd(3*i + 1);
      sprintf(title, "up: %d", iters[i]);
      Draw(upData, &x, &y, title, "");
      ringCanvas->Update();
      walkCanvas->cd(3*i + 1);
      plot.DrawChainScatter(x, y);
      walkCanvas->Update();

      // Pdf Proposal
      ((PdfProposal*)pdfProp)->Reset();
      mh.SetProposalFunction(*pdfProp);
      printf("Constructing chain with pdfProp\n");
      MarkovChain* pdfChain = mh.ConstructChain();
      interval.SetChain(*pdfChain);

      RooDataSet* pdfData = pdfChain->GetAsDataSet(&obs);
      ringCanvas->cd(3*i + 2);
      sprintf(title, "pdfProp: %d", iters[i]);
      Draw(pdfData, &x, &y, title, "");
      ringCanvas->Update();
      walkCanvas->cd(3*i + 2);
      plot.DrawChainScatter(x, y);
      walkCanvas->Update();

      // Bank of clues Pdf Proposal
      ((PdfProposal*)clueBankProp)->Reset();
      mh.SetProposalFunction(*clueBankProp);
      printf("Constructing chain with clueBankProp\n");
      MarkovChain* clueBankChain = mh.ConstructChain();
      interval.SetChain(*clueBankChain);

      RooDataSet* clueBankData = clueBankChain->GetAsDataSet(&obs);
      ringCanvas->cd(3*i + 3);
      sprintf(title, "clueBankProp: %d", iters[i]);
      Draw(clueBankData, &x, &y, title, "");
      ringCanvas->Update();
      walkCanvas->cd(3*i + 3);
      plot.DrawChainScatter(x, y);
      walkCanvas->Update();
   }

   /// print timing info
   t.Stop();
   t.Print();
}
