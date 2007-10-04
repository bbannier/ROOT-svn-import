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

#include "TDictionaryManager.h"

#include "TDataMemberInfo.h"

#include "G__ci.h"
#include "TClassInfo.h"

#include "TObjString.h"
#include "TRandom.h"
#include <string>

#include "TClassInfo.h"
#include "TClass.h"
#include "TVirtualStreamerInfo.h"


//_____________________________________________________________________________
TCintTypeInfo::TCintTypeInfo(const char* name, Char_t type, Int_t varnum){

      fTypeName = name;
      fType = type;
      fTagnum = varnum;
   
}

//_____________________________________________________________________________
const char* TCintTypeInfo::GetName() const{

  return fTypeName.Data();

}


//_____________________________________________________________________________
Char_t TCintTypeInfo::GetType(){

   return fType;
   
}


//_____________________________________________________________________________
Int_t TCintTypeInfo::GetTagnum(){

   return fTagnum;
   
}

//_____________________________________________________________________________
void TCintTypeInfo::SetName(const char* name){

      fTypeName = name;

}


//_____________________________________________________________________________
void TCintTypeInfo::SetType(Char_t type){

      fType = type;
}


//_____________________________________________________________________________
void TCintTypeInfo::SetTagnum(Int_t tagnum){

      fTagnum = tagnum;
}


//______________________________________________________________________________
TDictionaryManager::TDictionaryManager(){

   // We set CINT function pointers to the static functions. 
   // Setting up CallBack (aka Hook) from CINT to the Dictionary Manager

   G__set_funcmember_root_writer(TDictionaryManager::AddFunctionMember);
   G__set_datamember_root_writer(TDictionaryManager::AddDataMember);
   G__set_funcmember_root_reader(TDictionaryManager::FunctionMembersReader);
   G__set_datamember_root_reader(TDictionaryManager::DataMembersReader);
   G__set_get_dictname(TDictionaryManager::GetDictName);
   G__set_dictionary_writer(TDictionaryManager::WriteFile);

   // Currrent Dictionary Enabled
   TDictionaryManager::fCurrentDict = this;

   // TypesInfo Name
   TDictionaryManager::fTypesInfo.SetName("TypesList");

}

//______________________________________________________________________________
TDictionaryManager::TDictionaryManager(const char* dictName){

   // We set CINT function pointers to the static functions. 
   // Setting up CallBack (aka Hook) from CINT to the Dictionary Manager
   
   G__set_funcmember_root_writer(TDictionaryManager::AddFunctionMember);
   G__set_datamember_root_writer(TDictionaryManager::AddDataMember);
   G__set_funcmember_root_reader(TDictionaryManager::FunctionMembersReader);
   G__set_datamember_root_reader(TDictionaryManager::DataMembersReader);
   G__set_get_dictname(TDictionaryManager::GetDictName);
   G__set_dictionary_writer(TDictionaryManager::WriteFile);

   //TDictionaryManager::fFilesCache.SetOwner(kTRUE);

   // Currrent Dictionary Enabled
   TDictionaryManager::fCurrentDict = this;

   // Current Dictionary File 
   TFile* fDictFile = new TFile(dictName,"update");

   TDictionaryManager::fCurrentDictFile = fDictFile;

   // TypesInfo Name
   TDictionaryManager::fTypesInfo.SetName("TypesList");

}

//______________________________________________________________________________
TDictionaryManager::~TDictionaryManager(){

   // Close And Delete the file 
   if (TDictionaryManager::fCurrentDictFile){
      TDictionaryManager::fCurrentDictFile = 0;
    }

   // Current Dictionary Disabled
   TDictionaryManager::fCurrentDict = 0;
   
}

//______________________________________________________________________________
TFile* TDictionaryManager::GetDictionaryFile(const char* filename, const char* mode){

// Returns a pointer to a Dictionary File
// Cache System: We Keep the last recently used dictionaries (10 Dict) in a list
// This allows avoiding a lot of file openings

   // Does the file already exist?
   TFile* openFile = (TFile*) TDictionaryManager::fFilesCache.FindObject(filename);
   if(!openFile){ // No, It doesn't
      
      // We open the file through a CINT call. Why? LibCore.so is not linked against LibRIO.so so we cannot do TFile Open() directly
      openFile  = new TFile(filename,mode);

      // If the list has already 10 elements then the last recently used is removed.
      if (TDictionaryManager::fFilesCache.GetEntries() >= 10){

         //((TDirectory*) TDictionaryManager::fFilesCache.Last())->Close();
         TDictionaryManager::fFilesCache.RemoveLast();
      }
   }     
   else // Remove the file from the original current position for adding it later in the first place (Most recent used policy)
      TDictionaryManager::fFilesCache.Remove(openFile);

   // Current file is the most recently used then It will be the first one in the Cache List.
   TDictionaryManager::fFilesCache.AddFirst(openFile);

   return openFile; 

}


//______________________________________________________________________________
int  TDictionaryManager::WriteFile(const char* dictName){

   if (!TDictionaryManager::fCurrentDictFile){
      // Current Dictionary File 
      TFile* fDictFile = new TFile(dictName,"update");
      TDictionaryManager::fCurrentDictFile = fDictFile;
   }

   // Writes int the ROOT file all the read information from the classes sources.
   TIter next(& TDictionaryManager::fCurrentDict->fClassList);
   while(TClassInfo* classInfo = (TClassInfo* ) next())
      classInfo->Write();
      //TDictionaryManager::fCurrentDictFile->WriteObject(classInfo,classInfo->GetName());

   TDictionaryManager::fTypesInfo.Write("TypesList",1);

   TClassInfo::Class()->GetStreamerInfo()->ls();

   TDictionaryManager::fCurrentDictFile->Close();
   delete  TDictionaryManager::fCurrentDictFile;
   TDictionaryManager::fCurrentDictFile = 0;

   return 0;

}


//______________________________________________________________________________
void TDictionaryManager::DictGenEnable(){
   
   // Enables a new Dictionary Manager

   TDictionaryManager::fCurrentDict = this;

}

//______________________________________________________________________________
Int_t TDictionaryManager::AddType(TCintTypeInfo newType){

   // Add a new type to the types list

   // The type must have a name
   if (!strcmp(newType.GetName(),""))
       return -1;

   // We look for the type in the list
   for (Int_t i= 0; i < TDictionaryManager::fTypesInfo.GetEntriesFast(); i++){
   
      // If the type is already there we return the index in the array
      if (!strcmp(newType.GetName(),TDictionaryManager::fTypesInfo[i]->GetName())){
         return i;
      }
   }

   TCintTypeInfo* obj = new TCintTypeInfo(newType);
   // The type is not in the array. We add the new type and the index is returned
   TDictionaryManager::fTypesInfo.AddLast(obj);
   return (TDictionaryManager::fTypesInfo.GetEntriesFast()-1);
}

//______________________________________________________________________________
int TDictionaryManager::AddDataMember(const char* membername, void* p, Char_t type, Char_t reftype, Char_t structtype, Char_t enumvar, Int_t constvar, Int_t statictype, Int_t accessin,const char *expr, Int_t definemacro,const char *comment, const char* classname, const char* typedefname){

   // Add one Data Member (Attribute) to a class
   
   // Creates the CINT type information
   TCintTypeInfo memberType(typedefname,-1,-1);
   if (type=='u'||type=='U')
      memberType.SetType('c');
   if (enumvar)
      memberType.SetType('e');
   
   // The type is added to the list. We get its index in the types list
   Int_t type_index = TDictionaryManager::fCurrentDict->AddType(memberType);

   // We look for the class where the new data member is going to be added
   TClassInfo *classInfo = (TClassInfo*) TDictionaryManager::fCurrentDict->fClassList.FindObject(classname); 

   // If the class entry doesn't already exist in the class list, a new entry is created
   if (!classInfo){
         
      classInfo = new TClassInfo(classname);
   
      TDictionaryManager::fCurrentDict->fClassList.Add(classInfo);
      
   }
      
   // Add Data member to the proper class
   classInfo->AddDataMember(membername,p,type,reftype, constvar, statictype, accessin, expr, definemacro, comment, type_index);
  
   return 0;
   
}

  
//______________________________________________________________________________
int TDictionaryManager::AddFunctionMember(const char *funcname, Char_t type, Char_t reftype, Char_t structtype, Int_t para_nu, Int_t access, Int_t ansi, Int_t isconst,const char *paras, const char *comment, Int_t isvirtual, const char* classname, const char* symbol){

   /* TEMPORARY  RANDOM SYMBOL GENERATION*/
   TRandom gen;
   char funcSym[41];
   for (int i; i < 40; i++)
      funcSym[i] = (char) (gen.Integer(100000)%128);
         
   funcSym[40] = '\0';
   /* TEMPORARY */


   // Add one Member Function (Method) to a class

   // Creates CINT type information
   TCintTypeInfo memberType(classname,structtype,-1);
   Int_t type_index = TDictionaryManager::fCurrentDict->AddType(memberType);

   // We look for the class where the new member function is going to be added
   TClassInfo *classInfo = (TClassInfo*) TDictionaryManager::fCurrentDict->fClassList.FindObject(classname); 
   // If the class entry doesn't already exist in the class list, a new entry is created
   if (!classInfo){
         
      classInfo = new TClassInfo(classname);
   
      TDictionaryManager::fCurrentDict->fClassList.Add(classInfo);

   }
      
   // Add The Function Member to the proper class
   classInfo->AddFunctionMember(funcname, type,reftype,para_nu,access,ansi,isconst,paras,comment,isvirtual,type_index,(const char*) funcSym); 

   return 0;
}

//______________________________________________________________________________
const char* TDictionaryManager::GetDictName(){

   // Returns the Dictionary Name

   if (TDictionaryManager::fCurrentDictFile)
      return TDictionaryManager::fCurrentDictFile->GetName();
   else 
      return "";

}


//______________________________________________________________________________
Int_t  TDictionaryManager::FunctionMembersReader(const char* rootdictname, const char* sourcefile, const char* classname){

   return 0;

}



//______________________________________________________________________________
Int_t  TDictionaryManager::DataMembersReader(const char* rootdictname, const char* sourcefile, const char* classname){

   // Reads the data members (Atributes) of a class from the ROOT file
   // Call CINT functions to initialize its structures: 
   //         G__get_linked_tagnum -> Initialize the class entry in the G__struct 
   //         G__tag_memvar_setup -> Initialize the data members list in the G__struct (G__struct.memvar)
   //         G__memvar_setup -> One per Data Member. Initialize Data Members Entries G__struct.memvar[tagnum]

   // Open the Dictionary File through CINT
   TDirectory* readFile = TDictionaryManager::fCurrentDict->GetDictionaryFile(rootdictname, "open");
   
   // Reads the information of the class
   TClassInfo* readClass;
   readFile->GetObject(classname,readClass);
  
   // Reads Data Members of the class 
   THashList *dataMembers = readClass->GetDataMembers();

   // Reads CINT type ifnormation
   TObjArray *typesList; 
   readFile->GetObject("TypesList",typesList);

   // Initialize the entry of the class in the G__struct
   G__linked_taginfo classLink = { classname, 99, -1 };
   G__tag_memvar_setup(G__get_linked_tagnum(&classLink));

   // Iteration through data members 
   TIter dataMemberIter(dataMembers);
   TDataMemberInfo* currentDataMember;
   while (currentDataMember = (TDataMemberInfo *) dataMemberIter.Next()){
      
      // Get the index in the CINT type info list
      Int_t typeIndex = currentDataMember->GetFullTypeName();

      if (typeIndex == -1){
      
         G__memvar_setup((void *) NULL,currentDataMember->GetType(),currentDataMember->GetRefType(),currentDataMember->GetConst(),-1,-1,currentDataMember->GetStaticType(),currentDataMember->GetAccess(),currentDataMember->GetExpr().Data(),currentDataMember->GetDefineMacro(),currentDataMember->GetComment().Data());

      }
      else{ // The type of the data member is either a class or a typedef
         
         // Get CINT type of the data member
         TCintTypeInfo* typeInfo = (TCintTypeInfo*) typesList->At(typeIndex);

         int num;
         void* p = (void*)NULL;
            
            //G__linked_taginfo taginfo = { typeInfo->GetName() , typeInfo->GetType() , -1 };
            // Get the tagnum (Remember: the index in the G__struct)
            //num = G__get_linked_tagnum(&taginfo);
         if (typeInfo->GetTagnum() == -1){

            if (((typeInfo->GetType() == 'c')||(typeInfo->GetType() == 'e'))){
                      
               num = G__defined_tagname(typeInfo->GetName(),currentDataMember->GetType());
                         
               if (num==-1){
                  G__linked_taginfo taginfo = { typeInfo->GetName() , typeInfo->GetType() , -1 };
                  //Get the tagnum (Remember: the index in the G__struct)
                  num = G__get_linked_tagnum(&taginfo);
               }
               
            }
            else{ // Data Member type is a typedef
               // Get the typenum (Remember: the index in G__newtype)
               num = G__defined_typename(typeInfo->GetName());
            }

            typeInfo->SetTagnum(num);

         }
         else
            num =  typeInfo->GetTagnum();
         
         
         if (typeInfo->GetType() == 'e')
            p = (void*)G__PVOID;

         if (typeInfo->GetType() == 'e' || currentDataMember->GetType() == 'u' || currentDataMember->GetType() == 'U'){
         // Data Member Initializatin in CINT 
            G__memvar_setup(p,currentDataMember->GetType(),currentDataMember->GetRefType(),currentDataMember->GetConst(),num,-1, currentDataMember->GetStaticType(),currentDataMember->GetAccess(),currentDataMember->GetExpr().Data(),currentDataMember->GetDefineMacro(),currentDataMember->GetComment().Data());
         }
         else
            G__memvar_setup(p,currentDataMember->GetType(),currentDataMember->GetRefType(),currentDataMember->GetConst(),-1,num, currentDataMember->GetStaticType(),currentDataMember->GetAccess(),currentDataMember->GetExpr().Data(),currentDataMember->GetDefineMacro(),currentDataMember->GetComment().Data());

      }

   }
         
   return 0;
}



//______________________________________________________________________________
void TDictionaryManager::AddClass(const char* classname){

   // Add a Class to the Class List in the Dictionary

   // Look for the class
   TClassInfo *classInfo = (TClassInfo*) TDictionaryManager::fCurrentDict->fClassList.FindObject(classname); 
   // If the class doesn't already exist then We add it
   if (!classInfo){
         
      classInfo = new TClassInfo(classname);
   
      TDictionaryManager::fCurrentDict->fClassList.Add(classInfo);

   }

}

/*---------------------*/
/* STATIC DATA MEMBERS */
/*---------------------*/

//______________________________________________________________________________
TFile* TDictionaryManager::fCurrentDictFile = 0;

//______________________________________________________________________________
THashList TDictionaryManager::fFilesCache;

//______________________________________________________________________________
TObjArray TDictionaryManager::fTypesInfo; 

//______________________________________________________________________________
TDictionaryManager* TDictionaryManager::fCurrentDict = 0;

//______________________________________________________________________________
static TDictionaryManager gDefaultDict;
