
/* clr-info.cc */

#include "clr-info.h"

#ifdef TRACE
   #include "clr-trace.h"
#endif

#include <string>
#include <iostream>
#include <stdexcept> // class std::runtime_error

using std::string;

/* ---------------------------------------------------------------------- */

void init_trace ()
{
   #ifdef TRACE
      set_error_handlers();
   #endif
}

/* ---------------------------------------------------------------------- */

void show_message (const string msg, const string location)
{
   string txt = location;

   if (txt != "")
      txt = txt + " ";

   txt = txt + msg;

   std::cerr << "CLR " << txt << std::endl;
}

/* ---------------------------------------------------------------------- */

void info (const string msg, const string location)
{
   show_message (msg, location);
}

void warning (const string msg, const string location)
{
   show_message ("warning: " + msg, location);
}

void error (const string msg, const string location)
{
   show_message ("error: " + msg, location);
   #ifdef TRACE
      trace ();
   #endif
   throw new std::runtime_error ("error: " + msg  + ", " +  location);
}

/* ---------------------------------------------------------------------- */

