namespace Foam
{

class word;

template<class T, class Key=word, class Hash=int>
class HashTable
{
public:

   HashTable() {}
   ~HashTable(){}
   HashTable(const HashTable&) {}
   class iterator
   {
   public:
   private:
      
#if !defined(interp)
      HashTable<T, Key, Hash> &curHashTable_;
#endif
   };

private:

   iterator endIter_;
};

typedef HashTable<int> HashTableInt;
}


#ifdef __MAKECINT__

#pragma link C++ nestedtypedef;
#pragma link C++ nestedclass;
#pragma link C++ class Foam::HashTableInt;
#endif

