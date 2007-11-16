// @(#)root/fit:$Id: inc/Fit/DataVector.h,v 1.0 2006/01/01 12:00:00 moneta Exp $
// Author: L. Moneta Wed Aug 30 11:15:23 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class DataVector

#ifndef ROOT_Fit_DataVector
#define ROOT_Fit_DataVector

#ifndef ROOT_Fit_DataVector
#include "Fit/DataVectorfwd.h"
#endif

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
   class holding the fit data points. It is template on the type of point,
   which can be for example a binned or unbinned point. 
   It is basicaly a wrapper on an std::vector  

   @ingroup FitData

*/ 
//template < class Point > 
class DataVector {

public: 

//   typedef std::vector<Point>      FitData;
   typedef std::vector<double>      FitData;
   typedef  FitData::const_iterator const_iterator;
   typedef  FitData::iterator iterator;

   /** 
      default constructor for a vector of N -data
   */ 
   explicit DataVector (unsigned int n = 0) : 
      fData(std::vector<double>(n))
   {
      //if (n!=0) fData.reserve(n); 
   } 

   /**
      constructor from option and default range
    */
   explicit DataVector (const DataOptions & opt, unsigned int n = 0) : 
      fData(std::vector<double>(n)),
      fOptions(opt)
   {
      //if (n!=0) fData.reserve(n); 
   } 

   /**
      constructor from options and range
    */
   DataVector (const DataOptions & opt, const DataRange & range, unsigned int n = 0) : 
      fData(std::vector<double>(n)),
      fOptions(opt), 
      fRange(range)
   {
      //if (n!=0) fData.reserve(n); 
   } 


   /**
      Create from a compatible DataPoint iterator
    */
//    template<class Iterator> 
//    DataVector (Iterator begin, Iterator end, const DataOptions & opt, const DataRange & range) : 
//       fOptions(opt), 
//       fRange(range)
//    {
//       for (Iterator itr = begin; itr != end; ++itr) 
//          if (itr->IsInRange(range) )
//             Add(*itr); 
//    } 



   /** 
      Destructor (no operations)
   */ 
   ~DataVector ()  {}  

   // use default copy constructor and destructor
   

   /**
      const access to underlying vector 
    */
   const std::vector<double> & Data() const { return fData; }

   /**
      non-const access to underlying vector (in case of insertion/deletion) and iterator
    */
   std::vector<double> & Data()  { return fData; }

   /**
      const iterator access 
   */ 
   const_iterator begin() const { return fData.begin(); }
   const_iterator end() const { return fData.begin()+fData.size(); }

   /**
      non-const iterator access 
   */ 
   iterator begin() { return fData.begin(); }
   iterator end()   { return fData.begin()+fData.size(); }

   /**
      access to the point
    */ 
   const double & operator[] (unsigned int i)  const { return fData[i]; } 
   double & operator[] (unsigned int i)   { return fData[i]; } 

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
      full size of data vector (npoints * point size) 
    */
   unsigned int DataSize() const { return fData.size(); } 


private: 

      std::vector<double> fData; 
      DataOptions fOptions; 
      DataRange   fRange; 
}; 


//       // usefule typedef's of DataVector
//       class BinPoint;
      
//       // declaration for various type of data vectors
//       typedef DataVector<ROOT::Fit::BinPoint>                    BinData; 
//       typedef DataVector<ROOT::Fit::BinPoint>::const_iterator    BinDataIterator; 




class BinData : public DataVector { 

public : 

   /**
      constructor from dimension of point  and max number of points (to pre-allocate vector)
    */

   BinData(unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
      DataVector( (dim+2)*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   { 
   } 

   /**
      constructor from option and default range
    */
   explicit BinData (const DataOptions & opt, unsigned int dim = 1, unsigned int maxpoints = 0) : 
      DataVector( opt, (dim+2)*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   {
   } 

   /**
      constructor from options and range
    */
   BinData (const DataOptions & opt, const DataRange & range, unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
      DataVector( opt, range, (dim+2)*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   {
   } 


   /**
      Create from a compatible BinData set
    */
   
   BinData (const BinData & data , const DataOptions & opt, const DataRange & range) : 
      DataVector(opt,range, data.DataSize() ), 
      fDim(data.fDim),
      fNPoints(data.fNPoints) 
   {
//       for (Iterator itr = begin; itr != end; ++itr) 
//          if (itr->IsInRange(range) )
//             Add(*itr); 
   } 

   /**
      preallocate a data set given size and dimension
    */
   void Initialize(unsigned int maxpoints, unsigned int dim = 1) { 
      fDim = dim; 
      Data().resize( maxpoints * PointSize() );
   }

      
   unsigned int PointSize() const { 
      return fDim + 2; 
   }
   
      /**
         add one dim data
      */
   void Add(double x, double y, double ey) { 
      int index = fNPoints*PointSize(); 
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &(Data()[ index ]);
      *itr++ = x; 
      *itr++ = y; 
      *itr++ =  (ey!= 0) ? 1.0/ey : 0; 

      fNPoints++;
   }
   //for multi dim data
   void Add(double *x, double y, double ey) { 
      int index = fNPoints*PointSize(); 

      if (index + PointSize() > DataSize()) 
         std::cout << "Error - index is " << index << " point size is " << PointSize()  << "  allocated size = " << DataSize() << std::endl;
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &(Data()[ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 

      *itr++ = y; 
      *itr++ = (ey!= 0) ? 1.0/ey : 0; 

      fNPoints++;
   }

   const double * Coords(unsigned int ipoint) const { 
      return &(Data()[ ipoint*PointSize() ] );
   }

   double Value(unsigned int ipoint) const { 
      return Data()[ ipoint*PointSize() + fDim ];
   }

   double Error(unsigned int ipoint) const { 
      double invError = InvError(ipoint); 
      return invError != 0 ? 1.0/invError : 0; 
   } 

   /**
      return the inverse of error on the value 
    */
   double InvError(unsigned int ipoint) const {
      return Data()[ ipoint*PointSize() + fDim + 1 ];
   }


   /**
      resize the vector to the given npoints 
    */
   void Resize (unsigned int npoints) { 
      fNPoints = npoints; 
      Data().resize(PointSize() *npoints);
   }


   unsigned int NPoints() const { return fNPoints; } 

   /**
      return number of contained points 
      in case of integral option size is npoints -1 
    */ 
   unsigned int Size() const { 
      return (Opt().fIntegral) ? fNPoints-1 : fNPoints; 
   }


private: 


   unsigned int fDim; 
   unsigned int fNPoints; 

}; 


class UnBinData : public DataVector { 

public : 

   /**
      constructor from dimension of point  and max number of points (to pre-allocate vector)
    */

   UnBinData(unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
      DataVector( dim*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   { 
   } 

   /**
      constructor from option and default range
    */
   explicit UnBinData (const DataOptions & opt,  unsigned int maxpoints = 0, unsigned int dim = 1) : 
      DataVector( opt, dim*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   {
   } 

   /**
      constructor from options and range
    */
   UnBinData (const DataOptions & opt, const DataRange & range,  unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
      DataVector( opt, range, dim*maxpoints ), 
      fDim(dim),
      fNPoints(0)
   {
   } 


   /**
      Create from a compatible BinData set
    */
   
   UnBinData (const UnBinData & data , const DataOptions & opt, const DataRange & range) : 
      DataVector(opt,range, data.DataSize() ), 
      fDim(data.fDim),
      fNPoints(data.fNPoints) 
   {
//       for (Iterator itr = begin; itr != end; ++itr) 
//          if (itr->IsInRange(range) )
//             Add(*itr); 
   } 

   /**
      preallocate a data set given size and dimension
    */
   void Initialize(unsigned int maxpoints, unsigned int dim = 1) { 
      fDim = dim; 
      Data().resize( maxpoints * PointSize() );
   }

      
   unsigned int PointSize() const { 
      return fDim; 
   }
   
      /**
         add one dim data
      */
   void Add(double x) { 
      int index = fNPoints*PointSize(); 
      assert (index + PointSize() <= DataSize() ); 

      Data()[ index ] = x;

      fNPoints++;
   }
   //for multi dim data
   void Add(double *x) { 
      int index = fNPoints*PointSize(); 

      assert (index + PointSize() <= DataSize() ); 

      double * itr = &(Data()[ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 

      fNPoints++;
   }

   const double * Coords(unsigned int ipoint) const { 
      return &(Data()[ ipoint*PointSize() ] );
   }


   /**
      resize the vector to the given npoints 
    */
   void Resize (unsigned int npoints) { 
      fNPoints = npoints; 
      Data().resize(PointSize() *npoints);
   }


   unsigned int NPoints() const { return fNPoints; } 

   /**
      return number of contained points 
    */ 
   unsigned int Size() const { return fNPoints; }


private: 


   unsigned int fDim; 
   unsigned int fNPoints; 

}; 
  
   } // end namespace Fit

} // end namespace ROOT

#

#endif /* ROOT_Fit_DataVector */
