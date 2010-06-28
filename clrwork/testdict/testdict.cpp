// testdict.cpp

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/CodeGenAction.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/PCHReader.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/ParseAST.h"

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/System/Host.h"
#include "llvm/Target/TargetSelect.h"

#include "TSystem.h"

#include "clr-scan.h"
#include "dict.h"

#include <vector>
#include <iostream>

void AddIncludePath(clang::CompilerInstance* CI, const char* fileName)
{
   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   headerOpts.AddPath(fileName, clang::frontend::Angled, IsUserSupplied, IsFramework);
}

void SetupLangOptions(clang::CompilerInstance* CI)
{
   clang::LangOptions & langInfo = CI->getLangOpts();
   //langInfo.C99 = 1;
   //langInfo.HexFloats = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs = 1; // C94, C99, C++.
   langInfo.CPlusPlus = 1;
   //langInfo.CPlusPlus0x = 1;
   langInfo.CXXOperatorNames = 1;
   langInfo.Bool = 1;
   langInfo.NeXTRuntime = 1;
   langInfo.NoInline = 1;
   langInfo.Exceptions = 1;
   langInfo.GNUMode = 1;
   langInfo.NoInline = 1;
   langInfo.GNUInline = 1;
   langInfo.DollarIdents = 1;
   langInfo.POSIXThreads = 1;
}

void SetupCI(clang::CompilerInstance* CI)
{
   SetupLangOptions(CI);
   AddIncludePath(CI, ".");
   TString include = gSystem->Getenv("ROOTSYS");
   include.Append("/include");
   AddIncludePath(CI, include);
}

void OpenPCH(clang::CompilerInstance* CI, std::string fileName)
{
   clang::PreprocessorOptions & PO = CI->getInvocation().getPreprocessorOpts();
   std::string originalFile = clang::PCHReader::getOriginalSourceFile(fileName, CI->getDiagnostics());
   if (!originalFile.empty()) {
      PO.ImplicitPCHInclude = fileName;
      PO.Includes.push_back(originalFile);
   }
   else {
      std::cerr << "Cannot find original file for precompiled header: " + fileName << std::endl;
   }
}

clang::CompilerInstance* ParseFile(const char* fileName)
{
   // const char* dupl_name = strdup (fileName);
   clang::TextDiagnosticPrinter diagClient(llvm::errs(), clang::DiagnosticOptions());
   clang::Diagnostic diags(& diagClient);
   static const char* argv [] = { "program", "-x", "c++" };
   static const int argc = sizeof(argv) / sizeof(argv[0]);
   clang::CompilerInvocation * invocation = new clang::CompilerInvocation;
   clang::CompilerInvocation::CreateFromArgs(*invocation, argv + 1, argv + argc, diags);
   // Create a compiler instance to handle the actual work.
   clang::CompilerInstance * CI = new clang::CompilerInstance;
   CI->setLLVMContext(new llvm::LLVMContext);
   CI->setInvocation(invocation);
   // Create the compilers actual diagnostics engine.
   CI->createDiagnostics(argc, const_cast <char**>(argv));
   if (!CI->hasDiagnostics()) {
      std::cerr << "No diagnostics" << std::endl;
   }
   bool pch = false;
   if (TString(fileName).EndsWith(".pch")) {
      pch = true;
      OpenPCH(CI, fileName);
   }
   CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(), CI->getTargetOpts()));
   if (!CI->hasTarget()) {
      std::cerr << "No target" << std::endl;
   }
   CI->getTarget().setForcedLangOptions(CI->getLangOpts());
   CI->createFileManager();
   CI->createSourceManager();
   CI->createPreprocessor();
   clang::Preprocessor & PP = CI->getPreprocessor();
   SetupCI(CI);
   CI->getDiagnosticClient().BeginSourceFile(CI->getLangOpts(), &PP);
   CI->setASTContext(new clang::ASTContext(CI->getLangOpts(), PP.getSourceManager(), CI->getTarget(), PP.getIdentifierTable(), PP.getSelectorTable(), PP.getBuiltinInfo(), false, 0));
   CI->setASTConsumer(new clang::ASTConsumer());
   if (pch) {
      std::string src = fileName;
      src = "#include \"" + src + "\"\n";
      src = "/* nothing*/ \n";
      llvm::MemoryBuffer* SB = llvm::MemoryBuffer::getMemBufferCopy(src, "SIMPLE_BUFFER");
      if (!SB) {
         std::cerr << "Failed to create memory buffer" << std::endl;
      }
      // CI->getSourceManager().clearIDTables();
      CI->getSourceManager().createMainFileIDForMemBuffer(SB);
      if (CI->getSourceManager().getMainFileID().isInvalid()) {
         std::cerr << "compileString: Failed to create main file id" << std::endl;
      }
   }
   else {
      const clang::FileEntry* File = CI->getFileManager().getFile(fileName);
      if (File) {
         CI->getSourceManager().createMainFileID(File, clang::SourceLocation());
      }
      if (CI->getSourceManager().getMainFileID().isInvalid()) {
         std::cerr << "Error reading file" << std::endl;
      }
   }
   clang::ParseAST(PP, &CI->getASTConsumer(), CI->getASTContext());
   unsigned err_count = CI->getDiagnostics().getNumErrors();
   if (err_count) {
      std::cerr << "Parse failed" << std::endl;
   }
   return CI;
}

void Parse(std::vector<const char*> parsefiles, bool read_stubs = false)
{
   TScanner scanner;
   scanner.UseStubs(read_stubs);
   for (int i = 0; i < parsefiles.size(); ++i) {
      const char* fileName = parsefiles[i];
      clang::CompilerInstance* CI = ParseFile(fileName);
      scanner.Scan(&CI->getASTContext());
   }
}

void Generate(const char* name)
{
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict(generator);
   generator.Print(name);
}

int main(int argc, const char** argv)
{
   const char* name = "";
   std::vector<const char*> parsefiles;
   bool f_opt = false;
   bool unknown_opt = false;
   for (int i = 1; i < argc; ++i) {
      const char* s = argv[i];
      if (f_opt) {
         name = s;
         f_opt = false;
      }
      else if (s[0] == '-') {
         if (s[1] == 'f') {
            f_opt = true;
         }
         else {
            unknown_opt = true;
         }
      }
      else {
         parsefiles.push_back(s);
         f_opt = false;
      }
   }
   if (!parsefiles.size() || unknown_opt || f_opt) {
      std::cout << "Usage: " << argv[0] << " [-f output_file] input_file..." << std::endl;
      return 1;
   }
   Parse(parsefiles); // Do not read stubs.
   Generate(name);
   Parse(parsefiles, true); // Do read stubs.
   return 0;
}

