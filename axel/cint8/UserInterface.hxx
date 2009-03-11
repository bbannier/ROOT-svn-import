namespace Cint {
   // Class interpreting the non-C++ interface ("."-commands) for interpreter steering.
   // Also handles ".L ...+" by invoking Interpreter::Aclic() on the file.
   class UserInterface {
   public:
      // Initialize a meta interpreter, create an Interpreter to attach to
      UserInterface();

      // Connect to an interpreter instance; return true if successful
      bool Connect(Interpreter& interp);

      // Send a command (or a series of commands, delimited by newlines) to the interpreter.
      // Returns the interpreter's return value, or Reflex::Dummy::Value if there is none.
      Reflex::Value Send(const char* command);
      Reflex::Value Send(istream& in);

      // Switch to a different interpreter to steer, stacking the current one
      // so it can be switched back to later.
      bool SwitchTo(Interpreter& interp);

      // Switch back to the previous interpreter
      bool SwitchBack();


      // Attach to "terminal"-like input in
      void CIn(istream& in);
      // Attach output to out
      void COut(ostream& out);
      // Attach error output to err
      void CErr(ostream& err);

      // Set the debugger to use
      void SetDebugger(UserInterface& debugger);

      // Activate the debugger meta-interface
      void ActivateDebugger();

      // whether exceptions should be caught and handled by the UserInterface
      void HandleExceptions(bool on = true);

      // terminate the interpreter; higher values of force will be more brutal
      void KillInterpreter(int force = 0);

      // Set the history file to use (0 if none)
      bool SetHistoryFile(const char* filename);

      // Maximum number of history lines to keep
      void SetNumHistoryLines(long lines);
      long GetNumHistoryLines() const;

      // Whether to keep duplicate history lines
      void SetDuplicateHistoryLines(bool allowDuplicates = true);
      bool GetDuplicateHistoryLines() const;

      // Completion bool callback(std::string& line_to_complete), returning true
      // if line_to_complete was changed.
      typedef void* CompletionCallbackHandle_t;
      CompletionCallbackHandle_t RegisterCompletionCallback(const Reflex::Callback<bool,std:string&>& callback);
      bool UnregisterCompletionCallback(CompletionCallbackHandle_t handle);

      // End of line bool callback(const std::string& line_that_ended), returning true
      // if line is to be processed.
      typedef void* EndOfLineCallbackHandle_t;
      EndOfLineCallbackHandle_t RegisterEndOfLineCallback(const Reflex::Callback<bool,const std:string&>& callback);
      bool UnregisterEndOfLineCallback(EndOfLineCallbackHandle_t handle);

   protected:
      class ReadlineInterface {
         // handle readline-based terminal interaction + history file management
      };
   };
}
