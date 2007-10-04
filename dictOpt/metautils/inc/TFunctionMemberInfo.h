/******************************************************************************
 * Diego Marcos Segura                                                        *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 *****************************************************************************/

//______________________________________________________________________________
//                                                                      
// TFunctionMemberInfo
//                                                                       
// - This class defines the persistency information for methods (Function member)            
// - This class represents the information in the 
//   old source code based dictionaries             
//                                                                       
//______________________________________________________________________________

#ifndef ROOT_TFunctionMemberInfo
#define ROOT_TFunctionMemberInfo

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

class TClassInfo;

class TFunctionMemberInfo : public TNamed{

 private:
   
   /* TString fName; // Name of the Function */
/*    TString fComment; // Comment of the function */
   TString fParameters; // Parameters. This is a string which represents the parameters. Afterwards it will be parsed by CINT
   Int_t fFullTypeName; // Index in the CINT Type List of the Dictionary

   Char_t fType; // CINT-C++ type Matching
   Char_t fReftype; // Reference Type eg: Pointer to func? See common.h 
   Int_t fParaNum; // Number of parameters
   Char_t fAnsi; // Ansi?
   Char_t fAccess; // What about the Access? Public, Protected, Private
   Char_t fIsConst; // Constness?
   Char_t fIsVirtual; // Is it Virtual?
   TString fSymbol; // Mangled Symbol of the function

 public:

   TFunctionMemberInfo(){}

   virtual ~TFunctionMemberInfo(){}

   TFunctionMemberInfo(const char *funcname, Char_t type, Char_t reftype, Char_t para_nu, Char_t access, Char_t ansi, Char_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t typeindex, const char* symbol);

   Int_t GetFullTypeName();

   const TString& GetComment();

   Char_t GetType();
   Char_t GetRefType();
   Char_t GetConst();
   Char_t GetStaticType();
   Char_t GetAccess();
   Char_t GetIsConst();
   Char_t GetIsVirtual();

   const char *GetSymbol() const;
   const char *GetParameters() const;

   virtual ULong_t Hash() const;

   ClassDef(TFunctionMemberInfo,1);  //Dictionary containing class information

};

#endif

