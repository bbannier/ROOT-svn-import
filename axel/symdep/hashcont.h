#ifndef INCLUDE_HASHCONT
#define INCLUDE_HASHCONT

// Need
//   std::unordered_map<const char*, const char*, ConstCharHash_t, ConstCharEqual_t>
//   std::unordered_set<const char*, ConstCharHash_t, ConstCharEqual_t>
// or equivalent.
// Replace the first two with std::map, std::set if not available.
// Define typedefs
//   CPtrCPtrMap_t
//   CPtrSet_t

#ifdef _MSC_VER
# error "WINDOWS NOT SUPPORTED!"
# error "Windows does selective linking by default, so you don't need this tool."
#elif defined(__GNUC__)
# if (__GNUC__ > 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
#  include <unordered_map>
#  include <unordered_set>
#   define HASHMAP std::unordered_map
#   define HASHSET std::unordered_set
#   define HASHFCN std::hash<std::string>
# elif (__GNUC__ == 4)
#  include <tr1/unordered_map>
#  include <tr1/unordered_set>
#   define HASHMAP std::tr1::unordered_map
#   define HASHSET std::tr1::unordered_set
#   define HASHFCN std::tr1::hash<std::string>
# elif (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)
# include <ext/hash_map>
# include <ext/hash_set>
#   define HASHMAP __gnu_cxx::unordered_map
#   define HASHSET __gnu_cxx::unordered_set
#   define HASHFCN __gnu_cxx:::hash<const char*>
# endif
#elif defined(__INTEL_COMPILER)
# include <ext/hash_map>
# include <ext/hash_set>
# define HASHMAP std::hash_map
# define HASHSET std::hash_set
# define HASHFCN std::hash<std::string>
#endif

#include <cstring>

#ifdef HASHMAP

# include <string>
struct ConstCharHash_t {
   size_t operator() (const char* c) const {
      HASHFCN h;
      return h(c);
   }
};
struct ConstCharEqual_t {
   bool operator() (const char* x, const char* y) const {
      return (!x && !y) || (x && y && !strcmp(x, y));
   }
};

typedef HASHMAP <const char*, const char*, ConstCharHash_t, ConstCharEqual_t> CPtrCPtrMap_t;
typedef HASHSET <const char*, ConstCharHash_t, ConstCharEqual_t> CPtrSet_t;

# undef HASHMAP
# undef HASHSET
# undef HASHFCN

#else // HASHMAP
#include <map>
#include <set>

struct ConstCharLess_t {
   int operator() (const char* x, const char* y) const {
      // x < y?
      return (!x && y) || (x && y && (strcmp(x, y) < 0));
   }
};
typedef std::map<const char*, const char*, ConstCharLess_t> CPtrCPtrMap_t;
typedef std::set<const char*, ConstCharLess_t> CPtrSet_t;
#endif // HASHMAP

#endif // INCLUDE_HASHCONT
