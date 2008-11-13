// HIC SUNT LEONES

#include <iostream>
#include <map>

#include "math.h"

#include "RooStats/Rsc.h"

#include "TF1.h"
#include "TAxis.h"
#include "TCanvas.h"

namespace Rsc
    {
    TRandom3 random_generator;

    /**
    Perform 2 times a gaussian fit to fetch the center of the histo.
    To get the second fit range get an interval that tries to keep into account 
    the skewness of the distribution.
    **/
    double getHistoCenter(TH1* histo_orig, double n_rms, bool display_result){

        TCanvas* c = new TCanvas();
        c->cd();

        TH1F* histo = (TH1F*)histo_orig->Clone();

        // get the histo x extremes
        double x_min = histo->GetXaxis()->GetXmin(); 
        double x_max = histo->GetXaxis()->GetXmax();

        // First fit!

        TF1* gaus = new TF1("mygaus", "gaus", x_min, x_max);

        gaus->SetParameter("Constant",histo->GetEntries());
        gaus->SetParameter("Mean",histo->GetMean());
        gaus->SetParameter("Sigma",histo->GetRMS());

        histo->Fit(gaus);

        // Second fit!
        double sigma = gaus->GetParameter("Sigma");
        double mean = gaus->GetParameter("Mean");

        delete gaus;

        std::cout << "Center is 1st pass = " << mean << std::endl;

        double skewness = histo->GetSkewness();

        x_min = mean - n_rms*sigma - sigma*skewness/2;
        x_max = mean + n_rms*sigma - sigma*skewness/2;;

        TF1* gaus2 = new TF1("mygaus2", "gaus", x_min, x_max);
        gaus2->SetParameter("Mean",mean);

        histo->Fit(gaus2,"L","", x_min, x_max);

        histo->Draw();
        gaus2->Draw("same");

        double center = gaus2->GetParameter("Mean");


        delete gaus2;
        delete histo;
        if (not display_result)
            delete c;

        return center;


    }


    /**
    We let an orizzontal bar go down and we stop when we have the integral 
    equal to the desired one.
    **/

    double* getHistoPvals (TH1* histo, double percentage){

        if (percentage>1){
            std::cerr << "Percentage greater or equal to 1!\n";
            return NULL;
            }

        // Get the integral of the histo
        double* h_integral=histo->GetIntegral();

        // Start the iteration
        std::map<int,int> extremes_map;

        for (int i=0;i<histo->GetNbinsX();++i){
            for (int j=0;j<histo->GetNbinsX();++j){
                double integral = h_integral[j]-h_integral[i];
                if (integral>percentage){
                    extremes_map[i]=j;
                    break;
                    }
                }
            }

        // Now select the couple of extremes which have the lower bin content diff
        std::map<int,int>::iterator it;
        int a,b;
        double left_bin_center,right_bin_center;
        double diff=10e40;
        double current_diff;
        for (it = extremes_map.begin();it != extremes_map.end();++it){
            a=it->first;
            b=it->second;
            current_diff=fabs(histo->GetBinContent(a)-histo->GetBinContent(b));
            if (current_diff<diff){
                //std::cout << "a=" << a << " b=" << b << std::endl;
                diff=current_diff;
                left_bin_center=histo->GetBinCenter(a);
                right_bin_center=histo->GetBinCenter(b);
                }
            }

        double* d = new double[2];
        d[0]=left_bin_center;
        d[1]=right_bin_center;
        return d;
        }

//----------------------------------------------------------------------------//
/**
Get the median of an histogram.
**/
    double getMedian(TH1* histo){
    
        int xbin_median;
        Double_t* integral = histo->GetIntegral();
        int median_i = 0;
        for (int j=0;j<histo->GetNbinsX(); j++) 
            if (integral[j]<0.5) 
                median_i = j;
    
        double median_x = 
            histo->GetBinCenter(median_i)+
            (histo->GetBinCenter(median_i+1)-
            histo->GetBinCenter(median_i))*
            (0.5-integral[median_i])/(integral[median_i+1]-integral[median_i]);
    
        return median_x;
        }

    } // end of the Rsc Namespace
