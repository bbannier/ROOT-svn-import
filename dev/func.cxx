#include <iostream>
#include <functional>

int length(std::string& s) { return s.length(); } 
void three(int a, int b, int c) { std::cout << "a " << a << " b " << b << " c " << c << std::endl; }

//int func(const int&) { std::cout << "const int &" << std::endl; }
//int func(int &&) { std::cout << "int &&" << std::endl; }
int func(int &) { std::cout << "int &" << std::endl; }
int func(const int&&) { std::cout << "const int &&" << std::endl; }

int main()
{
   std::function<int(std::string&)> f;
   f = length;
   f = [](std::string& s)->unsigned { s += "!"; return s.size(); };
 
   std::string bla("bla"); 
   std::cout << f(bla) << std::endl; 

   std::function<void(int,int)> two = std::bind(three, std::placeholders::_1, std::placeholders::_1, std::placeholders::_2);
   two(1,4);
   std::function<void(int,int,int)> eerht = std::bind(three, std::placeholders::_3, std::placeholders::_2, std::placeholders::_1);
   eerht(8,3,6);

   int const x = 3;

   func(x);
   //func(5);
}

