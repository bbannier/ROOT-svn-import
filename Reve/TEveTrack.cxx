// @(#)root/reve:$Id$
// Authors: Matevz Tadel & Alja Mrak-Tadel: 2006, 2007

/*************************************************************************
 * Copyright (C) 1995-2007, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include <TEveTrack.h>
#include <TEveTrackPropagator.h>
#include <TEvePointSet.h>

#include <TPolyLine3D.h>
#include <TMarker.h>
#include <TPolyMarker3D.h>
#include <TColor.h>

// Updates
#include <TEveManager.h>
#include <TEveBrowser.h>
#include <TEveTrackProjected.h>
#include <TCanvas.h>

#include <vector>
#include <algorithm>
#include <functional>
#include <iostream>

//______________________________________________________________________________
// TEveTrack
//
// Visual representation of a track.
//

ClassImp(TEveTrack)

//______________________________________________________________________________
TEveTrack::TEveTrack() :
   TEveLine(),

   fV(),
   fP(),
   fBeta(0),
   fPdg(0),
   fCharge(0),
   fLabel(kMinInt),
   fIndex(kMinInt),
   fPathMarks(),

   fRnrStyle(0)
{
   // Default constructor.
}

//______________________________________________________________________________
TEveTrack::TEveTrack(TParticle* t, Int_t label, TEveTrackRnrStyle* rs):
   TEveLine(),

   fV(t->Vx(), t->Vy(), t->Vz()),
   fP(t->Px(), t->Py(), t->Pz()),
   fBeta(t->P()/t->Energy()),
   fPdg(0),
   fCharge(0),
   fLabel(label),
   fIndex(kMinInt),
   fPathMarks(),

   fRnrStyle(0)
{
   // Constructor from TParticle.

   SetRnrStyle(rs);
   fMainColorPtr = &fLineColor;

   TParticlePDG* pdgp = t->GetPDG();
   if (pdgp) {
      fPdg    = pdgp->PdgCode();
      fCharge = (Int_t) TMath::Nint(pdgp->Charge()/3);
   }

   SetName(t->GetName());
}

//______________________________________________________________________________
TEveTrack::TEveTrack(TEveMCTrack* t, TEveTrackRnrStyle* rs):
   TEveLine(),

   fV(t->Vx(), t->Vy(), t->Vz()),
   fP(t->Px(), t->Py(), t->Pz()),
   fBeta(t->P()/t->Energy()),
   fPdg(0),
   fCharge(0),
   fLabel(t->label),
   fIndex(t->index),
   fPathMarks(),

   fRnrStyle(0)
{
   // Constructor from TEveUtil Monte Carlo track.

   SetRnrStyle(rs);
   fMainColorPtr = &fLineColor;

   TParticlePDG* pdgp = t->GetPDG();
   if(pdgp == 0) {
      t->ResetPdgCode(); pdgp = t->GetPDG();
   }
   fCharge = (Int_t) TMath::Nint(pdgp->Charge()/3);

   SetName(t->GetName());
}

//______________________________________________________________________________
TEveTrack::TEveTrack(TEveRecTrack* t, TEveTrackRnrStyle* rs) :
   TEveLine(),

   fV(t->V),
   fP(t->P),
   fBeta(t->beta),
   fPdg(0),
   fCharge(t->sign),
   fLabel(t->label),
   fIndex(t->index),
   fPathMarks(),

   fRnrStyle(0)
{
   // Constructor from TEveUtil reconstructed track.

   SetRnrStyle(rs);
   fMainColorPtr = &fLineColor;

   SetName(t->GetName());
}

//______________________________________________________________________________
TEveTrack::TEveTrack(const TEveTrack& t) :
   TEveLine(),
   TQObject(),
   fV(t.fV),
   fP(t.fP),
   fBeta(t.fBeta),
   fPdg(t.fPdg),
   fCharge(t.fCharge),
   fLabel(t.fLabel),
   fIndex(t.fIndex),
   fPathMarks(),
   fRnrStyle(0)
{
   // Copy constructor.

   SetMainColor(t.GetMainColor());
   // TEveLine
   fRnrLine   = t.fRnrLine;
   fRnrPoints = t.fRnrPoints;
   // TLineAttrib
   fLineColor = t.fLineColor;
   fLineStyle = t.fLineStyle;
   fLineWidth = t.fLineWidth;
   SetPathMarks(t);
   SetRnrStyle (t.fRnrStyle);
}

//______________________________________________________________________________
TEveTrack::~TEveTrack()
{
   // Destructor.

   SetRnrStyle(0);
   for (vpPathMark_i i=fPathMarks.begin(); i!=fPathMarks.end(); ++i)
      delete *i;
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::SetStdTitle()
{
   // Set standard track title based on most data-member values.

   TString idx(fIndex == kMinInt ? "<undef>" : Form("%d", fIndex));
   TString lbl(fLabel == kMinInt ? "<undef>" : Form("%d", fLabel));
   SetTitle(Form("Index=%s, Label=%s\nChg=%d, Pdg=%d\n"
                 "pT=%.3f, pZ=%.3f\nV=(%.3f, %.3f, %.3f)",
                 idx.Data(), lbl.Data(), fCharge, fPdg,
                 fP.Perp(), fP.z, fV.x, fV.y, fV.z));
}

//______________________________________________________________________________
void TEveTrack::SetTrackParams(const TEveTrack& t)
{
   // Copy track parameters from t.
   // PathMarks are cleared.

   fV         = t.fV;
   fP         = t.fP;
   fBeta      = t.fBeta;
   fPdg       = t.fPdg;
   fCharge    = t.fCharge;
   fLabel     = t.fLabel;
   fIndex     = t.fIndex;

   SetMainColor(t.GetMainColor());
   // TEveLine
   fRnrLine   = t.fRnrLine;
   fRnrPoints = t.fRnrPoints;
   // TLineAttrib
   fLineColor = t.fLineColor;
   fLineStyle = t.fLineStyle;
   fLineWidth = t.fLineWidth;
   fPathMarks.clear();
   SetRnrStyle(t.fRnrStyle);
}

//______________________________________________________________________________
void TEveTrack::SetPathMarks(const TEveTrack& t)
{
   // Copy path-marks from t.

   const std::vector<TEvePathMark*>& refs = t.GetPathMarksRef();
   for(std::vector<TEvePathMark*>::const_iterator i=refs.begin(); i!=refs.end(); ++i)
   {
      fPathMarks.push_back(new TEvePathMark(**i));
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::SetRnrStyle(TEveTrackRnrStyle* rs)
{
   // Set track's render style.
   // Reference counts of old and new render-style are updated.

   if (fRnrStyle == rs) return;
   if (fRnrStyle) fRnrStyle->DecRefCount(this);
   fRnrStyle = rs;
   if (fRnrStyle) rs->IncRefCount(this);
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::SetAttLineAttMarker(TEveTrackList* tl)
{
   // Set line and marker attributes from TEveTrackList.

   SetLineColor(tl->GetLineColor());
   SetLineStyle(tl->GetLineStyle());
   SetLineWidth(tl->GetLineWidth());

   SetMarkerColor(tl->GetMarkerColor());
   SetMarkerStyle(tl->GetMarkerStyle());
   SetMarkerSize(tl->GetMarkerSize());
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::MakeTrack(Bool_t recurse)
{
   // Calculate track representation based on track data and current
   // settings of the render-style.
   // If recurse is true, descend into children.

   Reset(0);

   TEveTrackRnrStyle& RS((fRnrStyle != 0) ? *fRnrStyle : TEveTrackRnrStyle::fgDefStyle);
   if ((TMath::Abs(fV.z) < RS.fMaxZ) &&
       (fV.x*fV.x + fV.y*fV.y < RS.fMaxR*RS.fMaxR))
   {
      TEveVector currP = fP;
      Bool_t decay = kFALSE;
      TEveTrackPropagator prop(fRnrStyle, fCharge, fV, fP, fBeta);
      for (std::vector<TEvePathMark*>::iterator i=fPathMarks.begin(); i!=fPathMarks.end(); ++i)
      {
         TEvePathMark* pm = *i;
         if (RS.fFitReferences && pm->type == TEvePathMark::Reference)
         {
            if (TMath::Abs(pm->V.z) > RS.fMaxZ ||
               TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR)
               goto bounds;
            // printf("%s fit reference  \n", fName.Data());
            if (prop.GoToVertex(pm->V, currP)) {
               currP.x = pm->P.x; currP.y = pm->P.y; currP.z = pm->P.z;
            }
         }
         else if (RS.fFitDaughters &&  pm->type == TEvePathMark::Daughter)
         {
            if (TMath::Abs(pm->V.z) > RS.fMaxZ ||
                TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR)
               goto bounds;
            // printf("%s fit daughter  \n", fName.Data());
            if (prop.GoToVertex(pm->V, currP)) {
               currP.x -= pm->P.x; currP.y -= pm->P.y; currP.z -= pm->P.z;
            }
         }
         else if (RS.fFitDecay &&  pm->type == TEvePathMark::Decay)
         {
            if (TMath::Abs(pm->V.z) > RS.fMaxZ ||
                TMath::Sqrt(pm->V.x*pm->V.x + pm->V.y*pm->V.y) > RS.fMaxR)
               goto bounds;
            // printf("%s fit decay \n", fName.Data());
            prop.GoToVertex(pm->V, currP);
            decay = true;
            break;
         }
      } // loop path-marks

   bounds:
      if(!decay || RS.fFitDecay == kFALSE)
      {
         // printf("%s loop to bounds  \n",fName.Data() );
         prop.GoToBounds(currP);
      }
      //  make_polyline:
      prop.FillPointSet((TEvePointSet*)this);
   }

   if (recurse)
   {
      for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i)
      {
         TEveTrack* t = dynamic_cast<TEveTrack*>(*i);
         if (t) t->MakeTrack(recurse);
      }
   }
}

/******************************************************************************/

//______________________________________________________________________________
TClass* TEveTrack::ProjectedClass() const
{
   // Virtual from TEveProjectable, return TEveTrackProjected class.

   return TEveTrackProjected::Class();
}

/******************************************************************************/

namespace {

struct cmp_pathmark
{
   bool operator()(TEvePathMark* const & a, TEvePathMark* const & b)
   { return a->time < b->time; }
};

}

//______________________________________________________________________________
void TEveTrack::SortPathMarksByTime()
{
   // Sort registerd pat-marks by time.

   std::sort(fPathMarks.begin(), fPathMarks.end(), cmp_pathmark());
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::ImportHits()
{
   // Import hits with same label as the track.
   // Uses macro "hits_from_label.C".

   TEveUtil::LoadMacro("hits_from_label.C");
   gROOT->ProcessLine(Form("hits_from_label(%d, (TEveElement*)%p);",
                           fLabel, this));
}

//______________________________________________________________________________
void TEveTrack::ImportClusters()
{
   // Import clusters with same label as the track.
   // Uses macro "clusters_from_label.C".

   TEveUtil::LoadMacro("clusters_from_label.C");
   gROOT->ProcessLine(Form("clusters_from_label(%d, (TEveElement*)%p);",
                           fLabel, this));
}

//______________________________________________________________________________
void TEveTrack::ImportClustersFromIndex()
{
   // Import clusters marked with same reconstructed track index as the track.
   // Uses macro "clusters_from_index.C".

   static const TEveException eH("TEveTrack::ImportClustersFromIndex ");

   if (fIndex == kMinInt)
      throw(eH + "index not set.");

   TEveUtil::LoadMacro("clusters_from_index.C");
   gROOT->ProcessLine(Form("clusters_from_index(%d, (TEveElement*)%p);",
                           fIndex, this));
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::ImportKine()
{
   // Import kinematics of the track's label recursively.
   // Uses macro "kine_tracks.C".

   static const TEveException eH("TEveTrack::ImportKine ");

   if (fLabel == kMinInt)
      throw(eH + "label not set.");

   Int_t label;
   if (fLabel < 0) {
      Warning(eH, "label negative, taking absolute value.");
      label = -fLabel;
   } else {
      label = fLabel;
   }

   TEveUtil::LoadMacro("kine_tracks.C");
   gROOT->ProcessLine(Form("kine_track(%d, kTRUE, kTRUE, kTRUE, kTRUE, (TEveElement*)%p);",
                           label, this));

}

//______________________________________________________________________________
void TEveTrack::ImportKineWithArgs(Bool_t importMother, Bool_t importDaugters,
                                   Bool_t colorPdg,     Bool_t recurse)
{
   // Import kinematics of the track's label. Arguments steer the
   // import process:
   //   importMother     import particle with track's label
   //   importDaugters   import direct daughters of label
   //   colorPdg         color kinematics by PDG code
   //   recurse          recursive import of daughters' daughters
   // Uses macro "kine_tracks.C".

   static const TEveException eH("TEveTrack::ImportKineWithArgs ");

   if (fLabel == kMinInt)
      throw(eH + "label not set.");

   Int_t label;
   if (fLabel < 0) {
      Warning(eH, "label negative, taking absolute value.");
      label = -fLabel;
   } else {
      label = fLabel;
   }

   TEveUtil::LoadMacro("kine_tracks.C");
   gROOT->ProcessLine(Form("kine_track(%d, %d, %d, %d, %d, (TEveElement*)%p);",
                           label, importMother, importDaugters, colorPdg, recurse, this));
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::PrintKineStack()
{
   // Print kinematics pertaining to track's label.
   // Uses macro "print_kine_from_label.C".

   static const TEveException eH("TEveTrack::PrintKineStack ");

   if (fLabel == kMinInt)
      throw(eH + "label not set.");

   Int_t label;
   if (fLabel < 0) {
      Warning(eH, "label negative, taking absolute value.");
      label = -fLabel;
   } else {
      label = fLabel;
   }

   TEveUtil::LoadMacro("print_kine_from_label.C");
   gROOT->ProcessLine(Form("print_kine_from_label(%d);", label));
}

//______________________________________________________________________________
void TEveTrack::PrintPathMarks()
{
   // Print registered path-marks.

   static const TEveException eH("TEveTrack::PrintPathMarks ");

   printf("TEveTrack '%s', number of path marks %d, label %d\n",
          GetName(), fPathMarks.size(), fLabel);

   TEvePathMark* pm;
   for(vpPathMark_i i=fPathMarks.begin(); i!=fPathMarks.end(); i++)
   {
      pm = *i;
      printf("  %-9s  p: %8f %8f %8f Vertex: %8e %8e %8e %g \n",
             pm->type_name(),
             pm->P.x,  pm->P.y, pm->P.z,
             pm->V.x,  pm->V.y, pm->V.z,
             pm->time);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrack::CtrlClicked(TEveTrack* track)
{
   // Emits "CtrlClicked(TEveTrack*)" signal.
   // Called from TEveTrackGL on secondary-selection.

   Emit("CtrlClicked(TEveTrack*)", (Long_t)track);
}

//______________________________________________________________________________
void TEveTrack::SetLineStyle(Style_t lstyle)
{
   // Set line-style of the track.
   // The style is propagated to projected tracks.

   TAttLine::SetLineStyle(lstyle);
   std::list<TEveProjected*>::iterator pi = fProjectedList.begin();
   while (pi != fProjectedList.end())
   {
      TEveTrack* pt = dynamic_cast<TEveTrack*>(*pi);
      if (pt)
      {
         pt->SetLineStyle(lstyle);
         pt->ElementChanged();
      }
      ++pi;
   }
}


/******************************************************************************/
/******************************************************************************/

//______________________________________________________________________________
// TEveTrackRnrStyle
//
// Holding structure for a number of track rendering parameters.
//
// This is decoupled from TEveTrack/TEveTrackList to allow sharing of the
// RnrStyle among several instances. Back references are kept so the
// tracks can be recreated when the parameters change.
//
// TEveTrackList has Get/Set methods for RnrStlye. TEveTrackEditor and
// TEveTrackListEditor provide editor access.

ClassImp(TEveTrackRnrStyle)

Float_t       TEveTrackRnrStyle::fgDefMagField = 5;
const Float_t TEveTrackRnrStyle::fgkB2C        = 0.299792458e-3;
TEveTrackRnrStyle TEveTrackRnrStyle::fgDefStyle;

//______________________________________________________________________________
TEveTrackRnrStyle::TEveTrackRnrStyle() :
   TObject(),
   TEveRefBackPtr(),

   fMagField(fgDefMagField),

   fMaxR  (350),
   fMaxZ  (450),

   fMaxOrbs (0.5),
   fMinAng  (45),
   fDelta   (0.1),

   fEditPathMarks(kFALSE),
   fPMAtt(),

   fFitDaughters  (kTRUE),
   fFitReferences (kTRUE),
   fFitDecay      (kTRUE),

   fRnrDaughters  (kTRUE),
   fRnrReferences (kTRUE),
   fRnrDecay      (kTRUE),

   fRnrFV(kFALSE),
   fFVAtt()
{
   // Default constructor.

   fPMAtt.SetMarkerColor(4);
   fPMAtt.SetMarkerStyle(2);

   fFVAtt.SetMarkerSize(0.6);
   fFVAtt.SetMarkerColor(4);
   fFVAtt.SetMarkerStyle(2);
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackRnrStyle::RebuildTracks()
{
   // Rebuild all tracks using this render-style.

   TEveTrack* track;
   std::list<TEveElement*>::iterator i = fBackRefs.begin();
   while (i != fBackRefs.end())
   {
      track = dynamic_cast<TEveTrack*>(*i);
      track->MakeTrack();
      track->ElementChanged();
      ++i;
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackRnrStyle::SetMaxR(Float_t x)
{
   // Set maximum radius and rebuild tracks.

   fMaxR = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetMaxZ(Float_t x)
{
   // Set maximum z and rebuild tracks.

   fMaxZ = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetMaxOrbs(Float_t x)
{
   // Set maximum number of orbits and rebuild tracks.

   fMaxOrbs = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetMinAng(Float_t x)
{
   // Set minimum step angle and rebuild tracks.

   fMinAng = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetDelta(Float_t x)
{
   // Set maximum error and rebuild tracks.

   fDelta = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetFitDaughters(Bool_t x)
{
   // Set daughter creation point fitting and rebuild tracks.

   fFitDaughters = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetFitReferences(Bool_t x)
{
   // Set track-reference fitting and rebuild tracks.

   fFitReferences = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetFitDecay(Bool_t x)
{
   // Set decay fitting and rebuild tracks.

   fFitDecay = x;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetRnrDecay(Bool_t rnr)
{
   // Set decay rendering and rebuild tracks.

   fRnrDecay = rnr;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetRnrDaughters(Bool_t rnr)
{
   // Set daughter rendering and rebuild tracks.

   fRnrDaughters = rnr;
   RebuildTracks();
}

//______________________________________________________________________________
void TEveTrackRnrStyle::SetRnrReferences(Bool_t rnr)
{
   // Set track-reference rendering and rebuild tracks.

   fRnrReferences = rnr;
   RebuildTracks();
}


/******************************************************************************/
/******************************************************************************/

//______________________________________________________________________________
// TEveTrackList
//
// A list of tracks supporting change of common attributes and
// selection based on track parameters.

ClassImp(TEveTrackList)

//______________________________________________________________________________
TEveTrackList::TEveTrackList(TEveTrackRnrStyle* rs) :
   TEveElementList(),
   TAttMarker(1, 20, 1),
   TAttLine(1,1,1),

   fRecurse(kTRUE),
   fRnrStyle(0),
   fRnrLine(kTRUE),
   fRnrPoints(kFALSE),

   fMinPt (0), fMaxPt (0), fLimPt (0),
   fMinP  (0), fMaxP  (0), fLimP  (0)
{
   // Constructor. If TrackRenderStyle argument is 0, a new default
   // render-style is created.

   fChildClass = TEveTrack::Class(); // override member from base TEveElementList

   fMainColorPtr = &fLineColor;
   if (fRnrStyle== 0) rs = new TEveTrackRnrStyle;
   SetRnrStyle(rs);
}

//______________________________________________________________________________
TEveTrackList::TEveTrackList(const Text_t* name, TEveTrackRnrStyle* rs) :
   TEveElementList(name),
   TAttMarker(1, 20, 1),
   TAttLine(1,1,1),

   fRecurse(kTRUE),
   fRnrStyle      (0),
   fRnrLine(kTRUE),
   fRnrPoints(kFALSE),

   fMinPt (0), fMaxPt (0), fLimPt (0),
   fMinP  (0), fMaxP  (0), fLimP  (0)
{
   // Constructor. If TrackRenderStyle argument is 0, a new default
   // render-style is created.

   fChildClass = TEveTrack::Class(); // override member from base TEveElementList

   fMainColorPtr = &fLineColor;
   if (fRnrStyle== 0) rs = new TEveTrackRnrStyle;
   SetRnrStyle(rs);
}

//______________________________________________________________________________
TEveTrackList::~TEveTrackList()
{
   // Destructor.

   SetRnrStyle(0);
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetRnrStyle(TEveTrackRnrStyle* rs)
{
   // Set default render-style for tracks.
   // This is not enforced onto the tracks themselves but this is the
   // render-style that is show in the TEveTrackListEditor.

   if (fRnrStyle == rs) return;
   if (fRnrStyle) fRnrStyle->DecRefCount();
   fRnrStyle = rs;
   if (fRnrStyle) rs->IncRefCount();
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::MakeTracks(Bool_t recurse)
{
   // Regenerate the visual representations of tracks.
   // The momentum limits are rescanned during the same traversal.

   fLimPt = fLimP = 0;

   for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      track->MakeTrack(recurse);

      fLimPt = TMath::Max(fLimPt, track->fP.Perp());
      fLimP  = TMath::Max(fLimP,  track->fP.Mag());
      if (recurse)
         FindMomentumLimits(*i, recurse);
   }

   fLimPt = RoundMomentumLimit(fLimPt);
   fLimP  = RoundMomentumLimit(fLimP);
   if (fMaxPt == 0) fMaxPt = fLimPt;
   if (fMaxP  == 0) fMaxP  = fLimP;

   gReve->Redraw3D();
}

//______________________________________________________________________________
void TEveTrackList::FindMomentumLimits(TEveElement* el, Bool_t recurse)
{
   // Loop over track elements of argument el and find highest pT and p.
   // These are stored in members fLimPt and fLimP.

   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      TEveTrack* track = dynamic_cast<TEveTrack*>(*i);
      if (track)
      {
         fLimPt = TMath::Max(fLimPt, track->fP.Perp());
         fLimP  = TMath::Max(fLimP,  track->fP.Mag());
         if (recurse)
            FindMomentumLimits(*i, recurse);
      }
   }
}

//______________________________________________________________________________
Float_t TEveTrackList::RoundMomentumLimit(Float_t x)
{
   // Round the momentum limit up to a nice value.

   using namespace TMath;
   Double_t fac = Power(10, 1 - Floor(Log10(x)));
   return Ceil(fac*x) / fac;
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetRnrLine(Bool_t rnr)
{
   // Set rendering of track as line for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetRnrLine() == fRnrLine)
         track->SetRnrLine(rnr);
      if (fRecurse)
         SetRnrLine(rnr, *i);
   }
   fRnrLine = rnr;
}

//______________________________________________________________________________
void TEveTrackList::SetRnrLine(Bool_t rnr, TEveElement* el)
{
   // Set rendering of track as line for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && (track->GetRnrLine() == fRnrLine))
         track->SetRnrLine(rnr);
      if (fRecurse)
         SetRnrLine(rnr, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetRnrPoints(Bool_t rnr)
{
   // Set rendering of track as points for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetRnrPoints() == fRnrPoints)
         track->SetRnrPoints(rnr);
      if (fRecurse)
         SetRnrPoints(rnr, *i);
   }
   fRnrPoints = rnr;
}

//______________________________________________________________________________
void TEveTrackList::SetRnrPoints(Bool_t rnr, TEveElement* el)
{
   // Set rendering of track as points for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track)
         if (track->GetRnrPoints() == fRnrPoints)
            track->SetRnrPoints(rnr);
      if (fRecurse)
         SetRnrPoints(rnr, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetMainColor(Color_t col)
{
   // Set main (line) color for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetLineColor() == fLineColor)
         track->SetLineColor(col);
      if (fRecurse)
         SetLineColor(col, *i);
   }
   TEveElement::SetMainColor(col);
}

//______________________________________________________________________________
void TEveTrackList::SetLineColor(Color_t col, TEveElement* el)
{
   // Set line color for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetLineColor() == fLineColor)
         track->SetLineColor(col);
      if (fRecurse)
         SetLineColor(col, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetLineWidth(Width_t width)
{
   // Set line width for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetLineWidth() == fLineWidth)
         track->SetLineWidth(width);
      if (fRecurse)
         SetLineWidth(width, *i);
   }
   fLineWidth=width;
}

//______________________________________________________________________________
void TEveTrackList::SetLineWidth(Width_t width, TEveElement* el)
{
   // Set line width for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetLineWidth() == fLineWidth)
         track->SetLineWidth(width);
      if (fRecurse)
         SetLineWidth(width, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetLineStyle(Style_t style)
{
   // Set line style for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetLineStyle() == fLineStyle)
         track->SetLineStyle(style);
      if (fRecurse)
         SetLineStyle(style, *i);
   }
   fLineStyle=style;
}

//______________________________________________________________________________
void TEveTrackList::SetLineStyle(Style_t style, TEveElement* el)
{
   // Set line style for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetLineStyle() == fLineStyle)
         track->SetLineStyle(style);
      if (fRecurse)
         SetLineStyle(style, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetMarkerStyle(Style_t style)
{
   // Set marker style for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetMarkerStyle() == fMarkerStyle)
         track->SetMarkerStyle(style);
      if (fRecurse)
         SetMarkerStyle(style, *i);
   }
   fMarkerStyle=style;
}

//______________________________________________________________________________
void TEveTrackList::SetMarkerStyle(Style_t style, TEveElement* el)
{
   // Set marker style for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetMarkerStyle() == fMarkerStyle)
         track->SetMarkerStyle(style);
      if(fRecurse)
         SetMarkerStyle(style, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetMarkerColor(Color_t col)
{
   // Set marker color for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetMarkerColor() == fMarkerColor)
         track->SetMarkerColor(col);
      if (fRecurse)
         SetMarkerColor(col, *i);
   }
   fMarkerColor=col;
}

//______________________________________________________________________________
void TEveTrackList::SetMarkerColor(Color_t col, TEveElement* el)
{
   // Set marker color for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetMarkerColor() == fMarkerColor)
         track->SetMarkerColor(col);
      if (fRecurse)
         SetMarkerColor(col, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SetMarkerSize(Size_t size)
{
   // Set marker size for the list and the elements.

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      TEveTrack* track = (TEveTrack*)(*i);
      if (track->GetMarkerSize() == fMarkerSize)
         track->SetMarkerSize(size);
      if (fRecurse)
         SetMarkerSize(size, *i);
   }
   fMarkerSize=size;
}

//______________________________________________________________________________
void TEveTrackList::SetMarkerSize(Size_t size, TEveElement* el)
{
   // Set marker size for children of el.

   TEveTrack* track;
   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      track = dynamic_cast<TEveTrack*>(*i);
      if (track && track->GetMarkerSize() == fMarkerSize)
         track->SetMarkerSize(size);
      if (fRecurse)
         SetMarkerSize(size, *i);
   }
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackList::SelectByPt(Float_t min_pt, Float_t max_pt)
{
   // Select visibility of tracks by transverse momentum.
   // If data-member fRecurse is set, the selection is applied
   // recursively to all children.

   fMinPt = min_pt;
   fMaxPt = max_pt;

   const Float_t minptsq = min_pt*min_pt;
   const Float_t maxptsq = max_pt*max_pt;

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      const Float_t ptsq = ((TEveTrack*)(*i))->fP.Perp2();
      Bool_t on = ptsq >= minptsq && ptsq <= maxptsq;
      (*i)->SetRnrState(on);
      if (on && fRecurse)
         SelectByPt(min_pt, max_pt, *i);
   }
}

//______________________________________________________________________________
void TEveTrackList::SelectByPt(Float_t min_pt, Float_t max_pt, TEveElement* el)
{
   // Select visibility of el's children tracks by transverse momentum.

   const Float_t minptsq = min_pt*min_pt;
   const Float_t maxptsq = max_pt*max_pt;

   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      TEveTrack* track = dynamic_cast<TEveTrack*>(*i);
      if (track)
      {
         const Float_t ptsq = track->fP.Perp2();
         Bool_t on = ptsq >= minptsq && ptsq <= maxptsq;
         track->SetRnrState(on);
         if (on && fRecurse)
            SelectByPt(min_pt, max_pt, *i);
      }
   }
}

//______________________________________________________________________________
void TEveTrackList::SelectByP(Float_t min_p, Float_t max_p)
{
   // Select visibility of tracks by momentum.
   // If data-member fRecurse is set, the selection is applied
   // recursively to all children.

   fMinP = min_p;
   fMaxP = max_p;

   const Float_t minpsq = min_p*min_p;
   const Float_t maxpsq = max_p*max_p;

   for (List_i i=BeginChildren(); i!=EndChildren(); ++i)
   {
      const Float_t psq  = ((TEveTrack*)(*i))->fP.Mag2();
      Bool_t on = psq >= minpsq && psq <= maxpsq;
      (*i)->SetRnrState(psq >= minpsq && psq <= maxpsq);
      if (on && fRecurse)
         SelectByP(min_p, max_p, *i);
   }
}

//______________________________________________________________________________
void TEveTrackList::SelectByP(Float_t min_p, Float_t max_p, TEveElement* el)
{
   // Select visibility of el's children tracks by momentum.

   const Float_t minpsq = min_p*min_p;
   const Float_t maxpsq = max_p*max_p;

   for (List_i i=el->BeginChildren(); i!=el->EndChildren(); ++i)
   {
      TEveTrack* track = dynamic_cast<TEveTrack*>(*i);
      if (track)
      {
         const Float_t psq  = ((TEveTrack*)(*i))->fP.Mag2();
         Bool_t on = psq >= minpsq && psq <= maxpsq;
         track->SetRnrState(on);
         if (on && fRecurse)
            SelectByP(min_p, max_p, *i);
      }
   }
}

/******************************************************************************/

//______________________________________________________________________________
TEveTrack* TEveTrackList::FindTrackByLabel(Int_t label)
{
   // Find track by label, select it and display it in the editor.

   for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i) {
      if (((TEveTrack*)(*i))->GetLabel() == label) {
         TGListTree     *lt   = gReve->GetLTEFrame()->GetListTree();
         TGListTreeItem *mlti = lt->GetSelected();
         if (mlti->GetUserData() != this)
            mlti = FindListTreeItem(lt);
         TGListTreeItem *tlti = (*i)->FindListTreeItem(lt, mlti);
         lt->HighlightItem(tlti);
         lt->SetSelected(tlti);
         gReve->EditRenderElement(*i);
         return (TEveTrack*) *i;
      }
   }
   return 0;
}

//______________________________________________________________________________
TEveTrack* TEveTrackList::FindTrackByIndex(Int_t index)
{
   // Find track by index, select it and display it in the editor.

   for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i) {
      if (((TEveTrack*)(*i))->GetIndex() == index) {
         TGListTree     *lt   = gReve->GetLTEFrame()->GetListTree();
         TGListTreeItem *mlti = lt->GetSelected();
         if (mlti->GetUserData() != this)
            mlti = FindListTreeItem(lt);
         TGListTreeItem *tlti = (*i)->FindListTreeItem(lt, mlti);
         lt->HighlightItem(tlti);
         lt->SetSelected(tlti);
         gReve->EditRenderElement(*i);
         return (TEveTrack*) *i;
      }
   }
   return 0;
}

//______________________________________________________________________________
void TEveTrackList::ImportHits()
{
   // Import hits for all track.

   for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i) {
      ((TEveTrack*)(*i))->ImportHits();
   }
}

//______________________________________________________________________________
void TEveTrackList::ImportClusters()
{
   // Import clusters for all track.

   for (List_i i=fChildren.begin(); i!=fChildren.end(); ++i) {
      ((TEveTrack*)(*i))->ImportClusters();
   }
}

/******************************************************************************/

//______________________________________________________________________________
TClass* TEveTrackList::ProjectedClass() const
{
   // Virtual from TEveProjectable, returns TEveTrackListProjected class.

   return TEveTrackListProjected::Class();
}


/******************************************************************************/
/******************************************************************************/

#include <TEveGedEditor.h>

//______________________________________________________________________________
// TEveTrackCounter
//
// Provides event-based method for tagging of good / bad (or primary /
// secondary) tracks. A report can be written into a text file.
//
// TEveTrack status is toggled by using secondary-selection / ctrl-click
// functionality of the GL viewer.
//
// Some of the functionality is implemented in TEveTrackCounterEditor
// class.

ClassImp(TEveTrackCounter)

//______________________________________________________________________________
TEveTrackCounter* TEveTrackCounter::fgInstance = 0;

//______________________________________________________________________________
TEveTrackCounter::TEveTrackCounter(const Text_t* name, const Text_t* title) :
   TEveElement(),
   TNamed(name, title),

   fBadLineStyle (6),
   fClickAction  (CA_ToggleTrack),
   fAllTracks    (0),
   fGoodTracks   (0),
   fTrackLists   ()
{
   // Constructor.
   // Connects to global signal "TEveTrack", "CtrlClicked(TEveTrack*)".

   if (fgInstance == 0) fgInstance = this;
   TQObject::Connect("TEveTrack", "CtrlClicked(TEveTrack*)",
                     "TEveTrackCounter", this, "DoTrackAction(TEveTrack*)");
}

//______________________________________________________________________________
TEveTrackCounter::~TEveTrackCounter()
{
   // Destructor.
   // Disconnect from the global track signals.

   TQObject::Disconnect("TEveTrack", "DoTrackAction(TEveTrack*)");
   if (fgInstance == this) fgInstance = 0;
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackCounter::Reset()
{
   // Reset internal track-counters and track-list.

   printf("TEveTrackCounter::Reset()\n");
   fAllTracks  = 0;
   fGoodTracks = 0;
   TIter next(&fTrackLists);
   TEveTrackList* tlist;
   while ((tlist = dynamic_cast<TEveTrackList*>(next())))
      tlist->DecDenyDestroy();
   fTrackLists.Clear("nodelete");
}

//______________________________________________________________________________
void TEveTrackCounter::RegisterTracks(TEveTrackList* tlist, Bool_t goodTracks)
{
   // Register tracks from tlist and tlist itself.
   // If goodTracks is true, they are considered as primary/good
   // tracks.

   tlist->IncDenyDestroy();
   fTrackLists.Add(tlist);

   List_i i = tlist->BeginChildren();
   while (i != tlist->EndChildren())
   {
      TEveTrack* t = dynamic_cast<TEveTrack*>(*i);
      if (t != 0)
      {
         if (goodTracks)
         {
            ++fGoodTracks;
         } else {
            t->SetLineStyle(fBadLineStyle);
         }
         ++fAllTracks;
      }
      ++i;
   }
}

//______________________________________________________________________________
void TEveTrackCounter::DoTrackAction(TEveTrack* track)
{
   // Slot called when track is ctrl-clicked.
   //
   // No check is done if track actually belongs to one of the
   // registered track-lists.
   //
   // Probably it would be safer to copy good/bad tracks into special
   // sub-containers.
   // In this case one should also override RemoveElementLocal.

   switch (fClickAction)
   {

      case CA_PrintTrackInfo:
      {
         printf("TEveTrack '%s'\n", track->GetObject()->GetName());
         TEveVector &v = track->fV, &p = track->fP;
         printf("  Vx=%f, Vy=%f, Vz=%f; Pt=%f, Pz=%f, phi=%f)\n",
                v.x, v.y, v.z, p.Perp(), p.z, TMath::RadToDeg()*p.Phi());
         printf("  <other information should be printed ... full AliESDtrack>\n");
         break;
      }

      case CA_ToggleTrack:
      {
         if (track->GetLineStyle() == 1)
         {
            track->SetLineStyle(fBadLineStyle);
            --fGoodTracks;
         } else {
            track->SetLineStyle(1);
            ++fGoodTracks;
         }
         track->ElementChanged();
         gReve->Redraw3D();

         printf("TEveTrackCounter::CountTrack All=%d, Good=%d, Bad=%d\n",
                fAllTracks, fGoodTracks, fAllTracks-fGoodTracks);

         if (gReve->GetEditor()->GetModel() == GetObject())
            gReve->EditRenderElement(this);

         break;
      }

   } // end switch fClickAction
}

/******************************************************************************/

//______________________________________________________________________________
void TEveTrackCounter::OutputEventTracks(FILE* out)
{
   // Print good-track summary into a plain-text file by iteration
   // through all registered track-lists.
   // State of each track is determined by its line-style, it is
   // considered a good track if it's line style is solid.

   if (out == 0)
   {
      out = stdout;
      fprintf(out, "TEveTrackCounter::FinalizeEvent()\n");
   }

   fprintf(out, "Event = %d  Ntracks = %d\n", fEventId, fGoodTracks);

   TIter tlists(&fTrackLists);
   TEveTrackList* tlist;
   Int_t cnt = 0;
   while ((tlist = (TEveTrackList*) tlists()) != 0)
   {
      List_i i = tlist->BeginChildren();
      while (i != tlist->EndChildren())
      {
         TEveTrack* t = dynamic_cast<TEveTrack*>(*i);
         if (t != 0 && t->GetLineStyle() == 1)
         {
            ++cnt;
            fprintf(out, " %2d: chg=%+2d  pt=%8.5f  eta=%+8.5f\n",
                    cnt, t->fCharge, t->fP.Perp(), t->fP.Eta());
         }
         ++i;
      }
   }
}
