/*
 * MultiNest.h
 *
 *  Created on: Oct 29, 2009
 *      Author: Sven Kreiss, Kyle Cranmer
 *
 */

#ifndef __ROOSTATS_MULTINESTINTERFACE_H__
#define __ROOSTATS_MULTINESTINTERFACE_H__

#include "TObject.h"
#include "TH2F.h"
#include "TTree.h"
#include <string>
#include <iostream>

#include "RooAbsReal.h"
#include "RooArgList.h"

using namespace std;


namespace RooStats {

class MultiNestInterface {

public:
   MultiNestInterface(void) {}
   virtual ~MultiNestInterface ()  {}

private:
   MultiNestInterface(const MultiNestInterface&) {}

   MultiNestInterface & operator = (const MultiNestInterface & rhs)  {
      if (this == &rhs) return *this;  // time saving self-test
      return *this;
   }

public:
   ///////////////////////////////
   // TMultiNest interface

   virtual void Run(void) = 0;
   virtual void PrintConfig(ostream& out) = 0;

   virtual void MModal(int _mmodal) = 0;
   virtual void Ceff(int _ceff) = 0;
   virtual void Nlive(int _nlive) = 0;
   virtual void Efr(double _efr) = 0;
   virtual void Tol(double _tol) = 0;
   virtual void Ndims(int _ndims) = 0;
   virtual void Npar(int _nPar) = 0;
   virtual void NClsPar(int _nClsPar) = 0;
   virtual void UpdInt(int _updInt) = 0;
   virtual void Ztol(double _Ztol) = 0;
   virtual void MaxModes(int _maxModes) = 0;
   virtual void PWrap(int _pWrap1, int _pWrap2) = 0;
   virtual void RootPath(string _root) = 0;
   virtual void Seed(int _seed) = 0;
   virtual void Fb(int _fb) = 0;
   virtual void Resume(int _resume) = 0;
   virtual void Outfile(bool _outfile) = 0;
   virtual void InitMPI(bool _initMPI) = 0;
   virtual void LogZero(double _logZero) = 0;
   virtual void Context(void* _context) = 0;
   virtual void LogLike(void (*_LogLikePtr)(double *Cube, int &ndim, int &npars, double &lnew, void* context)) = 0;

   virtual int GetNlive(void) = 0;
   virtual size_t NumberOfEvaluations(void) = 0;


   virtual vector< vector<double> >& GetPosteriorDistribution(void) = 0;
   virtual vector< vector<double> >& GetLivePoints(void) = 0;





   /////////////////////////////////////////
   // actual MultiNest interface

   virtual void LogLike(RooAbsReal* /*logLike*/, RooArgList& /*argList*/) { cout << "ERROR" << endl; }
   virtual void NegLogLike(RooAbsReal* /*logLike*/, RooArgList& /*argList*/) { cout << "ERROR" << endl; }

   virtual void SetPrior(RooAbsPdf* /*p*/) { cout << "ERROR" << endl; }

protected:
   ClassDef(MultiNestInterface,1)
};

} // end namespace RooStats


#endif
