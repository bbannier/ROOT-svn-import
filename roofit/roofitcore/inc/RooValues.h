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

template<class T>
class RooValues {
 public:
  RooValues(T& valueCPU) ;
  RooValues(const RooValues& other) ;
  ~RooValues() { } 
  inline Bool_t isVector() const { return _valuesCPU.size()>0; }

  inline Int_t getSizeCPU() const { return _valuesCPU.size(); }
  inline void resizeCPU(Int_t n) { _valuesCPU.resize(n); }
  inline void reserveCPU(Int_t n) { _valuesCPU.reserve(n); }
  inline void push_backCPU(T& value) { _valuesCPU.push_back(value); }
  inline void clearCPU() { _valuesCPU.clear(); }

  inline T operator[](Int_t i) const { return _valuesCPU[i]; }
  inline T& operator[](Int_t i) { return _valuesCPU[i]; }

 private:
  std::vector<T> _valuesCPU ;
  T& _valueCPU ;

  //  ClassDef(RooValues,0) ;
} ;

template<class T>
RooValues<T>::RooValues(T& valueCPU) : _valueCPU(valueCPU)
{
}

template<class T>
RooValues<T>::RooValues(const RooValues& other) : _valueCPU(other._valueCPU) 
{
  _valuesCPU = other._valuesCPU ;
}



#endif
