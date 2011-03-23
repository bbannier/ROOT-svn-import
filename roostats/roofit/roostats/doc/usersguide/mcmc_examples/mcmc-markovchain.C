MCMCInterval* interval = (MCMCInterval*)mcmcCalc.GetInterval(); // must cast
const MarkovChain* chain = interval->GetChain();

// Print the contents of the chain
for (Int_t i = 0; i < chain->Size(); i++) {
   cout << "Entry # " << i << endl;
   const RooArgSet* entry = chain->Get(i);
   entry->Print("v");
   cout << "weight = " << chain->Weight() << endl; // weight of current entry
   cout << "NLL = "    << chain->NLL()    << endl; // NLL value of current entry
}
