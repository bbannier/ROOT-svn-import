#include "TDataMemberInfo.h"

//______________________________________________________________________________
TDataMemberInfo::TDataMemberInfo(const char* membername,void* p,  Char_t type, Char_t reftype, Char_t constvar, Char_t statictype, Char_t accessin,const char *expr, Int_t definemacro,const char *comment, Int_t typeindex){

   fName = membername;
   fType = type;
   fRefType = reftype;  
   fConstVar = constvar;
   fStaticType = statictype;
   fAccessIn = accessin;
   fExpr = expr;
   fDefineMacro = definemacro;
   fTitle = comment;
   fFullTypeName = typeindex;
   fPointer = (Int_t) p;

}

//______________________________________________________________________________
Int_t  TDataMemberInfo::GetFullTypeName(){

   return fFullTypeName;

}

//______________________________________________________________________________
const TString&  TDataMemberInfo::GetComment(){

   return fTitle;

}

//______________________________________________________________________________
const TString&  TDataMemberInfo::GetExpr(){

   return fExpr;

}
      
//______________________________________________________________________________
Char_t TDataMemberInfo::GetType(){

   return fType;

}

//______________________________________________________________________________
Char_t  TDataMemberInfo::GetRefType(){

   return fRefType;

}
//______________________________________________________________________________
Char_t  TDataMemberInfo::GetConst(){

   return fConstVar;

}

//______________________________________________________________________________
Char_t  TDataMemberInfo::GetStaticType(){

   return fStaticType;

}

//______________________________________________________________________________
Char_t  TDataMemberInfo::GetAccess(){

   return fAccessIn;

}

//______________________________________________________________________________
Char_t  TDataMemberInfo::GetDefineMacro(){

   return fDefineMacro;

}

//______________________________________________________________________________
ULong_t TDataMemberInfo::Hash() const{

   // Return the hash value based on the Class Name

   return fName.Hash();

}
