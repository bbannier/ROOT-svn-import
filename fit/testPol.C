{ 

   std::string fname("pol2");
   TF1 * func = new TF1("pol2",fname.c_str(),-5,5.);
   func->SetParameter(0,1);
   func->SetParameter(1,0);
   func->SetParameter(2,1.0);

   TRandom3 rndm;

   // fill an histogram 
   TH1D * h1 = new TH1D("h1","h1",5,-5.,5.);
//      h1->FillRandom(fname.c_str(),100);
   for (int i = 0; i <1000; ++i) 
      h1->Fill( func->GetRandom() );

   h1->Print();
   h1->Draw();

}
