#include <stdexcept>
#include <set>

#include "IOSFileContainer.h"
#include "IOSFileScanner.h"
#include "TSystem.h"
#include "TFile.h"
#include "TH1.h"

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
   types.insert("TF2");
   types.insert("TGraphPolar");
   types.insert("TMultiGraph");
}

}

//__________________________________________________________________________________________________________________________
FileContainer::FileContainer(const std::string &fileName)
{
   fFileName = gSystem->BaseName(fileName.c_str());

   fFileHandler.reset(TFile::Open(fileName.c_str(), "read"));
   if (!fFileHandler.get())
      throw std::runtime_error("File was not opened");

   std::set<std::string> visibleTypes;
   FillVisibleTypes(visibleTypes);

   FileUtils::ScanFileForVisibleObjects(fFileHandler.get(), visibleTypes, fFileContents, fOptions);
}

//__________________________________________________________________________________________________________________________
FileContainer::~FileContainer()
{
   for (size_type i = 0; i < fFileContents.size(); ++i)
      delete  fFileContents[i];
}

//__________________________________________________________________________________________________________________________
FileContainer::size_type FileContainer::GetNumberOfObjects()const
{
   return fFileContents.size();
}

//__________________________________________________________________________________________________________________________
TObject *FileContainer::GetObject(size_type ind)const
{
   return fFileContents[ind];
}

//__________________________________________________________________________________________________________________________
const char *FileContainer::GetDrawOption(size_type ind)const
{
   return fOptions[ind].c_str();
}

//__________________________________________________________________________________________________________________________
const char *FileContainer::GetFileName()const
{
   return fFileName.c_str();
}

//__________________________________________________________________________________________________________________________
FileContainer *CreateFileContainer(const char *fileName)
{
   try {
      FileContainer *newContainer = new FileContainer(fileName);
      return newContainer;
   } catch (const std::exception &) {//Only std exceptions.
      return 0;
   }
}

//__________________________________________________________________________________________________________________________
void DeleteFileContainer(FileContainer *container)
{
   delete container;
}

}
