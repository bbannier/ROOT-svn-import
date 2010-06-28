#include "ClingDict.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/ParseAST.h"

#include "Reflex/DictionaryGenerator.h"

#include "clr-scan.h"
#include "dict.h"

#include <vector>

//______________________________________________________________________________
ClingDict::ClingDict(const char* name, std::vector<const char*> parsefiles)
{
   fName = name;
   fParseFiles = parsefiles;
}

//______________________________________________________________________________
bool ClingDict::parse(clang::CompilerInstance* CI)
{
   TScanner scanner;
   for (int i = 0; i < fParseFiles.size(); ++i) {
      bool ok = parseFile(CI, fParseFiles[i]);
      if (!ok) {
         return false;
      }
      scanner.Scan(&CI->getASTContext());
   }
   return true;
}

//______________________________________________________________________________
void ClingDict::generate()
{
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict(generator);
   generator.Print(fName);
}

//______________________________________________________________________________
bool ClingDict::parseFile(clang::CompilerInstance* CI, const char* fileName)
{
   CI->createPreprocessor();
   clang::Preprocessor& PP = CI->getPreprocessor();
   // add current directory into include path
   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   headerOpts.AddPath(".", clang::frontend::Angled, IsUserSupplied, IsFramework);
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(), PP.getSourceManager(),
       CI->getTarget(), PP.getIdentifierTable(), PP.getSelectorTable(),
       PP.getBuiltinInfo(), false, 0));
   CI->setASTConsumer(new clang::ASTConsumer());
   const clang::FileEntry* File = CI->getFileManager().getFile(fileName);
   if (File) {
      CI->getSourceManager().createMainFileID(File, clang::SourceLocation());
   }
   if (CI->getSourceManager().getMainFileID().isInvalid()) {
      return false;
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   unsigned err_count = CI->getDiagnostics().getNumErrors();
   if (err_count) {
      return false;
   }
   return true;
}

