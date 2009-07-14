#include "Math/GeneticMinimizer.h"

#include "TMVA/GeneticAlgorithm.h"
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

GeneticMinimizer::GeneticMinimizer(int ): fFitness(0)
{
   fNsteps=40;
   fPopSize=300;
   fSC_steps=10;
   fSC_rate=5;
   fSC_factor=0.95;
   fConvCrit=0.001;
 }

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

void GeneticMinimizer::SetParameters(Int_t nsteps, Int_t popSize, Int_t SC_steps, 
                                     Int_t SC_rate, Double_t SC_factor, Double_t convCrit )
{
   fNsteps = nsteps;
   fPopSize = popSize;
   fSC_steps = SC_steps;
   fSC_rate = SC_rate;
   fSC_factor = SC_factor;
   fConvCrit = convCrit;   
}

bool GeneticMinimizer::Minimize() 
{
   TMVA::GeneticAlgorithm mg( *fFitness, fPopSize, fRanges );
   
   do {
      mg.Init();
      
      mg.CalculateFitness();
      
      // Just for debugging options
      //mg.GetGeneticPopulation().Print(0);
      
      mg.GetGeneticPopulation().TrimPopulation();
      
      mg.SpreadControl( fSC_steps, fSC_rate, fSC_factor );
      
   } while (!mg.HasConverged( fNsteps, fConvCrit ));  // converged if: fitness-improvement < CONVCRIT within the last CONVSTEPS loops
   
   TMVA::GeneticGenes* genes = mg.GetGeneticPopulation().GetGenes( 0 );
   std::vector<Double_t> gvec;
   gvec = genes->GetFactors();

   fResult = gvec;   

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
