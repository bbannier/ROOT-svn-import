#ifndef TEST_LIBCOMMON_SHARED_NAMESPACE_H
#define TEST_LIBCOMMON_SHARED_NAMESPACE_H

#ifdef WIN32
#ifdef test_LibCommon_EXPORTS
#define TEST_LIBCOMMON_API _declspec(dllexport)
#else
#define TEST_LIBCOMMON_API _declspec(dllimport)
#endif
#else
#define TEST_LIBCOMMON_API
#endif

namespace SharedNamespace
{
   class TEST_LIBCOMMON_API Dummy
   {
   public:
      Dummy();
      virtual ~Dummy();
   };

   TEST_LIBCOMMON_API Dummy* NewDummy();

}


#endif
