
/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate data analysis       *
 * Package: TMVA                                                                  *
 * Classes: PDEFoamDecisionTree                                                   *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      Implementation of decision tree like PDEFoam                              *
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      S. Jadach        - Institute of Nuclear Physics, Cracow, Poland           *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *      Peter Speckmayer - CERN, Switzerland                                      *
 *                                                                                *
 * Copyright (c) 2008, 2010:                                                            *
 *      CERN, Switzerland                                                         *
 *      MPI-K Heidelberg, Germany                                                 *
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

//_____________________________________________________________________
//
// PDEFoamDecisionTree
//
// This PDEFoam variant acts like a decision tree and stores in every
// cell the discriminant
//
//    D = #events with given class / total number of events
//
// as well as the statistical error on the discriminant.  It therefore
// acts as a discriminant estimator.  The decision tree-like behaviour
// is achieved by overriding PDEFoamDiscriminant::Explore() to use a
// decision tree-like cell splitting algorithm (given a separation
// type).
//
// This PDEFoam variant should be booked together with the
// PDEFoamDTDensity density estimator, which returns the events in a
// cell without sampling.
//
//_____________________________________________________________________

#ifndef ROOT_TMath
#include "TMath.h"
#endif

#ifndef ROOT_TMVA_PDEFoamDecisionTree
#include "TMVA/PDEFoamDecisionTree.h"
#endif
#ifndef ROOT_TMVA_PDEFoamDTDensity
#include "TMVA/PDEFoamDTDensity.h"
#endif

ClassImp(TMVA::PDEFoamDecisionTree)

//_____________________________________________________________________
TMVA::PDEFoamDecisionTree::PDEFoamDecisionTree() 
   : PDEFoamDiscriminant()
   , fDTSeparation(kFoam)
{
   // Default constructor for streamer, user should not use it.
}

//_____________________________________________________________________
TMVA::PDEFoamDecisionTree::PDEFoamDecisionTree(const TString& Name, UInt_t cls, EDTSeparation sep)
   : PDEFoamDiscriminant(Name, cls)
   , fDTSeparation(sep)
{}

//_____________________________________________________________________
TMVA::PDEFoamDecisionTree::PDEFoamDecisionTree(const PDEFoamDecisionTree &From)
   : PDEFoamDiscriminant(From)
   , fDTSeparation(kFoam)
{
   // Copy Constructor  NOT IMPLEMENTED (NEVER USED)
   Log() << kFATAL << "COPY CONSTRUCTOR NOT IMPLEMENTED" << Endl;
}

//_____________________________________________________________________
TMVA::PDEFoamDecisionTree::~PDEFoamDecisionTree()
{}

//_____________________________________________________________________
void TMVA::PDEFoamDecisionTree::Explore(PDEFoamCell *cell)
{
   // Internal subprogram used by Create.  It explores newly defined
   // cell with according to the decision tree logic.  The separation
   // set by the 'fDTSeparation' option is used (see also
   // GetSeparation()).
   //
   // The optimal division point for eventual future cell division is
   // determined/recorded.  Note that links to parents and initial
   // volume = 1/2 parent has to be already defined prior to calling
   // this routine.
   //
   // Note, that according to the decision tree logic, a cell is only
   // split, if the number of (unweighted) events in each dautghter
   // cell is greater than fNmin.

   if (!cell)
      Log() << kFATAL << "<DTExplore> Null pointer given!" << Endl;

   // create edge histograms
   std::vector<TH1F*> hsig, hbkg, hsig_unw, hbkg_unw;
   for (Int_t idim=0; idim<fDim; idim++) {
      hsig.push_back( new TH1F(Form("hsig_%i",idim), 
                               Form("signal[%i]",idim), fNBin, 0, 1 ));
      hbkg.push_back( new TH1F(Form("hbkg_%i",idim), 
                               Form("background[%i]",idim), fNBin, 0, 1 ));
      hsig_unw.push_back( new TH1F(Form("hsig_unw_%i",idim), 
                                   Form("signal_unw[%i]",idim), fNBin, 0, 1 ));
      hbkg_unw.push_back( new TH1F(Form("hbkg_unw_%i",idim), 
                                   Form("background_unw[%i]",idim), fNBin, 0, 1 ));
   }

   // Fill histograms
   PDEFoamDTDensity *distr = dynamic_cast<PDEFoamDTDensity*>(fDistr);
   if (distr == NULL)
      Log() << kFATAL << "<PDEFoamDecisionTree::Explore>: cast failed: PDEFoamDensity* --> PDEFoamDTDensity*" << Endl;
   distr->FillHist(cell, hsig, hbkg, hsig_unw, hbkg_unw);

   // ------ determine the best division edge
   Float_t xBest = 0.5;   // best division point
   Int_t   kBest = -1;    // best split dimension
   Float_t maxGain = -1.0; // maximum gain
   Float_t nTotS = hsig.at(0)->Integral(0, hsig.at(0)->GetNbinsX()+1);
   Float_t nTotB = hbkg.at(0)->Integral(0, hbkg.at(0)->GetNbinsX()+1);
   Float_t nTotS_unw = hsig_unw.at(0)->Integral(0, hsig_unw.at(0)->GetNbinsX()+1);
   Float_t nTotB_unw = hbkg_unw.at(0)->Integral(0, hbkg_unw.at(0)->GetNbinsX()+1);
   Float_t parentGain = (nTotS+nTotB) * GetSeparation(nTotS,nTotB);

   for (Int_t idim=0; idim<fDim; idim++) {
      Float_t nSelS=hsig.at(idim)->GetBinContent(0);
      Float_t nSelB=hbkg.at(idim)->GetBinContent(0);
      Float_t nSelS_unw=hsig_unw.at(idim)->GetBinContent(0);
      Float_t nSelB_unw=hbkg_unw.at(idim)->GetBinContent(0);
      for(Int_t jLo=1; jLo<fNBin; jLo++) {
         nSelS += hsig.at(idim)->GetBinContent(jLo);
         nSelB += hbkg.at(idim)->GetBinContent(jLo);
         nSelS_unw += hsig_unw.at(idim)->GetBinContent(jLo);
         nSelB_unw += hbkg_unw.at(idim)->GetBinContent(jLo);

         // proceed if total number of events in left and right cell
         // is greater than fNmin
         if ( !( (nSelS_unw + nSelB_unw) >= GetNmin() && 
                 (nTotS_unw-nSelS_unw + nTotB_unw-nSelB_unw) >= GetNmin() ) )
            continue;

         Float_t xLo = 1.0*jLo/fNBin;

         // calculate gain
         Float_t leftGain   = ((nTotS - nSelS) + (nTotB - nSelB))
            * GetSeparation(nTotS-nSelS,nTotB-nSelB);
         Float_t rightGain  = (nSelS+nSelB) * GetSeparation(nSelS,nSelB);
         Float_t gain = parentGain - leftGain - rightGain;

         if (gain >= maxGain) {
            maxGain = gain;
            xBest   = xLo;
            kBest   = idim;
         }
      } // jLo
   } // idim

   if (kBest >= fDim || kBest < 0) {
      // No best division edge found!  One must ensure, that this cell
      // is not chosen for splitting in PeekMax().  But since in
      // PeekMax() it is ensured that cell->GetDriv() > epsilon, one
      // should set maxGain to -1.0 (or even 0.0?) here.
      maxGain = -1.0;
   }
   
   // set cell properties
   cell->SetBest(kBest);
   cell->SetXdiv(xBest);
   if (nTotB+nTotS > 0)
      cell->SetIntg( nTotS/(nTotB+nTotS) );
   else 
      cell->SetIntg( 0.0 );
   cell->SetDriv(maxGain);
   cell->CalcVolume();

   // set cell element 0 (total number of events in cell) during
   // build-up
   if (GetNmin() > 0)
      SetCellElement( cell, 0, nTotS + nTotB);

   // clean up
   for (UInt_t ih=0; ih<hsig.size(); ih++)  delete hsig.at(ih);
   for (UInt_t ih=0; ih<hbkg.size(); ih++)  delete hbkg.at(ih);
   for (UInt_t ih=0; ih<hsig_unw.size(); ih++)  delete hsig_unw.at(ih);
   for (UInt_t ih=0; ih<hbkg_unw.size(); ih++)  delete hbkg_unw.at(ih);
}

//_____________________________________________________________________
Float_t TMVA::PDEFoamDecisionTree::GetSeparation(Float_t s, Float_t b)
{
   // Calculate the separation depending on 'fDTSeparation' for the
   // given number of signal and background events 's', 'b'.  Note,
   // that if (s+b) < 0 or s < 0 or b < 0 than the return value is 0.

   if (s+b <= 0 || s < 0 || b < 0 )
      return 0;

   Float_t p = s/(s+b);
   
   switch(fDTSeparation) {
   case kFoam:                   // p
      return p;
   case kGiniIndex:              // p * (1-p)
      return p*(1-p);
   case kMisClassificationError: // 1 - max(p,1-p)
      return 1 - TMath::Max(p, 1-p);
   case kCrossEntropy: // -p*log(p) - (1-p)*log(1-p)
      return (p<=0 || p >=1 ? 0 : -p*TMath::Log(p) - (1-p)*TMath::Log(1-p));
   default:
      Log() << kFATAL << "Unknown separation type" << Endl;
      break;
   }

   return 0;
}
