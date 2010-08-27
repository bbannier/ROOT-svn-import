
#ifndef ROOT_TString
#include "TString.h"
#endif

class TMotorcycle 
{
private:
   TString fBrand;      // brand
   TString fModel;      // model
   Int_t   fCubicCap;   // engine cubic capacity
   Float_t fPower;      // engine power (kW)
   
public:
   TMotorcycle();
   TMotorcycle(const char *brand, const char *model, Int_t cap, Float_t power);
   TMotorcycle(const TMotorcycle &c);
   virtual ~TMotorcycle();
   TMotorcycle& operator=(const TMotorcycle &c);

   void        ShowPicture();

   // getters
   const char *GetBrand() const { return fBrand.Data(); }
   const char *GetModel() const { return fModel.Data(); }
   Int_t       GetCubicCap() const { return fCubicCap; }
   Float_t     GetPower() const { return fPower; }
   
   // setters
   void        SetBrand(const char *brand) { fBrand = brand; }
   void        SetModel(const char *model) { fModel = model; }
   void        SetCubicCap(Int_t cap) { fCubicCap = cap; }
   void        SetPower(Float_t power) { fPower = power; }
};

