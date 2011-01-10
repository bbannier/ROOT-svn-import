// @(#)root/cint:$Id$

/* genrflxdict.cxx */

/* based on Reflex source code from 27-05-2010 */

#include "Reflex/Reflex.h"
#include "Reflex/DictionaryGenerator.h"
#include "Reflex/internal/ScopeBase.h"

using namespace Reflex;

#define ONLY_FUNCTIONS
#define NUMBERED_FUNCTIONS
#define REGISTER_FUNCTIONS

// output similar to genreflex
// #define EQUAL_OUTPUT

#include "clr-scan.h"

#ifdef REGISTER_FUNCTIONS
   #include "clr-reg.h"
#endif

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

void FunctionMember_Head(const Member & obj, DictionaryGenerator& generator);
void FunctionMember_Body(const Member & obj, DictionaryGenerator& generator, bool withRet);
void FunctionMember_Tail(const Member & obj, DictionaryGenerator& generator);

//-------------------------------------------------------------------------------
void
FunctionMember_GenerateDict(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.

   std::string mName = obj.Name();

   if (mName != "__getNewDelFunctions" && mName != "__getBasesTable") {

      FunctionMember_Head(obj, generator);
      generator.AddIntoClasses("{\n");

      #ifdef EQUAL_OUTPUT
         generator.AddIntoClasses("  ");
      #else
         generator.AddIntoClasses("   ");
      #endif

      const Type& retT = obj.TypeOf().ReturnType();
      if (obj.IsConstructor() || retT.Name() != "void")
      {
         FunctionMember_Body(obj, generator, true);
         #ifdef EQUAL_OUTPUT
            generator.AddIntoClasses("else   ");
         #else
            generator.AddIntoClasses("   else ");
         #endif
      }

      FunctionMember_Body(obj, generator, false);

      generator.AddIntoClasses("}\n");
      FunctionMember_Tail(obj, generator);
   }
} // GenerateDict

// see: cint/reflex/python/genreflex/gendict.py, function genMCODef
// see: cint/cint/src/rflx_gensrc.cxx, function gen_classdictdecls

//-------------------------------------------------------------------------------
void
FunctionMember_Head(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
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
      #endif // ONLY_FUNCTIONS

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
      #endif // ONLY_FUNCTIONS

      // STUB HEADER
      if (obj.IsConstructor()) {
         // CORRECT_FUNCTIONS
         #ifdef NUMBERED_FUNCTIONS
         generator.AddIntoClasses("void __stub_");
         #else
         generator.AddIntoClasses("static void constructor_");
         #endif

      } else if (obj.IsDestructor()) {
         // CORRECT_FUNCTIONS
         #ifdef NUMBERED_FUNCTIONS
         generator.AddIntoClasses("void __stub_");
         #else
         generator.AddIntoClasses("static void destructor_");
         #endif

      } else {
         if (!(obj.DeclaringScope().IsNamespace())) {
            #ifndef ONLY_FUNCTIONS
            generator.AddIntoFree(", method_");
            #endif // ONLY_FUNCTIONS

            // CORRECT_FUNCTIONS
            #ifdef NUMBERED_FUNCTIONS
            generator.AddIntoClasses("void __stub_");
            #else
            generator.AddIntoClasses("static void method_");
            #endif

         } else {
            // free function
            // CORRECT_FUNCTIONS
            #ifdef NUMBERED_FUNCTIONS
            generator.AddIntoClasses("void __stub_");
            #else
            generator.AddIntoClasses("static void function_");
            #endif
         }
      }

      #ifdef NUMBERED_FUNCTIONS
         // info ("Registering stub function " + IntToStd (method_index));

         // check if stub function is our function -- only for verification
         assert (obj.Stubfunction() == ClrStubFunction);

         // typecast stub context into our type
         TClrContext * context = reinterpret_cast <TClrContext *> (obj.Stubcontext ());
         assert (context != NULL);

         // store function number into context
         context->index = method_index;

         // add context to linked list of all contexts
         gClrReg->LinkContext (context);
      #endif

      #ifndef ONLY_FUNCTIONS
      if (!(obj.DeclaringScope().IsNamespace())) {
         generator.AddIntoFree(number);                                     //method_n
      }
      #endif // ONLY_FUNCTIONS
      generator.AddIntoClasses(number);

      #ifdef REGISTER_FUNCTIONS
         std::string funcName = TScanner::GetFuncProp(obj.Properties());

         // generate code
         generator.AddIntoInstances("      wrappers[");

         generator.AddIntoInstances("\"");
         generator.AddIntoInstances(funcName);
         // generator.AddIntoInstances(G__map_cpp_name (funcName.c_str ()));
         generator.AddIntoInstances("\"");

         generator.AddIntoInstances("] = ");

         #ifdef NUMBERED_FUNCTIONS
            generator.AddIntoInstances("__stub_");
         #else
            if (obj.IsConstructor()) {
               generator.AddIntoInstances("constructor_");
            } else if (obj.IsDestructor()) {
               generator.AddIntoInstances("destructor_");
            } else {
               generator.AddIntoInstances("method_");
            }
         #endif
         generator.AddIntoInstances(number);                                     //method_n
         generator.AddIntoInstances(";\n");
      #endif

      if (obj.IsConstructor()) {   // these have parameters
         // CORRECT_FUNCTIONS
         generator.AddIntoClasses("(void* retaddr, void* mem, const std::vector<void*>&");
         if (obj.FunctionParameterSize()) {
            generator.AddIntoClasses(" arg");
         }
         generator.AddIntoClasses(", void*)\n");

      } //is constructor/destructor with parameters
      else if (obj.IsDestructor()) {
         // CORRECT_FUNCTIONS
         generator.AddIntoClasses("(void*");
         if (retT.Name() != "void")
            generator.AddIntoClasses(" retaddr");
         generator.AddIntoClasses(", void * o, const std::vector<void*>&, void *)\n");
      } else {
         // method function with parameters

         if (obj.DeclaringScope().IsNamespace()) {
            // CORRECT_FUNCTIONS
            generator.AddIntoClasses(" (void*");
            if (retT.Name() != "void")
               generator.AddIntoClasses(" retaddr");
            generator.AddIntoClasses(", void*, const std::vector<void*>&"); // arg, void*)\n{");
         } else {
            // CORRECT_FUNCTIONS
            generator.AddIntoClasses(" (void*");
            if (retT.Name() != "void")
               generator.AddIntoClasses(" retaddr");
            generator.AddIntoClasses(", void* o, const std::vector<void*>&"); // arg, void*)\n{");
         }

         if (obj.FunctionParameterSize() > 0) {
            generator.AddIntoClasses(" arg");
         }

         generator.AddIntoClasses(", void*)\n");
      }


} // FunctionMember_Head

//-------------------------------------------------------------------------------
void
FunctionMember_Body(const Member & obj, DictionaryGenerator& generator, bool withRet) {
//-------------------------------------------------------------------------------
      const Type& retT = obj.TypeOf().ReturnType();

      std::string namespc = obj.DeclaringScope().Name(SCOPED);
      namespc = "::" + namespc;

      if (obj.IsConstructor()) {
         if (withRet)
         {
            generator.AddIntoClasses("if (retaddr) *(void**)");
            #ifndef EQUAL_OUTPUT
               generator.AddIntoClasses(" ");
            #endif
            generator.AddIntoClasses("retaddr = ");
         }
         generator.AddIntoClasses("::new(mem) " + namespc);
         generator.AddIntoClasses("(");
      }
      else if (obj.IsDestructor()) {
         #ifdef EQUAL_OUTPUT
            generator.AddIntoClasses(" ");
            generator.AddIntoClasses("(");
         #endif
         generator.AddIntoClasses("((" + namespc + "*)o)->");
         #ifdef EQUAL_OUTPUT
            generator.AddIntoClasses("::" + obj.DeclaringScope().Name() + "::");
         #endif
         generator.AddIntoClasses("~" + obj.DeclaringScope().Name());
         #ifdef EQUAL_OUTPUT
            generator.AddIntoClasses(")");
         #endif
         generator.AddIntoClasses("(");
      } else {
         // method function

         if (retT.Name() != "void") {
            #if 0
            if (retT.IsFundamental()) {
               generator.AddIntoClasses("static " + retT.Name(SCOPED) + " ret;\n");
               generator.AddIntoClasses("ret = ");
            } else
            #endif
              if (retT.IsReference() || retT.IsPointer()) {
               // CORRECT_FUNCTIONS
               if (withRet)
               {
                  generator.AddIntoClasses("if (retaddr) *(void**)");
                  #ifndef EQUAL_OUTPUT
                     generator.AddIntoClasses(" ");
                  #endif
                  generator.AddIntoClasses("retaddr = (void*)");
                  #ifdef EQUAL_OUTPUT
                     generator.AddIntoClasses(" ");
                  #endif
                  if (retT.IsReference()) {
                     generator.AddIntoClasses("&");
                  }
               }

            } else { // compound type
               // CORRECT_FUNCTIONS
               if (withRet)
               {
                  generator.AddIntoClasses("if (retaddr) new (retaddr) (");
                  #ifndef EQUAL_OUTPUT
                     if (retT.IsClass())
                        generator.AddIntoClasses("::");
                  #endif
                  generator.AddIntoClasses(retT.Name(SCOPED));
                  generator.AddIntoClasses(")");
                  #ifndef EQUAL_OUTPUT
                     generator.AddIntoClasses(" ");
                  #endif
                  generator.AddIntoClasses("(");
               }
            }
         }

         if (obj.DeclaringScope().IsNamespace()) {
            generator.AddIntoClasses(obj.Name());
         } else {
            #ifdef EQUAL_OUTPUT
               generator.AddIntoClasses("(");
            #endif
            generator.AddIntoClasses("((");
            #ifdef EQUAL_OUTPUT
               if (obj.IsConst ())
                  generator.AddIntoClasses("const "); // not necessary
            #endif
            generator.AddIntoClasses(namespc + "*)o)->" + obj.Name());
            #ifdef EQUAL_OUTPUT
               generator.AddIntoClasses(")");
            #endif
         }
         #ifndef EQUAL_OUTPUT
            generator.AddIntoClasses(" ");
         #endif
         generator.AddIntoClasses("(");
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

            if (methpara->IsClass() /* || methpara->IsPointer() && methpara->ReturnType().IsClass() */ )
               generator.AddIntoClasses("::");
            generator.AddIntoClasses(paraT);

            if (!methpara->IsPointer()) {
               generator.AddIntoClasses("*");
            }

            generator.AddIntoClasses(")arg[" + temp2.str() + "]");

            //still parameters left
            if ((args + 1) < obj.FunctionParameterSize()) {
               #ifdef EQUAL_OUTPUT
                  generator.AddIntoClasses(",\n    ");
               #else
                  generator.AddIntoClasses(", ");
               #endif
         }

            ++args;
            //fundam

         }

      } // funct. params!=0

      if (obj.IsConstructor()) {
         generator.AddIntoClasses(")");
      } else if (obj.IsDestructor()) {
         generator.AddIntoClasses(")");
      } else {
         if (retT.IsReference()) {
            // reference must be tested first
            generator.AddIntoClasses(")");
         } else if (retT.Name() == "void") {
            // CORRECT_FUNCTIONS
            generator.AddIntoClasses(")");
         } else if (retT.IsFundamental()) {
            // CORRECT_FUNCTIONS
            generator.AddIntoClasses(")");
            if (withRet)
               generator.AddIntoClasses(")");
         } else if (retT.IsPointer()) {
            generator.AddIntoClasses(")");
            #ifdef EQUAL_OUTPUT
            // if (withRet)
            //    generator.AddIntoClasses(")");
            #endif
         } else { // compound type
            generator.AddIntoClasses(")");
            if (withRet)
               generator.AddIntoClasses(")");
         }
      }

      generator.AddIntoClasses(";\n");
} // FunctionMember_Body

//-------------------------------------------------------------------------------
void
FunctionMember_Tail(const Member & obj, DictionaryGenerator& generator) {
//-------------------------------------------------------------------------------
      #ifndef ONLY_FUNCTIONS
      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances(", 0");
      } else {
         generator.AddIntoFree(", 0");
      }
      #endif // ONLY_FUNCTIONS

      #ifndef ONLY_FUNCTIONS
      if (obj.DeclaringScope().IsNamespace()) {
         generator.AddIntoInstances(", \"");
      } else {
         generator.AddIntoFree(", \"");
      }

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
      } else {
         generator.AddIntoFree("\"");
      }

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
      #endif // ONLY_FUNCTIONS
} // FunctionMember_Tail

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
      #endif // ONLY_FUNCTIONS

      generator.AddIntoClasses("\n// -- Stub functions for class " + obj.ThisType().Name() + "--\n");
      generator.AddIntoClasses("\n");

      Scope scope = obj;
      for (Member_Iterator mi = scope.Member_Begin();
           mi != scope.Member_End(); ++mi) {
         Member_GenerateDict(*mi, generator);      // call Members' own gendict
         generator.AddIntoClasses("\n");
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
      #endif // ONLY_FUNCTIONS

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
             #endif // ONLY_FUNCTIONS
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
      #endif // ONLY_FUNCTIONS
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
   gClrReg->ResetContexts();
   const ScopeBase * b = Scope::GlobalScope().ToScopeBase();
   generator.AddIntoClasses("} // end of namespace"); // !? NO NAMESPACE - SHOULD BE CHANGED
   #ifdef REGISTER_FUNCTIONS
   generator.AddIntoInstances("      std::map<std::string, Reflex::StubFunction> wrappers;\n");
   #endif
   Namespace_GenerateDict (*b, generator);
   #ifdef REGISTER_FUNCTIONS
   generator.AddIntoClasses("void gAddWrapperMap (std::map<std::string, Reflex::StubFunction> & wrappers);\n"); // !?
   generator.AddIntoInstances("      gAddWrapperMap(wrappers);\n");
   #endif
   generator.AddIntoClasses("namespace {");
}
