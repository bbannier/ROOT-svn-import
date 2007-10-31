// Author: Roel Aaij 15/08/2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TClass.h"
#include "Riostream.h"
#include "TSystem.h"
#include "TEnv.h"
#include "TVirtualTableInterface.h"
#include "TGResourcePool.h"
#include "TError.h"

ClassImp(TVirtualTableInterface)

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TVirtualTableInterface and TSimpleTableInterface.                    //
//                                                                      //
// This header defines a first design prototype for                     //
// TVirtualTableInterface and TSimpleTableInterface                     //
//                                                                      //
// TVirtualTableInterface is an abstract base class that defines all    //
// method that a table interface needs to supply in order to display    //
// data from a certain type of source in a TGTable. This prototype      //
// fucosses only on interfacing data from the data to the TGTable.      //
//                                                                      //
// See the documentation of TGTable, TGSimpleTableInterface and         //
// TTreeTableInterface for information on how to use an interface with  //
// a TGTable                                                            //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

//______________________________________________________________________________
TVirtualTableInterface::TVirtualTableInterface (UInt_t nrows, UInt_t ncolumns)   
   : fNRows(nrows), fNColumns(ncolumns)
{
   // Constructor
}

//______________________________________________________________________________
TVirtualTableInterface::~TVirtualTableInterface()
{ 
   // Destructor.
}

//______________________________________________________________________________
Double_t TVirtualTableInterface::GetValue(UInt_t row, UInt_t column)
{
   // Return the value of data at row,column as a double.

   return row * column;
}

//______________________________________________________________________________
const char *TVirtualTableInterface::GetValueAsString(UInt_t row, UInt_t column)
{
   // Return the string to use as a label for the cell at row,column. 

   Ssiz_t size = 1;
   TString str(size);
   str += GetValue(column, row);
   return str.Data();
}

//______________________________________________________________________________
const char *TVirtualTableInterface::GetRowHeader(UInt_t row)
{
   // Return the string to use as a label for row header at row.

   TString str("DRow ");
   str += row;
   return str.Data();
}

//______________________________________________________________________________
const char *TVirtualTableInterface::GetColumnHeader(UInt_t column)
{
   // Return the string to use as a label for column header at column.

   TString str("DCol ");
   str += column;
   return str.Data();
}

//______________________________________________________________________________
UInt_t TVirtualTableInterface::GetNRows() 
{
   // Return the amount of rows in the data source.

   return fNRows;
}

//______________________________________________________________________________
UInt_t TVirtualTableInterface::GetNColumns()
{
   // Return the amount of columns in the data source.

   return fNColumns;
}
