#ifndef INCL_MYPARTICLE_H
#define INCL_MYPARTICLE_H

#include "TObject.h"
#include "TRandom.h"

class Vertex : public TObject {

public:
   Float_t fX, fY, fZ;

   Vertex() : fX(0.0), fY(0.0), fZ(0.0) { }
   Vertex(Float_t x, Float_t y, Float_t z) : fX(x), fY(y), fZ(z) { }
   virtual ~Vertex() { }

   void    SetVertex(Float_t x, Float_t y, Float_t z) 
           { fX = x; fY = y; fZ = z; }

   ClassDef(Vertex, 1) // simple 3d vertex class
};

class MyParticle : public TObject {

public:
   Int_t   fPDGCode;
   Double_t fMass, fE, fPt;
   Vertex  fVertex;
   static const int kBlobSize = 128/8;
   Double_t fData[kBlobSize]; // BLOB

   MyParticle() : fPDGCode(0), fMass(0.0), fE(0.0), fPt(0.0) { }
   MyParticle(Float_t x, Float_t y, Float_t z, Float_t m, Float_t e, Float_t pt) : 
              fMass(m), fE(e), fPt(pt) {
     fVertex.SetVertex(x, y, z);
     gRandom->RndmArray(kBlobSize,fData);
   }
   virtual ~MyParticle() { }

   void     SetPDGCode(Int_t code) { fPDGCode = code; }
   void     SetMass(Float_t m) { fMass = m; }
   void     SetE(Float_t e) { fE = e; }
   void     SetPt(Float_t pt) { fPt = pt; }
   void     SetProps(Float_t m, Float_t e, Float_t pt) { fMass = m; fE = e; fPt = pt; }
   void     SetVertex(Float_t x, Float_t y, Float_t z)
                    { fVertex.SetVertex(x, y, z); }

   ClassDef(MyParticle, 1) // simple particle with 3d coordinates
};

class Electron : public MyParticle {

public:
   Electron() : MyParticle() { fPDGCode = 11; fMass = 5.10000e-04; }
   Electron(Float_t x, Float_t y, Float_t z, Float_t e, Float_t pt) : MyParticle(x, y, z, 5.10000e-04, e, pt)
   { fPDGCode = 11; }
   virtual ~Electron() { }
   
   ClassDef(Electron, 1) // Simple electron class
};

class Muon : public MyParticle {

public:
   Muon() : MyParticle() { fPDGCode = 13; fMass = 1.05660e-01; }
   Muon(Float_t x, Float_t y, Float_t z, Float_t e, Float_t pt) : MyParticle(x, y, z, 1.05660e-01, e, pt)
   { fPDGCode = 13; }
   virtual ~Muon() { }
   
   ClassDef(Muon, 1) // Simple muon class
};

class Pion : public MyParticle {

public:
   Pion() : MyParticle() { fPDGCode = 111; fMass = 1.34980e-01; }
   Pion(Float_t x, Float_t y, Float_t z, Float_t e, Float_t pt) : MyParticle(x, y, z, 1.34980e-01, e, pt)
   { fPDGCode = 111; }
   virtual ~Pion() { }
   
   ClassDef(Pion, 1) // Simple pion class
};

#ifdef __MAKECINT__ 
#pragma link C++ class Vertex+; 
#pragma link C++ class MyParticle+; 
#pragma link C++ class Electron+; 
#pragma link C++ class Muon+; 
#pragma link C++ class Pion+; 
#endif

#endif // INCL_MYPARTICLE_H
