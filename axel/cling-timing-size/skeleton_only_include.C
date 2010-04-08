#include <stdio.h>
#include <map>

void %FUNCNAME%()
{
   static const int pi100 = 314;
   long l = 0;

   for (int i = 0; i < 100; ++i) {
      for (int j = 0; j < 100; ++j) {
         l += j;
      }
      printf("%ld \n", l);
   }
   printf("\n");
}
