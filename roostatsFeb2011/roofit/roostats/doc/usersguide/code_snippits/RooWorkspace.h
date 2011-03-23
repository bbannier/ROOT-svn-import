class RooWorkspace : public TNamed {
  // ... a relevant subset of RooWorkspace public methods
  // Write this workspace to a ROOT file
  Bool_t writeToFile(const char* fileName, Bool_t recreate=kTRUE) ;

  // use a low-level factory to create and edit objects  in the workspace
  RooAbsArg* factory(const char* expr) ;

  // Accessor functions 
  RooAbsPdf*       pdf(const char* name) const ;
  RooAbsData*      data(const char* name) const ;
  RooRealVar*      var(const char* name) const ;
  const RooArgSet* set(const char* name) ;
  RooAbsReal*      function(const char* name) const ;
  RooCategory*     cat(const char* name) const ;
  RooAbsCategory*  catfunc(const char* name) const ;
  RooAbsArg*       arg(const char* name) const ;
  const RooArgSet& components() const ;
  TIterator*       componentIterator() const ;
  TObject*         obj(const char* name) const ;

  // Print contents of the workspace
  void Print(Option_t* opts=0) const ;

  // Import functions for dataset, functions, generic objects
  Bool_t import(const RooAbsArg& arg, const RooCmdArg& arg1=RooCmdArg(),...) ;
  Bool_t import(const RooArgSet& args, const RooCmdArg& arg1=RooCmdArg(),...);
  Bool_t import(RooAbsData& data, const RooCmdArg& arg1=RooCmdArg(),...);
  Bool_t import(const char *fileSpec, const RooCmdArg& arg1=RooCmdArg(),...);
  Bool_t import(TObject& object, Bool_t replaceExisting=kFALSE) ;

  // import class code for custom classes (eg. custom PDFs and fuctions)
  static void autoImportClassCode(Bool_t flag) ;

  // define a RooArgSet with a name
  Bool_t defineSet(const char* name, const RooArgSet& aset, ...);

  // Import, load and save parameter value snapshots
  Bool_t saveSnapshot(const char* name, const RooArgSet& params, 
		      Bool_t importValues=kFALSE) ;
  Bool_t loadSnapshot(const char* name) ;  

  // Group accessors
  RooArgSet allVars() const;
  RooArgSet allCats() const ;
  RooArgSet allFunctions() const ;
  RooArgSet allPdfs() const ;
  std::list<RooAbsData*> allData() const ;
  std::list<TObject*> allGenericObjects() const ;
} ;
