#include <iostream>
#include <forward_list>
#include <algorithm>
#include <unordered_set>
#include <iterator>
#include <tuple>
#include <array>


class Name{};
class Address{};
class Date{};

template<typename Tuple>
void someFunc(Tuple t)
{
   std::size_t numElems = std::tuple_size<Tuple>::value;

   typedef typename std::tuple_element<0, Tuple>::type FirstType;
}

int main()
{
   std::forward_list<int> fli {1,2,3,4,5};
   auto it = std::find(fli.cbegin(), fli.cend(), 3);

   fli.erase_after(it);   
   fli.push_front(0);
   fli.erase_after(fli.before_begin());

   for_each(fli.cbegin(), fli.cend(), [](int i) { std::cout << i << std::endl; }); 

   std::unordered_set<std::string> s = {"in", "the", "jungle", "welcome", "to", "the", "jungle", "it", "gets", "worse", "here",
"everyday", "learn", "to", "live", "like", "an", "animal"};

   auto numBuckets = s.bucket_count();
   for(std::size_t b = 0; b < numBuckets; ++b) {
      std::cout << "Bucket " << b << " has " << s.bucket_size(b) << " elements: ";
      std::copy(s.cbegin(b),s.cend(b), std::ostream_iterator<std::string>(std::cout, " "));
      std::cout << std::endl;
   }

   Name empName; Address empAddr; Date empDate;
   std::tuple<Name, Address, Date> info;
   std::tie(empName, empAddr, empDate) = info;


   const std::size_t arraySize = 10;
   typedef std::array<Name, arraySize> NameArray;
   NameArray arr;

   std::size_t numElements = std::tuple_size<NameArray>::value;
   std::size_t elemSize = sizeof(std::tuple_element<0, NameArray>::type);
   std::cout << "arr has " << numElements << " elements, each of size " << elemSize << std::endl;


   return 0;
}




