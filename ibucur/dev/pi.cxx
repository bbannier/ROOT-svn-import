#include <cmath>
#include <iostream>

class A {
public:
   virtual void f(int x) { std::cout << "A::f" << std::endl; }
};

class B : public A {
public:
   using A::f;
   virtual void f(int x, int y) { std::cout << "B::f" << std::endl; }
};

int main()
{
   double myPI = 3.141592653589793238;
   std::cout.precision(20);
   std::cout << 2.0*M_PI << std::endl;
   std::cout << 2.0*myPI << std::endl;
 
   B b;
   b.f(2);

  return 0;
}


