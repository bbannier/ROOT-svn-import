//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------
#ifndef __CLING__
#error "This file must not be included by compiled programs."
#endif
#ifdef CLING_RUNTIME_UNIVERSE_H
#error "CLING_RUNTIME_UNIVERSE_H Must only include once."
#endif

#define CLING_RUNTIME_UNIVERSE_H

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS // needed by System/DataTypes.h
#endif

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS // needed by System/DataTypes.h
#endif

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/ValuePrinter.h"
#include "cling/Interpreter/Value.h"

#include <stdio.h>

namespace cling {
  namespace runtime {
    // Stores the declarations of variables, which are going to be available
    // only during runtime
    Interpreter* gCling = 0;
    namespace internal {
      // Annotates that these variables shouldn't be used unless you know
      // what you're doing
      
      // Uncomment next line when figure out how to remove explicit template
      // specialization
      //template<typename T> class TypeExtractionHelper{};
      
      template<typename T>
      T EvaluateProxyT(const char* expr, void* varaddr[], clang::DeclContext* DC ) {
        Value result(gCling->EvaluateWithContext(expr, varaddr, DC));
        return result.getAs<T>();
      }

      class LifetimeHandler {
      private:
        void* m_Memory;
        std::string m_Type;
      public:
        // Temp constructor just to build what we need and then
        // we will give the concrete parameters
        LifetimeHandler(){}
        LifetimeHandler(llvm::StringRef expr,
                        void* varaddr[],
                        clang::DeclContext* DC,
                        llvm::StringRef type) {
          m_Type = type;
          std::string ctor("new ");
          ctor += type;
          ctor += "(";
          ctor += expr;
          ctor += ")";
          Value res = gCling->EvaluateWithContext(ctor.c_str(), varaddr, DC);
          //m_memory = (void*)res.value.Untyped;
        }
        ~LifetimeHandler() {
          std::string str;
          llvm::raw_string_ostream stream(str);
          stream<<"delete ("<< m_Type << "*) "<< m_Memory;
          gCling->processLine(str);
        }
        void* getMemory() const { return m_Memory; }
      };
    }
  } // end namespace runtime
} // end namespace cling

using namespace cling::runtime;

