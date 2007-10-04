/******************************************************************************
 * Diego Marcos Segura                                                        *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 *****************************************************************************/

//______________________________________________________________________________
//                                                                      
// TDataMemberInfo
//                                                                       
// - This class defines the persistency information for Data Members (Attributes)            
// - This class represents the information in the 
//   old source code based dictionaries             
//                                                                       
//______________________________________________________________________________

#ifndef ROOT_TDataMemberInfo
#define ROOT_TDataMemberInfo

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TClassInfo;

class TDataMemberInfo : public TNamed {

 private:
 
 /*   TString fName; // Data Member Name */
/*    TString fComment; // Data Member Comment */
   TString fExpr; // Expression

   Int_t fFullTypeName; //full type description of data member, e,g.: "class TDirectory*".
   
   Int_t fPointer;

   Char_t fType; // CINT-C++ type Matching
   Char_t fRefType;  // Reference Type eg: Pointer to func? See common.h 
   Char_t fConstVar; // Is it const? See. common.h
   Char_t fStaticType; // Is it Static?
   Char_t fAccessIn; // Access? Public, Protected, Private
   Char_t fDefineMacro; 
 
 public:

   TDataMemberInfo(){}

   TDataMemberInfo(const char* membername, void* p, Char_t type, Char_t reftype, Char_t constvar, Char_t statictype, Char_t accessin,const char *expr, Int_t definemacro,const char *comment, Int_t typeindex);
   
   virtual ~TDataMemberInfo(){}

   Int_t GetFullTypeName(); 
  
   const TString& GetComment();
   const TString& GetExpr();
      
   Char_t GetType();
   Char_t GetRefType();
   Char_t GetConst();
   Char_t GetStaticType();
   Char_t GetAccess();
   Char_t GetDefineMacro();

   virtual ULong_t Hash() const;

   ClassDef(TDataMemberInfo,1);  //Dictionary containing class information

};

#endif
