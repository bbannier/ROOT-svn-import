//------------------------------------------------------------------------------
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------
#ifndef __CLING__
#error "This file must not be included by compiled programs."
#endif
#ifdef CLING_DYNAMIC_LOOKUP_RUNTIME_UNIVERSE_H
#error "CLING_DYNAMIC_LOOKUP_RUNTIME_UNIVERSE_H Must only include once."
#endif

#define CLING_DYNAMIC_LOOKUP_RUNTIME_UNIVERSE_H

#include "cling/Interpreter/Interpreter.h"
#include "cling/Interpreter/ValuePrinter.h"
#include "cling/Interpreter/Value.h"

#include <stdio.h>
namespace cling {

  /// \brief Used to stores the declarations, which are going to be 
  /// available only at runtime. These are cling runtime builtins
  namespace runtime {

    /// \brief Provides builtins, which are neccessary for the dynamic scopes 
    /// and runtime bindings. These builtins should be used for other purposes.
    namespace internal {

      /// \brief Some of clang's routines rely on valid source locations and 
      /// source ranges. This member can be looked up and source locations and
      /// ranges can be passed in as parameters to these routines.
      ///
      /// Use instead of SourceLocation() and SourceRange(). This might help,
      /// when clang emits diagnostics on artificially inserted AST node.
      int InterpreterGeneratedCodeDiagnosticsMaybeIncorrect;

      /// \brief EvaluateT is used to replace all invalid source code that
      /// occurs, when cling's dynamic extensions are enabled.
      ///
      /// When the interpreter "sees" invalid code it mark it and skip all the 
      /// semantic checks (like the templates). Afterwards all these marked
      /// nodes are replaced with a call to EvaluateT, which makes valid
      /// C++ code. It is templated because it can be used in expressions and 
      /// T is the type when the evaluated expression.
      /// @tparam T The type of the evaluated expression
      /// @param[in] expr The expression that is being replaced
      /// @param[in] varaddr The addresses of the variables that replaced 
      /// expression contains
      /// @param[in] DC The declaration context, in which the expression will be
      /// evaluated at runtime
      template<typename T>
      T EvaluateT(DynamicExprInfo* ExprInfo, clang::DeclContext* DC ) {
        gCling->enableDynamicLookup();
        Value result(gCling->Evaluate(ExprInfo->getExpr(), DC));
        gCling->enableDynamicLookup(false);
        return result.getAs<T>();
      }

      /// \brief EvaluateT specialization for the case where we instantiate with
      /// void.
      template<>
      void EvaluateT(DynamicExprInfo* ExprInfo, clang::DeclContext* DC ) {
        gCling->enableDynamicLookup();
        Value result(gCling->Evaluate(ExprInfo->getExpr(), DC));
        gCling->enableDynamicLookup(false);
      }

      /// \brief LifetimeHandler is used in case of initialization using address
      /// on the automatic store (stack) instead of EvaluateT.
      ///
      /// The reason is to avoid the copy constructors that might be private.
      /// This is part of complex transformation, which aims to preserve the
      /// code behavior. For example:
      /// @code
      /// int i = 5;
      /// MyClass my(dep->Symbol(i))
      /// @endcode
      /// where dep->Symbol() is a symbol not known at compile-time
      /// transformed into:
      /// @code
      /// cling::runtime::internal::LifetimeHandler 
      /// __unique("dep->Sybmol(*(int*)@)",(void*[]){&i}, DC, "MyClass");
      /// MyClass &my(*(MyClass*)__unique.getMemory());
      /// @endcode
      class LifetimeHandler {
      private:
        /// \brief The memory on the free store, where the object will be 
        /// created.
        void* m_Memory;

        /// \brief The type of the object that will be created.
        std::string m_Type;
      public:
        /// \brief Constructs an expression, which creates the object on the
        /// free store and tells the interpreter to evaluate it.
        /// @param[in] expr The expression, which will be prepared for 
        /// evaluation
        /// @param[in] varadd The addresses of the variables that the expression
        /// contains
        /// @param[in] DC The declaration context, in which the expression will 
        /// be evaluated at runtime
        /// @param[in] type The type of the object, which will help to delete
        /// it, when the LifetimeHandler goes out of scope.
        LifetimeHandler(DynamicExprInfo* ExprInfo,
                        clang::DeclContext* DC,
                        llvm::StringRef type) {
          m_Type = type;
          std::string ctor("new ");
          ctor += type;
          ctor += "(\"";
          ctor += ExprInfo->getExpr();
          ctor += "\")";
          Value res = gCling->Evaluate(ctor.c_str(), DC);
          m_Memory = (void*)res.value.PointerVal;
        }

        /// \brief Clears up the free store, when LifetimeHandler goes out of
        /// scope.
        ~LifetimeHandler() {
          std::string str;
          llvm::raw_string_ostream stream(str);
          stream<<"delete ("<< m_Type << "*) "<< m_Memory;
          stream.flush();
          gCling->processLine(str);
        }
        void* getMemory() const { return m_Memory; }
      };
    }
  } // end namespace runtime
} // end namespace cling
