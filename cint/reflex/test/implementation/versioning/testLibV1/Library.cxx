#include "Library.h"

Library::Library()
: fBaseClass(NULL)
{
}

BaseClass* Library::getBaseClass() const
{
   return fBaseClass;
}

void Library::setBaseClass(BaseClass* value)
{
   fBaseClass = value;
}

double Library::version() const
{
   return 1.0;
}

const std::type_info* Library::typeId() const
{
   return &typeid(*this);
}

Value Library::smallName() const
{
   return Value("Library1.0 instance");
}

Reflex::Dictionary Library::Dictionary() const
{
   return Reflex::Dictionary::Defining<D>();
}

D::~D()
{
}
