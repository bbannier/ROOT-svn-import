// @(#)root/reflex:$Id$
// Author: Philippe Bourgau 2009

// CppUnit include file
#include <cppunit/extensions/HelperMacros.h>
#include <Reflex/SharedLibrary.h>
#include <Reflex/Type.h>
#include <Reflex/Member.h>
#include <Reflex/Scope.h>
#include <Reflex/SharedLibraryUtils.h>
#include <Reflex/Builder/DictionaryBuilder.h>
#include "testLibCommon/BaseLibrary.h"

using namespace std;
using namespace Reflex;


/** 
* Tests of the versioning capabilities of Reflex, (Names, Dictionary and DictionaryOverrider classes)
*/

class ReflexVersioningTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE( ReflexVersioningTest );

  CPPUNIT_TEST( types_for_two_versions_of_the_same_class_should_be_different );
  CPPUNIT_TEST( builtin_types_from_different_dictionaries_should_be_identical );
  CPPUNIT_TEST( reloading_a_library_should_use_the_existing_types );
  CPPUNIT_TEST( a_shared_dependency_type_should_be_the_same_from_two_dictionaries );
  CPPUNIT_TEST( types_should_be_registered_in_the_dictionary_of_their_library );
  CPPUNIT_TEST( types_from_members_should_be_resolved_in_the_correct_dictionary );
  CPPUNIT_TEST( types_should_be_absent_from_dependencies_dictionaries );

  CPPUNIT_TEST( current_dictionary_should_be_resolved_in_place );
  CPPUNIT_TEST( exe_dictionary_should_be_available_even_in_a_shared_library_without_reflex_info );

  CPPUNIT_TEST( types_should_not_leak_in_shared_namespaces );
  CPPUNIT_TEST( shared_types_should_not_be_accessible_from_overriding_namespaces );
  CPPUNIT_TEST( scopes_should_not_leak_in_shared_namespaces );
  CPPUNIT_TEST( shared_scopes_should_not_be_accessible_from_overriding_namespaces );
  CPPUNIT_TEST( members_should_not_leak_in_shared_namespaces );
  CPPUNIT_TEST( shared_members_should_not_be_accessible_from_overriding_namespaces );

  CPPUNIT_TEST( global_scope_should_not_be_his_own_parent );
  CPPUNIT_TEST( the_blank_named_scope_should_be_the_global_scope );
  CPPUNIT_TEST( different_versions_of_the_same_type_should_not_conflict_in_global_scope );

  CPPUNIT_TEST( test_current_shared_library);
  CPPUNIT_TEST( test_shared_library_dependencies);

  CPPUNIT_TEST( type_infos_for_the_same_type_should_always_be_the_same );
  CPPUNIT_TEST( type_infos_from_different_versions_should_be_different_pointers );

  CPPUNIT_TEST( searching_types_by_typeid_should_not_require_a_dictionary );
  CPPUNIT_TEST( typeid_for_classes_deriving_from_multiple_derived_classes_from_another_library_should_resolve_reflex_type );

  CPPUNIT_TEST_SUITE_END();
public:

  void setUp();
  void tearDown();

  void types_for_two_versions_of_the_same_class_should_be_different();
  void builtin_types_from_different_dictionaries_should_be_identical();
  void reloading_a_library_should_use_the_existing_types();
  void a_shared_dependency_type_should_be_the_same_from_two_dictionaries();
  void types_should_be_registered_in_the_dictionary_of_their_library();
  void types_from_members_should_be_resolved_in_the_correct_dictionary();
  void types_should_be_absent_from_dependencies_dictionaries();
  void types_should_not_leak_in_shared_namespaces();

  void current_dictionary_should_be_resolved_in_place();
  void exe_dictionary_should_be_available_even_in_a_shared_library_without_reflex_info();

  void shared_types_should_not_be_accessible_from_overriding_namespaces();
  void scopes_should_not_leak_in_shared_namespaces();
  void shared_scopes_should_not_be_accessible_from_overriding_namespaces();
  void members_should_not_leak_in_shared_namespaces();
  void shared_members_should_not_be_accessible_from_overriding_namespaces();

  void global_scope_should_not_be_his_own_parent();
  void the_blank_named_scope_should_be_the_global_scope();
  void different_versions_of_the_same_type_should_not_conflict_in_global_scope();

  void test_current_shared_library();
  void test_shared_library_dependencies();

  void type_infos_for_the_same_type_should_always_be_the_same();
  void type_infos_from_different_versions_should_be_different_pointers();

  void searching_types_by_typeid_should_not_require_a_dictionary();
  void typeid_for_classes_deriving_from_multiple_derived_classes_from_another_library_should_resolve_reflex_type();

public:

   static void classSetUp();

   static string sharedLibraryFileName(const string& libName);
   static Dictionary loadLibDictionary(const string& libName);

private:

   static const std::string sLibCommon;
   static const std::string sLibV1;
   static const std::string sLibV2;

   static Dictionary sLibCommonDico;
   static Dictionary sLibV1Dico;
   static Dictionary sLibV2Dico;

   Type fLibraryV1Class;
   Type fLibraryV2Class;

   Scope fSharedNamespaceCommon;
   Scope fSharedNamespaceV1;

};

Dictionary ReflexVersioningTest::sLibCommonDico;
Dictionary ReflexVersioningTest::sLibV1Dico;
Dictionary ReflexVersioningTest::sLibV2Dico;
const std::string ReflexVersioningTest::sLibCommon(ReflexVersioningTest::sharedLibraryFileName("test_LibCommon"));
const std::string ReflexVersioningTest::sLibV1(ReflexVersioningTest::sharedLibraryFileName("test_LibV1"));
const std::string ReflexVersioningTest::sLibV2(ReflexVersioningTest::sharedLibraryFileName("test_LibV2"));

string ReflexVersioningTest::sharedLibraryFileName(const string& libName) {

#ifdef WIN32
   return "lib"+libName+".dll";
#else
   return "lib"+libName+".so";
#endif
}

Dictionary ReflexVersioningTest::loadLibDictionary(const string& libraryFileName) {

   std::string libsDir = Reflex::DirectoryPartOfPath(Reflex::SharedLibraryDefining((void*)&ReflexVersioningTest::loadLibDictionary));

   SharedLibrary lib(libsDir+libraryFileName);
   if (!lib.Load())
      CPPUNIT_FAIL(lib.Error());

   return Dictionary::BySharedLibrary(libraryFileName);
}

void ReflexVersioningTest::setUp() {

   classSetUp();

   fLibraryV1Class = Type::ByName("Library", sLibV1Dico);
   CPPUNIT_ASSERT(fLibraryV1Class);

   fLibraryV2Class = Type::ByName("Library", sLibV2Dico);
   CPPUNIT_ASSERT(fLibraryV2Class);

   fSharedNamespaceCommon = Scope::ByName("SharedNamespace", sLibCommonDico);
   CPPUNIT_ASSERT(fSharedNamespaceCommon);

   fSharedNamespaceV1 = Scope::ByName("SharedNamespace", sLibV1Dico);
   CPPUNIT_ASSERT(fSharedNamespaceV1);
}

void ReflexVersioningTest::tearDown() {
   // TODO do something for unload
}

void ReflexVersioningTest::classSetUp() {

   static bool already_run = false;
   if (already_run) return;
   else             already_run = true;

   sLibCommonDico = loadLibDictionary(sLibCommon);
   sLibV1Dico = loadLibDictionary(sLibV1);
   sLibV2Dico = loadLibDictionary(sLibV2);
}

void ReflexVersioningTest::current_dictionary_should_be_resolved_in_place() {

   Reflex::Object libObject = fLibraryV1Class.Construct();
   BaseLibrary* libInstance = static_cast<BaseLibrary*>(libObject.Address());
   CPPUNIT_ASSERT(libInstance);
   CPPUNIT_ASSERT_EQUAL(sLibV1Dico, libInstance->Dictionary());

   libObject.Destruct();
}

void ReflexVersioningTest::exe_dictionary_should_be_available_even_in_a_shared_library_without_reflex_info() {

   CPPUNIT_ASSERT(Reflex::Dictionary::Main() != Reflex::DictionaryExe());
}

void ReflexVersioningTest::types_for_two_versions_of_the_same_class_should_be_different() {

   CPPUNIT_ASSERT(fLibraryV1Class != fLibraryV2Class);
}

void ReflexVersioningTest::builtin_types_from_different_dictionaries_should_be_identical() {

   Type doubleV1 = Type::ByName("double", sLibV1Dico);
   CPPUNIT_ASSERT(doubleV1);

   Type doubleV2 = Type::ByName("double", sLibV2Dico);
   CPPUNIT_ASSERT(doubleV2);

   CPPUNIT_ASSERT(doubleV1 == doubleV2);
}

void ReflexVersioningTest::reloading_a_library_should_use_the_existing_types() {

   Dictionary libDico = loadLibDictionary(sLibV1);
   Type libraryClass = Type::ByName("Library", libDico);
   CPPUNIT_ASSERT(libraryClass);

   CPPUNIT_ASSERT(fLibraryV1Class == libraryClass);
}

void ReflexVersioningTest::a_shared_dependency_type_should_be_the_same_from_two_dictionaries() {

   Type baseLibraryV1Class = Type::ByName("BaseLibrary", sLibV1Dico);
   CPPUNIT_ASSERT(baseLibraryV1Class);

   Type baseLibraryV2Class = Type::ByName("BaseLibrary", sLibV2Dico);
   CPPUNIT_ASSERT(baseLibraryV2Class);

   CPPUNIT_ASSERT(baseLibraryV1Class == baseLibraryV2Class);
}

void ReflexVersioningTest::types_should_be_registered_in_the_dictionary_of_their_library() {

   CPPUNIT_ASSERT_EQUAL(sLibV1 ,fLibraryV1Class.DictionaryGet().SharedLibraryName());
   CPPUNIT_ASSERT_EQUAL(sLibV2 ,fLibraryV2Class.DictionaryGet().SharedLibraryName());
}


void ReflexVersioningTest::types_from_members_should_be_resolved_in_the_correct_dictionary() {

   Type valueClass = Type::ByName("Value",sLibV1Dico);

   Type valueClassV1 = fLibraryV1Class.FunctionMemberByName("smallName").TypeOf().ReturnType();
   CPPUNIT_ASSERT(valueClassV1);
   CPPUNIT_ASSERT_EQUAL(sLibV1 ,valueClassV1.DictionaryGet().SharedLibraryName());

   Type valueClassV2 = fLibraryV2Class.FunctionMemberByName("smallName").TypeOf().ReturnType();
   CPPUNIT_ASSERT(valueClassV2);
   CPPUNIT_ASSERT_EQUAL(sLibV2 ,valueClassV2.DictionaryGet().SharedLibraryName());

   CPPUNIT_ASSERT(valueClassV1 != valueClassV2);
}

void ReflexVersioningTest::types_should_be_absent_from_dependencies_dictionaries() {

   CPPUNIT_ASSERT(Type::ByName("SharedNamespace::Zombie", sLibV1Dico));
   CPPUNIT_ASSERT(!Type::ByName("SharedNamespace::Zombie", sLibCommonDico));
}


void ReflexVersioningTest::types_should_not_leak_in_shared_namespaces() {

   Type zombie = fSharedNamespaceCommon.SubTypeByName("Zombie");
   CPPUNIT_ASSERT(!zombie);
}

void ReflexVersioningTest::shared_types_should_not_be_accessible_from_overriding_namespaces() {

   Type dummy = fSharedNamespaceV1.SubTypeByName("Dummy");
   CPPUNIT_ASSERT(!dummy);
}


void ReflexVersioningTest::scopes_should_not_leak_in_shared_namespaces() {

   Scope zombie = fSharedNamespaceCommon.SubScopeByName("Zombie");
   CPPUNIT_ASSERT(!zombie);
}

void ReflexVersioningTest::shared_scopes_should_not_be_accessible_from_overriding_namespaces() {

   Scope dummy = fSharedNamespaceV1.SubScopeByName("Dummy");
   CPPUNIT_ASSERT(!dummy);
}


void ReflexVersioningTest::members_should_not_leak_in_shared_namespaces() {

   Member zombie = fSharedNamespaceCommon.MemberByName("NewZombie");
   CPPUNIT_ASSERT(!zombie);
}
void ReflexVersioningTest::shared_members_should_not_be_accessible_from_overriding_namespaces() {

   Member dummy = fSharedNamespaceV1.MemberByName("Dummy");
   CPPUNIT_ASSERT(!dummy);
}


void ReflexVersioningTest::global_scope_should_not_be_his_own_parent() {
   Scope globalScope = Scope::GlobalScope();
   CPPUNIT_ASSERT(globalScope);

   Scope nirvana = globalScope.DeclaringScope();

   CPPUNIT_ASSERT(nirvana != globalScope);
}

void ReflexVersioningTest::the_blank_named_scope_should_be_the_global_scope() {
   Scope globalScope = Scope::GlobalScope(sLibV1Dico);
   CPPUNIT_ASSERT(globalScope);

   Scope blank_named_scope = Scope::ByName("", sLibV1Dico);
   CPPUNIT_ASSERT(blank_named_scope);

   CPPUNIT_ASSERT(blank_named_scope == globalScope);
}

namespace {
   size_t TopLevelLibraryTypeCount(const Reflex::Dictionary& dictionary) {
      Scope globalScope = Scope::GlobalScope(dictionary);
      size_t result = 0;
      for(Reflex::Type_Iterator i = globalScope.SubType_Begin(); i != globalScope.SubType_End(); ++i) {
         if ("Library" == i->Name()) {
            ++result;
         }
      }
      return result;
   }
}

void ReflexVersioningTest::different_versions_of_the_same_type_should_not_conflict_in_global_scope() {

   CPPUNIT_ASSERT(0 == TopLevelLibraryTypeCount(sLibCommonDico));
   CPPUNIT_ASSERT(1 == TopLevelLibraryTypeCount(sLibV1Dico));
   CPPUNIT_ASSERT(1 == TopLevelLibraryTypeCount(sLibV2Dico));
}

namespace
{
#ifdef WIN32
   std::string libName(const std::string& baseName) { return baseName+".dll"; }
   std::string exeName(const std::string& baseName) { return baseName+".exe"; }
#else
   std::string libName(const std::string& baseName) { return "lib"+baseName+".so"; }
   std::string exeName(const std::string& baseName) { return baseName; }
#endif

   std::string CurrentSharedLibraryPath()
   {
      return Reflex::SharedLibraryDefining((void*)&ReflexVersioningTest::classSetUp);
   }
}

void ReflexVersioningTest::test_current_shared_library()
{
   CPPUNIT_ASSERT_EQUAL(std::string(exeName("test_Reflex_versioning")), Reflex::NamePartOfPath(CurrentSharedLibraryPath()));
}

void ReflexVersioningTest::test_shared_library_dependencies()
{
   std::vector<std::string> dependencies;
   CollectSharedLibraryDependencies(CurrentSharedLibraryPath(), dependencies);
   CPPUNIT_ASSERT( 1 <= dependencies.size());
   CPPUNIT_ASSERT(dependencies.end() != std::find(dependencies.begin(), dependencies.end(), libName("Reflex")));
}

namespace
{
   const std::type_info& typeIdOfLibraryClass(const Reflex::Type& libraryClass)
   {
      Reflex::Object aLib = libraryClass.Construct();

      std::type_info* result = NULL;
      aLib.Invoke<std::type_info*>("typeId", result);

      return *result;
   }
}

void ReflexVersioningTest::type_infos_for_the_same_type_should_always_be_the_same()
{
   const char* a_type_info = typeIdOfLibraryClass(fLibraryV1Class).name();
   const char* another_type_info = typeIdOfLibraryClass(fLibraryV1Class).name();

   CPPUNIT_ASSERT(a_type_info == another_type_info);
}

void ReflexVersioningTest::type_infos_from_different_versions_should_be_different_pointers()
{
   const char* type_info_v1 = typeIdOfLibraryClass(fLibraryV1Class).name();
   const char* type_info_v2 = typeIdOfLibraryClass(fLibraryV2Class).name();

   CPPUNIT_ASSERT(type_info_v1 != type_info_v2);
}

void ReflexVersioningTest::searching_types_by_typeid_should_not_require_a_dictionary()
{
   const std::type_info& type_info_v1 = typeIdOfLibraryClass(fLibraryV1Class);
   Reflex::Type libV1Class = Reflex::Type::ByTypeInfo(type_info_v1);
   void* byTypeName = (void*)&Reflex::Type::ByName;
   CPPUNIT_ASSERT(fLibraryV1Class == libV1Class);

   const std::type_info& type_info_v2 = typeIdOfLibraryClass(fLibraryV2Class);
   Reflex::Type libV2Class = Reflex::Type::ByTypeInfo(type_info_v2);
   CPPUNIT_ASSERT(fLibraryV2Class == libV2Class);
}

void ReflexVersioningTest::typeid_for_classes_deriving_from_multiple_derived_classes_from_another_library_should_resolve_reflex_type()
{
   Reflex::Type d_class = Reflex::Type::ByName("D", sLibV1Dico);
   CPPUNIT_ASSERT(d_class);

   Reflex::Object d = d_class.Construct();

   C* c = (C*)d.Address();
   Reflex::Type resolved_d_class = Reflex::Type::ByTypeInfo(typeid(*c));
   CPPUNIT_ASSERT(resolved_d_class);
   CPPUNIT_ASSERT(d_class == resolved_d_class);

   d.Destruct();
}


// Class registration on cppunit framework
CPPUNIT_TEST_SUITE_REGISTRATION(ReflexVersioningTest);

// CppUnit test-driver common for all the cppunit test classes 
#include <CppUnit_testdriver.cpp>

