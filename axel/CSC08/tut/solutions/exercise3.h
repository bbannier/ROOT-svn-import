#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "TClonesArray.h"
#include "myparticle.h"

// generate dictionary for the vectors
#ifdef __MAKECINT__ 
#pragma link C++ class Vertex+; 
#pragma link C++ class MyParticle+; 
#pragma link C++ class Electron+; 
#pragma link C++ class Muon+; 
#pragma link C++ class Pion+; 
#endif
