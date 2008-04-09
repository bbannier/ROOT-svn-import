#include <iostream>
#include <algorithm>

#include <TRandom2.h>
#include <TStopwatch.h>
#include <TMath.h>

using std::cout;
using std::endl;

const unsigned int NUMTEST = 500;

// #define DEBUG

template <typename T> T randD() {
   static TRandom2 r( time( 0 ) );
   return (T) r.Uniform(-500,500);
}

template <typename T> double stressVector(unsigned int size, const char* type)
{
   cout << "Generating random vector of '" 
        << type << "'..." << endl;

   double totalTime = 0;
   double totalUnitTime = 0;

   T *vector = new T[size];
   std::generate(vector, &vector[size], randD<T>);

#ifdef DEBUG
   for ( unsigned int i = 0; i < size; ++i )
      cout << vector[i] << " " << endl;
#endif

   TStopwatch w;

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST; ++i )
      TMath::MinElement(size, vector);
   w.Stop();
   cout << "MinMaxElement() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/NUMTEST << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST; ++i )
      TMath::LocMin(size, vector);
   w.Stop();
   cout << "LocMin/Max() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/NUMTEST << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST; ++i )
      TMath::Mean(size, vector);
   w.Stop();
   cout << "Mean() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/NUMTEST << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST/2; ++i )
      TMath::Median(size, vector);
   w.Stop();
   cout << "Median() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/(NUMTEST/2) << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST; ++i )
      TMath::RMS(size, vector);
   w.Stop();
   cout << "RMS() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/NUMTEST << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST/4; ++i )
      TMath::GeomMean(size, vector);
   w.Stop();
   cout << "GeomMean() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/(NUMTEST/4) << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   Int_t index[size];
   w.Start( kTRUE );
   for ( unsigned int i = 0; i < NUMTEST/5; ++i )
      TMath::Sort(size, vector, index, kFALSE);
   w.Stop();
   cout << "Sort() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/(NUMTEST/5) << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   std::sort(vector, vector + size);
#ifdef DEBUG
   for ( unsigned int i = 0; i < size; ++i )
      cout << vector[i] << " " << endl;
#endif
   w.Start( kTRUE );
   for ( unsigned int i = 0; i < 100*NUMTEST; ++i )
      TMath::BinarySearch(size, vector, vector[ i % size ]);
   w.Stop();
   cout << "BinarySearch() Total Time: " << w.CpuTime() 
        << " Time: " << w.CpuTime()/(100*NUMTEST) << endl;
   totalUnitTime += w.CpuTime()/NUMTEST;
   totalTime += w.CpuTime();

   cout << "\nTotal Time: "      << totalTime << "\n"
        << "Total Unit Time: " << totalUnitTime << "\n" << endl;

   delete [] vector;

   return totalUnitTime;
}

void stressTMath(unsigned int size, char * type) 
{
   double totalTime = 0;
   
   cout << "Stress Test Start..." << endl;

   if ( strcmp(type, "Short_t") == 0 )
      totalTime += stressVector<Short_t>(size, type);
   else if ( strcmp(type, "Int_t") == 0 )
      totalTime += stressVector<Int_t>(size, type);
   else if ( strcmp(type, "Float_t") == 0 )
      totalTime += stressVector<Float_t>(size, type);
   else if ( strcmp(type, "Long_t") == 0 )
      totalTime += stressVector<Long_t>(size, type);
   else if ( strcmp(type, "Long64_t") == 0 )
      totalTime += stressVector<Long64_t>(size, type);
   else
      totalTime += stressVector<Double_t>(size, "Double_t");
   
   //cout << "Total Test Time: " << totalTime << "\n" << endl;

   cout << "End of Stress Test..." << endl;

   return;
}


int main(int argc, char* argv[])
{
   // Default size and data type
   unsigned int size = 100000;
   char* type = "Double_t";
      
   if ( argc == 1 )
   {
      cout << "Usage: " << argv[0]
           << " [TYPE OF ARRAY] [SIZE OF ARRAY]\n\n"
           << "where [TYPE OF ARRAY] is one of the following:\n"
           << "\t\"Short_t\"\n"
           << "\t\"Int_t\"\n"
           << "\t\"Float_t\"\n"
           << "\t\"Long_t\"\n"
           << "\t\"Long64_t\"\n"
           << "\t \"Double_t\"\n"
           << endl;
   }

   if ( argc > 2 )
      size = (unsigned int) atoi(argv[2]);

   if ( argc > 1 )
      type = argv[1];

   stressTMath(size, type);

   return 0;
}
