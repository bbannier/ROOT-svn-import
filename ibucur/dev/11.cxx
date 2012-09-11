#include <future>
#include <iostream>
#include <iterator>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>

typedef std::unordered_map<std::string, std::size_t> WordCountMapType;

WordCountMapType wordsInFile(char const * const fileName)
{
   std::ifstream file(fileName);
   WordCountMapType wordCounts;

   for(std::string word; file >> word; ) { ++wordCounts[word]; }

   return wordCounts;
}

template<typename MapIt>
void showCommonWords(MapIt begin, MapIt end, const std::size_t n)
{
   std::vector<MapIt> wordIters;
   wordIters.reserve(std::distance(begin, end));
   for(auto i = begin; i != end; ++i) wordIters.push_back();

   auto sortedRangeEnd = wordIters.begin() + n;

   std::partial_sort(wordIters.begin(), sortedRangeEnd, wordIters.end(),
      [](MapIt it1, MapIt it2) { return it1->second > it2->second; } );

   for (auto it = wordIters.cbegin(); it != sortedRangeEnd; ++it) {
      std::printf(" %-10s%10zu\n", (*it)->first.c_str(), (*it)->second);
   }

}

template<typename F, typename ... Ts>
auto logAndInvoke(std::ostream& os, F&& func, Ts&& ... args)->decltype(func(std::forward<Ts>(args)...))
{
   os << std::chrono::system_clock::now;
   return func(std::forward<Ts>(args)...);
}

template<typename T1, typename T2>
auto mult(T1&& a, T2&& b)->
   decltype(std::forward<T1>(a) * std::forward<T2>(b))
{
   return std::forward<T1>(a) * std::forward<T2>(b);
}



int main(int argc, char *argv[])
{
   std::vector<std::future<WordCountMapType>> futures;

   for(int argNum = 1; argNum < argc; ++argNum) {
      futures.push_back(std::async([=] { return wordsInFile(argv[argNum]); } ));
   }

   WordCountMapType wordCounts;
   for(auto& f : futures) {
      const auto results = f.get(); // move map returned by wordsInFile
   
      for(const auto& wordCount : results) {
         wordCounts[wordCount.first] += wordCount.second;
      }
   }

   return 0;
}

