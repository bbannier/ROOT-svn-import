#ifndef ROOT_IOSFileContainer
#define ROOT_IOSFileContainer

#include <vector>
#include <memory>
#include <string>

#ifndef ROOT_TString
#include "TString.h"
#endif

//
//TODO: These classes should be removed from graf2d/ios as an application-specific
//code which can be placed where it's used - in a RootBrowser application for iPad.
//

class TObject;
class TFile;

namespace ROOT_iOS {

enum EHistogramErrorOption {
   hetNoError,
   hetE,
   hetE1,
   hetE2,
   hetE3,
   hetE4
};


class FileContainer {
public:
   typedef std::vector<TObject *>::size_type size_type;

   FileContainer(const std::string &fileName);
   ~FileContainer();

   size_type GetNumberOfObjects()const;
   TObject *GetObject(size_type ind)const;
   const char *GetDrawOption(size_type ind)const;

   void SetErrorDrawOption(size_type ind, EHistogramErrorOption opt);
   EHistogramErrorOption GetErrorDrawOption(size_type ind)const;
   
   void SetMarkerDrawOption(size_type ind, bool on);
   bool GetMarkerDrawOption(size_type ind)const;

   const char *GetFileName()const;

private:
   std::string fFileName;

   std::auto_ptr<TFile> fFileHandler;
   std::vector<TObject *> fFileContents;
   std::vector<TString> fOptions;
};

//This is the function to be called from Obj-C++ code.
//Return: non-null pointer in case file was
//opened and its content read.
FileContainer *CreateFileContainer(const char *fileName);
//Just for symmetry.
void DeleteFileContainer(FileContainer *container);

}

#endif
