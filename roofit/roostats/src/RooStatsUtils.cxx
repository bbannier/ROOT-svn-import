// @(#)root/roostats:$Id$
// Author: Kyle Cranmer   28/07/2008

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/////////////////////////////////////////
// RooStats
//
// namespace for classes and functions of the RooStats package
/////////////////////////////////////////
#include "Rtypes.h"

#if !defined(R__ALPHA) && !defined(R__SOLARIS) && !defined(R__ACC) && !defined(R__FBSD)
NamespaceImp(RooStats)
#endif

#include "TTree.h"

#include "RooUniform.h"
#include "RooProdPdf.h"
#include "RooExtendPdf.h"
#include "RooSimultaneous.h"
#include "RooStats/ModelConfig.h"
#include "RooStats/RooStatsUtils.h"
#include <typeinfo>

using namespace std;

// this file is only for the documentation of RooStats namespace

namespace RooStats { 

   void FactorizePdf(const RooArgSet &observables, RooAbsPdf &pdf, RooArgList &obsTerms, RooArgList &constraints) {
   // utility function to factorize constraint terms from a pdf 
   // (from G. Petrucciani)
      const std::type_info & id = typeid(pdf);
      if (id == typeid(RooProdPdf)) {
         RooProdPdf *prod = dynamic_cast<RooProdPdf *>(&pdf);
         RooArgList list(prod->pdfList());
         for (int i = 0, n = list.getSize(); i < n; ++i) {
            RooAbsPdf *pdfi = (RooAbsPdf *) list.at(i);
            FactorizePdf(observables, *pdfi, obsTerms, constraints);
         }
      } else if (id == typeid(RooExtendPdf)) {
         TIterator *iter = pdf.serverIterator(); 
         // extract underlying pdf which is extended; first server is the pdf; second server is the number of events variable
         RooAbsPdf *updf = dynamic_cast<RooAbsPdf *>(iter->Next());
         assert(updf != 0);
         delete iter;
         FactorizePdf(observables, *updf, obsTerms, constraints);
      } else if (id == typeid(RooSimultaneous)) {    //|| id == typeid(RooSimultaneousOpt)) {
         RooSimultaneous *sim  = dynamic_cast<RooSimultaneous *>(&pdf);
         assert(sim != 0);
         RooAbsCategoryLValue *cat = (RooAbsCategoryLValue *) sim->indexCat().clone(sim->indexCat().GetName());
         for (int ic = 0, nc = cat->numBins((const char *)0); ic < nc; ++ic) {
            cat->setBin(ic);
            RooAbsPdf* catPdf = sim->getPdf(cat->getLabel());
            // it is possible that a pdf is not defined for every category
            if (catPdf != 0) FactorizePdf(observables, *catPdf, obsTerms, constraints);
         }
         delete cat;
      } else if (pdf.dependsOn(observables)) {
         if (!obsTerms.contains(pdf)) obsTerms.add(pdf);
      } else {
         if (!constraints.contains(pdf)) constraints.add(pdf);
      }
   }


   void FactorizePdf(RooStats::ModelConfig &model, RooAbsPdf &pdf, RooArgList &obsTerms, RooArgList &constraints) {
      // utility function to factorize constraint terms from a pdf 
      // (from G. Petrucciani)
      if (!model.GetObservables() ) { 
         oocoutE((TObject*)0,InputArguments) << "RooStatsUtils::FactorizePdf - invalid input model: missing observables" << endl;
         return;
      }
      return FactorizePdf(*model.GetObservables(), pdf, obsTerms, constraints);
   }


   RooAbsPdf * MakeNuisancePdf(RooAbsPdf &pdf, const RooArgSet &observables, const char *name) { 
      // make a nuisance pdf by factorizing out all constraint terms in a common pdf 
      RooArgList obsTerms, constraints;
      FactorizePdf(observables, pdf, obsTerms, constraints);
      if(constraints.getSize() == 0) {
         oocoutW((TObject *)0, Eval) << "RooStatsUtils::MakeNuisancePdf - no constraints found on nuisance parameters in the input model" << endl;
         return 0;
      } else if(constraints.getSize() == 1) {
         return dynamic_cast<RooAbsPdf *>(constraints.first()->clone(name));
      }
      return new RooProdPdf(name,"", constraints);
   }

   RooAbsPdf * MakeNuisancePdf(const RooStats::ModelConfig &model, const char *name) { 
      // make a nuisance pdf by factorizing out all constraint terms in a common pdf
      if (!model.GetPdf() || !model.GetObservables() ) { 
         oocoutE((TObject*)0, InputArguments) << "RooStatsUtils::MakeNuisancePdf - invalid input model: missing pdf and/or observables" << endl;
         return 0;
      }
      return MakeNuisancePdf(*model.GetPdf(), *model.GetObservables(), name);
   }

   RooAbsPdf * StripConstraints(RooAbsPdf &pdf, const RooArgSet &observables) { 
      const std::type_info & id = typeid(pdf);

      if (id == typeid(RooProdPdf)) {

         RooProdPdf *prod = dynamic_cast<RooProdPdf *>(&pdf);
         RooArgList list(prod->pdfList()); RooArgList newList;

         for (int i = 0, n = list.getSize(); i < n; ++i) {
            RooAbsPdf *pdfi = (RooAbsPdf *) list.at(i);
            RooAbsPdf *newPdfi = StripConstraints(*pdfi, observables);
            if(newPdfi != NULL) newList.add(*newPdfi);
         }

         if(newList.getSize() == 0) return NULL; // only constraints in product
         // return single component (no longer a product)
         else if(newList.getSize() == 1) return dynamic_cast<RooAbsPdf *>(newList.at(0)->clone(TString::Format("%s_unconstrained", 
                                                                                                               newList.at(0)->GetName()))); 
         else return new RooProdPdf(TString::Format("%s_unconstrained", prod->GetName()).Data(),
            TString::Format("%s without constraints", prod->GetTitle()).Data(), newList);

      } else if (id == typeid(RooExtendPdf)) {

         TIterator *iter = pdf.serverIterator(); 
         // extract underlying pdf which is extended; first server is the pdf; second server is the number of events variable
         RooAbsPdf *uPdf = dynamic_cast<RooAbsPdf *>(iter->Next());
         RooAbsReal *extended_term = dynamic_cast<RooAbsReal *>(iter->Next());
         assert(uPdf != NULL); assert(extended_term != NULL); assert(iter->Next() == NULL);
         delete iter;
         
         RooAbsPdf *newUPdf = StripConstraints(*uPdf, observables);
         if(newUPdf == NULL) return NULL; // only constraints in underlying pdf
         else return new RooExtendPdf(TString::Format("%s_unconstrained", pdf.GetName()).Data(),
            TString::Format("%s without constraints", pdf.GetTitle()).Data(), *newUPdf, *extended_term);
         
      } else if (id == typeid(RooSimultaneous)) {    //|| id == typeid(RooSimultaneousOpt)) {

         RooSimultaneous *sim  = dynamic_cast<RooSimultaneous *>(&pdf); assert(sim != NULL);
         RooAbsCategoryLValue *cat = (RooAbsCategoryLValue *) sim->indexCat().Clone(); assert(cat != NULL);
         RooArgList pdfList;

         for (int ic = 0, nc = cat->numBins((const char *)NULL); ic < nc; ++ic) {
            cat->setBin(ic);
            RooAbsPdf* catPdf = sim->getPdf(cat->getLabel());
            RooAbsPdf* newPdf = NULL;
            // it is possible that a pdf is not defined for every category
            if (catPdf != NULL) newPdf = StripConstraints(*catPdf, observables);
            if (newPdf == NULL) { delete cat; return NULL; } // all channels must have observables
            pdfList.add(*newPdf);
         }

         return new RooSimultaneous(TString::Format("%s_unconstrained", sim->GetName()).Data(), 
            TString::Format("%s without constraints", sim->GetTitle()).Data(), pdfList, *cat); 

      } else if (pdf.dependsOn(observables)) {  
         return (RooAbsPdf *) pdf.clone(TString::Format("%s_unconstrained", pdf.GetName()).Data());
      }

      return NULL; // just  a constraint term
   }

   RooAbsPdf * MakeUnconstrainedPdf(RooAbsPdf &pdf, const RooArgSet &observables, const char *name) { 
      // make a clone pdf without all constraint terms in a common pdf
      RooAbsPdf * unconstrainedPdf = StripConstraints(pdf, observables);
      if(!unconstrainedPdf) {
         oocoutE((TObject *)NULL, InputArguments) << "RooStats::MakeUnconstrainedPdf - invalid observable list passed (observables not found in original pdf) or invalid pdf passed (without observables)" << endl;
         return NULL;
      }
      if(name != NULL) unconstrainedPdf->SetName(name);
      return unconstrainedPdf;   
   }

   RooAbsPdf * MakeUnconstrainedPdf(const RooStats::ModelConfig &model, const char *name) { 
      // make a clone pdf without all constraint terms in a common pdf 
      if(!model.GetPdf() || !model.GetObservables()) {
         oocoutE((TObject *)NULL, InputArguments) << "RooStatsUtils::MakeUnconstrainedPdf - invalid input model: missing pdf and/or observables" << endl;
         return NULL;
      }
      return MakeUnconstrainedPdf(*model.GetPdf(), *model.GetObservables(), name);
   }

   // Helper class for GetAsTTree
   class BranchStore {
      public:
         std::map<TString, Double_t> varVals;
         double inval;

         BranchStore(const vector <TString> &params = vector <TString>(), double _inval = -999.) {
            inval = _inval;
            for(unsigned int i = 0;i<params.size();i++)
               varVals[params[i]] = _inval;
         }

         void AssignToTTree(TTree &myTree) {
            for(std::map<TString, Double_t>::iterator it = varVals.begin();it!=varVals.end();it++) {
               const TString& name = it->first;
               myTree.Branch( name, &varVals[name], TString::Format("%s/D", name.Data()));
            }
         }
         void ResetValues() {
            for(std::map<TString, Double_t>::iterator it = varVals.begin();it!=varVals.end();it++) {
               const TString& name = it->first;
               varVals[name] = inval;
            }
         }
   };

   BranchStore* CreateBranchStore(const RooDataSet& data) {
      if (data.numEntries() == 0) {
         return new BranchStore;
      }
      vector <TString> V;
      const RooArgSet* aset = data.get(0);
      RooAbsArg *arg(0);
      TIterator *it = aset->createIterator();
      for(;(arg = dynamic_cast<RooAbsArg*>(it->Next()));) {
         RooRealVar *rvar = dynamic_cast<RooRealVar*>(arg);
         if (rvar == NULL)
            continue;
         V.push_back(rvar->GetName());
         if (rvar->hasAsymError()) {
            V.push_back(TString::Format("%s_errlo", rvar->GetName()));
            V.push_back(TString::Format("%s_errhi", rvar->GetName()));
         }
         else if (rvar->hasError()) {
            V.push_back(TString::Format("%s_err", rvar->GetName()));
         }
      }
      delete it;
      return new BranchStore(V);
   }

   void FillTree(TTree &myTree, const RooDataSet &data) {
      BranchStore *bs = CreateBranchStore(data);
      bs->AssignToTTree(myTree);

      for(int entry = 0;entry<data.numEntries();entry++) {
         bs->ResetValues();
         const RooArgSet* aset = data.get(entry);
         RooAbsArg *arg(0);
         RooLinkedListIter it = aset->iterator();
         for(;(arg = dynamic_cast<RooAbsArg*>(it.Next()));) {
            RooRealVar *rvar = dynamic_cast<RooRealVar*>(arg);
            if (rvar == NULL)
               continue;
            bs->varVals[rvar->GetName()] = rvar->getValV();
            if (rvar->hasAsymError()) {
               bs->varVals[TString::Format("%s_errlo", rvar->GetName())] = rvar->getAsymErrorLo();
               bs->varVals[TString::Format("%s_errhi", rvar->GetName())] = rvar->getAsymErrorHi();
            }
            else if (rvar->hasError()) {
               bs->varVals[TString::Format("%s_err", rvar->GetName())] = rvar->getError();
            }
         }
         myTree.Fill();
      }
      delete bs;
   }

   TTree * GetAsTTree(TString name, TString desc, const RooDataSet& data) {
      TTree* myTree = new TTree(name, desc);
      FillTree(*myTree, data);
      return myTree;
   }
   
   TH1* ProfileMinOntoX( TH2& h2, bool subtractMin ) {
      // create a 1D histogram with proper name and title
      TString profileName( h2.GetName() );
      profileName += "_profileOntoX";
      TString profileTitle( h2.GetTitle() );
      profileTitle += " profile onto x-axis";
      TH1* h1 = new TH1D( profileName, profileTitle, h2.GetNbinsX(), h2.GetXaxis()->GetXmin(), h2.GetXaxis()->GetXmax() );
      
      // initialize to the maximum of the 2D hist
      for( int x=0; x < h1->GetNbinsX()+2; x++ ) h1->SetBinContent( x, h2.GetMaximum() );
      
      // do the filling and profiling
      for( int x=0; x < h2.GetNbinsX()+2; x++ ) {
         for( int y=0; y < h2.GetNbinsY()+2; y++ ) {
            int binNumber2D = x + y*(h2.GetNbinsX()+2);
            
            // profiling
            if( h1->GetBinContent(x) > h2.GetBinContent(binNumber2D) ) {
               h1->SetBinContent( x, h2.GetBinContent(binNumber2D) );
            }
         }
      }
      
      if( subtractMin ) {
         for( int x=0; x < h1->GetNbinsX()+2; x++ ) {
            h1->SetBinContent( x, h1->GetBinContent(x) - h1->GetMinimum() );
         }
      }
      
      return h1;
   }
   TH1* ProfileMinOntoY( TH2& h2, bool subtractMin ) {
      // create a 1D histogram with proper name and title
      TString profileName( h2.GetName() );
      profileName += "_profileOntoY";
      TString profileTitle( h2.GetTitle() );
      profileTitle += " profile onto y-axis";
      TH1* h1 = new TH1D( profileName, profileTitle, h2.GetNbinsY(), h2.GetYaxis()->GetXmin(), h2.GetYaxis()->GetXmax() );
      
      // initialize to the maximum of the 2D hist
      for( int x=0; x < h1->GetNbinsX()+2; x++ ) h1->SetBinContent( x, h2.GetMaximum() );
      
      // do the filling and profiling
      for( int y=0; y < h2.GetNbinsY()+2; y++ ) {
         for( int x=0; x < h2.GetNbinsX()+2; x++ ) {
            int binNumber2D = x + y*(h2.GetNbinsX()+2);
            
            // profiling
            if( h1->GetBinContent(y) > h2.GetBinContent(binNumber2D) ) {
               h1->SetBinContent( y, h2.GetBinContent(binNumber2D) );
            }
         }
      }
      
      if( subtractMin ) {
         for( int x=0; x < h1->GetNbinsX()+2; x++ ) {
            h1->SetBinContent( x, h1->GetBinContent(x) - h1->GetMinimum() );
         }
      }
      
      return h1;
   }


   double ContourLevelHPD( TH1* h, double integralValue ) {
      int numBins = h->GetNbinsX()+2;
      if( h->GetNbinsY() > 1 ) numBins *= h->GetNbinsY()+2;
      if( h->GetNbinsZ() > 1 ) numBins *= h->GetNbinsZ()+2;
   
      std::vector<double> bins;
      for( int i=0; i < numBins; i++ ) bins.push_back( h->GetBinContent(i) ); 
      std::sort( bins.begin(), bins.end(), std::greater<double>() );  // reverse sort using std::greater<>()

      double integral = h->Integral();   
      double cumulative = 0.0;
      for( std::vector<double>::iterator b=bins.begin(); b != bins.end(); b++ ) {
         cumulative += (*b)/integral;
         if( cumulative >= integralValue ) return *b;
      }
   
      return 0.0;
   }

   void HistMin( TH1* h1, TH1* h2 ) {
      int numBins1 = h1->GetNbinsX()+2;
      if( h1->GetNbinsY() > 1 ) numBins1 *= h1->GetNbinsY()+2;
      if( h1->GetNbinsZ() > 1 ) numBins1 *= h1->GetNbinsZ()+2;
      int numBins2 = h2->GetNbinsX()+2;
      if( h2->GetNbinsY() > 1 ) numBins2 *= h2->GetNbinsY()+2;
      if( h2->GetNbinsZ() > 1 ) numBins2 *= h2->GetNbinsZ()+2;
   
      if( numBins2 != numBins1 ) {
         std::cout << "ERROR HistMin(): histograms need to have the same dimensions." << std::endl; 
         return;
      }
   
      // Assume maximum in each histogram corresponds to unset bins.
      // Therefore, raise max to the max of both histograms.
      if( h1->GetMaximum() > h2->GetMaximum() ) {
         double h2OldMax = h2->GetMaximum();
         for( int i=0; i < numBins2; i++ ) {
            if( h2->GetBinContent(i) == h2OldMax ) h2->SetBinContent( i, h1->GetMaximum() );
         }
      }else{
         double h1OldMax = h1->GetMaximum();
         for( int i=0; i < numBins2; i++ ) {
            if( h1->GetBinContent(i) == h1OldMax ) h1->SetBinContent( i, h2->GetMaximum() );
         }
      }
   
      for( int i=0; i < numBins1; i++ ) {
         if( h2->GetBinContent(i) < h1->GetBinContent(i) ) {
            h1->SetBinContent( i, h2->GetBinContent(i) );
         }
      }
   }

   TH1D* RebinHist1DMin( TH1* h, int rebin ) {
      TH1D* hRebinned = new TH1D( 
         h->GetName(), h->GetTitle(),
         h->GetNbinsX()/rebin, h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax()
      );
   
      // nothing is smaller than min, so use min-1.0 as place holder for empty
      double minOrig = h->GetMinimum();
      for( int i=0; i < hRebinned->GetNbinsX()+2; i++ ) {
         hRebinned->SetBinContent( i, minOrig-1.0 );
      }

      for( int x=0; x < h->GetNbinsX(); x++ ) {
         //int xRebinned = floor(x/rebin);
         int bin = x+1;
         int binRebinned = hRebinned->FindBin( h->GetBinCenter(bin) ); //xRebinned+1;
         if( h->GetBinContent(bin) < hRebinned->GetBinContent(binRebinned) ||
             hRebinned->GetBinContent(binRebinned) == minOrig-1.0
         ) {
            hRebinned->SetBinContent( binRebinned, h->GetBinContent(bin) );
         }
      }

      for( int i=0; i < hRebinned->GetNbinsX()+2; i++ ) {
         if( hRebinned->GetBinContent(i) == minOrig-1.0 ) {
            hRebinned->SetBinContent( i, hRebinned->GetMaximum() );
         }
      }

      return hRebinned;   
   }

   TH2D* RebinHist2DMin( TH2* h, int rebin ) {
      TH2D* hRebinned = new TH2D( 
         TString(h->GetName())+"_rebinned", h->GetTitle(),
         h->GetNbinsX()/rebin, h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax(),
         h->GetNbinsY()/rebin, h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax()
      );
   
      // nothing is smaller than min, so use min-1.0 as place holder for empty
      double minOrig = h->GetMinimum();
      for( int i=0; i < (hRebinned->GetNbinsX()+2)*(hRebinned->GetNbinsY()+2); i++ ) {
         hRebinned->SetBinContent( i, minOrig-1.0 );
      }

      for( int x=0; x < h->GetNbinsX(); x++ ) {
         for( int y=0; y < h->GetNbinsY(); y++ ) {
            int xRebinned = x/rebin;
            int yRebinned = y/rebin;
            int bin = (y+1)*(h->GetNbinsY()+2) + (x+1);
            int binRebinned = (yRebinned+1)*(hRebinned->GetNbinsY()+2) + (xRebinned+1);
            if( h->GetBinContent(bin) < hRebinned->GetBinContent(binRebinned) ||
                hRebinned->GetBinContent(binRebinned) == minOrig-1.0
            ) {
               hRebinned->SetBinContent( binRebinned, h->GetBinContent(bin) );
            }
         }
      }

      for( int i=0; i < (hRebinned->GetNbinsX()+2)*(hRebinned->GetNbinsY()+2); i++ ) {
         if( hRebinned->GetBinContent(i) == minOrig-1.0 ) {
            hRebinned->SetBinContent( i, hRebinned->GetMaximum() );
         }
      }

      return hRebinned;   
   }


   TH1* MaxLFromNLLHist( TH1* nllHist ) {
      TString maxLName( "maxLHist2D_" );
      maxLName += nllHist->GetName();
   
      TH1* maxLHist = (TH1*)nllHist->Clone( maxLName );
   
      maxLHist->SetTitle( "Maximum Likelihood per Bin (subtracted)" );
      if( maxLHist->GetDimension() == 1 )
         maxLHist->GetYaxis()->SetTitle( "Maximum Likelihood per Bin (subtracted)" );
      else if( maxLHist->GetDimension() == 2 )
         maxLHist->GetZaxis()->SetTitle( "Maximum Likelihood per Bin (subtracted)" );
      else
         cout << "WARNING: not sure what to do with this histogram." << endl;

      int numBins = maxLHist->GetNbinsX()+2;
      if( maxLHist->GetDimension() >= 2 ) numBins *= maxLHist->GetNbinsY()+2;
      if( maxLHist->GetDimension() >= 3 ) numBins *= maxLHist->GetNbinsZ()+2;
      double minNLL = maxLHist->GetMinimum();
      for( int i=0; i < numBins; i++ ) {
         double newVal = exp(- (maxLHist->GetBinContent(i)-minNLL));
         //cout << "nll = " << h->GetBinContent(i) << "   L = " << newVal << endl;
         maxLHist->SetBinContent( i, newVal );
      }

      return maxLHist;
   }
   
   
}
