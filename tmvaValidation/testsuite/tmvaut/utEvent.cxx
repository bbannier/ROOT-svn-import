
#include "utEvent.h"

#include "TMath.h"

#include "TMVA/Event.h"

using namespace std;
using namespace TMVA;

utEvent::utEvent() :
   UnitTest("Event", __FILE__)
{
   // the test values for initialisation
   _testValueVec.push_back(1.);
   _testValueVec.push_back(2.);
   _testValueVec.push_back(3.);
   _t = 1.;
   _u = 2.;
   _v = 3.;
   _testPointerVec.push_back(&_t);
   _testPointerVec.push_back(&_u);
   _testPointerVec.push_back(&_v);
   _testTargetVec.push_back(11.);
   _testTargetVec.push_back(12.);
   _testTargetVec.push_back(13.);
   _testSpectatorVec.push_back(25.);
   _testScale       = 2.7818;
   _testClassVal    = 2;
   _testWeight      = 3.1415;
   _testBoostWeight = 0.1234;
   _testNVar        = 3;
}



void utEvent::run()
{
   _testConstructor1();
   _testConstructor3();
   _testConstructor2(); // constructor 3 must be called before...
   _testConstructor4();
   _testConstructor5();
   _testConstructor6();
}



void utEvent::_testConstructor1()
{
   _eventC1 = new Event();

   test_(_eventC1->IsDynamic()         == false);

   test_(floatCompare(_eventC1->GetWeight(), 1.));
   test_(floatCompare(_eventC1->GetOriginalWeight(), 1.));
   test_(floatCompare(_eventC1->GetBoostWeight(),1.));

   test_(_eventC1->GetWeight()         == 1.);
   test_(_eventC1->GetOriginalWeight() == 1.);
   test_(_eventC1->GetBoostWeight()    == 1.);
   test_(_eventC1->GetClass()          == 0);
   test_(_eventC1->GetNVariables()     == 0);
   test_(_eventC1->GetNTargets()       == 0);
   test_(_eventC1->GetNSpectators()    == 0);

   _testMutators();
}



void utEvent::_testConstructor2()
{
   _eventC2 = new Event(*_eventC3);

   test_(_eventC2->IsDynamic()         == false);
   
   test_(floatCompare(_eventC2->GetWeight(), _testWeight*_testBoostWeight));
   test_(floatCompare(_eventC2->GetOriginalWeight(), _testWeight));
   test_(floatCompare(_eventC2->GetBoostWeight(), _testBoostWeight));

   test_(_eventC2->GetClass()          == _testClassVal);
   test_(_eventC2->GetNVariables()     == (UInt_t)_testValueVec.size());
   test_(_eventC2->GetNTargets()       == (UInt_t)_testTargetVec.size());
   test_(_eventC2->GetNSpectators()    == (UInt_t)_testSpectatorVec.size());
   // ? const std::vector<UInt_t>* GetVariableArrangement() const { return fVariableArrangement; }

   _compareValueVec = _eventC2->GetValues();
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         test_(_eventC2->GetValue(index)  == *it);
         test_(_compareValueVec.at(index) == *it);
      }
   _compareTargetVec = _eventC2->GetTargets();
   for(vector<Float_t>::const_iterator it = _testTargetVec.begin(); it < _testTargetVec.end(); ++it)
      {
         unsigned int index = it - _testTargetVec.begin();
         test_(_eventC2->GetTarget(index)  == *it);
         test_(_compareTargetVec.at(index) == *it);
      }
   _compareSpectatorVec = _eventC2->GetSpectators();
   for(vector<Float_t>::const_iterator it = _testSpectatorVec.begin(); it < _testSpectatorVec.end(); ++it)
      {
         unsigned int index = it - _testSpectatorVec.begin();
         test_(_eventC2->GetSpectator(index)  == *it);
         test_(_compareSpectatorVec.at(index) == *it);
      }

}



void utEvent::_testConstructor3()
{
   _eventC3 = new Event( _testValueVec, _testTargetVec, _testSpectatorVec, _testClassVal, _testWeight, _testBoostWeight);

   test_(_eventC3->IsDynamic()         == false);

   test_(floatCompare(_eventC3->GetWeight(), _testWeight*_testBoostWeight));
   test_(floatCompare(_eventC3->GetOriginalWeight(), _testWeight));
   test_(floatCompare(_eventC3->GetBoostWeight(), _testBoostWeight));

   test_(_eventC3->GetClass()          == _testClassVal);
   test_(_eventC3->GetNVariables()     == (UInt_t)_testValueVec.size());
   test_(_eventC3->GetNTargets()       == (UInt_t)_testTargetVec.size());
   test_(_eventC3->GetNSpectators()    == (UInt_t)_testSpectatorVec.size());
   // ? const std::vector<UInt_t>* GetVariableArrangement() const { return fVariableArrangement; }

   _compareValueVec = _eventC3->GetValues();
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         test_(_eventC3->GetValue(index)  == *it);
         test_(_compareValueVec.at(index) == *it);
      }
   _compareTargetVec = _eventC3->GetTargets();
   for(vector<Float_t>::const_iterator it = _testTargetVec.begin(); it < _testTargetVec.end(); ++it)
      {
         unsigned int index = it - _testTargetVec.begin();
         test_(_eventC3->GetTarget(index)  == *it);
         test_(_compareTargetVec.at(index) == *it);
      }
   _compareSpectatorVec = _eventC3->GetSpectators();
   for(vector<Float_t>::const_iterator it = _testSpectatorVec.begin(); it < _testSpectatorVec.end(); ++it)
      {
         unsigned int index = it - _testSpectatorVec.begin();
         test_(_eventC3->GetSpectator(index)  == *it);
         test_(_compareSpectatorVec.at(index) == *it);
      }
}



void utEvent::_testConstructor4()
{
   _eventC4 = new Event( _testValueVec, _testTargetVec, _testClassVal, _testWeight, _testBoostWeight);

   test_(_eventC4->IsDynamic()         == false);

   test_(floatCompare(_eventC4->GetWeight(), _testWeight*_testBoostWeight));
   test_(floatCompare(_eventC4->GetOriginalWeight(), _testWeight));
   test_(floatCompare(_eventC4->GetBoostWeight(), _testBoostWeight));

   test_(_eventC4->GetClass()          == _testClassVal);
   test_(_eventC4->GetNVariables()     == (UInt_t)_testValueVec.size());
   test_(_eventC4->GetNTargets()       == (UInt_t)_testTargetVec.size());

   _compareValueVec = _eventC4->GetValues();
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         test_(_eventC4->GetValue(index)  == *it);
         test_(_compareValueVec.at(index) == *it);
      }
   _compareTargetVec = _eventC4->GetTargets();
   for(vector<Float_t>::const_iterator it = _testTargetVec.begin(); it < _testTargetVec.end(); ++it)
      {
         unsigned int index = it - _testTargetVec.begin();
         test_(_eventC4->GetTarget(index)  == *it);
         test_(_compareTargetVec.at(index) == *it);
      }
}



void utEvent::_testConstructor5()
{
   _eventC5 = new Event( _testValueVec, _testClassVal, _testWeight, _testBoostWeight);

   test_(_eventC5->IsDynamic()         == false);

   test_(floatCompare(_eventC5->GetWeight(), _testWeight*_testBoostWeight));
   test_(floatCompare(_eventC5->GetOriginalWeight(), _testWeight));
   test_(floatCompare(_eventC5->GetBoostWeight(), _testBoostWeight));

   test_(_eventC5->GetClass()          == _testClassVal);
   test_(_eventC5->GetNVariables()     == (UInt_t)_testValueVec.size());

   _compareValueVec = _eventC5->GetValues();
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         test_(_eventC5->GetValue(index)  == *it);
         test_(_compareValueVec.at(index) == *it);
      }
}



void utEvent::_testConstructor6()
{
   const vector<Float_t*>* _constPointerToPointerVec = &_testPointerVec;
   _eventC6 = new Event( _constPointerToPointerVec, _testNVar);

   // TODO I don't understand what the constructor is for
   // or in what cases it should be used

   test_(_eventC6->IsDynamic()         == true);

   //   test_(_eventC6->GetWeight()         == 1.);
   //   test_(_eventC6->GetOriginalWeight() == 1.);
   //   test_(_eventC6->GetBoostWeight()    == 1.);
   //   test_(_eventC6->GetClass()          == _testClassVal);
   //   test_(_eventC6->GetNVariables()     == (UInt_t)_testValueVec.size());

   // ? const std::vector<UInt_t>* GetVariableArrangement() const { return fVariableArrangement; }
  
   //   _compareValueVec = _eventC6->GetValues();
   //   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
   //     {
   //       unsigned int index = it - _testValueVec.begin();
   //       test_(_eventC6->GetValue(index)  == *it);
   //       test_(_compareValueVec.at(index) == *it);
   //     }
}



void utEvent::_testMutators()
{
   // the empty/default constructor is taken for these tests

   _eventC1->SetWeight(_testWeight);
   test_(_eventC1->GetWeight() == _testWeight);

   // ScaleWeight has been removed in the latest TMVA verisions...
   //   _eventC1->ScaleWeight(_testScale);  
   //   test_(floatCompare((float) _eventC1->GetWeight(), _testWeight*_testScale)); 

   _eventC1->SetBoostWeight(_testBoostWeight);
   test_(floatCompare( _eventC1->GetBoostWeight() , _testBoostWeight));
   _eventC1->ScaleBoostWeight(_testScale);
   test_(floatCompare( _eventC1->GetBoostWeight(), _testBoostWeight*_testScale));


   _eventC1->SetClass(_testClassVal);
   test_(_eventC1->GetClass() == (UInt_t)_testClassVal);

   // check variables
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         _eventC1->SetVal(index, *it);
      }
   _compareValueVec = _eventC1->GetValues();
   for(vector<Float_t>::const_iterator it = _testValueVec.begin(); it < _testValueVec.end(); ++it)
      {
         unsigned int index = it - _testValueVec.begin();
         test_(_eventC1->GetValue(index)  == *it);
         test_(_compareValueVec.at(index) == *it);
      }
   // check targets
   for(vector<Float_t>::const_iterator it = _testTargetVec.begin(); it < _testTargetVec.end(); ++it)
      {
         unsigned int index = it - _testTargetVec.begin();
         _eventC1->SetTarget(index, *it);
      }
   _compareTargetVec = _eventC1->GetTargets();
   for(vector<Float_t>::const_iterator it = _testTargetVec.begin(); it < _testTargetVec.end(); ++it)
      {
         unsigned int index = it - _testTargetVec.begin();
         test_(_eventC1->GetTarget(index)  == *it);
         test_(_compareTargetVec.at(index) == *it);
      }
   // check spectators
   for(vector<Float_t>::const_iterator it = _testSpectatorVec.begin(); it < _testSpectatorVec.end(); ++it)
      {
         unsigned int index = it - _testSpectatorVec.begin();
         _eventC1->SetSpectator(index, *it);
      }
   _compareSpectatorVec = _eventC1->GetSpectators();
   for(vector<Float_t>::const_iterator it = _testSpectatorVec.begin(); it < _testSpectatorVec.end(); ++it)
      {
         unsigned int index = it - _testSpectatorVec.begin();
         test_(_eventC1->GetSpectator(index)  == *it);
         test_(_compareSpectatorVec.at(index) == *it);
      }
  
   _eventC1->SetClass(_testClassVal);
   test_(_eventC1->GetClass() == (UInt_t)_testClassVal);

   // ??    _eventC1->SetVariableArrangement( std::vector<UInt_t>* const m ) const;
}

