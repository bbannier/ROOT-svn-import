// @(#)root/tmva $Id$
// Author: Tancredi Carli, Dominik Dannheim, Alexander Voigt

/**********************************************************************************
 * Project: TMVA - a Root-integrated toolkit for multivariate Data analysis       *
 * Package: TMVA                                                                  *
 * Class  : MethodBPDEFoam                                                        *
 * Web    : http://tmva.sourceforge.net                                           *
 *                                                                                *
 * Description:                                                                   *
 *      The PDEFoam method is an extension of the PDERS method, which divides     *
 *      the multi-dimensional phase space in a finite number of hyper-rectangles  *
 *      (cells) of constant event density. This "foam" of cells is filled with    *
 *      averaged probability-density information sampled from a training event    *
 *      sample.    
 *                                                                                *
 * Authors (alphabetical):                                                        *
 *      Tancredi Carli   - CERN, Switzerland                                      *
 *      Dominik Dannheim - CERN, Switzerland                                      *
 *      Peter Speckmayer <peter.speckmayer@cern.ch>  - CERN, Switzerland          *
 *      Alexander Voigt  - CERN, Switzerland                                      *
 *                                                                                *
 * Original author of the TFoam implementation:                                   *
 *      S. Jadach - Institute of Nuclear Physics, Cracow, Poland                  *
 *                                                                                * 
 * Copyright (c) 2008:                                                            *
 *      CERN, Switzerland                                                         * 
 *      MPI-K Heidelberg, Germany                                                 * 
 *                                                                                *
 * Redistribution and use in source and binary forms, with or without             *
 * modification, are permitted according to the terms listed in LICENSE           *
 * (http://tmva.sourceforge.net/LICENSE)                                          *
 **********************************************************************************/

#ifndef ROOT_TMVA_MethodBPDEFoam
#define ROOT_TMVA_MethodBPDEFoam

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// MethodBPDEFoam                                                           //
//                                                                          //
// The PDEFoam method is an                                                 //
// extension of the PDERS method, which divides the multi-dimensional       //
// phase space in a finite number of hyper-rectangles (cells) of constant   //
// event density.                                                           //
// This "foam" of cells is filled with averaged probability-density         //
// information sampled from a training event sample.                        //
//                                                                          //
// For a given number of cells, the binning algorithm adjusts the size      //
// and position of the cells inside the multidimensional phase space        //
// based on a binary-split algorithm, minimizing the variance of the        //
// event density in the cell.                                               //
// The binned event density information of the final foam is stored in      //
// binary trees, allowing for a fast and memory-efficient classification    //
// of events.                                                               //
//                                                                          //
// The implementation of PDEFoam is based on the Monte-Carlo integration    //
// package TFoam included in the analysis package ROOT.                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMVA_MethodPDEFoam
#include "TMVA/MethodPDEFoam.h"
#endif

namespace TMVA {

   class MethodBPDEFoam : public MethodPDEFoam {

   public:
      
      MethodBPDEFoam( const TString& jobName, 
		      const TString& methodTitle, 
		      DataSetInfo& dsi,
		      const TString& theOption = "BPDEFoam",
		      TDirectory* theTargetDir = 0 );

      MethodBPDEFoam( DataSetInfo& dsi, 
		      const TString& theWeightFile,  
		      TDirectory* theTargetDir = NULL );

      virtual ~MethodBPDEFoam( void );

      virtual Bool_t HasAnalysisType( Types::EAnalysisType type, UInt_t numberClasses, UInt_t numberTargets );
    
      // training methods
      void Train( void );

      // calculate the MVA value
      Double_t GetMvaValue( Double_t* err = 0 );

      // reset foams
      void ResetFoams();

      // single boost
      void Boost(UInt_t);

      // test the method
      void SingleTest(UInt_t);

      // reset boost weights of all events
      void ResetBoostWeights();

      // recalculate MVA cut value
      void FindMVACut();

      // initialize histograms
      void InitMonitorHistograms();

      // write Monitor histograms to file
      void WriteMonitorHistograms();

   protected:

      // get help message text
      void GetHelpMessage() const;

   private:

      // monitor histograms
      std::vector<TH1F*>* fMonitorHist;

      // reweighted training histograms
      std::vector<TH1F*>* fTrainingHist_S;
      std::vector<TH1F*>* fTrainingHist_B;

      // classifier respones (MVA value) on testing sample
      std::vector<TH1F*>* fMVAHist_S;
      std::vector<TH1F*>* fMVAHist_B;

      // the option handling methods
      void DeclareOptions();
      void ProcessOptions();

      // number of boosts
      UInt_t fBoostNum;

      // how to weight the misclassified events ( OneOverDiscr, LogisticMap )
      TString fWeightType;

      // Misclassification rates;
      Float_t fMethodError;
      Float_t fOrigMethodError;

      // whether to reset the boost weights of the training sample
      Bool_t  fResetBoostWeights;

      ClassDef(MethodBPDEFoam,0) // Analysis of PDEFoam discriminant (PDEFoam or Mahalanobis approach) 
   };

} // namespace TMVA

#endif // MethodBPDEFoam_H
