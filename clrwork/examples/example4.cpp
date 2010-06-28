
/* example4.cc */

extern int f1(float x, float y);
int * f2(float x, float y);
int f3(float x, float y)
{
   return x + y;
}

int p1(int i, int j)
{
   return i + j;
}

#if 1
int p2(int i, int j = 1);
#endif

/* ---------------------------------------------------------------------- */

class A {
   int x, y, z;
   int m() {
      return 1;
   }
};

namespace N {
namespace M {
typedef double T;

class B {
   int u, v;
   int q() {
      return 1;
   }
};
}
};

class C {
private:
   int x, y, z;
   static int u;
public:
   C() : x(0), y(0), z(0) {
      u ++;
   }
   C(int i) : x(i), y(i), z(i) { }
   ~ C() {
      u --;
   }

   static int get() {
      return u;
   }
   static void set(int i) {
      u = i;
   }
   int non_static_get() {
      return u + 1;
   }

   int one() {
      return 1;
   }

#if 1
   int sum() {
      return x + y + z;
   }
#endif

   int operator *(C second) {
      return 1;
   }

#if 1
   int operator - (C second) {
      return this->x - second.x;
   }
#endif

   operator bool () {
      return true;
   }
   operator double() {
      return 1.1;
   }

#if 0
   friend class A;
#endif
};

/* ---------------------------------------------------------------------- */

#if 0
using namespace N;
M::T v1;
#endif

#if 0
using N::M::T;
T v2;
#endif

/* ---------------------------------------------------------------------- */

#if 0
template <class T>
T min(T x, T y)
{
   return x < y ? x : y;
}

#endif

#if 0
template <int i>
int inc(int k)
{
   return k + i;
}

#endif

#if 0
template <class T>
class Pointer {
private:
   T * ref;
public:
   Pointer(T * init) : ref(init) { }
   T deref() {
      return * ref;
   }
};
#endif

/* ---------------------------------------------------------------------- */
