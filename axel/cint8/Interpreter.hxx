#include "FileRepository.h"

namespace Cint {
   // The interpreter.
   // NOTE: prompt interface (".L" etc) is not handled by the interpreter!

   class Interpreter {
   public:
      // Creates private Reflex instance. Lookup happens first on that local one, then
      // on the static one, on a per-container base, i.e. each reflex container is
      // a linked list local -> static.
      // Also attaches a private file repo object.
      // The name is used for debug output to identify the instance; the
      // allowDebugging flag can disable all of the tracepoint, callstack, source inspection
      // interfaces (labeled "Debug" and "Execution mode" below), for "closed" execution.
      Interpreter(const char* name = "CINT::Interpreter", bool allowDebugging = true);

      //======================
      //=== User interface ===
      //======================

      enum EError {
         kSuccess, // interpreter did not encounter an error while interpreting; the result is valid
         kErrorSource, // the source is not valid C[++]
         kErrorRuntime, // the interpreter encountered an error, e.g. division by zero, invalid cast, etc
         kErrorAbort, // the operation was aborted
         kNumErrors
      };
      // Loads and executes a piece of source code. Example:
      //    Execute(SourceCode("int a=0; ++a;")) // returns a Value object representing "int 1".
      //    Execute(SourceFile("MyMacro.C")) // returns the result of running the function MyMacro() 
      //       in the source file MyMacro.C
      // If addScopeDelimiter is true, the source will be surrounded by a pair of '{', '}' to
      // signal an unnamed macro.
      // If the returned Value is of type Cint::Interpreter::EError, its value with signal
      // the error encountered (i.e. kSuccess is never returned).
      Value Execute(Source& source, bool addScopeDelimiter = false);

      // Retrieve the the error code from the value returned by Execute
      EError GetError(const Value& value) const;

      // Loads source into the interpreter.
      // Initializes statics and adds types etc defined in the file to the Interpreters dictionary.
      // Fails for unnamed macros. ACLiC's "+" is handled by UserInterface.
      FileEntry* Load(Source& source);

      // Unload a file and all its dependencies; equivalent to FileRepository::Unload.
      // Returns true if the unload was successful.
      bool Unload(FileEntry* file);

      // semi-opaque type for checkpointing. Iterator on FileRepository's file entries;
      // ensures that the user-reference stays valid (in contrast to a simple FileEntry*)
      typedef FileEntry::Ref_t Checkpoint_t;

      // Create a checkpoint at the current Interpreter change
      // (dictionary, loaded files, etc). To be used by Reset later.
      const Checkpoint_t Checkpoint();

      // Reset the interpreter to an earlier state checkpointed by a call to Checkpoint()
      // or a call to Load (Reset and Unload are equivalent). Return true if reset was
      // successful.
      bool Reset(const Checkpoint_t& checkpoint);

      // Return true of the source is valid C++.
      // Currently simply bytecode compiles the source.
      bool CheckSyntax(Source& source);

      // Bytecode compile the source, with optimization optlevel.
      // Returns true on success.
      bool BytecodeCompile(Source& source, int optlevel = 2);

      // Bytecode compile a function, with optimization optlevel.
      // Returns true on success. Might nmodify func's MemberImpl::Properties().
      bool BytecodeCompile(Reflex::Member& func, int optlevel = 2);

      // Access to ACLiC, initialized with interpreter's include path etc:
      Aclic& Compiler();

      // Retrieve the Interpreter's file repository. Used to e.g. add include paths.
      FileRepository* GetFileReporitory() const;

      // Retrieve the Interpetere's garbage collector; 0 if not used.
      GarbageCollector* GetGarbageCollector() const;

      // Clone the interpreter, returning a new instance with its own, copied,
      // and thus independent context (aka fork).
      Interpreter* Fork() const;

      // On-demand template instantiation: instantiate template named name.
      Reflex::Type Instantiate(const char* name);

      // Lookup type named name, instantiating template if needed.
      // Note: Catalog().Lookup() does not instantiate templates!
      // Matches the functions of Reflex::NameLookup
      Reflex::Type LookupType(const std::string& nam, const Reflex::Scope& current);
      Reflex::Scope LookupScope(const std::string& nam, const Reflex::Scope& current);
      Reflex::Member LookupMember(const std::string& nam, const Reflex::Scope& current);

      // Function to call whenever a member lookup fails.
      // Allows to add special members (e.g. gDirectory content for ROOT).
      // Signature: Member callback(const Reflex::Scope& specialMemberScope, const std::string& name)
      // The function is called during a LookupMember in the Interpreter's Catalog.
      // Its specialMemberScope references a special ScopeImpl with a MemberByName() that calls
      // all callbacks until one callback returns a Member != Dummy::Member.
      typedef void* SpecialMemberCallbackHandle_t;
      SpecialMemberCallbackHandle_t RegisterSpecialMemberCallback(const Reflex::Callback<Member, const Reflex::Scope&, const std::string&>& callback);
      bool UnregisterSpecialMemberCallback(SpecialMemberCallbackHandle_t handle);


      //=======================
      //=== Debug interface ===
      //=======================

      // Message source: where does the message come from?
      enum EMessageSource {
         kInterpreter, // messages from the interpreter itself
         kBytecode, // bytecode-related messages
         kParser, // the parser's messages
         kExecution, // execution engine call related messages (method calls etc)
         kNumMessageSources
      };

      // Message severity: how important is the message?
      enum ESeverity {
         kFatal,              // fatal errors
         kError,              // recoverable errors
         kWarning,            // warnings
         kInfo,               // informational messages
         kDebug,              // debugging messages for user code
         kNumSeverity
      };

      // Register a void callback(EMessageSource, ESeverity, const char* message) for printing interpreter messages
      typedef void* MessageCallbackHandle_t;
      MessageCallbackHandle_t RegisterMessageCallback(const Reflex::Callback<void, EMessageSource, ESeverity, const char*> & callback);
      void UnregisterMessageCallback(MessageCallbackHandle_t);

      // Write source code to out, given a range of lines.
      // SourceLineRange defined in Trace.h
      void GetSource(std::string& out, const SourceLineRange& lines, bool preprocessed = false);

      // Retrieve current function stack. CallStack defined in Trace.h
      const CallStack& GetCallStack() const;


      // Register a trace callback, see TraceInfo. A TraceInfo object already registered with
      // another Interpreter instance will be removed from the other instance. Copy the
      // TraceInfo object instead. ITrace defined in Trace.h
      void Trace(ITrace& traceinfo);
      // Unregister trace
      void Untrace(ITrace& traceinfo);

      // Retrieve the interpreter's dictionary instance, used e.g.
      // for printing all known globals, all known classes, etc.
      const Reflex::Instance& GetDictionary() const;



      //====================================
      //=== Preliminary parser interface ===
      //====================================

      typedef void* PragmaCallbackHandle_t;
      // Register a void callback(const std::string&) to be invoked when the parser
      // sees a "#pragma " line.
      // Needed e.g. for handling "#pragma link"
      PragmaCallbackHandle_t RegisterPragmaCallback(const Reflex::Callback<void,const std::string&>& callback);
      bool UnregisterPragmaCallback(PragmaCallbackHandle_t pragmacallback);



      //================================
      //=== Execution mode interface ===
      //================================

      // Set the minimum warning level for messages to be sent to the callback registered by SetMessageCallback()
      void      SetWarningLevel(ESeverity severity = 0);
      ESeverity GetWarningLevel() const;

      // Whether undeclared variables are silently declared
      void SetAutoVarDeclaration(bool on = true);
      bool GetAutoVarDeclaration() const;

      // Whether CINT counts references and does garbage collection
      void EnableGarbageCollection(bool collect = true);
      // query using GetGarbageCollector()

      // CINT bytecode optimization level
      void SetBytecodeOptimization(int level = 2);
      int  GetBytecodeOptimization() const;

      // Do we need to deal with CINT's .lang: local language (EUC,SJIS,EUROPEAN,UNKNOWN)?
      
      // .q(qqqqq) implemented by meta interface.
      // .save only calls user settable emergency function - implemented by meta interface (if at all)
      // 
   };
}
