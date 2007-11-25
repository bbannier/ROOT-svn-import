// $Id: t0_digits.C,v 1.1 2007/04/17 12:18:53 mtadel Exp $

namespace Alieve {
class Event;
}



void t0_digits()
{
  AliRunLoader* rl =  Alieve::Event::AssertRunLoader();
  Int_t g_currentEvent = Alieve::gEvent->GetEventId();
    cout<<g_currentEvent<<endl;

  rl->LoadDigits("T0");
  TTree* dt = rl->GetTreeD("T0", false);
  //Alieve::T0DigitsInfo* di = new Alieve::T0DigitsInfo();
  //  di->SetDTree(dt);
  //di->Dump();
  //  AliSTARTdigit *digits = 0;
 
  AliT0digit *digits = 0;
  dt->SetBranchAddress("T0", &digits);
  dt->GetEntry(0);

  gStyle->SetPalette(1, 0);
  //  gEve->DisableRedraw();

  Alieve::T0Module::MakeModules(digits);

  //Alieve::T0Module* m = new Alieve::T0Module(di);
  //
  //gEve->EnableRedraw();
  //m->LoadQuadsDigits();
}

