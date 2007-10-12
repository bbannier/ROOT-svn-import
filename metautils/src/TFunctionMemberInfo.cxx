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
TFunctionMemberInfo::TFunctionMemberInfo(const char *funcname, int hash, Char_t type, Int_t p_tagtable, Int_t p_typetable, Char_t reftype, Char_t para_nu, Char_t access, Int_t ansi, Char_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t typeindex, const char* symbol, Int_t p_tagtable_index, Int_t p_typetable_index){

   fSymbol = symbol;
   fName = funcname;
   fHash = hash;
   fType = type;
   fReftype = reftype;
   fParaNum = para_nu;
   fP_tagtable = p_tagtable;
   fP_typetable = p_typetable;
   fAccess = access;
   fAnsi = ansi;
   fIsConst = isconst;
   fParameters = paras;
   fTitle = comment;
   fIsVirtual = isvirtual;
   fFullTypeName = typeindex;
   fTagtable_index = p_tagtable_index;
   fTypetable_index = p_typetable_index;   

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
const TString& TFunctionMemberInfo::GetParameters() const{

   return fParameters;   

}

//______________________________________________________________________________
Int_t TFunctionMemberInfo::GetParametersNumber() const{

   return fParaNum;
 
}

//______________________________________________________________________________
Int_t TFunctionMemberInfo::GetTagTable(){

   return fP_tagtable;

}

//______________________________________________________________________________
Int_t TFunctionMemberInfo::GetTypeTable(){

   return fP_typetable;

}

//______________________________________________________________________________
Char_t TFunctionMemberInfo::GetConst(){

   return fParaNum;

}

//______________________________________________________________________________
Int_t TFunctionMemberInfo::GetAnsi(){

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
Int_t TFunctionMemberInfo::GetCintHash() const{

   return fHash;
 
}

//______________________________________________________________________________
ULong_t TFunctionMemberInfo::Hash() const{

   // Return the hash value based on the Class Name

   return fName.Hash();

}

//______________________________________________________________________________
Int_t  TFunctionMemberInfo::GetTagTableIndex() const{

   return fTagtable_index;

}

//______________________________________________________________________________
Int_t  TFunctionMemberInfo::GetTypeTableIndex() const{

   return fTypetable_index;

}


