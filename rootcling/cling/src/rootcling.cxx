
/* testdict.cc */

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
#include "llvm/System/Signals.h"
#include "llvm/Target/TargetSelect.h"

#include "TSystem.h"

#include "clr-scan.h"
#include "clr-info.h"
#include "genrflxdict.h"

#include <vector>
#include <iostream>
#include <sstream>

/* -------------------------------------------------------------------------- */

std::string gPchFileName = "";
std::string gOrigFileName = "";

/* -------------------------------------------------------------------------- */

inline std::string CharsToStd (const char * s)
{
   return (s == NULL) ? "" : s;
}

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

/* -------------------------------------------------------------------------- */

void AddIncludePath (clang::CompilerInstance* CI, const std::string fileName)
{
   info ("AddIncludePath [" + fileName + "]");

   clang::HeaderSearchOptions& headerOpts = CI->getHeaderSearchOpts();
   const bool IsUserSupplied = false;
   const bool IsFramework = false;
   headerOpts.AddPath (fileName.c_str(), clang::frontend::Angled, IsUserSupplied, IsFramework);
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
   langInfo.NoInline = 1;
   langInfo.GNUInline = 1;
   langInfo.DollarIdents = 1;
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
      clang::PCHReader::getOriginalSourceFile(fileName, CI->getDiagnostics());

   if (! originalFile.empty())
   {
      PO.ImplicitPCHInclude = fileName;
      PO.Includes.push_back(originalFile);

      gPchFileName = fileName;
      gOrigFileName = originalFile;
   }
   else info ("Cannot find original file for precompiled header: " + fileName);
}

/* -------------------------------------------------------------------------- */

clang::CompilerInstance* ParseFileOrSource (const std::string fileName,
                                            const std::string sourceCode = "")
{
   // fileName != "" && sourceCode != "" ... compile file and then source from string

   bool pch = IsPCH (fileName);

   clang::TextDiagnosticPrinter diagClient (llvm::errs(), clang::DiagnosticOptions ());

   clang::Diagnostic diags (& diagClient);

   static const char* argv [] = { "program", "-x", "c++" };

   static const int argc = sizeof (argv) / sizeof (argv[0]);

   clang::CompilerInvocation * invocation = new clang::CompilerInvocation;
   clang::CompilerInvocation::CreateFromArgs
      (*invocation,
       argv + 1,
       argv + argc,
       diags);

   // Create a compiler instance to handle the actual work.
   clang::CompilerInstance * CI = new clang::CompilerInstance;
   CI->setLLVMContext (new llvm::LLVMContext);
   CI->setInvocation (invocation);

   // Create the compilers actual diagnostics engine.
   CI->createDiagnostics (argc, const_cast <char**> (argv));
   if (! CI->hasDiagnostics ())
      error ("No diagnostics");

   if (pch)
      OpenPCH (CI, fileName);

   CI->setTarget
   (
      clang::TargetInfo::CreateTargetInfo
         (CI->getDiagnostics(),
          CI->getTargetOpts())
   );

   if (!CI->hasTarget ())
      error ("No target");

   CI->getTarget().setForcedLangOptions(CI->getLangOpts());

   CI->createFileManager();
   CI->createSourceManager();

   SetupCI (CI);
   // set include directories before CI->createPreprocessor()

   CI->createPreprocessor();
   clang::Preprocessor & PP = CI->getPreprocessor();


   CI->getDiagnosticClient().BeginSourceFile (CI->getLangOpts(), &PP);

   CI->setASTContext
   (
      new clang::ASTContext
             (CI->getLangOpts (),
              PP.getSourceManager (),
              CI->getTarget (),
              PP.getIdentifierTable (),
              PP.getSelectorTable (),
              PP.getBuiltinInfo (),
              false,
              0)
   );

   CI->setASTConsumer (new clang::ASTConsumer());

   PP.getBuiltinInfo().InitializeBuiltins
      (PP.getIdentifierTable (),
       PP.getLangOptions().NoBuiltin);

   // PrintInfo (CI);

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
         CI->getSourceManager().createMainFileID(File, clang::SourceLocation());
      if (CI->getSourceManager().getMainFileID().isInvalid())
         error ("Error reading file");
   }

   clang::ParseAST (PP, & CI->getASTConsumer (), CI->getASTContext ());

   unsigned err_count = CI->getDiagnostics().getNumErrors();
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

std::string WriteStubFunctions (std::string inputFileName)
{
   clang::CompilerInstance* CI = ParseFile (inputFileName);

   clang::TranslationUnitDecl* tu =
      CI->getASTContext().getTranslationUnitDecl();

   /* fill Reflex dictionary */

   TScanner scanner;
   scanner.Scan(&CI->getASTContext(), tu);

   /* write stub functions */

   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict (generator);

   std::ostringstream stream;
   stream << generator;
   std::string source = stream.str();

   std::cout << source;

   return source;
}

void Process (std::string inputFileName)
{
   std::string source = WriteStubFunctions (inputFileName);

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
                         CI->getLLVMContext())
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

   /* find functions */

   TContext * current = GetFirstContext ();

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
         info ("Stub function recognized " + IntToStd (funcIndex));
         assert (current != NULL);
         // info (" ... " + IntToStd (current->index));
         assert (current->index == funcIndex);
         current->func = &func; // store LLVM funtion
         current = current->next;
      }
   }
}

void SimpleTest ()
{
   Reflex::Type t;
   Reflex::Object o;
   Reflex::Member m;

   t = Reflex::Type::ByName("C");

   o = t.Construct();

   m = t.MemberByName("one");

   m.Invoke(o);
}

/* -------------------------------------------------------------------------- */

#if 0
void ParseFiles (std::vector<const char*> input_files /* , bool read_stubs = false */)
{
   TScanner scanner;
   /* scanner.UseStubs (read_stubs); */

   for (int i = 0; i < input_files.size(); i++)
   {
      const char * fileName = input_files[i];

      clang::CompilerInstance* CI = ParseFile (fileName);

      clang::TranslationUnitDecl* tu =
         CI->getASTContext().getTranslationUnitDecl();

      scanner.Scan(&CI->getASTContext(), tu);
   }
}

void Generate (const char* output_file)
{
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict (generator);
   generator.Print (CharsToStd (output_file)); // print into file or standard output
}
#endif

/* -------------------------------------------------------------------------- */

int main (int argc, const char **argv)
{
   // NO init_trace ();

   #if 0
      llvm::sys::PrintStackTraceOnErrorSignal();
      llvm::PrettyStackTraceProgram X(argc, argv);
   #endif

   #if 0
      std::vector<const char*> input_files;
      const char * output_file = "";

      bool f_opt = false;
      bool unknown_opt = false;

      for (int i = 1; i < argc; i ++)
      {
         const char * s = argv[i];

         if (f_opt) {
            output_file = s;
            f_opt = false;
         } else if (s[0] == '-') {
            if (s[1] == 'f')
               f_opt = true;
            else
               unknown_opt = true;
         } else {
            input_files.push_back (s);
            f_opt = false;
         }
      }

      if (input_files.size() == 0 || unknown_opt || f_opt)
      {
         std::cout << "Usage: " << argv[0] << " [-f output_file] input_file..." << std::endl;
         return 1;
      }

      ParseFiles (input_files);
      Generate (output_file);
   #endif

   #if 1
      if (argc != 2)
      {
         std::cout << "Usage: " << argv[0] << " input_file..." << std::endl;
         return 1;
      }

      std::string input_file = CharsToStd (argv [1]);

      Process (input_file);
      SimpleTest ();
   #endif

   return 0;
}
