// @(#)root/geom:$Id$
// Author: Andrei Gheata   30/05/02

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/
// Author :                  date : Wed 24 Oct 2001 01:39:36 PM CEST

#ifndef ROOT_TGeoVolume
#define ROOT_TGeoVolume


#ifndef ROOT_TGeoAtt
#include "TGeoAtt.h"
#endif

#ifndef ROOT_TAttLine
#include "TAttLine.h"
#endif

#ifndef ROOT_TAttFill
#include "TAttFill.h"
#endif

#ifndef ROOT_TAtt3D
#include "TAtt3D.h"
#endif

#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

#ifndef ROOT_TGeoMedium
#include "TGeoMedium.h"
#endif

#ifndef ROOT_TGeoShape
#include "TGeoShape.h"
#endif

#ifndef ROOT_TGeoVoxelFinder
#include "TGeoVoxelFinder.h"
#endif

// forward declarations
class TH2F;
class TGeoNode;
class TGeoMatrix;
class TGeoPatternFinder;
class TGeoManager;

/*************************************************************************
 * TGeoVolume - class description
 *
 *************************************************************************/

class TGeoVolume : public TNamed,
                   public TGeoAtt,
                   public TAttLine,
                   public TAttFill,
                   public TAtt3D
{
protected :
   TObjArray         *fNodes;          // array of nodes inside this volume
   TGeoShape         *fShape;          // shape
   TGeoMedium        *fMedium;         // tracking medium
   TGeoPatternFinder *fFinder;         // finder object for divisions
   TGeoVoxelFinder   *fVoxels;         // finder object for bounding boxes
   TGeoManager       *fGeoManager;     //! pointer to TGeoManager owning this volume

   TObject           *fField;          //! just a hook for now
   TString            fOption;         //! option - if any
   Int_t              fNumber;         //  volume serial number in the list of volumes
   Int_t              fNtotal;         // total number of physical nodes

   TGeoVolume(const TGeoVolume&); 
   TGeoVolume& operator=(const TGeoVolume&);

public:
   enum EGeoVolumeTypes {
      kVolumeReplicated =  BIT(14),
      kVolumeSelected =    BIT(15),
      kVolumeDiv     =     BIT(16),
      kVolumeOverlap =     BIT(17),
      kVolumeImportNodes = BIT(18),
      kVolumeMulti   =     BIT(19),
      kVoxelsXYZ     =     BIT(20),
      kVoxelsCyl     =     BIT(21),
      kVolumeClone   =     BIT(22),
      kVolumeAdded   =     BIT(23)
   };
   // constructors
   TGeoVolume();
   TGeoVolume(const char *name, const TGeoShape *shape, const TGeoMedium *med=0);

   // destructor
   virtual ~TGeoVolume();
   // methods
   virtual void    cd(Int_t inode) const;
   void            Browse(TBrowser *b);
   Double_t        Capacity() const;
   void            CheckShapes();
   void            ClearNodes() {fNodes = 0;}
   void            ClearShape();
   void            CleanAll();
   virtual TGeoVolume *CloneVolume() const;
   void            CloneNodesAndConnect(TGeoVolume *newmother) const;
   void            CheckGeometry(Int_t nrays=1, Double_t startx=0, Double_t starty=0, Double_t startz=0) const;
   void            CheckOverlaps(Double_t ovlp=0.1, Option_t *option="") const; // *MENU*
   Int_t           CountNodes(Int_t nlevels=1000, Int_t option=0);
   Bool_t          Contains(Double_t *point) const {return fShape->Contains(point);}
   virtual Bool_t  IsAssembly() const {return kFALSE;}
   virtual Bool_t  IsFolder() const;
   Bool_t          IsRunTime() const {return fShape->IsRunTimeShape();}
   virtual Bool_t  IsVolumeMulti() const {return kFALSE;}
   virtual void    AddNode(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat=0, Option_t *option="");       // most general case
   void            AddNodeOffset(const TGeoVolume *vol, Int_t copy_no, Double_t offset=0, Option_t *option="");
   virtual void    AddNodeOverlap(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat=0, Option_t *option="");

   virtual TGeoVolume *Divide(const char *divname, Int_t iaxis, Int_t ndiv, Double_t start, Double_t step, Int_t numed=0, Option_t *option="");
   virtual Int_t   DistancetoPrimitive(Int_t px, Int_t py);
   virtual void    Draw(Option_t *option=""); // *MENU*
   virtual void    DrawOnly(Option_t *option=""); // *MENU*
   TH2F           *LegoPlot(Int_t ntheta=20, Double_t themin=0., Double_t themax=180.,
                            Int_t nphi=60, Double_t phimin=0., Double_t phimax=360.,
                            Double_t rmin=0., Double_t rmax=9999999, Option_t *option=""); // *MENU*
   virtual void    Paint(Option_t *option="");
   void            PrintNodes() const;
   void            PrintVoxels() const; // *MENU*
   virtual void    ExecuteEvent(Int_t event, Int_t px, Int_t py);

   Bool_t          IsActive() const {return TGeoAtt::IsActive();}
   Bool_t          IsActiveDaughters() const {return TGeoAtt::IsActiveDaughters();}
   Bool_t          IsAdded()     const {return TObject::TestBit(kVolumeAdded);}
   Bool_t          IsReplicated() const {return TObject::TestBit(kVolumeReplicated);}
   Bool_t          IsSelected() const  {return TObject::TestBit(kVolumeSelected);}
   Bool_t          IsCylVoxels() const {return TObject::TestBit(kVoxelsCyl);}
   Bool_t          IsXYZVoxels() const {return TObject::TestBit(kVoxelsXYZ);}
   Bool_t          IsTopVolume() const;
   Bool_t          IsValid() const {return fShape->IsValid();}
   virtual Bool_t  IsVisible() const {return TGeoAtt::IsVisible();}
   Bool_t          IsVisibleDaughters() const {return TGeoAtt::IsVisDaughters();}
   Bool_t          IsVisContainers() const {return TGeoAtt::IsVisContainers();}
   Bool_t          IsVisLeaves() const {return TGeoAtt::IsVisLeaves();}
   Bool_t          IsVisOnly() const {return TGeoAtt::IsVisOnly();}
   Bool_t          IsAllInvisible() const;
   Bool_t          IsRaytracing() const;
   static TGeoVolume *Import(const char *filename, const char *name="", Option_t *option="");
   Int_t           Export(const char *filename, const char *name="", Option_t *option="");
   TGeoNode       *FindNode(const char *name) const;
   void            FindOverlaps() const;
   Bool_t          FindMatrixOfDaughterVolume(TGeoVolume *vol) const;
   virtual Int_t   GetCurrentNodeIndex() const {return -1;}
   virtual Int_t   GetNextNodeIndex() const {return -1;}
   TObjArray      *GetNodes() {return fNodes;}
   Int_t           GetNdaughters() const;
   Int_t           GetNtotal() const {return fNtotal;}
   virtual Int_t   GetByteCount() const;
   TGeoManager    *GetGeoManager() const {return fGeoManager;}
   TGeoMaterial   *GetMaterial() const               {return fMedium->GetMaterial();}
   TGeoMedium     *GetMedium() const                 {return fMedium;}
   TObject        *GetField() const                  {return fField;}
   TGeoPatternFinder *GetFinder() const              {return fFinder;}
   TGeoVoxelFinder   *GetVoxels() const              {return (fVoxels && !fVoxels->IsInvalid())?fVoxels:NULL;}
   const char     *GetIconName() const               {return fShape->GetName();}
   Int_t           GetIndex(const TGeoNode *node) const;
   TGeoNode       *GetNode(const char *name) const;
   TGeoNode       *GetNode(Int_t i) const {return (TGeoNode*)fNodes->UncheckedAt(i);}
   Int_t           GetNodeIndex(const TGeoNode *node, Int_t *check_list, Int_t ncheck) const;
   Int_t           GetNumber() const {return fNumber;}
   virtual char   *GetObjectInfo(Int_t px, Int_t py) const;
   Bool_t          GetOptimalVoxels() const;
   Option_t       *GetOption() const { return fOption.Data(); }
   char           *GetPointerName() const;
   Char_t          GetTransparency() const {return (fMedium==0)?0:(fMedium->GetMaterial()->GetTransparency());}
   TGeoShape      *GetShape() const                  {return fShape;}
   void            GrabFocus(); // *MENU*
   void            Gsord(Int_t /*iaxis*/)                {;}
   Bool_t          IsStyleDefault() const;
   void            InspectMaterial() const; // *MENU*
   void            InspectShape() const {fShape->InspectShape();} // *MENU*
   virtual TGeoVolume *MakeCopyVolume(TGeoShape *newshape);
   void            MakeCopyNodes(const TGeoVolume *other);
   TGeoVolume     *MakeReflectedVolume(const char *newname="") const;
   Bool_t          OptimizeVoxels(); // *MENU*
   void            RandomPoints(Int_t npoints=1000000, Option_t *option=""); // *MENU*
   void            RandomRays(Int_t nrays=10000, Double_t startx=0, Double_t starty=0, Double_t startz=0); // *MENU*
   void            Raytrace(Bool_t flag=kTRUE); // *TOGGLE* *GETTER=IsRaytracing
   void            RegisterYourself(Option_t *option="");
   void            RemoveNode(TGeoNode *node);
   TGeoNode       *ReplaceNode(TGeoNode *nodeorig, TGeoShape *newshape=0, TGeoMatrix *newpos=0, TGeoMedium *newmed=0);
   void            SaveAs(const char *filename,Option_t *option="") const; // *MENU*
   virtual void    SavePrimitive(ostream &out, Option_t *option = "");
   void            SelectVolume(Bool_t clear = kFALSE);
   void            SetActivity(Bool_t flag=kTRUE) {TGeoAtt::SetActivity(flag);}
   void            SetActiveDaughters(Bool_t flag=kTRUE) {TGeoAtt::SetActiveDaughters(flag);}
   void            SetAsTopVolume(); // *TOGGLE* *GETTER=IsTopVolume
   void            SetAdded()      {TObject::SetBit(kVolumeAdded);}
   void            SetReplicated() {TObject::SetBit(kVolumeReplicated);}
   void            SetCurrentPoint(Double_t x, Double_t y, Double_t z);
   void            SetCylVoxels(Bool_t flag=kTRUE) {TObject::SetBit(kVoxelsCyl, flag); TObject::SetBit(kVoxelsXYZ, !flag);}
   void            SetNodes(TObjArray *nodes) {fNodes = nodes; TObject::SetBit(kVolumeImportNodes);}
   void            SetShape(const TGeoShape *shape);
   void            SetTransparency(Char_t transparency=0) {if (fMedium) fMedium->GetMaterial()->SetTransparency(transparency);} // *MENU*
   void            SetField(const TObject *field)          {fField = (TObject*)field;}
   void            SetOption(const char *option);
   void            SetAttVisibility(Bool_t vis) {TGeoAtt::SetVisibility(vis);}
   virtual void    SetVisibility(Bool_t vis=kTRUE); // *TOGGLE* *GETTER=IsVisible
   virtual void    SetVisContainers(Bool_t flag=kTRUE); // *TOGGLE* *GETTER=IsVisContainers
   virtual void    SetVisLeaves(Bool_t flag=kTRUE); // *TOGGLE* *GETTER=IsVisLeaves
   virtual void    SetVisOnly(Bool_t flag=kTRUE); // *TOGGLE* *GETTER=IsVisOnly
   virtual void    SetLineColor(Color_t lcolor);
   virtual void    SetLineStyle(Style_t lstyle);
   virtual void    SetLineWidth(Width_t lwidth);
   void            SetInvisible() {SetVisibility(kFALSE);}
   virtual void    SetMedium(const TGeoMedium *medium) {fMedium = (TGeoMedium*)medium;}
   void            SetVoxelFinder(const TGeoVoxelFinder *finder) {fVoxels=(TGeoVoxelFinder*)finder;}
   void            SetFinder(const TGeoPatternFinder *finder) {fFinder=(TGeoPatternFinder*)finder;}
   void            SetNumber(Int_t number) {fNumber = number;}
   void            SetNtotal(Int_t ntotal) {fNtotal = ntotal;}
   void            SortNodes();
   void            UnmarkSaved();
   Bool_t          Valid() const;
   void            VisibleDaughters(Bool_t vis=kTRUE); // *TOGGLE* *GETTER=IsVisibleDaughters
   void            InvisibleAll(Bool_t flag=kTRUE); // *TOGGLE* *GETTER=IsAllInvisible
   void            Voxelize(Option_t *option);
   Double_t        Weight(Double_t precision=0.01, Option_t *option="va"); // *MENU*
   Double_t        WeightA() const;

   ClassDef(TGeoVolume, 5)              // geometry volume descriptor
};

/*************************************************************************
 * TGeoVolumeMulti - class storing a list of volumes that have to
 *   be handled togeather at build time
 *
 *************************************************************************/

class TGeoVolumeMulti : public TGeoVolume
{
private:
   TObjArray       *fVolumes;      // list of volumes
   TGeoVolumeMulti *fDivision;     // division of this volume
   Int_t            fNumed;        // medium number for divisions
   Int_t            fNdiv;         // number of divisions
   Int_t            fAxis;         // axis of division
   Double_t         fStart;        // division start offset
   Double_t         fStep;         // division step
   Bool_t           fAttSet;       // flag attributes set

protected:
   TGeoVolumeMulti(const TGeoVolumeMulti&); 
   TGeoVolumeMulti& operator=(const TGeoVolumeMulti&);

public:
   TGeoVolumeMulti();
   TGeoVolumeMulti(const char* name, const TGeoMedium *med=0);
   virtual ~TGeoVolumeMulti();

   void            AddVolume(TGeoVolume *vol);
   TGeoVolume     *GetVolume(Int_t id) const {return (TGeoVolume*)fVolumes->At(id);}
   virtual void    AddNode(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat, Option_t *option="");       // most general case
   virtual void    AddNodeOverlap(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat, Option_t *option="");
   virtual TGeoVolume *Divide(const char *divname, Int_t iaxis, Int_t ndiv, Double_t start, Double_t step, Int_t numed=0, Option_t *option="");
   TGeoShape      *GetLastShape() const {return GetVolume(fVolumes->GetEntriesFast()-1)->GetShape();}
   Int_t           GetNvolumes() const {return fVolumes->GetEntriesFast();}
   Int_t           GetAxis() const {return fNdiv;}
   Int_t           GetNdiv() const {return fNdiv;}
   Double_t        GetStart() const {return fStart;}
   Double_t        GetStep() const {return fStep;}
   virtual Bool_t  IsVolumeMulti() const {return kTRUE;}
   virtual TGeoVolume *MakeCopyVolume(TGeoShape *newshape);
   virtual void    SetLineColor(Color_t lcolor);
   virtual void    SetLineStyle(Style_t lstyle);
   virtual void    SetLineWidth(Width_t lwidth);
   virtual void    SetMedium(const TGeoMedium *medium);
   virtual void    SetVisibility(Bool_t vis=kTRUE);


   ClassDef(TGeoVolumeMulti, 3)     // class to handle multiple volumes in one step
};

/*************************************************************************
 * TGeoVolumeAssembly - special assembly of volumes
 *  
 *
 *************************************************************************/

class TGeoVolumeAssembly : public TGeoVolume
{
private:
   Int_t           fCurrent;              //! index of current selected node
   Int_t           fNext;                 //! index of next node to be entered
public:
   TGeoVolumeAssembly();
   TGeoVolumeAssembly(const char *name);
   virtual ~TGeoVolumeAssembly();

   virtual void    AddNode(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat=0, Option_t *option=""); 
   virtual void    AddNodeOverlap(const TGeoVolume *vol, Int_t copy_no, TGeoMatrix *mat, Option_t *option);
   virtual TGeoVolume *CloneVolume() const;
   virtual TGeoVolume *Divide(const char *divname, Int_t iaxis, Int_t ndiv, Double_t start, Double_t step, Int_t numed=0, Option_t *option="");
   virtual void    DrawOnly(Option_t *) {;} 
   virtual Int_t   GetCurrentNodeIndex() const {return fCurrent;}
   virtual Int_t   GetNextNodeIndex() const {return fNext;}
   virtual Bool_t  IsAssembly() const {return kTRUE;}
   virtual Bool_t  IsVisible() const {return kFALSE;}
   static TGeoVolumeAssembly *MakeAssemblyFromVolume(TGeoVolume *vol);
   void            SetCurrentNodeIndex(Int_t index) {fCurrent = index;}
   void            SetNextNodeIndex(Int_t index) {fNext = index;}

   ClassDef(TGeoVolumeAssembly, 2)   // an assembly of volumes
};

inline Int_t TGeoVolume::GetNdaughters() const {if (!fNodes) return 0; return (fNodes->GetEntriesFast());}

#endif

