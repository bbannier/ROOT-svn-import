// @(#)root/test:$Id$
// Author: Anar Manafov   01/04/2008

//----------------------------------------------------------------
// This is a tests ROOT Iterators and STL algorithms.
// The test project covers the following cases:
// 1 - TList with std::for_each (Full iteration: from the Begin up to the End)
// 2 - TList with std::find_if
// 3 - TList with std::count_if
// 4 - TObjArray with std::for_each (Full iteration: from the Begin up to the End)
// 5 - TObjArray with std::find_if
// 6 - TObjArray with std::count_if
// 7 - TMap with std::for_each (Full iteration: from the Begin up to the End)
// 8 - TMap with std::for_each (Partial iteration: from the Begin up to the 3rd element)



// STD
#include <iostream>
#include <algorithm>
#include <sstream>
#include <stdexcept>
// ROOT
#include "TList.h"
#include "TObjString.h"
#include "TObjArray.h"
#include "TMap.h"

using namespace std;

static Int_t gCount = 0;

//______________________________________________________________________________
struct SEnumFunctor {
   bool operator()(TObject *aObj) const throw(exception) {
      if (!aObj)
         throw invalid_argument("SEnumFunctor: aObj is a NULL pointer");

      if ((aObj->IsA() == TObjString::Class())) {
         TObjString *str(dynamic_cast<TObjString*>(aObj));
         if (!str)
            throw runtime_error("SEnumFunctor: Container's element is not a TObjString object.");

         ++gCount;
         cout << str->String().Data() << endl;
      }

      if ((aObj->IsA() == TPair::Class())) {
         TPair *pair(dynamic_cast<TPair*>(aObj));
         if (!pair)
            throw runtime_error("SEnumFunctor: Container's element is not a TPair object.");

         TObjString *key(dynamic_cast<TObjString*>(pair->Key()));
         TObjString *value(dynamic_cast<TObjString*>(pair->Value()));
         if (!key || !value)
            throw runtime_error("SEnumFunctor: Can't retriev key/value of a pair");

         ++gCount;
         cout << key->String().Data() << " : " << value->String().Data() << endl;
      }

      return true;
   }
};

//______________________________________________________________________________
struct SFind : binary_function<TObject*, TString, bool> {
   bool operator()(TObject *_Obj, const TString &_ToFind) const {
      TObjString *str(dynamic_cast<TObjString*>(_Obj));
      return !str->String().CompareTo(_ToFind);
   }
};

//______________________________________________________________________________
// Checking a given container with for_each algorithm
// Full iteration: from Begin to End
template<class T>
void TestContainer_for_each(const T &container, Int_t aSize) throw(exception)
{
   gCount = 0; // TODO: a use of gCount is a very bad method. Needs to be revised.

   TIter iter(&container);
   for_each(iter.Begin(), TIter::End(), SEnumFunctor());
   if (aSize != gCount)
      throw runtime_error("Test case <TestList_for_each> has failed.");
   cout << "->> Ok." << endl;
}

//______________________________________________________________________________
// Checking a given container with for_each algorithm
// Partial iteration: from Begin to 3rd element
template<class T>
void TestContainer_for_each2(const T &container) throw(exception)
{
   gCount = 0; // TODO: a use of gCount is a very bad method. Needs to be revised.

   TIter iter(&container);
   TIter iter_end(&container);
   // Artificially shifting the iterator to the 4th potision - a new End iterator
   iter_end(); iter_end(); iter_end(); iter_end();
   for_each(iter.Begin(), iter_end, SEnumFunctor());
   if (3 != gCount)
      throw runtime_error("Test case <TestList_for_each2> has failed.");
   cout << "->> Ok." << endl;
}

//______________________________________________________________________________
// Checking a ROOT container with find_if algorithm
template<class T>
void TestContainer_find_if(const T &container, const TString &aToFind) throw(exception)
{
   typedef TIterCategory<T> iterator_t;

   iterator_t iter(&container);
   iterator_t found(
      find_if(iter.Begin(), iterator_t::End(), bind2nd(SFind(), aToFind))
   );
   if (!(*found))
      throw runtime_error("Test case <TestContainer_find_if> has failed.");

   TObjString *str(dynamic_cast<TObjString*>(*found));
   if (!str)
      throw runtime_error("Test case <TestContainer_find_if> has failed.");

   cout << "I found: " << str->String().Data() << endl;
   cout << "->> Ok." << endl;
}

//______________________________________________________________________________
// Checking a ROOT container with count_if algorithm
template<class T>
void TestContainer_count_if(const T &container, const TString &aToFind) throw(exception)
{
   typedef TIterCategory<T> iterator_t;

   iterator_t iter(&container);
   typename iterator_t::difference_type cnt(
      count_if(iter.Begin(), iterator_t::End(), bind2nd(SFind(), aToFind))
   );

   // we suppose to find exactly one match
   if (1 != cnt)
      throw runtime_error("Test case <TestContainer_count_if> has failed.");

   cout << "->> Ok." << endl;
}

//______________________________________________________________________________
void stressIterators() throw(exception)
{
   const Int_t size = 15;

   ostringstream ss;

   // TList
   TList list;
   for (int i = 0; i < size; ++i) {
      ss << "test string #" << i;
      TObjString *s(new TObjString(ss.str().c_str()));
      list.Add(s);
      ss.str("");
   }

   cout << "#1 ====================================" << endl;
   cout << "-----> " << "TestContainer_for_each<TList>(list, list.GetSize())" << endl;
   TestContainer_for_each<TList>(list, list.GetSize());

   cout << "#2 ====================================" << endl;
   cout << "-----> " << "TestContainer_find_if<TList>(list, \"test string #3\")" << endl;
   TestContainer_find_if<TList>(list, "test string #3");

   cout << "#3 ====================================" << endl;
   cout << "-----> " << "TestContainer_count_if<TList>(list, \"test string #3\")" << endl;
   TestContainer_count_if<TList>(list, "test string #3");


   // TObjArray
   TObjArray obj_array(size);
   for (int i = 0; i < size; ++i) {
      ss << "test string #" << i;
      TObjString *s(new TObjString(ss.str().c_str()));
      obj_array.Add(s);
      ss.str("");
   }

   cout << "#4 ====================================" << endl;
   cout << "-----> " << "TestContainer_for_each<TObjArray>(obj_array, obj_array.GetSize())" << endl;
   TestContainer_for_each<TObjArray>(obj_array, obj_array.GetSize());

   cout << "#5 ====================================" << endl;
   cout << "-----> " << "TestContainer_find_if<TObjArray>(obj_array, \"test string #3\")" << endl;
   TestContainer_find_if<TObjArray>(obj_array, "test string #3");

   cout << "#6 ====================================" << endl;
   cout << "-----> " << "TestContainer_count_if<TObjArray>(obj_array, \"test string #3\")" << endl;
   TestContainer_count_if<TObjArray>(obj_array, "test string #3");

   // TMap
   TMap map_container(size);
   for (int i = 0; i < size; ++i) {
      ss << "test string #" << i;
      TObjString *s(new TObjString(ss.str().c_str()));
      map_container.Add(s, new TObjString("value"));
      ss.str("");
   }
   
   cout << "#7 ====================================" << endl;
   cout << "-----> " << "TestContainer_for_each<TMap>(map_container, map_container.GetSize())" << endl;
   TestContainer_for_each<TMap>(map_container, map_container.GetSize());
   cout << "#8 ====================================" << endl;
   cout << "-----> " << "TestContainer_for_each2<TMap>(map_container)" << endl;
   TestContainer_for_each2<TMap>(map_container);
   cout << "#9 ====================================" << endl;
   cout << "-----> " << "TestContainer_find_if<TMap>(map_container, \"test string #3\")" << endl;

}

//______________________________________________________________________________
// return 0 on success and 1 otherwise
int main()
{
   try {

      stressIterators();

   } catch (const exception &e) {
      cerr << "Test has failed!" << endl;
      cerr << "Detailes: " << e.what() << endl;
      return 1;
   } catch (...) {
      cerr << "Test has failed!" << endl;
      cerr << "Unexpected error occurred." << endl;
      return 1;
   }

   cout << "\nTest successfully finished!" << endl;
   return 0;
}
