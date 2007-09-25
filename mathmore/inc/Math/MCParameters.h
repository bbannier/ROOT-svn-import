// structures collecting parameters 
// for MC multidimensional integration
//
// Author Magdalena Slawinska
//

#ifndef ROOT_MCParameters
#define ROOT_MCParameters


struct VegasParameters{
  double sigma;
  double chisq;
  double alpha;
  size_t iterations;
  
  VegasParameters():
    alpha( 1.5),
    iterations(5)
  {} 
 

//int stage;
  //int mode;
  //int verbose;

};


struct MiserParameters{
  double estimate_frac;
  size_t min_calls;
  size_t min_calls_per_bisection;
  double alpha;

  MiserParameters(unsigned int dim):
    estimate_frac(0.1),
    min_calls(16*dim),
    min_calls_per_bisection(32*min_calls) ,
    alpha(2.)
  {}
};

struct PlainParameters{

};

#endif
