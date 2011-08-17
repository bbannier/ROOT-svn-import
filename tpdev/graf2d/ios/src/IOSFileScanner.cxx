#include <stdexcept>
#include <memory>

#include "IOSFileScanner.h"
#include "TIterator.h"
#include "TFile.h"
#include "TList.h"
#include "TKey.h"
#include "TH1.h"

namespace ROOT_iOS {
namespace FileUtils {

//__________________________________________________________________________________________________________________________
TObject *ReadObjectForKey(TFile *inputFile, const TKey *key)
{
   TObject *obj = inputFile->Get(key->GetName());
   if (!obj)
      throw std::runtime_error(std::string("no object for key ") + key->GetName());

   //Objects of some types are onwed by the file. So I have to make
   //them free from such ownership to make
   //their processing later more uniform.
   if (TH1 *hist = dynamic_cast<TH1 *>(obj))
      hist->SetDirectory(0);
   //Check other types!

   return obj;
}


//__________________________________________________________________________________________________________________________
void ScanFileForVisibleObjects(TFile *inputFile, const std::set<std::string> &visibleTypes, std::vector<TObject *> &objects)
{
   //Find objects of visible types in a root file.
   const TList *keys = inputFile->GetListOfKeys();
   TIter next(keys);
   std::vector<TObject *>tmp;
   
   while (const TKey *k = static_cast<TKey *>(next())) {
      //Check, if object, pointed by the key, is supported.
      if (visibleTypes.find(k->GetClassName()) != visibleTypes.end()) {
         TObject *newObject = ReadObjectForKey(inputFile, k);
         try {
            tmp.push_back(newObject);
         } catch (const std::bad_alloc &) {
            delete newObject;
            throw;
         }
      }
   }
   
   objects.swap(tmp);
}

}
}

