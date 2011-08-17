#ifndef ROOT_IOSFileContainer
#define ROOT_IOSFileContainer

#include <vector>
#include <memory>
#include <string>

class TObject;
class TFile;

//
//Objects of this class will be created from Obj-C code, so
//its operations should not throw.
//

namespace ROOT_iOS {

class FileContainer {
public:
   typedef std::vector<TObject *>::size_type size_type;

   FileContainer(const std::string &fileName);
   ~FileContainer();

   bool IsValid()const;

   size_type NumberOfVisibleObjects()const;
   TObject *GetObject(size_type ind)const;

private:
   bool fIsOk;

   std::auto_ptr<TFile> fFileHandler;
   std::vector<TObject *> fFileContents;
};

}

#endif
