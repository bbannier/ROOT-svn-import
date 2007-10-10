// @(#)root/mathmore:$Id$
// Author: Magdalena Slawinska  10/2007

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2007 ROOT Foundation,  CERN/PH-SFT                   *
  *                                                                    *
  *                                                                    *
  **********************************************************************/


// Integration types for
// one and multidimensional integration
// eith a common interface

#ifndef ROOT_Math_AllIntegrationTypes
#define ROOT_Math_AllIntegrationTypes



namespace ROOT {
namespace Math {



  namespace Integration {

  
    // type of integration

    //for 1-dim integration
    enum OneDimType{ADAPTIVE, ADAPTIVESINGULAR, NONADAPTIVE};

    //for multi-dim integration
    enum MultiDimType{ADAPTIVE, MONTECARLOVEGAS, MONTECARLOMISER, MONTECARLOPLAIN};

  }    // end namespace AllIntegration


} // namespace Math
} // namespace ROOT

#endif /* ROOT_Math_AllIntegrationTypes */
