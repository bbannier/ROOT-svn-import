/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2002, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_INTEGRATOR_2D
#define ROO_INTEGRATOR_2D

#include "RooFitCore/RooIntegrator1D.hh"
class RooIntegratorConfig ;

class RooIntegrator2D : public RooIntegrator1D {
public:

  // Constructors, assignment etc
  RooIntegrator2D(const RooAbsFunc& function, RooIntegrator1D::SummationRule rule=RooIntegrator1D::Trapezoid,
		  Int_t maxSteps= 0, Double_t eps= 0) ; 
  RooIntegrator2D(const RooAbsFunc& function, const RooIntegratorConfig& config) ;
  RooIntegrator2D(const RooAbsFunc& function, Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax,
		  SummationRule rule= Trapezoid, Int_t maxSteps= 0, Double_t eps= 0) ; 
  RooIntegrator2D(const RooAbsFunc& function, Double_t xmin, Double_t xmax, Double_t ymin, Double_t ymax,
		  const RooIntegratorConfig& config) ;
  virtual ~RooIntegrator2D() ;

protected:

  RooIntegrator1D* _xIntegrator ;
  RooAbsFunc* _xint ;

  ClassDef(RooIntegrator2D,0) // 1-dimensional numerical integration engine
};

#endif
