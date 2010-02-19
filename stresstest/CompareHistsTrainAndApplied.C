
#include <fstream>

#include "TPad.h"

//#include "../macros/tmvaglob.C"
#include "../tmva/test/tmvaglob.C"

void writefile(const char* dir,
               const char* day,
               const std::vector<TString>& methodnames,
               const std::vector<int>& qual) {

   const char * dq[3] = {"good", "undec", "bad" };

   std::ofstream f (Form("%s/lt%s.html", dir, day));
   std::ofstream f2(Form("%s/rt%s.html", dir, day));
   std::vector<TString>::iterator mit = methodnames.begin();
   for(UInt_t i=0; i<methodnames.size(); i++) {
      TString& mn = methodnames[i];
      f << "              <tr><td class=\""<<dq[qual[i]]<<"\"><a href=\"#"<<mn<<"_"<<day<<"\">"<<mn<<"</a></td></tr>" << endl;
      if(i!=0) f2 << "              <tr height=\"6\"></tr>" << endl;
      f2 << "              <tr><td bgcolor=\"darkgray\"><a name=\""<<mn<<"_"<<day<<"\"></a><img src=\"plots/CompSig_"<<mn<<"_"<<day<<".png\"></td></tr>" << endl;

   }
   f.close();
   f2.close();
}

void CompareHistsTrainAndApplied(const char* trainFN = "../execs/TMVA.root",
                                 const char* applyFN = "../execs/TMVApp.root") {

   TDatime today;
   const char* dow[8] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
   const char* day = dow[today.GetDayOfWeek()];
   
   std::vector<TString> methodnames;
   std::vector<int> quality;

   TMVAGlob::Initialize( true );

   TFile* trainF = TFile::Open( trainFN );
   TFile* applyF = TFile::Open( applyFN );

   TIter next(trainF->GetListOfKeys());
   TKey *key(0);
   TString methodTypeName("");
   TString methodName("");
   while ((key = (TKey*)next())) {
      if (!TString(key->GetName()).BeginsWith("Method_")) continue;
      if( ! gROOT->GetClass(key->GetClassName())->InheritsFrom("TDirectory") ) continue;

      methodTypeName = key->GetName();
      
      TDirectory* mDir = (TDirectory*)key->ReadObj();
      TIter keyIt(mDir->GetListOfKeys());
      TKey *titkey;
      while ((titkey = (TKey*)keyIt())) {
         if (!gROOT->GetClass(titkey->GetClassName())->InheritsFrom("TDirectory")) continue;

         methodName = titkey->GetName();

         TDirectory *titDir = (TDirectory *)titkey->ReadObj();

         //cout << methodTypeName << "/" << methodName << endl;

         TDirectory* dir = (TDirectory*)titkey->ReadObj();

         TString hname = "MVA_" + methodName;
         TH1* sig   = dynamic_cast<TH1*>(dir->Get( hname + "_S" ));
         TH1* sigtr = dynamic_cast<TH1*>(dir->Get( hname + "_Train_S" ));
         TH1* bgd   = dynamic_cast<TH1*>(dir->Get( hname + "_B" ));
         TH1* bgdtr = dynamic_cast<TH1*>(dir->Get( hname + "_Train_B" ));

         TString fullHName(Form("%s/%s/%s",methodTypeName.Data(), methodName.Data(), hname.Data()));

         if(!sig) {
            cout << "ERROR: could not find " << fullHName << "_S in file " << trainFN << endl;
            continue;
         }
         if(!sigtr) {
            cout << "ERROR: could not find " << fullHName << "_Train_S in file " << trainFN << endl;
            continue;
         }
         if(!bgd) {
            cout << "ERROR: could not find " << fullHName << "_B in file " << trainFN << endl;
            continue;
         }
         if(!bgdtr) {
            cout << "ERROR: could not find " << fullHName << "_Train_B in file " << trainFN << endl;
            continue;
         }

         TH1* cmp = (TH1*)applyF->Get(hname);
         if(!cmp) {
            cout << "ERROR: could not find " << hname << " in file " << applyFN << endl;
            continue;
         }

         TMVAGlob::SetSignalAndBackgroundStyle( sigtr, bgdtr );
         TMVAGlob::SetSignalAndBackgroundStyle( sig, bgd );

         TMVAGlob::NormalizeHist( sig );
         TMVAGlob::NormalizeHist( bgd );
         TMVAGlob::NormalizeHist( sigtr );
         TMVAGlob::NormalizeHist( bgdtr );
         TMVAGlob::NormalizeHist( cmp );

         Float_t ymax = 0;
         ymax = TMath::Max( ymax, sig->GetMaximum() );
         ymax = TMath::Max( ymax, sigtr->GetMaximum() );
         ymax = TMath::Max( ymax, bgd->GetMaximum() );
         ymax = TMath::Max( ymax, bgdtr->GetMaximum() );

         ymax*=1.1;

         TH2F* frame = new TH2F( methodName, methodName, 
                                 1, sig->GetXaxis()->GetXmin(), sig->GetXaxis()->GetXmax(), 1, 0, ymax );

         frame->Draw();
         gPad;
         TMVAGlob::SetFrameStyle( frame );

         sigtr->Draw("samehist");
         sig->Draw("same");
         bgdtr->Draw("samehist");
         bgd->Draw("same");

         TString probaTextSig("Kolmogorov-Smirnov distance (Sig): ");
         TString probaTextBgd("Kolmogorov-Smirnov distance (Bgd): ");

         Float_t kolS, kolB;
         if( sig->GetNbinsX() == sigtr->GetNbinsX() &&
             sig->GetXaxis()->GetXmin() == sigtr->GetXaxis()->GetXmin() &&
             sig->GetXaxis()->GetXmax() == sigtr->GetXaxis()->GetXmax()
             ) {
            Float_t kolS = sig->KolmogorovTest( sigtr );
            probaTextSig += Form( " %5.3g", kolS );
         } else {
            if( sig->GetNbinsX() != sigtr->GetNbinsX() )
               probaTextSig += TString( " n.a. (#bins differ)" );
            else
               probaTextSig += TString( " n.a. (hist ranges differ)" );
         }

         if( bgd->GetNbinsX() == bgdtr->GetNbinsX() &&
             bgd->GetXaxis()->GetXmin() == bgdtr->GetXaxis()->GetXmin() &&
             bgd->GetXaxis()->GetXmax() == bgdtr->GetXaxis()->GetXmax()
             ) {
            Float_t kolB = bgd->KolmogorovTest( bgdtr );
            probaTextBgd += Form( " %5.3g", kolB );
         } else {
            if( bgd->GetNbinsX() != bgdtr->GetNbinsX() )
               probaTextBgd += TString( " n.a. (#bins differ)" );
            else
               probaTextBgd += TString( " n.a. (hist ranges differ)" );
         }
         
         TLatex tl;
         tl.SetNDC();
         tl.DrawLatex( 0.15, 0.82, probaTextSig);
         tl.DrawLatex( 0.15, 0.76, probaTextBgd);
         TDatime today;
         tl.SetTextAlign(32);
         tl.SetTextSize(0.03);
         tl.DrawLatex( 0.95, 0.95, today.AsString() );
         TMVAGlob::imgconv( gPad, Form("../www/plots/CompSig_%s_%s", methodName.Data(), day) );

         methodnames.push_back(methodName);

         if(kolS>0.7 && kolB>0.7) {
            quality.push_back(0); // good
         } else if(kolS<0.3 || kolB<0.3) {
            quality.push_back(2); // bad
         } else {
            quality.push_back(1); // undecided
         }
      }
   }

   writefile("../www/", day, methodnames, quality);
}
