// @(#)root/meta:$Id$
// Author: Fons Rademakers   20/06/96

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOT_TDictionary
#define ROOT_TDictionary

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TDictionary                                                          //
//                                                                      //
// This class defines an abstract interface that must be implemented    //
// by all classes that contain dictionary information.                  //
//                                                                      //
// The dictionary is defined by the followling classes:                 //
// TDataType                              (typedef definitions)         //
// TGlobal                                (global variables)            //
// TFunction                              (global functions)            //
// TClass                                 (classes)                     //
//    TBaseClass                          (base classes)                //
//    TDataMember                         (class datamembers)           //
//    TMethod                             (class methods)               //
//       TMethodArg                       (method arguments)            //
//                                                                      //
// All the above classes implement the TDictionary abstract interface   //
// (note: the indentation shows aggregation not inheritance).           //
// The ROOT dictionary system provides a very extensive RTTI            //
// environment that facilitates a.o. object inspectors, object I/O,     //
// ROOT Trees, etc. Most of the type information is provided by the     //
// CINT C++ interpreter.                                                //
//                                                                      //
// TMethodCall                            (method call environment)     //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

//#include "Property.h"
#define G__BIT_ISTAGNUM          0x0000000f
#define G__BIT_ISCLASS           0x00000001
#define G__BIT_ISSTRUCT          0x00000002
#define G__BIT_ISUNION           0x00000004
#define G__BIT_ISENUM            0x00000008
#define G__BIT_ISTYPEDEF         0x00000010
#define G__BIT_ISFUNDAMENTAL     0x00000020
#define G__BIT_ISABSTRACT        0x00000040
#define G__BIT_ISVIRTUAL         0x00000080
#define G__BIT_ISPUREVIRTUAL     0x00000100
#define G__BIT_ISPUBLIC          0x00000200
#define G__BIT_ISPROTECTED       0x00000400
#define G__BIT_ISPRIVATE         0x00000800
#define G__BIT_ISPOINTER         0x00001000
#define G__BIT_ISARRAY           0x00002000
#define G__BIT_ISSTATIC          0x00004000
#define G__BIT_ISDEFAULT         0x00008000

#define G__BIT_ISREFERENCE       0x00010000
#define G__BIT_ISDIRECTINHERIT   0x00020000
#define G__BIT_ISCCOMPILED       0x00040000
#define G__BIT_ISCPPCOMPILED     0x00080000
#define G__BIT_ISCOMPILED        0x000c0000
#define G__BIT_ISBYTECODE        0x02000000
#define G__BIT_ISCONSTANT        0x00100000
#define G__BIT_ISVIRTUALBASE     0x00200000
#define G__BIT_ISPCONSTANT       0x00400000
#define G__BIT_ISMETHCONSTANT    0x10000000

#define G__BIT_ISGLOBALVAR       0x00800000
#define G__BIT_ISLOCALVAR        0x01000000
#define G__BIT_ISEXPLICIT        0x04000000
#define G__BIT_ISNAMESPACE       0x08000000

#define G__BIT_ISUSINGVARIABLE   0x20000000

#define G__CLS_VALID             0x00000001

#define G__CLS_HASEXPLICITCTOR   0x00000010
#define G__CLS_HASIMPLICITCTOR   0x00000020
#define G__CLS_HASCTOR           0x00000030

#define G__CLS_HASDEFAULTCTOR    0x00000040
#define G__CLS_HASASSIGNOPR      0x00000080

#define G__CLS_HASEXPLICITDTOR   0x00000100
#define G__CLS_HASIMPLICITDTOR   0x00000200
#define G__CLS_HASDTOR           0x00000300

#define G__CLS_HASVIRTUAL        0x00001000
#define G__CLS_ISABSTRACT        0x00002000

typedef void CallFunc_t;
typedef void ClassInfo_t;
typedef void BaseClassInfo_t;
typedef void DataMemberInfo_t;
typedef void MethodInfo_t;
typedef void MethodArgInfo_t;
typedef void MethodArgInfo_t;
typedef void TypeInfo_t;
typedef void TypedefInfo_t;

enum EProperty {
   kIsClass        = G__BIT_ISCLASS,
   kIsStruct       = G__BIT_ISSTRUCT,
   kIsUnion        = G__BIT_ISUNION,
   kIsEnum         = G__BIT_ISENUM,
   kIsNamespace    = G__BIT_ISNAMESPACE,
   kIsTypedef      = G__BIT_ISTYPEDEF,
   kIsFundamental  = G__BIT_ISFUNDAMENTAL,
   kIsAbstract     = G__BIT_ISABSTRACT,
   kIsVirtual      = G__BIT_ISVIRTUAL,
   kIsPureVirtual  = G__BIT_ISPUREVIRTUAL,
   kIsPublic       = G__BIT_ISPUBLIC,
   kIsProtected    = G__BIT_ISPROTECTED,
   kIsPrivate      = G__BIT_ISPRIVATE,
   kIsPointer      = G__BIT_ISPOINTER,
   kIsArray        = G__BIT_ISARRAY,
   kIsStatic       = G__BIT_ISSTATIC,
   kIsUsingVariable= G__BIT_ISUSINGVARIABLE,
   kIsDefault      = G__BIT_ISDEFAULT,
   kIsReference    = G__BIT_ISREFERENCE,
   kIsConstant     = G__BIT_ISCONSTANT,
   kIsConstPointer = G__BIT_ISPCONSTANT,
   kIsMethConst    = G__BIT_ISMETHCONSTANT
};


class TDictionary : public TNamed {

public:
   TDictionary() { }
   TDictionary(const char* name): TNamed(name, "") { }
   virtual ~TDictionary() { }

   virtual Long_t      Property() const = 0;
   static TDictionary* GetDictionary(const char* name);

   // Type of STL container (returned by IsSTLContainer).
   enum ESTLType {kNone=0, kVector=1, kList, kDeque, kMap, kMultimap, kSet, kMultiset};

   ClassDef(TDictionary,0)  //ABC defining interface to dictionary
};

#endif
