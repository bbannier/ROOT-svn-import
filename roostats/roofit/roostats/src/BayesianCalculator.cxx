// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Lorenzo Moneta, Gregory Schott, Wouter Verkerke
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/**
   BayesianCalculator class
**/

// include other header files

#include "RooAbsFunc.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooArgSet.h"
#include "RooBrentRootFinder.h"
#include "RooFormulaVar.h"
#include "RooGenericPdf.h"
#include "RooPlot.h"
#include "RooProdPdf.h"

// include header file of this class 
#include "RooStats/BayesianCalculator.h"
#include "RooStats/ModelConfig.h"

#include "Math/IFunction.h"
#include "Math/IntegratorMultiDim.h"
#include "Math/Integrator.h"
#include "Math/RootFinder.h"
#include "RooFunctor.h"
#include "RooFunctor1DBinding.h"
#include "RooTFnBinding.h"
#include "RooMsgService.h"

#include "TAxis.h"
#include "TF1.h"

#include <map>
#include <cmath>

//#include "TRandom.h"
#include "RConfigure.h"

ClassImp(RooStats::BayesianCalculator)

namespace RooStats { 


// first some utility classes and functions 

#ifdef R__HAS_MATHMORE
   const ROOT::Math::RootFinder::EType kRootFinderType = ROOT::Math::RootFinder::kGSL_BRENT; 
#else 
   const ROOT::Math::RootFinder::EType kRootFinderType = ROOT::Math::RootFinder::kBRENT; 
#endif


ROOT::Math::IntegrationMultiDim::Type GetMultiDimIntegrationType(const char * type) { 
   // function to return the type of multi-dim integrator from the given string
   
   ROOT::Math::IntegrationMultiDim::Type integType = ROOT::Math::IntegrationMultiDim::kADAPTIVE; 
   if (type != 0) { 
      TString integrationType(type);  integrationType.ToUpper(); 
      if (integrationType.Contains("VEGAS"))   integType =  ROOT::Math::IntegrationMultiDim::kVEGAS; 
      if (integrationType.Contains("MISER"))   integType =  ROOT::Math::IntegrationMultiDim::kMISER; 
      if (integrationType.Contains("PLAIN"))   integType =  ROOT::Math::IntegrationMultiDim::kPLAIN; 
   }
#ifndef R__HAS_MATHMORE
   if (integType != ROOT::Math::IntegrationMultiDim::kADAPTIVE) { 
      oocoutI((TObject*)0,InputArguments) << "BayesianCalculator:  MathMore is not present - can use only adaptive multi-dim integration " << std::endl;
      integType =  ROOT::Math::IntegrationMultiDim::kADAPTIVE;
   }
#endif
   return integType; 
}
   



struct  LikelihoodFunction { 
   LikelihoodFunction(RooFunctor & f) : fFunc(f) {}

   double operator() (const double *x ) const { 
      double nll = fFunc(x);
      double likelihood =  std::exp(-nll);
//       ooccoutD((TObject*)0,NumIntegration)  
//          << "x[0] = " << x[0] << " x[1] = " << x[1] << "  nll = " << nll << " likelihood = " << likelihood << std::endl;
      return likelihood; 
   }

   // for the 1D case
   double operator() (double x) const { 
      double tmp = x; 
      return (*this)(&tmp); 
   }

   RooFunctor & fFunc; 
};

class PosteriorCdfFunction : public ROOT::Math::IGenFunction { 

public:
   PosteriorCdfFunction(ROOT::Math::IntegratorMultiDim & ig, double * xmin, double * xmax) : 
      fIntegrator(ig), fXmin(xmin), fXmax(xmax), fNorm(1.0), fOffset(0.0), fMaxX(xmax[0]), 
      fHasNorm(false), fUseOldValues(true) 
   {
      // compute first the normalization with  the poi 
      fNorm = (*this)(xmax[0] );  
      fHasNorm = true; 
      fNormCdfValues.insert(std::make_pair(xmin[0], 0) );
      fNormCdfValues.insert(std::make_pair(xmax[0], 1.0) );

   }

//    PosteriorCdfFunction(ROOT::Math::IntegratorMultiDim & ig, double * xmin, double * xmax, double norm) : 
//       fIntegrator(ig), fXmin(xmin), fXmax(xmax), fNorm(norm), fOffset(0.0), fMaxX(xmax[0]), fHasNorm(true)  {
//       // compute posterior cdf from a normalization given from outside 
//    }

   ROOT::Math::IGenFunction * Clone() const { 
      ooccoutD((TObject*)0,NumIntegration) << " cloning function .........." << std::endl;
      return new PosteriorCdfFunction(*this); 
   }

   void SetOffset(double offset) { fOffset = offset; }

private:
   double DoEval (double x) const { 
      // evaluate cdf at poi value x by integrating poi from [-xmin,x] and all the nuisances  
      fXmax[0] = x;
      if (x <= fXmin[0] ) return -fOffset; 
      // could also avoid a function evaluation at maximum
      if (x >= fMaxX && fHasNorm) return 1. - fOffset;  // cdf is bound to these values

      // computes the intergral using previous cdf estimate
      double  normcdf0 = 0; 
      if (fHasNorm && fUseOldValues) { 
         // look in the map of the stored cdf values the closes one
         std::map<double,double>::iterator itr = fNormCdfValues.upper_bound(x); 
         itr--;   // upper bound returns a poistion 1 up of the value we want
         if (itr != fNormCdfValues.end() ) { 
            fXmin[0] = itr->first; 
            normcdf0 = itr->second;
            ooccoutD((TObject*)0,NumIntegration) << "PosteriorCdfFunction:   position for x = " << x << " is  = " << itr->first << std::endl; 
         }
      }


      double cdf = fIntegrator.Integral(fXmin,fXmax);  
      double error = fIntegrator.Error(); 
      double normcdf =  cdf/fNorm;  // normalize the cdf 
      ooccoutD((TObject*)0,NumIntegration) << "PosteriorCdfFunction:   x0 = " << fXmin[0] << " x =  " 
                                          << x << " cdf(x) =  " << cdf << " +/- " << error 
                                          << "  norm-cdf(x) = " << normcdf << std::endl; 
      if (cdf != 0 && error/cdf > 0.2 ) 
         oocoutW((TObject*)0,NumIntegration) << "PosteriorCdfFunction: integration error  is larger than 20 %   x0 = " << fXmin[0]  
                                              << " x = " << x << " cdf(x) = " << cdf << " +/- " << error << std::endl;

      if (!fHasNorm) { 
         oocoutI((TObject*)0,NumIntegration) << "PosteriorCdfFunction - posterior norm. integral = " 
                                             << cdf << " +/- " << error << std::endl; 
         return cdf; 
      }

      normcdf += normcdf0;

      // store values in the map
      if (fUseOldValues) { 
         fNormCdfValues.insert(std::make_pair(x, normcdf) );
      }

      if (normcdf > 1. + 3 * error/fNorm) {
         oocoutW((TObject*)0,NumIntegration) << "PosteriorCdfFunction: normalized cdf values is larger than 1" 
                                              << " x = " << x << " normcdf(x) = " << normcdf << " +/- " << error/fNorm << std::endl;
      }

      return normcdf - fOffset;  // apply an offset (for finding the roots) 
   }

   ROOT::Math::IntegratorMultiDim & fIntegrator; 
   mutable double * fXmin; 
   mutable double * fXmax; 
   double fNorm; 
   double fOffset;
   double fMaxX;  // maxumum value of x 
   bool fHasNorm; // flag to control irst call to the function 
   bool fUseOldValues;  // use old cdf values
   mutable std::map<double,double> fNormCdfValues; 
};

class PosteriorFunction : public ROOT::Math::IGenFunction { 

public: 


   PosteriorFunction(RooAbsReal & nll, RooRealVar & poi, RooArgList & nuisParams, const char * integType = 0, double norm = 1.0) :
      fFunctor(nll, nuisParams, RooArgList() ),
      fLikelihood(fFunctor), 
      fPoi(&poi),
      fXmin(nuisParams.getSize() ),
      fXmax(nuisParams.getSize() ), 
      fNorm(norm)      
   { 

      for (unsigned int i = 0; i < fXmin.size(); ++i) { 
         RooRealVar & var = (RooRealVar &) nuisParams[i]; 
         fXmin[i] = var.getMin(); 
         fXmax[i] = var.getMax();
      }
      if (fXmin.size() == 1) { // 1D case  
         fIntegratorOneDim = std::auto_ptr<ROOT::Math::Integrator>(new ROOT::Math::Integrator() );
         fIntegratorOneDim->SetFunction(fLikelihood);
      }
      else { // multiDim case          
         fIntegratorMultiDim = 
            std::auto_ptr<ROOT::Math::IntegratorMultiDim>(new ROOT::Math::IntegratorMultiDim(GetMultiDimIntegrationType(integType) ) );
         fIntegratorMultiDim->SetFunction(fLikelihood, fXmin.size());
      }
   }
      
      
   ROOT::Math::IGenFunction * Clone() const { 
      assert(1); 
      return 0; // cannot clone this function for integrator 
   } 
   
private: 
   double DoEval (double x) const { 
      // evaluate posterior function at a poi value x by integrating all nuisance parameters

      fPoi->setVal(x);
      double f = 0; 
      double error = 0; 
      if (fXmin.size() == 1) { // 1D case  
         f = fIntegratorOneDim->Integral(fXmin[0],fXmax[0]); 
         error = fIntegratorOneDim->Error();
      }
      else {
         f = fIntegratorMultiDim->Integral(&fXmin[0],&fXmax[0]); 
         error = fIntegratorMultiDim->Error();
      }

      if (f != 0 && error/f > 0.2 ) 
         ooccoutW((TObject*)0,NumIntegration) << "PosteriorFunction::DoEval - Error from integration is larger than 20 % " 
                                              << "x = " << x << " p(x) = " << f << " +/- " << error << std::endl;

      return f / fNorm;
   }

   RooFunctor fFunctor; 
   LikelihoodFunction fLikelihood; 
   RooRealVar * fPoi;
   std::auto_ptr<ROOT::Math::Integrator>  fIntegratorOneDim; 
   std::auto_ptr<ROOT::Math::IntegratorMultiDim>  fIntegratorMultiDim; 
   std::vector<double> fXmin; 
   std::vector<double> fXmax; 
   double fNorm;
};



////////////////////////////////////////////////////////////////////////////////////////////////////////////

BayesianCalculator::BayesianCalculator() :
  fData(0),
  fPdf(0),
  fPriorPOI(0),
  fProductPdf (0), fLogLike(0), fLikelihood (0), fIntegratedLikelihood (0), fPosteriorPdf(0), 
  fPosteriorFunction(0),
  fLower(0), fUpper(0),
  fBrfPrecision(0.00005),
  fValidInterval(false),
  fSize(0.05)
{
   // default constructor
}

BayesianCalculator::BayesianCalculator( /* const char* name,  const char* title, */						   
						    RooAbsData& data,
                                                    RooAbsPdf& pdf,
						    const RooArgSet& POI,
						    RooAbsPdf& priorPOI,
						    const RooArgSet* nuisanceParameters ) :
   //TNamed( TString(name), TString(title) ),
  fData(&data),
  fPdf(&pdf),
  fPOI(POI),
  fPriorPOI(&priorPOI),
  fProductPdf (0), fLogLike(0), fLikelihood (0), fIntegratedLikelihood (0), fPosteriorPdf(0),
  fPosteriorFunction(0),
  fLower(0), fUpper(0),
  fBrfPrecision(0.00005),
  fNScanBins(-1),
  fValidInterval(false),  
  fSize(0.05)
{
   // constructor
   if (nuisanceParameters) fNuisanceParameters.add(*nuisanceParameters); 
}

BayesianCalculator::BayesianCalculator( RooAbsData& data,
                       ModelConfig & model) : 
   fData(&data), 
   fPdf(model.GetPdf()),
   fPriorPOI( model.GetPriorPdf()),
   fProductPdf (0), fLogLike(0), fLikelihood (0), fIntegratedLikelihood (0), fPosteriorPdf(0),
   fPosteriorFunction(0),
   fLower(0), fUpper(0),
   fBrfPrecision(0.00005),
   fNScanBins(-1),
   fValidInterval(false),
   fSize(0.05)
{
   // constructor from Model Config
   SetModel(model);
}


BayesianCalculator::~BayesianCalculator()
{
   // destructor
   ClearAll(); 
}

void BayesianCalculator::ClearAll() const { 
   // clear cached pdf objects
   if (fProductPdf) delete fProductPdf; 
   if (fLogLike) delete fLogLike; 
   if (fLikelihood) delete fLikelihood; 
   if (fIntegratedLikelihood) delete fIntegratedLikelihood; 
   if (fPosteriorPdf) delete fPosteriorPdf;    
   if (fPosteriorFunction) delete fPosteriorFunction; 
   fPosteriorPdf = 0; 
   fPosteriorFunction = 0; 
   fProductPdf = 0;
   fLogLike = 0; 
   fLikelihood = 0; 
   fIntegratedLikelihood = 0; 
   fLower = 0;
   fUpper = 0;
   fValidInterval = false;
}

void BayesianCalculator::SetModel(const ModelConfig & model) {
   // set the model
   fPdf = model.GetPdf();
   fPriorPOI =  model.GetPriorPdf(); 
   // assignment operator = does not do a real copy the sets (must use add method) 
   fPOI.removeAll();
   fNuisanceParameters.removeAll();
   if (model.GetParametersOfInterest()) fPOI.add( *(model.GetParametersOfInterest()) );
   if (model.GetNuisanceParameters())  fNuisanceParameters.add( *(model.GetNuisanceParameters() ) );

   // invalidate the cached pointers
   ClearAll(); 
}


RooArgSet* BayesianCalculator::GetMode(RooArgSet* /* parameters */) const
{
  /// Returns the value of the parameters for the point in
  /// parameter-space that is the most likely.
  // Should cover multi-dimensional cases...
  // How do we do if there are points that are equi-probable?

  return 0;
}


RooAbsReal* BayesianCalculator::GetPosteriorFunction() const
{
   /// build and return the posterior function (not normalized)
   if (fIntegratedLikelihood) return fIntegratedLikelihood; 
   if (fLikelihood) return fLikelihood; 

   // run some sanity checks
   if (!fPdf ) {
      coutE(InputArguments) << "BayesianCalculator::GetPosteriorPdf - missing pdf model" << std::endl;
      return 0;
   }
   if (!fPriorPOI) { 
      coutE(InputArguments) << "BayesianCalculator::GetPosteriorPdf - missing prior pdf" << std::endl;
      return 0;
   }
   if (fPOI.getSize() == 0) {
      coutE(InputArguments) << "BayesianCalculator::GetPosteriorPdf - missing parameter of interest" << std::endl;
      return 0;
   }
   if (fPOI.getSize() > 1) { 
      coutE(InputArguments) << "BayesianCalculator::GetPosteriorPdf - current implementation works only on 1D intervals" << std::endl;
      return 0; 
   }

   // create a unique name for the product pdf 
   TString prodName = TString("product_") + TString(fPdf->GetName()) + TString("_") + TString(fPriorPOI->GetName() );   
   fProductPdf = new RooProdPdf(prodName,"",RooArgList(*fPdf,*fPriorPOI));

   RooArgSet* constrainedParams = fProductPdf->getParameters(*fData);

   // use RooFit::Constrain() to make product of likelihood with prior pdf
   fLogLike = fProductPdf->createNLL(*fData, RooFit::Constrain(*constrainedParams) );

   delete constrainedParams;

   TString likeName = TString("likelihood_") + TString(fProductPdf->GetName());   
   fLikelihood = new RooFormulaVar(likeName,"exp(-@0)",RooArgList(*fLogLike));

   // if no nuisance p[arameter we can just return the likelihood funtion
   if (fNuisanceParameters.getSize() == 0) return fLikelihood; 

   // case of use RooFit 
   if (fIntegrationType.Contains("ROOFIT") ) { 
      fIntegratedLikelihood = fLikelihood->createIntegral(fNuisanceParameters);
   }

   // use integration method if there are nuisance parameters 
   else  { 

      RooRealVar* poi = dynamic_cast<RooRealVar*>( fPOI.first() ); 
      assert(poi);

      RooArgList nuisParams(fNuisanceParameters); 
      fPosteriorFunction = new PosteriorFunction(*fLogLike, *poi, nuisParams, fIntegrationType ); 
      
      TString name = "posteriorfunction_from_"; 
      name += fLogLike->GetName();  
      fIntegratedLikelihood = new RooFunctor1DBinding(name,name,*fPosteriorFunction,*poi);

   }


   return fIntegratedLikelihood; 
}

RooAbsPdf* BayesianCalculator::GetPosteriorPdf() const
{
   /// build and return the posterior pdf (i.e posterior function normalized to all range of poi
   ///NOTE: user must delete the returned object 
   
   RooAbsReal * plike = GetPosteriorFunction(); 
   
   // create a unique name on the posterior from the names of the components
   TString posteriorName = this->GetName() + TString("_posteriorPdf_") + plike->GetName(); 
   RooAbsPdf * posteriorPdf = new RooGenericPdf(posteriorName,"@0",*plike);

   return posteriorPdf;
}


RooPlot* BayesianCalculator::GetPosteriorPlot(bool norm, double precision ) const
{
  /// return a RooPlot with the posterior  and the credibility region

   if (!fLikelihood) GetPosteriorFunction(); 
   RooAbsReal * posterior = fIntegratedLikelihood; 
   if (norm) posterior = fPosteriorPdf; 
   if (!posterior) { 
      posterior = GetPosteriorFunction();
      if (norm) { 
         if (fPosteriorPdf) delete fPosteriorPdf;
         fPosteriorPdf = GetPosteriorPdf();
         posterior = fPosteriorPdf;
      }
   }
   if (!posterior) return 0;

   if (!fValidInterval) GetInterval();

   RooAbsRealLValue* poi = dynamic_cast<RooAbsRealLValue*>( fPOI.first() );
   assert(poi);

   RooPlot* plot = poi->frame();

   plot->SetTitle(TString("Posterior probability of parameter \"")+TString(poi->GetName())+TString("\""));  
   posterior->plotOn(plot,RooFit::Range(fLower,fUpper,kFALSE),RooFit::VLines(),RooFit::DrawOption("F"),RooFit::MoveToBack(),RooFit::FillColor(kGray),RooFit::Precision(precision));
   posterior->plotOn(plot);
   plot->GetYaxis()->SetTitle("posterior function");
   
   return plot; 
}

void BayesianCalculator::SetIntegrationType(const char * type) { 
   fIntegrationType = TString(type); 
   fIntegrationType.ToUpper(); 
}


SimpleInterval* BayesianCalculator::GetInterval() const
{
  /// returns a SimpleInterval with lower and upper bounds on the
  /// parameter of interest. Applies the central ordering rule to
  /// compute the credibility interval. Covers only the case with one
  /// single parameter of interest

   if (fValidInterval) 
      coutW(Eval) << "BayesianCalculator::GetInterval - recomputing interval for the same CL and same model" << std::endl;

   RooRealVar* poi = dynamic_cast<RooRealVar*>( fPOI.first() ); 
   assert(poi);

   RooAbsReal * posterior =  GetPosteriorFunction();

   if (fNScanBins > 0) { 

      coutI(Eval) << "BayesianCalculator::GetInterval - use a scan of posterior function in nbins = " << fNScanBins << std::endl;

      // use the scanned of the function
      TF1 * tmp = posterior->asTF(fPOI); 
      double prob[2]; 
      double limits[2];
      prob[0] = 0.5 * fSize;
      prob[1] = 1.- prob[0];
      // binned the function in nbins and evaluate at thos points
      tmp->SetNpx(fNScanBins);
      fApproxPosterior = (TF1*) tmp->Clone();
      fApproxPosterior->GetQuantiles(2,limits,prob);
      fLower = limits[0]; 
      fUpper = limits[1];
      // save this function for future reuse 
      // I can delete now original posterior and use this approximated copy
      delete tmp;
      TString name = posterior->GetName() + TString("_approx");
      TString title = posterior->GetTitle() + TString("_approx");
      RooAbsReal * posterior2 = new RooTFnBinding(name,title,fApproxPosterior,fPOI);
      if (posterior == fIntegratedLikelihood) { 
         delete fIntegratedLikelihood;
         fIntegratedLikelihood = posterior2; 
      }
      else if (posterior == fLikelihood) { 
         delete fLikelihood; 
         fLikelihood = posterior2;
      }
      else {
         assert(1); // should never happen this case
      }

   }

   else { 
      // use integration method if there are nuisance parameters 
      if (fNuisanceParameters.getSize() > 0) { 
         
         
         // need to remove the constant parameters
         RooArgList bindParams; 
         bindParams.add(fPOI);
         bindParams.add(fNuisanceParameters);
         
         
         RooFunctor functor_nll(*fLogLike, bindParams, RooArgList());
         
         
         // compute the intergal of the exp(-nll) function
         LikelihoodFunction fll(functor_nll);
      
         ROOT::Math::IntegratorMultiDim ig(GetMultiDimIntegrationType(fIntegrationType)); 
         ig.SetFunction(fll,bindParams.getSize()); 
         
         std::vector<double> pmin(bindParams.getSize());
         std::vector<double> pmax(bindParams.getSize());
         std::vector<double> par(bindParams.getSize());
         for (unsigned int i = 0; i < pmin.size(); ++i) { 
            RooRealVar & var = (RooRealVar &) bindParams[i]; 
            pmin[i] = var.getMin(); 
            pmax[i] = var.getMax();
            par[i] = var.getVal();
         } 
         
         
         //bindParams.Print("V");
         
         PosteriorCdfFunction cdf(ig, &pmin[0], &pmax[0] ); 
         
         
         //find the roots 

         ROOT::Math::RootFinder rf(kRootFinderType); 
         
         ccoutD(Eval) << "BayesianCalculator::GetInterval - finding roots of posterior using RF " << rf.Name() 
                      << " with precision = " << fBrfPrecision;
         
         cdf.SetOffset(fSize/2);
         ccoutD(NumIntegration) << "Integrating posterior to get cdf and search lower limit at p =" << fSize/2 << std::endl;
         bool ok = false; 
         ok = rf.Solve(cdf, poi->getMin(),poi->getMax() , 200,fBrfPrecision, fBrfPrecision); 
         if (!ok) 
            coutE(NumIntegration) << "BayesianCalculator::GetInterval - Error from root finder when searching lower limit !" << std::endl;
         fLower = rf.Root(); 
         cdf.SetOffset(1.-fSize/2);
         ccoutD(NumIntegration) << "Integrating posterior to get cdf and search upper interval limit at p =" << 1-fSize/2 << std::endl;
         ok = rf.Solve(cdf, fLower,poi->getMax() , 200, fBrfPrecision, fBrfPrecision); 
         if (!ok) 
            coutE(NumIntegration) << "BayesianCalculator::GetInterval - Error from root finder when searching upper limit !" << std::endl;

         fUpper = rf.Root(); 
      }

      // case of no nuisance - just use createCdf
      else { 

         if (!fPosteriorPdf) fPosteriorPdf = (RooAbsPdf*) GetPosteriorPdf();
         
         RooAbsReal* cdf = fPosteriorPdf->createCdf(fPOI,RooFit::ScanNoCdf());
         
         RooAbsFunc* cdf_bind = cdf->bindVars(fPOI,&fPOI);
         RooBrentRootFinder brf(*cdf_bind);
         brf.setTol(fBrfPrecision); // set the brf precision
         
         double tmpVal = poi->getVal();  // patch used because findRoot changes the value of poi
         
         double y = fSize/2;
         brf.findRoot(fLower,poi->getMin(),poi->getMax(),y);
         
         y=1-fSize/2;
         bool ret = brf.findRoot(fUpper,poi->getMin(),poi->getMax(),y);
         if (!ret) coutE(Eval) << "BayesianCalculator::GetInterval "
                               << "Error returned from Root finder, estimated interval is not fully correct" 
                            << std::endl;
      
         poi->setVal(tmpVal); // patch: restore the original value of poi
         
         delete cdf_bind;
         delete cdf;
      }
   }

   fValidInterval = true; 
   
   TString interval_name = TString("BayesianInterval_a") + TString(this->GetName());
   SimpleInterval * interval = new SimpleInterval(interval_name,*poi,fLower,fUpper,ConfidenceLevel());
   interval->SetTitle("SimpleInterval from BayesianCalculator");
   
   return interval;
}

} // end namespace RooStats

