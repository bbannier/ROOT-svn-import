// @(#)root/test:$Id: stressVector.cxx 19826 2007-09-19 19:56:11Z rdm $
// Author: Lorenzo Moneta   06/2005 
///////////////////////////////////////////////////////////////////////////////////
//
//  MathCore Benchmark test suite
//  ==============================
//
//  This program performs tests : 
//     - mathematical functions in particular the statistical functions by estimating 
//         pdf, cdf and quantiles. 
//     - cdf are estimated directly and compared with integral calulated ones 

#ifndef __CINT__

#include "Math/DistFunc.h"
#include "Math/IParamFunction.h"
#include "Math/Integrator.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include "TBenchmark.h"
#include "TROOT.h"
#include "TRandom3.h"
#include "TSystem.h"
#include "TTree.h"
#include "TFile.h"


#include "Math/Vector2D.h"
#include "Math/Vector3D.h"
#include "Math/Vector4D.h"
#include "Math/VectorUtil.h"

using namespace ROOT::Math; 

#endif


bool debug = true;  // print out reason of test failures
bool debugTime = false; // print out separate timings for vectors 

void PrintTest(std::string name) { 
   std::cout << std::left << std::setw(40) << name; 
}

void PrintStatus(int iret) { 
   if (iret == 0) 
      std::cout <<"\t\t................ OK" << std::endl;
   else  
      std::cout <<"\t\t............ FAILED " << std::endl;
}


int compare( std::string name, double v1, double v2, double scale = 2.0) {
  //  ntest = ntest + 1; 

   //std::cout << std::setw(50) << std::left << name << ":\t";   
   
  // numerical double limit for epsilon 
   double eps = scale* std::numeric_limits<double>::epsilon();
   int iret = 0; 
   double delta = v2 - v1;
   double d = 0;
   if (delta < 0 ) delta = - delta; 
   if (v1 == 0 || v2 == 0) { 
      if  (delta > eps ) { 
         iret = 1; 
      }
   }
   // skip case v1 or v2 is infinity
   else { 
      d = v1; 

      if ( v1 < 0) d = -d; 
      // add also case when delta is small by default
      if ( delta/d  > eps && delta > eps ) 
         iret =  1; 
   }

   if (iret) { 
      if (debug) { 
         int pr = std::cout.precision (18);
         std::cout << "\nDiscrepancy in " << name.c_str() << "() :\n  " << v1 << " != " << v2 << " discr = " << int(delta/d/eps) 
                   << "   (Allowed discrepancy is " << eps  << ")\n\n";
         std::cout.precision (pr);
      //nfail = nfail + 1;
      }
   }
   //else  
      //  std::cout <<".";
   
   return iret; 
}

#ifndef __CINT__


// trait class  for distinguishing the number of parameters for the various functions
template<class Func, unsigned int NPAR>
struct Evaluator { 
   static double F(Func f,  double x, const double * ) { 
      return f(x);
   }
};
template<class Func>
struct Evaluator<Func, 1> { 
   static double F(Func f,  double x, const double * p) { 
      return f(x,p[0]);
   }
};
template<class Func>
struct Evaluator<Func, 2> { 
   static double F(Func f,  double x, const double * p) { 
      return f(x,p[0],p[1]);
   }
};
template<class Func>
struct Evaluator<Func, 3> { 
   static double F(Func f,  double x, const double * p) { 
      return f(x,p[0],p[1],p[2]);
   }
};


// statistical function class 
// template on the number of parameters
template<class Func, class FuncQ, int NPAR, int NPARQ=NPAR-1> 
class StatFunction : public ROOT::Math::IParamFunction { 

public: 

   StatFunction(Func pdf, Func cdf, FuncQ quant) : fPdf(pdf), fCdf(cdf), fQuant(quant) 
   {
      fScale1 = 1.0E6; //scale for cdf test (integral)
      fScale2 = 10;  //scale for quantile test
      for(int i = 0; i< NPAR; ++i) fParams[i]=0;
   } 


   unsigned int NPar() const { return NPAR; } 
   const double * Parameters() const { return fParams; }
   ROOT::Math::IGenFunction * Clone() const { return new StatFunction(fPdf,fCdf,fQuant); }
   void SetParameters(const double * p) { std::copy(p,p+NPAR,fParams); }
   void SetParameters(double p0) { *fParams = p0; }
   void SetParameters(double p0, double p1) { *fParams = p0; *(fParams+1) = p1; }
   void SetParameters(double p0, double p1, double p2) { *fParams = p0; *(fParams+1) = p1; *(fParams+2) = p2; }

   double Cdf(double x) const { 
      return Evaluator<Func,NPAR>::F(fCdf,x, fParams); 
   }
   double Quantile(double x) const { 
      return Evaluator<FuncQ,NPARQ>::F(fQuant,x, fParams); 
   }

   // test cumulative function
   int Test(double x1, double x2, double xl = 1, double xu = 0, bool cumul = false); 
   
   void ScaleTol1(double s) { fScale1 *= s; }  
   void ScaleTol2(double s) { fScale2 *= s; }

private: 


   double DoEval(double x) const { 
      return Evaluator<Func,NPAR>::F(fPdf,x, fParams); 
   }

   Func fPdf; 
   Func fCdf;
   FuncQ fQuant; 
   double fParams[NPAR];
   double fScale1;
   double fScale2;
   int NFuncTest; 
};

int NFuncTest = 0; 

// test cdf at value f 
template<class F1, class F2, int N1, int N2> 
int StatFunction<F1,F2,N1,N2>::Test(double xmin, double xmax, double xlow, double xup, bool c) {

   int iret = 0; 

   // scan all values from xmin to xmax
   double dx = (xmax-xmin)/NFuncTest; 
   for (int i = 0; i < NFuncTest; ++i) { 
      double v1 = xmin + dx*i;  // value used  for testing
      double q1 = Cdf(v1);
      //std::cout << "v1 " << v1 << " pdf " << (*this)(v1) << " cdf " << q1 << " quantile " << Quantile(q1) << std::endl;  
      // calculate integral of pdf
      Integrator ig(IntegrationOneDim::ADAPTIVESINGULAR, 1.E-12,1.E-12,100000);
      ig.SetFunction(*this);
      double q2 = 0; 
      if (!c) { 
         // lower intergal (cdf) 
         if (xlow >= xup || xlow > xmin) 
            q2 = ig.IntegralLow(v1); 
         else 
            q2 = ig.Integral(xlow,v1); 

         // use a larger scale (integral error is 10-9)
         iret |= compare("test _cdf", q1, q2, fScale1 );
         // test the quantile 
         double v2 = Quantile(q1); 
         iret |= compare("test _quantile", v1, v2, fScale2 );
      }
      else { 
         // upper integral (cdf_c)
         if (xlow >= xup || xup < xmax) 
            q2 = ig.IntegralUp(v1); 
         else 
            q2 = ig.Integral(v1,xup);
 
         iret |= compare("test _cdf_c", q1, q2, fScale1);
         double v2 = Quantile(q1); 
         iret |= compare("test _quantile_c", v1, v2, fScale2 );
      }
      if (iret)  { 
         std::cout << "Failed test for x = " << v1 << " p = "; 
         for (int j = 0; j < N1; ++j) std::cout << fParams[j] << "\t"; 
         std::cout << std::endl;
         break;
      } 
   }
   if (c || iret != 0) PrintStatus(iret);
   return iret; 
}

// typedef defining the functions
typedef double ( * F0) ( double); 
typedef double ( * F1) ( double, double); 
typedef double ( * F2) ( double, double, double); 
typedef double ( * F3) ( double, double, double, double); 

typedef StatFunction<F2,F2,2,2> Dist_beta; 
typedef StatFunction<F2,F1,2> Dist_breitwigner; 
typedef StatFunction<F2,F1,2> Dist_chisquared; 
typedef StatFunction<F3,F2,3> Dist_fdistribution; 
typedef StatFunction<F3,F2,3> Dist_gamma; 
typedef StatFunction<F2,F1,2> Dist_gaussian; 
typedef StatFunction<F3,F2,3> Dist_lognormal; 
typedef StatFunction<F2,F1,2> Dist_tdistribution; 
 

#define CREATE_DIST(name) Dist_ ##name  dist( name ## _pdf, name ## _cdf, name ##_quantile );
#define CREATE_DIST_C(name) Dist_ ##name  distc( name ## _pdf, name ## _cdf_c, name ##_quantile_c );


template<class Distribution> 
int TestDist(Distribution & d, double x1, double x2) { 
   int ir = 0; 
   ir |= d.Test(x1,x2); 
   return ir; 
}

int testStatFunctions(int n) { 
   // test statistical functions 
   
   int iret = 0; 
   NFuncTest = n; 
      
   { 
      PrintTest("Beta distribution"); 
      CREATE_DIST(beta);
      dist.SetParameters( 2, 2);
      iret |= dist.Test(0.01,0.99,0.,1.);
      CREATE_DIST_C(beta);
      distc.SetParameters( 2, 2);
      iret |= distc.Test(0.01,0.99,0.,1.,true);
   }

   {
      PrintTest("Gamma distribution"); 
      CREATE_DIST(gamma);
      dist.SetParameters( 2, 1);
      iret |= dist.Test(0.05,5, 0.,1.);
      CREATE_DIST_C(gamma);
      distc.SetParameters( 2, 1);
      iret |= distc.Test(0.05,5, 0.,1.,true);
   }

   {
      PrintTest("Chisquare distribution"); 
      CREATE_DIST(chisquared);
      dist.SetParameters( 10, 0);
      dist.ScaleTol2(10);
      iret |= dist.Test(0.05,30, 0.);
      CREATE_DIST_C(chisquared);
      distc.SetParameters( 10, 0);
      distc.ScaleTol2(10000000);  // t.b.c.
      iret |= distc.Test(0.05,30, 0.,1.,true);
   }
   {
      PrintTest("Normal distribution "); 
      CREATE_DIST(gaussian);
      dist.SetParameters( 1, 0);
      dist.ScaleTol2(100);
      iret |= dist.Test(-4,4);
      CREATE_DIST_C(gaussian);
      distc.SetParameters( 1, 0);
      distc.ScaleTol2(100);
      iret |= distc.Test(-4,4,1,0,true);
   }
   {
      PrintTest("BreitWigner distribution "); 
      CREATE_DIST(breitwigner);
      dist.SetParameters( 1);
      dist.ScaleTol2(10);
      iret |= dist.Test(-5,5);
      CREATE_DIST_C(breitwigner);
      distc.SetParameters( 1);
      distc.ScaleTol2(10);
      iret |= distc.Test(-5,5,1,0,true);
   }
   {
      PrintTest("F    distribution "); 
      CREATE_DIST(fdistribution);
      dist.SetParameters( 5, 4);
      dist.ScaleTol1(1000000);
      dist.ScaleTol2(10);
      // if enlarge scale test fails
      iret |= dist.Test(0.05,5,0);
      CREATE_DIST_C(fdistribution);
      distc.SetParameters( 5, 4);
//       dist.ScaleTol1(1000000);
      distc.ScaleTol2(10);
      // if enlarge scale test fails
      iret |= distc.Test(0.05,5,0,1,true);
   }
   {
      PrintTest("t    distribution "); 
      CREATE_DIST(tdistribution);
      dist.SetParameters( 10 );
//       dist.ScaleTol1(1000);
       dist.ScaleTol2(5000);
      iret |= dist.Test(-10,10);
      CREATE_DIST_C(tdistribution);
      distc.SetParameters( 10 );
      distc.ScaleTol2(10000);  // t.b.c.
      iret |= distc.Test(-10,10,1,0,true);
   }
   {
      PrintTest("lognormal distribution"); 
      CREATE_DIST(lognormal);
      dist.SetParameters(1,1 );
      dist.ScaleTol1(1000);
      iret |= dist.Test(0.01,5,0);
      CREATE_DIST_C(lognormal);
      distc.SetParameters(1,1 );
      distc.ScaleTol2(1000000); // t.b.c.
      iret |= distc.Test(0.01,5,0,1,true);
   }


   return iret; 
}

//*******************************************************************************************************************
// GenVector tests
//*******************************************************************************************************************

// trait for getting vector name 

template<class V> 
struct VecType { 
   static std::string name() { return "Vector";}
}; 
template<>
struct VecType<XYVector> {
   static std::string name() { return "XYVector";}
}; 
template<>
struct VecType<Polar2DVector> {
   static std::string name() { return "Polar2DVector";}
}; 
template<>
struct VecType<XYZVector> {
   static std::string name() { return "XYZVector";}
}; 
template<>
struct VecType<Polar3DVector> {
   static std::string name() { return "Polar3DVector";}
}; 
template<>
struct VecType<RhoEtaPhiVector> {
   static std::string name() { return "RhoEtaPhiVector";}
}; 
template<>
struct VecType<PxPyPzEVector> {
   static std::string name() { return "PxPyPzEVector";}
}; 
template<>
struct VecType<PtEtaPhiEVector> {
   static std::string name() { return "PtEtaPhiEVector";}
}; 
template<>
struct VecType<PtEtaPhiMVector> {
   static std::string name() { return "PtEtaPhiMVector";}
}; 
template<>
struct VecType<PxPyPzMVector> {
   static std::string name() { return "PxPyPzMVector";}
}; 

// generic (2 dimension)
template<class V, int Dim>  
struct VecOp { 

   template<class It>
   static V  Create(It &x, It &y, It & , It&  ) {       return V(*x++,*y++);  }
   template<class It>
   static void Set(V & v, It &x, It &y, It &, It&) { v.SetXY(*x++,*y++);  }

   static double Add(const V & v) { return v.X() + v.Y(); }
   static double Delta(const V & v1, const V & v2) { double d = ROOT::Math::VectorUtil::DeltaPhi(v1,v2);  return d*d; } // is v2-v1


};
// specialized for 3D 
template<class V>  
struct VecOp<V,3> { 

   template<class It>
   static V  Create(It &x, It& y, It& z , It&  ) { return V(*x++,*y++,*z++); }
   template<class It>
   static void Set(V & v, It & x, It &y, It &z, It&) { v.SetXYZ(*x++,*y++,*z++); }
   static V  Create(double x, double y, double z , double  ) { return  V(x,y,z); }
   static void Set(V & v, double x, double y, double z, double) { v.SetXYZ(x,y,z); }
   static double Add(const V & v) { return v.X() + v.Y() + v.Z(); }
   static double Delta(const V & v1, const V & v2) { return ROOT::Math::VectorUtil::DeltaR2(v1,v2); }
         

};
// specialized for 4D 
template<class V>  
struct VecOp<V,4> { 

   template<class It>
   static V  Create(It &x, It &y, It &z , It &t ) { return V(*x++,*y++,*z++,*t++);}
   template<class It>
   static void Set(V & v, It & x, It &y, It &z, It &t) { v.SetXYZT(*x++,*y++,*z++,*t++);  }

   static double Add(const V & v) { return v.X() + v.Y() + v.Z() + v.E(); }
   static double Delta(const V & v1, const V & v2) { 
      return ROOT::Math::VectorUtil::DeltaR2(v1,v2) + ROOT::Math::VectorUtil::InvariantMass(v1,v2);  }

};

// internal structure to measure the time

TStopwatch gTimer; 
double gTotTime; 

struct Timer { 
   Timer()  {     
      gTimer.Start();
   }
   ~Timer() { 
      gTimer.Stop();
      gTotTime += Time();
      if (debugTime) printTime();
   }

   void printTime( std::string s = "") { 
      int pr = std::cout.precision(8);
      std::cout << s << "\t" << " time = " << Time() << "\t(sec)\t" 
         //    << time.CpuTime() 
                << std::endl;
      std::cout.precision(pr);
   }

   double Time()  { return gTimer.RealTime(); }  // use real time

   TStopwatch gTimer; 
   double gTotTime; 
};



template<int Dim> 
class VectorTest { 

private: 

// global data variables 
   std::vector<double> dataX; 
   std::vector<double> dataY;  
   std::vector<double> dataZ;  
   std::vector<double> dataE;  

   int nGen;
   int n2Loop ;

   double fSum; // total sum of x,y,z,t (for testing first addition)

public: 
  
   VectorTest(int n1, int n2=0) : 
      nGen(n1),
      n2Loop(n2)
   {
      gTotTime  = 0; 
      genData();
   }
    

  

   double TotalTime() const { return gTotTime; }  // use real time

   double Sum() const { return fSum; }

   int check(std::string name, double s1, double s2, double scale=1) {
      int iret = 0; 
      PrintTest(name);
      iret |= compare(name,s1,s2,scale);
      PrintStatus(iret);
      return iret; 
   }

   void print(std::string name) { 
      PrintTest(name);
      std::cout <<"\t\t..............\n";
   }


   void genData() { 

      // generate for all 4 d data 
      TRandom3 r(111); // use a fixed seed to be able to reproduce tests
      fSum = 0; 
      for (int i = 0; i < nGen ; ++ i) { 

         // generate a 4D vector and stores only the interested dimensions
         double phi = r.Rndm()*3.1415926535897931; 
         double eta = r.Uniform(-5.,5.); 
         double pt   = r.Exp(10.);
         double m = r.Uniform(0,10.); 
         if ( i%50 == 0 ) 
            m = r.BreitWigner(1.,0.01); 
         double E = sqrt( m*m + pt*pt*std::cosh(eta)*std::cosh(eta) );

         // fill vectors           
         PtEtaPhiEVector q( pt, eta, phi, E); 
         dataX.push_back( q.x() ); 
         dataY.push_back( q.y() ); 
         fSum += q.x() + q.y(); 
         if (Dim >= 3) { 
            dataZ.push_back( q.z() ); 
            fSum += q.z(); 
         }
         if (Dim >=4 ) {
            dataE.push_back( q.t() ); 
            fSum += q.t();
         } 
      }
      assert( int(dataX.size()) == nGen);
      assert( int(dataY.size()) == nGen);
      if (Dim >= 3) assert( int(dataZ.size()) == nGen);
      if (Dim >=4 ) assert( int(dataE.size()) == nGen);
// // //       dataZ.resize(nGen);
// // //       dataE.resize(nGen);
         
   }

   typedef std::vector<double>::const_iterator DataIt; 

   // test methods
   template <class V> 
   void testCreate( std::vector<V > & dataV) {     
      Timer tim;
      DataIt x = dataX.begin(); 
      DataIt y = dataY.begin(); 
      DataIt z = dataZ.begin(); 
      DataIt t = dataE.begin(); 
      while (x != dataX.end() ) { 
         dataV.push_back(VecOp<V,Dim>::Create(x,y,z,t) );
         assert(int(dataV.size()) <= nGen); 
      }
            
   }

   template <class V> 
   void testCreateAndSet( std::vector<V > & dataV) { 
      Timer tim;
      DataIt x = dataX.begin(); 
      DataIt y = dataY.begin(); 
      DataIt z = dataZ.begin(); 
      DataIt t = dataE.begin(); 
      while (x != dataX.end() ) { 
         V  v; 
         VecOp<V,Dim>::Set( v, x,y,z,t); 
         dataV.push_back(v); 
         assert(int(dataV.size()) <= nGen); 
      }
   }
    


   template <class V>
   double testAddition( const std::vector<V > & dataV) { 
      V v0;
      Timer t;
      for (int i = 0; i < nGen; ++i) { 
         v0 += dataV[i]; 
      }
      return VecOp<V,Dim>::Add(v0); 
   }  


   template <class V>
   double testOperations( const std::vector<V > & dataV) { 

      double tot = 0;
      Timer t;
      for (int i = 0; i < nGen-1; ++i) { 
         const V  & v1 = dataV[i]; 
         const V  & v2 = dataV[i+1]; 
         double a = v1.R();
         double b = v2.mag2(); // mag2 is defined for all dimensions;
         double c = 1./v1.Dot(v2);
         V v3 = c * ( v1/a + v2/b );
         tot += VecOp<V,Dim>::Add(v3);
      }
      return tot; 
   }  

   // mantain loop in gen otherwise is proportional to N**@
   template <class V>
   double testDelta( const std::vector<V > & dataV) { 
      double tot = 0;
      Timer t;
      for (int i = 0; i < nGen-1; ++i) { 
         const V  & v1 = dataV[i]; 
         const V  & v2 = dataV[i+1]; 
         tot += VecOp<V,Dim>::Delta(v1,v2);
      }
      return tot; 
   }  


//    template <class V>
//    double testDotProduct( const std::vector<V *> & dataV) { 
//       //unsigned int n = std::min(n2Loop, dataV.size() );
//       double tot = 0;
//       V v0 = *(dataV[0]);
//       Timer t; 
//       for (unsigned int i = 0; i < nGen-1; ++i) { 
//          V  & v1 = *(dataV[i]); 
//          tot += v0.Dot(v1);
//       }
//       return tot; 
//    }  




   template <class V1, class V2> 
   void testConversion( std::vector<V1 > & dataV1, std::vector<V2 > & dataV2) { 
    
      Timer t;
      for (int i = 0; i < nGen; ++i) { 
         dataV2.push_back( V2( dataV1[i] ) ); 
      }
   }



   // rotation 
   template <class V, class R> 
   double testRotation( std::vector<V > & dataV ) { 

      double sum = 0;
      double rotAngle = 1; 
      Timer t;
      for (unsigned int i = 0; i < nGen; ++i) { 
         V  & v1 = dataV[i];
         V v2 = v1;
         v2.Rotate(rotAngle);
         sum += VecOp<V,Dim>::Add(v2);
      }
      return sum;
   }

   template<class V> 
   double testWrite(const std::vector<V> & dataV, bool compress = false) {

      
      TFile file(VecType<V>::name().c_str(),"RECREATE","",compress);

      // create tree
      std::string tree_name="Tree with" + VecType<V>::name(); 

      TTree tree("t1",tree_name.c_str());

      V *v1 = new V();
      //std::cout << "typeID written : " << typeid(*v1).name() << std::endl;

      tree.Branch("Vector branch",VecType<V>::name().c_str(),&v1);

      Timer timer;
      for (int i = 0; i < nGen; ++i) { 
         *v1 = dataV[i];  
         tree.Fill();
      }

      return file.Write();
   }

   template<class V> 
   int testRead(std::vector<V> & dataV) { 
      
      dataV.clear(); 
      dataV.reserve(nGen); 
      
      
      std::string fname = VecType<V>::name() + ".root";
      
      TFile f1(fname.c_str());
      if (f1.IsZombie() ) { 
         std::cout << " Error opening file " << fname << std::endl; 
         return -1; 
      }
      

      // create tree
      TTree *tree = (TTree*)f1.Get("tree");
      
      V *v1 = 0;
      
      //std::cout << "reading typeID  : " << typeid(*v1).name() << std::endl;
      
      tree->SetBranchAddress("Vector branch",&v1);
      
      Timer timer;
      int n = (int) tree->GetEntries();
      if (n != nGen) { 
         std::cout << "wrong tree entries from file" << fname << std::endl;
         return -1; 
      }
      
      for (int i = 0; i < n; ++i) { 
         tree->GetEntry(i);
         dataV.push_back(*v1); 
      }
      
      return 0; 
   }


};



template<class V1, class V2, int Dim> 
int testVector(int ngen, bool testio=false) { 
   
   int iret = 0;

   
   VectorTest<Dim> a(ngen); 
   

   std::vector<V1> v1; 
   std::vector<V2> v2;  
   v1.reserve(ngen); 
   v2.reserve(ngen); 

   double sxy0, sxy1, sxy2 = 0; 
   double scale = 1; 

   a.testCreate(v1);             iret |= a.check(VecType<V1>::name()+" creation",v1.size(),ngen);
   sxy1 = a.testAddition(v1);    iret |= a.check(VecType<V1>::name()+" addition",sxy1,a.Sum(),Dim*4);
   v1.clear();
   assert(v1.size() == 0);
   a.testCreateAndSet(v1);       iret |= a.check(VecType<V1>::name()+" creation",v1.size(),ngen);
   sxy2 = a.testAddition(v1);    iret |= a.check(VecType<V1>::name()+" setting",sxy2,sxy1);
   sxy0 = sxy1; 

   a.testConversion(v1,v2);      iret |= a.check(VecType<V1>::name()+" -> " + VecType<V2>::name(),v1.size(),v2.size() );
   sxy2 = a.testAddition(v1);    iret |= a.check("Vector conversion",sxy2,sxy1);

   sxy1 = a.testOperations(v1);  a.print(VecType<V1>::name()+" operations");
   scale = Dim*20; 
   if (Dim==4) scale *= 10000000; // for problem with PtEtaPhiE
   sxy2 = a.testOperations(v2);  iret |= a.check(VecType<V2>::name()+" operations",sxy2,sxy1,scale);

   sxy1 = a.testDelta(v1);      a.print(VecType<V1>::name()+" delta values");
   scale = Dim*16; 
   if (Dim==4) scale *= 100; // for problem with PtEtaPhiE
   sxy2 = a.testDelta(v2);      iret |= a.check(VecType<V2>::name()+" delta values",sxy2,sxy1,scale);
 
   if (!testio) return iret; 

   double fsize = 0; 
   fsize = a.testWrite(v1);  iret |= a.check(VecType<V1>::name()+" write",fsize>100,1);
   iret |= a.testRead(v1);   iret |= a.check(VecType<V1>::name()+" read",iret,0);
   sxy1 = a.testAddition(v1);       iret |= a.check(VecType<V1>::name()+" after read",sxy1,sxy0);

   return iret; 

}



#endif

int testGenVectors(int ngen,bool io) { 

   int iret = 0; 
   std::cout <<"******************************************************************************\n";
   std::cout << "\tTest of Physics Vector (GenVector package)\n";
   std::cout <<"******************************************************************************\n";
   iret |= testVector<XYVector, Polar2DVector, 2>(ngen); 
   iret |= testVector<XYZVector, Polar3DVector, 3>(ngen); 
   iret |= testVector<XYZVector, RhoEtaPhiVector, 3>(ngen); 
   iret |= testVector<XYZVector, RhoZPhiVector, 3>(ngen); 
   iret |= testVector<XYZTVector, PtEtaPhiEVector, 4>(ngen,io); 
   iret |= testVector<XYZTVector, PtEtaPhiMVector, 4>(ngen); 
   iret |= testVector<XYZTVector, PxPyPzMVector, 4>(ngen); 

   return iret; 
}



int stressMathCore() { 

   int iret = 0; 

#ifdef __CINT__
   std::cout << "Test must be run in compile mode - please use ACLIC !!" << std::endl; 
   return 0; 
#endif
//    iret |= gSystem->Load("libMathCore");
//    iret |= gSystem->Load("libMathMore");
//    if (iret !=0) return iret; 


   TBenchmark bm; 
   bm.Start("stressMathCore");


   iret |= testStatFunctions(100);

   bool io = false; 
   iret |= testGenVectors(1000,io); 

   bm.Stop("stressMathCore");
   std::cout <<"******************************************************************************\n";
   bm.Print("stressMathCore");
   const double reftime = 3.67; // ref time on  macbook pro (intel core duo 2.2 GHz)
   double rootmarks = 800 * reftime / bm.GetCpuTime("stressMathCore");
   std::cout << " ROOTMARKS = " << rootmarks << " ROOT version: " << gROOT->GetVersion() << "\t" 
             << gROOT->GetSvnBranch() << "@" << gROOT->GetSvnRevision() << std::endl;
   std::cout <<"*******************************************************************************\n";
 
   return iret; 
}

//int main(int argc,const char *argv[]) { 
int main() { 
   return stressMathCore();
}
