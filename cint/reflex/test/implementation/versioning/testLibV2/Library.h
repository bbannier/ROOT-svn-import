#ifndef TEST_LIBV2_LIBRARY_H
#define TEST_LIBV2_LIBRARY_H

#ifdef WIN32
#ifdef test_LibV2_EXPORTS
#define TEST_LIBV2_API _declspec(dllexport)
#else
#define TEST_LIBV2_API _declspec(dllimport)
#endif
#else
#define TEST_LIBV2_API
#endif

#include "../testLibCommon/BaseLibrary.h"
#include "Value.h"

class TEST_LIBV2_API Library : public BaseLibrary
{
public:
   double version() const;
   const std::type_info* typeId() const;

   Value smallName() const;

   Reflex::Dictionary Dictionary() const;

};


class ClassV2 : public BaseClass
{
public:
   ClassV2();
   virtual ~ClassV2();
};

#endif
