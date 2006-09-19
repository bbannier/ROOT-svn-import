// @(#)root/geom:$Name:  $:$Id: TGeoPhysicalNode.h,v 1.8 2006/05/23 04:47:37 brun Exp $
// Author: Andrei Gheata   17/02/04

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGeoPhysicalNode
#define ROOT_TGeoPhysicalNode

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

#ifndef ROOT_TObjArray
#include "TObjArray.h"
#endif

#ifndef ROOT_TAttLine
#include "TAttLine.h"
#endif

#ifndef ROOT_TObject
#include "TGeoNode.h"
#endif

// forward declarations
class TGeoHMatrix;
class TGeoMatrix;
class TGeoVolume;
class TGeoNode;
class TGeoShape;

/*************************************************************************
 * TGeoPhysicalNode - class representing an unique object associated with a
 *   path.
 *
 *************************************************************************/

class TGeoPhysicalNode : public TObject,
                 public TAttLine
{
protected:
   Int_t             fLevel;          // depth in the geometry tree
   TObjArray        *fMatrices;       // global transformation matrices
   TObjArray        *fNodes;          // branch of nodes
   TGeoHMatrix      *fMatrixOrig;     // original local matrix of the last node in the path

   TGeoPhysicalNode(const TGeoPhysicalNode&); 
   TGeoPhysicalNode& operator=(const TGeoPhysicalNode&);

   void              SetAligned(Bool_t flag=kTRUE) {TObject::SetBit(kGeoPNodeAligned,flag);}

public:
   enum {
      kGeoPNodeFull    = BIT(10),     // full branch is visible (default only last node)
      kGeoPNodeVisible = BIT(11),     // this node is visible (default)
      kGeoPNodeVolAtt  = BIT(12),     // preserve volume attributes (default)
      kGeoPNodeAligned = BIT(13)      // alignment bit
   };

   // constructors
   TGeoPhysicalNode();
   TGeoPhysicalNode(const char *path);
   // destructor
   virtual ~TGeoPhysicalNode();

   void              Align(TGeoMatrix *newmat=0, TGeoShape *newshape=0, Bool_t check=kFALSE);
   void              cd() const;
   void              Draw(Option_t *option="");
   Int_t             GetLevel() const {return fLevel;}
   TGeoHMatrix      *GetMatrix(Int_t level=-1) const;
   TGeoHMatrix      *GetOriginalMatrix() const {return fMatrixOrig;}
   TGeoNode         *GetMother(Int_t levup=1) const;
   const char       *GetName() const;
   TGeoNode         *GetNode(Int_t level=-1) const;
   TGeoShape        *GetShape(Int_t level=-1) const;
   TGeoVolume       *GetVolume(Int_t level=-1) const;
   
 
   Bool_t            IsAligned() const {return TObject::TestBit(kGeoPNodeAligned);}
   Bool_t            IsVolAttributes() const {return TObject::TestBit(kGeoPNodeVolAtt);}
   Bool_t            IsVisible() const {return TObject::TestBit(kGeoPNodeVisible);}
   Bool_t            IsVisibleFull() const {return TObject::TestBit(kGeoPNodeFull);}

   Bool_t            SetPath(const char *path);
   void              SetBranchAsState();

   void              SetIsVolAtt(Bool_t flag=kTRUE) {TObject::SetBit(kGeoPNodeVolAtt,flag);}
   void              SetVisibility(Bool_t flag=kTRUE)  {TObject::SetBit(kGeoPNodeVisible,flag);}
   void              SetVisibleFull(Bool_t flag=kTRUE) {TObject::SetBit(kGeoPNodeFull,flag);}
   virtual void      Paint(Option_t *option = "");


   ClassDef(TGeoPhysicalNode, 1)               // base class for physical nodes
};

/*************************************************************************
 * TGeoPNEntry - class representing phisical node entry having a unique name
 *   associated to a path.
 *
 *************************************************************************/

class TGeoPNEntry : public TNamed
{
private:
   TGeoPhysicalNode *fNode;        // Physical node to which this applies
   TGeoHMatrix      *fMatrix;      // Additional matrix

protected:
   TGeoPNEntry(const TGeoPNEntry& pne) 
     : TNamed(pne), fNode(pne.fNode), fMatrix(NULL) { }
   TGeoPNEntry& operator=(const TGeoPNEntry& pne)
     {if(this!=&pne) {TNamed::operator=(pne); fNode=pne.fNode; fMatrix=pne.fMatrix;}
     return *this;}

public:
   TGeoPNEntry();
   TGeoPNEntry(const char *unique_name, const char *path);
   virtual ~TGeoPNEntry() {;}
   
   inline const char   *GetPath() const {return GetTitle();}
   TGeoHMatrix      *GetMatrix()  const {return fMatrix;}
   TGeoPhysicalNode *GetPhysicalNode() const {return fNode;}
   void              SetMatrix(TGeoHMatrix *matrix);
   void              SetPhysicalNode(TGeoPhysicalNode *node);
   
   ClassDef(TGeoPNEntry, 2)                  // a physical node entry with unique name
};

#endif

