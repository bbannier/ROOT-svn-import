/*
 * TMultinest.cxx
 *
 *  Created on: Oct 29, 2009
 *      Author: Sven Kreiss, Kyle Cranmer
 *
 * Based on MultiNest's eggbox example.
 */

#include "RooStats/MultiNestPlugin/TMultiNest.h"
#include <cmath>
#include <fstream>
#include <sstream>

#include <iostream>
using namespace std;
using namespace RooStats;



TMultiNest *TMultiNest::gMultiNest = NULL;



/*
 * From eggbox C++ example:
 */
#ifdef __INTEL_COMPILER                         // if the MultiNest library was compiled with ifort
       #define NESTRUN nested_mp_nestrun_
#elif defined __GNUC__                          // if the MultiNest library was compiled with gfortran
       #define NESTRUN __nested_MOD_nestrun
#else
       #error "Don't know how to link to Fortran libraries, check symbol table for your platform (nm libnest3.a | grep nestrun) & edit example_eggbox_C++/eggbox.cc"
#endif





/***************************************** C++ Interface to MultiNest **************************************************/

namespace nested
{
        // map the Fortran 90 entry points of libnest3.a to C++ functions

        // module nested, function nestRun maps to nested::run

        // the pass-by-reference nature of most of the Fortran is translated away
        // *apart* from the callbacks. The provided call back functions must still accept 
        // references rather than values. There is also some confusion as to the type
        // of the first argument of LogLike. 
        // Should it be a double * or an farray<double, 1> *? The former seems to 
        // work and is simpler.

        // This structure is reverse engineered from looking 
        // at gfortran stack traces. It is probably wrong
        
        template<typename type, int ndims> class farray_traits;
        
        template<> class farray_traits<double, 1> { public: static const int id = 537; };
        template<> class farray_traits<double, 2> { public: static const int id = 538; };
        template<> class farray_traits<int, 1> { public: static const int id = 265; };
        template<> class farray_traits<int, 2> { public: static const int id = 266; };

        // the extra data for f90 that defines how arrays are arranged.
        template<typename T, int ndim> class farray
        {
                public:
                        farray(T *_data, int w, int h = 0) : data(_data), offset(0), type(farray_traits<T, ndim>::id), 
                        x_stride(1), x_lbound(1), x_ubound(w), y_stride(w), y_lbound(1), y_ubound(h) {};
                        
                        T *data;
                        int offset;
                        int type;
                        int x_stride, x_lbound, x_ubound;
                        int y_stride, y_lbound, y_ubound;
        };
        
        extern "C" {
                void NESTRUN(int &mmodal, int &ceff, int &nlive, double &tol, double &efr, int &ndims,
                        int &nPar, int &nClsPar, int &maxModes, int &updInt, double &Ztol, char *root, int &seed,
                        int *pWrap, int &fb, int &resume, int &outfile, int &initMPI, double &logZero, int &maxiter,
                        void (*Loglike)(double *Cube, int &n_dim, int &n_par, double &lnew, void *),
                        void (*dumper)(int &, int &, int &, double **, double **, double **, double &, double &, double &, void *),
                        void *context, int &root_len);
        }

        static void run(bool mmodal, bool ceff, int nlive, double tol, double efr, int ndims, int nPar, int nClsPar, int maxModes,
                int updInt, double Ztol, const std::string & root, int seed, int *pWrap, bool fb, bool resume, bool outfile, 
                bool initMPI, double logZero, int maxiter, void (*LogLike)(double *Cube, int &n_dim, int &n_par, double &lnew, void *),
                void (*dumper)(int &, int &, int &, double **, double **, double **, double &, double &, double &, void *), void *context)
        {
                char t_root[100];
                std::fill(t_root, t_root + 100, ' ');
                snprintf(t_root, 99, "%s", root.c_str());
                int root_len = strlen(t_root);
                t_root[strlen(t_root)] = ' ';
        
                int t_fb = fb;
                int t_resume = resume;
                int t_outfile = outfile;
                int t_initMPI = initMPI;
                int t_mmodal = mmodal;
                int t_ceff = ceff;
                
                NESTRUN(t_mmodal, t_ceff, nlive, tol, efr, ndims, nPar, nClsPar, maxModes, updInt, Ztol, t_root, seed, pWrap, t_fb, 
                t_resume, t_outfile, t_initMPI, logZero, maxiter, LogLike, dumper, context, root_len);
        }
        

        extern "C" void DeltaFunctionPrior(double& r, double& x1, double& x2);
        extern "C" void UniformPrior(double& r, double& x1, double& x2);
        extern "C" void GaussianPrior(double& r, double& mu, double& sigma);
        extern "C" void LogPrior(double& r,double& x1,double& x2);
        extern "C" void SinPrior(double& r,double& x1,double& x2);
        extern "C" void CauchyPrior(double& r,double& x0,double& gamma);
        extern "C" void LogNormalPrior(double& r,double& a,double& sigma);
}

/***********************************************************************************************************************/

/************************************************* dumper routine ******************************************************/

// The dumper routine will be called every updInt*10 iterations
// MultiNest doesn not need to the user to do anything. User can use the arguments in whichever way he/she wants
//
//
// Arguments:
//
// nSamples                                             = total number of samples in posterior distribution
// nlive                                                = total number of live points
// nPar                                                 = total number of parameters (free + derived)
// physLive[1][nlive * (nPar + 1)]                      = 2D array containing the last set of live points (physical parameters plus derived parameters) along with their loglikelihood values
// posterior[1][nSamples * (nPar + 2)]                  = posterior distribution containing nSamples points. Each sample has nPar parameters (physical + derived) along with the their loglike value & posterior probability
// paramConstr[4*nPar]:
//      paramConstr[0] to paramConstr[nPar - 1]         = mean values of the parameters
//      paramConstr[nPar] to paramConstr[2*nPar - 1]    = standard deviation of the parameters
//      paramConstr[nPar*2] to paramConstr[3*nPar - 1]  = best-fit (maxlike) parameters
//      paramConstr[nPar*4] to paramConstr[4*nPar - 1]  = MAP (maximum-a-posteriori) parameters
// maxLogLike                                           = maximum loglikelihood value
// logZ                                                 = log evidence value
// logZerr                                              = error on log evidence value

void dumper(int &nSamples, int &nlive, int &nPar, double **physLive, double **posterior, double** /*paramConstr*/, double& /*maxLogLike*/, double& /*logZ*/, double& /*logZerr*/, void* /*context*/)
{
        // convert the 2D Fortran arrays to C++ arrays


        // the posterior distribution
        // postdist will have nPar parameters in the first nPar columns & loglike value & the posterior probability in the last two columns

        int i, j;

        //double postdist[nSamples][nPar + 2];
        vector< vector<double> >& postdist = TMultiNest::gMultiNest->GetPosteriorDistribution();
        postdist.clear();
        for( j = 0; j < nSamples; j++ ) {
                vector<double> par;
                for( i = 0; i < nPar + 2; i++ ) {
                        //postdist[j][i] = posterior[0][i * nSamples + j];
                        par.push_back(posterior[0][i * nSamples + j]);
                }
                postdist.push_back(par);
        }


        // last set of live points
        // pLivePts will have nPar parameters in the first nPar columns & loglike value in the last column

        //double pLivePts[nlive][nPar + 1];
        vector< vector<double> >& livePoints = TMultiNest::gMultiNest->GetLivePoints();
        livePoints.clear();
        for( j = 0; j < nlive; j++ ) {
                vector<double> par;
                for( i = 0; i < nPar + 1; i++ ) {
                        //pLivePts[j][i] = physLive[0][i * nlive + j];
                        par.push_back(physLive[0][i * nlive + j]);
                }
                livePoints.push_back(par);
        }
}


/***********************************************************************************************************************/




ClassImp(TMultiNest);

TMultiNest::TMultiNest(void)
   :
      fMmodal(1), fCeff(0), fNlive(1000), fEfr(0.8), fTol(0.5), fNdims(2), fNPar(2),
      fNClsPar(2), fUpdInt(100), fZtol(-1.e90), fMaxModes(100), fRoot("chains/tmp-"),
      fSeed(-1), fFb(1), fResume(0), 

      fOutfile(false),
      fInitMPI(false),
      fLogZero(log(0.0)),
      fMaxIter(1e9),
      
      fContext(0)
   {
   fPWrap[0] = 0;
   fPWrap[1] = 0;
   LogLike(&TMultiNest::LogLikeEggbox);

   gMultiNest = this;
}

void TMultiNest::Run() {
   if (fFb) PrintConfig(cout);
/*
   char rootc[100];
   for (int i = 0; i < 100; i++) {
      if (i < fRoot.size()) rootc[i] = fRoot[i];
      else rootc[i] = ' '; // just to be save for various Fortran compilers
   }
*/
/*
   char t_root[100];
   std::fill(t_root, t_root + 100, ' ');
   snprintf(t_root, 99, "%s", fRoot.c_str());
   int root_len = strlen(t_root);
   t_root[strlen(t_root)] = ' ';
*/
/*
 * This is also new, but is it needed?
 *
   int t_fb = fb;
   int t_resume = resume;
   int t_mmodal = mmodal;
   int t_ceff = ceff;
*/

   fNoOfLogLikeEv = 0;

//   __nested_MOD_nestrun(&fMmodal, &fCeff, &fNlive, &fTol, &fEfr, &fNdims, &fNPar, &fNClsPar, &fMaxModes, &fUpdInt,
//      &fZtol, rootc, &fSeed, fPWrap, &fFb, &fResume, *fLogLikePtr, &fContext);
//   nested::run(fMmodal, fCeff, fNlive, fTol, fEfr, fNdims, fNPar, fNClsPar, fMaxModes, fUpdInt,
//      fZtol, fRoot, fSeed, fPWrap, fFb, fResume, *fLogLikePtr, dumper);
   nested::run(fMmodal, fCeff, fNlive, fTol, fEfr, fNdims, fNPar, fNClsPar, fMaxModes, fUpdInt,
      fZtol, fRoot, fSeed, fPWrap, fFb, fResume, fOutfile, fInitMPI, fLogZero, fMaxIter, *fLogLikePtr, dumper, fContext);
}

void TMultiNest::PrintConfig(ostream& out) {
   out << "int fMmodal: " << fMmodal << endl;
   out << "int fCeff: " << fCeff << endl;
   out << "int fNlive: " << fNlive << endl;
   out << "double fEfr: " << fEfr << endl;
   out << "double fTol: " << fTol << endl;
   out << "int fNdims: " << fNdims << endl;
   out << "int fNPar: " << fNPar << endl;
   out << "int fNClsPar: " << fNClsPar << endl;
   out << "int fUpdInt: " << fUpdInt << endl;
   out << "double fZtol: " << fZtol << endl;
   out << "int fMaxModes: " << fMaxModes << endl;
   out << "int fPWrap[2]: " << fPWrap[0] << ", " << fPWrap[1] << endl;
   out << "string fRoot: " << fRoot << endl;
   out << "int fSeed: " << fSeed << endl;
   out << "int fFb: " << fFb << endl;
   out << "int fResume: " << fResume << endl;
   out << "bool fOutfile: " << fOutfile << endl;
   out << "bool fInitMPI: " << fInitMPI << endl;
   out << "double fLogZero: " << fLogZero << endl;
   out << "int fMaxIter: " << fMaxIter << endl;
}

// Now an example, sample an egg box likelihood
/**
 * \param Cube
 * 		Contains all parameters scaled to the range 0...1. In this function, for each parameter,
 * 		rescale to the right range and overwrite this parameter in Cube
 * 		with the rescaled one. Then, update chi for this parameter value.
 *
 * \param lnew
 * 		This is the "return" value: the log-likelihood at the parameter points given in Cube.
 */
void TMultiNest::LogLikeEggbox(double *Cube, int &ndim, int& /*npars*/, double &lnew, void* /*context*/) {
   double chi = 1.0;

   for (int i = 0; i < ndim; i++) {
      Cube[i] = Cube[i] * 10.0 * M_PI; // rescale parameter
      chi *= cos(Cube[i] / 2.0); // update chi
   }

   lnew = powf(chi + 2.0, 5.0);
}


/**
 * Create a 2D Histogram to see Likelihood values of the
 * "posterior" distribution.
 * For example, this can be used to visualize the eggbox
 * and the 2D rings example.
 */
void TMultiNest::Hist2D(TH2 *h, unsigned int xCol, unsigned int yCol, bool takeExp, bool livePointsOnly, bool reduceZeroBins) {
   vector< vector<double> >::iterator it;

   if(!livePointsOnly) {
      // remember: fPosteriorDistribution includes live points
      for(it = fPosteriorDistribution.begin(); it != fPosteriorDistribution.end(); it++) {
         if( it->size() < 3 ) {
            cout << "Need at least two parameters to do a 2D plot. Aborting." << endl;
            return;
         }
         if( xCol > it->size()-2  ||  yCol > it->size()-2 ) {
            cout << "x or y column invalid. Aborting." << endl;
            return;
         }

         double x = (*it)[xCol];
         double y = (*it)[yCol];
         double ll = (*it)[it->size()-2]; // second last value

         if(takeExp) h->SetBinContent(h->FindBin(x,y), exp(ll));
         else        h->SetBinContent(h->FindBin(x,y), ll);
      }
   }else{
      // live points only
      for(it = fLivePoints.begin(); it != fLivePoints.end(); it++) {
         if( it->size() < 3 ) {
            cout << "Need at least two parameters to do a 2D plot. Aborting." << endl;
            return;
         }
         if( xCol > it->size()-2  ||  yCol > it->size()-2 ) {
            cout << "x or y column invalid. Aborting." << endl;
            return;
         }

         double x = (*it)[xCol];
         double y = (*it)[yCol];
         double ll = (*it).back();

         if(takeExp) h->SetBinContent(h->FindBin(x,y), exp(ll));
         else        h->SetBinContent(h->FindBin(x,y), ll);
      }
   }


   if(reduceZeroBins) {
      double bottom = h->GetMinimum()/* - fabs(h->GetMaximum() - h->GetMinimum()) / 4.0*/;
      for( int x=1; x <= h->GetNbinsX(); x++) {
         for( int y=1; y <= h->GetNbinsY(); y++) {
            if( h->GetBinContent(x,y) == 0 ) h->SetBinContent( x,y, bottom );
         }
      }
   }
}



void TMultiNest::PosteriorHist1D(TH1 *h, unsigned int xCol) {
   vector< vector<double> >::iterator it;

   for(it = fPosteriorDistribution.begin(); it != fPosteriorDistribution.end(); it++) {
      if( it->size() < 2 ) {
         cout << "Need at least two parameters to do a 2D plot. Aborting." << endl;
         return;
      }
      if( xCol > it->size()-2 ) {
         cout << "column invalid. Aborting." << endl;
         return;
      }

      double x = (*it)[xCol];
      double postProb = (*it).back();

      h->Fill(x,postProb);
   }
}

void TMultiNest::PosteriorHist2D(TH2 *h, unsigned int xCol, unsigned int yCol) {
   vector< vector<double> >::iterator it;

   for(it = fPosteriorDistribution.begin(); it != fPosteriorDistribution.end(); it++) {
      if( it->size() < 3 ) {
         cout << "Need at least two parameters to do a 2D plot. Aborting." << endl;
         return;
      }
      if( xCol > it->size()-2  ||  yCol > it->size()-2 ) {
         cout << "x or y column invalid. Aborting." << endl;
         return;
      }

      double x = (*it)[xCol];
      double y = (*it)[yCol];
      double postProb = (*it).back();

      h->Fill(x,y,postProb);
   }
}

/*
void TMultiNest::ProfileFromFile(TProfile2D *h) {
   ifstream ifile((fRoot + ".txt").c_str());
   string line;

   while (getline(ifile, line)) {
      istringstream sline(line);
      double x = DoubleFromStream(sline);
      double y = DoubleFromStream(sline);
      double ll = DoubleFromStream(sline);
      h->Fill(x, y, ll);
   }

   ifile.close();
}
*/

/**
 * Works around a formatting bug in MultiNest/Fortran(?).
 * The bug is that for numbers with negative exponential with
 * more than three decimal places, the "E" is omitted which
 * C++ reads as two numbers.
 */
double TMultiNest::DoubleFromStream(istream& in) {
   string strv;
   in >> strv;

   double v = 0.0;
   if (strv[strv.length() - 4] != '-') {
      istringstream(strv) >> v;
      return v;
   } else {
      return 0.0;
   }
}
