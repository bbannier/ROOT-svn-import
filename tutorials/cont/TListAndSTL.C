// @(#)root/test:$Id$
// Author: Anar Manafov   02/04/2008

// This is an exmpale of using TList with STL algoritms in CINT.

// STD
#include <algorithm>
#include <iostream>
#include <sstream>

// ROOT
#include "TList.h"
#include "TCollection.h"
#include "TObjString.h"

// A functor for the for_each algorithm
struct SEnumFunctor {
   bool operator()(TObject *aObj) throw(exception) {
      if (!aObj)
         return false;

      TObjString *str(dynamic_cast<TObjString*>(aObj));
      if (!str)
         return false;

      cout << "Value: "<< str->String().Data() << endl;
      return true;
   }
};

// A functor for the find_if algorithm
struct SFind {
  // using this ugly constructor, since there is problems with std::bindX in CINT 
   SFind(const TString &aStr): fToFind(aStr) {
   }
   bool operator()(TObject *aObj) {
      TObjString *str(dynamic_cast<TObjString*>(aObj));
      return !str->String().CompareTo(fToFind);
   }
private:
   const TString fToFind;
};


// The "main" function 
void TListAndSTL()
{
   const Int_t size(10);
   
   // Initializing TList container
   TList list;
   ostringstream ss;
   for (int i = 0; i < size; ++i) {
      ss << "test string #" << i;
      TObjString *s(new TObjString(ss.str().c_str()));
      list.Add(s);
      ss.str("");
   }
   
   
   //->>>>>>> Example #1
   // running the std::for_each algorithm on the list
   TIter iter(&list);
   for_each(iter.Begin(), TIter::End(), SEnumFunctor());
  
   
   // The following can only be processed with ACLiC.
   // Cint still has some problem interpreting some of the STL containers.
#ifndef __CINT__
   //->>>>>>> Example #2
   // we can try to find something in the container
   // using the std::find_if algorithm on the list
   string strToFind("test string #4");
   SFind func(strToFind.c_str());
   TIterCategory<TList> iter_cat(&list);
   TIterCategory<TList> found(
      find_if(iter_cat.Begin(), TIterCategory<TList>::End(), func)
   );
   
   // Checking the result
   if (!(*found))
   {
      cerr << "Can't find the string: \"" << strToFind << "\" in the container" << endl;
      return;
   }

   TObjString *str(dynamic_cast<TObjString*>(*found));
   if (!str)
   {
     cerr << "Can't find the string: \"" << strToFind << "\" in the container" << endl;
     return;
   }

   std::cout << "The string has been found: " << str->String().Data() << std::endl;
#endif
}
