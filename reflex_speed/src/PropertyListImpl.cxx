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

#include "PropertyListImpl.h"
#include "Reflex/Any.h"
#include "Reflex/Tools.h"

#include <sstream>
#include <utility>

// SOLARIS CC FIX (this include file is needed for a fix for std::distance)
#include "stl_hash.h"
#include "ContainerImpl.h"
#include "ContainerSTLAdaptor.h"

/** the Key container */
typedef std::vector< std::string > Keys_t;

//-------------------------------------------------------------------------------
Keys_t& sKeys() {
//-------------------------------------------------------------------------------
   // Wrapper for static keys container.
   static Keys_t k;
   return k;
}


//-------------------------------------------------------------------------------
// specializations for map of string -> int:
template <>
class Reflex::Internal::ContainerTraits_KeyExtractor<std::string, size_t> {
public:
   const std::string& Get(size_t v) const { return sKeys()[v]; }
   const std::string& Get(size_t v, std::string&) const { return sKeys()[v]; }
};

template <>
struct Reflex::Internal::NodeValidator<size_t> {
   static void Invalidate(size_t& value) { value = (size_t)-1; }
   static bool IsValid(size_t value) { return value != (size_t)-1; }
};

typedef Reflex::Internal::ContainerImpl<std::string, size_t, Reflex::Internal::kUnique,
                                        Reflex::Internal::ContainerTools::NotRefCounted> KeyCont_t;

//-------------------------------------------------------------------------------
KeyCont_t& sKeyCont() {
//-------------------------------------------------------------------------------
// Wrapper for static key name -> vector position
   static KeyCont_t k;
   return k;
}

//-------------------------------------------------------------------------------
std::ostream & Reflex::Internal::operator<<(std::ostream & s,
                                             const PropertyListImpl & p) {
//-------------------------------------------------------------------------------
// Operator to put properties on the ostream.
   if (p.fProperties) {
      for (size_t i = 0; i < p.fProperties->size(); ++i) {
         Any & a = p.PropertyValue(i);
         if (a) s << sKeys()[i] << " : " << a << std::endl;
      }
   }
   return s;
}


//-------------------------------------------------------------------------------
Reflex::Internal::PropertyListImpl::~PropertyListImpl() {
//-------------------------------------------------------------------------------
// Destruct, deleting our fProperties.
   delete fProperties;
}


//-------------------------------------------------------------------------------
void
Reflex::Internal::PropertyListImpl::ClearProperties() {
//-------------------------------------------------------------------------------
// Clear, remove all properties.
   if (fProperties) delete fProperties;
   fProperties = 0;
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::PropertyListImpl::HasProperty(const std::string & key) const {
//-------------------------------------------------------------------------------
   // Return true if property has key.
   KeyCont_t::const_iterator i = sKeyCont().Find(key);
   if (!i) return false;
   else    return PropertyValue(*i);
}


//-------------------------------------------------------------------------------
bool
Reflex::Internal::PropertyListImpl::HasProperty(size_t key) const {
//-------------------------------------------------------------------------------
   // Return true if property has key.
   return PropertyValue(key);
}


//-------------------------------------------------------------------------------
const Reflex::Internal::IContainerImpl&
Reflex::Internal::PropertyListImpl::Keys() {
//-------------------------------------------------------------------------------
   // Return begin iterator of key container
   static Reflex::Internal::ContainerSTLAdaptor<Keys_t> k(sKeys());
   return k;
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::PropertyListImpl::KeysAsString() {
//-------------------------------------------------------------------------------
// Return a string containing all property keys.
   std::string buf;
   return Tools::StringVec2String(buf, sKeys());
}


//-------------------------------------------------------------------------------
const std::string&
Reflex::Internal::PropertyListImpl::KeyAt(size_t nth) {
//-------------------------------------------------------------------------------
// Return a string containing all property keys.
   return sKeys()[nth];
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::PropertyListImpl::KeyByName(const std::string & key,
                                              bool allocateNew /*= false*/) {
//-------------------------------------------------------------------------------
// Key is the static getter function to return the index of a key. If allocateNew is 
// set to true a new key will be allocated if it doesn't exist and it's index returned.
// Otherwise if the key exists the function returns it's index or 0 if no key exists.
// @param key the key to look for 
// @param allocateNew allocate a new key if the key doesn't exist
// @return key index or 0 if no key exists and allocateNew is set to false
   KeyCont_t::const_iterator i = sKeyCont().Find(key);
   if (i) return *i;
   if (!allocateNew) return (size_t)-1;

   // allocate
   size_t idx = sKeys().size();
   sKeys().push_back(key);
   sKeyCont().Insert(idx);
   return idx;
}


//-------------------------------------------------------------------------------
std::string 
Reflex::Internal::PropertyListImpl::PropertyAsString(const std::string & key) const {
//-------------------------------------------------------------------------------
// Return a property as a string.
   return PropertyAsString(PropertyKey(key));
}



//-------------------------------------------------------------------------------
std::string 
Reflex::Internal::PropertyListImpl::PropertyAsString(size_t key) const {
//-------------------------------------------------------------------------------
   // Return a string representation of the property with key.
   Any & a = PropertyValue(key);
   if (a) {
      std::ostringstream o;
      o << a;
      return o.str();
   }
   return "";
}



//-------------------------------------------------------------------------------
size_t
Reflex::Internal::PropertyListImpl::PropertyKey(const std::string & key,
                                              bool allocateNew) const {
//-------------------------------------------------------------------------------
   // return the index of property key, allocate a new one if allocateNew = true
   return KeyByName(key, allocateNew);
}


//-------------------------------------------------------------------------------
std::string
Reflex::Internal::PropertyListImpl::PropertyKeys() const {
//-------------------------------------------------------------------------------
// Return a string containing all property keys.
   std::vector<std::string> kv(sKeys().size());
   for (size_t i = 0; i < sKeys().size(); ++i) {
      if (PropertyValue(i)) kv.push_back(sKeys()[i]);
   }
   std::string buf;
   return Tools::StringVec2String(buf, kv);
}


//-------------------------------------------------------------------------------
size_t
Reflex::Internal::PropertyListImpl::PropertyCount() const {
//-------------------------------------------------------------------------------
// Returns the number of properties attached to this item. Don't use the output
// for iteration. Use KeySize() instead.
   size_t s = 0;
   if (fProperties) {
      for (size_t i = 0; i < fProperties->size(); ++i) {
         if (PropertyValue(i)) ++s;
      }
   }
   return s;
}


//-------------------------------------------------------------------------------
Reflex::Any &
Reflex::Internal::PropertyListImpl::PropertyValue(const std::string & key) const {
//-------------------------------------------------------------------------------
// Return a property as an Any object.
   return PropertyValue(PropertyKey(key));
}


//-------------------------------------------------------------------------------
Reflex::Any &
Reflex::Internal::PropertyListImpl::PropertyValue(size_t key) const {
//-------------------------------------------------------------------------------
   // Return property as Any object
   if (fProperties && key < fProperties->size()) return (*fProperties)[ key ];
   return Dummy::Any();
}

