#ifndef ROOT_TProofNode
#define ROOT_TProofNode

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif

class TProofNode : public TNamed
{
public:
   TProofNode(const char *wn, Int_t ram) : TNamed(wn,""), fPhysRam(ram), fNWrks(1) {}
   virtual ~TProofNode() {}

   void AddWrks(Int_t n = 1) { fNWrks += n; }
   Int_t GetPhysRam() const { return fPhysRam; }
   Int_t GetNWrks() const { return fNWrks; }
   void SetNWrks(Int_t n) { fNWrks = n; }

private:
   Int_t   fPhysRam;   // Physical RAM of this worker node
   Int_t   fNWrks;     // Number of workers on this node

   ClassDef(TProofNode,0)
 };

#endif
