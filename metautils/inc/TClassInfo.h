/******************************************************************************
 * Diego Marcos Segura                                                        *
 *                                                                            *
 * For the licensing terms see $ROOTSYS/LICENSE.                              *
 * For the list of contributors see $ROOTSYS/README/CREDITS.                  *
 *****************************************************************************/


//______________________________________________________________________________
//                                                                      
// TClassInfo
//                                                                       
// - This class defines the persistency information for classes            
// - This class will be written in a root file                             
// - This class replace the old source code based dictionaries             
//                                                                       
//______________________________________________________________________________

#ifndef ROOT_TClassInfo
#define ROOT_TClassInfo

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_THashList
#include "THashList.h"
#endif

class TClassInfo : public TObject{

 private:

// Generic Class Info //

   TString fClassName; // Class Name

// Members Information //

   THashList fDataMembers;   // List for data members */
   THashList fFunctionMembers; // List for methods */
   THashList fJumble; // List rag bag. e.g Shadow Classes

 public:

   TClassInfo(){}

   virtual ~TClassInfo(){}
   
   TClassInfo(const char* className){

      fClassName = className;

   };

   const char*	GetName() const;
   void SetName(const char* name);

   virtual ULong_t Hash() const;

   THashList* GetDataMembers();
   THashList* GetFunctionMembers();

   // Add a new Function Member (Method) to the class
   void AddFunctionMember(const char *funcname, Int_t type, Int_t reftype, Int_t para_nu, Int_t access, Int_t ansi, Int_t isconst,const char *paras, const char *comment, Int_t isvirtual, Int_t classindex, const char* symbol);

   // Add a new Data Member (Attribute) to the class
   void AddDataMember(const char* membername, void* p,Int_t type, Int_t reftype, Int_t constvar, Int_t statictype, Int_t accessin,const char *expr, Int_t definemacro,const char *comment, Int_t classindex);

   ClassDef(TClassInfo,1);  //Dictionary containing class information

};

#endif
