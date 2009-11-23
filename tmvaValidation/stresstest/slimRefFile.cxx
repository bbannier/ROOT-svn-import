
void slimRefFile(const TString& nameIn = "TMVArefBig.root", const TString& nameOut="TMVAref.root") {
   TFile * inf  = TFile::Open(nameIn,"READ");
   TFile * outf = TFile::Open(nameOut,"RECREATE");

   // get a list of methods
   TIter keyIt(inf->GetListOfKeys());
   TKey *mkey1;
   while((mkey1 = (TKey*)keyIt())) {
      if( ! TString(mkey1->GetName()).BeginsWith("Method_") ) continue;
      if( ! gROOT->GetClass(mkey1->GetClassName())->InheritsFrom("TDirectory") ) continue;

      TDirectory * targetdir1 = outf->mkdir(mkey1->GetName());
      dir = (TDirectory*)mkey1->ReadObj();

      TIter keyIt2(dir->GetListOfKeys());
      TKey *mkey2;
      while((mkey2 = (TKey*)keyIt2())) {
         if( ! gROOT->GetClass(mkey2->GetClassName())->InheritsFrom("TDirectory") ) continue;

         TDirectory * targetdir2 = targetdir1->mkdir(mkey2->GetName());
         dir = (TDirectory*)mkey2->ReadObj();
         
         TIter keyIt3(dir->GetListOfKeys());
         TKey *mkey3;
         while((mkey3 = (TKey*)keyIt3())) {
            if( ! TString(mkey3->GetName()).EndsWith("_effBvsS") ) continue;
            if( ! gROOT->GetClass(mkey3->GetClassName())->InheritsFrom("TH1") ) continue;
            targetdir2->Append(mkey3->ReadObj());
         }
      }
   }
   outf->Write();
   outf->Close();
   inf->Close();
}
