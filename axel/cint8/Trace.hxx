namespace Cint {
   // Class used to specify a range in the source file.
   class SourceLineRange {
   public:
      enum ELineSpec {
         kRelative, // line numbers are relative
         kAbsolute // line numbers are absolute
      };
      // Specify line numbers of a given stack frame, the ten lines surrounding the current line by default.
      SourceLineRange(int stackframe, long linefrom = -5, long lineto = 5, ELineSpec linespec = kRelative);
      // Specify absolute line numbers.
      SourceLineRange(const FileEntry::Ref& file, long linefrom, long lineto);
   };

   // Class representing a function call frame of the interpreter
   class CallFrame {
   public:
      // function name
      const std::string& Name() const;
      // location of the frame's function: file
      FileEntry::Ref File() const;
      // location of the frame's function: line
      long Line() const;
      // parameters the function was called with (with their current values)
      const Reflex::FunctionCall::Parameters& Parameters() const;
      // the frame's local variables; might be within by enclosing scopes in the same frame
      Reflex::Scope LocalVariables() const;
   };

   // Class representing the interpreter's function call stacks
   class CallStack {
   public:
      // return the depth of the call stack
      int Depth() const;
      // Return the frameidx-th frame; 0 being the current one
      const CallFrame& Frame(int frameidx) const;
   };

   class ExecutionModifier {
   public:
      enum EKind {
         kReturn, // return immediately
         kJump, // jump to line
         kNumKinds
      };

      // Initialize an execution modifier with a kind
      ExecutionModifier(EKind);
      // Initialize a kReturn-type execution modifier specifying the return's value
      ExecutionModifier(const Reflex::Value&);
      // Initialize a kJump-type execution modifier specifying the jump's target line number
      ExecutionModifier(long lineno);

      EKind GetKind() const;
   };

   class ITrace {
   public:
      // Create a TraceInfo given a function to call for every trace point.
      ITrace() {}
      // Destruct a ITrace object
      virtual ~ITrace() {}

      // Identifier for the trace point
      virtual const char* Name() { return "ANONYMOUS TRACE POINT"; }

      // Called by the Interpreter when it is attached using Interpreter::Trace()
      virtual void Attached(const Interpreter& /*interp*/) {}

      // Called by Interpreter when a callback is triggered.
      // If return != 0, the Interpreter will take the ExecutionModifier into account
      // for the expression that the trace point was triggered for.
      virtual const ExecutionModifier* Callback() = 0;

      // Called by the Interpreter to retrieve the CallStack object to be
      // updated before the call to Callback(). Return 0 if no callstack info is needed.
      // This interface is at least as fast as Interpreter::GetCallStack().
      virtual CallStack* CallStackToUpdate() const { return 0; }

      // Whether Callback() should be invoked; usually a function of
      // the context as specified by ContextRequests(). Non-const in case
      // the trace point does e.g. counting.
      virtual bool IsActive() { return false; }

      // Which context data is required by the Interpreter for filtering,
      // and which events must occur (i.e. context changes) for Callback() to be
      // triggered
      enum EContextRequest {
         kFileEntry      = 0x01, // Interpreter calls SetFileEntry() before calling Active()
         kFileLine       = 0x02, // Interpreter calls SetFileLine() before calling Active()
         kFunctionMember = 0x04, // Interpreter calls SetFunctionMember() before calling Active()
         kScope          = 0x08, // Interpreter calls SetScope() before calling Active()
         kExecutionState = 0x10, // Interpreter calls SetExecutionState() before calling Active()
         kBytecodeOp     = 0x20, // Next bytecode operation to be executed
         kNumFilterAll   = 0x3f
      };

      // which context info is needed from the Interpreter to determine the result of Active()
      // E.g. if the trace point is at file SomeFile.cxx, line 123, it would return
      // (kFileName | kFileLine) here, and test in IsActive() whether the condition matches,
      // given the values set by the Interpreter's calls to SetFileName(), SetFileLine().
      virtual int ContextRequests() const { return 0; }

      // For change of execution state:
      enum EExecutionState {
         kInterpretationEnter, // start of regular expression evaluation inside the interpreter
         kInterpretationExit, // end regular expression evaluation inside the interpreter
         kBytecodeEnter, // start of bytecode execution
         kBytecodeExit, // end of bytecode execution
         kCompiledEnter, // start of execution of compiled code
         kCompiledExit, // end of execution of compiled code
         kFunctionEnter, // Interpreter is about to enter a function
         kFunctionReturn, // Interpreter has just returned from a function
         kThrow, // an interpreted exception has been encountered
         kCatch, // an interpreted exception has been caught
         kInterpreterError, // an interpreter error has been detected
         kNumExecutionStates
      };

      // The Interpreter's way of informing about the next expression's context:
      // * its FileEntry
      virtual void SetFileEntry(const FileEntry::Ref_t& fileentry) {};
      // * its line number in the current file
      virtual void SetFileLine(long line) {};
      // * its function Reflex::Member
      virtual void SetFunctionName(const Reflex::Member& func) {};
      // * its function's declaring Reflex::Scope
      virtual void SetScope(const Reflex::Scope& scope) {};
      // * its execution state
      virtual void SetExecutionState(EExecutionState state) {}
      // * its bytecode operation (len bytes of data)
      virtual void SetBytecodeOp(void* data, int len) {}
   };
}
