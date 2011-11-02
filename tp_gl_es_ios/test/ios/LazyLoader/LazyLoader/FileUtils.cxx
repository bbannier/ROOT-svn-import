#include <stdexcept>
#include <set>

#include "TSystem.h"
#include "TString.h"
#include "TList.h"
#include "TFile.h"
#include "TKey.h"

#include "FileUtils.h"

namespace ROOT {
namespace iOS {
namespace Browser {


namespace {

//__________________________________________________________________________________________________________________________
void FillVisibleTypes(std::set<TString> &types)
{
   //To be extended.
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
   types.insert("TH2S");
}

const char *errorOptionToString[] = {"", "E", "E1", "E2", "E3", "E4"};

//__________________________________________________________________________________________________________________________
void RemoveErrorDrawOption(TString &options)
{
   const Ssiz_t pos = options.Index("E");
   if (pos != kNPOS) {

      Ssiz_t n = 1;
      if (pos + 1 < options.Length()) {
         const char nextChar = options[pos + 1];
         if (std::isdigit(nextChar) && (nextChar - '0' >= 1 && nextChar - '0' <= 4))
            n = 2;
      }
   
      options.Remove(pos, n);
   }
}

//__________________________________________________________________________________________________________________________
void RemoveMarkerDrawOption(TString &options)
{
   const Ssiz_t pos = options.Index("P");
   if (pos != kNPOS)
      options.Remove(pos, 1);
}

//__________________________________________________________________________________________________________________________
ObjectType ClassNameToType(const TString &className)
{
   if (className.Contains("TH1"))
      return ObjectType::h1d;
   else if (className.Contains("TH2"))
      return ObjectType::h2d;

   return ObjectType::unknown;
}

//__________________________________________________________________________________________________________________________
void FillObjectHandler(ObjectHandler &newHandler, const TKey *fileKey, TString &option)
{
   option = "";

   newHandler.fObject = 0;//We'll read object only on demand.
   newHandler.fFileKey = fileKey;
   
   //This is just a temporary hack: I have to know the type of object, to be able to
   //attach a generic thumbnail.
   newHandler.fType = ClassNameToType(fileKey->GetClassName());
   //TODO Additional work to set special options for different type. Nothing is done now for TH1/TH2 test.
}

//__________________________________________________________________________________________________________________________
void ScanFileForVisibleObjects(TFile *inputFile, const std::set<TString> &visibleTypes, std::vector<ObjectHandler> &objects, std::vector<TString> &options)
{
   //Find objects of visible types in a root file.
   const TList *keys = inputFile->GetListOfKeys();
   TIter next(keys);
   std::vector<ObjectHandler> tmp;
   std::vector<TString> opts;
   TString option;
   
   while (const TKey *k = static_cast<TKey *>(next())) {
      //Check, if object, pointed by the key, is supported.
      if (visibleTypes.find(k->GetClassName()) != visibleTypes.end()) {
         ObjectHandler newObject;
         FillObjectHandler(newObject, k, option);
         tmp.push_back(newObject);
         opts.push_back(option);
      }
   }
   
   objects.swap(tmp);
   options.swap(opts);
}

}//Unnamed namespace

//__________________________________________________________________________________________________________________________
ObjectHandler::ObjectHandler()
                  : fType(ObjectType::unknown),
                    fFileKey(0),
                    fObject(0)
{
}

//__________________________________________________________________________________________________________________________
ObjectHandler::ObjectHandler(ObjectType type, const TKey *key, TObject *obj)
                  : fType(type),
                    fFileKey(key),
                    fObject(obj)
{
}

//__________________________________________________________________________________________________________________________
FileContainer::FileContainer(const std::string &fileName)
{
   fFileName = gSystem->BaseName(fileName.c_str());

   fFileHandler.reset(TFile::Open(fileName.c_str(), "read"));
   if (!fFileHandler.get())
      throw std::runtime_error("File was not opened");

   std::set<TString> visibleTypes;
   FillVisibleTypes(visibleTypes);

   ScanFileForVisibleObjects(fFileHandler.get(), visibleTypes, fFileContents, fOptions);
}

//__________________________________________________________________________________________________________________________
FileContainer::~FileContainer()
{
   //I'll need this to delete loaded objects and attached files later.
}

//__________________________________________________________________________________________________________________________
FileContainer::size_type FileContainer::GetNumberOfObjects()const
{
   return fFileContents.size();
}

//__________________________________________________________________________________________________________________________
TObject *FileContainer::GetObject(size_type ind)const
{
   return fFileContents[ind].fObject;
}

//__________________________________________________________________________________________________________________________
Pad *FileContainer::GetPadAttached(size_type ind)const
{
   //No pad was attached yet.
   return 0;//fAttachedPads[ind];
}

//__________________________________________________________________________________________________________________________
void FileContainer::SetErrorDrawOption(size_type ind, EHistogramErrorOption opt)
{
   //Nothing to change.
   if (GetErrorDrawOption(ind) == opt)
      return;

   //1. Remove previous error options (if any).
   RemoveErrorDrawOption(fOptions[ind]);
   //2. Add new option.
   fOptions[ind] += errorOptionToString[int(opt)];
}

//__________________________________________________________________________________________________________________________
EHistogramErrorOption FileContainer::GetErrorDrawOption(size_type ind)const
{
   const TString &options = fOptions[ind];
   const Ssiz_t pos = options.Index("E");
   if (pos == kNPOS)
      return EHistogramErrorOption::hetNoError;
   
   if (pos + 1 < options.Length()) {
      const char nextChar = options[pos + 1];
      if (nextChar == '1')
         return EHistogramErrorOption::hetE1;
      if (nextChar == '2')
         return EHistogramErrorOption::hetE2;
      if (nextChar == '3')
         return EHistogramErrorOption::hetE3;
      if (nextChar == '4')
         return EHistogramErrorOption::hetE4;
   }
   
   return EHistogramErrorOption::hetE;
}

//__________________________________________________________________________________________________________________________
void FileContainer::SetMarkerDrawOption(size_type ind, bool on)
{
   if (GetMarkerDrawOption(ind) == on)
      return;

   RemoveMarkerDrawOption(fOptions[ind]);

   if (on)
      fOptions[ind] += "P";
}

//__________________________________________________________________________________________________________________________
bool FileContainer::GetMarkerDrawOption(size_type ind)const
{
   return fOptions[ind].Index("P") != kNPOS;
}

//__________________________________________________________________________________________________________________________
const char *FileContainer::GetFileName()const
{
   return fFileName.c_str();
}

//__________________________________________________________________________________________________________________________
FileContainer *CreateROOTFileContainer(const char *filePath)
{
   try {
      FileContainer *newContainer = new FileContainer(filePath);
      return newContainer;
   } catch (const std::exception &) {//Only std exceptions.
      return 0;
   }
}

//__________________________________________________________________________________________________________________________
void DeleteROOTFileContainer(FileContainer *container)
{
   delete container;
}

}//namespace Browser
}//namespace iOS
}//namespace ROOT
