#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TGraphErrors.h"
#include "TGraph2DErrors.h"
#include "TTree.h"
#include "TF1.h"
#include "TF2.h"

#include "Math/IFunction.h"
#include "Math/IParamFunction.h"
#include "TMath.h"
#include "Math/DistFunc.h"

#include "TUnuran.h"
#include "TUnuranMultiContDist.h"
#include "Math/MinimizerOptions.h"
#include "TBackCompFitter.h"
#include "TVirtualFitter.h"

#include "TRandom3.h"

#include "TROOT.h"
//#include "RConfigure.h"
#include "TCanvas.h"
#include "TApplication.h"

#include <vector>
#include <string>
#include <cassert>
#include <cmath>

#include "Riostream.h"
using namespace std;
#undef R__HAS_MATHMORE

const unsigned int __DRAW__ = 0;

TRandom3 rndm;

enum cmpOpts {
   cmpNone = 0,
   cmpPars = 1,
   cmpDist = 2,
   cmpChi2 = 4,
   cmpErr  = 8,
};

struct RefValue {
   const double* pars;
   const double  chi;
};

class CompareResult {
public:
   struct RefValue* refValue;
   int opts;
   double tolPar;
   double tolChi2;
   CompareResult(int _opts = cmpPars, double _tolPar = 3, double _tolChi2 = 0.01):
      refValue(0), opts(_opts), tolPar(_tolPar), tolChi2(_tolChi2) {};

   CompareResult(CompareResult const& copy):
      refValue(copy.refValue), opts(copy.opts), 
      tolPar(copy.tolPar), tolChi2(copy.tolChi2) {};

   void setRefValue(struct RefValue* _refValue)
   { 
      refValue = _refValue; 
   };

   int parameters(int npar, double val, double ref) const
   { 
      double ret = 0;
      if ( refValue && (opts & cmpPars) ) 
      {
         ret = compareResult(val, refValue->pars[npar], tolPar*ref);
//          printf("[TOL:%f]", ref);
      }
      return ret;
   };

   int chi2(double val) const
   { return ( refValue && (opts & cmpChi2) ) ? compareResult(val, refValue->chi, tolChi2) : 0; };

public:
   int compareResult(double v1, double v2, double tol = 0.01) const { 
      if (std::abs(v1-v2) > tol ) return 1; 
      return 0; 
   } 
};

double chi2FromFit(const TF1 * func )  { 
   // return last chi2 obtained from Fit method function
   assert(TVirtualFitter::GetFitter() != 0 ); 
   return (TVirtualFitter::GetFitter()->Chisquare(func->GetNpar(), func->GetParameters() ) );
}

void FillVariableRange(Double_t v[], Int_t numberOfBins, Double_t minRange, Double_t maxRange)
{
   Double_t minLimit = (maxRange-minRange)  / (numberOfBins*2);
   Double_t maxLimit = (maxRange-minRange)*4/ (numberOfBins);   
   v[0] = 0;
   for ( Int_t i = 1; i < numberOfBins + 1; ++i )
   {
      Double_t limit = rndm.Uniform(minLimit, maxLimit);
      v[i] = v[i-1] + limit;
   }
   
   Double_t k = (maxRange-minRange)/v[numberOfBins];
   for ( Int_t i = 0; i < numberOfBins + 1; ++i )
   {
      v[i] = v[i] * k + minRange;
   }
}

class algoType {
public:
   const char* type;
   const char* algo;
   const char* opts;
   CompareResult cmpResult;
   
   algoType(): type(0), algo(0), opts(0), cmpResult(0) {}

   algoType(const char* s1, const char* s2, const char* s3, 
            CompareResult _cmpResult):
      type(s1), algo(s2), opts(s3), cmpResult(_cmpResult) {}
};

vector<struct algoType> commonAlgos;
vector<struct algoType> treeFail;
vector<struct algoType> specialAlgos;
vector<struct algoType> noGraphAlgos;
vector<struct algoType> fumili;
vector<struct algoType> histGaus2D;

class ParLimit {
public:
   int npar;
   double min;
   double max;
   ParLimit(int _npar = 0, double _min = 0, double _max = 0): npar(_npar), min(_min), max(_max) {};
};

void SetParsLimits(vector<ParLimit>& v, TF1* func)
{
   for ( vector<ParLimit>::iterator it = v.begin();
         it !=  v.end(); ++it ) {
//       printf("Setting parameters: %d, %f, %f\n", (*it)->npar, (*it)->min, (*it)->max);
      func->SetParLimits( it->npar, it->min, it->max);
   }
}

class fitFunctions {
public:
   const char* name;
   double (*func)(double*, double*);
   unsigned int npars;
   vector<double> origPars;
   vector<double> fitPars;
   vector<ParLimit> parLimits;

   fitFunctions() {}

   fitFunctions(const char* s1, double (*f)(double*, double*),
                unsigned int n,
                double* v1, double* v2,
                vector<ParLimit>& limits):
      name(s1), func(f), npars(n), 
      origPars(npars), fitPars(npars), parLimits(limits.size())
   {
      copy(v1, v1 + npars, origPars.begin());
      copy(v2, v2 + npars, fitPars.begin());
      copy(limits.begin(), limits.end(), parLimits.begin());
   }
};

vector<struct fitFunctions> l1DFunctions;
vector<struct fitFunctions> l2DFunctions;
vector<struct fitFunctions> treeFunctions;

Double_t gaus1DImpl(Double_t* x, Double_t* p)
{
   return p[2]*TMath::Gaus(x[0], p[0], p[1]);
}

Double_t poly1DImpl(Double_t *x, Double_t *p)
{
   Double_t xx = x[0];
   return p[0]*xx*xx*xx+p[1]*xx*xx+p[2]*xx+p[3];
}


Double_t gaus2DImpl(Double_t *x, Double_t *p)
{
   return p[0]*TMath::Gaus(x[0], p[1], p[2])*TMath::Gaus(x[1], p[3], p[4]);
}

double gausNormal(Double_t* x, Double_t* p)
{
   return p[2]*TMath::Gaus(x[0],p[0],p[1],1);
}

double gaus2dnormal(double *x, double *p) { 
  
   double mu_x = p[0];
   double sigma_x = p[1];
   double mu_y = p[2];
   double sigma_y = p[3];
   double rho = p[4]; 
   double u = (x[0] - mu_x)/ sigma_x ;
   double v = (x[1] - mu_y)/ sigma_y ;
   double c = 1 - rho*rho ;
   double result = (1 / (2 * TMath::Pi() * sigma_x * sigma_y * sqrt(c))) 
      * exp (-(u * u - 2 * rho * u * v + v * v ) / (2 * c));
   return result;
}

double gausNd(double *x, double *p) { 

   double f = gaus2dnormal(x,p); 
   f *= ROOT::Math::normal_pdf(x[2],p[6],p[5]); 
   f *= ROOT::Math::normal_pdf(x[3],p[8],p[7]); 
   f *= ROOT::Math::normal_pdf(x[4],p[10],p[9]); 
   f *= ROOT::Math::normal_pdf(x[5],p[12],p[11]); 

   if (f <= 0) { 
      std::cout << "invalid f value " << f << " for x "; 
      for (int i = 0; i < 6; ++i) std::cout << "  " << x[i]; 
      std::cout << "\t P = "; 
      for (int i = 0; i < 11; ++i) std::cout << "  " << p[i]; 
      std::cout << "\n\n ";
      return 1.E-300;
   } 
   else if (f > 0) return f; 
   
   std::cout << " f is a nan " << f << std::endl; 
   for (int i = 0; i < 6; ++i) std::cout << "  " << x[i]; 
   std::cout << "\t P = "; 
   for (int i = 0; i < 11; ++i) std::cout << "  " << p[i]; 
   std::cout << "\n\n ";
   Error("gausNd","f is a nan");
   assert(1);
   return 0; 
}

double minX = -5.;
double maxX = +5.;
double minY = -5.;
double maxY = +5.;
int nbinsX = 30;
int nbinsY = 30;

enum testOpt {
   testOptPars  = 1,
   testOptChi   = 2,
   testOptErr   = 4,
   testOptColor = 8,
   testOptDebug = 16,
   testOptCheck = 32,
};

int defaultOptions = testOptColor | testOptCheck;// | testOptDebug;

template <typename T>
void printTestName(T* object, TF1* func)
{
   string str = "Test For Object '";
   str += object->GetName();
   str += "' with '";
   str += func->GetName();
   str += "'...";
   while ( str.length() != 65 )
      str += '.';
   printf("%s", str.c_str());
}

void printTitle(TF1* func)
{
   printf("\nMin Type    | Min Algo    | OPT  | PARAMETERS             ");
   int n = func->GetNpar();
   for ( int i = 1; i < n; ++i ) {
      printf("                       ");
   }
   printf(" | CHI2TEST        | ERRORS \n");
   fflush(stdout);
}

void printSeparator()
{
   fflush(stdout);
   printf("*********************************************************************"
          "********************************************************************\n");
   fflush(stdout);
}

void setColor(int red = 0)
{
   char command[13];
   if ( red ) 
      sprintf(command, "%c[%d;%d;%dm", 0x1B, 1, 1 + 30, 8 + 40);
   else 
      sprintf(command, "%c[%d;%d;%dm", 0x1B, 0, 0 + 30, 8 + 40);
   printf("%s", command);
}

int testFit(const char* str1, const char* str2, const char* str3,
               TF1* func, CompareResult const& cmpResult, int opts)
{
   bool debug = opts & testOptDebug;
   // so far, status will just count the number of parameters wronly
   // calculated. There is no other test of the fitters
   int status = 0;
   int diff = 0;

   double chi2 = 0;
   if (  opts & testOptChi || opts & testOptCheck )
         chi2 = chi2FromFit(func);

   fflush(stdout);
   if ( opts & testOptPars ) 
   {
      int n = func->GetNpar();
      double* values = func->GetParameters();
      if ( debug )
         printf("%-11s | %-11s | %-4s | ", str1, str2, str3);
      for ( int i = 0; i < n; ++i ) {
         if ( opts & testOptCheck )
            diff = cmpResult.parameters(i,
                                        values[i], 
                                        std::max(std::sqrt(chi2/func->GetNDF()),1.0)*func->GetParError(i));
         status += diff;
         if ( opts & testOptColor )
            setColor ( diff );
         if ( debug )
            printf("%10.6f +/-(%-6.3f) ", values[i], func->GetParError(i));
         fflush(stdout);
      }
      setColor(0);
   }

   if ( opts & testOptChi )
   {
      if ( debug )
         printf(" | chi2: %9.4f | ",  chi2);
   }

   if ( opts & testOptErr )
   {
      assert(TVirtualFitter::GetFitter() != 0 ); 
      TBackCompFitter* fitter = dynamic_cast<TBackCompFitter*>( TVirtualFitter::GetFitter() );
      assert(fitter != 0);
      const ROOT::Fit::FitResult& fitResult = fitter->GetFitResult();
      if ( debug )
         printf("err: ");
      int n = func->GetNpar();
      for ( int i = 0; i < n; ++i ) {
         if ( debug )
            printf("%c ", (fitResult.LowerError(i) == fitResult.UpperError(i))?'E':'D');
      }
      if ( debug )
         printf("| ");
   }

   if ( opts != 0 ) 
   {
      setColor(0);
      if ( debug )
         printf("\n");
   }
   fflush(stdout);

   return status;
}

template <typename T, typename F>
int testFitters(T* object, F* func, vector< vector<struct algoType> > listAlgos, struct fitFunctions const& fitFunction)
{
   // counts the number of parameters wronly calculated
   int status = 0;
   int numberOfTests = 0;
   const double* origpars = &(fitFunction.origPars[0]);
   const double* fitpars = &(fitFunction.fitPars[0]);

   func->SetParameters(fitpars);
   
   printTestName(object, func);
   ROOT::Math::MinimizerOptions::SetDefaultMinimizer(commonAlgos[0].type, commonAlgos[0].algo);
   object->Fit(func, "Q0");
   if ( defaultOptions & testOptDebug ) printTitle(func);
   struct RefValue ref = { origpars, chi2FromFit(func) };
   commonAlgos[0].cmpResult.setRefValue(&ref);
   int defMinOptions = testOptPars | testOptChi | testOptErr | defaultOptions;
   status += testFit(commonAlgos[0].type, commonAlgos[0].algo
                     , commonAlgos[0].opts, func
                     , commonAlgos[0].cmpResult, defMinOptions);
   numberOfTests += 1;

   if ( defaultOptions & testOptDebug )
   {
      printSeparator();
      func->SetParameters(origpars);
      status += testFit("Parameters", "Original", "", func, commonAlgos[0].cmpResult, testOptPars | testOptDebug);
      func->SetParameters(fitpars);
      status += testFit("Parameters", "Initial",  "", func, commonAlgos[0].cmpResult, testOptPars | testOptDebug);
      printSeparator();
   }

   for ( unsigned int j = 0; j < listAlgos.size(); ++j )
   {
      for ( unsigned int i = 0; i < listAlgos[j].size(); ++i ) 
      {
         int testFitOptions = testOptPars | testOptChi | testOptErr | defaultOptions;
         ROOT::Math::MinimizerOptions::SetDefaultMinimizer(listAlgos[j][i].type, listAlgos[j][i].algo);
         func->SetParameters(fitpars);
         fflush(stdout);
         object->Fit(func, listAlgos[j][i].opts);
         listAlgos[j][i].cmpResult.setRefValue(&ref);
         status += testFit(listAlgos[j][i].type, listAlgos[j][i].algo, listAlgos[j][i].opts
                           , func, listAlgos[j][i].cmpResult, testFitOptions);
         numberOfTests += 1;
         fflush(stdout);
      }
   }
   
   double percentageFailure = double( status * 100 ) / double( numberOfTests*func->GetNpar() );

   if ( defaultOptions & testOptDebug ) 
   {
      printSeparator();
      printf("Number of fails: %d Total Number of tests %d", status, numberOfTests);
      printf(" Percentage of failure: %f\n", percentageFailure );
   }

   // limit in the percentage of failure!
   return (percentageFailure < 4)?0:1;
}

int test1DObjects()
{
   // Counts how many tests failed.
   int globalStatus = 0;
   // To control if an individual test failed
   int status = 0;

   TF1* func = 0;
   TH1D* h1 = 0;
   TH1D* h2 = 0;
   TGraph* g1 = 0;
   TGraphErrors* ge1 = 0;
   TCanvas *c0 = 0, *c1 = 0, *c2 = 0, *c3 = 0;
   for ( unsigned int j = 0; j < l1DFunctions.size(); ++j )
   {
      if ( func ) delete func;
      func = new TF1( l1DFunctions[j].name, l1DFunctions[j].func, minX, maxX, l1DFunctions[j].npars);
      func->SetParameters(&(l1DFunctions[j].origPars[0]));
      SetParsLimits(l1DFunctions[j].parLimits, func);

      // fill an histogram 
      if ( h1 ) delete h1;
      h1 = new TH1D("Histogram 1D","h1-title",nbinsX,minX,maxX);
      for ( int i = 0; i < h1->GetNbinsX() + 1; ++i )
         h1->Fill( h1->GetBinCenter(i), rndm.Poisson( func->Eval( h1->GetBinCenter(i) ) ) );

      double v[nbinsX];
      FillVariableRange(v, nbinsX, minX, maxX);
      if ( h2 ) delete h2;
      h2 = new TH1D("Histogram 1D Variable","h2-title",nbinsX, v);
      for ( int i = 0; i < h2->GetNbinsX() + 1; ++i )
         h2->Fill( h2->GetBinCenter(i), rndm.Poisson( func->Eval( h2->GetBinCenter(i) ) ) );

      vector< vector<struct algoType> > listAlgos(5);
      listAlgos[0] = commonAlgos;
      listAlgos[1] = treeFail;
      listAlgos[2] = specialAlgos;
      listAlgos[3] = noGraphAlgos;
      listAlgos[4] = fumili;

      if ( c0 ) delete c0;
      c0 = new TCanvas("c0-1D", "Histogram1D Variable");
      if ( __DRAW__ ) h2->Draw();
      globalStatus += status = testFitters(h2, func, listAlgos, l1DFunctions[j]);
      printf("%s\n", (status?"FAILED":"OK"));

      if ( c1 ) delete c1;
      c1 = new TCanvas("c1-1D", "Histogram1D");
      if ( __DRAW__ ) h1->Draw();
      globalStatus += status = testFitters(h1, func, listAlgos, l1DFunctions[j]);
      printf("%s\n", (status?"FAILED":"OK"));
      
      if ( g1 ) delete g1;
      g1 = new TGraph(h1);
      g1->SetName("TGraph 1D");
      g1->SetTitle("TGraph 1D - title");
      if ( c2 ) delete c2;
      c2 = new TCanvas("c2-1D","TGraph");
      if ( __DRAW__ ) g1->Draw("AB*");
      vector< vector<struct algoType> > listAlgosGraph(4);
      listAlgosGraph[0] = commonAlgos;
      listAlgosGraph[1] = treeFail;
      listAlgosGraph[2] = specialAlgos;
      listAlgosGraph[3] = fumili;
      globalStatus += status = testFitters(g1, func, listAlgosGraph, l1DFunctions[j]);
      printf("%s\n", (status?"FAILED":"OK"));

      if ( ge1 ) delete ge1;
      ge1 = new TGraphErrors(h1);
      ge1->SetName("TGraphErrors 1D");
      ge1->SetTitle("TGraphErrors 1D - title");
      if ( c3 ) delete c3;
      c3 = new TCanvas("c3-1D","TGraphError");
      if ( __DRAW__ ) ge1->Draw("AB*");

      vector< vector<struct algoType> > listAlgosGE(3);
      listAlgosGE[0] = commonAlgos;
      listAlgosGE[1] = treeFail;
      listAlgosGE[2] = specialAlgos;
      globalStatus += status = testFitters(ge1, func, listAlgosGE, l1DFunctions[j]);
      printf("%s\n", (status?"FAILED":"OK"));
   }

   if ( ! __DRAW__ )
   {
      delete func;
      delete h1;
      delete h2;
      delete g1;
      delete ge1;
      delete c0;
      delete c1;
      delete c2;
      delete c3;
   }

   return globalStatus;
}

int test2DObjects()
{
   // Counts how many tests failed.
   int globalStatus = 0;
   // To control if an individual test failed
   int status = 0;

   TF2* func = 0;
   TH2D* h1 = 0;
   TH2D* h2 = 0;
   TGraph2D* g1 = 0;
   TGraph2DErrors* ge1 = 0;
   TCanvas *c0 = 0, *c1 = 0, *c2 = 0, *c3 = 0;
   for ( unsigned int h = 0; h < l2DFunctions.size(); ++h )
   {
      if ( func ) delete func;
      func = new TF2( l2DFunctions[h].name, l2DFunctions[h].func, minX, maxX, minY, maxY, l2DFunctions[h].npars);
      func->SetParameters(&(l2DFunctions[h].origPars[0]));
      SetParsLimits(l2DFunctions[h].parLimits, func);
      
      // fill an histogram 
      if ( h1 ) delete h1;
      h1 = new TH2D("Histogram 2D","h1-title",nbinsX,minX,maxX,nbinsY,minY,maxY);
      if ( ge1 ) delete ge1;
      ge1 = new TGraph2DErrors((h1->GetNbinsX() + 1) * (h1->GetNbinsY() + 1));
      unsigned int counter = 0;
      for ( int i = 0; i < h1->GetNbinsX() + 1; ++i )
         for ( int j = 0; j < h1->GetNbinsY() + 1; ++j ) 
         {
            double xc = h1->GetXaxis()->GetBinCenter(i);
            double yc = h1->GetYaxis()->GetBinCenter(j);
            double content = rndm.Poisson( func->Eval( xc, yc ) );
            h1->Fill( xc, yc, content );
            ge1->SetPoint(counter, xc, yc, content);
            ge1->SetPointError(counter, 
                               h1->GetXaxis()->GetBinWidth(i) / 2,
                               h1->GetYaxis()->GetBinWidth(j) / 2,
                               h1->GetBinError(i,j));
            counter += 1;
         }

      if ( h2 ) delete h2;
      double x[nbinsX];
      FillVariableRange(x, nbinsX, minX, maxX);
      double y[nbinsY];
      FillVariableRange(y, nbinsY, minY, maxY);
      h2 = new TH2D("Histogram 2D Variable","h2-title",nbinsX, x, nbinsY, y);
      for ( int i = 0; i < h2->GetNbinsX() + 1; ++i )
         for ( int j = 0; j < h2->GetNbinsY() + 1; ++j ) 
         {
            double xc = h2->GetXaxis()->GetBinCenter(i);
            double yc = h2->GetYaxis()->GetBinCenter(j);
            double content = rndm.Poisson( func->Eval( xc, yc ) );
            h2->Fill( xc, yc, content );
         }

      vector< vector<struct algoType> > listAlgos(5);
      listAlgos[0] = commonAlgos;
      listAlgos[1] = treeFail;
      listAlgos[2] = specialAlgos;
      listAlgos[3] = noGraphAlgos;
      listAlgos[4] = fumili;

      vector< vector<struct algoType> > listH2(1);
      listH2[0] = histGaus2D;

      if ( c0 ) delete c0;
      c0 = new TCanvas("c0-2D", "Histogram2D Variable");
      if ( __DRAW__ ) h2->Draw();
      globalStatus += status = testFitters(h2, func, listH2, l2DFunctions[h]);
      printf("%s\n", (status?"FAILED":"OK"));

      if ( c1 ) delete c1;
      c1 = new TCanvas("c1-2D", "Histogram2D");
      if ( __DRAW__ ) h1->Draw();
      globalStatus += status = testFitters(h1, func, listH2, l2DFunctions[h]);
      printf("%s\n", (status?"FAILED":"OK"));

      if ( g1 ) delete g1;
      g1 = new TGraph2D(h1);
      //Warning in <TROOT::Append>: Replacing existing TGraph2D: Graph2D (Potential memory leak).
      g1->SetName("TGraph 2D");
      g1->SetTitle("TGraph 2D - title");
      if ( c2 ) delete c2;
      c2 = new TCanvas("c2-2D","TGraph");
      if ( __DRAW__ ) g1->Draw("AB*");
      vector< vector<struct algoType> > listAlgosGraph(4);
      listAlgosGraph[0] = commonAlgos;
      listAlgosGraph[1] = treeFail;
      listAlgosGraph[2] = specialAlgos;
      listAlgosGraph[3] = fumili;
      globalStatus += status = testFitters(g1, func, listAlgosGraph, l2DFunctions[h]);
      printf("%s\n", (status?"FAILED":"OK"));

      
      ge1->SetName("TGraphErrors 2DGE");
      ge1->SetTitle("TGraphErrors 2DGE - title");
      if ( c3 ) delete c3;
      c3 = new TCanvas("c3-2DGE","TGraphError");
      if ( __DRAW__ ) ge1->Draw("AB*");

      vector< vector<struct algoType> > listAlgosGE(3);
      listAlgosGE[0] = commonAlgos;
      listAlgosGE[1] = treeFail;
      listAlgosGE[2] = specialAlgos;
      globalStatus += status = testFitters(ge1, func, listAlgosGE, l2DFunctions[h]);
      printf("%s\n", (status?"FAILED":"OK"));
   }

   if ( ! __DRAW__ )
   {
      delete func;
      delete h1;
      delete h2;
      delete g1;
      delete ge1;
      delete c0;
      delete c1;
      delete c2;
      delete c3;
   }

   return globalStatus;
}

class TreeWrapper {
public:

   const char* vars;
   const char* cuts;
   TTree *tree;

   void set(TTree* t, const char* v, const char* c)
   {
      tree = t;
      vars = v;
      cuts = c;
   }

   const char* GetName() const {
      return tree->GetName();
   }

   Int_t Fit(TF1* f1, Option_t* option = "")
   {
      return tree->UnbinnedFit(f1->GetName(), vars, cuts, option);
   }
};

int testUnBinedFit(int n = 10000)
{
   // Counts how many tests failed.
   int globalStatus = 0;
   // To control if an individual test failed
   int status = 0;

   double origPars[13] = {1,2,3,0.5, 0.5, 0, 3, 0, 4, 0, 5, 1, 10 };
//   double fitPars[13] =  {1,1,1,  1, 0.1, 0, 2, 0, 3, 0, 4, 0,  9 };

   TF2 * func = new TF2("gaus2Dn",gaus2dnormal,-10,-10,-10,10,5);
   func->SetParameters(origPars);

   TUnuranMultiContDist dist(func);
   TUnuran unr(&rndm); 

   // sampling with vnrou methods
   if (! unr.Init(dist,"vnrou")) {
         std::cerr << "error in init unuran " << std::endl; return -1;
   }

   TTree * tree =  new  TTree("tree","2 var gaus tree"); 
   double x,y,z,u,v,w;
   tree->Branch("x",&x,"x/D");
   tree->Branch("y",&y,"y/D");
   tree->Branch("z",&z,"z/D");
   tree->Branch("u",&u,"u/D");
   tree->Branch("v",&v,"v/D");
   tree->Branch("w",&w,"w/D");
   double xx[2];
   for (Int_t i=0;i<n;i++) {
      unr.SampleMulti(xx);
      x = xx[0];
      y = xx[1];
      z = rndm.Gaus(origPars[5],origPars[6]);
      u = rndm.Gaus(origPars[7],origPars[8]);
      v = rndm.Gaus(origPars[9],origPars[10]);
      w = rndm.Gaus(origPars[11],origPars[12]);
 
      tree->Fill();
      
   }

   delete func;

   vector< vector<struct algoType> > listAlgos(2);
   listAlgos[0] = commonAlgos;
   listAlgos[1] = treeFail;

   TreeWrapper tw;

   TF1 * f1 = new TF1(treeFunctions[0].name,treeFunctions[0].func,minX,maxY,treeFunctions[0].npars);   
   f1->SetParameters( &(treeFunctions[0].fitPars[0]) ); 
   f1->FixParameter(2,1);
   tw.set(tree, "x", "");
   globalStatus += status = testFitters(&tw, f1, listAlgos, treeFunctions[0]);
   printf("%s\n", (status?"FAILED":"OK"));

   TF2 * f2 = new TF2(treeFunctions[1].name,treeFunctions[1].func,minX,maxX,minY,maxY,treeFunctions[1].npars);   
   f2->SetParameters( &(treeFunctions[1].fitPars[0]) ); 
   tw.set(tree, "x:y", "");
   globalStatus += status = testFitters(&tw, f2, listAlgos, treeFunctions[1]);
   printf("%s\n", (status?"FAILED":"OK"));

   vector< vector<struct algoType> > listAlgosND(1);
   listAlgosND[0] = commonAlgos;

   TF1 * f4 = new TF1("gausND",gausNd,0,1,13);   
   f4->SetParameters(&(treeFunctions[2].fitPars[0]));
   tw.set(tree, "x:y:z:u:v:w", "");
   globalStatus += status = testFitters(&tw, f4, listAlgosND, treeFunctions[2]);
   printf("%s\n", (status?"FAILED":"OK"));

   delete tree;
   delete f1;
   delete f2;
   delete f4;

   return globalStatus;
}

void init_structures()
{
   commonAlgos.push_back( algoType( "Minuit",      "Migrad",      "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit",      "Minimize",    "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit",      "Scan",        "Q0", CompareResult(0)) );
   commonAlgos.push_back( algoType( "Minuit",      "Seek",        "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit2",     "Migrad",      "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit2",     "Simplex",     "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit2",     "Minimize",    "Q0", CompareResult())  );
   commonAlgos.push_back( algoType( "Minuit2",     "Scan",        "Q0", CompareResult(0)) );
   commonAlgos.push_back( algoType( "Minuit2",     "Fumili2",     "Q0", CompareResult())  );
#ifdef R__HAS_MATHMORE
   commonAlgos.push_back( algoType( "GSLMultiMin", "conjugatefr", "Q0", CompareResult()) );
   commonAlgos.push_back( algoType( "GSLMultiMin", "conjugatepr", "Q0", CompareResult()) );
   commonAlgos.push_back( algoType( "GSLMultiMin", "bfgs2",       "Q0", CompareResult()) );
   commonAlgos.push_back( algoType( "GSLMultiFit", "",            "Q0", CompareResult()) );
   commonAlgos.push_back( algoType( "GSLSimAn",    "",            "Q0", CompareResult()) );
#endif

   specialAlgos.push_back( algoType( "Minuit",      "Migrad",      "QE0", CompareResult()) );
   specialAlgos.push_back( algoType( "Minuit",      "Migrad",      "QW0", CompareResult()) );

   noGraphAlgos.push_back( algoType( "Minuit",      "Migrad",      "Q0I",  CompareResult()) );
   noGraphAlgos.push_back( algoType( "Minuit",      "Migrad",      "QL0",  CompareResult()) );
   noGraphAlgos.push_back( algoType( "Minuit",      "Migrad",      "QLI0", CompareResult()) );

   fumili.push_back( algoType( "Fumili",      "Fumili",      "Q0", CompareResult()) );

   treeFail.push_back( algoType( "Minuit",      "Simplex",     "Q0", CompareResult()) );

   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Minimize",    "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Scan",        "Q0",   CompareResult(0))         );
   histGaus2D.push_back( algoType( "Minuit",      "Seek",        "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit2",     "Migrad",      "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit2",     "Simplex",     "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit2",     "Minimize",    "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit2",     "Scan",        "Q0",   CompareResult(0))         );
   histGaus2D.push_back( algoType( "Minuit2",     "Fumili2",     "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Simplex",     "Q0",   CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "QE0",  CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "Q0I",  CompareResult(cmpPars,6)) );
   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "QL0",  CompareResult())          );
   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "QLI0", CompareResult())          );
   histGaus2D.push_back( algoType( "Minuit",      "Migrad",      "QW0",  CompareResult())          );
   histGaus2D.push_back( algoType( "Fumili",      "Fumili",      "Q0",   CompareResult(cmpPars,6)) );

   vector<ParLimit> emptyLimits(0);

   double gausOrig[] = {  0.,  3., 200.};
   double gausFit[] =  {0.5, 3.7,  250.};
   vector<ParLimit> gaus1DLimits;
   gaus1DLimits.push_back( ParLimit(1, 0, 5) );
   l1DFunctions.push_back( fitFunctions("GAUS",       gaus1DImpl, 3, gausOrig,  gausFit, gaus1DLimits) );
   double polyOrig[] = { 2, 3, 4, 200};
   double polyFit[] = { 6.4, -2.3, 15.4, 210.5};
   l1DFunctions.push_back( fitFunctions("Polynomial", poly1DImpl, 4, polyOrig, polyFit, emptyLimits) );

   double gaus2DOrig[] = { 500., +.5, 1.5, -.5, 2.0 };
   double gaus2DFit[] = { 510., .0, 1.8, -1.0, 1.6};
   l2DFunctions.push_back( fitFunctions("gaus2D", gaus2DImpl, 5, gaus2DOrig, gaus2DFit, emptyLimits) );

   double gausnOrig[3] = {1,2,1};
   double treeOrig[13] = {1,2,3,0.5, 0.5, 0, 3, 0, 4, 0, 5, 1, 10 };
   double treeFit[13]  = {1,1,1,  1, 0.1, 0, 2, 0, 3, 0, 4, 0,  9 };
   treeFunctions.push_back( fitFunctions("gausn", gausNormal, 3, gausnOrig, treeFit, emptyLimits ));
   treeFunctions.push_back( fitFunctions("gaus2Dn", gaus2dnormal, 5, treeOrig, treeFit, emptyLimits));
   treeFunctions.push_back( fitFunctions("gausND", gausNd, 13, treeOrig, treeFit, emptyLimits));
}

int stressFit() 
{ 
   rndm.SetSeed(10);

   init_structures();

   int iret = 0; 

   iret += test1DObjects();
   iret += test2DObjects();
//    iret += testUnBinedFit();

   return iret; 
}
   
int main(int argc, char** argv)
{
   TApplication* theApp = 0;

   if ( __DRAW__ )
      theApp = new TApplication("App",&argc,argv);

   int ret = stressFit();

   if ( __DRAW__ ) {
      theApp->Run();
      delete theApp;
      theApp = 0;
   }

   return ret;
}
