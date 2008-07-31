// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef REFLEX_BUILD
#define REFLEX_BUILD
#endif

#include "FunctionMember.h"

#include "Reflex/Scope.h"
#include "Reflex/Object.h"
#include "OwnedMember.h"
#include "Reflex/DictionaryGenerator.h"

#include "Function.h"
#include "Reflex/Tools.h"
#include "Reflex/EntityProperty.h"


//-------------------------------------------------------------------------------
Reflex::Internal::FunctionMember::FunctionMember(const char *  nam,
                                        const Type &  typ,
                                        StubFunction  stubFP,
                                        void*         stubCtx,
                                        const char *  parameters,
                                        unsigned int  modifiers,
                                        ETYPE          memType)
//-------------------------------------------------------------------------------
   : MemberBase(nam, typ, memType, modifiers),
     fStubFP(stubFP), 
     fStubCtx(stubCtx),
     fParameterNames(std::vector<std::string>()),
     fParameterDefaults(std::vector<std::string>()),
     fReqParameters(0)
{
// Contruct a FunctionMember (see FunctionBuilder)
   UpdateFunctionParameterNames(parameters);
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::FunctionMember::Name(unsigned int mod) const {
//-------------------------------------------------------------------------------
// Construct the qualified (if requested) name of the function member.
   std::string s = "";

   if (mod & kQualified) {
      if (Is(gPUBLIC))          { s += "public ";    }
      if (Is(gPROTECTED))       { s += "protected "; }
      if (Is(gPRIVATE))         { s += "private ";   }  
      if (Is(gEXTERN))          { s += "extern ";    }
      if (Is(gSTATIC))          { s += "static ";    }
      if (Is(gINLINE))          { s += "inline ";    }
      if (Is(gVirtual))         { s += "virtual ";   }
      if (Is(gEXPLICIT))        { s += "explicit ";  }
   }

   s += MemberBase::Name(mod); 

   return s;
}


/*/-------------------------------------------------------------------------------
  Reflex::Object
 
Reflex::Internal::FunctionMember::Invoke(const Object & obj,
  const std::vector < Object > & paramList) const {
//-----------------------------------------------------------------------------
  if (paramList.size() < FunctionParameterSize(true)) {
  throw RuntimeError("Not enough parameters given to function ");
  return Object();
  }
  void * mem = CalculateBaseObject(obj);
  std::vector < void * > paramValues;
  // needs more checking FIXME
  for (std::vector<Object>::const_iterator it = paramList.begin();
  it != paramList.end(); ++it) paramValues.push_back(it->Address());
  return Object(TypeOf().ReturnType(), fStubFP(mem, paramValues, fStubCtx));
  }
*/


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::FunctionMember::Invoke(const Object & obj,
                                      const std::vector < void * > & paramList) const {
//-----------------------------------------------------------------------------
// Invoke this function member with object obj. 
   if (paramList.size() < FunctionParameterSize(true)) {
      throw RuntimeError("Not enough parameters given to function ");
      return Object();
   }
   void * mem = CalculateBaseObject(obj);
   // parameters need more checking FIXME
   return Object(TypeOf().ReturnType(), fStubFP(mem, paramList, fStubCtx));
}


/*/-------------------------------------------------------------------------------
  Reflex::Object
 
Reflex::Internal::FunctionMember::Invoke(const std::vector < Object > & paramList) const {
//-------------------------------------------------------------------------------
  std::vector < void * > paramValues;
  // needs more checking FIXME
  for (std::vector<Object>::const_iterator it = paramList.begin();
  it != paramList.end(); ++it) paramValues.push_back(it->Address());
  return Object(TypeOf().ReturnType(), fStubFP(0, paramValues, fStubCtx));
  }
*/


//-------------------------------------------------------------------------------
Reflex::Object
Reflex::Internal::FunctionMember::Invoke(const std::vector < void * > & paramList) const {
//-------------------------------------------------------------------------------
// Call static function 
   // parameters need more checking FIXME
   return Object(TypeOf().ReturnType(), fStubFP(0, paramList, fStubCtx));
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::FunctionMember::FunctionParameterSize(bool required) const {
//-------------------------------------------------------------------------------
// Return number of function parameters. If required = true return number without default params.
   if (required) return fReqParameters;
   else            return TypeOf().FunctionParameterSize();
}



//-------------------------------------------------------------------------------
void
Reflex::Internal::FunctionMember::GenerateDict(DictionaryGenerator & generator) const {
//-------------------------------------------------------------------------------
// Generate Dictionary information about itself.   

   std::string mName = Name();

   if (mName != "__getNewDelFunctions"  && mName != "__getBasesTable") {

      // The return type
      const Type & retT = TypeOf().ReturnType();

      // Get function return type, register that to "used types"  
      std::string returntype = generator.GetTypeNumber(TypeOf().ReturnType());
    
      // Prevents __getNewDelFunctions getting into AddFunctionMember twice
      //if (generator.IsNewType(TypeOf()) && Name()!="__getNewDelFunctions") {
    
      if (Is(gPRIVATE)) {
         generator.AddIntoShadow (Name(kScoped) + "();\n");
      }
              
      // Get a number for the function type
      //std::string number = generator.GetTypeNumber(TypeOf());
          
      std::stringstream temp;
      temp<< generator.fMethodCounter;
      std::string number = temp.str();
   
      ++generator.fMethodCounter;
      
      // Get current Namespace location
      std::string namespc =  DeclaringScope().Name(kScoped);
        
      std::stringstream tempcounter;
      tempcounter<<generator.fMethodCounter;
            
      // Free function, shall be added into Instances-field only
      if (DeclaringScope().Is(gNAMESPACE)) {
         generator.AddIntoInstances("      Type t" + tempcounter.str() + " = FunctionTypeBuilder(type_" + returntype );
        
      } else { // "normal" function, inside a class
         
         generator.AddIntoFree(".AddFunctionMember(FunctionTypeBuilder(type_" + returntype );
      }
            
      
      // Get the parameters for function
      for (Type_Iterator params = TypeOf().FunctionParameter_Begin();
           params != TypeOf().FunctionParameter_End(); ++params) {
         
         
         if (DeclaringScope().Is(gNAMESPACE)) {
            generator.AddIntoInstances(", type_" + generator.GetTypeNumber((*params)) );
               
         } else {
            generator.AddIntoFree(", type_" + generator.GetTypeNumber((*params)) );
         }
         
      }


      
      if (DeclaringScope().Is(gNAMESPACE)) {
         generator.AddIntoInstances(");  FunctionBuilder(t" + tempcounter.str() + ", \"" 
                                    + Name() + "\", function_" + number); //function name
      }
   
                   
      else {  // normal function
           
         generator.AddIntoFree("), \"" + Name() + "\""); //function name
      }

      if      (Is(gConstructor)) generator.AddIntoFree(", constructor_");
      else if (Is(gDestructor) ) generator.AddIntoFree(", destructor_");

      
           
      if (Is(gConstructor)) {
         generator.AddIntoClasses("static void* constructor_");
              
      } else if (Is(gDestructor)) {
         generator.AddIntoClasses("static void* destructor_");
              
      } else {
              
         if (! (DeclaringScope().Is(gNAMESPACE))) {
            generator.AddIntoFree(", method_");
                   
            generator.AddIntoClasses("\nstatic void* method_");

         }
              
         else {
            // free function
            generator.AddIntoClasses("\nstatic void* function_");
         }
      }
           
      if (!(DeclaringScope().Is(gNAMESPACE))) generator.AddIntoFree(number); //method_n
      
            
      generator.AddIntoClasses(number);
    
      
      if (Is(gConstructor)) {  // these have parameters

         generator.AddIntoClasses("(void* mem, const std::vector<void*>&");  
         
         if (FunctionParameterSize()) generator.AddIntoClasses(" arg");
         
         generator.AddIntoClasses(", void*)\n{");
         generator.AddIntoClasses("\n  return ::new(mem) " + namespc);
         generator.AddIntoClasses("(");
       
      }//is constructor/destructor with parameters

      else if (Is(gDestructor)) {
         generator.AddIntoClasses("(void * o, const std::vector<void*>&, void *) {\n");
         generator.AddIntoClasses("  ((" + namespc + "*)o)->~" + DeclaringScope().Name() + "(");
      }
        
      else {
         // method function with parameters
       
         if (DeclaringScope().Is(gNAMESPACE)) {
            generator.AddIntoClasses(" (void*, const std::vector<void*>&");// arg, void*)\n{");
                
         } else {
            generator.AddIntoClasses(" (void* o, const std::vector<void*>&");// arg, void*)\n{");
         }

         if (FunctionParameterSize()>0) generator.AddIntoClasses(" arg");

         generator.AddIntoClasses(", void*)\n{");

         if (retT.Name() != "void") {

            if (retT.Is(gFUNDAMENTAL)) {
               generator.AddIntoClasses("static " + retT.Name(kScoped) + " ret;\n");
               generator.AddIntoClasses("ret = ");
            }
            else if (retT.Is(gReference) || retT.Is(gPointer)) {
               generator.AddIntoClasses("return (void*)");
               if (retT.Is(gReference)) generator.AddIntoClasses("&");
            }
            else { // compound type
               generator.AddIntoClasses("return new " + retT.Name(kScoped) + "(");
            }
         }
         
         if (DeclaringScope().Is(gNAMESPACE)) {
            generator.AddIntoClasses(Name() + "("); 
                
         } else {
            generator.AddIntoClasses("((" + namespc + "*)o)->"+ Name() + "(");   
         }

      }

      
      // Add to Stub Functions with some parameters
      if (FunctionParameterSize()>0) {
       
         unsigned args = 0;
       
         // Get all parameters
         for (Type_Iterator methpara = TypeOf().FunctionParameter_Begin();
              methpara != TypeOf().FunctionParameter_End(); ++methpara) {

            // get params for the function, can include pointers or references
            std::string param = generator.GetParams(*methpara);
                  
            std::stringstream temp2;
            temp2<<args;
                           
            // We de-deference parameter only, if it's not a pointer
            if (! methpara->Is(gPointer)) generator.AddIntoClasses("*");
          
            generator.AddIntoClasses("(");
                    
            //if (methpara->IsConst()) generator.AddIntoClasses("const ");
            
            std::string paraT = methpara->Name(kScoped|kQualified);
            if (methpara->Is(gReference)) paraT = paraT.substr(0,paraT.length()-1);

            generator.AddIntoClasses(paraT);

            if (! methpara->Is(gPointer)) generator.AddIntoClasses("*");

            generator.AddIntoClasses(") arg[" + temp2.str() + "]");
                        
            //still parameters left
            if ((args+1) < FunctionParameterSize())  generator.AddIntoClasses(", ");
                  
            ++args;
            //fundam
                  
         }
             
      } // funct. params!=0
                 
                  
      if (Is(gConstructor)) {
         generator.AddIntoClasses(");\n} \n");
    
      } else {

         generator.AddIntoClasses(")");

         if (retT.Name() == "void") {
            generator.AddIntoClasses(";\n  return 0;\n");
         }
         else if (retT.Is(gFUNDAMENTAL)) {
            generator.AddIntoClasses(";\n  return & ret;\n");
         }
         else if (retT.Is(gPointer) || retT.Is(gReference)) {
            generator.AddIntoClasses(";\n");
         }
         else { // compound type
            generator.AddIntoClasses(");\n");
         }

         generator.AddIntoClasses("\n} \n"); //);
      }    
      
   
      
        
      if (DeclaringScope().Is(gNAMESPACE)) {
         generator.AddIntoInstances(", 0");
      }
      else {
         generator.AddIntoFree(", 0");
      }
      
        
      if (DeclaringScope().Is(gNAMESPACE)) generator.AddIntoInstances(", \"");
      else                                 generator.AddIntoFree(", \"");     
           
         
      // Get the names of the function param.types (like MyInt)
      if (TypeOf().FunctionParameterSize()) {
         
         unsigned dot = 0;
         Type_Iterator params;
         StdString_Iterator parnam;

         

         for (params = TypeOf().FunctionParameter_Begin(), parnam = FunctionParameterName_Begin();
              params != TypeOf().FunctionParameter_End(), parnam != FunctionParameterName_End(); 
              ++params, ++parnam) {
            
            // THESE SHOULD ALSO INCLUDE DEFAULT VALUES,
            // LIKE int i=5 FunctionParameterDefault_Begin(), _End
            //
            
            if (DeclaringScope().Is(gNAMESPACE)) {
               generator.AddIntoInstances((*parnam));
               if ((dot+1) < FunctionParameterSize()) {
                  generator.AddIntoInstances(";");
               }
            }
            else {
               generator.AddIntoFree((*parnam));
               if ((dot+1) < FunctionParameterSize()) {
                  generator.AddIntoFree(";");
               }
            }
            ++dot;
         }

      }
    
      if (DeclaringScope().Is(gNAMESPACE)) generator.AddIntoInstances("\"");
      else                                 generator.AddIntoFree("\"");
   
    
      if (DeclaringScope().Is(gNAMESPACE))  { // free func
         generator.AddIntoInstances(", ");
           
         if (Is(gPUBLIC))         generator.AddIntoInstances("kPublic");
         else if (Is(gPRIVATE))   generator.AddIntoInstances("kPrivate");
         else if (Is(gPROTECTED)) generator.AddIntoInstances("kProtected");
           
         if (Is(gARTIFICIAL))  generator.AddIntoInstances(" | kArtificial");
           
         generator.AddIntoInstances(");\n");
         
         ++generator.fMethodCounter;
           
      } else {
         generator.AddIntoFree(", ");
           
         if (Is(gPUBLIC))         generator.AddIntoFree("kPublic");
         else if (Is(gPRIVATE))   generator.AddIntoFree("kPrivate");
         else if (Is(gPROTECTED)) generator.AddIntoFree("kProtected");
           
         if (Is(gVirtual))    generator.AddIntoFree(" | kVirtual");
         if (Is(gARTIFICIAL))  generator.AddIntoFree(" | kArtificial");
         if (Is(gConstructor)) generator.AddIntoFree(" | kConstructor");
         if (Is(gDestructor))  generator.AddIntoFree(" | kDestructor");
           
         generator.AddIntoFree(")\n");
      }
   }
}

//-------------------------------------------------------------------------------
void
Reflex::Internal::FunctionMember::UpdateFunctionParameterNames(const char* parameters)
//-------------------------------------------------------------------------------
{
   // Obtain the names and default values of the function parameters
   // The "real" number of parameters is obtained from the function type

   fParameterNames.clear();
   bool hadDefaultValues = !fParameterDefaults.empty();
   size_t numDefaultParams = 0;
   size_t type_npar = MemberBase::TypeOf().FunctionParameterSize();
   std::vector<std::string> params;
   if (parameters) Tools::StringSplit(params, parameters, ";");
   size_t npar = std::min(type_npar,params.size());
   for (size_t i = 0; i < npar ; ++i) {
      size_t pos = params[i].find("=");
      fParameterNames.push_back(params[i].substr(0,pos));
      if (pos != std::string::npos) {
         if (hadDefaultValues)
            throw RuntimeError("Attempt to redefine default values of parameters!");
         else {
            fParameterDefaults.push_back(params[i].substr(pos+1));
            ++numDefaultParams;
         }
      }
      else if (!hadDefaultValues){
         fParameterDefaults.push_back("");
      }
   }
   // padding with blanks
   for (size_t i = npar; i < type_npar; ++i) {
      fParameterNames.push_back("");
      if (!hadDefaultValues)
         fParameterDefaults.push_back("");
   }

   if (!hadDefaultValues)
      fReqParameters = type_npar - numDefaultParams;
}

