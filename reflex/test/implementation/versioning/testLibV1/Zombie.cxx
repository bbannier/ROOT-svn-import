#include "Zombie.h"

namespace SharedNamespace
{
   Zombie* NewZombie()
   {
      return new Zombie();
   }

   Zombie::Zombie()
   {
   }

   Zombie::~Zombie()
   {
   }
}
