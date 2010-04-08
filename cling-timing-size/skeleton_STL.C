#include <stdio.h>
#include <vector>

void pv(std::vector<double>* pvd) {
   int i = 0;
   while (((*pvd)[i] < 1000 && i < 100) || i <10) {
      if (i > 12) {
         if (i < 35) {
            if (i > 32) {
               if (i & 0x45 && (*pvd)[i] > 33.7) {
                  printf("vd[%d] = %g\n", i, (*pvd)[i]);
               }
            }
         }
      }
      ++i;
   }
}

void %FUNCNAME%()
{
   static const int pi100 = 314;

   for (int i = 0; i < 100; ++i) {
      for (int j = 0; j < 100; ++j) {
         std::vector<double> vd(100);
         for (int k = 0; k < 100; ++k) {
            double res = 100;
            res *= (i + j * pi100);
            res /= (++k);
            if ((k % 100) == (i % 100) && j == 42 && res < 1000 && res > 100) {
               switch (k % 10) {
               case 2: 
                  printf("2 ");
               case 5: 
                  printf("5 ");
               default: 
                  printf("any ");
               }
               printf("Funny result is %g\n", res);
            }
            if (res >= 100) vd[100] += res;
            else vd[size_t(res)] = res;
         }
         //printf("vec at 42 is = %g\n", vd[42]);
         pv(&vd);
      }
   }
}
