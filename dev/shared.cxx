#include <vector>
#include <iostream>
#include <memory>


using IntVector = std::vector<int>;


struct B;
struct A {
   ~A() { std::cout << "~A" << std::endl; }
   std::shared_ptr<B> b;
};
struct B {
   ~B() { std::cout << "~B" << std::endl; }
   std::weak_ptr<A> a;
};

struct Widget {};
Widget *getWidgetFromPool() { return new Widget; }
void returnWidgetToPool(const Widget* w) { delete w; } 


int main()
{
   IntVector v = {1, 3, 5};

   std::shared_ptr<IntVector> spv(new IntVector);

   std::cout << "shared_ptr: " << spv << std::endl;


   std::shared_ptr<A> a(new A);
   std::shared_ptr<B> b(new B);

   a->b = b;
   b->a = a;

   std::cout << "a count: " << a.use_count() << std::endl;
   std::cout << "b count: " << b.use_count() << std::endl;
   std::cout << "a->b count: " << a->b.use_count() << std::endl;
   std::cout << "b->a count: " << b->a.use_count() << std::endl;
   std::cout << "b->a expired: " << b->a.expired() << std::endl;
 
   std::unique_ptr<Widget, void(*)(const Widget *)> u_ptr { getWidgetFromPool(), returnWidgetToPool};
   std::unique_ptr<int[]> upa1 (new int[3] {7,2,1});
   
   std::unique_ptr<int[]> upa2;
   upa2 = std::move(upa1);
  
   for(int i = 0; i < 3; i++) std::cout << upa2[i] << " ";
   std::cout << std::endl;
 
   return 0;
}
