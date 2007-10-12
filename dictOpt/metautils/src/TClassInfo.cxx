/******************************************************************************
 * Diego Marcos Segura                                                        *
 * Hasta que el pueblo las canta, las coplas, coplas no son                   *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 *****************************************************************************/

//______________________________________________________________________________
//                                                                      
// TClassInfo
//                                                                       
// - This class defines the persistency information for classes            
// - This class represents the the information in the 
//   old source code based dictionaries             
//                                                                       
//______________________________________________________________________________

#ifndef ROOT_TFunctionMemberInfo
#include "TFunctionMemberInfo.h"
#endif

#ifndef ROOT_TDataMemberInfo
#include "TDataMemberInfo.h"
#endif

#include "TClassInfo.h"

//______________________________________________________________________________
const char *TClassInfo::GetName() const{

   // Return the class name
   
   return fClassName.Data();
}

//______________________________________________________________________________
void TClassInfo::SetName(const char *className){

   // Set the class name

   fClassName = className;

}

//______________________________________________________________________________
ULong_t TClassInfo::Hash() const{

   // Return the hash value based on the Class Name

   return fClassName.Hash();

}

//______________________________________________________________________________
THashList* TClassInfo::GetDataMembers(){

   return &fDataMembers;

}

//______________________________________________________________________________
THashList* TClassInfo::GetFunctionMembers(){

   return &fFunctionMembers;

}


//______________________________________________________________________________
void TClassInfo::AddFunctionMember(const char *funcname, Int_t hash, Int_t type, Int_t p_tagtable, Int_t p_typetable, Int_t reftype, Int_t para_nu, Int_t access, Int_t ansi, Int_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t classindex, const char* symbol, Int_t p_tagtable_index, Int_t p_typetable_index){

   TFunctionMemberInfo *newFuncMember = new TFunctionMemberInfo(funcname, hash, type, p_tagtable, p_typetable, reftype,para_nu,access,ansi,isconst,paras,comment,isvirtual,classindex,symbol, p_tagtable_index, p_typetable_index );
 
   fFunctionMembers.Add((TObject *)newFuncMember);
  
}


//______________________________________________________________________________
void TClassInfo::AddDataMember(const char* membername, void* p, Int_t type, Int_t reftype, Int_t constvar, Int_t statictype, Int_t accessin,const char *expr, Int_t definemacro,const char *comment, Int_t classindex){
   
   TDataMemberInfo *newDataMember = new TDataMemberInfo(membername,p, type,reftype, constvar, statictype, accessin, expr, definemacro, comment, classindex);
 
   fDataMembers.Add((TObject *)newDataMember);

}
