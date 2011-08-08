#ifndef ROOT_EuropeMapDemo
#define ROOT_EuropeMapDemo

#include <memory>

#include "DemoBase.h"

class TH2Poly;

namespace ROOT_iOS {
namespace Demos {

class EuropeMapDemo : public DemoBase {
public:
   EuropeMapDemo();
   ~EuropeMapDemo();
   
   //overriders.
   void ResetDemo() {}
   bool IsAnimated()const {return false;}
   unsigned NumOfFrames()const {return 1;}
   double AnimationTime()const {return 0.;}
   
   void StartAnimation(){}
   void NextStep(){}
   void StopAnimation(){}

   void AdjustPad(PadProxy *) {}

   void PresentDemo();
   
   bool Supports3DRotation() const {return false;}
private:
   std::auto_ptr<TH2Poly> fPoly;
   
   EuropeMapDemo(const EuropeMapDemo &rhs);
   EuropeMapDemo &operator = (const EuropeMapDemo &rhs);
};

}
}

#endif
