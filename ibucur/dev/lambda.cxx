#include <vector>
#include <algorithm>
#include <iostream>


int a;

std::function<int(int)> returnLambda()
{
   int c = 2;
   int b = 5;

   return [=](int x) { 
      std::cout << "a=" << a << std::endl;
      std::cout << "b=" << b << std::endl;
      std::cout << "c=" << c << std::endl;
      std::cout << "x=" << x << std::endl;
      return a*x*x + b*x + c == 0; };
}

std::function<bool(long)> f;

void setLambda()
{
   static int divisor = 5; // cannot be local
   f = [&](long x) { return x % divisor == 0; };
}



int main()
{
   std::vector<int> v {10, 5, 15, 4, 23};
   auto it = std::find_if(v.cbegin(), v.cend(), [](int i)->int { return i > 0 && i < 10; });

   std::cout << *it << std::endl;
   it++;


   std::function<int(int)> factorial = [&](int x) { return (x==1) ? 1 : (x * factorial(x-1)); };
   
   std::cout << "factorial = " << factorial(4) << std::endl;

   

   setLambda();
   std::cout << f(23L) << " " << f(15L) << " " << f(176L) << std::endl; 



   return 0;
}


