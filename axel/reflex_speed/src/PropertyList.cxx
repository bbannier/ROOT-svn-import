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

#include "Reflex/PropertyList.h"

#include "PropertyListImpl.h"
#include "Reflex/Any.h"

//-------------------------------------------------------------------------------
std::ostream & Reflex::operator<<(std::ostream & s,
                                         const PropertyList & p) {
//-------------------------------------------------------------------------------
// Operator to put a property list on the ostream.
   if (p.fPropertyListImpl) s << *(p.fPropertyListImpl); 
   return s;
}


//-------------------------------------------------------------------------------
size_t
Reflex::PropertyList::AddProperty(const std::string & key,
                                                       const Any & value) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->AddProperty(key, value);
   return 0;
}


//-------------------------------------------------------------------------------
void
Reflex::PropertyList::AddProperty(size_t key,
                                                     const Any & value) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->AddProperty(key, value);
}


//-------------------------------------------------------------------------------
size_t
Reflex::PropertyList::AddProperty(const std::string & key,
                                                       const char* value) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->AddProperty(key, value);
   return 0;
}


//-------------------------------------------------------------------------------
void
Reflex::PropertyList::AddProperty(size_t key,
                                                     const char* value) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->AddProperty(key, value);
}


//-------------------------------------------------------------------------------
void
Reflex::PropertyList::ClearProperties() const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) fPropertyListImpl->ClearProperties();
}


//-------------------------------------------------------------------------------
bool
Reflex::PropertyList::HasProperty(const std::string & key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->HasProperty(key);
   return false;
}


//-------------------------------------------------------------------------------
bool
Reflex::PropertyList::HasProperty(size_t key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->HasProperty(key);
   return false;
}


//-------------------------------------------------------------------------------
std::string 
Reflex::PropertyList::PropertyAsString(const std::string & key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->PropertyAsString(key);
   return "";
}


//-------------------------------------------------------------------------------
std::string 
Reflex::PropertyList::PropertyAsString(size_t key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->PropertyAsString(key);
   return "";
}


//-------------------------------------------------------------------------------
size_t
Reflex::PropertyList::PropertyKey(const std::string & key,
                                                       bool allocateNew) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->PropertyKey(key, allocateNew);
   return 0;
}


//-------------------------------------------------------------------------------
std::string
Reflex::PropertyList::PropertyKeys() const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->PropertyKeys();
   return "";
}


//-------------------------------------------------------------------------------
size_t
Reflex::PropertyList::PropertyCount() const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) return fPropertyListImpl->PropertyCount();
   return 0;
}


//-------------------------------------------------------------------------------
void
Reflex::PropertyList::RemoveProperty(const std::string & key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) fPropertyListImpl->RemoveProperty(key);
}


//-------------------------------------------------------------------------------
void
Reflex::PropertyList::RemoveProperty(size_t key) const {
//-------------------------------------------------------------------------------
   if (fPropertyListImpl) fPropertyListImpl->RemoveProperty(key);
}


//-------------------------------------------------------------------------------
Reflex::OrderedContainer<std::string>
Reflex::PropertyList::Keys() {
//-------------------------------------------------------------------------------
// Retrieve container of all known keys
   return Internal::PropertyListImpl::Keys();
}


//-------------------------------------------------------------------------------
std::string
Reflex::PropertyList::KeysAsString() {
//-------------------------------------------------------------------------------
// Return all keys as one string concatenation.
   return Internal::PropertyListImpl::KeysAsString();
}


//-------------------------------------------------------------------------------
const std::string &
Reflex::PropertyList::KeyAt(size_t nth) {
//-------------------------------------------------------------------------------
// Return key at position nth.
   return Internal::PropertyListImpl::KeyAt(nth);
}


//-------------------------------------------------------------------------------
size_t
Reflex::PropertyList::KeyByName(const std::string & key,
                                bool allocateNew) {
//-------------------------------------------------------------------------------
// Return the position of a Key. If allocateNew is set to true allocate a new key
// if necessary.
   return Internal::PropertyListImpl::KeyByName(key, allocateNew);
}


//-------------------------------------------------------------------------------
Reflex::Any &
Reflex::PropertyList::PropertyValue(const std::string & key) const {
//-------------------------------------------------------------------------------
// Get the value of a property as Any object.
   if (fPropertyListImpl) return fPropertyListImpl->PropertyValue(key);
   return Dummy::Any();
}


//-------------------------------------------------------------------------------
Reflex::Any &
Reflex::PropertyList::PropertyValue(size_t key) const {
//-------------------------------------------------------------------------------
// Get the value of a property as Any object.
   if (fPropertyListImpl) return fPropertyListImpl->PropertyValue(key);
   return Dummy::Any();
}

