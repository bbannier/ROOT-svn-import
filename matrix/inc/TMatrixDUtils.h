// @(#)root/matrix:$Name:  $:$Id: TMatrixDUtils.h,v 1.12 2002/10/25 11:19:02 rdm Exp $
// Author: Fons Rademakers   03/11/97

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TMatrixDUtils
#define ROOT_TMatrixDUtils


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// Matrix utility classes.                                              //
//                                                                      //
// This file defines utility classes for the Linear Algebra Package.    //
// The following classes are defined here:                              //
//   TElementActionD                                                    //
//   TElementPosActionD                                                 //
//   TLazyMatrixD                                                       //
//   THaarMatrixD                                                       //
//   THilbertMatrixD                                                    //
//   TMatrixDRow                                                        //
//   TMatrixDColumn                                                     //
//   TMatrixDDiag                                                       //
//   TMatrixDFlat                                                       //
//   TMatrixDPivoting                                                   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TMatrixD
#include "TMatrixD.h"
#endif


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TElementActionD                                                      //
//                                                                      //
// A class to do a specific operation on every vector or matrix element //
// (regardless of it position) as the object is being traversed.        //
// This is an abstract class. Derived classes need to implement the     //
// action function Operation().                                         //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TElementActionD {

friend class TMatrixD;
friend class TVectorD;

private:
   virtual void Operation(Double_t &element) const = 0;
   void operator=(const TElementActionD &) { }
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TElementPosActionD                                                   //
//                                                                      //
// A class to do a specific operation on every vector or matrix element //
// as the object is being traversed. This is an abstract class.         //
// Derived classes need to implement the action function Operation().   //
// In the action function the location of the current element is        //
// known (fI=row, fJ=columns).                                          //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TElementPosActionD {

friend class TMatrixD;
friend class TVectorD;

protected:
   mutable Int_t fI;        // i position of element being passed to Operation()
   mutable Int_t fJ;        // j position of element being passed to Operation()

private:
   virtual void Operation(Double_t &element) const = 0;
   void operator=(const TElementPosActionD &) { }
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TLazyMatrixD                                                         //
//                                                                      //
// Class used to make a lazy copy of a matrix, i.e. only copy matrix    //
// when really needed (when accessed).                                  //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TLazyMatrixD : public TObject {

friend class TMatrixD;

protected:
   Int_t fRowUpb;
   Int_t fRowLwb;
   Int_t fColUpb;
   Int_t fColLwb;

private:
   virtual void FillIn(TMatrixD &m) const = 0;

   TLazyMatrixD(const TLazyMatrixD &) : TObject() { }
   void operator=(const TLazyMatrixD &) { }

public:
   TLazyMatrixD() { fRowUpb = fRowLwb = fColUpb = fColLwb = 0; }
   TLazyMatrixD(Int_t nrows, Int_t ncols)
      : fRowUpb(nrows-1), fRowLwb(0), fColUpb(ncols-1), fColLwb(0) { }
   TLazyMatrixD(Int_t row_lwb, Int_t row_upb, Int_t col_lwb, Int_t col_upb)
      : fRowUpb(row_upb), fRowLwb(row_lwb), fColUpb(col_upb), fColLwb(col_lwb) { }

   ClassDef(TLazyMatrixD,1)  // Lazy matrix with double precision
};


class THaarMatrixD : public TLazyMatrixD {

private:
   void FillIn(TMatrixD &m) const;

public:
   THaarMatrixD(Int_t n, Int_t no_cols = 0);
};

class THilbertMatrixD : public TLazyMatrixD {

private:
   void FillIn(TMatrixD &m) const;

public:
   THilbertMatrixD(Int_t no_rows, Int_t no_cols);
   THilbertMatrixD(Int_t row_lwb, Int_t row_upb, Int_t col_lwb, Int_t col_upb);
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMatrixDRow                                                          //
//                                                                      //
// Class represents a row of a TMatrixD.                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TMatrixDRow : public TObject {

friend class TMatrixD;
friend class TVectorD;

private:
   const TMatrixD *fMatrix;  //! the matrix I am a row of
   Int_t           fRowInd;  // effective row index
   Int_t           fInc;     // if ptr = @a[row,i], then ptr+inc = @a[row,i+1]
   Double_t       *fPtr;     //! pointer to the a[row,0]

   static Double_t fgErr;      // used to return as reference in case of error

   TMatrixDRow() { fMatrix = 0; fInc = 0; fPtr = 0; }

public:
   TMatrixDRow(const TMatrixD &matrix, Int_t row);

   void operator=(Double_t val);
   void operator+=(const TMatrixDRow &r);
   void operator*=(const TMatrixDRow &r);
   void operator+=(Double_t val);
   void operator*=(Double_t val);

   void operator=(const TMatrixDRow &r);
   void operator=(const TVectorD &vec);

   const Double_t &operator()(Int_t i) const;
   Double_t &operator()(Int_t i);
   const Double_t &operator[](Int_t i) const;
   Double_t &operator[](Int_t i);

   ClassDef(TMatrixDRow,1)  // One row of a matrix (double precision)
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMatrixDColumn                                                       //
//                                                                      //
// Class represents a column of a TMatrixD.                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TMatrixDColumn : public TObject {

friend class TMatrixD;
friend class TVectorD;

private:
   const TMatrixD *fMatrix;         //! the matrix I am a column of
   Int_t           fColInd;         // effective column index
   Double_t       *fPtr;            //! pointer to the a[0,i] column

   static Double_t fgErr;      // used to return as reference in case of error

   TMatrixDColumn() { fMatrix = 0; fPtr = 0; }

public:
   TMatrixDColumn(const TMatrixD &matrix, Int_t col);

   void operator=(Double_t val);
   void operator+=(const TMatrixDColumn &c);
   void operator*=(const TMatrixDColumn &c);
   void operator+=(Double_t val);
   void operator*=(Double_t val);

   void operator=(const TMatrixDColumn &c);
   void operator=(const TVectorD &vec);

   const Double_t &operator()(Int_t i) const;
   Double_t &operator()(Int_t i);
   const Double_t &operator[](Int_t i) const;
   Double_t &operator[](Int_t i);

   ClassDef(TMatrixDColumn,1)  // One column of a matrix (double precision)
};


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMatrixDDiag                                                         //
//                                                                      //
// Class represents the diagonal of a matrix (for easy manipulation).   //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TMatrixDDiag : public TObject {

friend class TMatrixD;
friend class TVectorD;

private:
   const TMatrixD *fMatrix;  //! the matrix I am the diagonal of
   Int_t           fInc;     // if ptr=@a[i,i], then ptr+inc = @a[i+1,i+1]
   Int_t           fNdiag;   // number of diag elems, min(nrows,ncols)
   Double_t       *fPtr;     //! pointer to the a[0,0]

   static Double_t fgErr;      // used to return as reference in case of error

   TMatrixDDiag() { fMatrix = 0; fInc = 0; fNdiag = 0; fPtr = 0; }

public:
   TMatrixDDiag(const TMatrixD &matrix);

   void operator=(Double_t val);
   void operator+=(const TMatrixDDiag &d);
   void operator*=(const TMatrixDDiag &d);
   void operator+=(Double_t val);
   void operator*=(Double_t val);

   void operator=(const TMatrixDDiag &d);
   void operator=(const TVectorD &vec);

   const Double_t &operator()(Int_t i) const;
   Double_t &operator()(Int_t i);
   const Double_t &operator[](Int_t i) const;
   Double_t &operator[](Int_t i);
   Int_t  GetNdiags() const { return fNdiag; }

   ClassDef(TMatrixDDiag,1)  // Diagonal of a matrix (double  precision)
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMatrixDFlat                                                         //
//                                                                      //
// Class represents a flat matrix (for easy manipulation).              //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TMatrixDFlat : public TObject {

friend class TMatrixD;
friend class TVectorD;

private:
   const TMatrixD *fMatrix;  //! the matrix I am the diagonal of
   Double_t       *fPtr;     //! pointer to the a[0,0]

   static Double_t fgErr;      // used to return as reference in case of error

   TMatrixDFlat() { fMatrix = 0; fPtr = 0; }

public:
   TMatrixDFlat(const TMatrixD &matrix);

   void operator=(Double_t val);
   void operator+=(const TMatrixDFlat &d);
   void operator*=(const TMatrixDFlat &d);
   void operator+=(Double_t val);
   void operator*=(Double_t val);

   void operator=(const TMatrixDFlat &d);
   void operator=(const TVectorD &vec);

   const Double_t &operator()(Int_t i) const;
   Double_t &operator()(Int_t i);
   const Double_t &operator[](Int_t i) const;
   Double_t &operator[](Int_t i);

   ClassDef(TMatrixDFlat,1)  // Flat representation of a matrix
};

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TMatrixDPivoting                                                     //
//                                                                      //
// This class inherits from TMatrixD and it keeps additional            //
// information about what is being/has been pivoted.                    //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

class TMatrixDPivoting : public TMatrixD {

private:
   typedef Double_t* Index_t;           // wanted to have typeof(index[0])
   Index_t *const fRowIndex;            // fRowIndex[i] = ptr to the i-th
                                        // matrix row, or 0 if the row
                                        // has been pivoted. Note,
                                        // pivoted columns are marked
                                        // by setting fIndex[j] to zero.

                                // Information about the pivot that was
                                // just picked up
   Double_t fPivotValue;                // Value of the pivoting element
   Index_t  fPivotRow;                  // pivot's location (ptrs)
   Index_t  fPivotCol;
   Int_t    fPivotOdd;                  // parity of the pivot
                                        // (0 for even, 1 for odd)

   void PickUpPivot();                  // Pick up a pivot from
                                        // not-pivoted rows and cols

public:
   TMatrixDPivoting(const TMatrixD &m);
   ~TMatrixDPivoting();

   Double_t PivotingAndElimination();   // Perform the pivoting, return
                                        // the pivot value times (-1)^(pi+pj)
                                        // (pi,pj - pivot el row & col)
};

//----- inlines ----------------------------------------------------------------

#ifndef __CINT__

inline TMatrixDRow::TMatrixDRow(const TMatrixD &matrix, Int_t row)
       : fMatrix(&matrix), fInc(matrix.fNrows)
{
   if (!matrix.IsValid()) {
      Error("TMatrixDRow", "matrix is not initialized");
      return;
   }

   fRowInd = row - matrix.fRowLwb;

   if (fRowInd >= matrix.fNrows || fRowInd < 0) {
      Error("TMatrixDRow", "row #%d is not within the matrix", row);
      return;
   }

   fPtr = &(matrix.fIndex[0][fRowInd]);
}

inline void TMatrixDRow::operator=(const TMatrixDRow &mr)
{
   if (fMatrix != mr.fMatrix && AreCompatible(*fMatrix,*mr.fMatrix)) {
      Double_t *rp1 = fPtr;
      Double_t *rp2 = mr.fPtr;
      for ( ; rp1 < fPtr + fMatrix->fNelems; rp1 += fInc,rp2 += fInc)
         *rp1 = *rp2;
   }
}

inline const Double_t &TMatrixDRow::operator()(Int_t i) const
{
   // Get hold of the i-th row's element.

   fgErr = 0.0;

   if (!fMatrix->IsValid()) {
      Error("operator()", "matrix is not initialized");
      return fgErr;
   }

   Int_t acoln = i-fMatrix->fColLwb;           // Effective index

   if (acoln >= fMatrix->fNcols || acoln < 0) {
      Error("operator()", "TMatrixDRow index %d is out of row boundaries [%d,%d]",
            i, fMatrix->fColLwb, fMatrix->fNcols+fMatrix->fColLwb-1);
      return fgErr;
   }

   return fMatrix->fIndex[acoln][fPtr-fMatrix->fElements];
}

inline Double_t &TMatrixDRow::operator()(Int_t i)
{
   return (Double_t&)((*(const TMatrixDRow *)this)(i));
}

inline const Double_t &TMatrixDRow::operator[](Int_t i) const
{
   return (Double_t&)((*(const TMatrixDRow *)this)(i));
}

inline Double_t &TMatrixDRow::operator[](Int_t i)
{
   return (Double_t&)((*(const TMatrixDRow *)this)(i));
}

inline TMatrixDColumn::TMatrixDColumn(const TMatrixD &matrix, Int_t col)
       : fMatrix(&matrix)
{
   if (!matrix.IsValid()) {
      Error("TMatrixDColumn", "matrix is not initialized");
      return;
   }

   fColInd = col - matrix.fColLwb;

   if (fColInd >= matrix.fNcols || fColInd < 0) {
      Error("TMatrixDColumn", "column #%d is not within the matrix", col);
      return;
   }

   fPtr = &(matrix.fIndex[fColInd][0]);
}

inline void TMatrixDColumn::operator=(const TMatrixDColumn &mc)
{
   if (fMatrix != mc.fMatrix && AreCompatible(*fMatrix,*mc.fMatrix)) {
      Double_t *cp1 = fPtr;
      Double_t *cp2 = mc.fPtr;
      while (cp1 < fPtr + fMatrix->fNrows)
         *cp1++ = *cp2++;
   }
}

inline const Double_t &TMatrixDColumn::operator()(Int_t i) const
{
   // Access the i-th element of the column

   fgErr = 0.0;

   if (!fMatrix->IsValid()) {
      Error("operator()", "matrix is not initialized");
      return fgErr;
   }

   Int_t arown = i-fMatrix->fRowLwb;           // Effective indices

   if (arown >= fMatrix->fNrows || arown < 0) {
      Error("operator()", "TMatrixDColumn index %d is out of column boundaries [%d,%d]",
            i, fMatrix->fRowLwb, fMatrix->fNrows+fMatrix->fRowLwb-1);
      return fgErr;
   }

   return fPtr[arown];
}

inline Double_t &TMatrixDColumn::operator()(Int_t i)
{
   return (Double_t&)((*(const TMatrixDColumn *)this)(i));
}

inline const Double_t &TMatrixDColumn::operator[](Int_t i) const
{
   return (Double_t&)((*(const TMatrixDColumn *)this)(i));
}

inline Double_t &TMatrixDColumn::operator[](Int_t i)
{
   return (Double_t&)((*(const TMatrixDColumn *)this)(i));
}

inline TMatrixDDiag::TMatrixDDiag(const TMatrixD &matrix)
       : fMatrix(&matrix), fInc(matrix.fNrows+1),
         fNdiag(TMath::Min(matrix.fNrows, matrix.fNcols))
{
   if (!matrix.IsValid()) {
      Error("TMatrixDDiag", "matrix is not initialized");
      return;
   }
   fPtr = &(matrix.fElements[0]);
}

inline void TMatrixDDiag::operator=(const TMatrixDDiag &md)
{
   if (fMatrix != md.fMatrix && AreCompatible(*fMatrix,*md.fMatrix)) {
      Double_t *dp1 = fPtr;
      Double_t *dp2 = md.fPtr;
      Int_t i;
      for (i = 0; i < fNdiag; i++, dp1 += fInc, dp2 += fInc)
         *dp1 = *dp2;
   }
}

inline const Double_t &TMatrixDDiag::operator()(Int_t i) const
{
   // Get hold of the i-th diag element (indexing always starts at 0,
   // regardless of matrix' col_lwb and row_lwb)

   fgErr = 0.0;

   if (!fMatrix->IsValid()) {
      Error("operator()", "matrix is not initialized");
      return fgErr;
   }

   if (i >= fNdiag || i < 0) {
      Error("TMatrixDDiag", "TMatrixDDiag index %d is out of diag boundaries [0,%d]",
            i, fNdiag-1);
      return fgErr;
   }

   return fMatrix->fIndex[i][i];
}

inline Double_t &TMatrixDDiag::operator()(Int_t i)
{
   return (Double_t&)((*(const TMatrixDDiag *)this)(i));
}

inline const Double_t &TMatrixDDiag::operator[](Int_t i) const
{
   return (Double_t&)((*(const TMatrixDDiag *)this)(i));
}

inline Double_t &TMatrixDDiag::operator[](Int_t i)
{
   return (Double_t&)((*(const TMatrixDDiag *)this)(i));
}

inline TMatrixDFlat::TMatrixDFlat(const TMatrixD &matrix)
       : fMatrix(&matrix)
{
   if (!matrix.IsValid()) {
      Error("TMatrixDFlat", "matrix is not initialized");
      return;
   }
   fPtr = &(matrix.fElements[0]);
}

inline void TMatrixDFlat::operator=(const TMatrixDFlat &mf)
{
   if (fMatrix != mf.fMatrix && AreCompatible(*fMatrix,*mf.fMatrix)) {
      Double_t *fp1 = fPtr;
      Double_t *fp2 = mf.fPtr;
      while (fp1 < fPtr+fMatrix->fNelems)
         *fp1++ = *fp2++;
   }
}

inline const Double_t &TMatrixDFlat::operator()(Int_t i) const
{
   // Get hold of the i-th element (indexing always starts at 0,
   // regardless of matrix' col_lwb and row_lwb)

   fgErr = 0.0;

   if (!fMatrix->IsValid()) {
      Error("operator()", "matrix is not initialized");
      return fgErr;
   }

   if (i >= fMatrix->fNelems || i < 0) {
      Error("TMatrixDFlat", "TMatrixDFlat index %d is out of boundaries [0,%d]",
            i, fMatrix->fNelems-1);
      return fgErr;
   }

   return fMatrix->fElements[i];
}

inline Double_t &TMatrixDFlat::operator()(Int_t i)
{
   return (Double_t&)((*(const TMatrixDFlat *)this)(i));
}

inline const Double_t &TMatrixDFlat::operator[](Int_t i) const
{
   return (Double_t&)((*(const TMatrixDFlat *)this)(i));
}

inline Double_t &TMatrixDFlat::operator[](Int_t i)
{
   return (Double_t&)((*(const TMatrixDFlat *)this)(i));
}

#endif

#endif
