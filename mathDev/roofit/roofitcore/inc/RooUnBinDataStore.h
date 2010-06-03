
#ifndef ROO_UNBIN_DATA_STORE
#define ROO_UNBIN_DATA_STORE

#include "Rtypes.h"
#include "TNamed.h"
#include "TTree.h"

#include "RooArgSet.h"
#include "RooAbsDataStore.h"
#include "RooFormulaVar.h"

#include "Fit/UnBinData.h"

class RooUnBinDataStore : public RooAbsDataStore {
 public:

  RooUnBinDataStore() ; 
  RooUnBinDataStore(TTree* tree, const RooArgSet& vars, const char* wgtVarName=0) ; 

  // Empty ctor
  RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const char* wgtVarName=0) ;
  virtual RooAbsDataStore* clone(const RooArgSet& vars, const char* newname=0) const { return new RooUnBinDataStore(*this,vars,newname) ; }

  // Ctors from TTree
  RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, TTree &tree,
		   const RooFormulaVar& select, const char* wgtVarName=0);

  RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, TTree &tree,
		   const char* selExpr=0, const char* wgtVarName=0);

  // Ctors from DataStore
  RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const RooAbsDataStore& tds, 
		   const RooFormulaVar& select, const char* wgtVarName=0) ;
  RooUnBinDataStore(const char* name, const char* title, const RooArgSet& vars, const RooAbsDataStore& tds, 
		   const char* selExpr=0, const char* wgtVarName=0) ;

  RooUnBinDataStore(const char *name, const char *title, RooAbsDataStore& tds, 
		   const RooArgSet& vars, const RooFormulaVar* cutVar, const char* cutRange,
		   Int_t nStart, Int_t nStop, Bool_t /*copyCache*/, const char* wgtVarName=0) ;
  
  // Copy Ctors
  RooUnBinDataStore(const RooUnBinDataStore& other, const char* newname=0) ;
  RooUnBinDataStore(const RooUnBinDataStore& other, const RooArgSet& vars, const char* newname=0) ;
   virtual ~RooUnBinDataStore();

  // Write current row
  virtual Int_t fill() ;

  // Retrieve a row
  using RooAbsDataStore::get ;
  virtual const RooArgSet* get(Int_t index) const ;

  virtual Double_t weight() const ;
  virtual Double_t weight(Int_t index) const ;
  virtual Double_t weightError(RooAbsData::ErrorType etype=RooAbsData::Poisson) const ;
  virtual void weightError(Double_t& lo, Double_t& hi, RooAbsData::ErrorType etype=RooAbsData::Poisson) const ;
  virtual Bool_t isWeighted() const { return kFALSE; }

  // Change observable name
  virtual Bool_t changeObservableName(const char* from, const char* to) ;
  
  // Add one or more columns
  virtual RooAbsArg* addColumn(RooAbsArg& var, Bool_t adjustRange=kTRUE) ;
  virtual RooArgSet* addColumns(const RooArgList& varList) ;

  // Merge column-wise
  RooAbsDataStore* merge(const RooArgSet& allvars, std::list<RooAbsDataStore*> dstoreList) ;

  // Add rows 
  virtual void append(RooAbsDataStore& other) ;
  
  // General & bookkeeping methods
  virtual Bool_t valid() const ;
  virtual Int_t numEntries() const ;
  virtual void reset() { } // not needed

  // Constant term  optimizer interface
   virtual void cacheArgs(const RooAbsArg* /*owner*/, RooArgSet& /*varSet*/, const RooArgSet* /*nset*/=0) { } // not needed
  virtual const RooAbsArg* cacheOwner() { return 0; } // not needed
   virtual void setArgStatus(const RooArgSet& /*set*/, Bool_t /*active*/) { } // not needed
  virtual void resetCache() { } // not needed

  // Tree access
  TTree& tree() { } // to check!!!! Used inside RooDataSet
  virtual const TTree* tree() const { return 0 ; } 

  void loadValues(const TTree *t, const RooFormulaVar* select=0, const char* rangeName=0, Int_t nStart=0, Int_t nStop=2000000000) ;
  void loadValues(const RooAbsDataStore *tds, const RooFormulaVar* select=0, const char* rangeName=0, Int_t nStart=0, Int_t nStop=2000000000) ;

 protected:

  RooArgSet varsNoWeight(const RooArgSet& allVars, const char* wgtName=0) ;

  void attachCache(const RooAbsArg* newOwner, const RooArgSet& cachedVars) { } // not needed

   ROOT::Fit::UnBinData * _fitData;  //! Data vector for Double_t

  //  RooArgSet _varsww ;      // vars with weight variable
  //  TTree *_tree ;               //! External TTree holding the data points

};

#endif
