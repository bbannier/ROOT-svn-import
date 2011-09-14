// @(#)root/cint:$Id$

/* rootcling.cxx */

/* -------------------------------------------------------------------------- */

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclGroup.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/CodeGen/ModuleBuilder.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Serialization/ASTReader.h"

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Signals.h"
#include "llvm/Target/TargetSelect.h"

#include "cling/Interpreter/CIFactory.h"

#include "TSystem.h"
#include "TError.h"

#include "clr-scan.h"
#include "genrflxdict.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> // class std::runtime_error

/* -------------------------------------------------------------------------- */

inline std::string IntToStd (int num)
{
   std::ostringstream stream;
   stream << num;
   return stream.str ();
}

bool IsPCH (std::string fileName)
{
   bool pch = false;
   int len = fileName.length ();
   if (len > 4 &&  fileName.substr (len-4) == ".pch")
       pch = true;
   return pch;
}

/* ---------------------------------------------------------------------- */

void info (const std::string msg)
{
   Info("rootcling", "%s", msg.c_str());
}

void error (const std::string msg)
{
   Error("rootcling", "%s", msg.c_str());
   throw new std::runtime_error ("error: " + msg);
}

void AddIncludePath (clang::CompilerInstance* CI, const std::string fileName)
{
   info ("AddIncludePath [" + fileName + "]");

   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   const bool IsSysRootRelative = true;
   headerOpts.AddPath (fileName.c_str(), clang::frontend::Angled, IsUserSupplied, IsFramework, IsSysRootRelative);
}

void SetupLangOptions (clang::CompilerInstance* CI)
{
   clang::LangOptions & langInfo = CI->getLangOpts ();
   //langInfo.C99         = 1;
   //langInfo.HexFloats   = 1;
   langInfo.BCPLComment = 1; // Only for C99/C++.
   langInfo.Digraphs    = 1; // C94, C99, C++.
   langInfo.CPlusPlus   = 1;
   //langInfo.CPlusPlus0x = 1;
   langInfo.CXXOperatorNames = 1;
   langInfo.Bool = 1;
   langInfo.NeXTRuntime = 1;
   langInfo.NoInline = 1;
   langInfo.Exceptions = 1;
   langInfo.GNUMode = 1;
   langInfo.NoInline = 0;
   langInfo.GNUInline = 1;
   langInfo.DollarIdents = 0;
   langInfo.POSIXThreads = 1;
}

void SetupCI (clang::CompilerInstance* CI)
{
   SetupLangOptions (CI);

   // add current directory into include path
   AddIncludePath(CI, ".");

   // add ROOT include directory
   TString include = gSystem->Getenv("ROOTSYS");
   include.Append("/include");
   AddIncludePath(CI, include.Data());
}

/* -------------------------------------------------------------------------- */

void PrintInfo (clang::CompilerInstance * CI)
{
   const clang::HeaderSearchOptions & hsOpts = CI->getHeaderSearchOpts ();
   info ("Sysroot " +  hsOpts.Sysroot);
   int size = hsOpts.UserEntries.size();
   for (int i = 0; i < size; i++)
      info ("User entry " + hsOpts.UserEntries[i].Path);
   info ("EnvIncPath " + hsOpts.EnvIncPath);
   info ("CEnvIncPath " + hsOpts.CEnvIncPath);
   info ("CXXEnvIncPath " + hsOpts.CXXEnvIncPath);
   info ("ResourceDir " + hsOpts.ResourceDir);
}

/* -------------------------------------------------------------------------- */

void OpenPCH (clang::CompilerInstance* CI, std::string fileName)
{
   clang::PreprocessorOptions & PO = CI->getInvocation().getPreprocessorOpts();

   std::string originalFile =
      clang::ASTReader::getOriginalSourceFile(fileName, CI->getFileManager(), /*CI->getFileSystemOpts(),*/ CI->getDiagnostics());

   if (! originalFile.empty())
   {
      PO.ImplicitPCHInclude = fileName;
      PO.Includes.push_back(originalFile);
   }
   else info ("Cannot find original file for precompiled header: " + fileName);
}

/* -------------------------------------------------------------------------- */

clang::CompilerInstance* ParseFileOrSource (const std::string fileName,
                                            const std::string sourceCode = "")
{
   // fileName != "" && sourceCode != "" ... compile file and then source from string

   bool pch = IsPCH (fileName);

   static const char* argv [] = { "program", "-x", "c++" };
   static const int argc = sizeof (argv) / sizeof (argv[0]);
   const char* llvmdir = gSystem->Getenv("LLVMDIR");

   // Create a compiler instance to handle the actual work.
   
   clang::CompilerInstance * CI =  cling::CIFactory::createCI(0, 0, argc, argv, llvmdir);

   if (pch)
      OpenPCH (CI, fileName);

   SetupCI (CI);
   CI->createPreprocessor();
   clang::Preprocessor & PP = CI->getPreprocessor();

   CI->getDiagnosticClient().BeginSourceFile (CI->getLangOpts(), &PP);

   CI->setASTContext
   (
      new clang::ASTContext
             (CI->getLangOpts (),
              PP.getSourceManager (),
              &CI->getTarget (),
              PP.getIdentifierTable (),
              PP.getSelectorTable (),
              PP.getBuiltinInfo (),
              0)
   );

   CI->setASTConsumer (new clang::ASTConsumer());
   
   PP.getBuiltinInfo().InitializeBuiltins
      (
       PP.getIdentifierTable (),
       PP.getLangOptions()
      );
   
   PrintInfo (CI);

   if (pch || sourceCode != "") {

      std::string src = sourceCode;

      if (! pch && fileName != "")
         src = "#include \"" + fileName + "\"\n" + src;

      llvm::MemoryBuffer * SB = llvm::MemoryBuffer::getMemBufferCopy(src, "SIMPLE_BUFFER");
      if (SB == NULL)
         error ("Failed to create memory buffer");

      CI->getSourceManager().clearIDTables();
      CI->getSourceManager().createMainFileIDForMemBuffer(SB);
      if (CI->getSourceManager().getMainFileID().isInvalid())
         error ("compileString: Failed to create main file id");
   }
   else {
      const clang::FileEntry* File = CI->getFileManager().getFile(fileName);
      if (File)
         CI->getSourceManager().createMainFileID(File);
      if (CI->getSourceManager().getMainFileID().isInvalid())
         error ("Error reading file: "+ fileName);
   }

   clang::ParseAST(PP, & CI->getASTConsumer (), CI->getASTContext ());

   if (CI->hasPreprocessor())
      CI->getPreprocessor().EndSourceFile();

   CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnosticClient().getNumErrors());
   CI->getDiagnosticClient().EndSourceFile();

   unsigned err_count = CI->getDiagnosticClient().getNumErrors();
   if (err_count != 0)
      error ("Parse failed");

   return CI;
}

clang::CompilerInstance* ParseFile (const std::string fileName)
{
   return ParseFileOrSource (fileName, "");
}

clang::CompilerInstance* ParseSource (const std::string sourceCode)
{
   return ParseFileOrSource ("", sourceCode);
}

/* -------------------------------------------------------------------------- */

void FillDictionary (const std::string& inputFileName,
                     const std::string& selectionFileName)
{
   clang::CompilerInstance* CI = ParseFile (inputFileName);

   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();

   /* fill Reflex dictionary */

   TScanner scanner;
   scanner.Scan(&CI->getASTContext(), tu, selectionFileName);
}

void WriteDictionary (const std::string& outputFileName)
{
   info ("Writing " + outputFileName);
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict (generator);

   std::ofstream stream (outputFileName.c_str ());
   stream << generator;
}

void Process (const std::string& inputFileName)
{
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict(generator);

   std::ostringstream stream;
   stream << generator;
   std::string source = stream.str();

   /* parse source and get translation unit */

   // !? READ SOURCE AGAIN - SHOULD BE CHANGED
   clang::CompilerInstance* CI = ParseFileOrSource (inputFileName, source);

   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();

   if (tu == NULL)
      error ("Parse failed, no translation unit");

   /* generate code */

   llvm::OwningPtr <clang::CodeGenerator> codeGen
   (
      CreateLLVMCodeGen (CI->getDiagnostics(),
                         "SIMPLE_MODULE",
                         CI->getCodeGenOpts(),
                         * new llvm::LLVMContext())
   );

   codeGen->Initialize (CI->getASTContext ());

   info ("Running code generation");
   clang::TranslationUnitDecl::decl_iterator iter = tu->decls_begin ();
   clang::TranslationUnitDecl::decl_iterator iter_end = tu->decls_end ();
   for ( ; iter != iter_end; ++ iter )
      codeGen->HandleTopLevelDecl (clang::DeclGroupRef (*iter));
   codeGen->HandleTranslationUnit(CI->getASTContext ());
   info ("Finished code generation");

   llvm::Module* module = codeGen->ReleaseModule();
   if (module == NULL)
      error ("Module creation failed");

   /* find functions in generated module */

   TClrContext * current = gClrReg->GetFirstContext ();
   // iterate list of contexts
   // assuming that generated functions are inside translation unit
   // stored in the same order as in the source code

   const llvm::Module::FunctionListType & funcs = module->getFunctionList();
   llvm::Module::const_iterator I  = funcs.begin();
   llvm::Module::const_iterator IE = funcs.end();
   for  (; I != IE; ++I ) {
      const llvm::Function & func = *I;
      const std::string funcName = func.getName();
      // info ("function " + funcName);

      int funcIndex = 0;
      bool funcRecognized = false;

      int len = funcName.length();
      const char * s = funcName.c_str();
      if (len > 2 && s[0] == '_' && s[1] == 'Z'){
         int i = 2;
         int cnt = 0; // number after _Z prefix
         while (i < len && s[i] >= '0' && s[i] <= '9') {
             cnt = 10*cnt + s[i]-'0';
             i ++;
         }
         // is function name __stub_
         if (cnt > 5  && i+cnt < len &&
             s[i]=='_' && s[i+1]=='_' && s[i+2]=='s' &&
             s[i+3]=='t' && s[i+4]=='u' && s[i+5]=='b' && s[i+6] == '_') {
            i += 7;
            while (i < len && s[i] >= '0' && s[i] <= '9') {
               funcIndex = 10*funcIndex + s[i]-'0';
               funcRecognized = true;
               i ++;
            }
         }
      }

      if (funcRecognized)
      {
         if (current != NULL && current->index == funcIndex)
         {
            info ("Stub function recognized " + IntToStd (funcIndex));
            current->func = &func; // store LLVM funtion into context
            current = current->next;
         }
         else
         {
            error ("Expected stub function " + IntToStd (funcIndex));
         }
      }
   }
}

/* -------------------------------------------------------------------------- */

int main (int argc, const char **argv)
{
   std::string input_file = "";
   std::string selection_file = "";

   std::string dict_file = "";
   std::string classic_dict_file = "";

   bool unknown_option = false;
   bool too_many_args = false;

   for (int i = 1; i < argc; i ++) {
      std::string s = argv[i];

      if (s[0] == '-') {

         if (s == "--dict" && i+1<argc)
            dict_file = argv[++i];
         else if (s == "--classic-dict" && i+1<argc)
            classic_dict_file = argv[++i];
         else
            unknown_option = true;

         } else {

         if (input_file == "")
            input_file = s;
         else if (selection_file == "")
            selection_file = s;
         else
            too_many_args = true;

         }
      }

   if (input_file == "" || selection_file == "" || unknown_option || too_many_args) {
      std::cout << "Usage: " << argv[0] << " input_file selection.xml|linkdef.h" << std::endl;
      std::cout << "   [--dict file_name]" << std::endl;
      return 1;
   }

   // allocate gClrReg singleton
   TClrReg::Init();

   FillDictionary(input_file, selection_file);

   if (dict_file != "")
      WriteDictionary (dict_file);

   Process(input_file);

   return 0;
}
