/*
 * TMultiNest.h
 *
 *  Created on: Oct 29, 2009
 *      Author: Sven Kreiss, Kyle Cranmer
 *
 */

#ifndef __TMULTINEST_H__
#define __TMULTINEST_H__

#include "TObject.h"
#include "TH2F.h"
#include "TTree.h"
#include <string>

#include "RooStats/MultiNestInterface.h"

using namespace std;


namespace RooStats {

class TMultiNest : public MultiNestInterface {

public:
   TMultiNest(void);
   void Run(void);
   static void LogLikeEggbox(double *Cube, int &ndim, int &npars, double &lnew, void* context);
   void PrintConfig(ostream& out);

   void MModal(int _mmodal) {fMmodal = _mmodal;}
   void Ceff(int _ceff) {fCeff = _ceff;}
   void Nlive(int _nlive) {fNlive = _nlive;}
   void Efr(double _efr) {fEfr = _efr;}
   void Tol(double _tol) {fTol = _tol;}
   void Ndims(int _ndims) {fNdims = _ndims;}
   void Npar(int _nPar) {fNPar = _nPar;}
   void NClsPar(int _nClsPar) {fNClsPar = _nClsPar;}
   void UpdInt(int _updInt) {fUpdInt = _updInt;}
   void Ztol(double _Ztol) {fZtol = _Ztol;}
   void MaxModes(int _maxModes) {fMaxModes = _maxModes;}
   void PWrap(int _pWrap1, int _pWrap2) {fPWrap[0] = _pWrap1; fPWrap[1] = _pWrap2;}
   void RootPath(string _root) {fRoot = _root;}
   void Seed(int _seed) {fSeed = _seed;}
   void Fb(int _fb) {fFb = _fb;}
   void Resume(int _resume) {fResume = _resume;}
   void Outfile(bool _outfile) {fOutfile = _outfile;}
   void InitMPI(bool _initMPI) {fInitMPI = _initMPI;}
   void LogZero(double _logZero) {fLogZero = _logZero;}
   void MaxIter(int _maxIter) {fMaxIter = _maxIter;}
   void Context(void* _context) {fContext = _context;}
   using MultiNestInterface::LogLike;
   void LogLike(void (*_LogLikePtr)(double *Cube, int &ndim, int &npars, double &lnew, void* context)) {
      fLogLikePtr = _LogLikePtr;
   }

   int GetNlive(void) {return fNlive;}
   size_t NumberOfEvaluations(void) {return fNoOfLogLikeEv;}
   void Hist2D(TH2 *h, unsigned int x=0, unsigned int y=1, bool takeExp = false, bool livePointsOnly = false, bool reduceZeroBins = true); ///< deprecated
   void PosteriorHist1D(TH1 *h, unsigned int xCol = 0); ///< deprecated
   void PosteriorHist2D(TH2 *h, unsigned int xCol = 0, unsigned int yCol = 1); ///< deprecated
   //void ProfileFromFile(TProfile2D *h); ///< deprecated


   static TMultiNest *gMultiNest;

   vector< vector<double> >& GetPosteriorDistribution(void) { return fPosteriorDistribution; }
   vector< vector<double> >& GetLivePoints(void) { return fLivePoints; }


protected:
   int fMmodal;
   int fCeff;
   int fNlive;
   double fEfr;
   double fTol;
   int fNdims;
   int fNPar;
   int fNClsPar;
   int fUpdInt;
   double fZtol;
   int fMaxModes;
   int fPWrap[2];
   string fRoot;
   int fSeed;
   int fFb;
   int fResume;
   bool fOutfile; //v2.11
   bool fInitMPI; //v2.11
   double fLogZero; //v2.11
   int fMaxIter; //v2.17
   void* fContext;
   void (*fLogLikePtr)(double *Cube, int &ndim, int &npars, double &lnew, void* context);

   vector< vector<double> >fPosteriorDistribution;
   vector< vector<double> >fLivePoints;

   double DoubleFromStream(istream& in); ///< deprecated

   size_t fNoOfLogLikeEv;

protected:
   ClassDef(TMultiNest,1)
};

}


#endif
