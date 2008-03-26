// @(#)root/fit:$Id$
// Author: L. Moneta Wed Aug 30 11:15:23 2006

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class BinData

#ifndef ROOT_Fit_BinData
#define ROOT_Fit_BinData

#ifndef ROOT_Fit_DataVector
#include "Fit/DataVector.h"
#endif


#ifdef USE_BINPOINT_CLASS

#ifndef ROOT_Fit_BinPoint
#include "Fit/BinPoint.h"
#endif

#endif


namespace ROOT { 

   namespace Fit { 



//___________________________________________________________________________________
/** 
   BinData : class describing the binned data : 
              vectors of  x coordinates, y values and optionally error on y values and error on coordinates 
              The dimension of the coordinate is free
              There are 4 different options: 
              - only coordinates and values  (for binned likelihood fits)  : kNoError 
              - coordinate, values and error on  values (for normal least square fits)  : kValueError
              - coordinate, values, error on values and coordinates (for effective least square fits) : kCoordError
              - corrdinate, values, error on coordinates and asymmettric error on valyes : kAsymError

              In addition there is the option to construct Bindata copying the data in (using the DataVector class) 
              or using pointer to external data (DataWrapper) class. 
              In general is found to be more efficient to copy the data. 
              In case of really large data sets for limiting memory consumption then the other option can be used
              Specialized constructor exists for data up to 3 dimensions. 

              When the data are copying in the number of points can be set later (or re-set) using Initialize and 
              the data are pushed in (one by one) using the Add method. 

             @ingroup  FitData  
*/ 


class BinData  : public FitData  { 

public : 

   enum ErrorType { kNoError, kValueError, kCoordError, kAsymError };

   static unsigned int GetPointSize(ErrorType err, unsigned int dim) { 
      if (err == kNoError) return dim + 1;   // no errors
      if (err == kValueError) return dim + 2;  // error only on the value
      if (err == kCoordError) return 2 * dim + 2 ;  // error on value and coordinate
      return 2 * dim + 3;   // error on value (low and high)  and error on coordinate
    }

   ErrorType GetErrorType() const { 
      if (fPointSize == fDim + 1) return kNoError; 
      if (fPointSize == fDim + 2) return kValueError; 
      if (fPointSize == 2 * fDim + 2) return kCoordError; 
      assert( fPointSize == 2 * fDim + 3 ) ; 
      return kAsymError; 
   }

      

   /**
      constructor from dimension of point  and max number of points (to pre-allocate vector)
      Give a zero value and then use Initialize later one if the size is not known
    */

   explicit BinData(unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError); 

   /**
      constructor from option and default range
    */
   explicit BinData (const DataOptions & opt, unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError);

   /**
      constructor from options and range
      efault is 1D and value errors
    */
   BinData (const DataOptions & opt, const DataRange & range, unsigned int maxpoints = 0, unsigned int dim = 1, ErrorType err = kValueError ); 

   /** constructurs using external data */
   
   /**
      constructor from external data for 1D with errors on  coordinate and value
    */
   BinData(unsigned int n, const double * dataX, const double * val, const double * ex , const double * eval ); 
   
   /**
      constructor from external data for 2D with errors on  coordinate and value
    */
   BinData(unsigned int n, const double * dataX, const double * dataY, const double * val, const double * ex , const double * ey, const double * eval  ); 

   /**
      constructor from external data for 3D with errors on  coordinate and value
    */
   BinData(unsigned int n, const double * dataX, const double * dataY, const double * dataZ, const double * val, const double * ex , const double * ey , const double * ez , const double * eval   );

   /**
      copy constructor  
   */
   BinData(const BinData &);

   /** 
       assignment operator 
   */ 
   BinData & operator= (const BinData &);


   /**
      destructor
   */
   ~BinData(); 

   /**
      preallocate a data set given size and dimension
      need to be initialized with the with the right dimension before
    */
   void Initialize(unsigned int maxpoints, unsigned int dim = 1, ErrorType err = kValueError ); 


   /**
      return the size of a fit point (is the coordinate dimension + 1 for the value and eventually 
      the number of all errors
    */
   unsigned int PointSize() const { 
      return fPointSize; 
   }

   /**
      return the size of internal data  (number of fit points)
      if data are not copied in but used externally the size is 0
    */
   unsigned int DataSize() const { 
      if (fDataVector) return fDataVector->Size(); 
      return 0; 
   }

   /**
      flag to control if data provides error on the coordinates
    */
   bool HaveCoordErrors() const { 
      if (fPointSize > fDim +2) return true; 
      return false;
   }

   /**
      flag to control if data provides asymmetric errors on the value
    */
   bool HaveAsymErrors() const { 
      if (fPointSize > 2 * fDim +2) return true; 
      return false;
   }


   /**
      add one dim data with only coordinate and values
   */
   void Add(double x, double y ); 

   /**
      add one dim data with no error in the coordinate (x)
      in this case store the inverse of the error in the value (y)
   */
   void Add(double x, double y, double ey);

   /**
      add one dim data with  error in the coordinate (x)
      in this case store the value (y)  error and not the inverse 
   */
   void Add(double x, double y, double ex, double ey);

   /**
      add one dim data with  error in the coordinate (x) and asymmetric errors in the value (y)
      in this case store the y errors and not the inverse 
   */
   void Add(double x, double y, double ex, double eyl , double eyh);

   /**
      add multi-dim coordinate data with only value (no errors)
   */
   void Add(const double *x, double val); 

   /**
      add multi-dim coordinate data with only error in value 
   */
   void Add(const double *x, double val, double  eval); 

   /**
      add multi-dim coordinate data with both error in coordinates and value 
   */
   void Add(const double *x, double val, const double * ex, double  eval); 

   /**
      return a pointer to the coordinates data for the given fit point 
    */
   const double * Coords(unsigned int ipoint) const { 
      if (fDataVector) 
         return &((fDataVector->Data())[ ipoint*fPointSize ] );
      
      return fDataWrapper->Coords(ipoint);
   }

   /**
      return the value for the given fit point
    */
   double Value(unsigned int ipoint) const { 
      if (fDataVector)       
         return (fDataVector->Data())[ ipoint*fPointSize + fDim ];
     
      return fDataWrapper->Value(ipoint);
   }


   /**
      return error on the value for the given fit point
      Safe (but slower) method returning correctly the error on the value 
      in case of asymm errors return the average 0.5(eu + el)
    */ 
   double Error(unsigned int ipoint) const { 
      if (fDataVector) { 
         ErrorType type = GetErrorType(); 
         if (type == kNoError ) return 1; 
         // error on the value is the last element in the point structure
         double eval =  (fDataVector->Data())[ (ipoint+1)*fPointSize - 1];
         if (type == kValueError ) // need to invert (inverror is stored) 
            return eval != 0 ? 1.0/eval : 0; 
         else if (type == kAsymError) {  // return 1/2(el + eh) 
            double el = (fDataVector->Data())[ (ipoint+1)*fPointSize - 2];
            return 0.5 * (el+eval); 
         }
         return eval; // case of coord errors
      }

      return fDataWrapper->Error(ipoint);
   } 

   /**
      Return the inverse of error on the value for the given fit point
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


   /**
      Return a pointer to the errors in the coordinates for the given fit point
    */
   const double * CoordErrors(unsigned int ipoint) const {
      if (fDataVector) { 
         // error on the value is the last element in the point structure
         return  &(fDataVector->Data())[ (ipoint)*fPointSize + fDim + 1];
      }

      return fDataWrapper->CoordErrors(ipoint);
   }

   /**
      retrieve at the same time a  pointer to the coordinate data and the fit value
      More efficient than calling Coords(i) and Value(i)
    */
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

   /**
      retrieve in a single call a pointer to the coordinate data, value and inverse error for 
      the given fit point. 
      To be used only when type is kValueError otherwise the error is returned and not the inverse
   */
   const double * GetPoint(unsigned int ipoint, double & value, double & invError) const {
      if (fDataVector) { 
         assert(fPointSize == fDim +2); // value error
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

   /**
      Retrieve the errors on the point (coordinate and value) for the given fit point
      It must be called only when the coordinate errors are stored otherwise it will produce an 
      assert.
   */
   const double * GetPointError(unsigned int ipoint, double & errvalue) const {
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

   /**
      Get errors on the point (coordinate errors and asymmetric value errors) for the 
      given fit point. 
      It must be called only when the coordinate errors and asymmetric errors are stored 
      otherwise it will produce an assert.
   */
   const double * GetPointError(unsigned int ipoint, double & errlow, double & errhigh) const {
      // external data is not supported for asymmetric errors
      assert(fDataVector); 

      assert(fPointSize > 2 * fDim + 2); 
      unsigned int j = ipoint*fPointSize;
      const std::vector<double> & v = (fDataVector->Data());
      const double * ex = &v[j+fDim+1];
      errlow  = v[j + 2*fDim +1];
      errhigh = v[j + 2*fDim +2];
      return ex;
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

   /**
      return number of fit points
    */
   unsigned int NPoints() const { return fNPoints; } 

   /**
      return number of fit points 
      In case of integral option size is npoints -1 
    */ 
   unsigned int Size() const { 
      return (Opt().fIntegral) ? fNPoints-1 : fNPoints; 
   }

   /**
      return coordinate data dimension
    */
   unsigned int NDim() const { return fDim; } 

   /**
      apply a Log transformation of the data values 
      can be used for example when fitting an exponential or gaussian
      Transform the data in place need to copy if want to preserve original data
      The data sets must not contain negative values. IN case it does, 
      an empty data set is returned
    */
   BinData & LogTransform();

protected: 

   void SetNPoints(unsigned int n) { fNPoints = n; }

private: 


   unsigned int fDim;       // coordinate dimension
   unsigned int fPointSize; // total point size including value and errors (= fDim + 2 for error in only Y ) 
   unsigned int fNPoints;   // number of contained points in the data set (can be different than size of vector)

   DataVector * fDataVector;  // pointer to the copied in data vector
   DataWrapper * fDataWrapper;  // pointer to the external data wrapper structure

#ifdef USE_BINPOINT_CLASS
   mutable BinPoint fPoint; 
#endif

}; 

  
   } // end namespace Fit

} // end namespace ROOT



#endif /* ROOT_Fit_BinData */


