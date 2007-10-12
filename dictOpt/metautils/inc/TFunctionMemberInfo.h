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
   
   TString fParameters; // Parameters. This is a string which represents the parameters. Afterwards it will be parsed by CINT
   Int_t fFullTypeName; // Index in the CINT Type List of the Dictionary
   Int_t fHash;
   Char_t fType; // CINT-C++ type Matching
   Char_t fReftype; // Reference Type eg: Pointer to func? See common.h 
   Int_t fParaNum; // Number of parameters
   Int_t fP_tagtable;
   Int_t fP_typetable;
   Char_t fAnsi; // Ansi?
   Char_t fAccess; // What about the Access? Public, Protected, Private
   Char_t fIsConst; // Constness?
   Char_t fIsVirtual; // Is it Virtual?
   Int_t fTagtable_index;
   Int_t fTypetable_index;
   TString fSymbol; // Mangled Symbol of the function

 public:

   TFunctionMemberInfo(){}

   virtual ~TFunctionMemberInfo(){}

   TFunctionMemberInfo(const char *funcname, int hash, Char_t type,Int_t p_tagtable, Int_t p_typetable, Char_t reftype, Char_t para_nu, Char_t access, Int_t ansi, Char_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t typeindex, const char* symbol, Int_t p_tagtable_index, Int_t p_typetable_index);

   Int_t GetFullTypeName();

   const TString& GetComment();

   Char_t GetType();
   Char_t GetRefType();
   Int_t GetParametersNumber() const;
   Int_t GetTagTable();
   Int_t GetTypeTable();
   Char_t GetConst();
   Int_t GetAnsi();
   Char_t GetAccess();
   Char_t GetIsConst();
   Char_t GetIsVirtual(); 
   const char *GetSymbol() const;
   const TString& GetParameters() const;
   Int_t  GetCintHash() const;
   Int_t  GetTagTableIndex() const;
   Int_t  GetTypeTableIndex() const;

   virtual ULong_t Hash() const;

   ClassDef(TFunctionMemberInfo,1);  //Dictionary containing class information

};

#endif

