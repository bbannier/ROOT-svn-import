#ifndef __CLR_INFO_H__
#define __CLR_INFO_H__

#include <string>

void info(const std::string& msg, const std::string& location = "");
void warning(const std::string& msg, const std::string& location = "");
void error(const std::string& msg, const std::string& location = "");

#endif // __CLR_INFO_H__
