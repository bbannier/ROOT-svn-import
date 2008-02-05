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

/** 
@defgroup FitData Fit Data Classes 

Classes for describing the input data for fitting

@ingroup Fit
*/


#ifndef ROOT_Fit_DataVector
#include "Fit/DataVectorfwd.h"
#endif

#ifndef ROOT_Fit_DataOptions
#include "Fit/DataOptions.h"
#endif


#ifndef ROOT_Fit_DataRange
#include "Fit/DataRange.h"
#endif

#ifdef USE_BINPOINT_CLASS

#ifndef ROOT_Fit_BinPoint
#include "Fit/BinPoint.h"
#endif

#endif

#include <vector>
#include <cassert>
#include <iostream> 

namespace ROOT { 

   namespace Fit { 

/**
   Base class for all the fit data

   @ingroup FitData
 */

class FitData { 
  
public: 

   /// construct with default option and data range 
   FitData() {}

   /// construct passing options and default data range 
   FitData(const DataOptions & opt) : 
      fOptions(opt)
   {}

   /// construct passing options and data range 
   FitData (const DataOptions & opt, const DataRange & range) : 
      fOptions(opt), 
      fRange(range)
   {}

   /**
      access to options
    */
   const DataOptions & Opt() const { return fOptions; }
   DataOptions & Opt() { return fOptions; }

   /**
      access to range
    */
   const DataRange & Range() const { return fRange; }


private: 

      DataOptions fOptions; 
      DataRange   fRange; 

};
      


/** 
   class holding the fit data points. It is template on the type of point,
   which can be for example a binned or unbinned point. 
   It is basicaly a wrapper on an std::vector  

   @ingroup FitData

*/ 

class DataVector {

public: 


   typedef std::vector<double>      FData;
   typedef  FData::const_iterator const_iterator;
   typedef  FData::iterator iterator;

   /** 
      default constructor for a vector of N -data
   */ 
   explicit DataVector (unsigned int n ) : 
      fData(std::vector<double>(n))
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
   iterator end()   { return fData.end(); }

   /**
      access to the point
    */ 
   const double & operator[] (unsigned int i)  const { return fData[i]; } 
   double & operator[] (unsigned int i)   { return fData[i]; } 


   /**
      full size of data vector (npoints * point size) 
    */
   unsigned int Size() const { return fData.size(); } 


private: 

      std::vector<double> fData; 
}; 


//       // usefule typedef's of DataVector
//       class BinPoint;
      
//       // declaration for various type of data vectors
//       typedef DataVector<ROOT::Fit::BinPoint>                    BinData; 
//       typedef DataVector<ROOT::Fit::BinPoint>::const_iterator    BinDataIterator; 

/**
   class maintaining a pointer to external data
   Using this class avoids copying the data when performing a fit

   @ingroup FitData
 */

class DataWrapper { 

public: 

   explicit DataWrapper(const double * dataX ) :
      fDim(1),
      fValues(0), 
      fErrors(0),
      fCoords(std::vector<const double * >(1) ),
      fX(std::vector<double>(1) )
   {
      fCoords[0] = dataX; 
   }


   DataWrapper(const double * dataX, const double * val, const double * eval , const double * ex ) :
      fDim(1),
      fValues(val), 
      fErrors(eval),
      fCoords(std::vector<const double * >(1) ),
      fErrCoords(std::vector<const double * >(1) ),
      fX(std::vector<double>(1) ),
      fErr(std::vector<double>(1) )
   {
      fCoords[0] = dataX; 
      fErrCoords[0] = ex; 
   }

   DataWrapper(const double * dataX, const double * dataY, const double * val, const double * eval, const double * ex , const double * ey  ) : 
      fDim(2),
      fValues(val), 
      fErrors(eval),
      fCoords(std::vector<const double * >(2) ),
      fErrCoords(std::vector<const double * >(2) ),
      fX(std::vector<double>(2) ),
      fErr(std::vector<double>(2) )
   {
      fCoords[0] = dataX; 
      fCoords[1] = dataY; 
      fErrCoords[0] = ex; 
      fErrCoords[1] = ey; 
   }

   DataWrapper(const double * dataX, const double * dataY, const double * dataZ, const double * val, const double * eval, const double * ex , const double * ey, const double * ez  ) : 
      fDim(3),
      fValues(val), 
      fErrors(eval),
      fCoords(std::vector<const double * >(3) ),
      fErrCoords(std::vector<const double * >(3) ),
      fX(std::vector<double>(3) ),
      fErr(std::vector<double>(3) )
   {
      fCoords[0] = dataX; 
      fCoords[1] = dataY; 
      fCoords[2] = dataZ; 
      fErrCoords[0] = ex; 
      fErrCoords[1] = ey; 
      fErrCoords[2] = ez; 
   }


   const double * Coords(unsigned int ipoint) const { 
      for (unsigned int i = 0; i < fDim; ++i) { 
         const double * x = fCoords[i];
         fX[i] = x[ipoint];
      } 
      return &fX.front();
   }


   const double * CoordErrors(unsigned int ipoint) const { 
      for (unsigned int i = 0; i < fDim; ++i) { 
         const double * err = fErrCoords[i];
         fErr[i] = err[ipoint];
      } 
      return &fErr.front();
   }


   double Value(unsigned int ipoint) const { 
      return fValues[ipoint];
   }

   double Error(unsigned int ipoint) const { 
      return fErrors[ipoint];
   } 



private: 


   unsigned int fDim;
   const double * fValues; 
   const double * fErrors;   
   std::vector<const double *> fCoords;
   std::vector<const double *> fErrCoords;
   // cached vector to return x[] and errors on x
   mutable std::vector<double> fX;
   mutable std::vector<double> fErr;

};


/** 
   BinData : class describing the binned data (vectors of  x coordinates, y and error on y ) 
              but not error in X . For the Error in x one should use onother class

             @ingroup  FitData  
*/ 


class BinData  : public FitData  { 

public : 

   enum ErrorType { kNoError, kValueError, kCoordError };

   static unsigned int GetPointSize(ErrorType err, unsigned int dim) { 
      if (err == kNoError) return dim + 1;   // no errors
      if (err == kValueError) return dim + 2;  // error only on the value
      return 2 * (dim + 1);   // error on value and coordinate
    }

      

   /**
      constructor from dimension of point  and max number of points (to pre-allocate vector)
      Give a zero value and then use Initialize later one if the size is not known
    */

   explicit BinData(unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError) : 
//      DataVector( opt, GetPointSize(useErrorX,dim)*maxpoints ), 
      FitData(),
      fDim(dim),
      fPointSize(GetPointSize(err,dim) ),
      fNPoints(0),
      fDataVector(0),
      fDataWrapper(0)
   { 
      if (maxpoints > 0) fDataVector = new DataVector(fPointSize*maxpoints);
   } 

   /**
      constructor from option and default range
    */
   explicit BinData (const DataOptions & opt, unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError) : 
      // DataVector( opt, (dim+2)*maxpoints ), 
      FitData(opt),
      fDim(dim),
      fPointSize(GetPointSize(err,dim) ),
      fNPoints(0),
      fDataVector(0),
      fDataWrapper(0)
   { 
      if (maxpoints > 0) fDataVector = new DataVector(fPointSize*maxpoints);
   } 

   /**
      constructor from options and range
    */
   BinData (const DataOptions & opt, const DataRange & range, unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError ) : 
      //DataVector( opt, range, (dim+2)*maxpoints ), 
      FitData(opt,range),
      fDim(dim),
      fPointSize(GetPointSize(err,dim) ),
      fNPoints(0),
      fDataVector(0),
      fDataWrapper(0)
   { 
      if (maxpoints > 0) fDataVector = new DataVector(fPointSize*maxpoints);
   } 

   /** constructurs using external data */
   

   BinData(unsigned int n, const double * dataX, const double * val, const double * ex , const double * eval ) : 
      fDim(1), 
      fPointSize(0),
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper  = new DataWrapper(dataX, val, eval, ex);
   } 

   
   BinData(unsigned int n, const double * dataX, const double * dataY, const double * val, const double * ex , const double * ey, const double * eval  ) : 
      fDim(2), 
      fPointSize(0),
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper  = new DataWrapper(dataX, dataY, val, eval, ex, ey);
   } 

   BinData(unsigned int n, const double * dataX, const double * dataY, const double * dataZ, const double * val, const double * ex , const double * ey , const double * ez , const double * eval   ) : 
      fDim(3), 
      fPointSize(0),
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper  = new DataWrapper(dataX, dataY, dataZ, val, eval, ex, ey, ez);
   } 

private: 
   BinData(const BinData &) : FitData() {}
   BinData & operator= (const BinData &) { return *this; } 
public:  


//    /**
//       Create from a compatible BinData set
//     */
   
//    BinData (const BinData & data , const DataOptions & opt, const DataRange & range) : 
//       DataVector(opt,range, data.DataSize() ), 
//       fDim(data.fDim),
//       fNPoints(data.fNPoints) 
//    {
// //       for (Iterator itr = begin; itr != end; ++itr) 
// //          if (itr->IsInRange(range) )
// //             Add(*itr); 
//    } 

   ~BinData() {
      if (fDataVector) delete fDataVector; 
      if (fDataWrapper) delete fDataWrapper; 
   }

   /**
      preallocate a data set given size and dimension
      need to be initialized with the with the right dimension before
    */
   void Initialize(unsigned int maxpoints, unsigned int dim = 1, ErrorType err = kValueError ) { 
      if (fDataWrapper) delete fDataWrapper;
      fDataWrapper = 0; 
      fDim = dim; 
      fPointSize = GetPointSize(err,dim);  
      if (fDataVector) 
         (fDataVector->Data()).resize( maxpoints * fPointSize );
      else 
         fDataVector = new DataVector(fPointSize*maxpoints);
   }

      
   unsigned int PointSize() const { 
      return fPointSize; 
   }

   unsigned int DataSize() const { 
      if (fDataVector) return fDataVector->Size(); 
      return 0; 
   }

   bool UseCoordErrors() const { 
      if (fPointSize > fDim +2) return true; 
      return false;
   }
   
   /**
      add one dim data with no error in x
      in this case store the inverse of the error in y
   */
   void Add(double x, double y, double ey) { 
      int index = fNPoints*PointSize(); 
      //std::cout << "this = " << this << " index " << index << " fNPoints " << fNPoints << "  ds   " << DataSize() << std::endl; 
      assert (fDataVector != 0);
      assert (PointSize() == 3 ); 
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &((fDataVector->Data())[ index ]);
      *itr++ = x; 
      *itr++ = y; 
      *itr++ =  (ey!= 0) ? 1.0/ey : 0; 

      fNPoints++;
   }
   /**
      add one dim data with  error in x
      in this case store the y error and not the inverse 
   */
   void Add(double x, double y, double ex, double ey) { 
      int index = fNPoints*PointSize(); 
      assert (fDataVector != 0);
      assert (PointSize() == 4 ); 
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &((fDataVector->Data())[ index ]);
      *itr++ = x; 
      *itr++ = y; 
      *itr++ = ex; 
      *itr++ = ey; 

      fNPoints++;
   }


   /**
      add multi dim data with only value (no errors)
   */
   void Add(const double *x, double val) { 
      int index = fNPoints*PointSize(); 
      assert (fDataVector != 0);
      assert (PointSize() == fDim + 1 ); 

      if (index + PointSize() > DataSize()) 
         std::cout << "Error - index is " << index << " point size is " << PointSize()  << "  allocated size = " << DataSize() << std::endl;
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &((fDataVector->Data())[ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 
      *itr++ = val; 

      fNPoints++;
   }

   /**
      add multi dim data with only error in value 
   */
   void Add(const double *x, double val, double  eval) { 
      int index = fNPoints*PointSize(); 
      assert (fDataVector != 0);
      assert (PointSize() == fDim + 2 ); 

      if (index + PointSize() > DataSize()) 
         std::cout << "Error - index is " << index << " point size is " << PointSize()  << "  allocated size = " << DataSize() << std::endl;
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &((fDataVector->Data())[ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 
      *itr++ = val; 
      *itr++ =  (eval!= 0) ? 1.0/eval : 0; 

      fNPoints++;
   }


   /**
      add multi dim data with error in coordinates and value 
   */
   void Add(const double *x, double val, const double * ex, double  eval) { 
      int index = fNPoints*PointSize(); 
      assert (fDataVector != 0);
      assert (PointSize() == 2*fDim + 2 ); 

      if (index + PointSize() > DataSize()) 
         std::cout << "Error - index is " << index << " point size is " << PointSize()  << "  allocated size = " << DataSize() << std::endl;
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &((fDataVector->Data())[ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 
      *itr++ = val; 
      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = ex[i]; 
      *itr++ = eval; 

      fNPoints++;
   }

   const double * Coords(unsigned int ipoint) const { 
      if (fDataVector) 
         return &((fDataVector->Data())[ ipoint*fPointSize ] );
      
      return fDataWrapper->Coords(ipoint);
   }

   double Value(unsigned int ipoint) const { 
      if (fDataVector)       
         return (fDataVector->Data())[ ipoint*fPointSize + fDim ];
     
      return fDataWrapper->Value(ipoint);
   }

//#ifdef LATER
   /**
      return error on the value
    */ 
   double Error(unsigned int ipoint) const { 
      if (fDataVector) { 
         // error on the value is the last element in the point structure
         double eval =  (fDataVector->Data())[ (ipoint+1)*fPointSize - 1];
         //if (fWithCoordError) return eval; 
         // when error in the coordinate is not stored, need to invert it 
         return eval != 0 ? 1.0/eval : 0; 
      }

      return fDataWrapper->Error(ipoint);
   } 

   /**
      return the inverse of error on the value 
      useful when error in the coordinates are not stored and then this is used directly this as the weight in 
      the least square function
    */
   double InvError(unsigned int ipoint) const {
      if (fDataVector) { 
         // error on the value is the last element in the point structure
         double eval =  (fDataVector->Data())[ (ipoint+1)*fPointSize - 1];
         return eval; 
//          if (!fWithCoordError) return eval; 
//          // when error in the coordinate is stored, need to invert it 
//          return eval != 0 ? 1.0/eval : 0; 
      }
      //case data wrapper 

      double eval = fDataWrapper->Error(ipoint);
      return eval != 0 ? 1.0/eval : 0; 
   }
//#endif

   /**
      return a pointer to the errors in the coordinates
    */
   const double * CoordErrors(unsigned int ipoint) const {
      if (fDataVector) { 
         // error on the value is the last element in the point structure
         return  &(fDataVector->Data())[ (ipoint)*fPointSize + fDim + 1];
      }

      return fDataWrapper->CoordErrors(ipoint);
   }


   const double * GetPoint(unsigned int ipoint, double & value) const {
      if (fDataVector) { 
         unsigned int j = ipoint*fPointSize;
         const std::vector<double> & v = (fDataVector->Data());
         const double * x = &v[j];
         value = v[j+fDim];
         return x;
      } 
      value = fDataWrapper->Value(ipoint);
      return fDataWrapper->Coords(ipoint);
   }


   const double * GetPoint(unsigned int ipoint, double & value, double & invError) const {
      if (fDataVector) { 
         unsigned int j = ipoint*fPointSize;
         const std::vector<double> & v = (fDataVector->Data());
         const double * x = &v[j];
         value = v[j+fDim];
         invError = v[j+fDim+1];
         return x;
      } 
      value = fDataWrapper->Value(ipoint);
      double e = fDataWrapper->Error(ipoint);
      invError = ( e != 0 ) ? 1.0/e : 0; 
      return fDataWrapper->Coords(ipoint);
   }

   const double * GetPointError(unsigned int ipoint, double & errvalue) const {
// to be called only when coord errors are stored
      if (fDataVector) { 
         assert(fPointSize > fDim + 2); 
         unsigned int j = ipoint*fPointSize;
         const std::vector<double> & v = (fDataVector->Data());
         const double * ex = &v[j+fDim+1];
         errvalue = v[j + 2*fDim +1];
         return ex;
      } 
      errvalue = fDataWrapper->Error(ipoint);
      return fDataWrapper->CoordErrors(ipoint);
   }


#ifdef USE_BINPOINT_CLASS
   const BinPoint & GetPoint(unsigned int ipoint) const { 
      if (fDataVector) { 
         unsigned int j = ipoint*fPointSize;
         const std::vector<double> & v = (fDataVector->Data());
         const double * x = &v[j];
         double value = v[j+fDim];
         if (fPointSize > fDim + 2) {
            const double * ex = &v[j+fDim+1];
            double err = v[j + 2*fDim +1];
            fPoint.Set(x,value,ex,err);
         } 
         else {
            double invError = v[j+fDim+1];
            fPoint.Set(x,value,invError);
         }

      } 
      else { 
         double value = fDataWrapper->Value(ipoint);
         double e = fDataWrapper->Error(ipoint);
         if (fPointSize > fDim + 2) {
            fPoint.Set(fDataWrapper->Coords(ipoint), value, fDataWrapper->CoordErrors(ipoint), e);
         } else { 
            double invError = ( e != 0 ) ? 1.0/e : 0; 
            fPoint.Set(fDataWrapper->Coords(ipoint), value, invError);
         }
      }
      return fPoint; 
   }      


   const BinPoint & GetPointError(unsigned int ipoint) const { 
      if (fDataVector) { 
         unsigned int j = ipoint*fPointSize;
         const std::vector<double> & v = (fDataVector->Data());
         const double * x = &v[j];
         double value = v[j+fDim];
         double invError = v[j+fDim+1];
         fPoint.Set(x,value,invError);
      } 
      else { 
         double value = fDataWrapper->Value(ipoint);
         double e = fDataWrapper->Error(ipoint);
         double invError = ( e != 0 ) ? 1.0/e : 0; 
         fPoint.Set(fDataWrapper->Coords(ipoint), value, invError);
      }
      return fPoint; 
   }      
#endif

   /**
      resize the vector to the given npoints 
    */
   void Resize (unsigned int npoints) { 
      fNPoints = npoints; 
      (fDataVector->Data()).resize(PointSize() *npoints);
   }


   unsigned int NPoints() const { return fNPoints; } 

   /**
      return number of contained points 
      in case of integral option size is npoints -1 
    */ 
   unsigned int Size() const { 
      return (Opt().fIntegral) ? fNPoints-1 : fNPoints; 
   }

protected: 

   void SetNPoints(unsigned int n) { fNPoints = n; }

private: 


   unsigned int fDim;       // coordinate dimension
   unsigned int fPointSize; // total point size including value and errors (= fDim + 2 for error in only Y ) 
   unsigned int fNPoints;   // number of contained points in the data set (can be different than size of vector)

   DataVector * fDataVector; 
   DataWrapper * fDataWrapper;

#ifdef USE_BINPOINT_CLASS
   mutable BinPoint fPoint; 
#endif

}; 



/** 
   UnBinData : class describing the unbinned data (juat a vector of  x coordinates) 

             @ingroup  FitData  
*/ 
class UnBinData : public FitData { 

public : 

   /**
      constructor from dimension of point  and max number of points (to pre-allocate vector)
    */

   explicit UnBinData(unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
//      DataVector( dim*maxpoints ), 
      FitData(),
      fDim(dim),
      fNPoints(0),
      fDataVector(0), 
      fDataWrapper(0)
   { 
      if (maxpoints > 0) fDataVector = new DataVector( dim * maxpoints);
   } 

   /**
      constructor from option and default range
    */
   explicit UnBinData (const DataOptions & opt,  unsigned int maxpoints = 0, unsigned int dim = 1) : 
      FitData( opt), 
      fDim(dim),
      fNPoints(0), 
      fDataVector(0), 
      fDataWrapper(0)
   {
      if (maxpoints > 0) fDataVector = new DataVector( dim * maxpoints);
   } 

   /**
      constructor from options and range
    */
   UnBinData (const DataOptions & opt, const DataRange & range,  unsigned int maxpoints = 0, unsigned int dim = 1 ) : 
      FitData( opt, range), 
      fDim(dim),
      fNPoints(0),
      fDataVector(0), 
      fDataWrapper(0)
   {
      if (maxpoints > 0) fDataVector = new DataVector( dim * maxpoints);
   } 

   /**
      constructor for 1D external data
    */
   UnBinData(unsigned int n, const double * dataX ) : 
      FitData( ), 
      fDim(1), 
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper = new DataWrapper(dataX);
   } 

   /**
      constructor for 2D external data
    */
   UnBinData(unsigned int n, const double * dataX, const double * dataY ) : 
      FitData( ), 
      fDim(2), 
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper = new DataWrapper(dataX, dataY, 0, 0, 0, 0);
   } 

   /**
      constructor for 3D external data
    */
   UnBinData(unsigned int n, const double * dataX, const double * dataY, const double * dataZ ) : 
      FitData( ), 
      fDim(3), 
      fNPoints(n),
      fDataVector(0)
   { 
      fDataWrapper = new DataWrapper(dataX, dataY, dataZ, 0, 0, 0, 0, 0);
   } 


#ifdef LATER
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
#endif

   virtual ~UnBinData() {
      if (fDataVector) delete fDataVector; 
      if (fDataWrapper) delete fDataWrapper; 
   }

   /**
      preallocate a data set given size and dimension
    */
   void Initialize(unsigned int maxpoints, unsigned int dim = 1) { 
      fDim = dim; 
      assert(maxpoints > 0); 
      if (fDataVector) 
         (fDataVector->Data()).resize( maxpoints * PointSize() );
      else 
         fDataVector = new DataVector( dim * maxpoints);
   }

      
   unsigned int PointSize() const { 
      return fDim; 
   }

   unsigned int DataSize() const { 
      return fDataVector->Size();
   }
   
      /**
         add one dim data
      */
   void Add(double x) { 
      int index = fNPoints*PointSize(); 
      assert(fDataVector != 0);
      assert (index + PointSize() <= DataSize() ); 

      (fDataVector->Data())[ index ] = x;

      fNPoints++;
   }
   //for multi dim data

   void Add(double *x) { 
      int index = fNPoints*PointSize(); 

      assert(fDataVector != 0);
      assert (index + PointSize() <= DataSize() ); 

      double * itr = &( (fDataVector->Data()) [ index ]);

      for (unsigned int i = 0; i < fDim; ++i) 
         *itr++ = x[i]; 

      fNPoints++;
   }

   virtual   const double * Coords(unsigned int ipoint) const { 
      if (fDataVector) 
         return &( (fDataVector->Data()) [ ipoint*PointSize() ] );
      else 
         return fDataWrapper->Coords(ipoint); 
   }


   /**
      resize the vector to the given npoints 
    */
   void Resize (unsigned int npoints) { 
      if (fDataVector) {  
         fNPoints = npoints; 
         (fDataVector->Data()).resize(PointSize() *npoints);
      }
   }


   unsigned int NPoints() const { return fNPoints; } 

   /**
      return number of contained points 
    */ 
   unsigned int Size() const { return fNPoints; }



protected: 

   void SetNPoints(unsigned int n) { fNPoints = n; }

private: 

   unsigned int fDim; 
   unsigned int fNPoints; 
   
   DataVector * fDataVector; 
   DataWrapper * fDataWrapper; 

}; 

#ifdef LATER

/**
   class holding un-bin data but with external storage
 */
class UnBinDataExt : public UnBinData {

public: 

   UnBinDataExt(unsigned int n, const double * dataX ) : 
      UnBinData( 0,1 ), 
      fX(dataX), fY(0), fZ(0), 
      fCoord(std::vector<double>(1) )
   { 
      SetNPoints(n); 
   } 

   UnBinDataExt(unsigned int n, const double * dataX, const double * dataY ) : 
      UnBinData( 0,2 ), 
      fX(dataX), fY(dataY), fZ(0),
      fCoord(std::vector<double>(2) )
   { 
      SetNPoints(n); 
   } 
   UnBinDataExt(unsigned int n, const double * dataX, const double * dataY, const double * dataZ ) : 
      UnBinData( 0,3 ), 
      fX(dataX), fY(dataY), fZ(dataZ),
      fCoord(std::vector<double>(3) )
   { 
      SetNPoints(n); 
   } 


   virtual ~UnBinDataExt() {}


   virtual const double * Coords(unsigned int ipoint) const { 
      fCoord[0] = fX[ipoint]; 
      if (fY) fCoord[1] = fY[ipoint];
      if (fZ) fCoord[2] = fZ[ipoint];
      return &fCoord.front();
   }


private:

   const double * fX; 
   const double * fY; 
   const double * fZ; 

   mutable std::vector<double> fCoord;

};  
#endif
  
   } // end namespace Fit

} // end namespace ROOT



#endif /* ROOT_Fit_DataVector */
