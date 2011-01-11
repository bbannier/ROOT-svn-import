
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
#ifdef LLVM_2_8
   #include "llvm/System/Host.h"
   #include "llvm/System/Signals.h"
#else
   #include "llvm/Support/Host.h"
   #include "llvm/Support/Signals.h"
#endif
#include "llvm/Target/TargetSelect.h"

#ifndef INDEPENDENT
   #include "cling/Interpreter/CIBuilder.h"
#endif

#include "TSystem.h"

#include "clr-scan.h"
#include "genrflxdict.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept> // class std::runtime_error

#ifdef INDEPENDENT
   #include <string.h> // function strdup
   #include <queue>
   #include "clang/Frontend/TextDiagnosticPrinter.h"
#endif

#ifdef TRACE
   #include "clr-trace.h"
#endif

/* -------------------------------------------------------------------------- */

#define ENABLE_DEBUG_PRINT
#define ENABLE_DEBUG_COMMANDS

#ifdef ENABLE_DEBUG_PRINT
   #include "clr-print.h"
#endif

#ifdef ENABLE_DEBUG_COMMANDS
   #include "clr-cmds.h"
   #include "TCanvas.h"
#endif

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

/* ---------------------------------------------------------------------- */

void show_message (const std::string msg, const std::string location)
{
   std::string txt = location;

   if (txt != "")
      txt = txt + " ";

   txt = txt + msg;

   std::cerr << "CLR " << txt << std::endl;
}

void info (const std::string msg, const std::string location = "")
{
   show_message (msg, location);
}

void warning (const std::string msg, const std::string location = "")
{
   show_message ("warning: " + msg, location);
}

void error (const std::string msg, const std::string location = "")
{
   show_message ("error: " + msg, location);
   throw new std::runtime_error ("error: " + msg  + ", " +  location);
}

/* -------------------------------------------------------------------------- */

#ifndef INDEPENDENT

cling::CIBuilder* gCIBuilder = NULL;

clang::CompilerInstance* CreateCI ()
{
   static const char* argv [] = { "program", "-x", "c++" };

   static const int argc = sizeof (argv) / sizeof (argv[0]);

   const char* llvmdir = NULL;

   // Create a compiler instance to handle the actual work.
   gCIBuilder = new cling::CIBuilder(argc, argv, llvmdir);

   return gCIBuilder->createCI ();
}

void DestroyCI ()
{
   if (gCIBuilder != NULL)
       delete gCIBuilder;
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef INDEPENDENT
void ReadOptions (std::string file_name,
                  int & result_argc,
                  const char * * & result_argv)
{
   // add options from file

   int old_argc = result_argc;
   const char * * old_argv = result_argv;

   std::queue <std::string> list;

   ifstream f (file_name.c_str ());
   if (!f.good ())
      error (std::string ("Cannot open configuration file: ") + file_name);

   while (! f.eof ())
   {
      const int max = 256;
      char buf [max];
      f.getline (buf, max);
      std::string txt = std::string (buf);
      // info ("line: " + txt);

      int len = txt.length ();
      int inx = 0;

      while (inx < len)
      {
         while (inx < len && txt[inx] <= ' ') inx ++; // skip spaces
         int start = inx;
         while (inx < len && txt[inx] > ' ') inx ++; // valid characters
         if (start < inx)
         {
            std::string item = txt.substr(start, inx-start);
            list.push (item);
         }
      }
   }
   f.close ();

   int cnt = list.size ();

   int new_argc = old_argc + cnt;
   const char * * new_argv = new const char* [new_argc];

   for (int i = 0; i < old_argc; i ++)
      new_argv[i] = old_argv[i];

   for (int i = 0; i < cnt; i ++)
   {
      std::string txt = list.front ();
      list.pop ();
      new_argv[old_argc+i] = strdup (txt.c_str());
   }

   /*
   for (int i = 0; i < new_argc; i ++)
      info ("arg [" + IntToStd (i) + "] = " + CharsToStd (new_argv [i]));
   */

   result_argc = new_argc;
   result_argv = new_argv;
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef INDEPENDENT
clang::CompilerInstance* CreateCI ()
{
   static const char * args [] = { "program", "-x", "c++" };

   const char * * argv = args;
   int argc = sizeof (args) / sizeof (args[0]);

   #ifdef LLVM_2_8
      ReadOptions ("gcc-options.txt", argc, argv);
   #endif

   llvm::InitializeAllTargets();
   llvm::InitializeAllAsmPrinters();

   clang::CompilerInstance* CI = new clang::CompilerInstance();
   CI->setLLVMContext(new llvm::LLVMContext);

   clang::DiagnosticOptions DiagOpts;
   DiagOpts.ShowColors = 1;
   clang::DiagnosticClient *Client =
      new clang::TextDiagnosticPrinter (llvm::errs(), clang::DiagnosticOptions ());
   #ifdef LLVM_2_8
      clang::Diagnostic Diags (Client);
   #else
      llvm::IntrusiveRefCntPtr<clang::Diagnostic> Diags = clang::CompilerInstance::createDiagnostics(DiagOpts, 0, 0, Client);
      CI->setDiagnostics(Diags.getPtr());
   #endif

   #if 0
      clang::driver::Driver driver ("simple",
                                    llvm::sys::getHostTriple(),
                                    "a.out",
                                    /* IsProduction = */ false,
                                    /* CXXIsProduction = */ false,
                                    Diags);

      driver.setTitle ("clang interpreter");
   #endif

   clang::CompilerInvocation::CreateFromArgs
      (CI->getInvocation(),
       (argv + 1),
       (argv + argc),
       #ifdef LLVM_2_8
          Diags
       #else
          *Diags
       #endif
       );

   // Create the compilers actual diagnostics engine.
   CI->createDiagnostics (argc, const_cast <char**> (argv));
   if (! CI->hasDiagnostics ())
      error ("No diagnostics");

   CI->setTarget(clang::TargetInfo::CreateTargetInfo(CI->getDiagnostics(),
                 CI->getTargetOpts()));
   if (!CI->hasTarget())
      error ("No target");

   CI->getTarget().setForcedLangOptions(CI->getLangOpts());
   CI->createFileManager();
   #ifdef LLVM_2_8
      CI->createSourceManager();
   #else
      CI->createSourceManager(CI->getFileManager());
   #endif

   return CI;
}

void DestroyCI ()
{
}
#endif

/* -------------------------------------------------------------------------- */

#ifdef INDEPENDENT
#if 0
clang::CompilerInstance* CreateCI ()
{
   static const char * args [] = { "program", "-x", "c++" };

   const char * * argv = args;
   int argc = sizeof (args) / sizeof (args[0]);

   #ifdef LLVM_2_8
      ReadOptions ("gcc-options.txt", argc, argv);
   #endif

   clang::TextDiagnosticPrinter * diagClient =
      new clang::TextDiagnosticPrinter (llvm::errs(), clang::DiagnosticOptions ());

   #ifdef LLVM_2_8
      clang::Diagnostic diags (diagClient);
   #else
      clang::DiagnosticOptions diagOpts;
      diagOpts.ShowColors = 1;
      llvm::IntrusiveRefCntPtr<clang::Diagnostic> diags =
         clang::CompilerInstance::createDiagnostics(diagOpts, 0, 0, diagClient);
      // CI->setDiagnostics(diags.getPtr());
   #endif

   #if 0
   clang::driver::Driver driver ("simple",
                                 llvm::sys::getHostTriple(),
                                 "a.out",
                                 /* IsProduction = */ false,
                                 /* CXXIsProduction = */ false,
                                 diags);

   driver.setTitle ("clang interpreter");
   #endif

   clang::CompilerInvocation * invocation = new clang::CompilerInvocation;
   clang::CompilerInvocation::CreateFromArgs
       (*invocation,
        argv + 1, // skip program name
        argv + argc,
        #ifdef LLVM_2_8
          diags
        #else
          *diags
        #endif
       );

   // Create a compiler instance to handle the actual work.
   clang::CompilerInstance * CI = new clang::CompilerInstance;
   llvm::LLVMContext * context = new llvm::LLVMContext;
   CI->setLLVMContext (context);
   CI->setInvocation (invocation);

   // Create the compilers actual diagnostics engine.
   CI->createDiagnostics (argc, const_cast <char**> (argv));
   if (! CI->hasDiagnostics ())
      error ("No diagnostics");

   CI->setTarget
   (
      clang::TargetInfo::CreateTargetInfo
         (CI->getDiagnostics (),
          CI->getTargetOpts ())
   );

   if (!CI->hasTarget ())
      error ("No target");

   CI->getTarget().setForcedLangOptions(CI->getLangOpts());
   CI->createFileManager();

   #ifdef LLVM_2_8
      CI->createSourceManager();
   #else
      CI->createSourceManager (CI->getFileManager());
   #endif

   return CI;
}

void DestroyCI ()
{
}
#endif
#endif

/* -------------------------------------------------------------------------- */

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

   langInfo.Trigraphs = 1; // Trigraphs in source files.
   langInfo.BCPLComment = 1; // BCPL-style '//' comments.
   langInfo.Bool = 1; // 'bool', 'true', 'false' keywords.
   langInfo.DollarIdents = 1; // '$' allowed in identifiers.
   langInfo.AsmPreprocessor = 0; // Preprocessor in asm mode.
   langInfo.GNUMode = 1; // True in gnu99 mode false in c99 mode (etc)
   langInfo.GNUKeywords = 0; // True if GNU-only keywords are allowed
   langInfo.ImplicitInt = 0; // C89 implicit 'int'.
   langInfo.Digraphs = 1; // C94, C99 and C++
   langInfo.HexFloats = 1; // C99 Hexadecimal float constants.
   langInfo.C99 = 1; // C99 Support
   langInfo.Microsoft = 0; // Microsoft extensions.
   langInfo.Borland = 0; // Borland extensions.
   langInfo.CPlusPlus = 1; // C++ Support
   langInfo.CPlusPlus0x = 1; // C++0x Support
   langInfo.CXXOperatorNames = 1; // Treat C++ operator names as keywords.

   langInfo.ObjC1 = 0; // Objective-C 1 support enabled.
   langInfo.ObjC2 = 0; // Objective-C 2 support enabled.
   langInfo.ObjCNonFragileABI = 0; // Objective-C modern abi enabled
   langInfo.ObjCNonFragileABI2 = 0; // Objective-C enhanced modern abi enabled
   langInfo.ObjCDefaultSynthProperties = 0; // Objective-C auto-synthesized properties.
   langInfo.AppleKext = 0; // Allow apple kext features.

   langInfo.PascalStrings = 0; // Allow Pascal strings
   langInfo.WritableStrings  = 0; // Allow writable strings
   langInfo.ConstStrings = 1; // Add const qualifier to strings (-Wwrite-strings)
   langInfo.LaxVectorConversions = 0;
   langInfo.AltiVec = 0; // Support AltiVec-style vector initializers.
   langInfo.Exceptions = 1; // Support exception handling.
   langInfo.SjLjExceptions = 0; // Use setjmp-longjump exception handling.
   langInfo.RTTI = 1; // Support RTTI information.

   langInfo.NeXTRuntime = 1; // Use NeXT runtime.
   langInfo.Freestanding = 0; // Freestanding implementation
   langInfo.NoBuiltin = 0; // Do not use builtin functions (-fno-builtin)

   langInfo.ThreadsafeStatics = 1; // Whether static initializers are protected
                                  // by locks.
   langInfo.POSIXThreads = 1; // Compiling with POSIX thread support (-pthread)
   langInfo.Blocks = 0; // block extension to C
   langInfo.EmitAllDecls = 1; // Emit all declarations, even if
                              // they are unused.
   langInfo.MathErrno = 1; // Math functions must respect errno
                           // (modulo the platform support).

   langInfo.HeinousExtensions  = 0; // Extensions that we really don't like and
                                    // may be ripped out at any time.

   langInfo.Optimize = 0; // Whether __OPTIMIZE__ should be defined.
   langInfo.OptimizeSize = 0; // Whether __OPTIMIZE_SIZE__ should be
                              // defined.
   langInfo.Static = 0; // Should __STATIC__ be defined (as
                        // opposed to __DYNAMIC__).
   langInfo.PICLevel = 0; // The value for __PIC__, if non-zero.

   langInfo.GNUInline = 0; // Should GNU inline semantics be
                           // used (instead of C99 semantics).
   langInfo.NoInline = 1; // Should __NO_INLINE__ be defined.

   langInfo.ObjCGCBitmapPrint = 0; // Enable printing of gc's bitmap layout
                                   // for __weak/__strong ivars.

   langInfo.AccessControl = 1; // Whether C++ access control should
                               // be enabled.
   langInfo.CharIsSigned = 1; // Whether char is a signed or unsigned type
   langInfo.ShortWChar = 1; // Force wchar_t to be unsigned short int.

   langInfo.ShortEnums = 0; // The enum type will be equivalent to the
                            // smallest integer type with enough room.

   langInfo.OpenCL = 0; // OpenCL C99 language extensions.
   langInfo.CUDA = 0; // CUDA C++ language extensions.

   langInfo.AssumeSaneOperatorNew = 1; // Whether to add __attribute__((malloc))
                                      // to the declaration of C++'s new
                                      // operators
   langInfo.ElideConstructors = 1; // Whether C++ copy constructors should be
                                   // elided if possible.
   langInfo.CatchUndefined = 1; // Generate code to check for undefined ops.
   langInfo.DumpRecordLayouts = 0; /// Dump the layout of IRgen'd records.
   langInfo.DumpVTableLayouts = 0; /// Dump the layouts of emitted vtables.
   langInfo.NoConstantCFStrings = 0;  // Do not do CF strings
   langInfo.InlineVisibilityHidden = 0; // Whether inline C++ methods have
                                        // hidden visibility by default.

   langInfo.SpellChecking = 0; // Whether to perform spell-checking for error
                               // recovery.
   langInfo.SinglePrecisionConstants = 0; // Whether to treat double-precision
                                          // floating point constants as
                                          // single precision constants.
   langInfo.FastRelaxedMath  = 0; // OpenCL fast relaxed math (on its own,
                                  // defines __FAST_RELAXED_MATH__).
   langInfo.NoBitFieldTypeAlign = 0; // FIXME: This is just a temporary option, for testing purposes.


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
      #ifdef LLVM_2_8
      clang::ASTReader::getOriginalSourceFile(fileName, CI->getDiagnostics());
      #else
      clang::ASTReader::getOriginalSourceFile(fileName, CI->getFileManager(), /*CI->getFileSystemOpts(),*/ CI->getDiagnostics());
      #endif

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

   clang::CompilerInstance * CI = CreateCI ();

   if (pch)
      OpenPCH (CI, fileName);

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
              0)
   );

   CI->setASTConsumer (new clang::ASTConsumer());

   PP.getBuiltinInfo().InitializeBuiltins
      (
       PP.getIdentifierTable (),
       #ifdef LLVM_2_8
          PP.getLangOptions().NoBuiltin
       #else
          PP.getLangOptions()
       #endif
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
      #ifdef LLVM_2_8
         const clang::FileEntry* File = CI->getFileManager().getFile(fileName);
      #else
         const clang::FileEntry* File = CI->getFileManager().getFile(fileName);
      #endif
      if (File)
         CI->getSourceManager().createMainFileID(File);
      if (CI->getSourceManager().getMainFileID().isInvalid())
         error ("Error reading file");
   }

   clang::ParseAST(PP, & CI->getASTConsumer (), CI->getASTContext ());

   if (CI->hasPreprocessor())
      CI->getPreprocessor().EndSourceFile();

   #ifdef LLVM_2_8
      CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnostics().getNumErrors());
   #else
      CI->clearOutputFiles(/*EraseFiles=*/CI->getDiagnosticClient().getNumErrors());
   #endif
   CI->getDiagnosticClient().EndSourceFile();

   #ifdef LLVM_2_8
      unsigned err_count = CI->getDiagnostics().getNumErrors();
   #else
      unsigned err_count = CI->getDiagnosticClient().getNumErrors();
   #endif
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
   Reflex::DictionaryGenerator generator;
   GlobalScope_GenerateDict (generator);

   std::ofstream stream (outputFileName.c_str ());
   stream << generator;
}

void WriteClassicDictionary (const std::string& outputFileName,
                             const std::string& selectionFileName)
{
   Reflex::Scope scope = Reflex::Scope::GlobalScope ();

   Reflex::DictionaryGenerator generator;
   scope.GenerateDict(generator);

   generator.Use_selection (selectionFileName);
   generator.Print (outputFileName);
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

void SimpleTest ()
{
   Reflex::Type t;
   Reflex::Object o;
   Reflex::Member m;

   t = Reflex::Type::ByName("Example");

   o = t.Construct();

   m = t.MemberByName("hello");
   m.Invoke(o);
}

/* -------------------------------------------------------------------------- */

int main (int argc, const char **argv)
{
   #ifdef TRACE
      set_error_handlers ();
   #endif

   #if 0
      std::cout << "size of Example::t is " << sizeof (Example::t) << std::endl;
      std::cout << "size of C::t is " << sizeof (C::t) << std::endl;
      std::cout << "size of Example::u is " << sizeof (Example::u) << std::endl;
      return 0;
   #endif

   #if 0
      llvm::sys::PrintStackTraceOnErrorSignal();
      llvm::PrettyStackTraceProgram X(argc, argv);
   #endif

   std::string input_file = "";
   std::string selection_file = "";

   std::string dict_file = "";
   std::string classic_dict_file = "";

   #ifdef ENABLE_DEBUG_PRINT
   std::string debug_print_file = "";
   #endif
   #ifdef ENABLE_DEBUG_COMMANDS
   std::string debug_command_file = "";
   #endif

   bool unknown_option = false;
   bool too_many_args = false;

   for (int i = 1; i < argc; i ++) {
      std::string s = argv[i];

      if (s[0] == '-') {

         if (s == "--dict" && i+1<argc)
            dict_file = argv[++i];
         else if (s == "--classic-dict" && i+1<argc)
            classic_dict_file = argv[++i];
         #ifdef ENABLE_DEBUG_PRINT
         else if (s == "--debug-print" && i+1<argc)
            debug_print_file = argv[++i];
         #endif
         #ifdef ENABLE_DEBUG_COMMANDS
         else if (s == "--debug-commands" && i+1<argc)
            debug_command_file = argv[++i];
         #endif
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
      std::cout << "   [--classic-dict file_name]" << std::endl;
      #ifdef ENABLE_DEBUG_PRINT
      std::cout << "   [--debug-print file_name]" << std::endl;
      #endif
      #ifdef ENABLE_DEBUG_COMMANDS
      std::cout << "   [--debug-commands file_name]" << std::endl;
      #endif
         return 1;
      }

   // allocate gClrReg singleton
   TClrReg::Init();

   FillDictionary(input_file, selection_file);

   if (dict_file != "")
      WriteDictionary (dict_file);

   if (classic_dict_file != "")
      WriteClassicDictionary (classic_dict_file, selection_file);

   #if 1
   Process(input_file);
   #endif

   #if 0
      SimpleTest ();
   #endif

   #ifdef ENABLE_DEBUG_PRINT
   if (debug_print_file != "") {
      /* print Reflex dictionary - only for debugging */
      Reflex::Scope scope = Reflex::Scope::GlobalScope();
      TReflexPrinter io;
      io.Open(debug_print_file);
      io.EnableHtml(true);
      io.Print(scope);
      io.Close();
   }
   #endif

   #ifdef ENABLE_DEBUG_COMMANDS
   if (debug_command_file != "") {
      /* execute some methods - only for debugging */
      TReflexCommands io;
      io.Open(debug_command_file);
      io.Statements();
      io.Close();
   }
   #endif

   return 0;
}
