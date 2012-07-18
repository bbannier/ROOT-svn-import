// @(#)root/mathcore:$Id: DataVector.h 45076 2012-07-16 13:45:18Z mborinsk $
// Author: L. Moneta Wed Aug 30 11:15:23 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class DataVector

#ifndef ROOT_Fit_FitData
#define ROOT_Fit_FitData

/** 
@defgroup FitData Fit Data Classes 

Classes for describing the input data for fitting

@ingroup Fit
*/


#ifndef ROOT_Fit_DataOptions
#include "Fit/DataOptions.h"
#endif


#ifndef ROOT_Fit_DataRange
#include "Fit/DataRange.h"
#endif


#include <vector>
#include <cassert>
#include <iostream> 


namespace ROOT { 

   namespace Fit { 


/**
 * Base class for all the fit data types:
 * Stores the coordinates and the DataOptions

   @ingroup FitData
 */


/** 
   class holding the fit data points. It is template on the type of point,
   which can be for example a binned or unbinned point. 
   It is basicaly a wrapper on an std::vector  

   @ingroup FitData

*/ 

class FitData 
{ 
public: 

  /// construct with default option and data range 
  FitData( unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
    fMaxPoints(maxpoints), 
    fCoords(dim), 
    fCoordsPtr(dim)
  {
    InitCoordsVector( );
  }

  /// dummy virtual destructor
  virtual ~FitData( ) {}

  /// construct passing options and default data range 
  FitData( const DataOptions & opt, unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
    fOptions(opt), 
    fMaxPoints(maxpoints), 
    fCoords(dim), 
    fCoordsPtr(dim)
  {
    InitCoordsVector( );
  }


  /// construct passing range and default options 
  FitData( const DataRange & range, unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
    fRange(range), 
    fMaxPoints(maxpoints), 
    fCoords(dim), 
    fCoordsPtr(dim)
  {
    InitCoordsVector( );
  }

  /// construct passing options and data range 
  FitData ( const DataOptions & opt, const DataRange & range, 
    unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
    fOptions(opt), 
    fRange(range), 
    fMaxPoints(maxpoints), 
    fCoords(dim), 
    fCoordsPtr(dim)
  {
    InitCoordsVector( );
  }

  /**
    access to options
  */
  const DataOptions & Opt() const { return fOptions; }
  DataOptions & Opt() { return fOptions; }

  /**
    access to range
  */
  const DataRange & Range() const { return fRange; }

  /** 
     define a max size to avoid allocating too large arrays 
  */
  static unsigned int MaxSize()  { 
    return (unsigned int) (-1) / sizeof (double);
  }


protected:
  void InitCoordsVector ( )
  {
    for( int i=0; i < fCoords.size(); i++ )
    {
      fCoords[i].resize( fMaxPoints );
      fCoordsPtr[i] = &fCoords[i].front();
    }
  }

private:

  DataOptions   fOptions; 
  DataRange     fRange;
  
  bool          fWrapped;
  unsigned int  fMaxPoints;
  
  /** 
   * This vector stores the vectorizable data:
   * The inner vectors contain the coordinates data
   * fCoords[0] is the vector for the x-coords
   * fCoords[1] is the vector for the y-coords
   * etc.
   * The vector of pointers stores the pointers 
   * to the first elements of the corresponding 
   * elements
   * 
   * If fWrapped is true, fCoords is empty. 
   * the data can only be accessed by using 
   * fCoordsPtr.
  */
  std::vector< std::vector< double > > fCoords; 
  std::vector< double* > fCoordsPtr;
};
      

   } // end namespace Fit

} // end namespace ROOT



#endif /* ROOT_Fit_Data */

