#include "RooFit.h"
#include "RooMsgService.h"
#include "RooUnBinDataStore.h"
#include "RooAbsReal.h"

#include "Riostream.h"
#include "TTree.h"
#include "TChain.h"
#include "TDirectory.h"
#include "TROOT.h"

#include "Fit/UnBinData.h"

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore()
{
}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(TTree* tree, const RooArgSet& vars, const char* wgtVarName) :
  RooAbsDataStore("blah","blah",varsNoWeight(vars,wgtVarName))
{
  // Constructor to facilitate reading of legacy RooDataSets
  loadValues(tree);
}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
   _fitData = new ROOT::Fit::UnBinData(100,vars.getSize());
}


//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const char* name, const char* title, 
				   const RooArgSet& vars, TTree &tree,
				   const RooFormulaVar& select, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
  loadValues(&tree,&select) ;
}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const char* name, const char* title, 
				   const RooArgSet& vars, TTree &tree,
				   const char* selExpr, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
  if (selExpr && *selExpr) {
    // Create a RooFormulaVar cut from given cut expression
    RooFormulaVar select(selExpr,selExpr,_vars) ;
    loadValues(&tree,&select);
  } else {
    loadValues(&tree);
  }

}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const RooAbsDataStore& tds, 
				   const RooFormulaVar& select, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
  loadValues(&tds,&select) ;
}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const RooAbsDataStore& ads, 
				   const char* selExpr, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
  if (selExpr && *selExpr) {
    // Create a RooFormulaVar cut from given cut expression
    RooFormulaVar select(selExpr,selExpr,_vars) ;
    loadValues(&ads,&select);
  } else {
    loadValues(&ads);
  }
}


RooUnBinDataStore::RooUnBinDataStore(const char *name, const char *title, RooAbsDataStore& tds, 
				   const RooArgSet& vars, const RooFormulaVar* cutVar, const char* cutRange,
				   Int_t nStart, Int_t nStop, Bool_t /*copyCache*/, const char* wgtVarName) :
  RooAbsDataStore(name,title,varsNoWeight(vars,wgtVarName))
{
  // Deep clone cutVar and attach clone to this dataset
  RooFormulaVar* cloneVar = 0;
  if (cutVar) {    
    cloneVar = (RooFormulaVar*) cutVar->cloneTree() ;
    cloneVar->attachDataStore(tds) ;
  }

  loadValues(&tds,cloneVar,cutRange,nStart,nStop);

  if (cloneVar) delete cloneVar ;

}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const RooUnBinDataStore& other, const char* newname) :
  RooAbsDataStore(other,newname)
{
  loadValues(&other) ;
}

//_____________________________________________________________________________
RooUnBinDataStore::RooUnBinDataStore(const RooUnBinDataStore& other, const RooArgSet& vars, const char* newname) :
  RooAbsDataStore(other,varsNoWeight(vars,0),newname)
{
  loadValues(&other) ;
}

//_____________________________________________________________________________
RooUnBinDataStore::~RooUnBinDataStore()  { 
   if (_fitData) delete _fitData; 
}

//_____________________________________________________________________________
RooArgSet RooUnBinDataStore::varsNoWeight(const RooArgSet& allVars, const char* wgtName) 
{
  // Utility function for constructors
  // Return RooArgSet that is copy of allVars minus variable matching wgtName if specified

  RooArgSet ret(allVars) ;
  if(wgtName) {
    RooAbsArg* wgt = allVars.find(wgtName) ;
    if (wgt) {
      ret.remove(*wgt,kTRUE,kTRUE) ;
    }
  }

  // Alfio: Read only RooAbsReal for the moment
  RooAbsArg *var(0);
  RooArgSet newRet;
  TIterator* iter = ret.createIterator() ;
  while ((var=(RooAbsArg*)iter->Next())) {
    if (dynamic_cast<RooAbsReal*>(var)) {
      newRet.add(*var) ;
    }
  }

  delete iter;

  return newRet ;
}

//_____________________________________________________________________________
void RooUnBinDataStore::loadValues(const TTree *t, const RooFormulaVar* select, const char* /*rangeName*/, Int_t /*nStart*/, Int_t /*nStop*/)
{
  // Load values from tree into this data collection, optionally
  // selecting events using 'select' RooFormulaVar
  //
  // The source tree 't' is first clone as not disturb its branch
  // structure when retrieving information from it.
  
  // Clone source tree
  // WVE Clone() crashes on trees, CloneTree() crashes on tchains :-(

  // LM : Is cloning of Tree needed ???

  // Change directory to memory dir before cloning tree to avoid ROOT errors
  TString pwd(gDirectory->GetPath()) ;
  TString memDir(gROOT->GetName()) ;
  memDir.Append(":/") ;
  Bool_t notInMemNow= (pwd!=memDir) ;

  if (notInMemNow) {
    gDirectory->cd(memDir) ;
  }

  TTree* tClone ;
  if (dynamic_cast<const TChain*>(t)) {
    tClone = (TTree*) t->Clone() ; 
  } else {
    tClone = ((TTree*)t)->CloneTree() ;
  }

  // Change directory back to original directory
  tClone->SetDirectory(0) ;

  if (notInMemNow) {
    gDirectory->cd(pwd) ;
  }

  // Clone list of variables  
  RooArgSet *sourceArgSet = (RooArgSet*) _vars.snapshot(kFALSE) ;
  // Attach each variable to the dataset
  TIterator* sourceIter =  sourceArgSet->createIterator() ;
  RooAbsArg *sourceArg = 0;
  while ((sourceArg=(RooAbsArg*)sourceIter->Next())) {
    sourceArg->attachToTree(*tClone) ; // Alfio: require to add the friend class inside RooAbsArg
  }

  // Redirect formula servers to sourceArgSet
  RooFormulaVar* selectClone(0) ;
  if (select) {
    selectClone = (RooFormulaVar*) select->cloneTree() ;
    selectClone->recursiveRedirectServers(*sourceArgSet) ;
    
    RooArgSet branchList ;
    selectClone->branchNodeServerList(&branchList) ;
    TIterator* iter = branchList.createIterator() ;
    RooAbsArg* arg ;
    while((arg=(RooAbsArg*)iter->Next())) {
      arg->setOperMode(RooAbsArg::ADirty) ;
    }
    delete iter ;
  }

  // Loop over events in source tree   
  Int_t numInvalid(0) ;
  Int_t nevents = (Int_t)tClone->GetEntries();

  if (_fitData) _fitData = new ROOT::Fit::UnBinData();
  _fitData->Initialize(_fitData->Size() + nevents, _vars.getSize() );
     //_vectData.reserve(_vectData.size()+nevents*_vars.getSize()); // reserve more elements

  for (Int_t i=0; i < nevents; ++i) {
    Int_t entryNumber = tClone->GetEntryNumber(i);
    if (entryNumber<0) break;
    tClone->GetEntry(entryNumber,1);

    _vars.assignValueOnly(*sourceArgSet) ;

    _iterator->Reset() ;
    Bool_t allOK(kTRUE) ;
    while ((sourceArg=(RooAbsArg*)_iterator->Next())) {
      if (!sourceArg->isValid()) {
	numInvalid++ ;
	allOK=kFALSE ;
	break ;
      }       
    }   
    
    // Does this event pass the cuts?
    if (!allOK || (selectClone && selectClone->getVal()==0)) {
      continue ; 
    }    

    fill();

  }

  if (numInvalid>0) {
    coutI(Eval) << "RooUnBinDataStore::loadValues(" << GetName() << ") Ignored " << numInvalid << " out of range events" << endl ;
  }
  
  delete sourceIter ;
  delete sourceArgSet ;
  delete selectClone ;
  delete tClone;

}

//_____________________________________________________________________________
void RooUnBinDataStore::loadValues(const RooAbsDataStore *ads, const RooFormulaVar* select, 
                                  const char* rangeName, Int_t nStart, Int_t nStop)  
{
  // Load values from dataset 't' into this data collection, optionally
  // selecting events using 'select' RooFormulaVar
  //

  // Redirect formula servers to source data row
  RooFormulaVar* selectClone(0) ;
  if (select) {
    selectClone = (RooFormulaVar*) select->cloneTree() ;
    selectClone->recursiveRedirectServers(*ads->get()) ;

    RooArgSet branchList ;
    selectClone->branchNodeServerList(&branchList) ;
    TIterator* iter = branchList.createIterator() ;
    RooAbsArg* arg ;
    while((arg=(RooAbsArg*)iter->Next())) {
      arg->setOperMode(RooAbsArg::ADirty) ;
    }
    delete iter ;
  }

  // Force RDS internal initialization
  ads->get(0) ;

  // Loop over events in source tree   
  RooAbsArg* arg = 0;
  Int_t nevents = nStop < ads->numEntries() ? nStop : ads->numEntries() ;
  Int_t numInvalid(0) ;
  if (_fitData) _fitData = new ROOT::Fit::UnBinData();
  _fitData->Initialize(_fitData->Size() + nevents, _vars.getSize() );
  //_vectData.reserve(_vectData.size()+(nevents-nStart)*_vars.getSize()); // reserve more elements

  for(Int_t i=nStart; i < nevents ; ++i) {
    const RooArgSet *sourceVar = ads->get(i) ;

    // Does this event pass the cuts?
    if (selectClone && selectClone->getVal()==0) {
      continue ; 
    }

    _vars.assignValueOnly(*sourceVar) ;

    _iterator->Reset() ;
    // Check that all copied values are valid
    Bool_t allOK(kTRUE);
    while((arg=(RooAbsArg*)_iterator->Next())) {
      if (!arg->isValid() || (rangeName && !arg->inRange(rangeName))) {
	numInvalid++ ;
        allOK = kFALSE ;
        break ;
      }
    }

    if (!allOK) continue ;

    fill();

  }

  if (numInvalid>0) {
    coutI(Eval) << "RooUnBinDataStore::loadValues(" << GetName() << ") Ignored " << numInvalid << " out of range events" << endl ;
  }

  delete selectClone ;
  SetTitle(ads->GetTitle());

}


//_____________________________________________________________________________
const RooArgSet* RooUnBinDataStore::get(Int_t index) const 
{
  // Load the n-th data point (n='index') in memory
  // and return a pointer to the internal RooArgSet
  // holding its coordinates.
  
//   std::vector<Double_t>::const_iterator vectDataIter = _vectData.begin()+index*_vars.getSize();
//   if (vectDataIter>_vectData.end()-_vars.getSize())
//     return 0;

   if (!_fitData ||  (int) _fitData->PointSize() != _vars.getSize() ) return 0; 
   const double * vectDataIter = _fitData->Coords(index);


  _iterator->Reset();
  RooAbsReal *var(0); // Alfio: Read only RooAbsReal for the moment
  while ((var=(RooAbsReal*)_iterator->Next())) {
    var->_value = (*vectDataIter); var->setValueDirty(); // Alfio: require to add the friend class inside RooAbsReal
    ++vectDataIter;
  }

  return &_vars;

}


//_____________________________________________________________________________
Double_t RooUnBinDataStore::weight(Int_t index) const 
{
  // Return the weight of the n-th data point (n='index') in memory
  get(index) ;
  return weight() ;
}



//_____________________________________________________________________________
Double_t RooUnBinDataStore::weight() const 
{
  // Return the weight of the n-th data point (n='index') in memory
  return 1 ;
}


//_____________________________________________________________________________
Double_t RooUnBinDataStore::weightError(RooAbsData::ErrorType /*etype*/) const 
{
  return 0;
}

//_____________________________________________________________________________
void RooUnBinDataStore::weightError(Double_t& lo, Double_t& hi, RooAbsData::ErrorType /*etype*/) const
{

  lo = 0;
  hi = 0;

}

//_____________________________________________________________________________
Int_t RooUnBinDataStore::numEntries() const 
{
   return (_fitData) ? _fitData->Size() : 0; 
}

//_____________________________________________________________________________
Bool_t RooUnBinDataStore::valid() const 
{
  // Return true if currently loaded coordinate is considered valid within
  // the current range definitions of all observables
  return kTRUE ;
}


//_____________________________________________________________________________
Int_t RooUnBinDataStore::fill()
{
   if (!_fitData) return 0; 
   // extend size if needed by chunk of 100 events
   if (_fitData->Size() <= _fitData->DataSize() )
      _fitData->Resize(_fitData->Size() + 100);

  _iterator->Reset() ;
  std::vector<double> v; 
  v.reserve(_vars.getSize() );
  RooAbsReal *var(0); // Alfio: Read only RooAbsReal for the moment
  while ((var=(RooAbsReal*)_iterator->Next())) {
    v.push_back(var->getVal());
  }
  _fitData->Add(&v.front() );

  return sizeof(Double_t)*_vars.getSize();

}


//_____________________________________________________________________________
Bool_t RooUnBinDataStore::changeObservableName(const char* from, const char* to) 
{
  // Change name of internal observable named 'from' into 'to'

  // Find observable to be changed
  RooAbsArg* var = _vars.find(from) ;

  // Check that we found it
  if (!var) {
    coutE(InputArguments) << "RooUnBinDataStore::changeObservableName(" << GetName() << " no observable " << from << " in this dataset" << endl ;
    return kTRUE ;
  }

  // Process name change
  var->cleanBranchName() ;
  var->SetName(to) ;  

  return kFALSE ;

}


//_____________________________________________________________________________
RooAbsArg* RooUnBinDataStore::addColumn(RooAbsArg& newVar, Bool_t adjustRange)
{
  // Add a new column to the data set which holds the pre-calculated values
  // of 'newVar'. This operation is only meaningful if 'newVar' is a derived
  // value.
  //
  // The return value points to the added element holding 'newVar's value
  // in the data collection. The element is always the corresponding fundamental
  // type of 'newVar' (e.g. a RooRealVar if 'newVar' is a RooFormulaVar)
  //
  // Note: This function is explicitly NOT intended as a speed optimization
  //       opportunity for the user. Components of complex PDFs that can be
  //       precalculated with the dataset are automatically identified as such
  //       and will be precalculated when fitting to a dataset
  // 
  //       By forcibly precalculating functions with non-trivial Jacobians,
  //       or functions of multiple variables occurring in the data set,
  //       using addColumn(), you may alter the outcome of the fit. 
  //
  //       Only in cases where such a modification of fit behaviour is intentional, 
  //       this function should be used. 

  // Create a fundamental object of the right type to hold newVar values
  RooAbsArg* valHolder= newVar.createFundamental();
  // Sanity check that the holder really is fundamental
  if(!valHolder->isFundamental()) {
    coutE(InputArguments) << GetName() << "::addColumn: holder argument is not fundamental: \""
			  << valHolder->GetName() << "\"" << endl;
    return 0;
  }

  // Clone variable and attach to cloned tree 
  RooAbsArg* newVarClone = newVar.cloneTree() ;
  newVarClone->recursiveRedirectServers(_vars,kFALSE) ;

//   std::vector<Double_t> copyVectData(_vectData); // copy of data vector
//   _vectData.resize(copyVectData.size()+numEntries()); // increase the size of the vector, adding the elements for a new column
 
  // copy data vector (need to add a copy ctor)
  if (!_fitData) return 0; 
  ROOT::Fit::UnBinData * newFitData = new ROOT::Fit::UnBinData(); 
  newFitData->Initialize(_fitData->Size(), _fitData->PointSize() + 1);   // initialize with an extra variable


//   std::vector<Double_t>::iterator copyDataIter = copyVectData.begin();
//   std::vector<Double_t>::iterator vectDataIter = _vectData.begin();
//   while (copyDataIter!=copyVectData.end()) {

  std::vector<double> newVars(_fitData->PointSize() + 1);
     
  for (unsigned int i = 0; i < _fitData->Size(); ++i) { 

     const double * dataIter = _fitData->Coords(i);
     std::vector<Double_t>::iterator newDataIter = newVars.begin();

    _iterator->Reset();
    RooAbsReal *var(0); // Alfio: Read only RooAbsReal for the moment


    while ((var=(RooAbsReal*)_iterator->Next())) {
       // LM: why is compying also inside the valures ???
      var->_value = (*dataIter); var->setValueDirty(); // Alfio: require to add the friend class inside RooAbsReal
      (*newDataIter) = (*dataIter);
      ++newDataIter; ++dataIter;
    }

    newVarClone->syncCache(&_vars) ;
    valHolder->copyCache(newVarClone) ;

    (*newDataIter) = ((RooAbsReal*)valHolder)->getVal();

    //++newDataIter;
    newFitData->Add( &newVars.front() );

  }

  // Attach value place holder
  _vars.add(*valHolder) ;

  // replace the fit data
  delete _fitData; 
  _fitData = newFitData; 

  delete newVarClone ;  
  return valHolder ;

}


//_____________________________________________________________________________
RooArgSet* RooUnBinDataStore::addColumns(const RooArgList& varList)
{
  // Utility function to add multiple columns in one call
  // See addColumn() for details

  TIterator* vIter = varList.createIterator() ;
  RooAbsArg* var ;
  RooArgSet cloneSet;
  RooArgSet *holderSet = new RooArgSet;

  while((var=(RooAbsArg*)vIter->Next())) {
    // Create a fundamental object of the right type to hold newVar values
    RooAbsArg* valHolder = var->createFundamental();

    // Sanity check that the holder really is fundamental
    if(!valHolder->isFundamental()) {
      coutE(InputArguments) << GetName() << "::addColumn: holder argument is not fundamental: \""
           << valHolder->GetName() << "\"" << endl;
      delete vIter;
      delete holderSet;
      return 0;
    }
    
    holderSet->add(*valHolder) ;

    // Clone variable and attach to cloned tree 
    RooAbsArg* newVarClone = var->cloneTree() ;
    newVarClone->recursiveRedirectServers(_vars,kFALSE) ;

    cloneSet.add(*newVarClone) ;

  }

  delete vIter ;

  // copy data vector (need to add a copy ctor)
  if (!_fitData) return 0; 
  ROOT::Fit::UnBinData * newFitData = new ROOT::Fit::UnBinData(); 
  int nextra = holderSet->getSize(); // extra variables
  newFitData->Initialize(_fitData->Size(), _fitData->PointSize() + nextra);   // initialize with an extra variable


  std::vector<double> newVars(_fitData->PointSize() + nextra);
     

//   std::vector<Double_t> copyVectData(_vectData); // copy of data vector
//   _vectData.resize(copyVectData.size()+numEntries()*holderSet->getSize()); // increase the size of the vector, adding the elements for new columns

//   std::vector<Double_t>::iterator copyDataIter = copyVectData.begin();
//   std::vector<Double_t>::iterator vectDataIter = _vectData.begin();

  TIterator* cIter = cloneSet.createIterator() ;
  TIterator* hIter = holderSet->createIterator() ;
  RooAbsArg *cloneArg, *holder ;

  for (unsigned int i = 0; i < _fitData->Size(); ++i) { 

     //while (copyDataIter!=copyVectData.end()) {


     const double * dataIter = _fitData->Coords(i);
     std::vector<Double_t>::iterator newDataIter = newVars.begin();

    _iterator->Reset();
    RooAbsReal *var(0); // Alfio: Read only RooAbsReal for the moment


    while ((var=(RooAbsReal*)_iterator->Next())) {
       // LM: why is comying also inside the values ???
      var->_value = (*dataIter); var->setValueDirty(); // Alfio: require to add the friend class inside RooAbsReal
      (*newDataIter) = (*dataIter);
      ++newDataIter; ++dataIter;
    }


    cIter->Reset() ;
    hIter->Reset() ;
    while((cloneArg=(RooAbsArg*)cIter->Next())) {
      holder = (RooAbsArg*)hIter->Next() ;

      cloneArg->syncCache(&_vars) ;
      holder->copyCache(cloneArg) ;

      (*newDataIter) = ((RooAbsReal*)holder)->getVal();

      ++newDataIter;

    }

    newFitData->Add( &newVars.front() );

  }
  
  // Attach value place holder
  _vars.add(*holderSet) ;

  delete cIter ;
  delete hIter ;
  
  // replace the fit data
  delete _fitData; 
  _fitData = newFitData; 


  cloneSet.removeAll();
  return holderSet;

}


//_____________________________________________________________________________
RooAbsDataStore* RooUnBinDataStore::merge(const RooArgSet& allVars, list<RooAbsDataStore*> dstoreList)
{
  // Merge columns of supplied data set(s) with this data set.  All
  // data sets must have equal number of entries.  In case of
  // duplicate columns the column of the last dataset in the list
  // prevails
    
  RooUnBinDataStore* mergedStore = new RooUnBinDataStore("merged","merged",allVars) ;
  Int_t nevents = dstoreList.front()->numEntries() ;
  if (!mergedStore->_fitData) mergedStore->_fitData = new ROOT::Fit::UnBinData();
  mergedStore->_fitData->Initialize(nevents,allVars.getSize());

  for (int i=0 ; i<nevents ; i++) {

    // Copy data from self
    mergedStore->_vars = *get(i) ;
      
    // Copy variables from merge sets
    for (list<RooAbsDataStore*>::iterator iter = dstoreList.begin() ; iter!=dstoreList.end() ; iter++) {
      const RooArgSet* partSet = (*iter)->get(i) ;
      mergedStore->_vars = *partSet ;
    }

    mergedStore->fill() ;
    
  }
  return mergedStore ;
}


//_____________________________________________________________________________
void RooUnBinDataStore::append(RooAbsDataStore& other) 
{
  Int_t nevents = other.numEntries() ;
  //_vectData.reserve(_vectData.size()+nevents*_vars.getSize()); // reserve more elements
  if (!_fitData) _fitData = new ROOT::Fit::UnBinData();
  _fitData->Initialize(_fitData->Size()+ nevents, _vars.getSize()); // reserve more elements
  for (int i=0 ; i<nevents; i++) {  
    _vars = *other.get(i) ;
    fill() ;
  }
}
