#ifndef TEST_LIBV1_LIBRARY_H
#define TEST_LIBV1_LIBRARY_H

#ifdef WIN32
#ifdef test_LibV1_EXPORTS
#define TEST_LIBV1_API _declspec(dllexport)
#else
#define TEST_LIBV1_API _declspec(dllimport)
#endif
#else
#define TEST_LIBV1_API
#endif

#include "../testLibCommon/BaseLibrary.h"
#include "Value.h"

class TEST_LIBV1_API Library : public BaseLibrary
{
public:
   Library();

   double version() const;
   const std::type_info* typeId() const;

   Value smallName() const;

   Reflex::Dictionary Dictionary() const;

   BaseClass* getBaseClass() const;
   void setBaseClass(BaseClass* value);

private:
   BaseClass* fBaseClass;
};

class TEST_LIBV1_API D : public C
{
public:
   virtual ~D();
};

#endif
