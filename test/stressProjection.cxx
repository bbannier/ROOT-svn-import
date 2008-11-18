#include <iostream>
#include <cmath>

#include "TH3.h"
#include "TH2.h"
#include "TProfile2D.h"
#include "TProfile.h"
#include "THnSparse.h"
#include "TRandom2.h"

#define BINSIZE 10
#define LOWER_LIMIT 0
#define UPPER_LIMIT 10

// In case of deviation, the profiles' content will not work anymore
// try only for testing the statistics
#define CENTRE_DEVIATION 0.0

using std::cout;
using std::endl;
using std::fabs;

// extern declarations
enum compareOptions {
   cmpOptDebug=1,
   cmpOptNoError=2,
   cmpOptStats=4
};

int equals(const char* msg, TH1D* h1, TH1D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(const char* msg, TH2D* h1, TH2D* h2, int options = 0, double ERRORLIMIT = 1E-15);
int equals(Double_t n1, Double_t n2, double ERRORLIMIT = 1E-15);
int compareStatistics( TH1* h1, TH1* h2, bool debug, double ERRORLIMIT = 1E-15);
// extern declarations - end

class HistogramTester {
   
private:
   TH3D* h3;
   TH2D* h2XY;
   TH2D* h2XZ;
   TH2D* h2YX;
   TH2D* h2YZ;
   TH2D* h2ZX;
   TH2D* h2ZY;
   TH1D* h1X;
   TH1D* h1Y;
   TH1D* h1Z;

   TH1D* h1XStats;
   TH1D* h1YStats;
   TH1D* h1ZStats;
   
   TProfile2D* pe2XY;
   TProfile2D* pe2XZ;
   TProfile2D* pe2YX;
   TProfile2D* pe2YZ;
   TProfile2D* pe2ZX;
   TProfile2D* pe2ZY;
   
   TH2D* h2wXY;
   TH2D* h2wXZ;
   TH2D* h2wYX;
   TH2D* h2wYZ;
   TH2D* h2wZX;
   TH2D* h2wZY;
   
   TProfile* pe1XY;
   TProfile* pe1XZ;
   TProfile* pe1YX;
   TProfile* pe1YZ;
   TProfile* pe1ZX;
   TProfile* pe1ZY;

   TH1D* hw1XZ;
   TH1D* hw1XY;
   TH1D* hw1YX;
   TH1D* hw1YZ;
   TH1D* hw1ZX;
   TH1D* hw1ZY;
   
   THnSparseD* s3;

   bool buildWithWeights;

   TRandom2 r;
   
public:
   HistogramTester()
   {
      CreateHistograms();
      buildWithWeights = false;
   }
   
   void CreateHistograms()
   {
      h3 = new TH3D("h3","h3", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      h2XY = new TH2D("h2XY", "h2XY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2XZ = new TH2D("h2XZ", "h2XZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2YX = new TH2D("h2YX", "h2YX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2YZ = new TH2D("h2YZ", "h2YZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2ZX = new TH2D("h2ZX", "h2ZX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2ZY = new TH2D("h2ZY", "h2ZY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                      BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      // The bit is set for all the histograms (It's a statistic variable)
      TH1::StatOverflows(kTRUE);

      h1X = new TH1D("h1X", "h1X", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h1Y = new TH1D("h1Y", "h1Y", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h1Z = new TH1D("h1Z", "h1Z", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      h1XStats = new TH1D("h1XStats", "h1XStats", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h1YStats = new TH1D("h1YStats", "h1YStats", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h1ZStats = new TH1D("h1ZStats", "h1ZStats", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      pe2XY = new TProfile2D("pe2XY", "pe2XY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe2XZ = new TProfile2D("pe2XZ", "pe2XZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe2YX = new TProfile2D("pe2YX", "pe2YX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe2YZ = new TProfile2D("pe2YZ", "pe2YZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe2ZX = new TProfile2D("pe2ZX", "pe2ZX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe2ZY = new TProfile2D("pe2ZY", "pe2ZY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                               BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      
      h2wXY = new TH2D("h2wXY", "h2wXY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2wXZ = new TH2D("h2wXZ", "h2wXZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2wYX = new TH2D("h2wYX", "h2wYX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2wYZ = new TH2D("h2wYZ", "h2wYZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2wZX = new TH2D("h2wZX", "h2wZX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      h2wZY = new TH2D("h2wZY", "h2wZY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT, 
                                         BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      h2wXY->Sumw2();
      h2wXZ->Sumw2();
      h2wYX->Sumw2();
      h2wYZ->Sumw2();
      h2wZX->Sumw2();
      h2wZY->Sumw2();

      pe1XY = new TProfile("pe1XY", "pe1XY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe1XZ = new TProfile("pe1XZ", "pe1XZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe1YX = new TProfile("pe1YX", "pe1YX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe1YZ = new TProfile("pe1YZ", "pe1YZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe1ZX = new TProfile("pe1ZX", "pe1ZX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      pe1ZY = new TProfile("pe1ZY", "pe1ZY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      hw1XY = new TH1D("hw1XY", "hw1XY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      hw1XZ = new TH1D("hw1XZ", "hw1XZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      hw1YX = new TH1D("hw1YX", "hw1YX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      hw1YZ = new TH1D("hw1YZ", "hw1YZ", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      hw1ZX = new TH1D("hw1ZX", "hw1ZX", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);
      hw1ZY = new TH1D("hw1ZY", "hw1ZY", BINSIZE, LOWER_LIMIT, UPPER_LIMIT);

      hw1XZ->Sumw2();
      hw1XY->Sumw2();
      hw1YX->Sumw2();
      hw1YZ->Sumw2();
      hw1ZX->Sumw2();
      hw1ZY->Sumw2();

      Int_t bsize[] = {BINSIZE, BINSIZE, BINSIZE};
      Double_t xmin[] = {LOWER_LIMIT, LOWER_LIMIT, LOWER_LIMIT};
      Double_t xmax[] = {UPPER_LIMIT, UPPER_LIMIT, UPPER_LIMIT};
      s3 = new THnSparseD("s3","s3", 3, bsize, xmin, xmax);

   }
   
   void DeleteHistograms()
   {
      delete h3;
      
      delete h2XY;
      delete h2XZ;
      delete h2YX;
      delete h2YZ;
      delete h2ZX;
      delete h2ZY;

      delete h1X;
      delete h1Y;
      delete h1Z;
      
      delete h1XStats;
      delete h1YStats;
      delete h1ZStats;

      delete pe2XY;
      delete pe2XZ;
      delete pe2YX;
      delete pe2YZ;
      delete pe2ZX;
      delete pe2ZY;
      
      delete h2wXY;
      delete h2wXZ;
      delete h2wYX;
      delete h2wYZ;
      delete h2wZX;
      delete h2wZY;

      delete pe1XY;
      delete pe1XZ;
      delete pe1YX;
      delete pe1YZ;
      delete pe1ZY;
      delete pe1ZX;

      delete hw1XY;
      delete hw1XZ;
      delete hw1YX;
      delete hw1YZ;
      delete hw1ZX;
      delete hw1ZY;

      delete s3;
   }
   
   virtual ~HistogramTester()
   {
      DeleteHistograms();
   }
   
   
   void buildHistograms()
   {
      for ( int ix = 0; ix <= h3->GetXaxis()->GetNbins() + 1; ++ix ) {
         double x = CENTRE_DEVIATION * h3->GetXaxis()->GetBinWidth(ix) + h3->GetXaxis()->GetBinCenter(ix);
         for ( int iy = 0; iy <= h3->GetYaxis()->GetNbins() + 1; ++iy ) {
            double y = CENTRE_DEVIATION * h3->GetYaxis()->GetBinWidth(iy) + h3->GetYaxis()->GetBinCenter(iy);
            for ( int iz = 0; iz <= h3->GetZaxis()->GetNbins() + 1; ++iz ) {
               double z = CENTRE_DEVIATION * h3->GetZaxis()->GetBinWidth(iz) + h3->GetZaxis()->GetBinCenter(iz);
               for ( int i = 0; i < (int) r.Uniform(1,3); ++i )
               {
                  h3->Fill(x,y,z);

                  Double_t points[] = {x,y,z};
                  s3->Fill(points);
                  
                  h2XY->Fill(x,y);
                  h2XZ->Fill(x,z);
                  h2YX->Fill(y,x);
                  h2YZ->Fill(y,z);
                  h2ZX->Fill(z,x);
                  h2ZY->Fill(z,y);
                  
                  h1X->Fill(x);
                  h1Y->Fill(y);
                  h1Z->Fill(z);

                  if ( ix > 0 && ix < h3->GetXaxis()->GetNbins() + 1 &&
                       iy > 0 && iy < h3->GetYaxis()->GetNbins() + 1 &&
                       iz > 0 && iz < h3->GetZaxis()->GetNbins() + 1 )
                  {
                     h1XStats->Fill(x);
                     h1YStats->Fill(y);
                     h1ZStats->Fill(z);
                  }

                  pe2XY->Fill(x,y,z);
                  pe2XZ->Fill(x,z,y);
                  pe2YX->Fill(y,x,z);
                  pe2YZ->Fill(y,z,x);
                  pe2ZX->Fill(z,x,y);
                  pe2ZY->Fill(z,y,x);
                  
                  h2wXY->Fill(x,y,z);
                  h2wXZ->Fill(x,z,y);
                  h2wYX->Fill(y,x,z);
                  h2wYZ->Fill(y,z,x);
                  h2wZX->Fill(z,x,y);
                  h2wZY->Fill(z,y,x);

                  pe1XY->Fill(x,y);
                  pe1XZ->Fill(x,z);
                  pe1YX->Fill(y,x);
                  pe1YZ->Fill(y,z);
                  pe1ZX->Fill(z,x);
                  pe1ZY->Fill(z,y);

                  hw1XY->Fill(x,y);
                  hw1XZ->Fill(x,z);
                  hw1YX->Fill(y,x);
                  hw1YZ->Fill(y,z);
                  hw1ZX->Fill(z,x);
                  hw1ZY->Fill(z,y);
               }
            }
         }
      }

      buildWithWeights = false;
   }

   void buildHistogramsWithWeights()
   {
      for ( int ix = 0; ix <= h3->GetXaxis()->GetNbins() + 1; ++ix ) {
         double x = CENTRE_DEVIATION * h3->GetXaxis()->GetBinWidth(ix) + h3->GetXaxis()->GetBinCenter(ix);
         for ( int iy = 0; iy <= h3->GetYaxis()->GetNbins() + 1; ++iy ) {
            double y = CENTRE_DEVIATION * h3->GetYaxis()->GetBinWidth(iy) + h3->GetYaxis()->GetBinCenter(iy);
            for ( int iz = 0; iz <= h3->GetZaxis()->GetNbins() + 1; ++iz ) {
               double z = CENTRE_DEVIATION * h3->GetZaxis()->GetBinWidth(iz) + h3->GetZaxis()->GetBinCenter(iz);
               Double_t w = (Double_t) r.Uniform(1,3);

               h3->Fill(x,y,z,w);

               Double_t points[] = {x,y,z};
               s3->Fill(points,w);
               
               h2XY->Fill(x,y,w);
               h2XZ->Fill(x,z,w);
               h2YX->Fill(y,x,w);
               h2YZ->Fill(y,z,w);
               h2ZX->Fill(z,x,w);
               h2ZY->Fill(z,y,w);
               
               h1X->Fill(x,w);
               h1Y->Fill(y,w);
               h1Z->Fill(z,w);
                   
               if ( ix > 0 && ix < h3->GetXaxis()->GetNbins() + 1 &&
                    iy > 0 && iy < h3->GetYaxis()->GetNbins() + 1 &&
                    iz > 0 && iz < h3->GetZaxis()->GetNbins() + 1 )
               {
                  h1XStats->Fill(x,w);
                  h1YStats->Fill(y,w);
                  h1ZStats->Fill(z,w);
               }              

               pe2XY->Fill(x,y,z,w);
               pe2XZ->Fill(x,z,y,w);
               pe2YX->Fill(y,x,z,w);
               pe2YZ->Fill(y,z,x,w);
               pe2ZX->Fill(z,x,y,w);
               pe2ZY->Fill(z,y,x,w);
               
               h2wXY->Fill(x,y,z*w);
               h2wXZ->Fill(x,z,y*w);
               h2wYX->Fill(y,x,z*w);
               h2wYZ->Fill(y,z,x*w);
               h2wZX->Fill(z,x,y*w);
               h2wZY->Fill(z,y,x*w);
               
               pe1XY->Fill(x,y,w);
               pe1XZ->Fill(x,z,w);
               pe1YX->Fill(y,x,w);
               pe1YZ->Fill(y,z,w);
               pe1ZX->Fill(z,x,w);
               pe1ZY->Fill(z,y,w);
               
               hw1XY->Fill(x,y*w);
               hw1XZ->Fill(x,z*w);
               hw1YX->Fill(y,x*w);
               hw1YZ->Fill(y,z*w);
               hw1ZX->Fill(z,x*w);
               hw1ZY->Fill(z,y*w);
            }
         }
      }

      buildWithWeights = true;
   }
   
   void buildHistograms(int xmin, int xmax,
                        int ymin, int ymax,
                        int zmin, int zmax)
   {
      for ( int ix = 0; ix <= h3->GetXaxis()->GetNbins() + 1; ++ix ) {
         double x = CENTRE_DEVIATION * h3->GetXaxis()->GetBinCenter(ix);
         for ( int iy = 0; iy <= h3->GetYaxis()->GetNbins() + 1; ++iy ) {
            double y = CENTRE_DEVIATION * h3->GetYaxis()->GetBinCenter(iy);
            for ( int iz = 0; iz <= h3->GetZaxis()->GetNbins() + 1; ++iz ) {
               double z = CENTRE_DEVIATION * h3->GetZaxis()->GetBinCenter(iz);
               for ( int i = 0; i < (int) r.Uniform(1,3); ++i )
               {
                  h3->Fill(x,y,z);

                  Double_t points[] = {x,y,z};
                  s3->Fill(points);
                  
                  if ( h3->GetXaxis()->FindBin(x) >= xmin && h3->GetXaxis()->FindBin(x) <= xmax &&
                       h3->GetYaxis()->FindBin(y) >= ymin && h3->GetYaxis()->FindBin(y) <= ymax &&
                       h3->GetZaxis()->FindBin(z) >= zmin && h3->GetZaxis()->FindBin(z) <= zmax )
                  {
                     cout << "Filling (" << x << "," << y << "," << z << ")!" << endl;
                     
                     h2XY->Fill(x,y);
                     h2XZ->Fill(x,z);
                     h2YX->Fill(y,x);
                     h2YZ->Fill(y,z);
                     h2ZX->Fill(z,x);
                     h2ZY->Fill(z,y);
                     
                     h1X->Fill(x);
                     h1Y->Fill(y);
                     h1Z->Fill(z);
                     
                     pe2XY->Fill(x,y,z);
                     pe2XZ->Fill(x,z,y);
                     pe2YX->Fill(y,x,z);
                     pe2YZ->Fill(y,z,x);
                     pe2ZX->Fill(z,x,y);
                     pe2ZY->Fill(z,y,x);
                     
                     h2wXY->Fill(x,y,z);
                     h2wXZ->Fill(x,z,y);
                     h2wYX->Fill(y,x,z);
                     h2wYZ->Fill(y,z,x);
                     h2wZX->Fill(z,x,y);
                     h2wZY->Fill(z,y,x);

                     pe1XY->Fill(x,y);
                     pe1XZ->Fill(x,z);
                     pe1YX->Fill(y,x);
                     pe1YZ->Fill(y,z);
                     pe1ZX->Fill(z,x);
                     pe1ZY->Fill(z,y);
                     
                     hw1XY->Fill(x,y);
                     hw1XZ->Fill(x,z);
                     hw1YX->Fill(y,x);
                     hw1YZ->Fill(y,z);
                     hw1ZX->Fill(z,x);
                     hw1ZY->Fill(z,y);
                  }
               }
            }
         }
      }
      
      h3->GetXaxis()->SetRange(xmin, xmax);
      h3->GetYaxis()->SetRange(ymin, ymax);
      h3->GetZaxis()->SetRange(zmin, zmax);
      
      h2XY->GetXaxis()->SetRange(xmin, xmax);
      h2XY->GetYaxis()->SetRange(ymin, ymax);
      
      h2XZ->GetXaxis()->SetRange(xmin, xmax);
      h2XZ->GetZaxis()->SetRange(zmin, zmax);
      
      h2YX->GetYaxis()->SetRange(ymin, ymax);
      h2YX->GetXaxis()->SetRange(xmin, xmax);
      
      h2YZ->GetYaxis()->SetRange(ymin, ymax);
      h2YZ->GetZaxis()->SetRange(zmin, zmax);
      
      h2ZX->GetZaxis()->SetRange(zmin, zmax);
      h2ZX->GetXaxis()->SetRange(xmin, xmax);
      
      h2ZY->GetZaxis()->SetRange(zmin, zmax);
      h2ZY->GetYaxis()->SetRange(ymin, ymax);
      
      h1X->GetXaxis()->SetRange(xmin, xmax);
      h1Y->GetXaxis()->SetRange(ymin, ymax);
      h1Z->GetXaxis()->SetRange(zmin, zmax);

      // Neet to set up the rest of the ranges!

      s3->GetAxis(1)->SetRange(xmin, xmax);
      s3->GetAxis(2)->SetRange(ymin, ymax);
      s3->GetAxis(3)->SetRange(zmin, zmax);

      buildWithWeights = false;
   }
   
   int compareHistograms()
   {
      int status = 0;
      int options = 0;
      
      // TH2 derived from TH3
      options = cmpOptStats;
      status += equals("TH3 -> XY", h2XY, (TH2D*) h3->Project3D("yx"), options);
      status += equals("TH3 -> XZ", h2XZ, (TH2D*) h3->Project3D("zx"), options);
      status += equals("TH3 -> YX", h2YX, (TH2D*) h3->Project3D("XY"), options);
      status += equals("TH3 -> YZ", h2YZ, (TH2D*) h3->Project3D("ZY"), options);
      status += equals("TH3 -> ZX", h2ZX, (TH2D*) h3->Project3D("XZ"), options);
      status += equals("TH3 -> ZY", h2ZY, (TH2D*) h3->Project3D("YZ"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // TH1 derived from TH3
      options = cmpOptStats;
      status += equals("TH3 -> X", h1X, (TH1D*) h3->Project3D("x"), options);
      status += equals("TH3 -> Y", h1Y, (TH1D*) h3->Project3D("y"), options);
      status += equals("TH3 -> Z", h1Z, (TH1D*) h3->Project3D("z"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // TH1 derived from h2XY
      options = cmpOptStats;
      status += equals("TH2XY -> X", h1X, (TH1D*) h2XY->ProjectionX("x"), options);
      status += equals("TH2XY -> Y", h1Y, (TH1D*) h2XY->ProjectionY("y"), options);
      // TH1 derived from h2XZ
      status += equals("TH2XZ -> X", h1X, (TH1D*) h2XZ->ProjectionX("x"), options);
      status += equals("TH2XZ -> Z", h1Z, (TH1D*) h2XZ->ProjectionY("z"), options);
      // TH1 derived from h2YX
      status += equals("TH2YX -> Y", h1Y, (TH1D*) h2YX->ProjectionX("y"), options);
      status += equals("TH2YX -> X", h1X, (TH1D*) h2YX->ProjectionY("x"), options);
      // TH1 derived from h2YZ
      status += equals("TH2YZ -> Y", h1Y, (TH1D*) h2YZ->ProjectionX("y"), options);
      status += equals("TH2YZ -> Z", h1Z, (TH1D*) h2YZ->ProjectionY("z"), options);
      // TH1 derived from h2ZX
      status += equals("TH2ZX -> Z", h1Z, (TH1D*) h2ZX->ProjectionX("z"), options);
      status += equals("TH2ZX -> X", h1X, (TH1D*) h2ZX->ProjectionY("x"), options);
      // TH1 derived from h2ZY
      status += equals("TH2ZY -> Z", h1Z, (TH1D*) h2ZY->ProjectionX("z"), options);
      status += equals("TH2ZY -> Y", h1Y, (TH1D*) h2ZY->ProjectionY("y"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // Now the histograms comming from the Profiles!
      options = cmpOptStats;
      status += equals("TH3 -> PBXY", h2XY, (TH2D*) h3->Project3DProfile("yxUO")->ProjectionXY("1", "B"), options);
      status += equals("TH3 -> PBXZ", h2XZ, (TH2D*) h3->Project3DProfile("zxUO")->ProjectionXY("2", "B"), options);
      status += equals("TH3 -> PBYX", h2YX, (TH2D*) h3->Project3DProfile("xyUO")->ProjectionXY("3", "B"), options);
      status += equals("TH3 -> PBYZ", h2YZ, (TH2D*) h3->Project3DProfile("zyUO")->ProjectionXY("4", "B"), options);
      status += equals("TH3 -> PBZX", h2ZX, (TH2D*) h3->Project3DProfile("xzUO")->ProjectionXY("5", "B"), options);
      status += equals("TH3 -> PBZY", h2ZY, (TH2D*) h3->Project3DProfile("yzUO")->ProjectionXY("6", "B"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // test directly project3dprofile
      options = cmpOptStats;
      status += equals("TH3 -> PXY", (TH2D*) pe2XY, (TH2D*) h3->Project3DProfile("yx UO"), options);
      status += equals("TH3 -> PXZ", (TH2D*) pe2XZ, (TH2D*) h3->Project3DProfile("zx UO"), options);
      status += equals("TH3 -> PYX", (TH2D*) pe2YX, (TH2D*) h3->Project3DProfile("xy UO"), options);
      status += equals("TH3 -> PYZ", (TH2D*) pe2YZ, (TH2D*) h3->Project3DProfile("zy UO"), options);
      status += equals("TH3 -> PZX", (TH2D*) pe2ZX, (TH2D*) h3->Project3DProfile("xz UO"), options);
      status += equals("TH3 -> PZY", (TH2D*) pe2ZY, (TH2D*) h3->Project3DProfile("yz UO"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // test option E of ProjectionXY
      options = 0;
      status += equals("TH3 -> PEXY", (TH2D*) pe2XY, (TH2D*) h3->Project3DProfile("yx UO")->ProjectionXY("1", "E"), options);
      status += equals("TH3 -> PEXZ", (TH2D*) pe2XZ, (TH2D*) h3->Project3DProfile("zx UO")->ProjectionXY("2", "E"), options);
      status += equals("TH3 -> PEYX", (TH2D*) pe2YX, (TH2D*) h3->Project3DProfile("xy UO")->ProjectionXY("3", "E"), options);
      status += equals("TH3 -> PEYZ", (TH2D*) pe2YZ, (TH2D*) h3->Project3DProfile("zy UO")->ProjectionXY("4", "E"), options);
      status += equals("TH3 -> PEZX", (TH2D*) pe2ZX, (TH2D*) h3->Project3DProfile("xz UO")->ProjectionXY("5", "E"), options);
      status += equals("TH3 -> PEZY", (TH2D*) pe2ZY, (TH2D*) h3->Project3DProfile("yz UO")->ProjectionXY("6", "E"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // test option W of ProjectionXY
      
      // The error fails when built with weights. It is not properly calculated
      if ( buildWithWeights ) options = cmpOptNoError;
      status += equals("TH3 -> PWXY", (TH2D*) h2wXY, (TH2D*) h3->Project3DProfile("yx UO")->ProjectionXY("1", "W"), options);
      status += equals("TH3 -> PWXZ", (TH2D*) h2wXZ, (TH2D*) h3->Project3DProfile("zx UO")->ProjectionXY("2", "W"), options);
      status += equals("TH3 -> PWYX", (TH2D*) h2wYX, (TH2D*) h3->Project3DProfile("xy UO")->ProjectionXY("3", "W"), options);
      status += equals("TH3 -> PWYZ", (TH2D*) h2wYZ, (TH2D*) h3->Project3DProfile("zy UO")->ProjectionXY("4", "W"), options);
      status += equals("TH3 -> PWZX", (TH2D*) h2wZX, (TH2D*) h3->Project3DProfile("xz UO")->ProjectionXY("5", "W"), options);
      status += equals("TH3 -> PWZY", (TH2D*) h2wZY, (TH2D*) h3->Project3DProfile("yz UO")->ProjectionXY("6", "W"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // test 1D histograms
      options = cmpOptStats;
      status += equals("TH2XY -> PBX", h1X, (TH1D*) h2XY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2XY -> PBX", h1Y, (TH1D*) h2XY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2XZ -> PBX", h1X, (TH1D*) h2XZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2XZ -> PBZ", h1Z, (TH1D*) h2XZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2YX -> PBY", h1Y, (TH1D*) h2YX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2YX -> PBX", h1X, (TH1D*) h2YX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2YZ -> PBY", h1Y, (TH1D*) h2YZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2YZ -> PBZ", h1Z, (TH1D*) h2YZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2ZX -> PBZ", h1Z, (TH1D*) h2ZX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2ZX -> PBX", h1X, (TH1D*) h2ZX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2ZY -> PBZ", h1Z, (TH1D*) h2ZY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      status += equals("TH2ZY -> PBY", h1Y, (TH1D*) h2ZY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "B"),options);
      options = 0;
      cout << "----------------------------------------------" << endl;

      // 1D testing direct profiles 
      options = cmpOptStats;
      status += equals("TH2XY -> PX", pe1XY, (TH1D*) h2XY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2XY -> PX", pe1YX, (TH1D*) h2XY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      status += equals("TH2XZ -> PX", pe1XZ, (TH1D*) h2XZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2XZ -> PZ", pe1ZX, (TH1D*) h2XZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      status += equals("TH2YX -> PY", pe1YX, (TH1D*) h2YX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2YX -> PX", pe1XY, (TH1D*) h2YX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      status += equals("TH2YZ -> PY", pe1YZ, (TH1D*) h2YZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2YZ -> PZ", pe1ZY, (TH1D*) h2YZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      status += equals("TH2ZX -> PZ", pe1ZX, (TH1D*) h2ZX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2ZX -> PX", pe1XZ, (TH1D*) h2ZX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      status += equals("TH2ZY -> PZ", pe1ZY, (TH1D*) h2ZY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1), options);
      status += equals("TH2ZY -> PY", pe1YZ, (TH1D*) h2ZY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1), options);
      options = 0;
      cout << "----------------------------------------------" << endl;

      // 1D testing e profiles
      options = 0;
      status += equals("TH2XY -> PEX", pe1XY, 
                       (TH1D*) h2XY->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2XY -> PEX", pe1YX, 
                       (TH1D*) h2XY->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2XZ -> PEX", pe1XZ, 
                       (TH1D*) h2XZ->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2XZ -> PEZ", pe1ZX, 
                       (TH1D*) h2XZ->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2YX -> PEY", pe1YX, 
                       (TH1D*) h2YX->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2YX -> PEX", pe1XY, 
                       (TH1D*) h2YX->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2YZ -> PEY", pe1YZ, 
                       (TH1D*) h2YZ->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2YZ -> PEZ", pe1ZY, 
                       (TH1D*) h2YZ->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2ZX -> PEZ", pe1ZX, 
                       (TH1D*) h2ZX->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2ZX -> PEX", pe1XZ, 
                       (TH1D*) h2ZX->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2ZY -> PEZ", pe1ZY, 
                       (TH1D*) h2ZY->ProfileX("8", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      status += equals("TH2ZY -> PEY", pe1YZ, 
                       (TH1D*) h2ZY->ProfileY("8", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "E"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;

      // 1D testing w profiles
      // The error is not properly propagated when build with weights :S
      if ( buildWithWeights ) options = cmpOptNoError;
      status += equals("TH2XY -> PWX", hw1XY, 
                       (TH1D*) h2XY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2XY -> PWX", hw1YX, 
                       (TH1D*) h2XY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2XZ -> PWX", hw1XZ, 
                       (TH1D*) h2XZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2XZ -> PWZ", hw1ZX, 
                       (TH1D*) h2XZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2YX -> PWY", hw1YX, 
                       (TH1D*) h2YX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2YX -> PWX", hw1XY, 
                       (TH1D*) h2YX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2YZ -> PWY", hw1YZ, 
                       (TH1D*) h2YZ->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2YZ -> PWZ", hw1ZY, 
                       (TH1D*) h2YZ->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2ZX -> PWZ", hw1ZX, 
                       (TH1D*) h2ZX->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2ZX -> PWX", hw1XZ, 
                       (TH1D*) h2ZX->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2ZY -> PWZ", hw1ZY, 
                       (TH1D*) h2ZY->ProfileX("7", 0,h2XY->GetXaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      status += equals("TH2ZY -> PWY", hw1YZ, 
                       (TH1D*) h2ZY->ProfileY("7", 0,h2XY->GetYaxis()->GetNbins()+1)->ProjectionX("1", "W"), options);
      options = 0;
      cout << "----------------------------------------------" << endl;
      
      // TH2 derived from STH3
      options = cmpOptStats;
      status += equals("STH3 -> XY", h2XY, (TH2D*) s3->Projection(1,0), options);
      status += equals("STH3 -> XZ", h2XZ, (TH2D*) s3->Projection(2,0), options);
      status += equals("STH3 -> YX", h2YX, (TH2D*) s3->Projection(0,1), options);
      status += equals("STH3 -> YZ", h2YZ, (TH2D*) s3->Projection(2,1), options);
      status += equals("STH3 -> ZX", h2ZX, (TH2D*) s3->Projection(0,2), options);
      status += equals("STH3 -> ZY", h2ZY, (TH2D*) s3->Projection(1,2), options);
      options = 0;
      cout << "----------------------------------------------" << endl;

      // TH1 derived from STH3
      options = cmpOptStats;
      status += equals("STH3 -> X", h1X, (TH1D*) s3->Projection(0), options);
      status += equals("STH3 -> Y", h1Y, (TH1D*) s3->Projection(1), options);
      status += equals("STH3 -> Z", h1Z, (TH1D*) s3->Projection(2), options);
      options = 0;
      cout << "----------------------------------------------" << endl;

      return status;
   }
   
};

int stressProjection(bool testWithoutWeights = true,
                     bool testWithWeights = true)
{
   int status = 0;
   
   if ( testWithoutWeights )
   {
      cout << "**********************************\n"
           << "       Test without weights       \n" 
           << "**********************************\n"
           << endl;
      
      HistogramTester ht;
      ht.buildHistograms();
      //ht.buildHistograms(2,4,5,6,8,10);
      status += ht.compareHistograms();
   }

   if ( testWithWeights )
   {
      cout << "**********************************\n"
           << "        Test with weights         \n" 
           << "**********************************\n"
           << endl;

      HistogramTester ht;
      ht.buildHistogramsWithWeights();
      status += ht.compareHistograms();
   }

   return status;
}
