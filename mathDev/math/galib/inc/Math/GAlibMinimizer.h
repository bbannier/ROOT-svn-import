// @(#)root/mathcore:$Id

/**********************************************************************
 *                                                                    *
 * Copyright (c) 2006  LCG ROOT Math Team, CERN/PH-SFT                *
 *                                                                    *
 *                                                                    *
 **********************************************************************/

// Header file for class GAlibMinimizer

#ifndef ROOT_Math_GAlibMinimizer
#define ROOT_Math_GAlibMinimizer

#include <vector>

#include "Math/Minimizer.h"

class GAGenome;
class GAParameterList;
template <class T> class GAAlleleSetArray;

namespace ROOT { 
   namespace Math { 

//_______________________________________________________________________________
/** 
   GAlibMinimizer

   @ingroup MultiMin
*/

class GAlibMinimizer: public ROOT::Math::Minimizer {

// Right from GAStatistics!
public:
  enum {
    NoScores=0x00,
    Mean=0x01,
    Maximum=0x02,
    Minimum=0x04,
    Deviation=0x08,
    Diversity=0x10,
    AllScores=0xff
    };

public: 

   GAlibMinimizer ();
   virtual ~GAlibMinimizer ();

   virtual void Clear();
   using ROOT::Math::Minimizer::SetFunction;
   virtual void SetFunction(const ROOT::Math::IMultiGenFunction & func); 

   virtual bool SetLimitedVariable(unsigned int , const std::string& , double , double , double, double);
   virtual bool SetVariable(unsigned int ivar, const std::string & name, double val, double step); 

   virtual  bool Minimize(); 
   virtual double MinValue() const; 
   virtual const double *  X() const; 

   virtual unsigned int NCalls() const;    
   virtual unsigned int NDim() const;  
   virtual unsigned int NFree() const;  

   int SetProperty(const char*, const void*);
   int SetProperty(const char* s, int v);
   int SetProperty(const char* s, unsigned int v);
   int SetProperty(const char* s, char v);
   int SetProperty(const char* s, char* v);
   int SetProperty(const char* s, double v);

   // Functions not used
   virtual const double *  MinGradient() const;  
   virtual bool ProvidesError() const; 
   virtual const double * Errors() const;
   virtual double Edm() const; 
   virtual double CovMatrix(unsigned int i, unsigned int j) const;  

private:
   const ROOT::Math::IBaseFunctionMultiDim *fObjective;
   double fMinValue;
   double *fX;
   GAParameterList* fParams;
   GAAlleleSetArray<float>* fAllele;

   static float Objective(GAGenome& g);

}; 
   } // end namespace Math
} // end namespace ROOT

#endif /* ROOT_Math_GAlibMinimizer */
