/* /% C++ %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Source file Type.cxx
 ************************************************************************
 * Description:
 *  Extended Run Time Type Identification API
 ************************************************************************
 * Author                  Masaharu Goto
 * Copyright(c) 1995~2005  Masaharu Goto
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

#include "Api.h"
#include "common.h"
#include "FastAllocString.h"

/*********************************************************************
* class G__TypeInfo
*********************************************************************/

Cint::G__TypeInfo::~G__TypeInfo()
{
}

Cint::G__TypeInfo::G__TypeInfo()
   : G__ClassInfo(), type(0), typenum(-1), reftype(0), isconst(0)
{
}

#ifndef __MAKECINT__
Cint::G__TypeInfo::G__TypeInfo(const Cint::G__TypeInfo& rhs)
   : G__ClassInfo(rhs)
{
   type = rhs.type;
   typenum = rhs.typenum;
   reftype = rhs.reftype;
   isconst = rhs.isconst;
}
#endif // __MAKECINT__

#ifndef __MAKECINT__
Cint::G__TypeInfo& Cint::G__TypeInfo::operator=(const Cint::G__TypeInfo& rhs)
{
   if (this != &rhs) {
      // FIXME: Call of G__ClassInfo copy assignment operator is missing!
      type = rhs.type;
      typenum = rhs.typenum;
      reftype = rhs.reftype;
      isconst = rhs.isconst;
   }
   return *this;
}
#endif // __MAKECINT__

Cint::G__TypeInfo::G__TypeInfo(const char* typenamein)
   : G__ClassInfo(), type(0), typenum(-1), reftype(0), isconst(0)
{
   Init(typenamein);
}

#ifndef __MAKECINT__
Cint::G__TypeInfo::G__TypeInfo(G__value buf)
   : G__ClassInfo(), type(0), typenum(-1), reftype(0), isconst(0)
{
   Init(buf);
}
#endif // __MAKECINT__

void Cint::G__TypeInfo::Init(const char* typenamein)
{
   G__value buf = G__string2type_body(typenamein, 2);
   type = buf.type;
   tagnum = buf.tagnum;
   typenum = buf.typenum;
   reftype = buf.obj.reftype.reftype;
   isconst = buf.obj.i;
   class_property = 0;
}

#ifndef __MAKECINT__
void Cint::G__TypeInfo::Init(G__value& buf)
{
   type = buf.type;
   typenum = buf.typenum;
   tagnum = buf.tagnum;
   if ((type != 'd') && (type != 'f')) {
      reftype = buf.obj.reftype.reftype;
   }
   else {
      reftype = 0;
   }
   isconst = buf.isconst;
}
#endif // __MAKECINT__

#ifndef __MAKECINT__
void Cint::G__TypeInfo::Init(struct G__var_array* var, int ig15)
{
   type    = var->type[ig15];
   typenum = var->p_typetable[ig15];
   tagnum  = var->p_tagtable[ig15];
   reftype = var->reftype[ig15];
   isconst = var->constvar[ig15];
}
#endif // __MAKECINT__

int Cint::G__TypeInfo::operator==(const G__TypeInfo& a)
{
   if (type == a.type && tagnum == a.tagnum && typenum == a.typenum && reftype == a.reftype) {
      return 1;
   }
   return 0;
}

int Cint::G__TypeInfo::operator!=(const G__TypeInfo& a)
{
   if (type == a.type && tagnum == a.tagnum && typenum == a.typenum && reftype == a.reftype) {
      return 0;
   }
   return 1;
}

int Cint::G__TypeInfo::Type() const
{
   return type;
}

int Cint::G__TypeInfo::Typenum() const
{
   return typenum;
}

int Cint::G__TypeInfo::Reftype() const
{
   return reftype;
}

int Cint::G__TypeInfo::Isconst() const
{
   return isconst;
}

const char* Cint::G__TypeInfo::Name()
{
   static G__FastAllocString buf(G__LONGLINE);
   buf = G__type2string((int)type, (int)tagnum, (int)typenum, (int)reftype , (int)isconst);
   return buf;
}

const char* Cint::G__TypeInfo::TrueName()
{
   static G__FastAllocString buf(G__LONGLINE);
   buf = G__type2string((int)type, (int)tagnum, -1, (int)reftype, (int)isconst);
   return buf;
}

int Cint::G__TypeInfo::Size() const
{
   G__value buf;
   buf.type = (int)type;
   buf.tagnum = (int)tagnum;
   buf.typenum = (int)typenum;
   buf.ref = reftype;
   if (isupper(type)) {
      buf.obj.reftype.reftype = reftype;
      return sizeof(void*);
   }
   return G__sizeof(&buf);
}

long Cint::G__TypeInfo::Property()
{
   long property = 0;
   if (typenum != -1) {
      property |= G__BIT_ISTYPEDEF;
   }
   if (tagnum == -1) {
      property |= G__BIT_ISFUNDAMENTAL;
   }
   else {
      if (
         strcmp(G__struct.name[tagnum], "G__longlong") == 0 ||
         strcmp(G__struct.name[tagnum], "G__ulonglong") == 0 ||
         strcmp(G__struct.name[tagnum], "G__longdouble") == 0
      ) {
         property |= G__BIT_ISFUNDAMENTAL;
         if (
            (typenum != -1) &&
            (
               strcmp(G__newtype.name[typenum], "long long") == 0 ||
               strcmp(G__newtype.name[typenum], "unsigned long long") == 0 ||
               strcmp(G__newtype.name[typenum], "long double") == 0)
         ) {
            property &= (~G__BIT_ISTYPEDEF);
         }
      }
      else {
         if (G__ClassInfo::IsValid()) {
            property |= G__ClassInfo::Property();
         }
      }
   }
   if (isupper((int)type)) {
      property |= G__BIT_ISPOINTER;
   }
   if (reftype == G__PARAREFERENCE || reftype > G__PARAREF) {
      property |= G__BIT_ISREFERENCE;
   }
   if (isconst & G__CONSTVAR) {
      property |= G__BIT_ISCONSTANT;
   }
   if (isconst & G__PCONSTVAR) {
      property |= G__BIT_ISPCONSTANT;
   }
   return property;
}

void* Cint::G__TypeInfo::New()
{
   if (G__ClassInfo::IsValid()) {
      return G__ClassInfo::New();
   }
   return new char[Size()];
}

int Cint::G__TypeInfo::IsValid()
{
   if (G__ClassInfo::IsValid()) {
      return 1;
   }
   else if (type) {
      return 1;
   }
   return 0;
}

G__value Cint::G__TypeInfo::Value() const
{
   G__value buf;
   buf.type = type;
   buf.tagnum = tagnum;
   buf.typenum = typenum;
   buf.isconst = (G__SIGNEDCHAR_T)isconst;
   buf.obj.reftype.reftype = reftype;
   buf.obj.i = 1;
   buf.ref = 0;
   return buf;
}

int Cint::G__TypeInfo::Next()
{
   return 0;
}

