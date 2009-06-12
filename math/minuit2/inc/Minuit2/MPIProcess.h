// @(#)root/minuit2:$Id$
// Author: A. Lazzaro 2009
/***************************************************************************
 * Package: Minuit2                                                        *
 *    File: $Id$         *
 *  Author: Alfio Lazzaro, alfio.lazzaro@mi.infn.it                        *
 *                                                                         *
 * Copyright: (C) 2008 by Universita' and INFN, Milan                      *
 ***************************************************************************/

#ifndef MPIPROCESS
#define MPIPROCESS

// disable MPI calls
//#define MPIPROC

#include "Minuit2/MnMatrix.h"

#ifdef MPIPROC
#include "mpi.h"
#endif


namespace ROOT {


   class MPITerminate {
   public:
      ~MPITerminate() { 
#ifdef MPIPROC
         if (MPI::Is_initialized() && !(MPI::Is_finalized())) {
            std::cout << "MPIProcess:: End MPI on #"
                      << MPI::COMM_WORLD.Get_rank() << " processor"
                      << std::endl;

            MPI::Finalize();
         }
#endif
      }

   };


   class MPIProcess {
   public:
      MPIProcess(unsigned int nelements, unsigned int indexComm);
      ~MPIProcess();

      inline unsigned int NumElements4JobIn() const { return _numElements4JobIn; }
      inline unsigned int NumElements4JobOut() const { return _numElements4JobOut; }

      inline unsigned int NumElements4Job(unsigned int rank) const
      { return NumElements4JobIn()+((rank<NumElements4JobOut()) ? 1 : 0); }

      inline unsigned int StartElementIndex() const 
      { return ((_rank<NumElements4JobOut()) ? (_rank*NumElements4Job(_rank)) :
                (_nelements-(_size-_rank)*NumElements4Job(_rank))); }

      inline unsigned int EndElementIndex() const
      { return StartElementIndex()+NumElements4Job(_rank); }

      inline unsigned int GetMPISize() const { return _size; }
      inline unsigned int GetMPIRank() const { return _rank; }

      bool SyncVector(ROOT::Minuit2::MnAlgebraicVector &mnvector);  
      bool SyncMatrix(ROOT::Minuit2::MnAlgebraicSymMatrix &mnmatrix);  

      static unsigned int GetMPIGlobalRank() { StartMPI(); return _globalRank; }
      static unsigned int GetMPIGlobalSize() { StartMPI(); return _globalSize; }
      static inline void StartMPI() {
#ifdef MPIPROC  
         if (!(MPI::Is_initialized())) {    
            MPI::Init();    
            std::cout << "MPIProcess:: Start MPI on #" << MPI::COMM_WORLD.Get_rank() << " processor"
                      << std::endl;
         }
         _globalSize = MPI::COMM_WORLD.Get_size();
         _globalRank = MPI::COMM_WORLD.Get_rank();
#endif
      }

      static void TerminateMPI() { 
#ifdef MPIPROC
         if (_communicators[0]!=0 && _communicators[1]!=0) {
            delete _communicators[0]; _communicators[0] = 0; _indecesComm[0] = 0;
            delete _communicators[1]; _communicators[1] = 0; _indecesComm[1] = 0;
         }
#endif

         MPITerminate terminate; 
      }

      static bool SetCartDimension(unsigned int dimX, unsigned int dimY);
      static bool SetDoFirstMPICall(bool doFirstMPICall = true);

      inline void SumReduce(const double& sub, double& total) {
         total = sub;

#ifdef MPIPROC
         if (_size>1) {
            _communicator->Allreduce(&sub,&total,1,MPI::DOUBLE,MPI::SUM);
         }
#endif
      }

   private:

#ifdef MPIPROC
      void MPISyncVector(double *ivector, int svector, double *ovector);
#endif

   private:
      unsigned int _nelements;
      unsigned int _size;
      unsigned int _rank;  

      static unsigned int _globalSize;
      static unsigned int _globalRank;  

      static unsigned int _cartSizeX;
      static unsigned int _cartSizeY;
      static unsigned int _cartDimension;
      static bool _newCart;

      unsigned int _numElements4JobIn;
      unsigned int _numElements4JobOut;

#ifdef MPIPROC
      static MPI::Intracomm *_communicator;
      static int _indexComm; // maximum 2 communicators, so index can be 0 and 1
      static MPI::Intracomm *_communicators[2]; // maximum 2 communicators
      static unsigned int _indecesComm[2];
#endif

   };

} // namespace ROOT

#endif


