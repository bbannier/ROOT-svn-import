// $Id: t0_raw.C,v 1.1 2007/04/17 12:18:53 mtadel Exp $

namespace Alieve {
class Event;
}



void t0_raw()
{
  AliRunLoader* rl =  Alieve::Event::AssertRunLoader();
  Int_t ievt = Alieve::gEvent->GetEventId();
    cout<<ievt<<endl;

  gStyle->SetPalette(1, 0);

  Alieve::T0Module::LoadRaw("raw.root",ievt);


}

