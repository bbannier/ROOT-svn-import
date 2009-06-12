// @(#)root/minuit2:$Id$
// Author: A. Lazzaro 2009
/***************************************************************************
 * Package: Minuit2                                                        *
 *    File: $Id$         *
 *  Author: Alfio Lazzaro, alfio.lazzaro@mi.infn.it                        *
 *                                                                         *
 * Copyright: (C) 2008 by Universita' and INFN, Milan                      *
 ***************************************************************************/

#include "Minuit2/MPIProcess.h"

#include <iostream>

namespace ROOT {

  unsigned int MPIProcess::_globalSize = 1;
   unsigned int MPIProcess::_globalRank = 0;

   // By default all procs are for X
   unsigned int MPIProcess::_cartSizeX = 0;
   unsigned int MPIProcess::_cartSizeY = 0;
   unsigned int MPIProcess::_cartDimension = 0;
   bool MPIProcess::_newCart = true;

#ifdef MPIPROC
   MPI::Intracomm* MPIProcess::_communicator = 0;
   int MPIProcess::_indexComm = -1; // -1 for no-initialization
   MPI::Intracomm* MPIProcess::_communicators[2] = {0};
   unsigned int MPIProcess::_indecesComm[2] = {0};
#endif

   MPIProcess::MPIProcess(unsigned int nelements, unsigned int indexComm) :
      _nelements(nelements), _size(1), _rank(0)
   {
#ifdef MPIPROC

      StartMPI();

      // check local requested index for communicator, valid values are 0 and 1
      indexComm = (indexComm==0) ? 0 : 1;

      if (_globalSize==_cartDimension && 
          _cartSizeX!=_cartDimension && _cartSizeY!=_cartDimension) {
         //declare the cartesian topology

         if (_communicator==0 && _indexComm<0 && _newCart) {
            // first call, declare the topology
            std::cout << "MPIProcess:: Declare cartesian Topology (" 
                      << _cartSizeX << "x" << _cartSizeY << ")" << std::endl;
      
            int color = _globalRank / _cartSizeY;
            int key = _globalRank % _cartSizeY;

            _communicators[0] = new MPI::Intracomm(MPI::COMM_WORLD.Split(key,color)); // rows for Minuit
            _communicators[1] = new MPI::Intracomm(MPI::COMM_WORLD.Split(color,key)); // columns for NLL

            _newCart = false;

         }

         _indexComm++;

         if (_indexComm>1 || _communicator==(&(MPI::COMM_WORLD))) { // Remember, no more than 2 dimensions in the topology!
            std::cerr << "MPIProcess:: Error: Requiring more than 2 dimensions in the topology!" << std::endl;
            MPI::COMM_WORLD.Abort(-1);
         } 

         // requiring columns as first call. In this case use all nodes
         if (((unsigned int)_indexComm)<indexComm)
            _communicator = &(MPI::COMM_WORLD);
         else {
            _indecesComm[_indexComm] = indexComm;
            _communicator = _communicators[_indecesComm[_indexComm]];
         }

      }
      else {
         // no cartesian topology
         if (_cartDimension!=0 && _globalSize!=_cartDimension) {
            std::cout << "MPIProcess:: Cartesian dimension doesn't correspond to # total procs!" << std::endl;
            std::cout << "MPIProcess:: Ignoring topology, use all procs for X." << std::endl;
            std::cout << "MPIProcess:: Resetting topology..." << std::endl;
            _cartSizeX = _globalSize;
            _cartSizeY = 1;
            _cartDimension = _globalSize;
         }

         if (_indexComm<0) {
            if (_cartSizeX==_cartDimension) {
               _communicators[0] = &(MPI::COMM_WORLD);
               _communicators[1] = 0;
            }
            else {
               _communicators[0] = 0;
               _communicators[1] = &(MPI::COMM_WORLD);
            }
         }

         _indexComm++;

         if (_indexComm>1) { // Remember, no more than 2 nested MPI calls!
            std::cerr << "MPIProcess:: Error: More that 2 nested MPI calls!" << std::endl;
            MPI::COMM_WORLD.Abort(-1);
         }

         _indecesComm[_indexComm] = indexComm;

         // require 2 nested communicators
         if (_communicator!=0 && _communicators[indexComm]!=0) {
            std::cerr << "MPIProcess:: Requiring 2 nested MPI calls!" << std::endl;
            std::cout << "MPIProcess:: Ignoring second call." << std::endl;
            _indecesComm[_indexComm] = (indexComm==0) ? 1 : 0;
         } 

         _communicator = _communicators[_indecesComm[_indexComm]];

      }

      // set size and rank
      if (_communicator!=0) {
         _size = _communicator->Get_size();
         _rank = _communicator->Get_rank();
      }
      else {
         // no MPI calls
         _size = 1;
         _rank = 0;
      }
    

      if (_size>_nelements) {
         std::cerr << "MPIProcess:: Error: more processors than elements!" << std::endl;
         MPI::COMM_WORLD.Abort(-1);
      }

#endif

      _numElements4JobIn = _nelements/_size;
      _numElements4JobOut = _nelements%_size;

   }

   MPIProcess::~MPIProcess()
   {
#ifdef MPIPROC  
      _communicator = 0;
      _indexComm--; 
      if (_indexComm==0)
         _communicator = _communicators[_indecesComm[_indexComm]];
    
#endif

   }

   bool MPIProcess::SyncVector(ROOT::Minuit2::MnAlgebraicVector &mnvector)
   {
    
      // In case of just one job, don't need sync, just go
      if (_size<2)
         return false;
    
#ifdef MPIPROC
      unsigned int numElements4ThisJob = NumElements4Job(_rank);
      unsigned int startElementIndex = StartElementIndex();
      unsigned int endElementIndex = EndElementIndex();

      double dvectorJob[numElements4ThisJob];
      for(unsigned int i = startElementIndex; i<endElementIndex; i++)
         dvectorJob[i-startElementIndex] = mnvector(i);

      double dvector[_nelements];
      MPISyncVector(dvectorJob,numElements4ThisJob,dvector);

      for (unsigned int i = 0; i<_nelements; i++) {
         mnvector(i) = dvector[i];
      }                             

      return true;

#else

      std::cerr << "Error: no MPI syncronization is possible!" << std::endl;
      exit(-1);

#endif

      return true;
   }
  

   bool MPIProcess::SyncMatrix(ROOT::Minuit2::MnAlgebraicSymMatrix &mnmatrix)
   {
  
      // In case of just one job, don't need sync, just go
      if (_size<2)
         return false;
    
#ifdef MPIPROC
      unsigned int numElements4ThisJob = NumElements4Job(_rank);
      unsigned int startElementIndex = StartElementIndex();
      unsigned int endElementIndex = EndElementIndex();
      unsigned int n = mnmatrix.Nrow();

      unsigned int offsetVect = 0;
      for (unsigned int i = 0; i<startElementIndex; i++)
         if ((i+offsetVect)%(n-1)==0) offsetVect += (i+offsetVect)/(n-1);

      double dvectorJob[numElements4ThisJob];
      for(unsigned int i = startElementIndex; i<endElementIndex; i++) {

         int x = (i+offsetVect)/(n-1);
         if ((i+offsetVect)%(n-1)==0) offsetVect += x;
         int y = (i+offsetVect)%(n-1)+1;

         dvectorJob[i-startElementIndex] = mnmatrix(x,y);
      
      }

      double dvector[_nelements];
      MPISyncVector(dvectorJob,numElements4ThisJob,dvector);

      offsetVect = 0;
      for (unsigned int i = 0; i<_nelements; i++) {
    
         int x = (i+offsetVect)/(n-1);
         if ((i+offsetVect)%(n-1)==0) offsetVect += x;
         int y = (i+offsetVect)%(n-1)+1;

         mnmatrix(x,y) = dvector[i];

      }

      return true;

#else

      std::cerr << "Error: no MPI syncronization is possible!" << std::endl;
      exit(-1);

#endif

      return true;
   }

#ifdef MPIPROC
   void MPIProcess::MPISyncVector(double *ivector, int svector, double *ovector)
   {
      int offsets[_size];
      int nconts[_size];
      nconts[0] = NumElements4Job(0);
      offsets[0] = 0;
      for (unsigned int i = 1; i<_size; i++) {
         nconts[i] = NumElements4Job(i);
         offsets[i] = nconts[i-1] + offsets[i-1];
      }

      _communicator->Allgatherv(ivector,svector,MPI::DOUBLE,
                                ovector,nconts,offsets,MPI::DOUBLE); 
    
   }

   bool MPIProcess::SetCartDimension(unsigned int dimX, unsigned int dimY)  
   {
      if (_communicator!=0 || _indexComm>=0) {
         std::cout << "MPIProcess:: MPIProcess already declared! Ignoring command..." << std::endl;
         return false;
      }
      if (dimX*dimY<=0) {
         std::cout << "MPIProcess:: Invalid topology! Ignoring command..." << std::endl;
         return false;
      }

      StartMPI();

      if (_globalSize!=dimX*dimY) {
         std::cout << "MPIProcess:: Cartesian dimension doesn't correspond to # total procs!" << std::endl;
         std::cout << "MPIProcess:: Ignoring command..." << std::endl;
         return false;
      }

      if (_cartSizeX!=dimX || _cartSizeY!=dimY) {
         _cartSizeX = dimX; _cartSizeY = dimY; 
         _cartDimension = _cartSizeX*_cartSizeY;
         _newCart = true;

         if (_communicators[0]!=0 && _communicators[1]!=0) {
            delete _communicators[0]; _communicators[0] = 0; _indecesComm[0] = 0;
            delete _communicators[1]; _communicators[1] = 0; _indecesComm[1] = 0;
         }
      }

      return true;

   }

   bool MPIProcess::SetDoFirstMPICall(bool doFirstMPICall)
   { 

      StartMPI();

      bool ret;
      if (doFirstMPICall)
         ret = SetCartDimension(_globalSize,1);
      else
         ret = SetCartDimension(1,_globalSize);

      return ret;

   }

#endif

#ifdef MPIPROC
   MPITerminate dummyMPITerminate = MPITerminate();
#endif

} // namespace ROOT
