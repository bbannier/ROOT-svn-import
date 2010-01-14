#include "Dummy.h"

namespace SharedNamespace
{
   Dummy* NewDummy()
   {
      return new Dummy();
   }

   Dummy::Dummy()
   {
   }
   Dummy::~Dummy()
   {
   }
}
