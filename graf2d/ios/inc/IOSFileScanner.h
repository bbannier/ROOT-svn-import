#ifndef ROOT_IOSFileScanner
#define ROOT_IOSFileScanner

#include <vector>
#include <string>
#include <set>
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
void ScanFileForVisibleObjects(TFile *file, const std::set<std::string> &visibleTypes, std::vector<TObject *> &objects, std::vector<std::string> &options);

}
}

#endif

