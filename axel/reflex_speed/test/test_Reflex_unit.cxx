// @(#)root/reflex:$Id$
// Author: Stefan Roiser 2004

// CppUnit include file
#include "cppunit/extensions/HelperMacros.h"

// Seal include files
#include "Reflex/Reflex.h"

#include "src/Fundamental.h"
#include "src/Pointer.h"
#include "src/DataMember.h"
#include "src/Array.h"
#include "src/Typedef.h"
#include "src/Namespace.h"
#include "src/Function.h"
#include "src/FunctionMember.h"
#include "src/Class.h"
#include "src/ClassTemplateInstance.h"
#include "src/OwnedPropertyList.h"
#include "src/PropertyListImpl.h"
#include "src/OwnedMember.h"
#include "Reflex/SharedLibrary.h"

// Standard C++ include files
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <string.h>

using namespace Reflex;
using namespace Reflex::Internal;
using namespace std;


/** test_SealKenel_Properties.cpp
  *
  * Simple test cases to exercise Framework properties
  */

class ReflexUnitTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( ReflexUnitTest );
  CPPUNIT_TEST( container );
  CPPUNIT_TEST( ordered_container );
  CPPUNIT_TEST( any_type );
  CPPUNIT_TEST( empty_type );
  CPPUNIT_TEST( empty_scope );
  CPPUNIT_TEST( empty_member );
  CPPUNIT_TEST( exception );
  CPPUNIT_TEST( property_list );
  CPPUNIT_TEST( fundamental_type );
  CPPUNIT_TEST( pointer_type );
  CPPUNIT_TEST( reference_type );
  CPPUNIT_TEST( array_type );
  CPPUNIT_TEST( typedef_type );
  CPPUNIT_TEST( class_type );
  CPPUNIT_TEST( type_base );
  CPPUNIT_TEST( object_basics );
  CPPUNIT_TEST( namespace_scope );
  CPPUNIT_TEST( namespace_members );
  CPPUNIT_TEST( function_member );
  CPPUNIT_TEST( type_template );
  CPPUNIT_TEST( member );
  CPPUNIT_TEST( tools );
  CPPUNIT_TEST( global_scope );
  CPPUNIT_TEST( setClassAtts );
  CPPUNIT_TEST( object_value );
  CPPUNIT_TEST( sharedlibrary );
   
  CPPUNIT_TEST( shutdown );
  CPPUNIT_TEST_SUITE_END();
public:
  void setUp () {}
  void container();
  void ordered_container();
  void any_type();
  void empty_type();
  void empty_scope();
  void empty_member();
  void exception();
  void property_list();
  void fundamental_type();
  void pointer_type();
  void reference_type();
  void array_type();
  void typedef_type();
  void class_type();
  void type_base();
  void object_basics();
  void typedef_basics();
  void namespace_scope();
  void namespace_members();
  void function_member();
  void type_template();
  void member();
  void tools();
  void global_scope();
  void setClassAtts();
  void object_value();
  void sharedlibrary();

  void shutdown() {}
  void tearDown() {}
};

struct foo { 
  foo(int i) : m(i) {}
  int m; 
};

struct bar { 
  bar(float i) :b(i) {}
  float b; 
};

void ReflexUnitTest::container()
{
   Type tInt(Type::ByName("int"));
   Type tFloat(Type::ByName("float"));

   Reflex::Internal::ContainerImpl<std::string, Reflex::Type, Reflex::Internal::kUnique> typesU;

   size_t numAllocatedNodes = typesU.Arena()->Entries();

   CPPUNIT_ASSERT_EQUAL((size_t)0, typesU.Size());
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));

   typesU.Insert(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Insert(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Insert(tFloat);
   CPPUNIT_ASSERT_EQUAL((size_t)2, typesU.Size());
   CPPUNIT_ASSERT(typesU.Find("int") != typesU.End());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.End());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.FindValue(tInt) != typesU.End());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.End());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.FindValue(tInt));

   typesU.Remove(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("float"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(tFloat == *typesU.Begin());

   typesU.Insert(tInt);
   CPPUNIT_ASSERT(typesU.Find("int") != typesU.End());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.End());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.FindValue(tInt) != typesU.End());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.End());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.FindValue(tInt));

   typesU.Remove(tFloat);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Remove(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)0, typesU.Size());
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT_EQUAL(numAllocatedNodes, typesU.Arena()->Entries());
}

void ReflexUnitTest::ordered_container()
{
   Type tInt(Type::ByName("int"));
   Type tFloat(Type::ByName("float"));

   Reflex::Internal::OrderedContainerImpl<std::string, Reflex::Type, Reflex::Internal::kUnique> typesU;

   size_t numAllocatedNodes = typesU.Arena()->Entries();

   CPPUNIT_ASSERT_EQUAL((size_t)0, typesU.Size());
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));

   typesU.Insert(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Insert(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Insert(tFloat);
   CPPUNIT_ASSERT_EQUAL((size_t)2, typesU.Size());
   CPPUNIT_ASSERT(typesU.Find("int") == typesU.Begin());
   CPPUNIT_ASSERT(typesU.Find("float") == ++typesU.Begin());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.FindValue(tInt) == typesU.Begin());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) == ++typesU.Begin());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.FindValue(tInt));

   typesU.Remove(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("float"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(tFloat == *typesU.Begin());

   typesU.Insert(tInt);
   CPPUNIT_ASSERT(typesU.Find("int") == ++typesU.Begin());
   CPPUNIT_ASSERT(typesU.Find("float") == typesU.Begin());
   CPPUNIT_ASSERT(typesU.Find("float") != typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.FindValue(tInt) == ++typesU.Begin());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) == typesU.Begin());
   CPPUNIT_ASSERT(typesU.FindValue(tFloat) != typesU.FindValue(tInt));

   typesU.Remove(tFloat);
   CPPUNIT_ASSERT_EQUAL((size_t)1, typesU.Size());
   CPPUNIT_ASSERT_EQUAL(std::string("int"), typesU.Begin()->Name());
   CPPUNIT_ASSERT(typesU.Begin() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.Begin() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("float"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tFloat));
   CPPUNIT_ASSERT(tInt == *typesU.Begin());

   typesU.Remove(tInt);
   CPPUNIT_ASSERT_EQUAL((size_t)0, typesU.Size());
   CPPUNIT_ASSERT(typesU.End() == typesU.Find("int"));
   CPPUNIT_ASSERT(typesU.End() == typesU.FindValue(tInt));
   CPPUNIT_ASSERT_EQUAL(numAllocatedNodes, typesU.Arena()->Entries());
}

void ReflexUnitTest::any_type()
{
  Any empty;
  CPPUNIT_ASSERT(empty.Empty());
  empty = foo(20); 
  CPPUNIT_ASSERT(!empty.Empty());
  CPPUNIT_ASSERT_EQUAL(typeid(foo).name(),empty.TypeInfo().name());

  Any i(10);
  CPPUNIT_ASSERT(!i.Empty());
  CPPUNIT_ASSERT_EQUAL(10, any_cast<int>(i));
  //CPPUNIT_ASSERT_THROW(any_cast<float>(i), Reflex::BadAnyCast);
  try {
    any_cast<float>(i);
  }
  catch (Reflex::BadAnyCast e) {
    CPPUNIT_ASSERT_EQUAL(std::string("BadAnyCast: failed conversion using any_cast"), std::string(e.what()));
  }
  Any f(10.66F);
  CPPUNIT_ASSERT_EQUAL(10.66F, any_cast<float>(f));
  Any d(99.77);
  CPPUNIT_ASSERT_EQUAL(99.77, any_cast<double>(d));
  Any s(string("string"));
  CPPUNIT_ASSERT_EQUAL(string("string"), any_cast<string>(s));
  const char* cs = "c-string";
  Any c(cs);
  CPPUNIT_ASSERT_EQUAL(cs, any_cast<const char*>(c));
  stringstream ss;
  ss << i;
  CPPUNIT_ASSERT_EQUAL(ss.str(), string("10"));

}


void ReflexUnitTest::empty_type() {
  Type t;
  CPPUNIT_ASSERT_EQUAL(38, int(Catalog::Instance().Types().Size()));
  CPPUNIT_ASSERT(!t);
  CPPUNIT_ASSERT_EQUAL(size_t(0), size_t(t.Allocate()));
  CPPUNIT_ASSERT(t.Bases().Begin() == t.Bases().End());
  CPPUNIT_ASSERT_EQUAL(0, int(t.Bases().Size()));
  CPPUNIT_ASSERT(!t.CastObject(Type(),Object()));
  CPPUNIT_ASSERT(!t.Construct());
  Scope ts(t);
  CPPUNIT_ASSERT(ts.DataMembers().Begin() == ts.DataMembers().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.DataMembers().Size()));
  CPPUNIT_ASSERT(!t.DeclaringScope());
  CPPUNIT_ASSERT(!t.DynamicType(Object()));
  CPPUNIT_ASSERT(ts.FunctionMembers().Begin() == ts.FunctionMembers().End());
  CPPUNIT_ASSERT_EQUAL(0,int(ts.FunctionMembers().Size()));
  CPPUNIT_ASSERT(!(bool)t.HasBase(Type()));
  CPPUNIT_ASSERT_EQUAL(size_t(0), size_t(t.Id()));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kAbstract));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kArray));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kClassOrStruct));
  CPPUNIT_ASSERT_EQUAL(false, ts.IsComplete());
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kConst));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kConstVolatile));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kEnum));
  CPPUNIT_ASSERT_EQUAL(true, t.IsEquivalentTo(Type()));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kFunction));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kFundamental));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kPointer));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kPointerToMember));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kReference));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kStruct));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kTemplateInstance));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kTypedef));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kUnion));
  CPPUNIT_ASSERT_EQUAL(true, t.IsUnqualified());
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kVirtual));
  CPPUNIT_ASSERT_EQUAL(false, t.Is(kVolatile));
  CPPUNIT_ASSERT_EQUAL(0, int(t.ArrayLength()));
  CPPUNIT_ASSERT(!ts.Members().ByName(""));
  CPPUNIT_ASSERT(ts.Members().Begin() == ts.Members().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.Members().Size()));
  CPPUNIT_ASSERT(ts.MemberTemplates().Begin() == ts.MemberTemplates().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.MemberTemplates().Size()));
  CPPUNIT_ASSERT_EQUAL(std::string(""), t.Name());
  std::string buf("foo");
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), t.Name(buf));
  CPPUNIT_ASSERT_EQUAL(std::string("foo"), buf);
  CPPUNIT_ASSERT(t.FunctionParameters().Begin() == t.FunctionParameters().End());
  CPPUNIT_ASSERT_EQUAL(0, int(t.FunctionParameters().Size()));
  CPPUNIT_ASSERT(!t.Properties());
  CPPUNIT_ASSERT(!t.ReturnType());
  CPPUNIT_ASSERT(!t.DeclaringScope());
  CPPUNIT_ASSERT_EQUAL(0, int(t.SizeOf()));
  CPPUNIT_ASSERT(ts.SubScopes().Begin() == ts.SubScopes().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.SubScopes().Size()));
  CPPUNIT_ASSERT(ts.SubTypes().Begin() == ts.SubTypes().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.SubTypes().Size()));
  CPPUNIT_ASSERT(t.TemplateArguments().Begin() == t.TemplateArguments().End());
  CPPUNIT_ASSERT_EQUAL(0, int(t.TemplateArguments().Size()));
  CPPUNIT_ASSERT(!t.TemplateFamily());
  CPPUNIT_ASSERT(!t.ToType());
  CPPUNIT_ASSERT(!t.FinalType());
  CPPUNIT_ASSERT(!t.RawType());
  CPPUNIT_ASSERT(!t.FinalType().RawType());
  CPPUNIT_ASSERT_EQUAL(std::string(typeid(void).name()), std::string(t.TypeInfo().name()));
  CPPUNIT_ASSERT(ts.SubTypeTemplates().Begin() == ts.SubTypeTemplates().End());
  CPPUNIT_ASSERT_EQUAL(0, int(ts.SubTypeTemplates().Size()));
  CPPUNIT_ASSERT_EQUAL(kETUnresolved,t.TypeType());
  CPPUNIT_ASSERT_EQUAL(std::string("kUnresolved"), t.TypeTypeAsString());
}

void ReflexUnitTest::empty_scope() {
  Scope s;
  CPPUNIT_ASSERT(s.DataMembers().Begin() == s.DataMembers().End());
}

void ReflexUnitTest::empty_member() {
  Member m;
  CPPUNIT_ASSERT(!m.Get());
}

void ReflexUnitTest::exception() {
   /* no exception anymore, UpdateMembers is now part of Scope:
  Type t = Type::ByName("int");
  CPPUNIT_ASSERT(t);
  //CPPUNIT_ASSERT_THROW(t.updateMembers(), Reflex::RuntimeError);
  try {
    t.UpdateMembers();
  }
  catch (Reflex::RuntimeError e) {
    CPPUNIT_ASSERT_EQUAL(std::string("REFLEX: Function UpdateMembers can only be called on Class/Struct"), std::string(e.what()));
  }
  */
}


struct AnyStruct {
   AnyStruct() : i(1), d(1.0), b(true) {}
   int i;
   double d;
   bool b;
};


void ReflexUnitTest::property_list()
{
   OwnedPropertyList pl(new PropertyListImpl());

  CPPUNIT_ASSERT(pl);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pl.PropertyCount());
  CPPUNIT_ASSERT_EQUAL((size_t)1, PropertyList::Keys().Size());
  CPPUNIT_ASSERT_EQUAL(std::string("Description"), *PropertyList::Keys().Begin());
  CPPUNIT_ASSERT_EQUAL(std::string("Description"), *PropertyList::Keys().RBegin());

  pl.AddProperty("Description", "some blabla");
  CPPUNIT_ASSERT_EQUAL((size_t)1, pl.PropertyCount());
  CPPUNIT_ASSERT_EQUAL((size_t)1, PropertyList::Keys().Size());

  CPPUNIT_ASSERT_EQUAL(NPos(), pl.PropertyKey("int"));
  CPPUNIT_ASSERT_EQUAL(size_t(1), pl.PropertyKey("int",true));
  CPPUNIT_ASSERT_EQUAL(size_t(2), PropertyList::Keys().Size());
  pl.AddProperty("int", 10);
  CPPUNIT_ASSERT_EQUAL(size_t(1), pl.PropertyKey("int"));
  CPPUNIT_ASSERT(pl.HasProperty("int"));
  CPPUNIT_ASSERT_EQUAL(10, any_cast<int>(pl.PropertyValue("int")));
  CPPUNIT_ASSERT_EQUAL(string("10"), pl.PropertyAsString("int"));
  CPPUNIT_ASSERT_EQUAL((size_t)2, pl.PropertyCount());

  pl.AddProperty("double", 10.8);
  CPPUNIT_ASSERT(pl.HasProperty("double"));
  CPPUNIT_ASSERT_EQUAL(10.8, any_cast<double>(pl.PropertyValue("double")));
  CPPUNIT_ASSERT_EQUAL(string("10.8"), pl.PropertyAsString("double"));
  CPPUNIT_ASSERT_EQUAL((size_t)3, pl.PropertyCount());

  pl.AddProperty("char*", "this is a char*" );
  CPPUNIT_ASSERT(pl.HasProperty("char*"));
  CPPUNIT_ASSERT(!strcmp("this is a char*",any_cast<const char*>(pl.PropertyValue("char*"))));
  CPPUNIT_ASSERT_EQUAL(string("this is a char*"), pl.PropertyAsString("char*"));
  CPPUNIT_ASSERT_EQUAL((size_t)4, pl.PropertyCount());
  CPPUNIT_ASSERT_EQUAL((size_t)4, PropertyList::Keys().Size());
  CPPUNIT_ASSERT_EQUAL(std::string("char*"), *(PropertyList::Keys().RBegin()));

  string s = "this is a string";
  pl.AddProperty("string", s);
  CPPUNIT_ASSERT(pl.HasProperty("string"));
  CPPUNIT_ASSERT_EQUAL(s, any_cast<string>(pl.PropertyValue("string")));
  CPPUNIT_ASSERT_EQUAL(s, pl.PropertyAsString("string"));
  CPPUNIT_ASSERT( Dummy::Any() == pl.PropertyValue("none"));
  CPPUNIT_ASSERT( Dummy::Any() == pl.PropertyValue(NPos()));
  CPPUNIT_ASSERT_EQUAL(string(""), pl.PropertyAsString("none"));
  CPPUNIT_ASSERT_EQUAL(std::string("string"), *PropertyList::Keys().RBegin());
  CPPUNIT_ASSERT_EQUAL((size_t)5, pl.PropertyCount());
  CPPUNIT_ASSERT_EQUAL(string("Description, int, double, char*, string"), pl.PropertyKeys());
  
  pl.RemoveProperty("char*");
  CPPUNIT_ASSERT(!pl.HasProperty("char*"));
  CPPUNIT_ASSERT_EQUAL((size_t)4, pl.PropertyCount());

  OwnedPropertyList pl2(new PropertyListImpl());
  
  CPPUNIT_ASSERT(pl2);
  CPPUNIT_ASSERT_EQUAL((size_t)0, pl2.PropertyCount());
  CPPUNIT_ASSERT_EQUAL((size_t)5, PropertyList::Keys().Size());
  CPPUNIT_ASSERT(!pl2.HasProperty("string"));

  AnyStruct strct;
  pl.AddProperty("struct", strct);
  CPPUNIT_ASSERT(pl.HasProperty("struct"));
  CPPUNIT_ASSERT_EQUAL(1, any_cast<AnyStruct>(pl.PropertyValue("struct")).i);
  CPPUNIT_ASSERT_EQUAL(1.0, any_cast<AnyStruct>(pl.PropertyValue("struct")).d);
  CPPUNIT_ASSERT_EQUAL(true, any_cast<AnyStruct>(pl.PropertyValue("struct")).b);
  AnyStruct & strct2 = *any_cast<AnyStruct>(&pl.PropertyValue("struct"));
  strct2.i = 2;
  strct2.d = 2.0;
  strct2.b = false;
  CPPUNIT_ASSERT_EQUAL(2, any_cast<AnyStruct>(pl.PropertyValue("struct")).i);
  CPPUNIT_ASSERT_EQUAL(2.0, any_cast<AnyStruct>(pl.PropertyValue("struct")).d);
  CPPUNIT_ASSERT_EQUAL(false, any_cast<AnyStruct>(pl.PropertyValue("struct")).b);
  CPPUNIT_ASSERT_EQUAL((size_t)6, PropertyList::Keys().Size());

  pl.Delete();
}

struct myInt { int m;};

void ReflexUnitTest::fundamental_type()
{
  Fundamental itype("myInt",sizeof(myInt),typeid(myInt), Catalog::Instance());
  CPPUNIT_ASSERT(itype.Is(kFundamental));
  CPPUNIT_ASSERT(!itype.Is(kClassOrStruct));
  CPPUNIT_ASSERT_EQUAL( sizeof(myInt), itype.SizeOf());
  string name;
  itype.Name(name);
  CPPUNIT_ASSERT_EQUAL( string("myInt"), name);

  Type itypet = itype;
  Object iobj = itypet.Construct();
  CPPUNIT_ASSERT( iobj.TypeOf() == itype.ThisType() );
  itype.Destruct(iobj.Address());
}

//struct MyUnknownType {};

void ReflexUnitTest::type_base() {
  {
    Fundamental f("fundamental",0,typeid(UnknownType), Catalog::Instance());
    Type ft = Type::ByName("fundamental");
    CPPUNIT_ASSERT(ft);
    Type ftt = f;
    CPPUNIT_ASSERT(ft.Id()==ftt.Id());

    Type fy = Type::ByTypeInfo(typeid(UnknownType));
    CPPUNIT_ASSERT(fy);
    Type fyy = fy;
    CPPUNIT_ASSERT(fy.Id()==fyy.Id());
  }
  Type ft = Type::ByName("fundamental"); // Type is not deleted
  CPPUNIT_ASSERT(!ft);
  Fundamental f("fundamental",0,typeid(UnknownType), Catalog::Instance());
  CPPUNIT_ASSERT(ft);
}

void ReflexUnitTest::pointer_type() {
  Fundamental f("fundamental",0,typeid(UnknownType), Catalog::Instance());
  Pointer fp(f.ThisType(),0, typeid(void*), Catalog::Instance());
  Type fpt = Type::ByName("fundamental*");
  CPPUNIT_ASSERT(fpt);
  CPPUNIT_ASSERT(fpt.Is(kPointer));
  CPPUNIT_ASSERT(!fpt.Is(kFundamental));
  CPPUNIT_ASSERT_EQUAL(string("fundamental*"), fpt.Name());
  CPPUNIT_ASSERT_EQUAL(string("fundamental*"), fpt.Name());
  CPPUNIT_ASSERT(f.ThisType() == fpt.ToType());
}

void ReflexUnitTest::reference_type() {
  Fundamental f("fundamental",0,typeid(UnknownType), Catalog::Instance());
  DataMember d("myDataMember",f.ThisType(),0,kEDReference);
  Type frt = d.TypeOf();
  CPPUNIT_ASSERT(frt);
  CPPUNIT_ASSERT(frt.Is(kReference));
  CPPUNIT_ASSERT(!frt.Is(kPointer));
  CPPUNIT_ASSERT(frt.Is(kFundamental));
  CPPUNIT_ASSERT_EQUAL(string("fundamental"), frt.Name(kNone));
  CPPUNIT_ASSERT_EQUAL(string("fundamental&"), frt.Name(kQualified));
  CPPUNIT_ASSERT(f.ThisType().Id() == d.TypeOf().Id());
}

void ReflexUnitTest::array_type() {
  Type f = Type::ByName("float");
  Array fa(f, 0, 10, typeid(float[10]), Catalog::Instance());
  Type fat = Type::ByName("float[10]");
  CPPUNIT_ASSERT(fat);
  CPPUNIT_ASSERT(fat.Is(kArray));
  CPPUNIT_ASSERT(!fat.Is(kPointer));
  CPPUNIT_ASSERT(!fat.Is(kFundamental));
  CPPUNIT_ASSERT_EQUAL(string("float[10]"), fat.Name());
  CPPUNIT_ASSERT_EQUAL(string("float[10]"), fat.Name(Reflex::kQualified));
  CPPUNIT_ASSERT(f == fat.ToType());
  CPPUNIT_ASSERT_EQUAL(10*sizeof(float), fat.SizeOf());
}


void ReflexUnitTest::typedef_type() {

  Reflex::Type t = Reflex::Type::ByName("unsigned long");
  CPPUNIT_ASSERT(t);
  CPPUNIT_ASSERT_EQUAL(t.Name(), std::string("unsigned long"));
  CPPUNIT_ASSERT_EQUAL(t.Name(Reflex::kScoped), std::string("unsigned long"));
  CPPUNIT_ASSERT_EQUAL(t.Name(Reflex::kFinal), std::string("unsigned long int"));

  //Fundamental i("int",sizeof(int),typeid(int));
  Typedef my_i("myInt", Type::ByName("int"), Catalog::Instance());
  Typedef your_i("yourInt", Type::ByName("myInt"), Catalog::Instance());
  CPPUNIT_ASSERT(my_i.Is(kTypedef));
  CPPUNIT_ASSERT(your_i.Is(kTypedef));
  string buf;
  my_i.Name(buf);
  CPPUNIT_ASSERT_EQUAL(string("myInt"), buf);
  buf.clear();
  your_i.Name(buf);
  CPPUNIT_ASSERT_EQUAL(string("yourInt"), buf);
  buf.clear();
  my_i.Name(buf,Reflex::kFinal);
  CPPUNIT_ASSERT_EQUAL(string("int"), buf);
  buf.clear();
  your_i.Name(buf, Reflex::kFinal);
  CPPUNIT_ASSERT_EQUAL(string("int"), buf);
  CPPUNIT_ASSERT_EQUAL(sizeof(int), my_i.SizeOf());
}

void ReflexUnitTest::namespace_scope() {
  {
    Namespace ns("Namespace", Catalog::Instance());
    Scope s = Scope::ByName("Namespace");
    CPPUNIT_ASSERT(s);
    CPPUNIT_ASSERT_EQUAL(kETNamespace, s.ScopeType());
    CPPUNIT_ASSERT_EQUAL(string("kNamespace"), s.ScopeTypeAsString());
    CPPUNIT_ASSERT(s.Is(kNamespace));
    CPPUNIT_ASSERT(!s.Is(kClassOrStruct));
    CPPUNIT_ASSERT(!s.IsTopScope());
    CPPUNIT_ASSERT_EQUAL(string("Namespace"), s.Name());

    Scope g = Scope::ByName("");
    CPPUNIT_ASSERT(g);
    CPPUNIT_ASSERT(g.IsTopScope());
    CPPUNIT_ASSERT(g.Is(kNamespace));
    CPPUNIT_ASSERT_EQUAL(string(""), g.Name());
    CPPUNIT_ASSERT(ns.DeclaringScope() == g);
    Namespace s1("Namespace::Subspace1", Catalog::Instance());
    Namespace s2("Namespace::Subspace2", Catalog::Instance());
    CPPUNIT_ASSERT(Scope::ByName("Namespace::Subspace1"));
    CPPUNIT_ASSERT(Scope::ByName("Namespace::Subspace2"));
    CPPUNIT_ASSERT_EQUAL(s.Id(), s1.DeclaringScope().Id());
    CPPUNIT_ASSERT_EQUAL(s.Id(), s2.DeclaringScope().Id());
    CPPUNIT_ASSERT_EQUAL(size_t(2), s.SubScopes().Size());
    CPPUNIT_ASSERT_EQUAL(size_t(1), g.SubScopes().Size());
    OrderedContainer<Scope>::const_iterator iSubScope = s.SubScopes().Begin();
    CPPUNIT_ASSERT_EQUAL(s1.ThisScope().Id(), iSubScope->Id());
    CPPUNIT_ASSERT_EQUAL(s2.ThisScope().Id(), (++iSubScope)->Id());
  }
  CPPUNIT_ASSERT(!Scope::ByName("Namespace"));
  CPPUNIT_ASSERT(!Scope::ByName("Namespace::Subspace1"));
  CPPUNIT_ASSERT(!Scope::ByName("Namespace::Subspace2"));
  Namespace s1("Namespace::Subspace1", Catalog::Instance());
  CPPUNIT_ASSERT(Scope::ByName("Namespace::Subspace1"));
}

void * s_f1( void*, const vector<void*>&, void* ) {return 0;}

void ReflexUnitTest::namespace_members() {
  Namespace n("Members", Catalog::Instance());
  CPPUNIT_ASSERT_EQUAL(size_t(0), n.Members().Size());

  Type i = Type::ByName("int");
  Type f = Type::ByName("float");
  Member m1 = Member(new DataMember("d1",i,0));
  Member m2 = Member(new DataMember("d2",f,4));
  n.AddMember(m1);
  n.AddMember(m2);
  CPPUNIT_ASSERT_EQUAL(size_t(2), n.Members().Size());
  CPPUNIT_ASSERT_EQUAL(size_t(2), n.DataMembers().Size());
  OrderedContainer<Member>::const_iterator iDB = n.ThisScope().DataMembers().Begin();
  CPPUNIT_ASSERT_EQUAL(string("Members::d1"), iDB->Name());
  CPPUNIT_ASSERT_EQUAL(string("d1"), iDB->Name(kNone));
  CPPUNIT_ASSERT_EQUAL(string("Members::d2"), (++iDB)->Name());
  CPPUNIT_ASSERT_EQUAL(string("d2"), (++iDB)->Name(kNone));

  vector<Type> params;
  params.push_back(i);
  params.push_back(f);
  Function fun(f, params, typeid(void (void)), Catalog::Instance());
  string buf;
  fun.Name(buf);
  CPPUNIT_ASSERT_EQUAL(string("float (int, float)"), buf);

  Member m3 = Member(new FunctionMember("f1",fun.ThisType(),s_f1,0,"i;f=99.9"));
  Member m4 = Member(new FunctionMember("f2",fun.ThisType(),s_f1,0,"j=10"));
  n.AddMember(m3);
  n.AddMember(m4);
  CPPUNIT_ASSERT_EQUAL(size_t(2), n.FunctionMembers().Size());
  OrderedContainer<Member>::const_iterator iFM = n.ThisScope().FunctionMembers().Begin();
  CPPUNIT_ASSERT_EQUAL(string("Members::f1"), iFM->Name());
  CPPUNIT_ASSERT_EQUAL(string("Members::f2"), (++iFM)->Name());

  CPPUNIT_ASSERT_EQUAL(size_t(4), n.Members().Size());
  Container<Member>::const_iterator iM = n.ThisScope().Members().Begin();
  CPPUNIT_ASSERT_EQUAL(string("Members::d1"), iM->Name());
  CPPUNIT_ASSERT_EQUAL(string("Members::d2"), (++iM)->Name());
  CPPUNIT_ASSERT_EQUAL(string("Members::f1"), (++iM)->Name());
  CPPUNIT_ASSERT_EQUAL(string("Members::f2"), (++iM)->Name());
}

void ReflexUnitTest::function_member() {
  vector<Type> params;
  params.push_back(Type::ByName("int"));
  params.push_back(Type::ByName("double"));
  params.push_back(Type::ByName("bool"));
  Function fun(Type::ByName("float"),params,typeid(void (void)), Catalog::Instance());
  OwnedMember fun1 (new FunctionMember("fun1",fun.ThisType(),0,0,"arg1;arg2;arg3",kEDStatic));
  OwnedMember fun2 (new FunctionMember("fun2",fun.ThisType(),0,0,"arg1;arg2=99.9;arg3=true"));
  OwnedMember fun3 (new FunctionMember("fun3",fun.ThisType(),0,0,"arg1;arg2"));
  CPPUNIT_ASSERT_EQUAL(size_t(3), fun1.FunctionParameterSize() );
  OrderedContainer<std::string>::const_iterator iParName = fun1.FunctionParameterNames().Begin();
  CPPUNIT_ASSERT_EQUAL(string("arg1"), *iParName );
  CPPUNIT_ASSERT_EQUAL(string("arg2"), *(++iParName) );
  CPPUNIT_ASSERT_EQUAL(string("arg3"), *(++iParName) );
  OrderedContainer<std::string>::const_iterator iParDef = fun2.FunctionParameterDefaults().Begin();
  CPPUNIT_ASSERT_EQUAL(string(""),     *iParDef );
  CPPUNIT_ASSERT_EQUAL(string("99.9"), *(++iParDef) );
  CPPUNIT_ASSERT_EQUAL(string("true"), *(++iParDef) );
  OrderedContainer<std::string>::const_iterator iParName3 = fun3.FunctionParameterNames().Begin();
  CPPUNIT_ASSERT_EQUAL(string("arg1"), *iParName3 );
  CPPUNIT_ASSERT_EQUAL(string("arg2"), *(++iParName3) );
  CPPUNIT_ASSERT_EQUAL(string(""),     *(++iParName3) );
  fun1.Delete();
  fun2.Delete();
  fun3.Delete();
}


void ReflexUnitTest::object_basics() {
  Type i = Type::ByName("int");
  Object o1 = i.Construct();
  Object o2 = o1;
  CPPUNIT_ASSERT(o1 == o2);
  o1.Destruct();
}

void ReflexUnitTest::class_type() {
  {
    Class _foo("foo", sizeof(foo), typeid(foo), Catalog::Instance());
    Class _bar("bar", sizeof(bar), typeid(bar), Catalog::Instance());
    CPPUNIT_ASSERT(Type::ByName("foo"));
    CPPUNIT_ASSERT(_foo.ThisType() == Type::ByName("foo"));
    CPPUNIT_ASSERT(_foo.ThisType() == Type::ByName("::foo"));
    CPPUNIT_ASSERT(_foo.ThisType() == Type::ByTypeInfo(typeid(foo)));
    CPPUNIT_ASSERT(_bar.ThisType() == Type::ByName("bar"));
    CPPUNIT_ASSERT(_bar.ThisType() == Type::ByName("::bar"));
    CPPUNIT_ASSERT(_bar.ThisType() == Type::ByTypeInfo(typeid(bar)));
  }
  CPPUNIT_ASSERT(!Type::ByName("foo"));
  CPPUNIT_ASSERT(Type::ByName("foo").Id());
  CPPUNIT_ASSERT(!Type::ByTypeInfo(typeid(foo)));
  CPPUNIT_ASSERT(!Type::ByName("bar"));
  CPPUNIT_ASSERT(Type::ByName("bar").Id());
  CPPUNIT_ASSERT(!Type::ByTypeInfo(typeid(bar)));
}


void ReflexUnitTest::type_template() {
  Namespace _n("mmm", Catalog::Instance());
  ClassTemplateInstance _cti("mmm::template<int,float,dummy>",
                             1,
                             typeid(UnknownType),
                             Catalog::Instance(),
                             0);

  Scope s = Scope::ByName("mmm");

  CPPUNIT_ASSERT_EQUAL(1, (int)s.SubTypeTemplates().Size());
  
  TypeTemplate tt = *s.SubTypeTemplates().Begin();
  
  CPPUNIT_ASSERT(tt == true);
  CPPUNIT_ASSERT(tt.Name()       == "template");
  CPPUNIT_ASSERT(tt.Name(Reflex::kScoped) == "mmm::template");
  
  CPPUNIT_ASSERT_EQUAL(1, (int)tt.TemplateInstances().Size());
  
  Type t1 = *tt.TemplateInstances().Begin(); 
  CPPUNIT_ASSERT( t1 );
  CPPUNIT_ASSERT_EQUAL(std::string("mmm::template<int,float,dummy>"), t1.Name(Reflex::kScoped|kQualified|Reflex::kFinal));
  CPPUNIT_ASSERT( t1.Is(kClassOrStruct) );
  CPPUNIT_ASSERT( t1.Is(kTemplateInstance) );

  Type c1 = Type::ByName("mmm::template<int,float,dummy>");
  CPPUNIT_ASSERT( c1 );
  CPPUNIT_ASSERT(c1 == t1);
  CPPUNIT_ASSERT_EQUAL(std::string("mmm::template<int,float,dummy>"), c1.Name(Reflex::kScoped|kQualified|Reflex::kFinal));
  CPPUNIT_ASSERT_EQUAL(std::string("template<int,float,dummy>"), c1.Name());
  CPPUNIT_ASSERT( c1.Is(kClassOrStruct) );
  CPPUNIT_ASSERT( c1.Is(kTemplateInstance) );

  Scope s1 = Scope::ByName("mmm::template<int,float,dummy>");
  CPPUNIT_ASSERT( s1 );
  CPPUNIT_ASSERT_EQUAL(std::string("mmm::template<int,float,dummy>"), s1.Name(Reflex::kScoped|kQualified|Reflex::kFinal));
  CPPUNIT_ASSERT_EQUAL(std::string("template<int,float,dummy>"), s1.Name());
  CPPUNIT_ASSERT( s1.Is(kClassOrStruct) );
  CPPUNIT_ASSERT( s1.Is(kTemplateInstance) );
  
  CPPUNIT_ASSERT_EQUAL( 3, (int)c1.TemplateArguments().Size() );
  Container<TemplateArgument>::const_iterator iTA = c1.TemplateArguments().Begin();
  CPPUNIT_ASSERT( *iTA );
  CPPUNIT_ASSERT( *(++iTA) );
  CPPUNIT_ASSERT( ! *(++iTA) );

  iTA = c1.TemplateArguments().Begin();
  CPPUNIT_ASSERT_EQUAL( string("int"), iTA->Name() );
  CPPUNIT_ASSERT_EQUAL( string("float"), (++iTA)->Name() );
  CPPUNIT_ASSERT_EQUAL( string("dummy"), (++iTA)->Name() );
 
}


void ReflexUnitTest::member() {

  Member m1 = Member(0);
  CPPUNIT_ASSERT( ! m1 );

  Member m2 = Member();
  CPPUNIT_ASSERT( ! m2 );

}


void ReflexUnitTest::tools() {

  std::string t0 = "NS1::NS2::Type";
  CPPUNIT_ASSERT_EQUAL(size_t(10), Tools::GetBasePosition(t0));
  CPPUNIT_ASSERT_EQUAL(std::string("NS1::NS2"), Tools::GetScopeName(t0));
  CPPUNIT_ASSERT_EQUAL(std::string("Type"), Tools::GetBaseName(t0));
  CPPUNIT_ASSERT_EQUAL(std::string("NS1"), Tools::GetScopeName(t0,true));
  CPPUNIT_ASSERT_EQUAL(std::string("NS2::Type"), Tools::GetBaseName(t0,true));

  std::string t1 = "std::vector<int>";
  CPPUNIT_ASSERT_EQUAL(size_t(5), Tools::GetBasePosition(t1));
  CPPUNIT_ASSERT_EQUAL(std::string("std"), Tools::GetScopeName(t1));
  CPPUNIT_ASSERT_EQUAL(std::string("vector<int>"), Tools::GetBaseName(t1));

  std::string t2 = "std::vector<int>::iterator";
  CPPUNIT_ASSERT_EQUAL(size_t(18), Tools::GetBasePosition(t2));
  CPPUNIT_ASSERT_EQUAL(std::string("std::vector<int>"), Tools::GetScopeName(t2));
  CPPUNIT_ASSERT_EQUAL(std::string("iterator"), Tools::GetBaseName(t2));

  std::string t3 = "std::vector<A::B::C>";
  CPPUNIT_ASSERT_EQUAL(size_t(5), Tools::GetBasePosition(t3));
  CPPUNIT_ASSERT_EQUAL(std::string("std"), Tools::GetScopeName(t3));
  CPPUNIT_ASSERT_EQUAL(std::string("vector<A::B::C>"), Tools::GetBaseName(t3));

  std::string t4 = "std::vector<A::B::C>::iterator";
  CPPUNIT_ASSERT_EQUAL(size_t(22), Tools::GetBasePosition(t4));
  CPPUNIT_ASSERT_EQUAL(std::string("std::vector<A::B::C>"), Tools::GetScopeName(t4));
  CPPUNIT_ASSERT_EQUAL(std::string("iterator"), Tools::GetBaseName(t4));

  std::string t5 = "std::vector<void (*) (int)>";
  CPPUNIT_ASSERT_EQUAL(size_t(5), Tools::GetBasePosition(t5));
  CPPUNIT_ASSERT_EQUAL(std::string("std"), Tools::GetScopeName(t5));
  CPPUNIT_ASSERT_EQUAL(std::string("vector<void (*) (int)>"), Tools::GetBaseName(t5));

  std::string t6 = "std::vector<void (*) (int)>::iterator";
  CPPUNIT_ASSERT_EQUAL(size_t(29), Tools::GetBasePosition(t6));
  CPPUNIT_ASSERT_EQUAL(std::string("std::vector<void (*) (int)>"), Tools::GetScopeName(t6));
  CPPUNIT_ASSERT_EQUAL(std::string("iterator"), Tools::GetBaseName(t6));

  std::string t7 = "std::reverse_iterator<std::_Rb_tree_const_iterator<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::pair<int, std::vector<float, std::allocator<float> > > > > >";
  CPPUNIT_ASSERT_EQUAL(std::string("std"), Tools::GetScopeName(t7));
  CPPUNIT_ASSERT_EQUAL(std::string("reverse_iterator<std::_Rb_tree_const_iterator<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::pair<int, std::vector<float, std::allocator<float> > > > > >"), Tools::GetBaseName(t7));
  CPPUNIT_ASSERT_EQUAL(std::string("std"), Tools::GetScopeName(t7,true));
  CPPUNIT_ASSERT_EQUAL(std::string("reverse_iterator<std::_Rb_tree_const_iterator<std::pair<std::basic_string<char, std::char_traits<char>, std::allocator<char> > const, std::pair<int, std::vector<float, std::allocator<float> > > > > >"), Tools::GetBaseName(t7,true));
}


void ReflexUnitTest::setClassAtts() {

   Type t = Type::ByName("int");
   CPPUNIT_ASSERT(t);
   CPPUNIT_ASSERT_EQUAL(size_t(sizeof(int)), t.SizeOf());
   CPPUNIT_ASSERT_EQUAL(std::string(typeid(int).name()), std::string(t.TypeInfo().name()));

   t.SetSize(99);
   t.SetTypeInfo(typeid(void));

   CPPUNIT_ASSERT_EQUAL(size_t(99), t.SizeOf());
   CPPUNIT_ASSERT_EQUAL(std::string(typeid(void).name()), std::string(t.TypeInfo().name()));

   t.SetSize(sizeof(int));
   t.SetTypeInfo(typeid(int));

   CPPUNIT_ASSERT_EQUAL(size_t(sizeof(int)), t.SizeOf());
   CPPUNIT_ASSERT_EQUAL(std::string(typeid(int).name()), std::string(t.TypeInfo().name()));
}


void ReflexUnitTest::global_scope() {

   Scope g = Scope::ByName("");
   CPPUNIT_ASSERT(g);
   CPPUNIT_ASSERT_EQUAL((size_t)0, g.SubScopeLevel());
   
   Scope g2 = Scope::GlobalScope();
   CPPUNIT_ASSERT(g2);
   CPPUNIT_ASSERT_EQUAL((size_t)0, g2.SubScopeLevel());

   CPPUNIT_ASSERT_EQUAL(g.Id(), g2.Id());
   CPPUNIT_ASSERT_EQUAL(g.Name(), g2.Name());

}

void ReflexUnitTest::object_value() {
  //---Primitives
  bool  t = true;   bool b = false; 
  char  c = -99;    unsigned char uc = 99;
  short s = -999;   unsigned short us = 999;
  int   i = -9999;  unsigned int ui = 9999;
  long  l = -99999; unsigned long ul = 99999;
  float f = 1.12F;  double d = 1.12; long double ld = 1.12;

  ValueObject vt(t), vb(b), vc(c), vuc(uc), vs(s), vus(us), vi(i), vui(ui), vl(l), vul(ul), vf(f), vd(d), vld(ld);
  CPPUNIT_ASSERT( t  == vt.Value<bool>() );
  CPPUNIT_ASSERT( b  == vb.Value<bool>() );
  CPPUNIT_ASSERT( c  == vc.Value<char>() );
  CPPUNIT_ASSERT( uc == vuc.Value<unsigned char>() );
  CPPUNIT_ASSERT( i  == vi.Value<int>() );
  CPPUNIT_ASSERT( ui == vui.Value<unsigned int>() );
  CPPUNIT_ASSERT( l  == vl.Value<long>() );
  CPPUNIT_ASSERT( ul == vul.Value<unsigned long>() );
  CPPUNIT_ASSERT( f  == vf.Value<float>() );
  CPPUNIT_ASSERT( d  == vd.Value<double>() );
  CPPUNIT_ASSERT( ld == vld.Value<long double>() );


  //---Pointers
  char*  pc = "Hello";
  char** ppc = &pc;
  string* ps = new string(pc);
  const int* cpi = &i;
  int const * pci = &i;
  int * const ipc = &i;
  ValueObject vpc(pc), vppc(ppc), vps(ps), vcpi(cpi), vpci(pci), vipc(ipc);

  CPPUNIT_ASSERT( pc  == vpc.Address() );
  CPPUNIT_ASSERT( ppc  == vppc.Address() );
  CPPUNIT_ASSERT( ps  == vps.Address() );
  CPPUNIT_ASSERT( cpi  == vcpi.Address() );
  CPPUNIT_ASSERT( pci  == vpci.Address() );
  CPPUNIT_ASSERT( ipc  == vipc.Address() );

  //---Classes and structs
  string str("Hello");
  ValueObject vstr(str);
  CPPUNIT_ASSERT( str  == vstr.Value<string>() );

  //---Copies
  ValueObject vvi = vi;
  CPPUNIT_ASSERT( i  == vvi.Value<int>() );
  ValueObject vvstr;
  vvstr = vstr;
  CPPUNIT_ASSERT( str  == vvstr.Value<string>() );
  vvi = 99;
  CPPUNIT_ASSERT( 99  == vvi.Value<int>() );
  vvi = vstr;
  CPPUNIT_ASSERT( str  == vvi.Value<string>() );

}


void ReflexUnitTest::sharedlibrary() {

   SharedLibrary sl("doesntexist.so");
   CPPUNIT_ASSERT(! sl.Load());
   std::string errstr = sl.Error();
   CPPUNIT_ASSERT(! errstr.empty());
#if defined(_WIN32)
   CPPUNIT_ASSERT(true); // fixme
#elif defined(__APPLE__)
   CPPUNIT_ASSERT( errstr.find("not found") != std::string::npos);
#elif defined(__linux)
   CPPUNIT_ASSERT( errstr.find("No such file or directory") != std::string::npos);
#endif

}


// Class registration on cppunit framework
CPPUNIT_TEST_SUITE_REGISTRATION(ReflexUnitTest);

// CppUnit test-driver common for all the cppunit test classes 
#include<CppUnit_testdriver.cpp>

