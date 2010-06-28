// Check for annotations

#include "util/HelperMacros.hpp"
#include "Reflex/Type.h"
#include "Reflex/Member.h"
#include "Reflex/AnnotationList.h"

#include "Annots.hpp"

using namespace Reflex;


REFLEX_TEST(given_a_type_with_annotations_when_genreflex_builds_it_then_annotations_should_be_accessible_from_reflex_runtime)
{
   Type modelClass = Type::ByTypeInfo(typeid(ModelClass));
   CPPUNIT_ASSERT(modelClass);

   AnnotationList annotations = modelClass.Annotations();
   CPPUNIT_ASSERT(annotations);

   const Credits* credits = annotations.AnnotationWithType<Credits>();
   CPPUNIT_ASSERT(credits);
}

REFLEX_TEST(given_an_annotation_with_initialized_fields_when_genreflex_builds_it_then_initialized_fields_should_be_initialized_by_reflex_runtime)
{
   Type modelClass = Type::ByTypeInfo(typeid(ModelClass));

   CPPUNIT_ASSERT_EQUAL(1U, modelClass.Annotations().AnnotationWithType<MetaModel>()->getRevision());
   CPPUNIT_ASSERT_EQUAL(std::string("Philippe Bourgau"), modelClass.Annotations().AnnotationWithType<Credits>()->getAuthor());
   CPPUNIT_ASSERT_EQUAL(std::string("0.9"), modelClass.Annotations().AnnotationWithType<Credits>()->getVersion()->getMajor());
}

REFLEX_TEST(given_an_annotation_with_list_fields_when_genreflex_builds_it_then_the_list_should_be_accessible_from_reflex_runtime)
{
   Type modelClass = Type::ByTypeInfo(typeid(ModelClass));

   std::vector<std::string> reviewers = modelClass.Annotations().AnnotationWithType<Credits>()->getReviewers();
   CPPUNIT_ASSERT(2U == reviewers.size());
   CPPUNIT_ASSERT_EQUAL(std::string("Me"), reviewers[0]);
   CPPUNIT_ASSERT_EQUAL(std::string("You"), reviewers[1]);

   std::vector<const Field*> fields = modelClass.Annotations().AnnotationWithType<MetaModel>()->getFields();
   CPPUNIT_ASSERT(1U == fields.size());
   CPPUNIT_ASSERT_EQUAL(std::string("Surprise"), fields[0]->getName());
}

REFLEX_TEST(given_a_member_with_annotations_when_genreflex_builds_it_then_annotations_should_be_accessible_from_reflex_runtime)
{
   Type modelClass = Type::ByTypeInfo(typeid(ModelClass));
   Member supriseGetter = modelClass.MemberByName("getSuprise");
   CPPUNIT_ASSERT(supriseGetter);

   AnnotationList annotations = supriseGetter.Annotations();
   CPPUNIT_ASSERT(annotations);

   const Getter* getter = annotations.AnnotationWithType<Getter>();
   CPPUNIT_ASSERT(getter);
}

