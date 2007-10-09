// @(#)root/mathmore:$Id$
// Author: Magdalena Slawinska  10/2007

 /**********************************************************************
  *                                                                    *
  * Copyright (c) 2007 ROOT Foundation,  CERN/PH-SFT                   *
  *                                                                    *
  * This library is free software; you can redistribute it and/or      *
  * modify it under the terms of the GNU General Public License        *
  * as published by the Free Software Foundation; either version 2     *
  * of the License, or (at your option) any later version.             *
  *                                                                    *
  * This library is distributed in the hope that it will be useful,    *
  * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
  * General Public License for more details.                           *
  *                                                                    *
  * You should have received a copy of the GNU General Public License  *
  * along with this library (see file COPYING); if not, write          *
  * to the Free Software Foundation, Inc., 59 Temple Place, Suite      *
  * 330, Boston, MA 02111-1307 USA, or contact the author.             *
  *                                                                    *
  **********************************************************************/


// Integration types for
// one and multidimensional integration
// eith a common interface

#ifndef ROOT_Math_AllIntegrationTypes
#define ROOT_Math_AllIntegrationTypes



namespace ROOT {
namespace Math {



  namespace AllIntegration {

  
    // type of integration

    //for 1-dim integration
    enum OneDimType{ADAPTIVE, ADAPTIVESINGULAR, NONADAPTIVE};

    //for multi-dim integration
    enum MultiDimType{ADAPTIVE, MONTECARLOVEGAS, MONTECARLOMISER, MONTECARLOPLAIN};

  }    // end namespace AllIntegration


} // namespace Math
} // namespace ROOT

#endif /* ROOT_Math_AllIntegrationTypes */
