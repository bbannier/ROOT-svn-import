namespace Cint {
   // Represents source code, either as a source file, as an istream, or as a string,
   // see derived classes. Allows CINT to store source-specific data with the code
   // object, e.g. bytecode (which would also be Interpreter specific, as type names can
   // have different meanings for different Interpreter instances). Allows CINT to store
   // parse tree with source (in the future, once the parser is modularized). Allows CINT
   // to persistify the "pre-compiled" source.
   class Source {
   public:
      Source();
      virtual ~Source();

      // Return the istream representing the code for a given Interpreter instance.
      virtual istream& operator()(const Interpreter& interpreter);
   };

   // SourceCode creates a ghost FileEntry object to keep track of dependencies
   // for unloading and type registration. Allows definition of functions in
   // plain source without connection to file (new feature!)
   class SourceCode: public Source {
   public:
      // Create a code object passing source code, e.g. SourceCode s("int a; ++a;")
      SourceCode(const char* code);
      // Create a code object passing source code read from an istream
      SourceCode(std::istream& code);
   };

   // SourceFile determines the underlying file (aka FileRepository::Which()) once.
   // Subsequent uses of the SourceFile will reference the same file on disk even if
   // the interpreter has a changed -I path, but changed files will be re-read. The
   // result of Which() is cached per Interpreter object.
   class SourceFile: public Source {
   public:
      // Create a code object passing a file's name, e.g. SourceFile s("mymacro.C")
      // Implementation detail:
      // The Interpreter using a SourceFile object will register the result of calling
      // Which() within the SourceFile object, under its reference.
      SourceFile(const char* filename);
   };

}