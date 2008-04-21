#include "Math/GeneticMinimizer.h"

#include "TMVA/GeneticAlgorithm.h"
#include "TMVA/GeneticFitter.h"
#include "TMVA/IFitterTarget.h"

#include "Math/IFunction.h"

namespace ROOT {
namespace Math {
   
class MultiGenFunctionFitness : public TMVA::IFitterTarget {
private:
   unsigned int NCalls;
   const ROOT::Math::IMultiGenFunction& f;

public:
   MultiGenFunctionFitness(const ROOT::Math::IMultiGenFunction& function) : f(function) { NCalls = 0; }

   unsigned int getNCalls() { return NCalls; }
   unsigned int getNDims() { return f.NDim(); }

   Double_t Evaluate(const std::vector<double> & factors ){
      return f(&factors[0]);
   }

   Double_t EstimatorFunction(std::vector<double> & factors ){
      NCalls += 1;
      return f(&factors[0]);
   }
};

GeneticMinimizer::GeneticMinimizer(): fFitness(0)
{ }

GeneticMinimizer::~GeneticMinimizer()
{
   if ( fFitness )
   {
      delete fFitness;
      fFitness = 0;
   }
}

void GeneticMinimizer::Clear() 
{
   fRanges.clear();
   fResult.clear();
   if ( fFitness )
   {
      delete fFitness;
      fFitness = 0;
   }
}

void GeneticMinimizer::SetFunction(const ROOT::Math::IMultiGenFunction & func) 
{
   Clear();

   fFitness = new MultiGenFunctionFitness(func);
}  

bool GeneticMinimizer::SetLimitedVariable(unsigned int , const std::string & , double , double , double lower , double upper ) 
{ 
   fRanges.push_back( new TMVA::Interval(lower,upper) );

   return true;
}

bool GeneticMinimizer::SetVariable(unsigned int, const std::string&, double, double) 
{
   //It does nothing! As there is no variable if it has no limits!
   return false;
}

bool GeneticMinimizer::Minimize() 
{
   const TString name( "GeneticMinimzer" );
   const TString opts( "PopSize=100:Steps=30:Seed=0" );
   
   TMVA::GeneticFitter mg( *fFitness, name, fRanges, opts);
   //mg.SetParameters( 4, 30, 200, 10,5, 0.95, 0.001 );
   
   // Run the GA and obtain the results
   mg.Run(fResult);

   return true;
}  

double GeneticMinimizer::MinValue() const 
{
   if ( fFitness )
      return static_cast<MultiGenFunctionFitness*>(fFitness)->Evaluate(fResult);
   else
      return 0;
}  

const double *  GeneticMinimizer::X() const { return &fResult[0]; }  

unsigned int GeneticMinimizer::NCalls() const 
{
   if ( fFitness )
      return static_cast<MultiGenFunctionFitness*>(fFitness)->getNCalls();
   else
      return 0;
}

unsigned int GeneticMinimizer::NDim() const 
{
   if ( fFitness )
      return static_cast<MultiGenFunctionFitness*>(fFitness)->getNDims();
   else
      return 0;
}   
unsigned int GeneticMinimizer::NFree() const 
{
   // They should be the same in this case!
   return NDim();
}   

// Functions we don't need...
const double *  GeneticMinimizer::MinGradient() const { return 0; }   
bool GeneticMinimizer::ProvidesError() const { return false; }  
const double * GeneticMinimizer::Errors() const { return 0; }
double GeneticMinimizer::Edm() const { return 0; }
double GeneticMinimizer::CovMatrix(unsigned int, unsigned int) const { return 0; }

}
}
