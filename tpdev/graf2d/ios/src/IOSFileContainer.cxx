#include <stdexcept>
#include <set>

#include "IOSFileContainer.h"
#include "IOSFileScanner.h"
#include "TFile.h"


namespace ROOT_iOS {
namespace {

//__________________________________________________________________________________________________________________________
void FillVisibleTypes(std::set<std::string> &types)
{
   types.insert("TH1C");
   types.insert("TH1D");
   types.insert("TH1F");
   types.insert("TH1I");
   types.insert("TH1K");
   types.insert("TH1S");
   types.insert("TH2C");
   types.insert("TH2D");
   types.insert("TH2F");
   types.insert("TH2I");
   types.insert("TH2Poly");
   types.insert("TH2S");
   types.insert("TH3C");
   types.insert("TH3D");
   types.insert("TH3F");
   types.insert("TH3I");
   types.insert("TH3S");
}

}

//__________________________________________________________________________________________________________________________
FileContainer::FileContainer(const std::string &fileName)
                  : fIsOk(false)
{
   try {
      fFileHandler.reset(TFile::Open(fileName.c_str(), "read"));
      if (!fFileHandler.get())
         return;
         
      std::set<std::string> visibleTypes;
      FillVisibleTypes(visibleTypes);
      
      FileUtils::ScanFileForVisibleObjects(fFileHandler.get(), visibleTypes, fFileContents);
   } catch (const std::exception &) {//Only std exceptions.
      return;
   }
   
   fIsOk = true;
}

//__________________________________________________________________________________________________________________________
FileContainer::~FileContainer()
{
   for (size_type i = 0; i < fFileContents.size(); ++i)
      delete  fFileContents[i];
}

//__________________________________________________________________________________________________________________________
bool FileContainer::IsValid()const
{
   return fIsOk;
}

//__________________________________________________________________________________________________________________________
FileContainer::size_type FileContainer::NumberOfVisibleObjects()const
{
   return fFileContents.size();
}

//__________________________________________________________________________________________________________________________
TObject *FileContainer::GetObject(size_type ind)const
{
   return fFileContents[ind];
}

}
