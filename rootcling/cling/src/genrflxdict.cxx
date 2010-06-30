
/* dict.cxx */

/* based on Reflex source code from 27-05-2010 */

#include "Reflex/Reflex.h"
#include "Reflex/DictionaryGenerator.h"
#include "Reflex/internal/ScopeBase.h"

using namespace Reflex;

#define CLASSIC_CODE
// #define CLING_STUB
#define ONLY_FUNCTIONS
#define NUMBERED_FUNCTIONS
// #define REGISTER_FUNCTIONS

#include "clr-scan.h"
#include "clr-info.h"

#ifdef REGISTER_FUNCTIONS
   #include "G__ci.h"
   // #include "clr-reg.h"
#endif

TContext* gFirstContext = NULL;
TContext* gLastContext = NULL;

TContext* GetFirstContext ()
{
   return gFirstContext;
}

void LinkContext (TContext * context)
{
   if (gLastContext == NULL)
      gFirstContext = context;
   else
      gLastContext->next = context;

   gLastContext = context;

   context->next = NULL;
}

void DeleteContexts ()
{
   TContext* p = gFirstContext;
   while (p != NULL)
   {
      TContext* t = p;
      p = p->next;
      delete t;
   }

   gFirstContext = NULL;
   gLastContext = NULL;
}

inline std::string IntToStd (int num)
{
   std::ostringstream stream;
   stream << num;
   return stream.str ();
}

void Member_GenerateDict(const Member & obj, DictionaryGenerator& generator);
void ScopeBase_GenerateDict(const Scope & obj, DictionaryGenerator& generator);

//-------------------------------------------------------------------------------
void
DataMember_GenerateDict(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   const Scope& declScope = obj.DeclaringScope();

   if (declScope.IsUnion()) {
      // FIXME

   } else if (declScope.IsEnum()) {
      std::stringstream tmp;
      tmp << obj.Offset();

      if (declScope.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances("\n.AddItem(\"" + obj.Name() + "\", " + tmp.str() + ")");
      } else { // class, struct
         generator.AddIntoFree(obj.Name() + "=" + tmp.str());
      }
   } else { // class, struct
      const Type& rType = obj.TypeOf().RawType();

      if (obj.TypeOf().IsArray()) {
         Type t = obj.TypeOf();

         std::stringstream temp;
         temp << t.ArrayLength();

         generator.AddIntoShadow(t.ToType().Name(SCOPED) + " " + obj.Name() + "[" + temp.str() + "];\n");

      } else if (obj.TypeOf().IsPointer() && obj.TypeOf().RawType().IsFunction()) {
         Type t = obj.TypeOf().ToType();
         generator.AddIntoShadow(t.ReturnType().Name(SCOPED) + "(");

         if (t.DeclaringScope().IsClass()) {
            generator.AddIntoShadow(t.DeclaringScope().Name(SCOPED) + "::");
         }

         generator.AddIntoShadow("*" + t.Name() + ")(");

         for (size_t parameters = 0; parameters < t.FunctionParameterSize();
              ++parameters) {
            generator.AddIntoShadow(t.FunctionParameterAt(parameters).Name());

            if (t.FunctionParameterSize() > parameters) {
               generator.AddIntoShadow(",");
            }
         }

         generator.AddIntoShadow(");\n");

      } else {
         std::string tname = obj.TypeOf().Name(SCOPED);

         if ((rType.IsClass() || rType.IsStruct()) && (!rType.IsPublic())) {
            tname = generator.Replace_colon(rType.Name(SCOPED));

            if (rType != obj.TypeOf()) {
               tname = tname + obj.TypeOf().Name(SCOPED).substr(tname.length());
            }
         }
         generator.AddIntoShadow(tname + " " + obj.Name() + ";\n");
      }

      //register type and get its number
      std::string typenumber = generator.GetTypeNumber(obj.TypeOf());

      generator.AddIntoFree(".AddDataMember(type_" + typenumber + ", \"" + obj.Name() + "\", ");
      generator.AddIntoFree("OffsetOf (__shadow__::" +
                            generator.Replace_colon(obj.DeclaringScope().Name(SCOPED)));
      generator.AddIntoFree(", " + obj.Name() + "), ");

      if (obj.IsPublic()) {
         generator.AddIntoFree("PUBLIC");
      } else if (obj.IsPrivate()) {
         generator.AddIntoFree("PRIVATE");
      } else if (obj.IsProtected()) {
         generator.AddIntoFree("PROTECTED");
      }

      if (obj.IsVirtual()) {
         generator.AddIntoFree(" | VIRTUAL");
      }

      if (obj.IsArtificial()) {
         generator.AddIntoFree(" | ARTIFICIAL");
      }

      generator.AddIntoFree(")\n");

   }
} // GenerateDict

//-------------------------------------------------------------------------------
void
FunctionMember_GenerateDict(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   std::string mName = obj.Name();

   if (mName != "__getNewDelFunctions" && mName != "__getBasesTable") {
      // The return type
      const Type& retT = obj.TypeOf().ReturnType();

      #ifndef ONLY_FUNCTIONS
      // Get function return type, register that to "used types"
      std::string returntype = generator.GetTypeNumber(obj.TypeOf().ReturnType());

      // Prevents __getNewDelFunctions getting into AddFunctionMember twice
      //if(generator.IsNewType( TypeOf() ) && Name()!="__getNewDelFunctions" ) {

      if (obj.IsPrivate()) {
         generator.AddIntoShadow(obj.Name(SCOPED) + "();\n");
      }
      #endif

      // Get a number for the function type
      //std::string number = generator.GetTypeNumber( TypeOf() );

      std::stringstream temp;
      #ifdef NUMBERED_FUNCTIONS
         int method_index = generator.fMethodCounter;
      #endif
      temp << generator.fMethodCounter;
      std::string number = temp.str();

      ++generator.fMethodCounter;

      // Get current Namespace location
      std::string namespc = obj.DeclaringScope().Name(SCOPED);

      std::stringstream tempcounter;
      tempcounter << generator.fMethodCounter;


      #ifndef ONLY_FUNCTIONS
      // Free function, shall be added into Instances-field only
      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances("      Type t" + tempcounter.str() + " = FunctionTypeBuilder(type_" + returntype);

      } else { // "normal" function, inside a class
         generator.AddIntoFree(".AddFunctionMember(FunctionTypeBuilder(type_" + returntype);
      }

      // Get the parameters for function
      for (Type_Iterator params = obj.TypeOf().FunctionParameter_Begin();
           params != obj.TypeOf().FunctionParameter_End(); ++params) {
         if (obj.DeclaringScope().IsNamespace()) {
            generator.AddIntoInstances(", type_" + generator.GetTypeNumber((*params)));

         } else {
            generator.AddIntoFree(", type_" + generator.GetTypeNumber((*params)));
         }

      }

      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances(");  FunctionBuilder(t" + tempcounter.str() + ", \""
                                    + obj.Name() + "\", function_" + number);  //function name
      } else { // normal function
         generator.AddIntoFree("), \"" + obj.Name() + "\"");  //function name
      }

      if (obj.IsConstructor()) {
         generator.AddIntoFree(", constructor_");
      } else if (obj.IsDestructor()) {
         generator.AddIntoFree(", destructor_");
      }
      #endif

      #ifdef CLASSIC_CODE
      if (obj.IsConstructor()) {
         #ifdef NUMBERED_FUNCTIONS
         generator.AddIntoClasses("void* __stub_");
         #else
         generator.AddIntoClasses("static void* constructor_");
         #endif

      } else if (obj.IsDestructor()) {
         #ifdef NUMBERED_FUNCTIONS
         generator.AddIntoClasses("void* __stub_");
         #else
         generator.AddIntoClasses("static void* destructor_");
         #endif

      } else {
         if (!(obj.DeclaringScope().IsNamespace())) {
            #ifndef ONLY_FUNCTIONS
            generator.AddIntoFree(", method_");
            #endif

            #ifdef NUMBERED_FUNCTIONS
            generator.AddIntoClasses("\n void* __stub_");
            #else
            generator.AddIntoClasses("\nstatic void* method_");
            #endif

         } else {
            // free function
            #ifdef NUMBERED_FUNCTIONS
            generator.AddIntoClasses("\nvoid* __stub_");
            #else
            generator.AddIntoClasses("\nstatic void* function_");
            #endif
         }
      }

      #ifdef NUMBERED_FUNCTIONS
         // info ("Registering stub function " + IntToStd (method_index));
         assert (obj.Stubfunction() == ClrStubFunction);
         TContext * context = reinterpret_cast <TContext *> (obj.Stubcontext ());
         assert (context != NULL);
         context->index = method_index;
         LinkContext (context);
         info ("Stub function written " + IntToStd (context->index));
      #endif

      #ifndef ONLY_FUNCTIONS
      if (!(obj.DeclaringScope().IsNamespace())) {
         generator.AddIntoFree(number);                                     //method_n


      }
      #endif
      generator.AddIntoClasses(number);

      #ifdef REGISTER_FUNCTIONS
         #if 0
         std::string funcName = obj.Name(Reflex::SCOPED) + "(";
         bool first = true;
         for (Type_Iterator params = obj.TypeOf().FunctionParameter_Begin();
              params != obj.TypeOf().FunctionParameter_End(); ++params) {
            if (! first)
               funcName += ",";
            first = false;

            Reflex::Type t = *params;
            funcName += t.Name(Reflex::SCOPED);
         }
         funcName += ")";
         #endif
         std::string funcName = TScanner::GetFuncProp(obj.Properties());

         gClrReg->RegisterFunctionStub (funcName.c_str (), NULL); // !?

         generator.AddIntoInstances("      RegisterFunctionStub(");

         generator.AddIntoInstances("\"");
         generator.AddIntoInstances(funcName);
         // generator.AddIntoInstances(G__map_cpp_name (funcName.c_str ()));
         generator.AddIntoInstances("\"");

         generator.AddIntoInstances(", ");

         if (obj.IsConstructor()) {
            generator.AddIntoInstances("constructor_");
         } else if (obj.IsDestructor()) {
            generator.AddIntoInstances("destructor_");
         } else {
            generator.AddIntoInstances("method_");
         }
         generator.AddIntoInstances(number);                                     //method_n
         generator.AddIntoInstances(");\n");
      #endif

      if (obj.IsConstructor()) {   // these have parameters
         generator.AddIntoClasses("(void* mem, const std::vector<void*>&");

         if (obj.FunctionParameterSize()) {
            generator.AddIntoClasses(" arg");
         }

         generator.AddIntoClasses(", void*)\n{");
         generator.AddIntoClasses("\n  return ::new(mem) " + namespc);
         generator.AddIntoClasses("(");

      } //is constructor/destructor with parameters
      else if (obj.IsDestructor()) {
         generator.AddIntoClasses("(void * o, const std::vector<void*>&, void *) {\n");
         generator.AddIntoClasses("  ((" + namespc + "*)o)->~" + obj.DeclaringScope().Name() + "(");
      } else {
         // method function with parameters

         if (obj.DeclaringScope().IsNamespace()) {
            generator.AddIntoClasses(" (void*, const std::vector<void*>&"); // arg, void*)\n{");

         } else {
            generator.AddIntoClasses(" (void* o, const std::vector<void*>&"); // arg, void*)\n{");
         }

         if (obj.FunctionParameterSize() > 0) {
            generator.AddIntoClasses(" arg");
         }

         generator.AddIntoClasses(", void*)\n{");

         if (retT.Name() != "void") {
            if (retT.IsFundamental()) {
               generator.AddIntoClasses("static " + retT.Name(SCOPED) + " ret;\n");
               generator.AddIntoClasses("ret = ");
            } else if (retT.IsReference() || retT.IsPointer()) {
               generator.AddIntoClasses("return (void*)");

               if (retT.IsReference()) {
                  generator.AddIntoClasses("&");
               }
            } else { // compound type
               generator.AddIntoClasses("return new " + retT.Name(SCOPED) + "(");
            }
         }

         if (obj.DeclaringScope().IsNamespace()) {
            generator.AddIntoClasses(obj.Name() + "( ");

         } else {
            generator.AddIntoClasses("((" + namespc + "*)o)->" + obj.Name() + "( ");
         }

      }

      // Add to Stub Functions with some parameters
      if (obj.FunctionParameterSize() > 0) {
         unsigned args = 0;

         // Get all parameters
         for (Type_Iterator methpara = obj.TypeOf().FunctionParameter_Begin();
              methpara != obj.TypeOf().FunctionParameter_End(); ++methpara) {
            // get params for the function, can include pointers or references
            std::string param = generator.GetParams(*methpara);

            std::stringstream temp2;
            temp2 << args;

            // We de-deference parameter only, if it's not a pointer
            if (!methpara->IsPointer()) {
               generator.AddIntoClasses("*");
            }

            generator.AddIntoClasses("(");

            //if( methpara->IsConst()) generator.AddIntoClasses("const ");

            std::string paraT = methpara->Name(SCOPED | QUALIFIED);

            if (methpara->IsReference()) {
               paraT = paraT.substr(0, paraT.length() - 1);
            }

            generator.AddIntoClasses(paraT);

            if (!methpara->IsPointer()) {
               generator.AddIntoClasses("*");
            }

            generator.AddIntoClasses(") arg[" + temp2.str() + "]");

            //still parameters left
            if ((args + 1) < obj.FunctionParameterSize()) {
               generator.AddIntoClasses(", ");
            }

            ++args;
            //fundam

         }

      } // funct. params!=0

      if (obj.IsConstructor()) {
         generator.AddIntoClasses(");\n} \n");

      } else {
         generator.AddIntoClasses(")");

         if (retT.Name() == "void") {
            generator.AddIntoClasses(";\n  return 0;\n");
         } else if (retT.IsFundamental()) {
            generator.AddIntoClasses(";\n  return & ret;\n");
         } else if (retT.IsPointer() || retT.IsReference()) {
            generator.AddIntoClasses(";\n");
         } else { // compound type
            generator.AddIntoClasses(");\n");
         }

         generator.AddIntoClasses("\n} \n"); //);
      }

      #ifndef ONLY_FUNCTIONS
      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances(", 0");
      } else {
         generator.AddIntoFree(", 0");
      }
      #endif
      #endif

      #ifdef CLING_STUB
         if (!(obj.DeclaringScope().IsNamespace())) {
            generator.AddIntoFree(", ClingStub, new StubParam (\"");
            std::string s = obj.Properties().PropertyAsString ("func"); // !? TScanner::fgFuncKey
            generator.AddIntoFree(s);
            generator.AddIntoFree("\")");
         }
      #endif

      #ifndef ONLY_FUNCTIONS
      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances(", \"");
      } else { generator.AddIntoFree(", \""); }

      // Get the names of the function param.types (like MyInt)
      if (obj.TypeOf().FunctionParameterSize()) {
         unsigned dot = 0;
         Type_Iterator params;
         StdString_Iterator parnam;

         for (params = obj.TypeOf().FunctionParameter_Begin(), parnam = obj.FunctionParameterName_Begin();
              params != obj.TypeOf().FunctionParameter_End(), parnam != obj.FunctionParameterName_End();
              ++params, ++parnam) {
            // THESE SHOULD ALSO INCLUDE DEFAULT VALUES,
            // LIKE int i=5 FunctionParameterDefault_Begin(), _End
            //

            if (obj.DeclaringScope().IsNamespace()) {
               generator.AddIntoInstances((*parnam));

               if ((dot + 1) < obj.FunctionParameterSize()) {
                  generator.AddIntoInstances(";");
               }
            } else {
               generator.AddIntoFree((*parnam));

               if ((dot + 1) < obj.FunctionParameterSize()) {
                  generator.AddIntoFree(";");
               }
            }
            ++dot;
         }

      }

      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances("\"");
      } else { generator.AddIntoFree("\""); }

      if (obj.DeclaringScope().IsNamespace()) {  // free func
         generator.AddIntoInstances(", ");

         if (obj.IsPublic()) {
            generator.AddIntoInstances("PUBLIC");
         } else if (obj.IsPrivate()) {
            generator.AddIntoInstances("PRIVATE");
         } else if (obj.IsProtected()) {
            generator.AddIntoInstances("PROTECTED");
         }

         if (obj.IsArtificial()) {
            generator.AddIntoInstances(" | ARTIFICIAL");
         }

         generator.AddIntoInstances(");\n");

         // !? ++generator.fMethodCounter;

      } else {
         generator.AddIntoFree(", ");

         if (obj.IsPublic()) {
            generator.AddIntoFree("PUBLIC");
         } else if (obj.IsPrivate()) {
            generator.AddIntoFree("PRIVATE");
         } else if (obj.IsProtected()) {
            generator.AddIntoFree("PROTECTED");
         }

         if (obj.IsVirtual()) {
            generator.AddIntoFree(" | VIRTUAL");
         }

         if (obj.IsArtificial()) {
            generator.AddIntoFree(" | ARTIFICIAL");
         }

         if (obj.IsConstructor()) {
            generator.AddIntoFree(" | CONSTRUCTOR");
         }

         if (obj.IsDestructor()) {
            generator.AddIntoFree(" | DESTRUCTOR");
         }

         generator.AddIntoFree(")\n");
      }
      #endif
   }
} // GenerateDict

//-------------------------------------------------------------------------------
void
Enum_GenerateDict(const Scope & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   size_t lastMember = obj.DataMemberSize() - 1;

   if (!(obj.DeclaringScope().IsNamespace())) {
      generator.AddIntoFree("\n.AddEnum(\"" + obj.Name() + "\", \"");

      for (size_t i = 0; i < obj.DataMemberSize(); ++i) {
         Member_GenerateDict (obj.DataMemberAt(i), generator);

         if (i < lastMember) {
            generator.AddIntoFree(";");
         }
      }

      generator.AddIntoFree("\",");

      if (obj.IsPublic()) {
         generator.AddIntoFree("typeid(" + obj.Name(SCOPED) + "), PUBLIC)");
      } else if (obj.IsProtected()) {
         generator.AddIntoFree("typeid(Reflex::ProtectedEnum), PROTECTED)");
      } else if (obj.IsPrivate()) {
         generator.AddIntoFree("typeid(Reflex::PrivateEnum), PRIVATE)");
      }
   } else {
      generator.AddIntoInstances("      EnumBuilder(\"" + obj.Name(SCOPED) + "\", typeid(" + obj.Name(SCOPED) + "), PUBLIC)");

      for (size_t i = 0; i < obj.DataMemberSize(); ++i) {
         Member_GenerateDict(obj.DataMemberAt(i), generator);
      }
      generator.AddIntoInstances(";\n");

   }
} // GenerateDict

//-------------------------------------------------------------------------------
void
Class_GenerateDict(const TypeBase & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   // Selection file usage
   bool selected = true;

   /*
      // selection file used
      if (generator.fSelections.size() != 0 || generator.fPattern_selections.size() != 0) {
      selected = false;

      // normal selection
      for (unsigned i = 0; i < generator.fSelections.size(); ++i) {
         if (generator.fSelections.at(i) == (*this).Name(SCOPED)) {
            selected = true;
         }
      }

      // pattern selection
      for (unsigned i = 0; i < generator.fPattern_selections.size(); ++i) {
         if ((*this).Name(SCOPED).find(generator.fPattern_selections.at(i)) != std::string::npos) {
            selected = true;
         }
      }

      }
      // EndOf Selection file usage
    */

   if (selected == true) {
      #ifndef ONLY_FUNCTIONS
      std::string typenumber = generator.GetTypeNumber(obj.ThisType());

      if (generator.fSelections.size() != 0 || generator.fPattern_selections.size() != 0) {
         std::cout << "  * selecting class " << obj.Name(SCOPED) << "\n";
      }

      generator.AddIntoInstances("      " + generator.Replace_colon(obj.ThisType().Name(SCOPED)) + "_dict();\n");

      // Outputten only, if inside a namespace
      if (obj.ThisType().DeclaringScope().IsTopScope() && (!obj.DeclaringScope().IsNamespace())) {
         generator.AddIntoShadow("\nnamespace " + obj.ThisType().Name() + " {");
      }

      // new
      if (obj.ThisType().DeclaringScope().IsClass()) {
         generator.AddIntoShadow("};");
      }


      // begin of the Dictionary-part
      generator.AddIntoShadow("\nclass " + generator.Replace_colon(obj.ThisType().Name(SCOPED)) + " {\n");
      generator.AddIntoShadow("public:\n");

      if ((obj.ThisType().DeclaringScope().IsClass())) {
         generator.AddIntoFree(";\n}\n");
      }

      generator.AddIntoFree("\n\n// ------ Dictionary for class " + obj.ThisType().Name() + "\n");
      generator.AddIntoFree("void " + generator.Replace_colon(obj.ThisType().Name(SCOPED)) + "_dict() {\n");
      generator.AddIntoFree("ClassBuilder(\"" + obj.ThisType().Name(SCOPED));

      if (obj.IsPublic()) {
         generator.AddIntoFree("\", typeid(" + obj.ThisType().Name(SCOPED) + "), sizeof(" + obj.ThisType().Name(SCOPED) + "), ");
      } else if (obj.IsProtected()) {
         generator.AddIntoFree("\", typeid(Reflex::ProtectedClass), 0,");
      } else if (obj.IsPrivate()) {
         generator.AddIntoFree("\", typeid(Reflex::PrivateClass), 0,");
      }

      if (obj.ThisType().IsPublic()) {
         generator.AddIntoFree("PUBLIC");
      }

      if (obj.ThisType().IsPrivate()) {
         generator.AddIntoFree("PRIVATE");
      }

      if (obj.ThisType().IsProtected()) {
         generator.AddIntoFree("PROTECTED");
      }

      if (obj.ThisType().IsVirtual()) {
         generator.AddIntoFree(" | VIRTUAL");
      }
      generator.AddIntoFree(" | CLASS)\n");
      #endif

      generator.AddIntoClasses("\n// -- Stub functions for class " + obj.ThisType().Name() + "--\n");

      Scope scope = obj;
      for (Member_Iterator mi = scope.Member_Begin();
           mi != scope.Member_End(); ++mi) {
         Member_GenerateDict(*mi, generator);      // call Members' own gendict
      }

      #ifndef ONLY_FUNCTIONS
      if (obj.ThisType().DeclaringScope().IsTopScope() && (!obj.DeclaringScope().IsNamespace())) {
         generator.AddIntoShadow("\nnamespace " + obj.ThisType().Name() + " {");
      }

//       std::stringstream tempcounter;
//       tempcounter << generator.fMethodCounter;
//       generator.AddIntoClasses("\nstatic void* method_x" + tempcounter.str());
//       generator.AddIntoClasses(" ( void*, const std::vector<void*>&, void*)\n{\n");
//       generator.AddIntoClasses("  static NewDelFunctions s_funcs;\n");

//       generator.AddIntoFree(".AddFunctionMember<void*(void)>(\"__getNewDelFunctions\", method_x" + tempcounter.str());
//       generator.AddIntoFree(", 0, 0, PUBLIC | ARTIFICIAL)");

//       std::string temp = "NewDelFunctionsT< ::" + ThisType().Name(SCOPED) + " >::";
//       generator.AddIntoClasses("  s_funcs.fNew         = " + temp + "new_T;\n");
//       generator.AddIntoClasses("  s_funcs.fNewArray    = " + temp + "newArray_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDelete      = " + temp + "delete_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDeleteArray = " + temp + "deleteArray_T;\n");
//       generator.AddIntoClasses("  s_funcs.fDestructor  = " + temp + "destruct_T;\n");
//       generator.AddIntoClasses("  return &s_funcs;\n}\n ");

//       ++generator.fMethodCounter;

      if (obj.ThisType().DeclaringScope().IsTopScope() && (!obj.DeclaringScope().IsNamespace())) {
         generator.AddIntoShadow("}\n");        // End of top namespace
      }

      // Recursive call
      ScopeBase_GenerateDict(obj, generator);

      if (!(obj.ThisType().DeclaringScope().IsClass())) {
         generator.AddIntoShadow("};\n");
      }

      if (!(obj.ThisType().DeclaringScope().IsClass())) {
         generator.AddIntoFree(";\n}\n");
      }
      #endif

   } //new type
} // GenerateDict


//-------------------------------------------------------------------------------
void
Namespace_GenerateDict(const Scope & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   if (obj.Name() != "" && generator.IsNewType(obj)) {
      std::stringstream tempcounter;
      tempcounter << generator.fMethodCounter;

      generator.fStr_namespaces << "NamespaceBuilder nsb" + tempcounter.str() +
      " (\"" << obj.Name(SCOPED) << "\");\n";

      // !? ++generator.fMethodCounter;
   }

   for (Member_Iterator mi = obj.Member_Begin(); mi != obj.Member_End(); ++mi) {
      Member_GenerateDict(*mi, generator);    // call Members' own gendict
   }

   ScopeBase_GenerateDict(obj, generator);


} // GenerateDict

//-------------------------------------------------------------------------------
void
ScopeBase_GenerateDict(const Scope & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

  if (generator.Use_recursive()) {
      for (Reverse_Scope_Iterator subScopes = obj.SubScope_RBegin(); subScopes != obj.SubScope_REnd(); ++subScopes) {
//    for( Scope_Iterator subScopes = SubScope_Begin(); subScopes!= SubScope_End(); ++subScopes ) {

         const Scope & s = *subScopes;
         // std::cout << "SubScope " << s.Name() << std::endl;
         if (s.IsClass ())
         {
             const TypeBase * c = dynamic_cast <const TypeBase *> (s.ToScopeBase());
             // std::cout << "Class " << c->Name() << std::endl;
             Class_GenerateDict (*c, generator);
         }
         else if (s.IsEnum ())
         {
             // const ScopeBase * e = s.ToScopeBase();
             // std::cout << "Enum " << e->Name() << std::endl;
             #ifndef ONLY_FUNCTIONS
                Enum_GenerateDict (s, generator);
             #endif
         }
      }
  }
}

//-------------------------------------------------------------------------------
void
Member_GenerateDict(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
   // std::cout << "Member " << obj.Name () << std::endl;
   if (obj.IsDataMember ())
   {
      // MemberBase* d = obj.ToMemberBase();
      #ifndef ONLY_FUNCTIONS
         DataMember_GenerateDict (obj, generator);
      #endif
   }
   else if (obj.IsFunctionMember ()) {
      // MemberBase* f = obj.ToMemberBase();
      FunctionMember_GenerateDict (obj, generator);
   }
}

//-------------------------------------------------------------------------------
void
GlobalScope_GenerateDict(DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
   gFirstContext = NULL;
   gLastContext = NULL;

   const ScopeBase * b = Scope::GlobalScope().ToScopeBase();
   generator.AddIntoClasses("} // end of namespace"); // !? NO NAMESPACE - SHOULD BE CHANGED
   Namespace_GenerateDict (*b, generator);
   generator.AddIntoClasses("namespace {");
}