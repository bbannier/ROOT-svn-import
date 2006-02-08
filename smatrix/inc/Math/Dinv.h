// @(#)root/smatrix:$Name:  $:$Id: Dinv.h,v 1.3 2005/12/07 16:44:05 moneta Exp $
// Authors: T. Glebe, L. Moneta    2005  

#ifndef  ROOT_Math_Dinv
#define  ROOT_Math_Dinv
// ********************************************************************
//
// source:
//
// type:      source code
//
// created:   03. Apr 2001
//
// author:    Thorsten Glebe
//            HERA-B Collaboration
//            Max-Planck-Institut fuer Kernphysik
//            Saupfercheckweg 1
//            69117 Heidelberg
//            Germany
//            E-mail: T.Glebe@mpi-hd.mpg.de
//
// Description: square Matrix inversion
//              Code was taken from CERNLIB::kernlib dfinv function, translated
//              from FORTRAN to C++ and optimized.
//              n:    Order of the square matrix
//              idim: First dimension of array A
//
// changes:
// 03 Apr 2001 (TG) creation
//
// ********************************************************************
#include "Math/Dfactir.h"
#include "Math/Dfinv.h"
#include "Math/Dsinv.h"


 
namespace ROOT { 

  namespace Math { 



/** Inverter.
    Class to specialize calls to Dinv. Dinv computes the inverse of a square
    matrix if dimension $idim$ and order $n$. The content of the matrix will be
    replaced by its inverse. In case the inversion fails, the matrix content is
    destroyed. Invert specializes Dinv by the matrix order. E.g. if the order
    of the matrix is two, the routine Inverter<2> is called which implements
    Cramers rule.

    @author T. Glebe
*/
//==============================================================================
// Inverter class
//==============================================================================
template <unsigned int idim, unsigned int n = idim>
class Inverter {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs) {

#ifdef XXX
      if (n < 1 || n > idim) {
	return false;
      }
#endif



      /* Initialized data */
      static unsigned int work[n];
      for(unsigned int i=0; i<n; ++i) work[i] = 0;

      static typename MatrixRep::value_type det = 0;

      /* Function Body */
      
      /*  N.GT.3 CASES.  FACTORIZE MATRIX AND INVERT. */
      if (Dfactir<MatrixRep,n,idim>(rhs,det,work) == false) {
	std::cerr << "Dfactir failed!!" << std::endl;
	return false;
      }
      return Dfinv<MatrixRep,n,idim>(rhs,work);
  } // Dinv


  // symmetric function (copy in a general one) 
  template <class T>
  static bool Dinv(MatRepSym<T,idim> & rhs) {
    // not very efficient but need to re-do Dsinv for new storage of 
    // symmetric matrices
    MatRepStd<T,idim>  tmp; 
    for (unsigned int i = 0; i< idim*idim; ++i) 
      tmp[i] = rhs[i];
    if (! Inverter<idim>::Dinv(tmp) ) return false;
    // recopy the data
    for (unsigned int i = 0; i< idim*n; ++i) 
      rhs[i] = tmp[i];

    return true; 
  }



}; // class Inverter


/** Inverter<0>.
    In case of zero order, do nothing.

    @author T. Glebe
*/
//==============================================================================
// Inverter<0>
//==============================================================================
template <>
class Inverter<0> {
public:
  ///
  template <class MatrixRep>
  inline static bool Dinv(MatrixRep& rhs) { return true; }
};
    

/** Inverter<1>.
    $1\times1$ (sub-)matrix. $a_{11} \to 1/a_{11}$

    @author T. Glebe
*/
//==============================================================================
// Inverter<1>
//==============================================================================
template <>
class Inverter<1> {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs) {
    
    if (rhs[0] == 0.) {
      return false;
    }
    rhs[0] = 1. / rhs[0];
    return true;
  }
};


/** Inverter<2>.
    $2\times2$ (sub-)matrix. Use Cramers rule.

    @author T. Glebe
*/
//==============================================================================
// Inverter<2>: Cramers rule
//==============================================================================

template <>
class Inverter<2> {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs) {

    typename MatrixRep::value_type det = rhs[0] * rhs[3] - rhs[2] * rhs[1];
    
    if (det == 0.) { return false; }

    typename MatrixRep::value_type s = 1. / det;
    typename MatrixRep::value_type c11 = s * rhs[3];


    rhs[2] = -s * rhs[2];
    rhs[1] = -s * rhs[1];
    rhs[3] =  s * rhs[0];
    rhs[0] = c11;


    return true;
  }

  // specialization for the symmetric matrices
  template <class T>
  static bool Dinv(MatRepSym<T,2> & rep) {
    
    T * rhs = rep.Array(); 

    T det = rhs[0] * rhs[2] - rhs[1] * rhs[1];

    
    if (det == 0.) { return false; }

    T s = 1. / det;
    T c11 = s * rhs[2];

    rhs[1] = -s * rhs[1];
    rhs[2] =  s * rhs[0];
    rhs[0] = c11;
    return true;
  }

};


/** Inverter<3>.
    $3\times3$ (sub-)matrix. Use pivotisation.

    @author T. Glebe
*/
//==============================================================================
// Inverter<3>
//==============================================================================

template <>
class Inverter<3> {
public:
  ///
  // use Cramer Rule
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs); 

  template <class T>
  static bool Dinv(MatRepSym<T,3> & rhs);

};

/** 
    Inverter<4> Using Cramers rule.
*/
template <>
class Inverter<4> {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs); 

  template <class T>
  static bool Dinv(MatRepSym<T,4> & rhs);

};

/** 
    Inverter<5> Using Cramers rule.
*/
template <>
class Inverter<5> {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs); 

  template <class T>
  static bool Dinv(MatRepSym<T,5> & rhs);

};

/** 
    Inverter<6> Using Cramers rule.
*/
template <>
class Inverter<6> {
public:
  ///
  template <class MatrixRep>
  static bool Dinv(MatrixRep& rhs); 

  template <class T>
  static bool Dinv(MatRepSym<T,6> & rhs);

};


  }  // namespace Math

}  // namespace ROOT
          

#include "CramerInversion.icc"
#include "CramerInversionSym.icc"

#endif  /* ROOT_Math_Dinv */
