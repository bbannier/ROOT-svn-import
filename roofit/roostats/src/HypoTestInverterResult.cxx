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
   HypoTestInverterResult class: holds the array of hypothesis test results and compute a confidence interval.
   Based on the RatioFinder code available in the RooStatsCms package developed by Gregory Schott and Danilo Piparo
   Ported and adapted to RooStats by Gregory Schott
   Some contributions to this class have been written by Matthias Wolf (error estimation)
**/


// include header file of this class 
#include "RooStats/HypoTestInverterResult.h"

#include "RooStats/HypoTestInverterPlot.h"
#include "RooStats/HybridResult.h"
#include "RooStats/SamplingDistribution.h"
#include "RooMsgService.h"
#include "RooGlobalFunc.h"

#include "TF1.h"
#include "TGraphErrors.h"
#include <cmath>
#include "Math/BrentRootFinder.h"
#include "Math/WrappedFunction.h"

ClassImp(RooStats::HypoTestInverterResult)

using namespace RooStats;
using namespace RooFit;


HypoTestInverterResult::HypoTestInverterResult(const char * name ) :
   SimpleInterval(name),
   fUseCLs(false),
   fInterpolateLowerLimit(true),
   fInterpolateUpperLimit(true),
   fFittedLowerLimit(false),
   fFittedUpperLimit(false),
   fLowerLimitError(0),
   fUpperLimitError(0)
{
  // default constructor
}


HypoTestInverterResult::HypoTestInverterResult( const char* name,
						const RooRealVar& scannedVariable,
						double cl ) :
   SimpleInterval(name,scannedVariable,-999,999,cl), 
   fUseCLs(false),
   fInterpolateLowerLimit(true),
   fInterpolateUpperLimit(true),
   fFittedLowerLimit(false),
   fFittedUpperLimit(false),
   fLowerLimitError(0),
   fUpperLimitError(0)
{
  // constructor 
   fYObjects.SetOwner();
}


HypoTestInverterResult::~HypoTestInverterResult()
{
   // destructor
   // no need to delete explictly the objects in the TList since the TList owns the objects
}


bool HypoTestInverterResult::Add( const HypoTestInverterResult& /* otherResult */  )
{
  /// Merge this HypoTestInverterResult with another
  /// HypoTestInverterResult passed as argument

  std::cout << "Sorry, this function is not yet implemented\n";

  return true;
}

 
double HypoTestInverterResult::GetXValue( int index ) const
{
  if ( index >= ArraySize() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  return fXValues[index];
}

double HypoTestInverterResult::GetYValue( int index ) const
{
  if ( index >= ArraySize() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  if (fUseCLs) 
    return ((HypoTestResult*)fYObjects.At(index))->CLs();
  else 
     return ((HypoTestResult*)fYObjects.At(index))->CLsplusb();  // CLs+b
}

double HypoTestInverterResult::GetYError( int index ) const
{
  if ( index >= ArraySize() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return -999;
  }

  if (fUseCLs) 
    return ((HypoTestResult*)fYObjects.At(index))->CLsError();
  else 
    return ((HypoTestResult*)fYObjects.At(index))->CLsplusbError();
}

HypoTestResult* HypoTestInverterResult::GetResult( int index ) const
{
  if ( index >= ArraySize() || index<0 ) {
    std::cout << "Problem: You are asking for an impossible array index value\n";
    return 0;
  }

  return ((HypoTestResult*) fYObjects.At(index));
}

struct InterpolatedGraph { 
   InterpolatedGraph( const TGraph & g, double target, const char * interpOpt) : 
      fGraph(g), fTarget(target), fInterpOpt(interpOpt) {}

   // return interpolated value for x - target
   double operator() (double x) const { 
      return fGraph.Eval(x, (TSpline*) 0, fInterpOpt) - fTarget;
   }
   const TGraph & fGraph;
   double fTarget;
   TString fInterpOpt;
}; 

double HypoTestInverterResult::GetGraphX(const TGraph & graph, double y0, const char * opt) const  {
   // return the X value of the given graph for the target value y0
   // the graph is evaluated using linea rinterpolation by default. 
   // if option = "S" a TSpline3 is used 

   InterpolatedGraph f(graph,y0,opt);
   ROOT::Math::BrentRootFinder brf;
   ROOT::Math::WrappedFunction<InterpolatedGraph> wf(f);

   // find reasanable xmin and xmax 
   double xmin = graph.GetX()[0];
   double xmax = graph.GetX()[graph.GetN()-1];
   if ( graph.GetY()[0] < y0 ) { 
      const RooRealVar* var = dynamic_cast<RooRealVar*>( fParameters.first() );
      if (var) xmin = var->getMin();
   }
   if (graph.GetY()[graph.GetN()-1] > y0 ) { 
      const RooRealVar* var = dynamic_cast<RooRealVar*>( fParameters.first() );
      if (var) xmax = var->getMax();
   }

   brf.SetFunction(wf,xmin,xmax);
   brf.SetNpx(20);
   brf.Solve(100, 1.E-6, 1.E-6);
   return brf.Root();
}

double HypoTestInverterResult::FindInterpolatedLimit(double target, const char * opt)
{
   // interpolate to find limit value
   std::cout << "Interpolate the upper limit between the 2 results closest to the target confidence level" << endl;

   if (ArraySize()<2) {
      std::cout << "Error: not enough points to get the inverted interval\n";
      if (target<0.5) return ((RooRealVar*)fParameters.first())->getMax();
      else return ((RooRealVar*)fParameters.first())->getMin();
   }

   // sort the values in x 
   int n = ArraySize();
   std::vector<unsigned int> index(n );
   TMath::SortItr(fXValues.begin(), fXValues.end(), index.begin(), false); 
   // make a graph with the sorted point
   TGraph graph(n);
   for (int i = 0; i < n; ++i) 
      graph.SetPoint(i, GetXValue(index[i]), GetYValue(index[i] ) );

   
   return GetGraphX(graph, target, opt);

/*   
   double v1 = GetYValue(index[0])-target;
   int i1 = index[0];
   double v2 = GetYValue(index[1])-target;
   int i2 = index[1];

   

   std::cout << i1 << "  " << i2  << "  " << v1 << "  " << v2 << "  " 
             << GetXValue(i1) << "  " << GetXValue(i2) << std::endl;
   
   if (ArraySize()>2) {
      for (int j=2; j<ArraySize(); j++) {
         int i = index[j];
         double vt = GetYValue(i)-target;
         if ( vt > 0 && vt < v1  ) { // point above the line
            v1 = vt; 
            i1 = i; 
         }
         else if ( vt < 0 &&  vt > v2) { // point below the line
            v2 = vt;
            i2 = i;
         }
         std::cout << i1 << "  " << i2  << "  " << v1 << "  " << v2 << std::endl;
      }
   }
   
   std::cout << "Interpolate between points " << i1 << "  " << i2  
             << "  " 
             << GetXValue(i1) << "  " << GetXValue(i2) << std::endl;

   return GetXValue(i1)+(target-GetYValue(i1))*(GetXValue(i2)-GetXValue(i1))/(GetYValue(i2)-GetYValue(i1));
*/
}

int HypoTestInverterResult::FindClosestPointIndex(double target)
{
  // find the object with the smallest error that is < 1 sigma from the target
  double bestValue = fabs(GetYValue(0)-target);
  int bestIndex = 0;
  for (int i=1; i<ArraySize(); i++) {
    if ( fabs(GetYValue(i)-target)<GetYError(i) ) { // less than 1 sigma from target CL
      double value = fabs(GetYValue(i)-target);
      if ( value<bestValue ) {
	bestValue = value;
	bestIndex = i;
      }
    }
  }

  return bestIndex;
}

Double_t HypoTestInverterResult::LowerLimit()
{
  if (fFittedLowerLimit) return fLowerLimit;
  //std::cout << "finding point with cl = " << 1-(1-ConfidenceLevel())/2 << endl;
  if ( fInterpolateLowerLimit ){
     fLowerLimit = FindInterpolatedLimit(1-ConfidenceLevel());
  } else {
     fLowerLimit = GetXValue( FindClosestPointIndex((1-ConfidenceLevel())) );
  }
  return fLowerLimit;
}

Double_t HypoTestInverterResult::UpperLimit()
{
   //std::cout << "finding point with cl = " << (1-ConfidenceLevel())/2 << endl;
  if (fFittedUpperLimit) return fUpperLimit;
  if ( fInterpolateUpperLimit ) {
     fUpperLimit = FindInterpolatedLimit((1-ConfidenceLevel()));
  } else {
     fUpperLimit = GetXValue( FindClosestPointIndex((1-ConfidenceLevel())) );
  }
  return fUpperLimit;
}

Double_t HypoTestInverterResult::CalculateEstimatedError(double target)
{
  // Return an error estimate on the upper limit.  This is the error on
  // either CLs or CLsplusb divided by an estimate of the slope at this
  // point.
   
  if (ArraySize()==0) {
    std::cout << "Empty result \n";
    return 0;
  }

  if (ArraySize()<2) {
    std::cout << "not enough points to get the inverted interval\n";
  }
 
  // The graph contains the points sorted by their x-value
  HypoTestInverterPlot plot("plot", "", this);
  TGraphErrors* graph = plot.MakePlot();
  double* xs = graph->GetX();
  const double minX = xs[0];
  const double maxX = xs[ArraySize()-1];

  TF1 fct("fct", "exp([0] * x + [1] * x**2)", minX, maxX);
  graph->Fit(&fct,"Q");

  int index = FindClosestPointIndex(target);
  double m = fct.Derivative( GetXValue(index) );
  double theError = fabs( GetYError(index) / m);

  delete graph;

  return theError;
}


Double_t HypoTestInverterResult::LowerLimitEstimatedError()
{
  if (fFittedLowerLimit) return fLowerLimitError;

   //std::cout << "The HypoTestInverterResult::LowerLimitEstimatedError() function evaluates only a rought error on the upper limit. Be careful when using this estimation\n";
  if (fInterpolateLowerLimit) std::cout << "The lower limit was an interpolated results... in this case the error is even less reliable (the Y-error bars are currently not used in the interpolation)\n";

  return CalculateEstimatedError(1-ConfidenceLevel());
}


Double_t HypoTestInverterResult::UpperLimitEstimatedError()
{

  if (fFittedUpperLimit) return fUpperLimitError;

   //std::cout << "The HypoTestInverterResult::UpperLimitEstimatedError() function evaluates only a rought error on the upper limit. Be careful when using this estimation\n";
  if (fInterpolateUpperLimit) std::cout << "The upper limit was an interpolated results... in this case the error is even less reliable (the Y-error bars are currently not used in the interpolation)\n";

  return CalculateEstimatedError((1-ConfidenceLevel()));
}

SamplingDistribution *  HypoTestInverterResult::GetBackgroundDistribution(int index ) const { 
   // get the background distribution

   HypoTestResult * firstResult = (HypoTestResult*) fYObjects.At(index);
   if (!firstResult) return 0;
   return firstResult->GetBackGroundIsAlt() ? firstResult->GetAltDistribution() : firstResult->GetNullDistribution();
}

SamplingDistribution *  HypoTestInverterResult::GetSignalAndBackgroundDistribution(int index) const { 
   // get the background distribution
   HypoTestResult * result = (HypoTestResult*) fYObjects.At(index);
   if (!result) return 0;
   return !result->GetBackGroundIsAlt() ? result->GetAltDistribution() : result->GetNullDistribution();       
}

SamplingDistribution *  HypoTestInverterResult::GetExpectedDistribution(int index) const { 
   // get the expected p-value distribution at the scanned point index

   static bool useFirstB = false;
   // get the S+B distribution 
   int bIndex = (useFirstB) ? 0 : index;
 
   SamplingDistribution * bDistribution = GetBackgroundDistribution(bIndex);
   SamplingDistribution * sbDistribution = GetSignalAndBackgroundDistribution(index);
   if (!bDistribution || !sbDistribution) return 0;

   HypoTestResult * result = (HypoTestResult*) fYObjects.At(index);

   // create a new HypoTestResult
   HypoTestResult tempResult; 
   tempResult.SetNullDistribution( sbDistribution );
   tempResult.SetAltDistribution( bDistribution );
   tempResult.SetBackgroundAsAlt( true);
   tempResult.SetPValueIsRightTail( result->GetPValueIsRightTail() );
   
   std::vector<double> values(bDistribution->GetSize()); 
   for (int i = 0; i < bDistribution->GetSize(); ++i) { 
      tempResult.SetTestStatisticData( bDistribution->GetSamplingDistribution()[i] );
      values[i] = (fUseCLs) ? tempResult.CLs() : tempResult.CLsplusb();
   }
   return new SamplingDistribution("expected values","expected values",values);
}



SamplingDistribution *  HypoTestInverterResult::GetUpperLimitDistribution(const char * opt ) const { 

   //std::cout << "Interpolate the upper limit between the 2 results closest to the target confidence level" << endl;

  if (ArraySize()<2) {
    std::cout << "Error: not enough points to get the inverted interval\n";
    return 0; 
  }

  oocoutI(this,Eval) << "HypoTestInverterResult - computing upper limit distribution...." << std::endl;



  double target = 1-fConfidenceLevel;
  std::vector<SamplingDistribution*> distVec(ArraySize() );
  for (unsigned int i = 0; i < distVec.size(); ++i) {
     distVec[i] =  GetExpectedDistribution(i);
  }

  // sort the values in x 
  std::vector<unsigned int> index(ArraySize() );
  TMath::SortItr(fXValues.begin(), fXValues.end(), index.begin(), false); 

  // SamplingDistribution * dist0 = distVec[index[0]];
  // SamplingDistribution * dist1 = distVec[index[1]];

  int n = distVec[0]->GetSize();
  std::vector<double> limits(n);
  // loop on the p values and find the limit for each expcted one 
  for (int j = 0; j < n; ++j ) {

     TGraph g(ArraySize() );
     int npoints = ArraySize();
     for (int k = 0; k < npoints ; ++k) { 
        g.SetPoint(k, GetXValue(index[k]), distVec[index[k]]->GetSamplingDistribution()[j] );
     }

     limits[j] = GetGraphX(g, target,opt);

     // double y1 = dist0->GetSamplingDistribution()[j];
     // double y2 = dist1->GetSamplingDistribution()[j];
     // double v1 = std::abs(y1-target);
     // int i1 = index[0];
     // double v2 = std::abs(y2-target);
     // int i2 = index[1];

     // if (ArraySize()>2) {
     //    for (int k=2; k<ArraySize(); k++) {
     //       double yi = distVec[index[k]]->GetSamplingDistribution()[j];
     //       double vt = std::abs(y1-target);
     //       if ( vt<v1 || vt<v2 ) {
     //          if ( v1<v2 ) {
     //             v2 = vt;
     //             y2 = yi;
     //             i2 = index[k];
     //          } else {
     //             v1 = vt;
     //             y1 = yi;
     //             i1 = index[k];
     //          }
     //       }
     //    }
     // }
     //limits[j] =  GetXValue(i1)+(target-y1)*(GetXValue(i2)-GetXValue(i1))/(y2-y1);
     // oocoutI(this,Eval) << "HypoTestInverterResult - interpolate between points " <<  GetXValue(i1) << " ," << y1
     //                    << " and " << GetXValue(i2)  << " , " << y2 << " limit is " << limits[j] << std::endl;

  }
  return new SamplingDistribution("Expected upper Limit","Expected upper limits",limits);
  
}
