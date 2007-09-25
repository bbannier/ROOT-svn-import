//
// Header file for class GSLMCIntegrator
// 
// Author: Magdalena Slawinska
//

#ifndef ROOT_Math_GSLIntegrator
#define ROOT_Math_GSLIntegrator

#ifndef ROOT_Math_MCIntegrationTypes
#include "Math/MCIntegrationTypes.h"
#endif

#include "Math/IFunctionfwd.h"
#include "Math/IFunction.h"
#include <vector>
//#include "Math/IntegratorMultiDim.h"
#include "Math/GSLMultiMinFunctionAdapter.h"

#include "Math/MCIntegrationTypes.h"
#include "Math/MCParameters.h"


#include <iostream>
/**

@defgroup Integration Numerical Monte Carlo Integration

*/

namespace ROOT {
namespace Math {


   
   class GSLMCIntegrationWorkspace;
   class GSLMonteFunctionWrapper;
   class GSLRngWrapper;
   
   
   /**
      
    Class for performing numerical integration of a multidimensional function.
    It uses the numerical integration algorithms of GSL, which reimplements the
    algorithms used in the QUADPACK, a numerical integration package written in Fortran.
    
    Plain MC, MISER and VEGAS integration algorithms are supported for integration over finite (hypercubic) ranges.
    
    <A HREF="http://www.gnu.org/software/gsl/manual/gsl-ref_16.html#SEC248">GSL Manual</A>.
    
    @ingroup Integration
    */
   
   
   class GSLMCIntegrator {
      
   public:
                  
      // constructors
      
      
      /** Default constructor of GSL MCIntegrator 
      
      @param absTol desired absolute Error
      @param relTol desired relative Error
      @param size maximum number of sub-intervals
      */
      
      GSLMCIntegrator(size_t dim, double absTol = 1.E-6, double relTol = 1E-4, size_t calls = 500000);
            
      
      /** constructor of GSL MCIntegrator. Plain MC is set as default integration type (??)
         
         @param type type of integration. The possible types are defined in the Integration::Type enumeration
         @param absTol desired absolute Error
         @param relTol desired relative Error
         @param size maximum number of sub-intervals
         */
      
      
      GSLMCIntegrator(size_t dim, MCIntegration::Type type, double absTol = 1.E-6, double relTol = 1E-4, size_t calls = 500000);
      
      
      /**
         generic constructor for GSL Integrator
       
       @param type type of integration. The possible types are defined in the Integration::Type enumeration
       @param absTol desired absolute Error
       @param relTol desired relative Error
       @param dim function dimensionality
       @param calls number of function evaluations
       
       */
      /*
      GSLMCIntegrator(const MCIntegration::Type type, const Integration::GKRule rule, double absTol = 1.E-9, double relTol = 1E-6, size_t size = 1000, gsl_rng* r );
      */
      
      virtual ~GSLMCIntegrator();
      //~GSLIntegrator();
      
      // disable copy ctrs
      /*
   private:
         
      GSLMCIntegrator(const GSLMCIntegrator &);
      GSLMCIntegrator & operator=(const GSLMCIntegrator &);
      */
   public:
         
         
         // template methods for generic functors
         
         /**
         method to set the a generic integration function
          
          @param f integration function. The function type must implement the assigment operator, <em>  double  operator() (  double  x ) </em>
          
          */
         
         
      void SetFunction(const IMultiGenFunction &f); 
      
      void SetFunction( GSLMonteFuncPointer f, void * p = 0); 
      
      // methods using GSLMonteFuncPointer
      
      /**
         evaluate the Integral of a function f over the defined hypercube (a,b)
       @param f integration function. The function type must implement the mathlib::IGenFunction interface
       @param a lower value of the integration interval
       @param b upper value of the integration interval
       */
      
      double Integral(const GSLMonteFuncPointer & f, double* a, double* b);
      
      // to be added later    
      //double Integral(const GSLMonteFuncPointer & f);
        
      double Integral(double* a, double* b);
    
      
      double Integral(GSLMonteFuncPointer f, void * p, double* a, double* b);
  
      /**
         return the type of the integration used
       */
      //MCIntegration::Type MCType() const;   
      
      /**
         return  the Result of the last Integral calculation
       */
      double Result() const;
      
      /**
         return the estimate of the absolute Error of the last Integral calculation
       */
      double Error() const;
      
      /**
         return the Error Status of the last Integral calculation
       */
      int Status() const;
      
      
      // setter for control Parameters  (getters are not needed so far )
      
      /**
         set the desired relative Error
       */
      void SetRelTolerance(double relTolerance);
      
      
      /**
         set the desired absolute Error
       */
      void SetAbsTolerance(double absTolerance);
      
      /**
	 to be added later as options for basic MC methods
       The possible rules are defined in the Integration::GKRule enumeration.
       The integration rule can be modified only for ADAPTIVE type integrations
       */
      //void SetIntegrationRule(Integration::GKRule );
      

      /**
	 set random number generator
      */
      void SetGenerator(GSLRngWrapper* r);

      /**
	 set integration method
      */
      void SetType(MCIntegration::Type type);

      /**
	 set integration mode for VEGAS method
      */

      void SetMode(MCIntegration::Mode mode);
         
      /**
	 set default parameters for VEGAS method
      */
      void SetParameters(const VegasParameters &p);

   
      /**
	 set default parameters for MISER method
      */
      void SetParameters(const MiserParameters &p);

      /**
	 set parameters for PLAIN method
      */
      //void SetPParameters(const PlainParameters &p);   

      /**
	 returns the error sigma from the last iteration of the Vegas algorithm
      */  
      double Sigma();

      /**
	 returns chi-squared per degree of freedom for the estimate of the integral in the Vegas algorithm
      */  
      double ChiSqr();


   protected:
         
      // internal method to check validity of GSL function pointer
      bool CheckFunction(); 
      
   private:
      //type of intergation method   
      MCIntegration::Type fType;
       
      //mode for VEGAS integration
      MCIntegration::Mode fMode;
      GSLRngWrapper * fRng;

      double fAbsTol;
      double fRelTol;
      size_t fDim;
      size_t fCalls;
      
      // cache Error, Result and Status of integration
      
      double fResult;
      double fError;
      int fStatus;
      
      
      GSLMCIntegrationWorkspace * fWorkspace;
      GSLMonteFunctionWrapper * fFunction;
     
   };
   
   



} // namespace Math
} // namespace ROOT


#endif /* ROOT_Math_GSLIntegrator */
