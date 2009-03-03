#include "Math/GAlibMinimizer.h"

#include "Math/IFunction.h"

#include "Riostream.h"

#include <ga/ga.h>
#include <ga/std_stream.h>

#define INSTANTIATE_REAL_GENOME
#include <ga/GARealGenome.h>

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#define MIN_VALUE -10
#define MAX_VALUE 10
#define INC       0.005

#include "Math/IFunction.h"

namespace ROOT { 
namespace Math {
   
GAlibMinimizer::GAlibMinimizer(): 
   fObjective(0), fMinValue(0), fX(0)
{
   fParams = new GAParameterList();
   GASimpleGA::registerDefaultParameters(*fParams);
   fParams->set("minimaxi", -1);

   fAllele = new GAAlleleSetArray<float>();
}

GAlibMinimizer::~GAlibMinimizer()
{
   cout << "Writing parameters... ";
   fParams->write("parameters.dat");
   cout << "done!" << endl;
   if ( fX ) delete fX;
   if ( fParams ) delete fParams;
   if ( fAllele ) delete fAllele;
}

void GAlibMinimizer::Clear() 
{
   if ( fX ) {
      delete fX;
      fX = 0;
   }
}

void GAlibMinimizer::SetFunction(const ROOT::Math::IMultiGenFunction & func) 
{
   fObjective = &func;
}  

bool GAlibMinimizer::SetLimitedVariable(unsigned int , const std::string & , double , double , double lower , double upper ) 
{ 
   fAllele->add(lower, upper);
   return false;
}

bool GAlibMinimizer::SetVariable(unsigned int, const std::string&, double, double) 
{
   //It does nothing! As there is no variable if it has no limits!
   return false;
}

// This objective function returns the sin of the value in the genome.
float GAlibMinimizer::Objective(GAGenome& g) {
  GARealGenome& genome = (GARealGenome &)g;
  double x[] = { genome.gene(0) };
  const ROOT::Math::IBaseFunctionMultiDim& f = 
     *static_cast<const ROOT::Math::IBaseFunctionMultiDim*>(g.userData());
  double value = f(x);
  return value;
}

bool GAlibMinimizer::Minimize() 
{
   assert(fObjective);
   assert(fAllele->size());
   GARealGenome genome(*fAllele, Objective, (void*)fObjective);

   GASimpleGA ga(genome);

   GASigmaTruncationScaling scale;
   ga.scaling(scale);		// set the scaling method to our sharing

   ga.parameters(*fParams);
   ga.initialize();
   
// dump the initial population to file
   ofstream outfile;

   cout << "printing initial population to file..." << endl;
   outfile.open("popi.dat", (STD_IOS_OUT | STD_IOS_TRUNC));
   for(int ii=0; ii<ga.population().size(); ii++){
      genome = ga.population().individual(ii);
      outfile << genome.gene(0) << "\t" << genome.score() << "\n";
   }
   outfile.close();

   while(!ga.done()) ga.step();

// dump the final population to file
   
   cout << "printing final population to file..." << endl;
   outfile.open("popf.dat", (STD_IOS_OUT | STD_IOS_TRUNC));
   for(int i=0; i<ga.population().size(); i++){
      genome = ga.population().individual(i);
      outfile << genome.gene(0) << "\t" << genome.score() << "\n";
   }
   outfile.close();
   
// dump the function to file so you can plot the population on it
   
   cout << "printing function to file..." << endl;
   outfile.open("sinusoid.dat", (STD_IOS_OUT | STD_IOS_TRUNC));
   for(float x=MIN_VALUE; x<=MAX_VALUE; x+=INC){
      outfile << genome.gene(0,x) << "\t" << genome.score() << "\n";
   }
   outfile.close();
   
   // save min values and array
   if ( fX ) delete fX;
   fX = new double[fAllele->size()];
   genome = ga.population().individual(0);
   for ( int i = 0; i < fAllele->size(); ++i )
      fX[i] = genome.gene(i);
   fMinValue = genome.score();
   
   return true;
}  
   
double GAlibMinimizer::MinValue() const 
{
   return (fX?fMinValue:0);
}  

const double *  GAlibMinimizer::X() const { return fX; }  

unsigned int GAlibMinimizer::NCalls() const 
{
   return 0;
}

unsigned int GAlibMinimizer::NDim() const 
{
   return (fObjective?fObjective->NDim():0);
}   
unsigned int GAlibMinimizer::NFree() const 
{
   return NDim();
}   

int GAlibMinimizer::SetProperty(const char* s, const void* v)
{ return fParams->set(s, v); }

int GAlibMinimizer::SetProperty(const char* s, int v)
{ return fParams->set(s, v); }

int GAlibMinimizer::SetProperty(const char* s, unsigned int v)
{ return fParams->set(s, v); }

int GAlibMinimizer::SetProperty(const char* s, char v)
{ return fParams->set(s, v); }

int GAlibMinimizer::SetProperty(const char* s, char* v)
{ return fParams->set(s, v); }

int GAlibMinimizer::SetProperty(const char* s, double v)
{ return fParams->set(s, v); }

// Functions we don't need...
const double *  GAlibMinimizer::MinGradient() const { return 0; }   
bool GAlibMinimizer::ProvidesError() const { return false; }  
const double * GAlibMinimizer::Errors() const { return 0; }
double GAlibMinimizer::Edm() const { return 0; }
double GAlibMinimizer::CovMatrix(unsigned int, unsigned int) const { return 0; }

}
}
