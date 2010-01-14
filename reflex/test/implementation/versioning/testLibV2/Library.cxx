#include "Library.h"

double Library::version() const
{
   return 2.0;
}

const std::type_info* Library::typeId() const
{
   return &typeid(*this);
}

Value Library::smallName() const
{
   return Value("Value2.0 instance");
}

Reflex::Dictionary Library::Dictionary() const
{
   return Reflex::Dictionary::Defining<ClassV2>();
}

ClassV2::ClassV2()
{
}
ClassV2::~ClassV2()
{
}

