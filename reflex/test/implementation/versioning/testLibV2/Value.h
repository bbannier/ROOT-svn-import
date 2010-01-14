#ifndef TEST_LIBV2_VALUE_H
#define TEST_LIBV2_VALUE_H

#ifdef WIN32
#ifdef test_LibV2_EXPORTS
#define TEST_LIBV2_API _declspec(dllexport)
#else
#define TEST_LIBV2_API _declspec(dllimport)
#endif
#else
#define TEST_LIBV2_API
#endif

#include <string>


class TEST_LIBV2_API Value
{
public:
   Value();
   Value(const Value& other);
   Value(const std::string& value);
   virtual ~Value();

   std::string value() const;

private:
   const std::string _value;

private: // forbidden
   Value& operator=(const Value&);
};


#endif
