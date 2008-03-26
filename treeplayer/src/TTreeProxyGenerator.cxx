// @(#)root/treeplayer:$Id$
// Author: Philippe Canal 06/06/2004

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers and al.        *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

/*
  TODO:
  Have separate names for the wrapper classes in the cases of: [done]
  clones/non clones
  split/non split
  split levels

  Have a solution for passing top+"."+middle to the parents classes [probably done .. need testing]

  Have a solution for the return by references of abstract classes [not done]

  Have object inside ClonesArray properly treated! [done]
  Why is there 2 TRef proxy classes? [done]

  check why some inheritance are TObjProxy and not TPx_

  Be smart enough to avoid issue about having 2 classes one unrolled and one non unrolled!

  When using in interpreted mode understand why the reloading reloads the calling script and then crashes :(

  CINT does not properly call the custom operators when doing return fNtrack.

  CINT does not handle fMatrix[2][1] well.

  The user's function in script.h are not exposed by ACLiC.

  Review the method to avoid the useless refreshing of the generated file
  - for most efficiency it would require a different name for each tree
*/

#include "TTreeProxyGenerator.h"

#include "TFriendProxyDescriptor.h"
#include "TBranchProxyDescriptor.h"
#include "TBranchProxyClassDescriptor.h"

#include "TList.h"
#include "Varargs.h"
#include <stdio.h>

class TTree;
class TBranch;
class TStreamerElement;

#include "TClass.h"
#include "TClassEdit.h"
#include "TClonesArray.h"
#include "TError.h"
#include "TROOT.h"
#include "TObjString.h"

#include "TTreeFormula.h"
#include "TFormLeafInfo.h"


#include "TBranchElement.h"
#include "TChain.h"
#include "TFile.h"
#include "TFriendElement.h"
#include "TLeaf.h"
#include "TTree.h"
#include "TVirtualStreamerInfo.h"
#include "TStreamerElement.h"
#include "TSystem.h"
#include "TLeafObject.h"
#include "TVirtualCollectionProxy.h"

void Debug(Int_t level, const char *va_(fmt), ...)
{
   // Use this function in case an error occured.

   if (gDebug>=level) {
      va_list ap;
      va_start(ap,va_(fmt));
      ErrorHandler(kInfo,"TTreeProxyGenerator",va_(fmt), ap);
      va_end(ap);
   }
}

namespace {

   Bool_t AreDifferent(const TString& from, const TString& to)
   {
      FILE *left = fopen(from.Data(),"r");
      FILE *right = fopen(to.Data(),"r");

      char leftbuffer[256];
      char rightbuffer[256];

      char *lvalue,*rvalue;

      Bool_t areEqual = kTRUE;

      do {
         lvalue = fgets(leftbuffer, sizeof(leftbuffer), left);
         rvalue = fgets(rightbuffer, sizeof(rightbuffer), right);

         if (lvalue&&rvalue) {
            if (strstr(lvalue,"by ROOT version")) {
               // skip the comment line with the time and date
            } else {
               areEqual = areEqual && (0 == strncmp(lvalue,rvalue,sizeof(leftbuffer)));
            }
         }
         if (lvalue&&!rvalue) areEqual = kFALSE;
         if (rvalue&&!lvalue) areEqual = kFALSE;

      } while(areEqual && lvalue && rvalue);

      fclose(left);
      fclose(right);

      return !areEqual;
   }
}

namespace ROOT {

   TString GetArrayType(TStreamerElement *element, const char *subtype,
                        TTreeProxyGenerator::EContainer container)
   {
      TString result;
      int ndim = 0;
      if (element && element->InheritsFrom(TStreamerBasicPointer::Class())) {
         TStreamerBasicPointer * elem = (TStreamerBasicPointer*)element;
         const char *countname = elem->GetCountName();
         if (countname && strlen(countname)>0) ndim = 1;
      }
      ndim += element->GetArrayDim();

      TString middle;
      if (container == TTreeProxyGenerator::kClones) {
         middle = "Cla";
      } else if  (container == TTreeProxyGenerator::kSTL) {
         middle = "Stl";
      }

      if (ndim==0) {
         result = "T";
         result += middle;
         result += subtype;
         result += "Proxy";
      } else if (ndim==1) {
         result = "T";
         result += middle;
         result += "Array";
         result += subtype;
         result += "Proxy";
      } else {
         result = "T";
         result += middle;
         result += "ArrayProxy<";
         for(Int_t ind = ndim - 2; ind > 0; --ind) {
            result += "TMultiArrayType<";
         }
         result += "TArrayType<";
         result += element->GetTypeName();
         result += ",";
         result += element->GetMaxIndex(ndim-1);
         result += "> ";
         for(Int_t ind = ndim - 2; ind > 0; --ind) {
            result += ",";
            result += element->GetMaxIndex(ind);
            result += "> ";
         }
         result += ">";
      }
      return result;

      /*
        if (!strcmp("unsigned int", name))
        sprintf(line, "%u", *(unsigned int *)buf);
        else if (!strcmp("int", name))
        sprintf(line, "%d", *(int *)buf);
        else if (!strcmp("unsigned long", name))
        sprintf(line, "%lu", *(unsigned long *)buf);
        else if (!strcmp("long", name))
        sprintf(line, "%ld", *(long *)buf);
        else if (!strcmp("unsigned short", name))
        sprintf(line, "%hu", *(unsigned short *)buf);
        else if (!strcmp("short", name))
        sprintf(line, "%hd", *(short *)buf);
        else if (!strcmp("unsigned char", name))
        sprintf(line, "%u", *(unsigned char *)buf);
        else if (!strcmp("bool", name))
        sprintf(line, "%u", *(unsigned char *)buf);
        else if (!strcmp("char", name))
        sprintf(line, "%d", *(char *)buf);
        else if (!strcmp("float", name))
        sprintf(line, "%g", *(float *)buf);
        else if (!strcmp("double", name))
        sprintf(line, "%g", *(double *)buf);
      */
   }

   TTreeProxyGenerator::TTreeProxyGenerator(TTree* tree,
                                            const char *script,
                                            const char *fileprefix,
                                            const char *option, UInt_t maxUnrolling) :
      fMaxDatamemberType(2),
      fScript(script),
      fCutScript(),
      fPrefix(fileprefix),
      fHeaderFileName(),
      fOptionStr(option),
      fOptions(0),
      fMaxUnrolling(maxUnrolling),
      fTree(tree),
      fCurrentListOfTopProxies(&fListOfTopProxies)
   {
      // Constructor.

      ParseOptions();

      AnalyzeTree(fTree);

      WriteProxy();
   }

   TTreeProxyGenerator::TTreeProxyGenerator(TTree* tree,
                                            const char *script, const char *cutscript,
                                            const char *fileprefix,
                                            const char *option, UInt_t maxUnrolling) :
      fMaxDatamemberType(2),
      fScript(script),
      fCutScript(cutscript),
      fPrefix(fileprefix),
      fHeaderFileName(),
      fOptionStr(option),
      fOptions(0),
      fMaxUnrolling(maxUnrolling),
      fTree(tree),
      fCurrentListOfTopProxies(&fListOfTopProxies)
   {
      // Constructo.

      ParseOptions();

      AnalyzeTree(fTree);

      WriteProxy();
   }

   Bool_t TTreeProxyGenerator::NeedToEmulate(TClass *cl, UInt_t /* level */)
   {
      // Return true if we should create a nested class representing this class

      return cl!=0 && cl->TestBit(TClass::kIsEmulation);
   }

   TBranchProxyClassDescriptor*
   TTreeProxyGenerator::AddClass( TBranchProxyClassDescriptor* desc )
   {
      // Add a Class Descriptor.

      if (desc==0) return 0;

      TBranchProxyClassDescriptor *existing =
         (TBranchProxyClassDescriptor*)fListOfClasses(desc->GetName());

      int count = 0;
      while (existing) {
         if (! existing->IsEquivalent( desc )  ) {
            TString newname = desc->GetRawSymbol();
            count++;
            newname += "_";
            newname += count;

            desc->SetName(newname);
            existing = (TBranchProxyClassDescriptor*)fListOfClasses(desc->GetName());
         } else {
            // we already have the exact same class
            delete desc;
            return existing;
         }
      }
      fListOfClasses.Add(desc);
      return desc;
   }

   void TTreeProxyGenerator::AddFriend( TFriendProxyDescriptor* desc )
   {
      // Add Friend descriptor.

      if (desc==0) return;

      TFriendProxyDescriptor *existing =
         (TFriendProxyDescriptor*)fListOfFriends(desc->GetName());

      int count = 0;
      while (existing) {
         if (! existing->IsEquivalent( desc )  ) {
            TString newname = desc->GetName();
            count++;
            newname += "_";
            newname += count;

            desc->SetName(newname);
            existing = (TFriendProxyDescriptor*)fListOfFriends(desc->GetName());

         } else {

            desc->SetDuplicate();
            break;
         }
      }

      // Insure uniqueness of the title also.
      TString basetitle = desc->GetTitle();
      TIter next( &fListOfFriends );
      while ( (existing = (TFriendProxyDescriptor*)next()) ) {
         if (strcmp(existing->GetTitle(),desc->GetTitle())==0) {

            TString newtitle = basetitle;
            count++;
            newtitle += "_";
            newtitle += count;

            desc->SetTitle(newtitle);

            // Restart of the begining of the loop.
            next = &fListOfFriends;
         }
      }

      fListOfFriends.Add(desc);
   }

   void TTreeProxyGenerator::AddForward( const char *classname )
   {
      // Add a forward declaration request.

      TObject *obj = fListOfForwards.FindObject(classname);
      if (obj) return;

      if (strstr(classname,"<")!=0) {
         // this is a template instantiation.
         // let's ignore it for now

         if (gDebug>=6) Warning("AddForward","Forward declaration of templated class not implemented yet.");
      } else if (strcmp(classname,"string")==0) {
         // no need to forward declare string
      } else {
         fListOfForwards.Add(new TNamed(classname,Form("class %s;\n",classname)));
      }
      return;
   }

   void TTreeProxyGenerator::AddForward(TClass *cl)
   {
      // Add a forward declaration request.

      if (cl) AddForward(cl->GetName());
   }

   void TTreeProxyGenerator::AddHeader(TClass *cl)
   {
      // Add a header inclusion request.

      if (cl==0) return;

      TObject *obj = fListOfHeaders.FindObject(cl->GetName());
      if (obj) return;

      TString directive;

      Int_t stlType;
      if (cl->GetCollectionProxy() && (stlType=TClassEdit::IsSTLCont(cl->GetName()))) {
         const char *what = "";
         switch(stlType)  {
            case TClassEdit::kVector:   what = "vector"; break;
            case TClassEdit::kList:     what = "list"; break;
            case TClassEdit::kDeque:    what = "deque"; break;
            case TClassEdit::kMap:      what = "map"; break;
            case TClassEdit::kMultiMap: what = "map"; break;
            case TClassEdit::kSet:      what = "set"; break;
            case TClassEdit::kMultiSet: what = "set"; break;
         }
         directive = "#include <";
         directive.Append(what);
         directive.Append(">\n");
      } else if (cl->GetDeclFileName() && strlen(cl->GetDeclFileName()) ) {
         // Actually we probably should look for the file ..
         const char *filename = cl->GetDeclFileName();

         if (!filename) return;

#ifdef R__WIN32
         TString inclPath("include;prec_stl"); // GetHtml()->GetIncludePath());
#else
         TString inclPath("include:prec_stl"); // GetHtml()->GetIncludePath());
#endif
         Ssiz_t posDelim = 0;
         TString inclDir;
         TString sIncl(filename);
#ifdef R__WIN32
         const char* pdelim = ";";
         static const char ddelim = '\\';
#else
         const char* pdelim = ":";
         static const char ddelim = '/';
#endif
         while (inclPath.Tokenize(inclDir, posDelim, pdelim))
         {
            if (sIncl.BeginsWith(inclDir)) {
               filename += inclDir.Length();
               if (filename[0] == ddelim || filename[0] == '/') {
                  ++filename;
               }
               break;
            }
         }
         directive = Form("#include \"%s\"\n",filename);
      }
      if (directive.Length()) {
         TIter i( &fListOfHeaders );
         for(TNamed *n = (TNamed*) i(); n; n = (TNamed*)i() ) {
            if (directive == n->GetTitle()) {
               return;
            }
         }
         fListOfHeaders.Add(new TNamed(cl->GetName(),directive.Data()));
      }
   }

   void TTreeProxyGenerator::AddHeader(const char *classname)
   {
      // Add a header inclusion request.

      AddHeader(TClass::GetClass(classname));
   }

   void TTreeProxyGenerator::AddPragma(const char *pragma_text)
   {
      // Add a forward declaration request.

      TIter i( &fListOfPragmas );
      for(TObjString *n = (TObjString*) i(); n; n = (TObjString*)i() ) {
         if (pragma_text == n->GetString()) {
            return;
         }
      }

      fListOfPragmas.Add( new TObjString( pragma_text ) );

   }

   void TTreeProxyGenerator::AddDescriptor(TBranchProxyDescriptor *desc)
   {
      // Add a branch descriptor.

      if (desc) {
         TBranchProxyDescriptor *existing =
            (TBranchProxyDescriptor*)((*fCurrentListOfTopProxies)(desc->GetName()));
         if (existing) {
            Warning("TTreeProxyGenerator","The branch name \"%s\" is duplicated. Only the first instance \n"
               "\twill be available directly. The other instance(s) might be available via their complete name\n"
               "\t(including the name of their mother branche's name).",desc->GetName());
         } else {
            fCurrentListOfTopProxies->Add(desc);
            UInt_t len = strlen(desc->GetTypeName());
            if ((len+2)>fMaxDatamemberType) fMaxDatamemberType = len+2;
         }
      }
   }

static TString GetContainedClassName(TBranchElement *branch, TStreamerElement *element, Bool_t ispointer)
{
   TString cname = branch->GetClonesName();
   if (cname.Length()==0) {
      // We may have any unsplit clones array
      Long64_t i = branch->GetTree()->GetReadEntry();
      if (i<0) i = 0;
      branch->GetEntry(i);
      char *obj = branch->GetObject();


      TBranchElement *parent = (TBranchElement*)branch->GetMother()->GetSubBranch(branch);
      const char *pclname = parent->GetClassName();

      TClass *clparent = TClass::GetClass(pclname);
      // TClass *clm = TClass::GetClass(GetClassName());
      Int_t lOffset = 0; // offset in the local streamerInfo.
      if (clparent) {
         const char *ename = 0;
         if (element) {
            ename = element->GetName();
            lOffset = clparent->GetStreamerInfo()->GetOffset(ename);
         } else {
            lOffset = 0;
         }
      }
      else Error("AnalyzeBranch", "Missing parent for %s.", branch->GetName());

      TClonesArray *arr;
      if (ispointer) {
         arr = (TClonesArray*)*(void**)(obj+lOffset);
      } else {
         arr = (TClonesArray*)(obj+lOffset);
      }
      cname = arr->GetClass()->GetName();

   }
   if (cname.Length()==0) {
      Error("AnalyzeBranch",
         "Introspection of TClonesArray in older file not implemented yet.");
   }
   return cname;
}

static TVirtualStreamerInfo *GetStreamerInfo(TBranch *branch, TIter current, TClass *cl)
{
   // Return the correct TStreamerInfo of class 'cname' in the list of
   // branch (current) [Assuming these branches correspond to a flattened
   // version of the class.

   TVirtualStreamerInfo *objInfo = 0;
   TBranchElement *b = 0;
   TString cname = cl->GetName();
   while( ( b = (TBranchElement*)current() ) ) {
      if ( cname == b->GetInfo()->GetName() ) {
         objInfo = b->GetInfo();
         break;
      }
   }
   if (objInfo == 0 && branch->GetTree()->GetDirectory()->GetFile()) {
      TVirtualStreamerInfo *i = (TVirtualStreamerInfo *)branch->GetTree()->GetDirectory()->GetFile()->GetStreamerInfoCache()->FindObject(cname);
      if (i) {
         // NOTE: Is this correct for Foreigh classes?
         objInfo = (TVirtualStreamerInfo *)cl->GetStreamerInfo(i->GetClassVersion());
      }
   }
   return objInfo;
}

static TVirtualStreamerInfo *GetBaseClass(TStreamerElement *element)
{
   TStreamerBase *base = dynamic_cast<TStreamerBase*>(element);
   if (base) {
      TVirtualStreamerInfo *info = base->GetClassPointer()->GetStreamerInfo(base->GetBaseVersion());
      if (info) return info;
   }
   return 0;
}

   UInt_t TTreeProxyGenerator::AnalyzeBranches(UInt_t level,TBranchProxyClassDescriptor *topdesc,
                                               TBranchElement *branch, TVirtualStreamerInfo *info)
   {
      // Analyze the sub-branch and populate the TTreeProxyGenerator or the topdesc with
      // its findings.

      if (info==0) info = branch->GetInfo();

      TIter branches( branch->GetListOfBranches() );

      return AnalyzeBranches( level, topdesc, branches, info );
   }

   UInt_t TTreeProxyGenerator::AnalyzeBranches(UInt_t level,
                                               TBranchProxyClassDescriptor *topdesc,
                                               TIter &branches,
                                               TVirtualStreamerInfo *info)
   {
      // Analyze the list of sub branches of a TBranchElement by looping over
      // the streamer elements and create the appropriate class proxies.

/*

   Find the content class name (GetClassName)
   Record wether this is a collection or not

   Find the StreamerInfo

   For each streamerelement
      if element is base
         if name match, loop over subbranches
         otherwise loop over current branches
      else if eleement is object (or pointer to object?)
         if name match go ahead, loop over subbranches
         if name does not match. loop over current branches (fix names).
      else
         add branch.

*/
      UInt_t lookedAt = 0;
      EContainer container = kNone;
      TString middle;
      TString proxyTypeName;
      TBranchProxyClassDescriptor::ELocation isclones = TBranchProxyClassDescriptor::kOut;
      TString containerName;
      TString subBranchPrefix;
      Bool_t skipped = false;

      {
         TIter peek = branches;
         TBranchElement *branch = (TBranchElement*)peek();
         if (topdesc && topdesc->IsClones()) {
            container = kClones;
            middle = "Cla";
            isclones = TBranchProxyClassDescriptor::kClones;
            containerName = "TClonesArray";
         } else if (topdesc && topdesc->IsSTL()) {
            container = kSTL;
            middle = "Stl";
            isclones = TBranchProxyClassDescriptor::kSTL;
            containerName = topdesc->GetContainerName();
         } else if (!topdesc && branch && branch->GetBranchCount() == branch->GetMother()) {
            if ( ((TBranchElement*)(branch->GetMother()))->GetType()==3)  {
               container = kClones;
               middle = "Cla";
               isclones = TBranchProxyClassDescriptor::kClones;
               containerName = "TClonesArray";
            } else {
               container = kSTL;
               middle = "Stl";
               isclones = TBranchProxyClassDescriptor::kSTL;
               containerName = branch->GetMother()->GetClassName();
            }
         } else if (branch->GetType() == 3) {
            isclones = TBranchProxyClassDescriptor::kClones;
            containerName = "TClonesArray";
         } else if (branch->GetType() == 4) {
            isclones = TBranchProxyClassDescriptor::kSTL;
            containerName = branch->GetMother()->GetSubBranch(branch)->GetClassName();
         }
         if (topdesc) {
            subBranchPrefix = topdesc->GetSubBranchPrefix();
         } else {
            TBranchElement *mom = (TBranchElement*)branch->GetMother();
            subBranchPrefix = mom->GetName();
            if (subBranchPrefix[subBranchPrefix.Length()-1]=='.') {
               subBranchPrefix.Remove(subBranchPrefix.Length()-1);
            } else if (mom->GetType()!=3 && mom->GetType() != 4) {
               subBranchPrefix = "";
            }
         }
      }
      TIter elements( info->GetElements() );
      for( TStreamerElement *element = (TStreamerElement*)elements();
           element;
           element = (TStreamerElement*)elements() )
      {
         Bool_t isBase = false;
         Bool_t usedBranch = kTRUE;
         TString prefix;
         TIter peek = branches;
         TBranchElement *branch = (TBranchElement*)peek();

         if (branch==0) {
            Error("AnalyzeBranches","Ran out of branches when looking in branch %s, class %s",
                  topdesc->GetBranchName(), info->GetName());
            return lookedAt;
         }

         if (info->GetClass()->GetCollectionProxy() && strcmp(element->GetName(),"This")==0) {
            // Skip the artifical streamer element.
            continue;
         }

         if (element->GetType()==-1) {
            // This is an ignored TObject base class.
            continue;
         }

         TString branchname = branch->GetName();
         TString branchEndname;
         {
            TLeaf *leaf = (TLeaf*)branch->GetListOfLeaves()->At(0);
            if (leaf && !isclones && !(branch->GetType() == 3)) branchEndname = leaf->GetName();
            else branchEndname = branch->GetName();
            Int_t pos;
            pos = branchEndname.Index(".");
            if (pos!=-1) {
               if (subBranchPrefix.Length() &&
                  branchEndname.BeginsWith( subBranchPrefix ) ) {
                     // brprefix += topdesc->GetSubBranchPrefix();
                  branchEndname.Remove(0,subBranchPrefix.Length()+1);
               }
            }
         }

         Bool_t ispointer = false;
         switch(element->GetType()) {

            case TVirtualStreamerInfo::kBool:    { proxyTypeName = "T" + middle + "BoolProxy"; break; }
            case TVirtualStreamerInfo::kChar:    { proxyTypeName = "T" + middle + "CharProxy"; break; }
            case TVirtualStreamerInfo::kShort:   { proxyTypeName = "T" + middle + "ShortProxy"; break; }
            case TVirtualStreamerInfo::kInt:     { proxyTypeName = "T" + middle + "IntProxy"; break; }
            case TVirtualStreamerInfo::kLong:    { proxyTypeName = "T" + middle + "LongProxy"; break; }
            case TVirtualStreamerInfo::kLong64:  { proxyTypeName = "T" + middle + "Long64Proxy"; break; }
            case TVirtualStreamerInfo::kFloat:   { proxyTypeName = "T" + middle + "FloatProxy"; break; }
            case TVirtualStreamerInfo::kFloat16: { proxyTypeName = "T" + middle + "Float16Proxy"; break; }
            case TVirtualStreamerInfo::kDouble:  { proxyTypeName = "T" + middle + "DoubleProxy"; break; }
            case TVirtualStreamerInfo::kDouble32:{ proxyTypeName = "T" + middle + "Double32Proxy"; break; }
            case TVirtualStreamerInfo::kUChar:   { proxyTypeName = "T" + middle + "UCharProxy"; break; }
            case TVirtualStreamerInfo::kUShort:  { proxyTypeName = "T" + middle + "UShortProxy"; break; }
            case TVirtualStreamerInfo::kUInt:    { proxyTypeName = "T" + middle + "UIntProxy"; break; }
            case TVirtualStreamerInfo::kULong:   { proxyTypeName = "T" + middle + "ULongProxy"; break; }
            case TVirtualStreamerInfo::kULong64: { proxyTypeName = "T" + middle + "ULong64Proxy"; break; }
            case TVirtualStreamerInfo::kBits:    { proxyTypeName = "T" + middle + "UIntProxy"; break; }

            case TVirtualStreamerInfo::kCharStar: { proxyTypeName = GetArrayType(element,"Char",container); break; }

               // array of basic types  array[8]
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kBool:    { proxyTypeName = GetArrayType(element,"Bool",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kChar:    { proxyTypeName = GetArrayType(element,"Char",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kShort:   { proxyTypeName = GetArrayType(element,"Short",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kInt:     { proxyTypeName = GetArrayType(element,"Int",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kLong:    { proxyTypeName = GetArrayType(element,"Long",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kLong64:  { proxyTypeName = GetArrayType(element,"Long64",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kFloat:   { proxyTypeName = GetArrayType(element,"Float",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kFloat16: { proxyTypeName = GetArrayType(element,"Float16",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kDouble:  { proxyTypeName = GetArrayType(element,"Double",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kDouble32:{ proxyTypeName = GetArrayType(element,"Double32",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUChar:   { proxyTypeName = GetArrayType(element,"UChar",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUShort:  { proxyTypeName = GetArrayType(element,"UShort",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUInt:    { proxyTypeName = GetArrayType(element,"UInt",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kULong:   { proxyTypeName = GetArrayType(element,"ULong",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kULong64: { proxyTypeName = GetArrayType(element,"ULong64",container ); break; }
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kBits:    { proxyTypeName = GetArrayType(element,"UInt",container ); break; }

               // pointer to an array of basic types  array[n]
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kBool:    { proxyTypeName = GetArrayType(element,"Bool",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kChar:    { proxyTypeName = GetArrayType(element,"Char",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kShort:   { proxyTypeName = GetArrayType(element,"Short",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kInt:     { proxyTypeName = GetArrayType(element,"Int",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kLong:    { proxyTypeName = GetArrayType(element,"Long",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kLong64:  { proxyTypeName = GetArrayType(element,"Long64",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kFloat:   { proxyTypeName = GetArrayType(element,"Float",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kFloat16: { proxyTypeName = GetArrayType(element,"Float16",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kDouble:  { proxyTypeName = GetArrayType(element,"Double",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kDouble32:{ proxyTypeName = GetArrayType(element,"Double32",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUChar:   { proxyTypeName = GetArrayType(element,"UChar",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUShort:  { proxyTypeName = GetArrayType(element,"UShort",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUInt:    { proxyTypeName = GetArrayType(element,"UInt",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kULong:   { proxyTypeName = GetArrayType(element,"ULong",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kULong64: { proxyTypeName = GetArrayType(element,"ULong64",container ); break; }
            case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kBits:    { proxyTypeName = GetArrayType(element,"UInt",container ); break; }

               // array counter //[n]
            case TVirtualStreamerInfo::kCounter: { proxyTypeName = "T" + middle + "IntProxy"; break; }


            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObjectp:
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObjectP:
            case TVirtualStreamerInfo::kObjectp:
            case TVirtualStreamerInfo::kObjectP:
            case TVirtualStreamerInfo::kAnyp:
            case TVirtualStreamerInfo::kAnyP:
            case TVirtualStreamerInfo::kSTL + TVirtualStreamerInfo::kObjectp:
            case TVirtualStreamerInfo::kSTL + TVirtualStreamerInfo::kObjectP:
            // set as pointers and fall through to the next switches
               ispointer = true;
            case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObject:
            case TVirtualStreamerInfo::kObject:
            case TVirtualStreamerInfo::kTString:
            case TVirtualStreamerInfo::kTNamed:
            case TVirtualStreamerInfo::kTObject:
            case TVirtualStreamerInfo::kAny:
            case TVirtualStreamerInfo::kBase:
            case TVirtualStreamerInfo::kSTL: {
               TClass *cl = element->GetClassPointer();
               R__ASSERT(cl);

               proxyTypeName = Form("T%sObjProxy<%s >", middle.Data(), cl->GetName());
               TString cname = cl->GetName();
               if (cl==TClonesArray::Class()) {
                  isclones = TBranchProxyClassDescriptor::kClones;
                  cname = GetContainedClassName(branch, element, ispointer);
                  containerName = "TClonesArray";
               } else if (cl->GetCollectionProxy()) {
                  isclones = TBranchProxyClassDescriptor::kSTL;
                  containerName = cl->GetName();
                  TClass *valueClass = cl->GetCollectionProxy()->GetValueClass();
                  if (valueClass) cname = valueClass->GetName();
                  else {
                     proxyTypeName = Form("TStlSimpleProxy<%s >", cl->GetName());
//                   AddPragma(Form("#pragma create TClass %s;\n", cl->GetName()));
                     AddPragma(Form("#pragma link C++ class %s;\n", cl->GetName()));
                  }
               }

               TBranch *parent = branch->GetMother()->GetSubBranch(branch);
               TVirtualStreamerInfo *objInfo = 0;
               if (branch->GetListOfBranches()->GetEntries()) {
                  objInfo = ((TBranchElement*)branch->GetListOfBranches()->At(0))->GetInfo();
               } else {
                  objInfo = branch->GetInfo();
               }
               if (element->IsBase()) {
                  isBase = true;
                  prefix  = "base";

                  if (cl == TObject::Class() && info->GetClass()->CanIgnoreTObjectStreamer())
                  {
                     continue;
                  }

                  TBranchProxyClassDescriptor *cldesc = 0;

                  if (branchEndname == element->GetName()) {
                     // We have a proper node for the base class, recurse

                     if (branch->GetListOfBranches()->GetEntries() == 0) {
                        // The branch contains a non-split base class that we are unfolding!

                        // See AnalyzeTree for similar code!
                        TBranchProxyClassDescriptor *cldesc = 0;

                        TVirtualStreamerInfo *info = branch->GetInfo();
                        if (strcmp(cl->GetName(),info->GetName())!=0) {
                           info = cl->GetStreamerInfo(); // might be the wrong version
                        }
                        cldesc = new TBranchProxyClassDescriptor(cl->GetName(), info,
                                                                 branch->GetName(),
                                                                 isclones, 0 /* unsplit object */, 
                                                                 containerName);

                        TStreamerElement *elem = 0;

                        TIter next(info->GetElements());
                        while( (elem = (TStreamerElement*)next()) ) {
                           AnalyzeElement(branch,elem,level+1,cldesc,"");
                        }

                     } else {

                        Int_t pos = branchname.Last('.');
                        if (pos != -1) {
                           branchname.Remove(pos);
                        }
                        TString prefix = topdesc ? topdesc->GetSubBranchPrefix() : parent->GetName();
                        cldesc = new TBranchProxyClassDescriptor(cl->GetName(), objInfo,
                                                                 branchname,
                                                                 prefix,
                                                                 isclones, branch->GetSplitLevel(),
                                                                 containerName);
                        lookedAt += AnalyzeBranches( level+1, cldesc, branch, objInfo);
                     }
                  } else {
                     // We do not have a proper node for the base class, we need to loop over
                     // the next branches
                     Int_t pos = branchname.Last('.');
                     if (pos != -1) {
                        branchname.Remove(pos);
                     }
                     TString prefix = topdesc ? topdesc->GetSubBranchPrefix() : parent->GetName();
                     objInfo = GetBaseClass( element );
                     if (objInfo == 0) {
                        // There is no data in this base class
                        continue;
                     }
                     cl = objInfo->GetClass();
                     cldesc = new TBranchProxyClassDescriptor(cl->GetName(), objInfo,
                                                              branchname,
                                                              prefix,
                                                              isclones, branch->GetSplitLevel(),
                                                              containerName);
                     usedBranch = kFALSE;
                     lookedAt += AnalyzeBranches( level, cldesc, branches, objInfo );
                  }

                  TBranchProxyClassDescriptor *added = AddClass(cldesc);
                  if (added) proxyTypeName = added->GetName();

               } else {
                  TBranchProxyClassDescriptor *cldesc = 0;

                  if (branchEndname == element->GetName()) {
                     // We have a proper node for the base class, recurse
                     if (branch->GetListOfBranches()->GetEntries() == 0) {
                        // The branch contains a non-split object that we are unfolding!

                        // See AnalyzeTree for similar code!
                        TBranchProxyClassDescriptor *cldesc = 0;

                        TVirtualStreamerInfo *info = branch->GetInfo();
                        if (strcmp(cl->GetName(),info->GetName())!=0) {
                           info = cl->GetStreamerInfo(); // might be the wrong version
                        }
                        cldesc = new TBranchProxyClassDescriptor(cl->GetName(), info,
                                                                 branch->GetName(),
                                                                 isclones, 0 /* unsplit object */,
                                                                 containerName);

                        TStreamerElement *elem = 0;

                        TIter next(info->GetElements());
                        while( (elem = (TStreamerElement*)next()) ) {
                           AnalyzeElement(branch,elem,level+1,cldesc,"");
                        }

                     } else {

                        if (isclones) {
                           // We have to guess the version number!
                           cl = TClass::GetClass(cname);
                           objInfo = GetStreamerInfo(branch,branch->GetListOfBranches(),cl);
                        }
                        cldesc = new TBranchProxyClassDescriptor(cl->GetName(), objInfo,
                                                                 branch->GetName(),
                                                                 branch->GetName(),
                                                                 isclones, branch->GetSplitLevel(),
                                                                 containerName);
                        lookedAt += AnalyzeBranches( level+1, cldesc, branch, objInfo);
                     }
                  } else {
                     // We do not have a proper node for the base class, we need to loop over
                     // the next branches
                     TString prefix = topdesc ? topdesc->GetSubBranchPrefix() : parent->GetName();
                     if (prefix.Length()) prefix += ".";
                     prefix += element->GetName();
                     objInfo = branch->GetInfo();
                     Int_t pos = branchname.Last('.');
                     if (pos != -1) {
                        branchname.Remove(pos);
                     }
                     if (isclones) {
                        // We have to guess the version number!
                        cl = TClass::GetClass(cname);
                        objInfo = GetStreamerInfo(branch, branches, cl);
                     }
                     cldesc = new TBranchProxyClassDescriptor(cl->GetName(), objInfo,
                                                              branchname,
                                                              prefix,
                                                              isclones, branch->GetSplitLevel(),
                                                              containerName);
                     usedBranch = kFALSE;
                     skipped = kTRUE;
                     lookedAt += AnalyzeBranches( level, cldesc, branches, objInfo );
                  }

                  TBranchProxyClassDescriptor *added = AddClass(cldesc);
                  if (added) proxyTypeName = added->GetName();

               }

               AddForward(cl);
               AddHeader(cl);
               break;
            }

            default:
               Error("AnalyzeBranch",
                     "Unsupported type for %s (%d).",
                     branch->GetName(),element->GetType());

         }

         TString dataMemberName = element->GetName();
         if (topdesc) {
            topdesc->AddDescriptor(  new TBranchProxyDescriptor( dataMemberName.Data(),
               proxyTypeName, branchname, true, skipped ), isBase );
         } else {
            dataMemberName.Prepend(prefix);
            AddDescriptor( new TBranchProxyDescriptor( dataMemberName.Data(),
               proxyTypeName, branchname, true, skipped ) );
         }

         if (usedBranch) {
            branches.Next();
            ++lookedAt;
         }
      }
      return lookedAt;
   }

   UInt_t TTreeProxyGenerator::AnalyzeOldLeaf(TLeaf *leaf, UInt_t /* level */,
                                              TBranchProxyClassDescriptor *topdesc)
   {
      // Analyze the leaf and populate the `TTreeProxyGenerator or
      // the topdesc with its findings.

      if (leaf->IsA()==TLeafObject::Class()) {
         Error("AnalyzeOldLeaf","TLeafObject not supported yet");
         return 0;
      }

      TString leafTypeName = leaf->GetTypeName();
      Int_t pos = leafTypeName.Last('_');
      if (pos!=-1) leafTypeName.Remove(pos);

      Int_t len = leaf->GetLen();
      TLeaf *leafcount = leaf->GetLeafCount();

      UInt_t dim = 0;
      std::vector<Int_t> maxDim;
      //maxDim[0] = maxDim[1] = maxDim[2] = 1;

      TString dimensions;
      TString temp = leaf->GetName();
      pos = temp.Index("[");
      if (pos!=-1) {
         if (pos) temp.Remove(0,pos);
         dimensions.Append(temp);
      }
      temp = leaf->GetTitle();
      pos = temp.Index("[");
      if (pos!=-1) {
         if (pos) temp.Remove(0,pos);
         dimensions.Append(temp);
      }

      Int_t dimlen = dimensions.Length();

      if (dimlen) {
         const char *current = dimensions.Data();

         Int_t index;
         Int_t scanindex ;
         while (current) {
            current++;
            if (current[0] == ']') {
               maxDim.push_back(-1); // maxDim[dim] = -1; // Loop over all elements;
            } else {
               scanindex = sscanf(current,"%d",&index);
               if (scanindex) {
                  maxDim.push_back(index); // maxDim[dim] = index;
               } else {
                  maxDim.push_back(-2); // maxDim[dim] = -2; // Index is calculated via a variable.
               }
            }
            dim ++;
            current = (char*)strstr( current, "[" );
         }

      }
      //char *twodim = (char*)strstr(leaf->GetTitle(),"][");

      if (leafcount) {
         len = leafcount->GetMaximum();
      }


      TString type;
      switch (dim) {
         case 0: {
            type = "T";
            type += leafTypeName;
            type += "Proxy";
            break;
         }
         case 1: {
            type = "TArray";
            type += leafTypeName;
            type += "Proxy";
            break;
         }
         default: {
            type = "TArrayProxy<";
            for(Int_t ind = dim - 2; ind > 0; --ind) {
               type += "TMultiArrayType<";
            }
            type += "TArrayType<";
            type += leaf->GetTypeName();
            type += ",";
            type += maxDim[dim-1];
            type += "> ";
            for(Int_t ind = dim - 2; ind > 0; --ind) {
               type += ",";
               type += maxDim[ind];
               type += "> ";
            }
            type += ">";
            break;
         }
      }

      TString branchName = leaf->GetBranch()->GetName();
      TString dataMemberName = leaf->GetName();

      TBranchProxyDescriptor *desc;
      if (topdesc) {
         topdesc->AddDescriptor( desc = new TBranchProxyDescriptor( dataMemberName.Data(),
                                                                    type,
                                                                    branchName.Data() ),
                                 0 );
      } else {
         AddDescriptor( desc = new TBranchProxyDescriptor( dataMemberName.Data(),
                                                           type,
                                                           branchName.Data() ) );
      }

      return 0;

   }

   UInt_t TTreeProxyGenerator::AnalyzeOldBranch(TBranch *branch, UInt_t level,
                                                TBranchProxyClassDescriptor *topdesc)
   {
      // Analyze the branch and populate the TTreeProxyGenerator or the topdesc with
      // its findings.  Sometimes several branch of the mom are also analyzed,
      // the number of such branches is returned (this happens in the case of
      // embedded objects inside an object inside a clones array split more than
      // one level.

      UInt_t extraLookedAt = 0;
      TString prefix;

      TString branchName = branch->GetName();

      TObjArray *leaves = branch->GetListOfLeaves();
      Int_t nleaves = leaves ? leaves->GetEntriesFast() : 0;

      if (nleaves>1) {

         // Create a holder
         TString type = "unknown";
         TBranchProxyClassDescriptor *cldesc = new TBranchProxyClassDescriptor(branch->GetName());
         TBranchProxyClassDescriptor *added = AddClass(cldesc);
         if (added) type = added->GetName();

         for(int l=0;l<nleaves;l++) {
            TLeaf *leaf = (TLeaf*)leaves->UncheckedAt(l);
            extraLookedAt += AnalyzeOldLeaf(leaf,level+1,cldesc);
         }

         TString dataMemberName = branchName;

         TBranchProxyDescriptor *desc;
         if (topdesc) {
            topdesc->AddDescriptor( desc = new TBranchProxyDescriptor( dataMemberName.Data(),
                                                                       type,
                                                                       branchName.Data() ),
                                    0 );
         } else {
            // leafname.Prepend(prefix);
            AddDescriptor( desc = new TBranchProxyDescriptor( dataMemberName.Data(),
                                                              type,
                                                              branchName.Data() ) );
         }

      } else {

         TLeaf *leaf = (TLeaf*)leaves->UncheckedAt(0);
         extraLookedAt += AnalyzeOldLeaf(leaf,level,topdesc);

      }


      return extraLookedAt;

   }

   void TTreeProxyGenerator::AnalyzeTree(TTree *tree)
   {
      // Analyze a TTree and its (potential) friends.

      TIter next( tree->GetListOfBranches() );
      TBranch *branch;
      while ( (branch = (TBranch*)next()) ) {
         TVirtualStreamerInfo *info = 0;
         const char *branchname = branch->GetName();
         const char *classname = branch->GetClassName();
         if (classname && strlen(classname)) {
            AddForward( classname );
            AddHeader( classname );
         }

         TBranchProxyClassDescriptor *desc = 0;
         TClass *cl = TClass::GetClass(classname);
         TString type = "unknown";
         if (cl) {
            TBranchProxyClassDescriptor::ELocation isclones = TBranchProxyClassDescriptor::kOut;
            TString containerName = "";
            if (cl==TClonesArray::Class()) {
               isclones = TBranchProxyClassDescriptor::kClones;
               containerName = "TClonesArray";
               if (branch->IsA()==TBranchElement::Class()) {
                  const char *cname = ((TBranchElement*)branch)->GetClonesName();
                  TClass *ncl = TClass::GetClass(cname);
                  if (ncl) {
                     cl = ncl;
                     info = GetStreamerInfo(branch,branch->GetListOfBranches(),cl);
                  } else {
                     Error("AnalyzeTree",
                           "Introspection of TClonesArray in older file not implemented yet.");
                  }
               } else {
                  TClonesArray **ptr = (TClonesArray**)branch->GetAddress();
                  TClonesArray *clones = 0;
                  if (ptr==0) {
                     clones = new TClonesArray;
                     branch->SetAddress(&clones);
                     ptr = &clones;
                  }
                  branch->GetEntry(0);
                  TClass *ncl = *ptr ? (*ptr)->GetClass() : 0;
                  if (ncl) {
                     cl = ncl;
                  } else {
                     Error("AnalyzeTree",
                           "Introspection of TClonesArray for %s failed.",branch->GetName());
                  }
               }
            } else if (cl->GetCollectionProxy()) {
               isclones = TBranchProxyClassDescriptor::kSTL;
               containerName = cl->GetName();
               if (cl->GetCollectionProxy()->GetValueClass()) {
                  cl = cl->GetCollectionProxy()->GetValueClass();
               } else {
                  type = Form("TStlSimpleProxy<%s >", cl->GetName());
                  AddHeader(cl);
                  AddPragma(Form("#pragma link C++ class %s;\n", cl->GetName()));
                  AddDescriptor( new TBranchProxyDescriptor( branchname, type, branchname ) );
                  continue;
               }
            }
            if (cl) {
               if (NeedToEmulate(cl,0) || branchname[strlen(branchname)-1] == '.' || branch->GetSplitLevel()) {
                  TBranchElement *be = dynamic_cast<TBranchElement*>(branch);
                  TVirtualStreamerInfo *info = (be && !isclones) ? be->GetInfo() : cl->GetStreamerInfo(); // the 2nd hand need to be fixed
                  desc = new TBranchProxyClassDescriptor(cl->GetName(), info, branchname,
                     isclones, branch->GetSplitLevel(),containerName);
               } else {
                  type = Form("TObjProxy<%s >",cl->GetName());
               }
            }
         }

         if ( branch->GetListOfBranches()->GetEntries() == 0 ) {

            if (cl) {
               // We have a non-splitted object!

               if (desc) {
                  TVirtualStreamerInfo *info = cl->GetStreamerInfo();
                  TStreamerElement *elem = 0;

                  TIter next(info->GetElements());
                  while( (elem = (TStreamerElement*)next()) ) {
                     AnalyzeElement(branch,elem,1,desc,"");
                  }

                  desc = AddClass(desc);
                  if (desc) {
                     type = desc->GetName();

                     TString dataMemberName = branchname;

                     AddDescriptor( new TBranchProxyDescriptor( dataMemberName, type, branchname ) );
                  }
               }
            } else {

               // We have a top level raw type.
               AnalyzeOldBranch(branch, 0, 0);
            }

         } else {

            // We have a splitted object

            TIter subnext( branch->GetListOfBranches() );
            if (desc) {
               AnalyzeBranches(1,desc,dynamic_cast<TBranchElement*>(branch),info);
            }
            desc = AddClass(desc);
            type = desc->GetName();
            TString dataMemberName = branchname;
            AddDescriptor( new TBranchProxyDescriptor( dataMemberName, type, branchname ) );

            if ( branchname[strlen(branchname)-1] != '.' ) {
               // If there is no dot also include the data member directly

               AnalyzeBranches(1,0,dynamic_cast<TBranchElement*>(branch),info);

               subnext.Reset();
            }

         } // if split or non split
      }

      // Now let's add the TTreeFriend (if any)
      if (tree->GetListOfFriends()) {
         TFriendElement *fe;
         Int_t count = 0;

         TIter nextfriend(tree->GetListOfFriends());
         while ((fe = (TFriendElement*)nextfriend())) {
            TTree *t = fe->GetTree();
            TFriendProxyDescriptor *desc;
            desc = new TFriendProxyDescriptor(t->GetName(), fe->GetName(), count);

            AddFriend( desc );

            fCurrentListOfTopProxies = desc->GetListOfTopProxies();
            AnalyzeTree(t);

            count++;
         }
      }
      fCurrentListOfTopProxies = &fListOfTopProxies;
   }

   void TTreeProxyGenerator::AnalyzeElement(TBranch *branch, TStreamerElement *element,
                                            UInt_t level, TBranchProxyClassDescriptor *topdesc,
                                            const char *path)
   {
      // Analyze the element and populate the TTreeProxyGenerator or the topdesc with
      // its findings.

      TString dataMemberName;
      TString pxDataMemberName;
      TString type;

      // TString prefix;
      Bool_t isBase = false;
      TString cname;
      TString middle;
      TBranchProxyClassDescriptor::ELocation isclones = TBranchProxyClassDescriptor::kOut;
      TString containerName;
      EContainer container = kNone;
      if (topdesc) {
         if (topdesc->IsClones()) {
            container = kClones;
            middle = "Cla";
            isclones = TBranchProxyClassDescriptor::kClones;
            containerName = "TClonesArray";
         } else if (topdesc->IsSTL()) {
            container = kSTL;
            middle = "Stl";
            isclones = TBranchProxyClassDescriptor::kSTL;
            containerName = topdesc->GetContainerName();
         }
      }

      if (!element) return;

      if (strcmp(element->GetName(),"This")==0) {
         // Skip the artifical streamer element.
         return;
      }

      if (element->GetType()==-1) {
         // This is an ignored TObject base class.
         return;
      }


      Bool_t ispointer = false;
      switch(element->GetType()) {

         case TVirtualStreamerInfo::kBool:    { type = "T" + middle + "BoolProxy"; break; }
         case TVirtualStreamerInfo::kChar:    { type = "T" + middle + "CharProxy"; break; }
         case TVirtualStreamerInfo::kShort:   { type = "T" + middle + "ShortProxy"; break; }
         case TVirtualStreamerInfo::kInt:     { type = "T" + middle + "IntProxy"; break; }
         case TVirtualStreamerInfo::kLong:    { type = "T" + middle + "LongProxy"; break; }
         case TVirtualStreamerInfo::kLong64:  { type = "T" + middle + "Long64Proxy"; break; }
         case TVirtualStreamerInfo::kFloat:   { type = "T" + middle + "FloatProxy"; break; }
         case TVirtualStreamerInfo::kFloat16: { type = "T" + middle + "Float16Proxy"; break; }
         case TVirtualStreamerInfo::kDouble:  { type = "T" + middle + "DoubleProxy"; break; }
         case TVirtualStreamerInfo::kDouble32:{ type = "T" + middle + "Double32Proxy"; break; }
         case TVirtualStreamerInfo::kUChar:   { type = "T" + middle + "UCharProxy"; break; }
         case TVirtualStreamerInfo::kUShort:  { type = "T" + middle + "UShortProxy"; break; }
         case TVirtualStreamerInfo::kUInt:    { type = "T" + middle + "UIntProxy"; break; }
         case TVirtualStreamerInfo::kULong:   { type = "T" + middle + "ULongProxy"; break; }
         case TVirtualStreamerInfo::kULong64: { type = "T" + middle + "ULong64Proxy"; break; }
         case TVirtualStreamerInfo::kBits:    { type = "T" + middle + "UIntProxy"; break; }

         case TVirtualStreamerInfo::kCharStar: { type = GetArrayType(element,"Char",container); break; }

            // array of basic types  array[8]
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kBool:    { type = GetArrayType(element,"Bool",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kChar:    { type = GetArrayType(element,"Char",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kShort:   { type = GetArrayType(element,"Short",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kInt:     { type = GetArrayType(element,"Int",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kLong:    { type = GetArrayType(element,"Long",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kLong64:  { type = GetArrayType(element,"Long64",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kFloat:   { type = GetArrayType(element,"Float",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kFloat16: { type = GetArrayType(element,"Float16",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kDouble:  { type = GetArrayType(element,"Double",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kDouble32:{ type = GetArrayType(element,"Double32",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUChar:   { type = GetArrayType(element,"UChar",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUShort:  { type = GetArrayType(element,"UShort",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kUInt:    { type = GetArrayType(element,"UInt",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kULong:   { type = GetArrayType(element,"ULong",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kULong64: { type = GetArrayType(element,"ULong64",container ); break; }
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kBits:    { type = GetArrayType(element,"UInt",container ); break; }

            // pointer to an array of basic types  array[n]
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kBool:    { type = GetArrayType(element,"Bool",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kChar:    { type = GetArrayType(element,"Char",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kShort:   { type = GetArrayType(element,"Short",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kInt:     { type = GetArrayType(element,"Int",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kLong:    { type = GetArrayType(element,"Long",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kLong64:  { type = GetArrayType(element,"Long64",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kFloat:   { type = GetArrayType(element,"Float",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kFloat16: { type = GetArrayType(element,"Float16",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kDouble:  { type = GetArrayType(element,"Double",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kDouble32:{ type = GetArrayType(element,"Double32",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUChar:   { type = GetArrayType(element,"UChar",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUShort:  { type = GetArrayType(element,"UShort",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kUInt:    { type = GetArrayType(element,"UInt",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kULong:   { type = GetArrayType(element,"ULong",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kULong64: { type = GetArrayType(element,"ULong64",container ); break; }
         case TVirtualStreamerInfo::kOffsetP + TVirtualStreamerInfo::kBits:    { type = GetArrayType(element,"UInt",container ); break; }

            // array counter //[n]
         case TVirtualStreamerInfo::kCounter: { type = "T" + middle + "IntProxy"; break; }


         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObjectp:
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObjectP:
         case TVirtualStreamerInfo::kObjectp:
         case TVirtualStreamerInfo::kObjectP:
         case TVirtualStreamerInfo::kAnyp:
         case TVirtualStreamerInfo::kAnyP:
         case TVirtualStreamerInfo::kSTL + TVirtualStreamerInfo::kObjectp:
         case TVirtualStreamerInfo::kSTL + TVirtualStreamerInfo::kObjectP:
            // set as pointers and fall through to the next switches
            ispointer = true;
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kObject:
         case TVirtualStreamerInfo::kObject:
         case TVirtualStreamerInfo::kTString:
         case TVirtualStreamerInfo::kTNamed:
         case TVirtualStreamerInfo::kTObject:
         case TVirtualStreamerInfo::kAny:
         case TVirtualStreamerInfo::kOffsetL + TVirtualStreamerInfo::kAny:
         case TVirtualStreamerInfo::kSTL:
         case TVirtualStreamerInfo::kBase: {
            TClass *cl = element->GetClassPointer();
            if (cl) {
               type = Form("T%sObjProxy<%s >",
                           middle.Data(),cl->GetName());
               cname = cl->GetName();
               if (cl==TClonesArray::Class()) {
                  isclones = TBranchProxyClassDescriptor::kClones;
                  containerName = "TClonesArray";

                  Long64_t i = branch->GetTree()->GetReadEntry();
                  if (i<0) i = 0;
                  branch->GetEntry(i);

                  //char *obj = branch->GetObject();

                  // now need to follow it through to this pointer!

                  TClonesArray *arr;

                  TString fullpath = branch->GetName();
                  fullpath += ".";
                  if (path && strlen(path)>0) fullpath.Append(path).Append(".");
                  fullpath += element->GetName();

                  TTreeFormula *formula = new TTreeFormula("clones",fullpath,branch->GetTree());

                  TFormLeafInfo *leafinfo = formula->GetLeafInfo(0);
                  TLeaf *leaf = formula->GetLeaf(0);
                  R__ASSERT(leaf && leafinfo);

                  arr = (TClonesArray*)leafinfo->GetLocalValuePointer(leaf,0);

                  /*
                    if (ispointer) {
                    arr = (TClonesArray*)*(void**)(obj+lOffset);
                    } else {
                    arr = (TClonesArray*)(obj+lOffset);
                    }
                  */
                  cname = arr->GetClass()->GetName();

                  if (cname.Length()==0) {
                     Error("AnalyzeTree",
                           "Introspection of TClonesArray in older file not implemented yet.");
                  }
                  delete formula;
               } else if (cl->GetCollectionProxy()) {
                  isclones = TBranchProxyClassDescriptor::kSTL;
                  containerName = cl->GetName();
                  cl = cl->GetCollectionProxy()->GetValueClass();
               }
            }
            else Error("AnalyzeTree","missing class for %s.",branch->GetName());
            if (element->IsA()==TStreamerBase::Class()) {
               // prefix  = "base";
               isBase = true;
            }
            AddForward(cl);
            AddHeader(cl);
            break;
         }

         default:
            Error("AnalyzeTree",
                  "Unsupported type for %s %s %d",
                  branch->GetName(), element->GetName(), element->GetType());

      }

      dataMemberName = element->GetName();

      if (level<=fMaxUnrolling) {

         // See AnalyzeTree for similar code!
         TBranchProxyClassDescriptor *cldesc;

         TClass *cl = TClass::GetClass(cname);
         if (cl && cl->CanSplit()) {
            cldesc = new TBranchProxyClassDescriptor(cl->GetName(), cl->GetStreamerInfo(),
                                                     branch->GetName(),
                                                     isclones, 0 /* non-split object */,
                                                     containerName);

            TVirtualStreamerInfo *info = cl->GetStreamerInfo();
            TStreamerElement *elem = 0;

            TString subpath = path;
            if (subpath.Length()>0) subpath += ".";
            subpath += dataMemberName;

            TIter next(info->GetElements());
            while( (elem = (TStreamerElement*)next()) ) {
               AnalyzeElement(branch, elem, level+1, cldesc, subpath.Data());
            }

            TBranchProxyClassDescriptor *added = AddClass(cldesc);
            if (added) type = added->GetName();
         }

      }

      pxDataMemberName = /* prefix + */ dataMemberName;
      TBranchProxyDescriptor *desc;
      if (topdesc) {
         topdesc->AddDescriptor( desc = new TBranchProxyDescriptor( pxDataMemberName.Data(), type,
                                                                    dataMemberName.Data(), false),
                                 isBase );
      } else {
         Error("AnalyzeTree","topdesc should not be null in TTreeProxyGenerator::AnalyzeElement.");
      }
   }

   //----------------------------------------------------------------------------------------------
   void TTreeProxyGenerator::ParseOptions()
   {
      // Parse the options string.

      TString opt = fOptionStr;

      fOptions = 0;
      if ( opt.Contains("nohist") ) {
         opt.ReplaceAll("nohist","");
         fOptions |= kNoHist;
      }
   }

   //----------------------------------------------------------------------------------------------
   static Bool_t R__AddPragmaForClass(TTreeProxyGenerator *gen, TClass *cl)
   {
      // Add the "pragma C++ class" if needed and return
      // true if it has been added _or_ if it is known to
      // not be needed.
      // (I.e. return kFALSE if a container of this class
      // can not have a "pragma C++ class" 
      
      if (!cl) return kFALSE;
      if (cl->GetCollectionProxy()) {
         TClass *valcl = cl->GetCollectionProxy()->GetValueClass();
         if (!valcl) {
            gen->AddPragma(Form("#pragma link C++ class %s;\n", cl->GetName()));
            return kTRUE;
         } else if (R__AddPragmaForClass(gen, valcl)) {
            gen->AddPragma(Form("#pragma link C++ class %s;\n", cl->GetName()));
            return kTRUE;
         }
      } 
      if (cl->IsLoaded()) return kFALSE;
      return kFALSE;
   }

   //----------------------------------------------------------------------------------------------
   static Bool_t R__AddPragmaForClass(TTreeProxyGenerator *gen, const char *classname)
   {
      // Add the "pragma C++ class" if needed and return
      // true if it has been added _or_ if it is known to
      // not be needed.
      // (I.e. return kFALSE if a container of this class
      // can not have a "pragma C++ class" 

      return R__AddPragmaForClass( gen, TClass::GetClass(classname) );

   }

   //----------------------------------------------------------------------------------------------
   void TTreeProxyGenerator::WriteProxy()
   {
      // Check whether the file exist and do something useful if it does
      if (fScript.Length()==0) {
         Error("WriteProxy","No user script has been specified.");
         return;
      }

      TString fileLocation = gSystem->DirName(fScript);

      TString incPath = gSystem->GetIncludePath(); // of the form -Idir1  -Idir2 -Idir3
      incPath.Append(":").Prepend(" ");
      incPath.ReplaceAll(" -I",":");       // of form :dir1 :dir2:dir3
      while ( incPath.Index(" :") != -1 ) {
         incPath.ReplaceAll(" :",":");
      }
      incPath.Prepend(fileLocation+":.:");

      const char *filename = gSystem->Which(incPath,fScript);
      if (filename==0) {
         Error("WriteProxy","Can not find the user's script: %s",fScript.Data());
         return;
      }
      const char *cutfilename = 0;
      if (fCutScript.Length()) {
         fileLocation = gSystem->DirName(fCutScript);
         incPath.Prepend(fileLocation+":.:");
         cutfilename = gSystem->Which(incPath,fCutScript);
         if (cutfilename==0) {
            Error("WriteProxy","Can not find the user's cut script: %s",fCutScript.Data());
            return;
         }
      }

      fHeaderFileName = fPrefix;
      fHeaderFileName.Append(".h");

      // Check to see if the target file exist.
      // If they do we will generate the proxy in temporary file and modify the original
      // if and only if it is different.

      Bool_t updating = kFALSE;
      if (gSystem->GetPathInfo( fHeaderFileName, 0, (Long_t*)0, 0, 0 ) == 0) {
         // file already exist
         updating = kTRUE;
      }

      TString classname = gSystem->BaseName(fPrefix);

      TString treefile;
      Bool_t ischain = fTree->InheritsFrom(TChain::Class());
      if (fTree->GetDirectory() && fTree->GetDirectory()->GetFile())
         treefile = fTree->GetDirectory()->GetFile()->GetName();
      else
         treefile = "Memory Directory";

      TString scriptfunc = fScript;
      Ssiz_t dot_pos = scriptfunc.Last('.');
      if (dot_pos == kNPOS) {
         Error("WriteProxy","User's script (%s) has no extension! Nothing will be written.",scriptfunc.Data());
         return;
      }
      scriptfunc.Replace( dot_pos, fScript.Length()-dot_pos, "");
      TString scriptHeader = scriptfunc;
      const char * extensions[] = { ".h", ".hh", ".hpp", ".hxx",  ".hPP", ".hXX" };

      int i;
      for (i = 0; i < 6; i++ ) {
         TString possible = scriptHeader;
         possible.Append(extensions[i]);
         const char *name = gSystem->Which(incPath,possible);
         if (name) {
            scriptHeader = possible;
            fListOfHeaders.Add(new TNamed("script",Form("#include \"%s\"\n",
                                                        scriptHeader.Data())));
            break;
         }
      }
      scriptfunc = gSystem->BaseName(scriptfunc);


      TString cutscriptfunc = fCutScript;
      if (cutfilename) {
         Ssiz_t dot_pos = cutscriptfunc.Last('.');
         cutscriptfunc.Replace( dot_pos, fCutScript.Length()-dot_pos, "");
         TString cutscriptHeader = cutscriptfunc;
         const char * extensions[] = { ".h", ".hh", ".hpp", ".hxx",  ".hPP", ".hXX" };

         for (i = 0; i < 6; i++ ) {
            TString possible = cutscriptHeader;
            possible.Append(extensions[i]);
            const char *name = gSystem->Which(incPath,possible);
            if (name) {
               cutscriptHeader = possible;
               fListOfHeaders.Add(new TNamed("cutscript",Form("#include \"%s\"\n",
                                                              cutscriptHeader.Data())));
               break;
            }
         }
         cutscriptfunc = gSystem->BaseName(cutscriptfunc);
      }

      FILE *hf;
      TString tmpfilename;
      if (updating) {
         tmpfilename = gSystem->BaseName( tmpnam(0) );
         tmpfilename.Append("_proxy.h");
         hf = fopen(tmpfilename.Data(),"w");
      } else {
         hf = fopen(fHeaderFileName.Data(),"w");
      }
      if (hf == 0) {
         Error("WriteProxy","Unable to open the file %s for writing.",fHeaderFileName.Data());
         return;
      }

      TDatime td;
      fprintf(hf,   "/////////////////////////////////////////////////////////////////////////\n");
      fprintf(hf,   "//   This class has been automatically generated \n");
      fprintf(hf,   "//   (at %s by ROOT version %s)\n",td.AsString(),gROOT->GetVersion());
      if (!ischain) {
         fprintf(hf,"//   from TTree %s/%s\n",fTree->GetName(),fTree->GetTitle());
         fprintf(hf,"//   found on file: %s\n",treefile.Data());
      } else {
         fprintf(hf,"//   from TChain %s/%s\n",fTree->GetName(),fTree->GetTitle());
      }
      fprintf(hf,   "/////////////////////////////////////////////////////////////////////////\n");
      fprintf(hf,"\n");
      fprintf(hf,"\n");

      fprintf(hf,"#ifndef %s_h\n",classname.Data());
      fprintf(hf,"#define %s_h\n",classname.Data());
      fprintf(hf,"\n");


      fprintf(hf,"// System Headers needed by the proxy\n");
      fprintf(hf,"#if defined(__CINT__) && !defined(__MAKECINT__)\n");
      fprintf(hf,"   #define ROOT_Rtypes\n");
      fprintf(hf,"   #define ROOT_TError\n");
      fprintf(hf,"#endif\n");
      fprintf(hf,"#include <TROOT.h>\n");
      fprintf(hf,"#include <TChain.h>\n");
      fprintf(hf,"#include <TFile.h>\n");
      fprintf(hf,"#include <TSelectorDraw.h>\n");
      fprintf(hf,"#include <TPad.h>\n");
      fprintf(hf,"#include <TH1.h>\n");
      fprintf(hf,"#include <TBranchProxy.h>\n");
      fprintf(hf,"#include <TBranchProxyDirector.h>\n");
      fprintf(hf,"#include <TBranchProxyTemplate.h>\n");
      fprintf(hf,"#include <TFriendProxy.h>\n");
      fprintf(hf,"#include <TMethodCall.h>\n\n");
      fprintf(hf,"using namespace ROOT;\n"); // questionable
      fprintf(hf,"\n");

      fprintf(hf,"// forward declarations needed by this particular proxy\n");
      TIter next( &fListOfForwards );
      TObject *current;
      while ( (current=next()) ) {
         if (strstr(current->GetTitle(),"::")==0) {
            // We can not forward declared nested classes (well we might be able to do so for
            // the one nested in a namespace but it is not clear yet if we can really reliably
            // find this information)
            fprintf(hf,current->GetTitle());
         }
      }

      fprintf(hf,"\n\n");
      fprintf(hf,"// Header needed by this particular proxy\n");
      next = &fListOfHeaders;
      TObject *header;
      while ( (header = next()) ) {
         fprintf(hf,header->GetTitle());
      }
      fprintf(hf,"\n\n");

      fprintf(hf, "class %s : public TSelector {\n", classname.Data());
      fprintf(hf, "   public :\n");
      fprintf(hf, "   TTree          *fChain;    //!pointer to the analyzed TTree or TChain\n");
      fprintf(hf, "   TSelectorDraw  *fHelper;   //!helper class to create the default histogram\n");
      fprintf(hf, "   TList          *fInput;    //!input list of the helper\n");
      fprintf(hf, "   TH1            *htemp;     //!pointer to the histogram\n");
      fprintf(hf, "   TBranchProxyDirector  fDirector; //!Manages the proxys\n\n");

      fprintf(hf, "   // Optional User methods\n");
      fprintf(hf, "   TClass         *fClass;    // Pointer to this class's description\n");
      fprintf(hf, "   TMethodCall     fBeginMethod;\n");
      fprintf(hf, "   TMethodCall     fSlaveBeginMethod;\n");
      fprintf(hf, "   TMethodCall     fNotifyMethod;\n");
      fprintf(hf, "   TMethodCall     fProcessMethod;\n");
      fprintf(hf, "   TMethodCall     fSlaveTerminateMethod;\n");
      fprintf(hf, "   TMethodCall     fTerminateMethod;\n");

      if (fListOfClasses.LastIndex()>=0) {
         fprintf(hf, "\n   // Wrapper class for each unwounded class\n");
         next = &fListOfClasses;
         TBranchProxyClassDescriptor *clp;
         while ( (clp = (TBranchProxyClassDescriptor*)next()) ) {
            clp->OutputDecl(hf, 3, fMaxDatamemberType);
         }
      }

      if (fListOfFriends.LastIndex()>=0) {
         fprintf(hf, "\n   // Wrapper class for each friend TTree\n");
         next = &fListOfFriends;
         TFriendProxyDescriptor *clp;
         while ( (clp = (TFriendProxyDescriptor*)next()) ) {
            if (!clp->IsDuplicate()) clp->OutputClassDecl(hf, 3, fMaxDatamemberType);
         }
      }

      fprintf(hf, "\n   // Proxy for each of the branches, leaves and friends of the tree\n");
      next = &fListOfTopProxies;
      TBranchProxyDescriptor *data;
      while ( (data = (TBranchProxyDescriptor*)next()) ) {
         data->OutputDecl(hf, 3, fMaxDatamemberType);
      }
      if (fListOfFriends.LastIndex()>=0) {
         next = &fListOfFriends;
         TFriendProxyDescriptor *clp;
         while ( (clp = (TFriendProxyDescriptor*)next()) ) {
            clp->OutputDecl(hf, 3, fMaxDatamemberType);
         }
      }
      fprintf(hf,"\n\n");

      // Constructor
      fprintf(hf,      "   %s(TTree *tree=0) : \n",classname.Data());
      fprintf(hf,      "      fChain(0)");
      fprintf(hf,   ",\n      fHelper(0)");
      fprintf(hf,   ",\n      fInput(0)");
      fprintf(hf,   ",\n      htemp(0)");
      fprintf(hf,   ",\n      fDirector(tree,-1)");
      fprintf(hf,   ",\n      fClass                (TClass::GetClass(\"%s\"))",classname.Data());
      fprintf(hf,   ",\n      fBeginMethod          (fClass,\"%s_Begin\",\"(TTree*)0\")",scriptfunc.Data());
      fprintf(hf,   ",\n      fSlaveBeginMethod     (fClass,\"%s_SlaveBegin\",\"(TTree*)0\")",scriptfunc.Data());
      fprintf(hf,   ",\n      fNotifyMethod         (fClass,\"%s_Notify\",\"\")",scriptfunc.Data());
      fprintf(hf,   ",\n      fProcessMethod        (fClass,\"%s_Process\",\"0\")",scriptfunc.Data());
      fprintf(hf,   ",\n      fSlaveTerminateMethod (fClass,\"%s_SlaveTerminate\",\"\")",scriptfunc.Data());
      fprintf(hf,   ",\n      fTerminateMethod      (fClass,\"%s_Terminate\",\"\")",scriptfunc.Data());
      next = &fListOfTopProxies;
      while ( (data = (TBranchProxyDescriptor*)next()) ) {
         fprintf(hf,",\n      %-*s(&fDirector,\"%s\")",
                 fMaxDatamemberType, data->GetDataName(), data->GetBranchName());
      }
      next = &fListOfFriends;
      TFriendProxyDescriptor *clp;
      while ( (clp = (TFriendProxyDescriptor*)next()) ) {
          fprintf(hf,",\n      %-*s(&fDirector,tree,%d)",
                 fMaxDatamemberType, clp->GetTitle(), clp->GetIndex());
      }

      fprintf(hf,    "\n      { }\n");

      // Other functions.
      fprintf(hf,"   ~%s();\n",classname.Data());
      fprintf(hf,"   Int_t   Version() const {return 1;}\n");
      fprintf(hf,"   void    Begin(::TTree *tree);\n");
      fprintf(hf,"   void    SlaveBegin(::TTree *tree);\n");
      fprintf(hf,"   void    Init(::TTree *tree);\n");
      fprintf(hf,"   Bool_t  Notify();\n");
      fprintf(hf,"   Bool_t  Process(Long64_t entry);\n");
      fprintf(hf,"   void    SetOption(const char *option) { fOption = option; }\n");
      fprintf(hf,"   void    SetObject(TObject *obj) { fObject = obj; }\n");
      fprintf(hf,"   void    SetInputList(TList *input) {fInput = input;}\n");
      fprintf(hf,"   TList  *GetOutputList() const { return fOutput; }\n");
      fprintf(hf,"   void    SlaveTerminate();\n");
      fprintf(hf,"   void    Terminate();\n");
      fprintf(hf,"\n");
      fprintf(hf,"   ClassDef(%s,0);\n",classname.Data());
      fprintf(hf,"\n\n");

      fprintf(hf,"//inject the user's code\n");
      fprintf(hf,"#include \"%s\"\n",fScript.Data());

      if (cutfilename) {
         fprintf(hf,"#include \"%s\"\n",fCutScript.Data());
      }

      // Close the class.
      fprintf(hf,"};\n");
      fprintf(hf,"\n");
      fprintf(hf,"#endif\n");
      fprintf(hf,"\n\n");

      fprintf(hf,"#ifdef __MAKECINT__\n");
      if (fListOfClasses.LastIndex()>=0) {
         TBranchProxyClassDescriptor *clp;
         next = &fListOfClasses;
         while ( (clp = (TBranchProxyClassDescriptor*)next()) ) {
            fprintf(hf,"#pragma link C++ class %s::%s-;\n",classname.Data(),clp->GetName());
            if (clp->GetContainerName().Length()) {
               R__AddPragmaForClass(this, clp->GetContainerName());
            }
         }
         next = &fListOfPragmas;
         TObjString *prag;
         while ( (prag = (TObjString*)next()) ) {
            fprintf(hf,"%s",prag->String().Data());
         }
      }
      fprintf(hf,"#pragma link C++ class %s;\n",classname.Data());
      fprintf(hf,"#endif\n");
      fprintf(hf,"\n\n");

      // Write the implementations.
      fprintf(hf,"inline %s::~%s() {\n",classname.Data(),classname.Data());
      fprintf(hf,"   // destructor. Clean up helpers.\n");
      fprintf(hf,"\n");
      fprintf(hf,"   delete fHelper;\n");
      fprintf(hf,"   delete fInput;\n");
      fprintf(hf,"}\n");
      fprintf(hf,"\n");
      fprintf(hf,"inline void %s::Init(TTree *tree)\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"//   Set branch addresses\n");
      fprintf(hf,"   if (tree == 0) return;\n");
      fprintf(hf,"   fChain = tree;\n");
      fprintf(hf,"   fDirector.SetTree(fChain);\n");
      fprintf(hf,"   delete fHelper;\n");
      fprintf(hf,"   fHelper = new TSelectorDraw();\n");
      fprintf(hf,"   delete fInput;\n");
      fprintf(hf,"   fInput  = new TList();\n");
      fprintf(hf,"   fInput->SetOwner();\n");
      fprintf(hf,"   fInput->Add(new TNamed(\"varexp\",\"0.0\")); // Fake a double size histogram\n");
      fprintf(hf,"   fInput->Add(new TNamed(\"selection\",\"\"));\n");
      fprintf(hf,"   fHelper->SetInputList(fInput);\n");
      fprintf(hf,"}\n");
      fprintf(hf,"\n");
      fprintf(hf,"Bool_t %s::Notify()\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"   // Called when loading a new file.\n");
      fprintf(hf,"   // Get branch pointers.\n");
      fprintf(hf,"   fDirector.SetTree(fChain);\n");
      fprintf(hf,"   if (fNotifyMethod.IsValid()) fNotifyMethod.Execute(this);\n");
      fprintf(hf,"   \n");
      fprintf(hf,"   return kTRUE;\n");
      fprintf(hf,"}\n");
      fprintf(hf,"   \n");

      // generate code for class member function Begin
      fprintf(hf,"\n");
      fprintf(hf,"inline void %s::Begin(TTree *tree)\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"   // The Begin() function is called at the start of the query.\n");
      fprintf(hf,"   // When running with PROOF Begin() is only called on the client.\n");
      fprintf(hf,"   // The tree argument is deprecated (on PROOF 0 is passed).\n");
      fprintf(hf,"\n");
      fprintf(hf,"   TString option = GetOption();\n");
      fprintf(hf,"   if (fBeginMethod.IsValid()) fBeginMethod.Execute(this,Form(\"0x%%lx\",tree));\n");
      fprintf(hf,"\n");
      fprintf(hf,"}\n");

      // generate code for class member function SlaveBegin
      fprintf(hf,"\n");
      fprintf(hf,"inline void %s::SlaveBegin(TTree *tree)\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"   // The SlaveBegin() function is called after the Begin() function.\n");
      fprintf(hf,"   // When running with PROOF SlaveBegin() is called on each slave server.\n");
      fprintf(hf,"   // The tree argument is deprecated (on PROOF 0 is passed).\n");
      fprintf(hf,"\n");
      fprintf(hf,"   Init(tree);\n");
      fprintf(hf,"\n");
      fprintf(hf,"   TString option = GetOption();\n");
      fprintf(hf,"   fHelper->SetOption(option);\n");
      fprintf(hf,"   fHelper->Begin(tree);\n");
      fprintf(hf,"   htemp = (TH1*)fHelper->GetObject();\n");
      if (cutfilename) {
         fprintf(hf,"   htemp->SetTitle(\"%s {%s}\");\n",fScript.Data(),fCutScript.Data());
      } else {
         fprintf(hf,"   htemp->SetTitle(\"%s\");\n",fScript.Data());
      }
      fprintf(hf,"   fObject = htemp;\n");
      fprintf(hf,"   if (fSlaveBeginMethod.IsValid()) {\n");
      fprintf(hf,"      fSlaveBeginMethod.Execute(this,Form(\"0x%%lx\",tree));\n");
      fprintf(hf,"   }\n");
      fprintf(hf,"\n");
      fprintf(hf,"}\n");
      fprintf(hf,"\n");

      // generate code for class member function Process
      fprintf(hf,"inline Bool_t %s::Process(Long64_t entry)\n",classname.Data());
      fprintf(hf,"{\n");

      fprintf(hf,"   // The Process() function is called for each entry in the tree (or possibly\n"
              "   // keyed object in the case of PROOF) to be processed. The entry argument\n"
              "   // specifies which entry in the currently loaded tree is to be processed.\n"
              "   // It can be passed to either TTree::GetEntry() or TBranch::GetEntry()\n"
              "   // to read either all or the required parts of the data. When processing\n"
              "   // keyed objects with PROOF, the object is already loaded and is available\n"
              "   // via the fObject pointer.\n"
              "   //\n"
              "   // This function should contain the \"body\" of the analysis. It can contain\n"
              "   // simple or elaborate selection criteria, run algorithms on the data\n"
              "   // of the event and typically fill histograms.\n\n");
      fprintf(hf,"   // WARNING when a selector is used with a TChain, you must use\n");
      fprintf(hf,"   //  the pointer to the current TTree to call GetEntry(entry).\n");
      fprintf(hf,"   //  The entry is always the local entry number in the current tree.\n");
      fprintf(hf,"   //  Assuming that fChain is the pointer to the TChain being processed,\n");
      fprintf(hf,"   //  use fChain->GetTree()->GetEntry(entry).\n");
      fprintf(hf,"\n");
      fprintf(hf,"\n");
      fprintf(hf,"   fDirector.SetReadEntry(entry);\n");
      if (fOptions & kNoHist) {
         if (cutfilename) {
            fprintf(hf,"   if (%s()) %s();\n",cutscriptfunc.Data(),scriptfunc.Data());
         } else {
            fprintf(hf,"   %s();\n",scriptfunc.Data());
         }
      } else {
         if (cutfilename) {
            fprintf(hf,"   if (%s()) htemp->Fill(%s());\n",cutscriptfunc.Data(),scriptfunc.Data());
         } else {
            fprintf(hf,"   htemp->Fill(%s());\n",scriptfunc.Data());
         }
      }
      fprintf(hf,"   if (fProcessMethod.IsValid()) fProcessMethod.Execute(this,Form(\"%%d\",entry));\n");
      fprintf(hf,"   return kTRUE;\n");
      fprintf(hf,"\n");
      fprintf(hf,"}\n\n");

      // generate code for class member function SlaveTerminate
      fprintf(hf,"inline void %s::SlaveTerminate()\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"   // The SlaveTerminate() function is called after all entries or objects\n"
              "   // have been processed. When running with PROOF SlaveTerminate() is called\n"
              "   // on each slave server.");
      fprintf(hf,"\n");
      fprintf(hf,"   if (fSlaveTerminateMethod.IsValid()) fSlaveTerminateMethod.Execute(this);\n");
      fprintf(hf,"}\n\n");

      // generate code for class member function Terminate
      fprintf(hf,"inline void %s::Terminate()\n",classname.Data());
      fprintf(hf,"{\n");
      fprintf(hf,"   // Function called at the end of the event loop.\n");
      fprintf(hf,"   Int_t drawflag = (htemp && htemp->GetEntries()>0);\n");
      fprintf(hf,"   \n");
      fprintf(hf,"   if (!drawflag && !fOption.Contains(\"goff\") && !fOption.Contains(\"same\")) {\n");
      fprintf(hf,"      gPad->Clear();\n");
      fprintf(hf,"   } else {\n");
      fprintf(hf,"      if (fOption.Contains(\"goff\")) drawflag = false;\n");
      fprintf(hf,"      if (drawflag) htemp->Draw(fOption);\n");
      fprintf(hf,"   }\n");
      fprintf(hf,"   if (fTerminateMethod.IsValid()) fTerminateMethod.Execute(this);\n");
      fprintf(hf,"   delete fHelper; fHelper = 0;\n");
      fprintf(hf,"}\n");

      fclose(hf);

      if (updating) {
         // over-write existing file only if needed.
         if (AreDifferent(fHeaderFileName,tmpfilename)) {
            gSystem->Unlink(fHeaderFileName);
            gSystem->Rename(tmpfilename,fHeaderFileName);
         } else gSystem->Unlink(tmpfilename);
      }
   }

}
