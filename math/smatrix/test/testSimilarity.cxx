#ifndef DIM_L
#define DIM_L 5
#endif

#ifdef HAVE_VC
#include "Vc_smatrix.h"
#endif

typedef double T;
//typedef Vc::double_v T;

T zero_T = 0.0;
T one_T = 1.0;

#include "Math/SVector.h"
#include "Math/SMatrix.h"

using namespace ROOT::Math;

int main(int argc, char **argv) {
	int runs = 100000;
	if(argc == 2)
		runs = atoi(argv[1]);
	T outputAcc = static_cast<T>(0.0);


	// Setup of data structures
	// Force compiler to not remove any code
	volatile int zero = 0;
	volatile int one = 1;
	volatile int DIM_L_minus_1 = DIM_L-1;
// 	volatile T zero_T = 0.0;
// 	volatile T one_T = 1.0;

	//typedef SMatrix<T, DIM_L, DIM_L, MatRepSym<T, DIM_L> >  MatSD;
	//typedef SMatrix<T, 5, DIM_L>  Mat5D;
	//typedef SMatrix<T, DIM_L, 5>  MatD5;
	typedef SMatrix<T, DIM_L, DIM_L>  Mat55;
	//typedef SMatrix<double, 5>  MatS5;
	typedef SMatrix<T, DIM_L, DIM_L, MatRepSym<T, DIM_L> > MatS5;
	//typedef SVector<T, DIM_L> VecD;
	
	MatS5 C;
	T endVal = 0;
	Mat55 M = SMatrixIdentity();
	for(int i = 0; i < 100*runs; ++i)
	{
		C(DIM_L_minus_1,DIM_L_minus_1) = zero_T; // Force compiler to not remove any code
		M(DIM_L_minus_1,DIM_L_minus_1) = zero_T;   // Force compiler to not remove any code
		MatS5 temp = ROOT::Math::Similarity(M, C);
		endVal += temp(zero,zero); 
	}
	outputAcc += endVal;

	// The end
        std::cout << "result  for size " << DIM_L << " = " << outputAcc << std::endl;
        return 0;
//	return static_cast<int>(outputAcc);
}
