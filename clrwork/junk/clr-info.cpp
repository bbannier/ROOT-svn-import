#include "clr-info.h"

#include <iostream>
#include <stdexcept>
#include <string>

static void show_message(const std::string& location, const std::string& msg)
{
   std::cerr << "CLR: " << location;
   if (location.size()) {
      std::cerr << " ";
   }
   std::cerr << msg << std::endl;
}

void info(const std::string msg, const std::string location)
{
   show_message(location, msg);
}

void warning(const std::string msg, const std::string location)
{
   show_message(location, "warning: " + msg);
}

void error(const std::string msg, const std::string location)
{
   show_message(location, "error: " + msg);
   throw new std::runtime_error("error: " + location + ": " + msg);
}

