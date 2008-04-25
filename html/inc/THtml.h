// @(#)root/html:$Id$
// Author: Nenad Buncic   18/10/95

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_THtml
#define ROOT_THtml


////////////////////////////////////////////////////////////////////////////
//                                                                        //
// THtml                                                                  //
//                                                                        //
// Html generates documentation for all ROOT classes                      //
// using XHTML 1.0 transitional                                           //
//                                                                        //
////////////////////////////////////////////////////////////////////////////

#ifndef ROOT_THashList
#include "THashList.h"
#endif

#include <map>

class TClass;
class TClassDocInfo;
class TVirtualMutex;

class THtml: public TObject {
protected:
   struct TDocSyntax {
      TString        fClassDocTag;     // tag for class documentation
      TString        fAuthorTag;       // tag for author
      TString        fLastUpdateTag;   // tag for last update
      TString        fCopyrightTag;    // tag for copyright
      TString        fDocStyle;        // doc style (only "Doc++" has special treatment)
   };

   struct TLinkInfo {
      TString        fXwho;            // URL for name lookup
      TString        fROOTURL;         // Root URL for ROOT's reference guide for libs that are not in fLibURLs
      std::map<std::string, TString> fLibURLs; // URL for documentation of external libraries
      TString        fHomepage;        // URL of homepage
      TString        fSearchStemURL;   // URL stem used to build search URL
      TString        fSearchEngine;    // link to search engine
      TString        fViewCVS;         // link to ViewCVS; %f is replaced by the filename (no %f: it's appended)
      TString        fWikiURL;         // URL stem of class's wiki page, %c replaced by mangled class name (no %c: appended)
   };

   struct TOutputStyle {
      TString        fHeader;          // header file name
      TString        fFooter;          // footerer file name
      TString        fCharset;         // Charset for doc pages
   };

   struct TPathInfo {
      enum EDotAccess {
         kDotUnknown,
         kDotFound,
         kDotNotFound
      };

      TPathInfo():
         fFoundDot(kDotUnknown),
         fInputPath("./:src/:include/"),
         fDocPath("doc"),
         fMacroPath(".:doc/macros"),
         fIncludePath("include"),
         fOutputDir("htmldoc"),
      EDotAccess     fFoundDot;        // whether dot is accessible
      TString        fInputPath;       // directories to look for classes; prepended to Decl/ImplFileName()
      TString        fDocPath;         // subdir to check for module documentation ("doc" for ROOT)
      TString        fMacroPath;       // subdir for macros run via the Begin/End Macro directive; ("doc/macros:." for ROOT)
      TString        fSourceDir;       // where to look for classes; prepended to Decl/ImplFileName() (".:src:include" by default)
      TString        fDotDir;          // directory of GraphViz's dot binary
      TString        fEtcDir;          // directory containing auxiliary files
      TString        fOutputDir;       // output directory
   };

   struct TDocEntityInfo {
      TString        fClassFilter;     // filter used for buidling known classes
      THashList      fClasses;         // known classes
      std::map<TClass*,std::string> fGuessedDeclFileNames; // names of additional decl file names
      std::map<TClass*,std::string> fGuessedImplFileNames; // names of additional impl file names
      THashList      fLibDeps;         // Library dependencies
   };

protected:   
   TString        fCounter;         // counter string
   TString        fCounterFormat;   // counter printf-like format
   TString        fProductName;     // name of the product to document
   TIter         *fThreadedClassIter; // fClasses iterator for MakeClassThreaded
   Int_t          fThreadedClassCount; // counter of processed classes for MakeClassThreaded
   TVirtualMutex *fMakeClassMutex; // Mutex for MakeClassThreaded
   TDocSyntax     fDocSyntax;      // doc syntax configuration
   TLinkInfo      fLinkInfo;       // link (URL) configuration
   TOutputStyle   fOutputStyle;    // output style configuration
   TPathInfo      fPathInfo;       // path configuration
   TDocEntityInfo fDocEntityInfo;  // data for documented entities
   TModuleDefinition* fModuleDef; // object translating classes to module names
   TFileDefinition* fFileDef; // object translating classes to file names

   virtual void    CreateJavascript() const;
   virtual void    CreateStyleSheet() const;
   void            CreateListOfTypes();
   void            CreateListOfClasses(const char* filter);
   void            MakeClass(void* cdi, Bool_t force=kFALSE);
   TClassDocInfo  *GetNextClass();
   void            HelperDeleted(THelperBase* who);

   static void    *MakeClassThreaded(void* info);

public:
   //______________________________________________________________
   // Helper base class.
   class THelperBase {
   public:
      virtual ~THelperBase();
      void    SetOwner(THtml& html);
      THtml*  GetOwner() const { return fHtml; }
   private:
      THtml*  fHtml; // object owning the helper
   };

   //______________________________________________________________
   // Helper class to translate between classes and their
   // modules. Can be derived from and thus replaced by
   // the user; see THtml::SetModuleDefinition().
   class TModuleDefinition: public THelperBase {
      virtual bool GetModule(const TClass* cl, TString& out_modulename) const;
   };

   //______________________________________________________________
   // Helper class to translate between classes and their
   // filenames. Can be derived from and thus replaced by
   // the user; see THtml::SetFileDefinition().
   class TFileDefinition: public THelperBase {
   public:
      virtual bool GetDeclFileName(const TClass* cl, TString& out_filename) const;
      virtual bool GetImplFileName(const TClass* cl, TString& out_filename) const;
   protected:
      void SplitClassIntoDirFile(const TString& clname, TString& dir, TString& filename) const;
      void ExpandSearchPath(TString& path) const;
   };

   //______________________________________________________________
   // Helper class to translate between file names and their
   // version used for documentation. Can be derived from and thus
   // replaced by the user; see THtml::SetPathDefinition().
   class TPathDefinition: public THelperBase {
   public:
      virtual bool GetMacroPath(const TClass* cl, TString& out_dir) const;
      virtual bool GetIncludeAs(const TClass* cl, TString& out_include_as) const;
      virtual bool GetDocDir(const TString& module, TString& doc_dir) const;

      // Set the include path; first matching element (separated by ":") will be
      // removed from classes' 
      virtual void SetIncludePath(const char* include) { fIncludePath = include}
   protected:
      TString        fIncludePath;     // include path, removed from DeclFileName() for documentation ("include" for ROOT)
   };

   THtml();
   virtual      ~THtml();

   static void   LoadAllLibs();

   // Functions to generate documentation
   void          Convert(const char *filename, const char *title, 
                         const char *dirname = "", const char *relpath="../");
   void          CreateHierarchy();
   void          MakeAll(Bool_t force=kFALSE, const char *filter="*",
                         int numthreads = 1);
   void          MakeClass(const char *className, Bool_t force=kFALSE);
   void          MakeIndex(const char *filter="*");
   void          MakeTree(const char *className, Bool_t force=kFALSE);

   // Configuration setters
   void          SetModuleDefinition(const TModuleDefinition& md);
   void          SetFileDefinition(const TFileDefinition& fd);
   void          SetPathDefinition(const TPathDefinition& pd);
   void          SetProductName(const char* product) { fProductName = product; }
   void          SetOutputDir(const char *dir) { fPathInfo.fOutputDir = dir; }
   void          SetSourceDir(const char *dir);
   void          SetIncludePath(const char *path) { fPathInfo.fIncludePath = path; }
   void          SetEtcDir(const char* dir) { fPathInfo.fEtcDir = dir; }
   void          SetDocPath(const char* path) { fPathInfo.fDocPath = path; }
   void          SetDotDir(const char* dir) { fPathInfo.fDotDir = dir; fPathInfo.fFoundDot = -1; }
   void          SetRootURL(const char* url) { fLinkInfo.fROOTURL = url; }
   void          SetLibURL(const char* lib, const char* url) { fLinkInfo.fLibURLs[lib] = url; }
   void          SetXwho(const char *xwho) { fLinkInfo.fXwho = xwho; }
   void          SetMacroPath(const char* path) {fPathInfo.fMacroPath = path;}
   void          AddMacroPath(const char* path);
   void          SetCounterFormat(const char* format) { fCounterFormat = format; }
   void          SetClassDocTag(const char* tag) { fDocSyntax.fClassDocTag = tag; }
   void          SetAuthorTag(const char* tag) { fDocSyntax.fAuthorTag = tag; }
   void          SetLastUpdateTag(const char* tag) { fDocSyntax.fLastUpdateTag = tag; }
   void          SetCopyrightTag(const char* tag) { fDocSyntax.fCopyrightTag = tag; }
   void          SetHeader(const char* file) { fOutputStyle.fHeader = file; }
   void          SetFooter(const char* file) { fOutputStyle.fFooter = file; }
   void          SetHomepage(const char* url) { fLinkInfo.fHomepage = url; }
   void          SetSearchStemURL(const char* url) { fLinkInfo.fSearchStemURL = url; }
   void          SetSearchEngine(const char* url) { fLinkInfo.fSearchEngine = url; }
   void          SetViewCVS(const char* url) { fLinkInfo.fViewCVS = url; }
   void          SetWikiURL(const char* url) { fLinkInfo.fWikiURL = url; }
   void          SetCharset(const char* charset) { fOutputStyle.fCharset = charset; }
   void          SetDocStyle(const char* style) { fDocSyntax.fDocStyle = style; }

   // Configuration getters
   const TModuleDefinition& GetModuleDefinition() const;
   const TFileDefinition&   GetFileDefinition() const;
   const TPathDefinition&   GetPathDefinition() const;
   const TString&      GetProductName() const { return fProductName; }
   const TString&      GetOutputDir(Bool_t createDir = kTRUE) const;
   const TString&      GetSourceDir() const { return fPathInfo.fSourceDir; }
   const TString&      GetIncludePath() const { return fPathInfo.fIncludePath; }
   virtual const char* GetEtcDir();
   const TString&      GetModuleDocPath() const { return fPathInfo.fDocPath; }
   const TString&      GetDotDir() const { return fPathInfo.fDotDir; }
   const char*         GetURL(const char* lib = 0) const;
   const TString&      GetXwho() const { return fLinkInfo.fXwho; }
   const TString&      GetMacroPath() const { return fPathInfo.fMacroPath; }
   const char*         GetCounterFormat() const { return fCounterFormat; }
   const TString&      GetClassDocTag() const { return fDocSyntax.fClassDocTag; }
   const TString&      GetAuthorTag() const { return fDocSyntax.fAuthorTag; }
   const TString&      GetLastUpdateTag() const { return fDocSyntax.fLastUpdateTag; }
   const TString&      GetCopyrightTag() const { return fDocSyntax.fCopyrightTag; }
   const TString&      GetHeader() const { return fOutputStyle.fHeader; }
   const TString&      GetFooter() const { return fOutputStyle.fFooter; }
   const TString&      GetHomepage() const { return fLinkInfo.fHomepage; }
   const TString&      GetSearchStemURL() const { return fLinkInfo.fSearchStemURL; }
   const TString&      GetSearchEngine() const { return fLinkInfo.fSearchEngine; }
   const TString&      GetViewCVS() const { return fLinkInfo.fViewCVS; }
   const TString&      GetWikiURL() const { return fLinkInfo.fWikiURL; }
   const TString&      GetCharset() const { return fOutputStyle.fCharset; }
   const TString&      GetDocStyle() const { return fDocSyntax.fDocStyle; }

   // Functions that should only be used by TDocOutput etc.
   Bool_t              CopyFileFromEtcDir(const char* filename) const;
   virtual void        CreateAuxiliaryFiles() const;
   virtual TClass*     GetClass(const char *name) const;
   const char*         GetCounter() const { return fCounter; }
   virtual const char* GetDeclFileName(TClass* cl) const;
   void                GetDerivedClasses(TClass* cl, std::map<TClass*, Int_t>& derived) const;
   virtual const char* GetImplFileName(TClass* cl) const;
   virtual const char* GetFileName(const char *filename) const;
   virtual void        GetSourceFileName(TString& filename);
   virtual void        GetHtmlFileName(TClass *classPtr, TString& filename) const;
   virtual const char* GetHtmlFileName(const char* classname) const;
   TCollection*        GetLibraryDependencies() { return &fDocEntityInfo.fLibDeps; }
   const TList*        GetListOfModules() const { return &fDocEntityInfo.fModules; }
   const TList*        GetListOfClasses() const { return &fDocEntityInfo.fClasses; }
   TVirtualMutex*      GetMakeClassMutex() const { return  fMakeClassMutex; }
   virtual void        GetModuleName(TString& module, const char* filename) const;
   virtual void        GetModuleNameForClass(TString& module, TClass* cl) const;
   Bool_t              HaveDot();
   static Bool_t       IsNamespace(const TClass*cl);
   void                SetDeclFileName(TClass* cl, const char* filename);
   void                SetFoundDot(Bool_t found = kTRUE) { fPathInfo.fFoundDot = found; }
   void                SetImplFileName(TClass* cl, const char* filename);

   // unused
   void                ReplaceSpecialChars(std::ostream&, const char*) {
      Error("ReplaceSpecialChars",
            "Removed, call TDocOutput::ReplaceSpecialChars() instead!"); }
   void                SetEscape(char /*esc*/ ='\\') {} // for backward comp

   ClassDef(THtml,0)  //Convert class(es) into HTML file(s)
};

R__EXTERN THtml *gHtml;

#endif
