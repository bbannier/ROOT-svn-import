#include "TVector3.h"
#include <vector>

class Particle {
public:
   double fPosX,fPosY,fPosZ; // particle position nearest to interaction point
   double fMomentum; // particle momentum
   double fMomentumPhi; // particle direction (phi)
   double fMomentumEta; // particle direction (eta)
};

class EventData {
public:
   std::vector<Particle> fParticles; // particles of the event
   int fEventSize; // size (in bytes) of the event

   ClassDef(EventData,1); // Data for an event
};
