/* /% C++ %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Header file Type.h
 ************************************************************************
 * Description:
 *  Extended Run Time Type Identification API
 ************************************************************************
 * Copyright(c) 1995~2003  Masaharu Goto
 *
 * For the licensing terms see the file COPYING
 *
 ************************************************************************/

#ifndef G__TYPEINFOX_H
#define G__TYPEINFOX_H

#ifndef G__API_H
#include "Api.h"
#endif

namespace Cint {

/*********************************************************************
* class G__TypeInfo
*
*********************************************************************/
class
#ifndef __CINT__
   G__EXPORT
#endif
   // FIXME: Warning, G__TypeReader in src/bc_type.h inherits from G__TypeInfo, we may need a virtual destructor.
      G__TypeInfo : public G__ClassInfo {
   friend class G__DataMemberInfo;
   friend class G__MethodInfo;
   friend class G__MethodArgInfo;
protected:
   long type;
   long typenum;
   long reftype;
   long isconst;
public:
   ~G__TypeInfo();
   G__TypeInfo();
#ifndef __MAKECINT__
   G__TypeInfo(const G__TypeInfo&);
   G__TypeInfo& operator=(const G__TypeInfo&);
#endif // __MAKECINT__

   G__TypeInfo(const char*);
#ifndef __MAKECINT__
   G__TypeInfo(G__value);
#endif // __MAKECINT__

   void Init(const char*);
#ifndef __MAKECINT__
   void Init(G__value&);
   void Init(struct G__var_array*, int);
#endif // __MAKECINT__

   int operator==(const G__TypeInfo&);
   int operator!=(const G__TypeInfo&);

   int Type() const;
   int Typenum() const;
   int Reftype() const;
   int Isconst() const;

   const char* Name();
   const char* TrueName();
   int Size() const;
   long Property();
   int IsValid();
   void* New();
   G__value Value() const;
private:
   int Next(); // prohibit use of next
};

} // namespace Cint

using namespace Cint;
#endif
