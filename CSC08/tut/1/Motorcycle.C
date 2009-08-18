
#include "TString.h"
#include "TSystem.h"
#include "Motorcycle.h"
#include "Riostream.h"
#include "TImage.h"
#include "TPad.h"

#include "Motorcycle.icc" // never mind this file :-)

//______________________________________________________________________________
TMotorcycle::TMotorcycle() : fBrand(""), fModel(""), fCubicCap(0), fPower(0.0)
{
   // Default constructor.
}

//______________________________________________________________________________
TMotorcycle::TMotorcycle(const char *brand, const char *model, Int_t cap, 
                         Float_t power)
{
   // Constructor.

   fBrand    = brand;
   fModel    = model;
   fCubicCap = cap;
   fPower    = power;
}

//______________________________________________________________________________
TMotorcycle::TMotorcycle(const TMotorcycle &c)
{
   // Copy constructor.

   fBrand    = c.fBrand;
   fModel    = c.fModel;
   fCubicCap = c.fCubicCap;
   fPower    = c.fPower;
}

//______________________________________________________________________________
TMotorcycle::~TMotorcycle()
{
   // Destructor.
}

//______________________________________________________________________________
TMotorcycle& TMotorcycle::operator=(const TMotorcycle &c)
{
   // Assignment operator.

   if (this!=&c) {
      fBrand    = c.fBrand;
      fModel    = c.fModel;
      fCubicCap = c.fCubicCap;
      fPower    = c.fPower;
   } 
   return *this;
}

//______________________________________________________________________________
void TMotorcycle::ShowPicture()
{
   // Display some user feedback.

   TImage *img = 0;
   if (fBrand.Contains("Suzuki", TString::kIgnoreCase))
      img = TImage::Open((char **)Suzuki_xpm);
   if (fBrand.Contains("Kawasaki", TString::kIgnoreCase))
      img = TImage::Open((char **)Kawasaki_xpm);
   if (fBrand.Contains("Honda", TString::kIgnoreCase))
      img = TImage::Open((char **)Honda_xpm);
   if (fBrand.Contains("Yamaha", TString::kIgnoreCase))
      img = TImage::Open((char **)Yamaha_xpm);
   if (fBrand.Contains("Harley", TString::kIgnoreCase))
      img = TImage::Open((char **)Harley_xpm);
   if (img == 0)
      return;
   img->SetConstRatio(1);
   img->SetImageQuality(TAttImage::kImgBest);
   img->Draw("xxx");
   gPad->Update();
}
