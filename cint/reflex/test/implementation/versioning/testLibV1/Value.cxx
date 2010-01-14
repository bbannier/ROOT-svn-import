#include "Value.h"


Value::Value()
{
}
Value::Value(const Value& other)
: _value(other._value)
{
}

Value::Value(const std::string& value)
: _value(value)
{
}

Value::~Value()
{
}

std::string Value::value() const
{
   return _value;
}
