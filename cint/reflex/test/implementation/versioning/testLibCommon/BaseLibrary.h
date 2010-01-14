#ifndef TEST_LIBCOMMON_BASELIBRARY_H
#define TEST_LIBCOMMON_BASELIBRARY_H

#ifdef WIN32
#ifdef test_LibCommon_EXPORTS
#define TEST_LIBCOMMON_API _declspec(dllexport)
#else
#define TEST_LIBCOMMON_API _declspec(dllimport)
#endif
#else
#define TEST_LIBCOMMON_API
#endif

#include <Reflex/Dictionary.h>

class TEST_LIBCOMMON_API BaseLibrary
{
public:
   virtual ~BaseLibrary();
   virtual double version() const = 0;
   virtual const std::type_info* typeId() const = 0;

   virtual Reflex::Dictionary Dictionary() const = 0;
};



class TEST_LIBCOMMON_API BaseClass
{
public:
   virtual ~BaseClass();
};

class TEST_LIBCOMMON_API A
{
public:
   virtual ~A();
};

class TEST_LIBCOMMON_API B
{
public:
   virtual ~B();
};

class TEST_LIBCOMMON_API C : public A, public B
{
public:
   virtual ~C();
};

#endif
