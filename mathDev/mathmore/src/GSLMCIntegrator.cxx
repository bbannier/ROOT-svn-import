//
// implementation file for class GSLMCIntegrator
// Author: Magdalena Slawinska
// 
//

#include "Math/IFunctionfwd.h"
#include "Math/IFunction.h"
#include <vector>

#include "GSLMonteFunctionWrapper.h"

#include "Math/GSLMCIntegrator.h"
#include "GSLMCIntegrationWorkspace.h"
#include "GSLRngWrapper.h"


#include "gsl/gsl_monte_vegas.h"
#include "gsl/gsl_monte_miser.h"
#include "gsl/gsl_monte_plain.h"
/**

@defgroup Integration Numerical Monte Carlo Integration

*/


namespace ROOT {
namespace Math {


   
   /**
      
    Class for performing numerical integration of a multidimensional function.
    It uses the numerical integration algorithms of GSL, which reimplements the
    algorithms used in the QUADPACK, a numerical integration package written in Fortran.
    
    Plain MC, MISER and VEGAS integration algorithms are supported for integration over finite (hypercubic) ranges.
    
    <A HREF="http://www.gnu.org/software/gsl/manual/gsl-ref_16.html#SEC248">GSL Manual</A>.
    
    @ingroup Integration
    */
   
   
                   
      // constructors
      
      
      /** Default constructor of GSL Integrator for Adaptive Singular integration
      
      @param absTol desired absolute Error
      @param relTol desired relative Error
      @param size maximum number of sub-intervals
      */
      
     GSLMCIntegrator::GSLMCIntegrator(size_t dim, double absTol, double relTol, size_t calls):
     fAbsTol(absTol),
     fRelTol(relTol),
     fDim(dim),
     fCalls(calls),
     //fr(r),
     fWorkspace(0),
     fFunction(0)
     {
       /*
       if (type ==  MCIntegration::VEGAS)
	fWorkspace = new GSLVegasIntegrationWorkspace(fDim);
      else if (fType ==  MCIntegration::MISER) 
	fWorkspace = new GSLMiserIntegrationWorkspace(fDim);
	else if (fType ==  MCIntegration::PLAIN)  
	fWorkspace = new GSLPlainIntegrationWorkspace(fDim);*/
	//else 
	//std::cerr << "Wrong integration type" << std::endl;*/
       fRng = new GSLRngWrapper();      
       fRng->Allocate();
      
     }    

        
      
      /** constructor of GSL MCIntegrator.Plain MC is set as default integration type
         
         @param type type of integration. The possible types are defined in the Integration::Type enumeration
         @param absTol desired absolute Error
         @param relTol desired relative Error
         @param size maximum number of sub-intervals
         */
      
      
     GSLMCIntegrator::GSLMCIntegrator(size_t dim, MCIntegration::Type type, double absTol, double relTol, size_t calls):
      fType(type),
      fAbsTol(absTol),
      fRelTol(relTol),
      fDim(dim),
      //fr(r),
      fCalls(calls),
      fWorkspace(0),
      fFunction(0)
      {
	//set Workspace according to type
	SetType(type);

	//set random number generator
	fRng = new GSLRngWrapper();      
        fRng->Allocate();
      
      
     }
       
 
       //maybe to be added later; for various rules within basic methods
      //GSLIntegrator(const Integration::Type type, const Integration::GKRule rule, double absTol = 1.E-9, double relTol = 1E-6, size_t size = 1000);
      
      
      GSLMCIntegrator::~GSLMCIntegrator()
     {
       // delete workspace 
       if (fWorkspace) delete fWorkspace;
       if (fRng != 0) delete fRng;
       if (fFunction != 0) delete fFunction;
        fRng = 0;

     }
      
      
      // disable copy ctrs
  
         
     //GSLMCIntegrator(const GSLMCIntegrator &);
     //GSLMCIntegrator & operator=(const GSLMCIntegrator &);
      
   
         
         
     
         
     /**
       method to set the a generic integration function
          
       @param f integration function. The function type must implement the assigment operator, <em>  double  operator() (  double  x ) </em>
          
     */
         
      void GSLMCIntegrator::SetFunction(const IMultiGenFunction &f)
     {
       if(fFunction == 0) fFunction = new  GSLMonteFunctionWrapper();
       fFunction->SetFunction(f);
     } 
      
      void GSLMCIntegrator::SetFunction( GSLMonteFuncPointer f, void * p )
     {
       if(fFunction == 0) fFunction = new  GSLMonteFunctionWrapper();
       fFunction->SetFuncPointer( f );
       fFunction->SetParams ( p );
     }



     double GSLMCIntegrator::Integral(double* a, double* b)
     {
       assert(fRng != 0);
       gsl_rng* fr = fRng->Rng();
       assert(fr != 0);
       if (!CheckFunction()) return 0;  
       if ( fType == MCIntegration::VEGAS) 
       {
    	 GSLVegasIntegrationWorkspace * ws = dynamic_cast<GSLVegasIntegrationWorkspace *>(fWorkspace); 
	 assert(ws != 0);
	 if(fMode == MCIntegration::IMPORTANCE) ws->GetWS()->mode = 0;
	 else if(fMode == MCIntegration::STRATIFIED) ws->GetWS()->mode = 1;
	 else if(fMode == MCIntegration::IMPORTANCE_ONLY) ws->GetWS()->mode = 2;

         fStatus = gsl_monte_vegas_integrate( fFunction->GetFunc(), a, b , fDim, fCalls, fr, ws->GetWS(),  &fResult, &fError);
       }
       else if (fType ==  MCIntegration::MISER) 
       {
	 GSLMiserIntegrationWorkspace * ws = dynamic_cast<GSLMiserIntegrationWorkspace *>(fWorkspace); 
	 assert(ws != 0); 
         fStatus = gsl_monte_miser_integrate( fFunction->GetFunc(), a, b , fDim, fCalls, fr, ws->GetWS(),  &fResult, &fError);
       }
       else if (fType ==  MCIntegration::PLAIN) 
       {
	 GSLPlainIntegrationWorkspace * ws = dynamic_cast<GSLPlainIntegrationWorkspace *>(fWorkspace); 
	 assert(ws != 0); 
           fStatus = gsl_monte_plain_integrate( fFunction->GetFunc(), a, b , fDim, fCalls, fr, ws->GetWS(),  &fResult, &fError);
       }
       /**/
       else 
       {
      
	 fResult = 0;
	 fError = 0;
	 fStatus = -1;
	 std::cerr << "GSLIntegrator - Error: Unknown integration type" << std::endl;
	 throw std::exception(); 
       }
   
       return fResult;
   
     }

      /**
         evaluate the Integral of a function f over the defined interval (a,b)
       @param f integration function. The function type must implement the mathlib::IGenFunction interface
       @param a lower value of the integration interval
       @param b upper value of the integration interval
       */
     
      double GSLMCIntegrator::Integral(const GSLMonteFuncPointer & f, double* a, double* b)
      {
          SetFunction(f,(void*)0);
          return Integral(a,b);
      }


      /* to be added later           
      double GSLMCIntegrator::Integral(GSLMonteFuncPointer f, void * p, double* a, double* b)
     {

     }
      
     */
  //MCIntegration::Type GSLMCIntegrator::MCType() const {return fType;}

      /**
         return  the Result of the last Integral calculation
       */
      double GSLMCIntegrator::Result() const { return fResult; }
      
      /**
         return the estimate of the absolute Error of the last Integral calculation
       */
      double GSLMCIntegrator::Error() const { return fError; }
      
      /**
         return the Error Status of the last Integral calculation
       */
      int GSLMCIntegrator::Status() const { return fStatus; }
      
      
      // setter for control Parameters  (getters are not needed so far )
      
      /**
         set the desired relative Error
       */
      void GSLMCIntegrator::SetRelTolerance(double relTol){ this->fRelTol = relTol; }
           
      /**
         set the desired absolute Error
       */
      void GSLMCIntegrator::SetAbsTolerance(double absTol){ this->fAbsTol = absTol; }
           
      void GSLMCIntegrator::SetGenerator(GSLRngWrapper* r){ this->fRng = r; } 
      
      /**
	 set integration type
      */
  void GSLMCIntegrator::SetType (MCIntegration::Type type)
 {
  
   if(type ==  ROOT::Math::MCIntegration::VEGAS)
   {
     fType=type;
     fWorkspace = new GSLVegasIntegrationWorkspace(fDim);
	  
   }

   else if (type ==  ROOT::Math::MCIntegration::MISER) 
   {
     fType=type;
     fWorkspace = new GSLMiserIntegrationWorkspace(fDim);
   }
   else if (type ==  ROOT::Math::MCIntegration::PLAIN)   
   {
     fType=type;
     fWorkspace =new GSLPlainIntegrationWorkspace(fDim);
     }
   else 
     {
        std::cerr << "GSLIntegrator - Error: Unknown integration type" << std::endl;
        throw std::exception(); 
     }
}  

     /**
	 set integration mode for VEGAS method
      */

      void GSLMCIntegrator::SetMode(MCIntegration::Mode mode)
      {
	if(fType ==  ROOT::Math::MCIntegration::VEGAS)
	{  fMode = mode; }

	else std::cerr << "Mode not matching integration type";
      }


      /**
	 set method parameters
       */
      void GSLMCIntegrator::SetParameters(const VegasParameters &p)
      {
         if (fType ==  MCIntegration::VEGAS) 
	 {
	   GSLVegasIntegrationWorkspace * ws = dynamic_cast<GSLVegasIntegrationWorkspace *>(fWorkspace); 
	   assert(ws != 0);
	   ws->SetParameters(p);
	 }
	 else 
	   std::cerr << "GSLIntegrator - Error: Parameters not mathing integration type" << std::endl;
      }

      void GSLMCIntegrator::SetParameters(const MiserParameters &p)
      {
         if (fType ==  MCIntegration::MISER) 
	 {
	   GSLMiserIntegrationWorkspace * ws = dynamic_cast<GSLMiserIntegrationWorkspace *>(fWorkspace); 
	   assert(ws != 0); 
	   ws->SetParameters(p);
	 }
	 else
	   std::cerr << "GSLIntegrator - Error: Parameters not mathing integration type" << std::endl;
      }

	


  //----------- methods specific for VEGAS

      /**
	 returns the error sigma from the last iteration of the VEGAS algorithm
      */  
      double GSLMCIntegrator::Sigma()
      {
	if(fType == MCIntegration::VEGAS)
	{
	  GSLVegasIntegrationWorkspace * ws = dynamic_cast<GSLVegasIntegrationWorkspace *>(fWorkspace);
	  return ws->GetWS()->sigma;
	}
	else 
	  {  
	    std::cerr << "Parameter not mathcing integration type";
	    return 0;
	  }

      }


      /**
	 returns chi-squared per degree of freedom for the estimate of the integral
      */  
      double GSLMCIntegrator::ChiSqr()
      {
	if(fType == MCIntegration::VEGAS)
	{
	    GSLVegasIntegrationWorkspace * ws = dynamic_cast<GSLVegasIntegrationWorkspace *>(fWorkspace);
	   return ws->GetWS()->chisq;
	}
	else 
	{
	  std::cerr << "Parameter not mathcing integration type";
	  return 0;
	}
      }



      // internal method to check validity of GSL function pointer
      bool GSLMCIntegrator::CheckFunction() 
      { 
	return true;
	/*
	// check if a function has been previously set.
	if (fFunction->IsValid()) return true; 
	fStatus = -1; fResult = 0; fError = 0;
	std::cerr << "GS:Integrator - Error : Function has not been specified " << std::endl; 
	return false; */
      }
      
 
    

} // namespace Math
} // namespace ROOT



