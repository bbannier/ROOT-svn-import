// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// Copyright CERN, CH-1211 Geneva 23, 2004-2006, All rights reserved.
//
// Permission to use, copy, modify, and distribute this software for any
// purpose is hereby granted without fee, provided that this copyright and
// permissions notice appear in all copies and derivatives.
//
// This software is provided "as is" without express or implied warranty.

#ifndef Reflex_FunctionMember
#define Reflex_FunctionMember

// Include files
#include "MemberBase.h"


namespace Reflex {

   // forward declarations
   class Type;
   class Object;
   class DictionaryGenerator;

namespace Internal {

   /**
   * @class FunctionMember FunctionMember.h Reflex/FunctionMember.h
   * @author Stefan Roiser
   * @date 24/11/2003
   * @ingroup Ref
   */
   class FunctionMember : public MemberBase {

   public:

      /** default constructor */
      FunctionMember( const char *   nam,
         const Type &   typ,
         StubFunction   stubFP,
         void *         stubCtx = 0,
         const char *   params = 0,
         unsigned int   modifiers = 0,
         TYPE           memType = FUNCTIONMEMBER );


      /** destructor */
      virtual ~FunctionMember() {}


      /**
      * GenerateDict will produce the dictionary information of this type
      * @param generator a reference to the dictionary generator instance
      */
      virtual void GenerateDict(DictionaryGenerator &generator) const;


      /** 
      * Name returns the Name of the member 
      * @param mod modifiers can be or'ed as argument
      * SCOPED - fully scoped name
      * @return name of the member
      */
      const std::string& Name( std::string& buf, unsigned int mod = SCOPED ) const;


      /** Invoke the function (if return At as void*) */
      /*Object Invoke( const Object & obj, 
      const std::vector < Object > & paramList ) const;*/
      Object Invoke( const Object & obj, 
         const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /** Invoke the function (for static functions) */
      //Object Invoke( const std::vector < Object > & paramList ) const;
      Object Invoke( const std::vector < void * > & paramList = 
         std::vector<void*>()) const;


      /** number of parameters */
      size_t FunctionParameterSize( bool required = false ) const;


      /** 
      * FunctionParameterDefaultss returns the collaction of default values 
      * (as strings) for function parameters.
      * @return collection of default values for parameters
      */
      const OrderedContainer<std::string>& FunctionParameterDefaults() const;


      /** 
      * FunctionParametertNames returns a collection of the parameter names
      * @return parameter names
      */
      const OrderedContainer<std::string>& FunctionParameterNames() const;


      /** return a pointer to the context */
      void * Stubcontext() const;


      /** return the pointer to the stub function */
      StubFunction Stubfunction() const;


      /** 
      * UpdateFunctionParameterNames updates the names of parameters
      * @param  parameters new list of ';' separated parameter names, must not specify default values
      */
      virtual void UpdateFunctionParameterNames(const char* parameters);

   private:

      /** pointer to the stub function */
      StubFunction fStubFP;


      /** user data for the stub function */
      void*  fStubCtx;


      /** FunctionParameterAt names */
      mutable
         std::vector < std::string > fParameterNames;


      /** FunctionParameterAt names */
      mutable
         std::vector < std::string > fParameterDefaults;


      /** number of required parameters */
      size_t fReqParameters;

   }; // class FunctionMember
} //namespace Internal
} //namespace Reflex


//-------------------------------------------------------------------------------
inline void *
Reflex::Internal::FunctionMember::Stubcontext() const {
//-------------------------------------------------------------------------------
   return fStubCtx;
}


//-------------------------------------------------------------------------------
inline Reflex::StubFunction 
Reflex::Internal::FunctionMember::Stubfunction() const {
//-------------------------------------------------------------------------------
   return fStubFP;
}

#endif // Reflex_FunctionMember
