//Generated at Wed Jun  9 18:25:46 2010
//Do not modify.

#include "Reflex/Builder/ReflexBuilder.h"
#include <typeinfo>
using namespace Reflex;

namespace {
}

// ---------------------- Shadow classes -----------------------------------
namespace __shadow__ {

}

// ---------------------- Stub functions -----------------------------------
namespace {

static void* function_0(void*, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = f1(*(float*) arg[0], *(float*) arg[1]);
   return & ret;
}

static void* function_1(void*, const std::vector<void*>& arg, void*)
{
   return (void*)f2(*(float*) arg[0], *(float*) arg[1]);
}

static void* function_2(void*, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = f3(*(float*) arg[0], *(float*) arg[1]);
   return & ret;
}

static void* function_3(void*, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = p1(*(int*) arg[0], *(int*) arg[1]);
   return & ret;
}

static void* function_4(void*, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = p2(*(int*) arg[0], *(int*) arg[1]);
   return & ret;
}

// -- Stub functions for class C--
static void* constructor_5(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) C();
}
static void* constructor_6(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) C(*(int*) arg[0]);
}
static void* destructor_7(void * o, const std::vector<void*>&, void *)
{
   ((C*)o)->~C();
   return 0;
}

static void* method_8(void* o, const std::vector<void*>&, void*)
{
   static int ret;
   ret = ((C*)o)->get();
   return & ret;
}

static void* method_9(void* o, const std::vector<void*>& arg, void*)
{
   ((C*)o)->set(*(int*) arg[0]);
   return 0;
}

static void* method_10(void* o, const std::vector<void*>&, void*)
{
   static int ret;
   ret = ((C*)o)->non_static_get();
   return & ret;
}

static void* method_11(void* o, const std::vector<void*>&, void*)
{
   static int ret;
   ret = ((C*)o)->one();
   return & ret;
}

static void* method_12(void* o, const std::vector<void*>&, void*)
{
   static int ret;
   ret = ((C*)o)->sum();
   return & ret;
}

static void* method_13(void* o, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = ((C*)o)->operator*(*(C*) arg[0]);
   return & ret;
}

static void* method_14(void* o, const std::vector<void*>& arg, void*)
{
   static int ret;
   ret = ((C*)o)->operator-(*(C*) arg[0]);
   return & ret;
}

static void* method_15(void* o, const std::vector<void*>&, void*)
{
   return new _Bool(((C*)o)->operator _Bool());
}

static void* method_16(void* o, const std::vector<void*>&, void*)
{
   static double ret;
   ret = ((C*)o)->operator double();
   return & ret;
}
static void* constructor_17(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) C(*(const C*) arg[0]);
}

static void* method_18(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((C*)o)->operator=(*(const C*) arg[0]);
}

// -- Stub functions for class A--

static void* method_19(void* o, const std::vector<void*>&, void*)
{
   static int ret;
   ret = ((A*)o)->m();
   return & ret;
}
static void* constructor_20(void* mem, const std::vector<void*>&, void*)
{
   return ::new(mem) A();
}
static void* constructor_21(void* mem, const std::vector<void*>& arg, void*)
{
   return ::new(mem) A(*(const A*) arg[0]);
}

static void* method_22(void* o, const std::vector<void*>& arg, void*)
{
   return (void*) & ((A*)o)->operator=(*(const A*) arg[0]);
}
static void* destructor_23(void * o, const std::vector<void*>&, void *)
{
   ((A*)o)->~A();
   return 0;
}
} // unnamed namespace

// -------------------- Class dictionaries ---------------------------------
// --------------------- Dictionary instances ------------------------------
namespace {
struct Dictionaries {
   Dictionaries() {
      RegisterFunctionStub("f1(float,float)", method_0);
      RegisterFunctionStub("f2(float,float)", method_1);
      RegisterFunctionStub("f3(float,float)", method_2);
      RegisterFunctionStub("p1(int,int)", method_3);
      RegisterFunctionStub("p2(int,int)", method_4);
      RegisterFunctionStub("C::C()", constructor_5);
      RegisterFunctionStub("C::C(int)", constructor_6);
      RegisterFunctionStub("C::~C()", destructor_7);
      RegisterFunctionStub("C::get()", method_8);
      RegisterFunctionStub("C::set(int)", method_9);
      RegisterFunctionStub("C::non_static_get()", method_10);
      RegisterFunctionStub("C::one()", method_11);
      RegisterFunctionStub("C::sum()", method_12);
      RegisterFunctionStub("C::operator*(class C)", method_13);
      RegisterFunctionStub("C::operator-(class C)", method_14);
      RegisterFunctionStub("C::operator _Bool()", method_15);
      RegisterFunctionStub("C::operator double()", method_16);
      RegisterFunctionStub("C::C(class C const &)", constructor_17);
      RegisterFunctionStub("C::operator=(class C const &)", method_18);
      RegisterFunctionStub("A::m()", method_19);
      RegisterFunctionStub("A::A()", constructor_20);
      RegisterFunctionStub("A::A(class A const &)", constructor_21);
      RegisterFunctionStub("A::operator=(class A const &)", method_22);
      RegisterFunctionStub("A::~A()", destructor_23);
   }
   ~Dictionaries() {
   }
};
static Dictionaries instance;
}

// End of Dictionary
