/******************************************************************************
 * Diego Marcos Segura                                                        *
 * Hasta que el pueblo las canta, las coplas, coplas no son                   *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 *****************************************************************************/
#ifndef ROOT_TClassInfo
#include "TClassInfo.h"
#endif

#include "TFunctionMemberInfo.h"

//______________________________________________________________________________
//                                                                      
// TFunctionMemberInfo
//                                                                       
// - This class defines the persistency information for methods (Function member)            
// - This class will be written in a root file                             
// - This class replace the old source code based dictionaries             
//                                                                       
//______________________________________________________________________________


//______________________________________________________________________________
TFunctionMemberInfo::TFunctionMemberInfo(const char *funcname, Char_t type, Char_t reftype, Char_t para_nu, Char_t access, Char_t ansi, Char_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t typeindex, const char* symbol){

   fSymbol = symbol;
   fName = funcname;
   fType = type;
   fReftype = reftype;
   fParaNum = para_nu;
   fAccess = access;
   fAnsi = ansi;
   fIsConst = isconst;
   fParameters = paras;
   fTitle = comment;
   fIsVirtual = isvirtual;
   fFullTypeName = typeindex;

}

//______________________________________________________________________________
Int_t  TFunctionMemberInfo::GetFullTypeName(){

   return fFullTypeName;

}

//______________________________________________________________________________
const TString&  TFunctionMemberInfo::GetComment(){

   return fTitle;

}


//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetType(){

   return fType;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetRefType(){

   return fReftype;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetConst(){

   return fParaNum;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetStaticType(){

   return fAnsi;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetAccess(){

   return fAccess;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetIsConst(){

   return fIsConst;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetIsVirtual(){

   return fIsVirtual;

}


//______________________________________________________________________________
const char *TFunctionMemberInfo::GetSymbol() const{

   return fSymbol.Data();   

}

//______________________________________________________________________________
const char *TFunctionMemberInfo::GetParameters() const{

   return fParameters.Data();   

}

//______________________________________________________________________________
ULong_t TFunctionMemberInfo::Hash() const{

   // Return the hash value based on the Class Name

   return fName.Hash();

}
