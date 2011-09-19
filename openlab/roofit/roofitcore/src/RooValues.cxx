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

#include "RooFit.h"
#include "RooValues.h"

//dummy class    

ClassImp(RooValues)
;

RooValues::RooValues(Double_t& value) :
  _value(value)
{
}

RooValues::RooValues(const RooValues& other) :
  _valuesCPU(other._valuesCPU),
  _value(other._value)
{

}

