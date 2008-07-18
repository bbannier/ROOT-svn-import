// @(#)root/core:$Id$
// author: Lukasz Janyst <ljanyst@cern.ch>

#ifndef ROOT_TSchemaHelper
#define ROOT_TSchemaHelper

#include <string>

namespace ROOT
{
   struct TSchemaHelper
   {
      TSchemaHelper(): fFunctionPtr( 0 ) {}
      std::string fTarget;
      std::string fSourceClass;
      std::string fSource;
      std::string fCode;
      std::string fVersion;
      std::string fChecksum;
      std::string fEmbed;
      std::string fInclude;
      void*       fFunctionPtr;
   };
}

#endif // ROOT_TSchemaHelper
