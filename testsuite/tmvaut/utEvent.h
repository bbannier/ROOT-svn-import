#ifndef UTEVENT_H
#define UTEVENT_H

// Author: Christoph Rosemann   Dec. 2009
// TMVA unit tests

#include <vector>

#ifndef ROOT_Rtypes
#include "Rtypes.h"
#endif

#include "UnitTest.h"

namespace TMVA {
   class Event;
}

class utEvent : public UnitTesting::UnitTest
{
public:
   utEvent();
   void run();

private:
   // the test calls in different blocks
   // the distinctions are arbitrary:
   void _testConstructor1();
   void _testConstructor2();
   void _testConstructor3();
   void _testConstructor4();
   void _testConstructor5();
   void _testConstructor6();
   void _testMutators();

   // there are six different constructors:
   TMVA::Event* _eventC1;
   TMVA::Event* _eventC2;
   TMVA::Event* _eventC3;
   TMVA::Event* _eventC4;
   TMVA::Event* _eventC5;
   TMVA::Event* _eventC6;

   // the values needed to create all "Event" objects in all ways
   std::vector<Float_t>  _testValueVec;
   std::vector<Float_t*> _testPointerVec;
   std::vector<Float_t>  _testTargetVec;
   std::vector<Float_t>  _testSpectatorVec;

   std::vector<Float_t>  _compareValueVec;
   std::vector<Float_t>  _compareTargetVec;
   std::vector<Float_t>  _compareSpectatorVec;

   Float_t               _t, _u, _v;
   Float_t               _testScale;
   UInt_t                _testClassVal;
   Float_t               _testWeight;
   Float_t               _testBoostWeight;
   UInt_t                _testNVar;
};
#endif // UTEVENT_H
