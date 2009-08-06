#include "TEveManager.h"
#include "TEveGeoShape.h"
#include "TEveTrans.h"
#include "TGLViewer.h"
#include "TEveVSDStructs.h"
#include "TGeoTube.h"

#include "TSystem.h"
#include "iostream"
#include "TMath.h"

#include "TEveGraphNode.h"

using namespace std;


int dirsize(const TString &);
int read_and_register_dir(const TString& dir, TEveElement* parent, Int_t level);

TEveGraphNode *file_list = 0;
const Float_t kZ_d   = 2;

void init(Int_t mode = 0)
{
  cout <<"mode: "<< mode << endl;

  TEveManager::Create();

  TEveScene *scene = gEve->SpawnNewScene("Hierachical Scene", "OoogaDooga");
  scene->SetHierarchical(kTRUE);

  gEve->GetDefaultViewer()->AddScene(scene);

  file_list = new TEveGraphNode("Files");
  scene->AddElement(file_list);

  TString dir="/home/sdavkova/root/core";
  Int_t   nlevels = 2;
  read_and_register_dir(dir, file_list, nlevels);

  gEve->GetDefaultGLViewer()->SetCurrentCamera(TGLViewer::kCameraPerspXOY);

  switch (mode)
  {
  default:
  case 0:
    file_list->position_linear(20);
    break;
  case 1:
    file_list->position_circle(20);
    break;
  case 2:
     file_list->position_spiral();
    break;
  
  }
  
  //file_list->DisableListElements(kTRUE, kFALSE);
}


int read_and_register_dir(const TString& dir, TEveElement* parent, Int_t level)
{
  if (level <= 0)
    return 0;

  // Open directory
  void* dh = gSystem->OpenDirectory(dir);
  // try also with "." in directory .../root/tutorials/eve
  if (!dh)
  {
    cout<<"Can't open!";
    exit(1);
  }
  const char* de;
  Int_t sum_size = 0;
  while (de = gSystem->GetDirEntry(dh))
  {
    if (de[0] == '.') continue;

    FileStat_t fs;
    TString path = dir + "/" + de;
    //printf("Blaa! %s\n", path.Data());
    gSystem->GetPathInfo(path, fs);
    TString dof;
    
    TEveGraphNode *b;
    b = new TEveGraphNode(de);
    
    Int_t size = 0;
    if (fs.fMode & kS_IFDIR)
    {
      dof="dir";
      size = read_and_register_dir(path, b, level - 1);
    }
    else
    {
      dof="file";
      size = fs.fSize;
    }
    sum_size += size;
    printf("%-20s %8d %8x %s\n",de, size, fs.fMode, dof.Data());
    b->SetElementTitle(Form("%s - %d", de, size));
    b->SetShape(new TGeoTube(0, TMath::Log10(TMath::Max(1024,size)), kZ_d));
    b->SetMainColor(kCyan);
    b->SetPickable(kTRUE); // Allow selection in GL window
    b->SetSize(size);
    b->SetSizeChildren(sum_size);
    parent->AddElement(b);
  }
  gSystem->FreeDirectory(dh);
  return sum_size;
}

int dirsize(const TString& dir)
{
  int size=0;
  // Open directory
  void* dh = gSystem->OpenDirectory(dir);
  // try also with "." in directory .../root/tutorials/eve
  if (!dh)
  {
    cout<<"Can't open!";
    exit(1);
  }
  const char* de;
  while (de = gSystem->GetDirEntry(dh))
  {
    if (de[0] == '.') continue;


    FileStat_t fs;
    TString xpath = dir + "/" + de;
    gSystem->GetPathInfo(xpath, fs);
    TString dof;
    if (fs.fMode & kS_IFDIR)
    {
      dof="dir";
      size += dirsize(xpath);
    }
    else
    {
      dof="file";
      size += fs.fSize;
    }
  }

  // printf("%-20s %8d %8x %s\n",de, fs.fSize, fs.fMode, dof.Data());}
  gSystem->FreeDirectory(dh);
  return size;

}
