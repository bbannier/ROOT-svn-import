// @(#)root/html:$Id: TDocInfo.cxx 23937 2008-05-20 16:44:59Z axel $
// Author: Axel Naumann 2009

/*************************************************************************
 * Copyright (C) 1995-2009 Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TDocSourceCollector.h"


//______________________________________________________________________________
void TDocSourceCollector::TFileDefinition::ExpandSearchPath(TString& path) const
{
   // Create all permutations of path and THtml's input path:
   // path being PP/ and THtml's input being .:include/:src/ gives
   // .:./PP/:include:include/PP/:src/:src/PP
   THtml* owner = GetOwner();
   if (!owner) return;

   TString pathext;
   TString inputdir = owner->GetInputPath();
   TString tok;
   Ssiz_t start = 0;
   while (inputdir.Tokenize(tok, start, THtml::GetDirDelimiter())) {
      if (pathext.Length())
         pathext += GetDirDelimiter();
      if (tok.EndsWith("\\"))
         tok.Remove(tok.Length() - 1);
      pathext += tok;
      if (path.BeginsWith(tok))
         pathext += GetDirDelimiter() + path;
      else
         pathext += GetDirDelimiter() + tok + "/" + path;
   }
   path = pathext;

}

//______________________________________________________________________________
void TDocSourceCollector::TFileDefinition::SplitClassIntoDirFile(const TString& clname, TString& dir,
                                                   TString& filename) const
{
   // Given a class name with a scope, split the class name into directory part
   // and file name: A::B::C becomes module B, filename C.
   TString token;
   Ssiz_t from = 0;
   filename = "";
   dir = "";
   while (clname.Tokenize(token, from, "::") ) {
      dir = filename;
      filename = token;
   }

   // convert from Scope, class to module, filename.h
   dir.ToLower();
}


//______________________________________________________________________________
bool TDocSourceCollector::TFileDefinition::GetDeclFileName(const TClass* cl, TString& out_filename,
                                             TString& out_fsys, TFileSysEntry** fse) const
{
   // Determine cl's declaration file name. Usually it's just
   // cl->GetDeclFileName(), but sometimes conversions need to be done
   // like include/ to abc/cde/inc/. If no declaration file name is
   // available, look for b/inc/C.h for class A::B::C. out_fsys will contain
   // the file system's (i.e. local machine's) full path name to the file.
   // The function returns false if the class's header file cannot be found.
   //
   // If your software cannot be mapped into this scheme then derive your
   // own class from TFileDefinition and pass it to THtml::SetFileDefinition().

   return GetFileName(cl, true, out_filename, out_fsys, fse);
}

//______________________________________________________________________________
bool TDocSourceCollector::TFileDefinition::GetImplFileName(const TClass* cl, TString& out_filename,
                                             TString& out_fsys, TFileSysEntry** fse) const
{
   // Determine cl's implementation file name. Usually it's just
   // cl->GetImplFileName(), but sometimes conversions need to be done.
   // If no implementation file name is available look for b/src/C.cxx for
   // class A::B::C. out_fsys will contain the file system's (i.e. local 
   // machine's) full path name to the file.
   // The function returns false if the class's source file cannot be found.
   //
   // If your software cannot be mapped into this scheme then derive your
   // own class from TFileDefinition and pass it to THtml::SetFileDefinition().

   return GetFileName(cl, false, out_filename, out_fsys, fse);
}


//______________________________________________________________________________
void TDocSourceCollector::TFileDefinition::NormalizePath(TString& filename) const
{
   // Remove "/./" and collapse "/subdir/../" to "/"
   static const char* delim[] = {"/", "\\\\"};
   for (int i = 0; i < 2; ++i) {
      const char* d = delim[i];
      filename = filename.ReplaceAll(TString::Format("%c.%c", d[0], d[0]), TString(d[0]));
      TPRegexp reg(TString::Format("%s[^%s]+%s\\.\\.%s", d, d, d, d));
      while (reg.Substitute(filename, TString(d[0]), "", 0, 1)) {}
   }
}


//______________________________________________________________________________
TString TDocSourceCollector::TFileDefinition::MatchFileSysName(TString& filename, TFileSysEntry** fse) const
{
   // Find filename in the list of system files; return the system file name
   // and change filename to the file name as included.
   // filename must be normalized (no "/./" etc) before calling.

   TList* bucket = GetOwner()->GetLocalFiles()->GetEntries().GetListForObject(gSystem->BaseName(filename));
   TString filesysname;
   if (bucket) {
      TIter iFS(bucket);
      TFileSysEntry* fsentry = 0;
      while ((fsentry = (TFileSysEntry*) iFS())) {
         if (!filename.EndsWith(fsentry->GetName()))
            continue;
         fsentry->GetFullName(filesysname, kTRUE); // get the short version
         if (!filename.EndsWith(filesysname)) {
            filesysname = "";
            continue;
         }
         filename = filesysname;
         fsentry->GetFullName(filesysname, kFALSE); // get the long version
         if (fse) *fse = fsentry;
         break;
      }
   }
   return filesysname;
}


//______________________________________________________________________________
bool TDocSourceCollector::TFileDefinition::GetFileName(const TClass* cl, bool decl,
                                         TString& out_filename, TString& out_fsys,
                                         TFileSysEntry** fse) const
{
   // Common implementation for GetDeclFileName(), GetImplFileName()

   out_fsys = "";

   if (!cl) {
      out_filename = "";
      return false;
   }

   TString possibleFileName;
   TString possiblePath;
   TString filesysname;

   TString clfile = decl ? cl->GetDeclFileName() : cl->GetImplFileName();
   NormalizePath(clfile);
             
   out_filename = clfile;
   if (clfile.Length()) {
      // check that clfile doesn't start with one of the include paths;
      // that's not what we want (include/TObject.h), we want the actual file
      // if it exists (core/base/inc/TObject.h)

      // special case for TMath namespace:
      if (clfile == "include/TMathBase.h") {
         clfile = "math/mathcore/inc/TMath.h";
         out_filename = clfile;
      }

      TString inclDir;
      TString inclPath(GetOwner()->GetPathInfo().fIncludePath);
      Ssiz_t pos = 0;
      Ssiz_t longestMatch = kNPOS;
      while (inclPath.Tokenize(inclDir, pos, GetOwner()->GetDirDelimiter())) {
         if (clfile.BeginsWith(inclDir) && (longestMatch == kNPOS || inclDir.Length() > longestMatch))
            longestMatch = inclDir.Length();
      }
      if (longestMatch != kNPOS) {
         clfile.Remove(0, longestMatch);
         if (clfile.BeginsWith("/") || clfile.BeginsWith("\\"))
            clfile.Remove(0, 1);
         TString asincl(clfile);
         GetOwner()->GetPathDefinition().GetFileNameFromInclude(asincl, clfile);
         out_filename = clfile;
      } else {
         // header file without a -Iinclude-dir prefix
         filesysname = MatchFileSysName(out_filename, fse);
         if (filesysname[0]) {
            clfile = out_filename;
         }
      }
   } else {
      // check for a file named like the class:
      filesysname = cl->GetName();
      int templateLevel = 0;
      Ssiz_t end = filesysname.Length();
      Ssiz_t start = end - 1;
      for (; start >= 0 && (templateLevel || filesysname[start] != ':'); --start) {
         if (filesysname[start] == '>')
            ++templateLevel;
         else if (filesysname[start] == '<') {
            --templateLevel;
            if (!templateLevel)
               end = start;
         }
      }
      filesysname = filesysname(start + 1, end - start - 1);
      if (decl)
         filesysname += ".h";
      else
         filesysname += ".cxx";
      out_filename = filesysname;
      filesysname = MatchFileSysName(out_filename, fse);
      if (filesysname[0]) {
         clfile = out_filename;
      }
   }

   if (!decl && !clfile.Length()) {
      // determine possible impl file name from the decl file name,
      // replacing ".whatever" by ".cxx", and looking for it in the known
      // file names
      TString declSysFileName;
      if (GetFileName(cl, true, filesysname, declSysFileName)) {
         filesysname = gSystem->BaseName(filesysname);
         Ssiz_t posExt = filesysname.Last('.');
         if (posExt != kNPOS)
            filesysname.Remove(posExt);
         filesysname += ".cxx";
         out_filename = filesysname;
         filesysname = MatchFileSysName(out_filename, fse);
         if (filesysname[0]) {
            clfile = out_filename;
         }
      }
   }

   if (clfile.Length() && !decl) {
      // Do not return the source file for these packages, even though we can find them.
      // THtml needs to have the class description in the source file if it finds the
      // source file, and these classes have their class descriptions in the header files.
      // THtml needs to be improved to collect all of a class' documentation before writing
      // it out, so it can take the class doc from the header even though a source exists.
      static const char* vetoClasses[] = {"math/mathcore/", "math/mathmore/", "math/genvector/", 
                                          "math/minuit2/", "math/smatrix/"};
      for (unsigned int i = 0; i < sizeof(vetoClasses) / sizeof(char*); ++i) {
         if (clfile.Contains(vetoClasses[i])) {
            // of course there are exceptions from the exceptions:
            // TComplex and TRandom, TRandom1,...
            if (strcmp(cl->GetName(), "TComplex")
                && strcmp(cl->GetName(), "TMath")
                && strncmp(cl->GetName(), "TRandom", 7)) {
               out_filename = "";
               return false;
            } else break;
         }
      }
   }


   if (!clfile.Length()) {
      // determine possible decl file name from class + scope name:
      // A::B::C::myclass will result in possible file name myclass.h
      // in directory C/inc/
      out_filename = cl->GetName();
      if (!out_filename.Contains("::")) {
         out_filename = "";
         return false;
      }
      SplitClassIntoDirFile(out_filename, possiblePath, possibleFileName);

      // convert from Scope, class to module, filename.h
      if (possibleFileName.Length()) {
         if (decl)
            possibleFileName += ".h";
         else
            possibleFileName += ".cxx";
      }
      if (possiblePath.Length())
         possiblePath += "/";
      if (decl)
         possiblePath += "inc/";
      else
         possiblePath += "src/";
      out_filename = possiblePath + "/" + possibleFileName;
   } else {
      possiblePath = gSystem->DirName(clfile);
      possibleFileName = gSystem->BaseName(clfile);
   }

   if (possiblePath.Length())
      ExpandSearchPath(possiblePath);
   else possiblePath=".";

   out_fsys = gSystem->FindFile(possiblePath, possibleFileName, kReadPermission);
   if (out_fsys.Length()) {
      NormalizePath(out_fsys);
      return true;
   }
   out_filename = "";
   return false;
}

