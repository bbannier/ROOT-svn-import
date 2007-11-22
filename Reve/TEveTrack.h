// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TEveTrack
#define ROOT_TEveTrack

#include <vector>

#include <TEveVSDStructs.h>
#include <TEveElement.h>
#include <TEveLine.h>

#include <TPolyMarker3D.h>
#include <TMarker.h>
#include <TQObject.h>

class TEveTrackRnrStyle;
class TEveTrackList;

class TEveTrack : public TEveLine,
		  public TQObject
{
   friend class TEveTrackRnrStyle;
   friend class TEveTrackList;
   friend class TEveTrackCounter;
   friend class TEveTrackGL;

public:
   typedef std::vector<TEvePathMark*>           vpPathMark_t;
   typedef std::vector<TEvePathMark*>::iterator vpPathMark_i;

protected:
   TEveVector         fV;          // Starting vertex
   TEveVector         fP;          // Starting momentum
   Double_t           fBeta;       // Relativistic beta factor
   Int_t              fPdg;        // PDG code
   Int_t              fCharge;     // Charge in units of e0
   Int_t              fLabel;      // Simulation label
   Int_t              fIndex;      // Reconstruction index
   vpPathMark_t       fPathMarks;  // TEveVector of known points along the track

   TEveTrackRnrStyle *fRnrStyle;   // Pointer to shared render-style

public:
   TEveTrack();
   TEveTrack(TParticle* t, Int_t label, TEveTrackRnrStyle* rs);
   TEveTrack(TEveMCTrack*  t, TEveTrackRnrStyle* rs);
   TEveTrack(TEveRecTrack* t, TEveTrackRnrStyle* rs);
   TEveTrack(const TEveTrack& t);
   virtual ~TEveTrack();

   virtual void SetStdTitle();

   virtual void SetTrackParams(const TEveTrack& t);
   virtual void SetPathMarks  (const TEveTrack& t);

   virtual void MakeTrack(Bool_t recurse=kTRUE);

   TEveTrackRnrStyle* GetRnrStyle() const  { return fRnrStyle; }
   void SetRnrStyle(TEveTrackRnrStyle* rs);
   void SetAttLineAttMarker(TEveTrackList* tl);

   Int_t GetPdg()    const   { return fPdg;   }
   void SetPdg(Int_t pdg)    { fPdg = pdg;    }
   Int_t GetCharge() const   { return fCharge; }
   void SetCharge(Int_t chg) { fCharge = chg; }
   Int_t GetLabel()  const   { return fLabel; }
   void  SetLabel(Int_t lbl) { fLabel = lbl;  }
   Int_t GetIndex()  const   { return fIndex; }
   void  SetIndex(Int_t idx) { fIndex = idx;  }

   void          AddPathMark(TEvePathMark* pm) { fPathMarks.push_back(pm); }
   vpPathMark_t& GetPathMarksRef()               { return fPathMarks; }
   const vpPathMark_t& GetPathMarksRef() const   { return fPathMarks; }
   void          SortPathMarksByTime();

   //--------------------------------

   void ImportHits();              // *MENU*
   void ImportClusters();          // *MENU*
   void ImportClustersFromIndex(); // *MENU*
   void ImportKine();              // *MENU*
   void ImportKineWithArgs(Bool_t importMother=kTRUE, Bool_t impDaugters=kTRUE,
                           Bool_t colorPdg    =kTRUE, Bool_t recurse    =kTRUE); // *MENU*
   void PrintKineStack();          // *MENU*
   void PrintPathMarks();          // *MENU*

   //--------------------------------

   virtual void CtrlClicked(TEveTrack*); // *SIGNAL*
   virtual void SetLineStyle(Style_t lstyle);

   virtual const TGPicture* GetListTreeIcon() { return fgListTreeIcons[4]; };

   virtual TClass* ProjectedClass() const;

   ClassDef(TEveTrack, 1); // Track with given vertex, momentum and optional referece-points (path-marks) along its path.
};


/******************************************************************************/
// TEveTrackRnrStyle
/******************************************************************************/

class TEveTrackRnrStyle : public TObject,
			  public TEveRefBackPtr
{
private:
   void                     RebuildTracks();

public:
   Float_t                  fMagField;      // Constant magnetic field along z.

   // TEveTrack limits
   Float_t                  fMaxR;          // Max radius for track extrapolation
   Float_t                  fMaxZ;          // Max z-coordinate for track extrapolation.
   // Helix limits
   Float_t                  fMaxOrbs;       // Maximal angular path of tracks' orbits (1 ~ 2Pi).
   Float_t                  fMinAng;        // Minimal angular step between two helix points.
   Float_t                  fDelta;         // Maximal error at the mid-point of the line connecting to helix points.

   // Path-mark control
   Bool_t                   fEditPathMarks; // Show widgets for path-mark control in GUI editor.
   TMarker                  fPMAtt;         // Marker attributes for rendering of path-marks.

   Bool_t                   fFitDaughters;  // Pass through daughter creation points when extrapolating a track.
   Bool_t                   fFitReferences; // Pass through given track-references when extrapolating a track.
   Bool_t                   fFitDecay;      // Pass through decay point when extrapolating a track.

   Bool_t                   fRnrDaughters;  // Render daughter path-marks.
   Bool_t                   fRnrReferences; // Render track-reference path-marks.
   Bool_t                   fRnrDecay;      // Render decay path-marks.

   // First vertex control
   Bool_t                   fRnrFV;         // Render first vertex.
   TMarker                  fFVAtt;         // Marker attributes for fits vertex.

   TEveTrackRnrStyle();

   // callbacks
   void   SetEditPathMarks(Bool_t x) { fEditPathMarks = x; }
   void   SetRnrDaughters(Bool_t x);
   void   SetRnrReferences(Bool_t x);
   void   SetRnrDecay(Bool_t x);

   void   SetRnrFV(Bool_t x){  fRnrFV = x;}

   void   SetFitDaughters(Bool_t x);
   void   SetFitReferences(Bool_t x);
   void   SetFitDecay(Bool_t x);

   void   SetMaxR(Float_t x);
   void   SetMaxZ(Float_t x);
   void   SetMaxOrbs(Float_t x);
   void   SetMinAng(Float_t x);
   void   SetDelta(Float_t x);

   Float_t GetMagField() const     { return fMagField; }
   void    SetMagField(Float_t mf) { fMagField = mf; }

   static Float_t       fgDefMagField; // Default value for constant solenoid magnetic field.
   static const Float_t fgkB2C;        // Constant for conversion of momentum to curvature.
   static TEveTrackRnrStyle fgDefStyle;    // Default track render-style.

   ClassDef(TEveTrackRnrStyle, 1); // Extrapolation parameters for tracks.
};


/******************************************************************************/
// TEveTrackList
/******************************************************************************/

class TEveTrackList : public TEveElementList,
		      public TEveProjectable,
		      public TAttMarker,
		      public TAttLine
{
   friend class TEveTrackListEditor;

private:
   TEveTrackList(const TEveTrackList&);            // Not implemented
   TEveTrackList& operator=(const TEveTrackList&); // Not implemented

   Bool_t               fRecurse;    // Recurse when propagating marker/line attributes to tracks.

protected:
   TEveTrackRnrStyle   *fRnrStyle;   // Basic track rendering parameters, not enforced to elements.

   Bool_t               fRnrLine;    // Render track as line.
   Bool_t               fRnrPoints;  // Render track as points.

   Float_t              fMinPt;      // Minimum track pT for display selection.
   Float_t              fMaxPt;      // Maximum track pT for display selection.
   Float_t              fLimPt;      // Highest track pT in the container.
   Float_t              fMinP;       // Minimum track p for display selection.
   Float_t              fMaxP;       // Maximum track p for display selection.
   Float_t              fLimP;       // Highest track p in the container.

   Float_t RoundMomentumLimit(Float_t x);

public:
   TEveTrackList(TEveTrackRnrStyle* rs=0);
   TEveTrackList(const Text_t* name, TEveTrackRnrStyle* rs=0);
   virtual ~TEveTrackList();

   void  MakeTracks(Bool_t recurse=kTRUE);
   void  FindMomentumLimits(TEveElement* el, Bool_t recurse);

   void  SetRnrStyle(TEveTrackRnrStyle* rs);
   TEveTrackRnrStyle*  GetRnrStyle(){return fRnrStyle;}

   //--------------------------------

   virtual void   SetMainColor(Color_t c);
   virtual void   SetLineColor(Color_t c){SetMainColor(c);}
   virtual void   SetLineColor(Color_t c, TEveElement* el);
   virtual void   SetLineWidth(Width_t w);
   virtual void   SetLineWidth(Width_t w, TEveElement* el);
   virtual void   SetLineStyle(Style_t s);
   virtual void   SetLineStyle(Style_t s, TEveElement* el);

   virtual void   SetMarkerColor(Color_t c);
   virtual void   SetMarkerColor(Color_t c, TEveElement* el);
   virtual void   SetMarkerSize(Size_t s);
   virtual void   SetMarkerSize(Size_t s, TEveElement* el);
   virtual void   SetMarkerStyle(Style_t s);
   virtual void   SetMarkerStyle(Style_t s, TEveElement* el);

   void SetRnrLine(Bool_t rnr);
   void SetRnrLine(Bool_t rnr, TEveElement* el);
   Bool_t GetRnrLine(){return fRnrLine;}

   void SetRnrPoints(Bool_t r);
   void SetRnrPoints(Bool_t r, TEveElement* el);
   Bool_t GetRnrPoints(){return fRnrPoints;}

   void SelectByPt(Float_t min_pt, Float_t max_pt);
   void SelectByPt(Float_t min_pt, Float_t max_pt, TEveElement* el);
   void SelectByP (Float_t min_p,  Float_t max_p);
   void SelectByP (Float_t min_p,  Float_t max_p,  TEveElement* el);

   //--------------------------------

   TEveTrack* FindTrackByLabel(Int_t label); // *MENU*
   TEveTrack* FindTrackByIndex(Int_t index); // *MENU*

   void ImportHits();     // *MENU*
   void ImportClusters(); // *MENU*

   virtual TClass* ProjectedClass() const;

   ClassDef(TEveTrackList, 1); // A list of tracks supporting change of common attributes and selection based on track parameters.
};


/******************************************************************************/
// TEveTrackCounter
/******************************************************************************/

class TEveTrackCounter : public TEveElement, public TNamed
{
   friend class TEveTrackCounterEditor;

public:
   enum ClickAction_e { CA_PrintTrackInfo, CA_ToggleTrack };

private:
   TEveTrackCounter(const TEveTrackCounter&);            // Not implemented
   TEveTrackCounter& operator=(const TEveTrackCounter&); // Not implemented

protected:
   Int_t fBadLineStyle;  // TEveLine-style used for secondary/bad tracks.
   Int_t fClickAction;   // Action to take when a track is ctrl-clicked.

   Int_t fEventId;       // Current event-id.

   Int_t fAllTracks;     // Counter of all tracks.
   Int_t fGoodTracks;    // Counter of good tracks.

   TList fTrackLists;    // List of TrackLists registered for management.

public:
   TEveTrackCounter(const Text_t* name="TEveTrackCounter", const Text_t* title="");
   virtual ~TEveTrackCounter();

   Int_t GetEventId() const { return fEventId; }
   void  SetEventId(Int_t id) { fEventId = id; }

   void Reset();

   void RegisterTracks(TEveTrackList* tlist, Bool_t goodTracks);

   void DoTrackAction(TEveTrack* track);

   Int_t GetClickAction() const  { return fClickAction; }
   void  SetClickAction(Int_t a) { fClickAction = a; }

   void OutputEventTracks(FILE* out=0);

   static TEveTrackCounter* fgInstance;

   ClassDef(TEveTrackCounter, 1); // Class for selection of good/primary tracks with basic processing functionality.
};


#endif
