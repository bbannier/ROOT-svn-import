#ifndef TEST_LIBV1_VALUE_H
#define TEST_LIBV1_VALUE_H

#ifdef WIN32
#ifdef test_LibV1_EXPORTS
#define TEST_LIBV1_API _declspec(dllexport)
#else
#define TEST_LIBV1_API _declspec(dllimport)
#endif
#else
#define TEST_LIBV1_API
#endif

#include <string>


class TEST_LIBV1_API Value
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
