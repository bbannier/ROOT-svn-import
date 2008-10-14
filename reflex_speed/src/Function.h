// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_Function
#define Reflex_Function

// Include files
#include "TypeBase.h"
#include "Reflex/Type.h"
#include "ContainerSTLAdaptor.h"

namespace Reflex {

   // forward declarations


namespace Internal {

   /** 
   * @class Function Function.h Reflex/Function.h
   * @author Stefan Roiser
   * @date 24/11/2003 
   * @ingroup Ref
   */
   class Function : public TypeBase {

   public:


      /** default constructor */
      Function(const Type & retType,
         const std::vector< Type > & parameters,
         const std::type_info & ti,
         const Catalog& catalog,
         ETYPE functionType = kETFunction);


      /** destructor */
      virtual ~Function() {}


      /**
      * Name will return the Name of the function
      * @param  mod modifiers to be applied when generating the Name
      * @return Name of function
      */
      const std::string& Name(std::string& buf, unsigned int mod = kScoped | kQualified) const;


      /**
      * FunctionParameters returns an ordered container of the type of the function parameters;
      * returns a reference to the dummy container if this type is not a function.
      * @return reflection information of nth function parameter
      */
      const IContainerImpl& FunctionParameters() const;


      /**
      * ReturnType will return a pointer to the At of the return At.
      * @return pointer to Type of return At
      */
      virtual Type ReturnType() const;


      /** static function that composes the At Name */
      static const std::string& BuildTypeName(std::string& buf,
         const Type & ret,
         const std::vector< Type > & param,
         unsigned int typemod = 0,
         unsigned int mod = kScoped | kQualified,
         const Scope & scope = Scope());

      /** static function that composes the At Name */
      static std::string BuildTypeName(const Type & ret,
         const std::vector< Type > & param,
         unsigned int typemod = 0,
         unsigned int mod = kScoped | kQualified,
         const Scope & scope = Scope()) {
            std::string buf;
            return BuildTypeName(buf, ret, param, typemod, mod, scope);
      }

      /** static function that composes the function name as a pointer type */
      static const std::string& BuildPointerTypeName(std::string& buf,
         const Type & ret,
         const std::string& name,
         const OrderedContainer< Type > & pars,
         unsigned int typemod = 0,
         unsigned int mod = kScoped | kQualified,
         const Scope & scope = Scope());

   private:

      /** 
      * container of parameter types 
      * @label function parameter types
      * @link aggregation
      * @clientCardinality 1
      * @supplierCardinality 0..*
      */
      mutable
         std::vector<Type> fParameters;

      /** 
      * IContainerImpl interface for container of parameter types
      */
      const ContainerSTLAdaptor< std::vector<Type> > fParametersAdaptor;

      /**
      * return type
      * @label return type
      * @link aggregation
      * @supplierCardinality 1
      * @clientCardinality 1
      */
      Type fReturnType;

   }; // class Function
} // namespace Internal
} // namespace Reflex


//-------------------------------------------------------------------------------
inline const Reflex::Internal::IContainerImpl&
Reflex::Internal::Function::FunctionParameters() const {
//-------------------------------------------------------------------------------
   return fParametersAdaptor;
}


//-------------------------------------------------------------------------------
inline Reflex::Type
Reflex::Internal::Function::ReturnType() const {
//-------------------------------------------------------------------------------
   return fReturnType;
}


#endif // Reflex_Function
