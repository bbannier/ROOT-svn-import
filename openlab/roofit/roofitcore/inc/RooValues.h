/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id$                                
 * Authors:                                                                  *
 *   Alfio Lazzaro, CERN openlab, alfio.lazzaro@cern.ch                      *
 *   Yngve Sneen Lindal, CERN openlab, yngve.sneen.lindal@cern.ch            *
 *                                                                           *
 * Copyright (c) 2011, CERN openlab. All rights reserved.                    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_VALUES
#define ROO_VALUES

#include "Rtypes.h"
#include "Riostream.h"

class RooValues {
 public:
  RooValues() {} ;
  inline Bool_t isEmpty() const { return _valuesCPU.empty(); }
  inline Int_t getSize() const { return _valuesCPU.size(); }

  inline void resize(Int_t n) { _valuesCPU.resize(n); } // Use CPU
  // inline void resize(Int_t n, Int_t gpuID) { _valuesGPU.resize(); }

  inline Double_t operator[](Int_t i) const { return _valuesCPU[i]; }
  //  __device inline Double_t operator[](Int_t i, Int_t gpuID) { return _values.size()==1 ? _values[0] : _values[i]; }

 private:
  std::vector<Double_t> _valuesCPU; // CPU data
  //  std::vector<Double_t> _valuesGPU;
  
  ClassDef(RooValues,1)
};

#endif
