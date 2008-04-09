namespace Cint {
   // Implement common functionality for making reflection information persistant,
   // aka dictionary generator.
   namespace ReflectionPersistency {
      // Reasons for other types to depend on a type
      enum EDependencyReason {
         kDependsName = 0x01, // other types need the type name declared, e.g. NeedName* fMember;
         kDependsType = 0x02, // other types need the completly defined type, e.g. NeedType fMember;
         kRequested   = 0x04, // thsi type was explicitly requested
         kDependencyReasonsAll = 0x07
      };

      // A type and why other types depend on it
      class TypeDependencyInfo {
      public:
         TypeDependencyInfo(const Reflex::Type& type, EDependencyReason reason);
         Reflex::Type Type() const;
         EDependencyReason DependencyReason() const;
      };

      // Given a set of types "in", calculate the ordered list of types "out" that
      // the types "in" depend on. Take into account name and type dependencies, i.e.
      // also deal with
      //   class B; class A { B* fB; }; class B { A fA; }.
      // After calling GetDependencies(set(A, B), out), out would contain
      //   0: TypeDependencyInfo(B, kDependsName)
      //   1: TypeDependencyInfo(A, kDependsType | kRequested)
      //   2: TypeDependencyInfo(B, kRequested)
      // This function is needed for creating fwd decls in dictionaries, and for determining
      // what to write the shadow classes for (only non-public, kDependsType | kRequested types)
      void GetDependencies(const std::set<Reflex::Type>& in, std::list<TypeDependencyInfo>& out);
   };

   // Abstract interface definition for a selecting which types, scopes, and members to generate
   // reflection output for.
   class ISelector {
   public:
      ISelector();
      virtual ~ISelector();

      // Bits signaling the result of the selection
      enum ESelectionResult {
         kThisNo  = 0x00, // do not generate reflection output for this element
         kThisYes = 0x01, // generate reflection output for this element

         kSubDataMembers = 0x10, // check again for members
         kSubFunctionMembers = 0x20, // check again for function members
         kSubTypes  = 0x40, // check again for types declared within this element, i.e. This::SubType
         kSubScopes = 0x80, // check again for scopes declared within this element, i.e. This::SubScope
         kSubAll    = 0xf0, // check again for sub-scopes, -types, -members
      };

      virtual ESelectionResult Selected(const Reflex::Scope&) = 0;
      virtual ESelectionResult Selected(const Reflex::Type&) = 0;
      virtual ESelectionResult Selected(const Reflex::Member&) = 0;
   };

   // Abstract interface definition for a writer of persistency information.
   class IReflectionWriter {
   public:
      IReflectionWriter();
      virtual ~IReflectionWriter();

      // For comments in generated output or messages
      virtual const char* Name() const = 0;
      virtual void Write(Interpreter& interp, ISelector& selector) = 0;
   };



   //////////////////////////////////////////////////////
   ////        USE CASES - not part of CINT8!
   //////////////////////////////////////////////////////

   // Class implementing LinkDef.h based selection mechanism
   class LinkDefSelector: public ISelector {
   public:
      // Registers pragma callback with interp
      LinkDefSelector(Interpreter& interp);

      // the callback function, seeing pragmaline occurrences
      void PragmaLinkCallback(const std::string& pragmaline);

      ESelectionResult Selected(const Reflex::Scope&);
      ESelectionResult Selected(const Reflex::Type&);
      ESelectionResult Selected(const Reflex::Member&);
   };

   // Class implementing selection.xml based selection mechanism
   class XMLSelector: public ISelector {
   public:
      // c'tor setting the selection file
      XMLSelector(Interpreter& interp, const char* selectionfile);

      ESelectionResult Selected(const Reflex::Scope&);
      ESelectionResult Selected(const Reflex::Type&);
      ESelectionResult Selected(const Reflex::Member&);
   };

   // Class implementing dictionary.cxx output
   class CxxReflectionWriter: public IReflectionWriter {
   public:
      // C'tor taking the name and the output stream
      CxxReflectionWriter(const char* name, ostream& out);

      // Return "CxxReflectionWriter" and name as set in the c'tor
      const char* Name() const;

      // Generate the dictionary output for all selected scopes, types, and members
      void Write(Interpreter& interp, ISelector& selector);
   };

   // Class implementing dictionary.root output
   class ROOTReflectionWriter: public IReflectionWriter {
   public:
      // C'tor taking the name and the output file name
      ROOTReflectionWriter(const char* name, const char* outfilename);

      // Return "CxxReflectionWriter" and name as set in the c'tor
      const char* Name() const;

      // Generate the dictionary output for all selected scopes, types, and members
      void Write(Interpreter& interp, ISelector& selector);
   };

}
