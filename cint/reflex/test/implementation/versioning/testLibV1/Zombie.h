#ifndef TEST_LIBV1_ZOMBIE_H
#define TEST_LIBV1_ZOMBIE_H

#ifdef WIN32
#ifdef test_LibV1_EXPORTS
#define TEST_LIBV1_API _declspec(dllexport)
#else
#define TEST_LIBV1_API _declspec(dllimport)
#endif
#else
#define TEST_LIBV1_API
#endif

namespace SharedNamespace
{
   class TEST_LIBV1_API Zombie
   {
   public:
      Zombie();
      virtual ~Zombie();
   };

   TEST_LIBV1_API Zombie* NewZombie();

}

#endif
