#ifndef FILEUTILS_INCLUDED
#define FILEUTILS_INCLUDED

#ifndef ROOT_TString
#include "TString.h"
#endif

class TObject;
class TFile;
class TKey;

namespace ROOT {
namespace iOS {

class Pad;

namespace Browser {

enum class ObjectType {
   unknown,
   h1d,
   h2d
};

enum class EHistogramErrorOption {
   hetNoError,
   hetE,
   hetE1,
   hetE2,
   hetE3,
   hetE4
};

struct ObjectHandler {
   ObjectHandler();
   ObjectHandler(ObjectType type, const TKey *key, TObject *obj);

   void LoadObject();

   ObjectType fType;
   const TKey *fFileKey;
   TObject *fObject;
};

class FileContainer {
public:
   typedef std::vector<ObjectHandler>::size_type size_type;
   
   FileContainer(const std::string &fileName);
   ~FileContainer();

   size_type GetNumberOfObjects()const;
   TObject *GetObject(size_type ind)const;
   Pad *GetPadAttached(size_type ind)const;

   void SetErrorDrawOption(size_type ind, EHistogramErrorOption opt);
   EHistogramErrorOption GetErrorDrawOption(size_type ind)const;
   
   void SetMarkerDrawOption(size_type ind, bool on);
   bool GetMarkerDrawOption(size_type ind)const;

   const char *GetFileName()const;
private:
   std::string fFileName;

   std::auto_ptr<TFile> fFileHandler;

   std::vector<ObjectHandler> fFileContents;
   std::vector<TString> fOptions;
   std::vector<Pad *> fAttachedPads;
   
   FileContainer(const FileContainer &rhs) = delete;
   FileContainer &operator = (const FileContainer &rhs) = delete;
};

FileContainer *CreateROOTFileContainer(const char *filePath);
void DeleteROOTFileContainer(FileContainer *container);

}
}
}

#endif
