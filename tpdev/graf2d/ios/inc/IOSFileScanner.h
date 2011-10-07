#ifndef ROOT_IOSFileScanner
#define ROOT_IOSFileScanner

#include <vector>
#include <set>

#ifndef ROOT_TString
#include "TString.h"
#endif

//
//scan_file looks through file's contents, 
//looking for TH1 and TGraph derived
//objects (objects, which can be visualized
//by the current ROOT code for iOS)
//

class TObject;
class TFile;

namespace ROOT_iOS {
namespace FileUtils {

//Find objects of "visible" types in a root file.
void ScanFileForVisibleObjects(TFile *file, const std::set<TString> &visibleTypes, std::vector<TObject *> &objects, std::vector<TString> &options);

}
}

#endif

