// Author: Roel Aaij   21/07/2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TVirtualTableInterface
#define ROOT_TVirtualTableInterface

// class TString;
// class TArrayD;

class TVirtualTableInterface {

private:

protected:
   UInt_t       fNRows;    // Number of rows in the data
   UInt_t       fNColumns; // Number of columns in the data

public:
   TVirtualTableInterface(UInt_t nrows, UInt_t ncols);
   virtual ~TVirtualTableInterface();

   virtual Double_t    GetValue(UInt_t row, UInt_t column);
   virtual const char *GetValueAsString(UInt_t row, UInt_t column);
   virtual const char *GetRowHeader(UInt_t row);
   virtual const char *GetColumnHeader(UInt_t column); 
   virtual UInt_t      GetNRows(); 
   virtual UInt_t      GetNColumns();

   ClassDef(TVirtualTableInterface, 0)
};

#endif
