/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TDictionaryManager
#define ROOT_TDictionaryManager

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDictionaryManager.h                                                 //
//                                                                      //
// This class defines an interface between ROOTCINT and CINT            //
// This class manages Cint Dictionaries based on ROOT Files             //
// Interfaces for creating and reading these dictionaries are           //
// provided                                                             //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TObject
#include "TObject.h"
#endif

#ifndef ROOT_THashList
#include "THashList.h"
#endif

#ifndef ROOT_TString
#include "TString.h"
#endif

#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

#ifndef ROOT_TFile
#include "TFile.h"
#endif

class TFile;
class TClassInfo;
class TCintTypeInfo;

class TDictionaryManager : public TObject {

private:
   
   static TObjArray fTypesInfo; // CINT Types Information. 

   THashList fClassList; // List of classes in the dictionary

   static TFile* fCurrentDictFile; // Root file which contains the dictionary information.  

   TFile* fDictFile;

   static THashList fFilesCache; // Files Cache. This allows to avoid opening several times recently accesed dictionary files 
   
   static TDictionaryManager *fCurrentDict; // Current Dictionary Generator. Static which allows having multiple dictionary generators

public:
   
   TFile* GetDictionaryFile(const char* filename, const char* mode);
 
   TDictionaryManager();

   TDictionaryManager(const char* dictName);
   
   virtual ~TDictionaryManager();

   void DictGenEnable(); // Enable the current dictionary

   void AddClass(const char* classname); // Add one class to the dictionary

   static int WriteFile(const char* dictName); // Write the classes into a root file.

   Int_t AddType(TCintTypeInfo newType); // Add type to the Cint Types Information

   // Static Function Called by a CallBack Pattern (aka Hook) from CINT for adding a new Function Member to a class
   static Int_t AddFunctionMember(const char *funcname, Char_t type, Char_t reftype, Char_t structtype, Int_t para_nu, Int_t access, Int_t ansi, Int_t isconst,const char *paras, const char *comment, Int_t isvirtual, const char* classname, const char* symbol);

   // Static Function Called by a CallBack Pattern (aka Hook) from CINT for adding a new  Member to a class
   static Int_t AddDataMember(const char* membername,  void* p, Char_t type, Char_t reftype, Char_t structtype, Char_t enumvar, Int_t constvar, Int_t statictype, Int_t accessin,const char *expr, Int_t definemacro,const char *comment, const char* classname, const char* typedefname);

   // Static Function Called by a CallBack Pattern (aka Hook) from CINT for setting up FunctionMembers in CINT structures
   static Int_t FunctionMembersReader(const char* rootdictname, const char* sourcefile, const char* classname);

   // Static Function Called by a CallBack Pattern (aka Hook) from CINT for setting up FunctionMembers in CINT structures
   static Int_t DataMembersReader(const char* rootdictname, const char* sourcefile, const char* classname);

   // Static Function Called by a CallBack Pattern (aka Hook) from CINT for getting the dict name  
   static const char* GetDictName(); 

   ClassDef(TDictionaryManager,1);  // Interface for TClass generators

};



// In case of type='U' or 'u' the type can be e = enum, c = class and s = struct 
enum StructTypes { kEnum = 'e', 
                   kClass = 'c', 
                   kStruct ='s',
                   kUnion = 'u',
                   kTypeDef='t'}; 

class TCintTypeInfo : public TObject {

 private:
   
   TString fTypeName; // Complete Type Name e.g. NamesSpace1::NameSpace2::..::Type
   char fType; // Cint types C++ matching. e.g. Class 'c' Float 'f' ... 
   Int_t fTagnum; // Tagnum or typenum it depends of having [class, struct, enum] or [Int_t, float_t...]
   // Note: Tagnum and typenum are the index in G__struct or G__newtype
   
 public:

   TCintTypeInfo() {};

   virtual ~TCintTypeInfo() {};

   TCintTypeInfo(const  TCintTypeInfo&s) : TObject(), fTypeName(s.fTypeName), fType(s.fType), fTagnum(s.fTagnum) { };

   TCintTypeInfo(const char* name, char type, Int_t varnum);

   const char*	GetName() const; // Return the complete name of the type
   Char_t GetType(); // Return the CINT C++ type matching
   Int_t GetTagnum();

   void SetName(const char* name); 
   void SetType(Char_t type); 
   void SetTagnum(Int_t tagnum);

   ClassDef(TCintTypeInfo,1);  // Interface for TClass generators


};

#endif


