/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 * @(#)root/roofitcore:$Id$
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/

// -- CLASS DESCRIPTION [MISC] --
// The RooWorkspace is a persistable container for RooFit projects. A workspace
// can contain and own variables, p.d.f.s, functions and datasets. All objects
// that live in the workspace are owned by the workspace. The import() method
// enforces consistency of objects upon insertion into the workspace (e.g. no
// duplicate object with the same name are allowed) and makes sure all objects
// in the workspace are connected to each other. Easy accessor methods like
// pdf(), var() and data() allow to refer to the contents of the workspace by
// object name. The entire RooWorkspace can be saved into a ROOT TFile and organises
// the consistent streaming of its contents without duplication.

#include "RooFit.h"
#include "RooWorkspace.h"
#include "RooAbsPdf.h"
#include "RooRealVar.h"
#include "RooCategory.h"
#include "RooAbsData.h"
#include "RooCmdConfig.h"
#include "RooMsgService.h"
#include "TInterpreter.h"
#include "TClassTable.h"
#include "TBaseClass.h"
#include "TSystem.h"
#include "TRegexp.h"
#include <map>
#include <string>
#include <list>
using namespace std ;

#include "TClass.h"
#include "Riostream.h"
#include <string.h>
#include <assert.h>

ClassImp(RooWorkspace)
;
ClassImp(RooWorkspace::CodeRepo)
;
ClassImp(RooWorkspace::WSDir)
;

list<string> RooWorkspace::_classDeclDirList ;
list<string> RooWorkspace::_classImplDirList ;
string RooWorkspace::_classFileExportDir = ".wscode.%s" ;
Bool_t RooWorkspace::_autoClass = kFALSE ;

void RooWorkspace::addClassDeclImportDir(const char* dir) 
{
  _classDeclDirList.push_back(dir) ;
}

void RooWorkspace::addClassImplImportDir(const char* dir) 
{
  _classImplDirList.push_back(dir) ;
}

void RooWorkspace::setClassFileExportDir(const char* dir) 
{
  if (dir) {
    _classFileExportDir = dir ;
  } else {
    _classFileExportDir = ".wscode.%s" ;
  }
}

void RooWorkspace::autoImportClassCode(Bool_t flag) 
{
  _autoClass = flag ; 
}


RooWorkspace::RooWorkspace() : _classes(this), _dir(0)
{
  // Default constructor
}


RooWorkspace::RooWorkspace(const char* name, const char* title) : TNamed(name,title?title:name), _classes(this), _dir(0)
// Empty workspace constructor
{
}


RooWorkspace::RooWorkspace(const RooWorkspace& other) : TNamed(other), _classes(this), _dir(0)
{
  // Workspace copy constructor
  other._allOwnedNodes.snapshot(_allOwnedNodes,kTRUE) ;
  TIterator* iter = other._dataList.MakeIterator() ;
  TObject* data ;
  while((data=iter->Next())) {
    _dataList.Add(data->Clone()) ;
  }
  delete iter ;
}


RooWorkspace::~RooWorkspace() 
{
  // Workspace destructor
  _dataList.Delete() ;
  if (_dir) {
    delete _dir ;
  }
}


Bool_t RooWorkspace::import(const RooAbsArg& arg, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3) 
{
  //  Import a RooAbsArg object, e.g. function, p.d.f or variable into the workspace. This import function clones the input argument and will
  //  own the clone. If a composite object is offered for import, e.g. a p.d.f with parameters and observables, the
  //  complete tree of objects is imported. If any of the _variables_ of a composite object (parameters/observables) are already 
  //  in the workspace the imported p.d.f. is connected to the already existing variables. If any of the _function_ objects (p.d.f, formulas) 
  //  to be imported already exists in the workspace an error message is printed and the import of the entire tree of objects is cancelled. 
  //  Several optional arguments can be provided to modify the import procedure.
  //
  //  Accepted arguments
  //  -------------------------------
  //  RenameConflictNodes(const char* suffix) -- Add suffix to branch node name if name conflicts with existing node in workspace
  //  RenameNodes(const char* suffix) -- Add suffix to all branch node names including top level node
  //  RenameVariable(const char* inputName, const char* outputName) -- Rename variable as specified upon import.
  //
  //  The RenameConflictNodes and RenameNodes arguments are mutually exclusive. The RenameVariable argument can be repeated
  //  as often as necessary to rename multiple variables. Alternatively, a single RenameVariable argument can be given with
  //  two comma separated lists.

  RooLinkedList args ;
  args.Add((TObject*)&arg1) ;
  args.Add((TObject*)&arg2) ;
  args.Add((TObject*)&arg3) ;

  // Select the pdf-specific commands 
  RooCmdConfig pc(Form("RooWorkspace::import(%s)",GetName())) ;

  pc.defineString("conflictSuffix","RenameConflictNodes",0) ;
  pc.defineString("allSuffix","RenameAllNodes",0) ;
  pc.defineString("varChangeIn","RenameVar",0,"",kTRUE) ;
  pc.defineString("varChangeOut","RenameVar",1,"",kTRUE) ;
  pc.defineMutex("RenameConflictNodes","RenameAllNodes") ;

  // Process and check varargs 
  pc.process(args) ;
  if (!pc.ok(kTRUE)) {
    return kTRUE ;
  }

  // Decode renaming logic into suffix string and boolean for conflictOnly mode
  const char* suffixC = pc.getString("conflictSuffix") ;
  const char* suffixA = pc.getString("allSuffix") ;
  const char* varChangeIn = pc.getString("varChangeIn") ;
  const char* varChangeOut = pc.getString("varChangeOut") ;

  // Turn zero length strings into null pointers 
  if (suffixC && strlen(suffixC)==0) suffixC = 0 ;
  if (suffixA && strlen(suffixA)==0) suffixA = 0 ;

  Bool_t conflictOnly = suffixA ? kFALSE : kTRUE ;
  const char* suffix = suffixA ? suffixA : suffixC ;
  
  // Scan for overlaps with current contents
  if (!suffix && _allOwnedNodes.find(arg.GetName())) {
    coutE(ObjectHandling) << "RooWorkSpace::import(" << GetName() << ") ERROR importing object named " << arg.GetName() 
			 << ": already in the workspace and no conflict resolution protocol specified" << endl ;
    return kTRUE ;    
  }

  // Make list of conflicting nodes
  RooArgSet conflictNodes ;
  RooArgSet branchSet ;
  arg.branchNodeServerList(&branchSet) ;
  TIterator* iter = branchSet.createIterator() ;
  RooAbsArg* branch ;
  while ((branch=(RooAbsArg*)iter->Next())) {
    if (_allOwnedNodes.find(branch->GetName())) {
      conflictNodes.add(*branch) ;
    }
  }
  delete iter ;
  
  // Terminate here if there are conflicts and no resolution protocol
  if (conflictNodes.getSize()>0 && !suffix) {
      coutE(ObjectHandling) << "RooWorkSpace::import(" << GetName() << ") ERROR object named " << arg.GetName() << ": component(s) " 
	   << conflictNodes << " already in the workspace and no conflict resolution protocol specified" << endl ;      
      return kTRUE ;
  }
    
  // Now create a working copy of the incoming object tree
  RooArgSet* cloneSet = (RooArgSet*) RooArgSet(arg).snapshot(kTRUE) ;
  RooAbsArg* cloneTop = cloneSet->find(arg.GetName()) ;

  // Mark all nodes for renaming if we are not in conflictOnly mode
  if (!conflictOnly) {
    conflictNodes.removeAll() ;
    conflictNodes.add(branchSet) ;
  }

  // Mark nodes that are to be renamed with special attribute
  TIterator* citer = conflictNodes.createIterator() ;
  string topName2 = cloneTop->GetName() ;
  RooAbsArg* cnode ;
  while ((cnode=(RooAbsArg*)citer->Next())) {
    RooAbsArg* cnode2 = cloneSet->find(cnode->GetName()) ;
    string origName = cnode2->GetName() ;
    cnode2->SetName(Form("%s_%s",cnode2->GetName(),suffix)) ;
    cnode2->SetTitle(Form("%s (%s)",cnode2->GetTitle(),suffix)) ;
    string tag = Form("ORIGNAME:%s",origName.c_str()) ;
    cnode2->setAttribute(tag.c_str()) ;

    // Save name of new top level node for later use
    if (cnode2==cloneTop) {
      topName2 = cnode2->GetName() ;
    }

    coutI(ObjectHandling) << "RooWorkspace::import(" << GetName() 
		       << ") Resolving name conflict in workspace by changing name of imported node  " 
		       << origName << " to " << cnode2->GetName() << endl ;
  }  
  delete citer ;

  // Process any change in variable names 
  if (strlen(varChangeIn)>0) {
    
    // Parse comma separated lists into map<string,string>
    char tmp[1024] ;
    strcpy(tmp,varChangeIn) ;
    list<string> tmpIn,tmpOut ;
    char* ptr = strtok(tmp,",") ;
    while (ptr) {
      tmpIn.push_back(ptr) ;
      ptr = strtok(0,",") ;
    }
    strcpy(tmp,varChangeOut) ;
    ptr = strtok(tmp,",") ;
    while (ptr) {
      tmpOut.push_back(ptr) ;
      ptr = strtok(0,",") ;
    }    
    map<string,string> varMap ;
    list<string>::iterator iin = tmpIn.begin() ;
    list<string>::iterator iout = tmpOut.begin() ;
    for (;iin!=tmpIn.end() ; ++iin,++iout) {
      varMap[*iin]=*iout ;
    }       
    
    // Process all changes in variable names
    TIterator* cliter = cloneSet->createIterator() ;
    while ((cnode=(RooAbsArg*)cliter->Next())) {
      
      if (varMap.find(cnode->GetName())!=varMap.end()) { 	
	string origName = cnode->GetName() ;
	cnode->SetName(varMap[cnode->GetName()].c_str()) ;
	string tag = Form("ORIGNAME:%s",origName.c_str()) ;
	cnode->setAttribute(tag.c_str()) ;
	coutI(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") Changing name of variable " 
			   << origName << " to " << cnode->GetName() << " on request" << endl ;
      }    
    }
    delete cliter ;
  }
  
  // Now clone again with renaming effective
  RooArgSet* cloneSet2 = (RooArgSet*) RooArgSet(*cloneTop).snapshot(kTRUE) ;
  RooAbsArg* cloneTop2 = cloneSet2->find(topName2.c_str()) ;

  // Make final check list of conflicting nodes
  RooArgSet conflictNodes2 ;
  RooArgSet branchSet2 ;
  arg.branchNodeServerList(&branchSet) ;
  TIterator* iter2 = branchSet2.createIterator() ;
  RooAbsArg* branch2 ;
  while ((branch2=(RooAbsArg*)iter2->Next())) {
    if (_allOwnedNodes.find(branch2->GetName())) {
      conflictNodes2.add(*branch2) ;
    }
  }
  delete iter2 ;

  // Terminate here if there are conflicts and no resolution protocol
  if (conflictNodes2.getSize()) {
    coutE(ObjectHandling) << "RooWorkSpace::import(" << GetName() << ") ERROR object named " << arg.GetName() << ": component(s) " 
			  << conflictNodes2 << " cause naming conflict after conflict resolution protocol was executed" << endl ;      
    return kTRUE ;
  }
    
  // Print a message for each imported node
  iter = cloneSet2->createIterator() ;
  RooAbsArg* node ;
  RooArgSet recycledNodes ;
  while((node=(RooAbsArg*)iter->Next())) {

    if (_autoClass) {
      if (!_classes.autoImportClass(node->IsA())) {
	coutW(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") WARNING: problems import class code of object " 
			      << node->IsA()->GetName() << "::" << node->GetName() << ", reading of workspace will require external definition of class" << endl ;
      }
    }

    // Check if node is already in workspace (can only happen for variables)
    if (_allOwnedNodes.find(node->GetName())) {
      // Do not import node, add not to list of nodes that require reconnection
      coutI(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") using existing copy of variable " << node->IsA()->GetName() 
			 << "::" << node->GetName() << " for import of " << cloneTop2->IsA()->GetName() << "::" 
			 << cloneTop2->GetName() << endl ;      
      recycledNodes.add(*_allOwnedNodes.find(node->GetName())) ;

    } else {
      // Import node
      coutI(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") importing " << node->IsA()->GetName() << "::" 
			 << node->GetName() << endl ;
      _allOwnedNodes.addOwned(*node) ;
      if (_dir) {
	_dir->InternalAppend(node) ;
      }
    }
  }

  // Release working copy
  delete cloneSet ;


  // Reconnect any nodes that need to be
  if (recycledNodes.getSize()>0) {
    iter->Reset() ;
    while((node=(RooAbsArg*)iter->Next())) {
      node->redirectServers(recycledNodes) ;
    }
  }

  delete iter ;
  

  return kFALSE ;
}


Bool_t RooWorkspace::import(RooAbsData& data, const RooCmdArg& arg1, const RooCmdArg& arg2, const RooCmdArg& arg3) 
{
  //  Import a dataset (RooDataSet or RooDataHist) into the work space. The workspace will contain a copy of the data
  //  The dataset and its variables can be renamed upon insertion with the options below
  //
  //  Accepted arguments
  //  -------------------------------
  //  RenameDataset(const char* suffix) -- Rename dataset upon insertion
  //  RenameVariable(const char* inputName, const char* outputName) -- Change names of observables in dataset upon insertion

  coutI(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") importing dataset " << data.GetName() << endl ;

  RooLinkedList args ;
  args.Add((TObject*)&arg1) ;
  args.Add((TObject*)&arg2) ;
  args.Add((TObject*)&arg3) ;

  // Select the pdf-specific commands 
  RooCmdConfig pc(Form("RooWorkspace::import(%s)",GetName())) ;

  pc.defineString("dsetName","RenameDataset",0,"") ;
  pc.defineString("varChangeIn","RenameVar",0,"",kTRUE) ;
  pc.defineString("varChangeOut","RenameVar",1,"",kTRUE) ;

  // Process and check varargs 
  pc.process(args) ;
  if (!pc.ok(kTRUE)) {
    return kTRUE ;
  }

  // Decode renaming logic into suffix string and boolean for conflictOnly mode
  const char* dsetName = pc.getString("dsetName") ;
  const char* varChangeIn = pc.getString("varChangeIn") ;
  const char* varChangeOut = pc.getString("varChangeOut") ;

  // Transform emtpy string into null pointer
  if (dsetName && strlen(dsetName)==0) {
    dsetName=0 ;
  }

  // Rename dataset if required
  RooAbsData* clone ;
  if (dsetName) {
    coutI(ObjectHandling) << "RooWorkSpace::import(" << GetName() << ") changing name of dataset from  " << data.GetName() << " to " << dsetName << endl ;
    clone = (RooAbsData*) data.Clone(dsetName) ;
  } else {
    clone = (RooAbsData*) data.Clone(data.GetName()) ;
  }


  // Process any change in variable names 
  if (strlen(varChangeIn)>0) {
    
    // Parse comma separated lists of variable name changes
    char tmp[1024] ;
    strcpy(tmp,varChangeIn) ;
    list<string> tmpIn,tmpOut ;
    char* ptr = strtok(tmp,",") ;
    while (ptr) {
      tmpIn.push_back(ptr) ;
      ptr = strtok(0,",") ;
    }
    strcpy(tmp,varChangeOut) ;
    ptr = strtok(tmp,",") ;
    while (ptr) {
      tmpOut.push_back(ptr) ;
      ptr = strtok(0,",") ;
    }    
    list<string>::iterator iin = tmpIn.begin() ;
    list<string>::iterator iout = tmpOut.begin() ;

    for (; iin!=tmpIn.end() ; ++iin,++iout) {
      coutI(ObjectHandling) << "RooWorkSpace::import(" << GetName() << ") changing name of dataset observable " << *iin << " to " << *iout << endl ;
      clone->changeObservableName(iin->c_str(),iout->c_str()) ;
    }
  }

  // Now import the dataset observables
  TIterator* iter = clone->get()->createIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    import(*arg) ;
  }
  delete iter ;
    
  _dataList.Add(clone) ;
  return kFALSE ;
}


Bool_t RooWorkspace::importClassCode(TClass* theClass, Bool_t doReplace) 
{
  return _classes.autoImportClass(theClass,doReplace) ;
}


Bool_t RooWorkspace::importClassCode(const char* pat, Bool_t doReplace)  
{
  Bool_t ret(kTRUE) ;

  TRegexp re(pat,kTRUE) ;
  TIterator* iter = componentIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    TString className = arg->IsA()->GetName() ;
    if (className.Index(re)>=0 && !_classes.autoImportClass(arg->IsA(),doReplace)) {
      coutW(ObjectHandling) << "RooWorkspace::import(" << GetName() << ") WARNING: problems import class code of object " 
			    << arg->IsA()->GetName() << "::" << arg->GetName() << ", reading of workspace will require external definition of class" << endl ;
      ret = kFALSE ;
    }
  }  
  delete iter ;
  return ret ;
}


Bool_t RooWorkspace::merge(const RooWorkspace& /*other*/) 
{
  // Stub for merge function with another workspace (not implemented yet)
  return kFALSE ;
}


Bool_t RooWorkspace::join(const RooWorkspace& /*other*/) 
{
  // Stub for join function with another workspace (not implemented yet)
  return kFALSE ;
}

RooAbsPdf* RooWorkspace::pdf(const char* name) 
{ 
  // Retrieve p.d.f (RooAbsPdf) with given name. A null pointer is returned if not found
  return dynamic_cast<RooAbsPdf*>(_allOwnedNodes.find(name)) ; 
}

RooAbsReal* RooWorkspace::function(const char* name) 
{ 
  // Retrieve function (RooAbsReal) with given name. Note that all RooAbsPdfs are also RooAbsReals. A null pointer is returned if not found.
  return dynamic_cast<RooAbsReal*>(_allOwnedNodes.find(name)) ; 
}

RooRealVar* RooWorkspace::var(const char* name) 
{ 
  // Retrieve real-valued variable (RooRealVar) with given name. A null pointer is returned if not found
  return dynamic_cast<RooRealVar*>(_allOwnedNodes.find(name)) ; 
}

RooCategory* RooWorkspace::cat(const char* name) 
{ 
  // Retrieve discrete variable (RooCategory) with given name. A null pointer is returned if not found
  return dynamic_cast<RooCategory*>(_allOwnedNodes.find(name)) ; 
}

RooAbsData* RooWorkspace::data(const char* name) 
{
  // Retrieve dataset (binned or unbinned) with given name. A null pointer is returned if not found
  return (RooAbsData*)_dataList.FindObject(name) ;
}


// RooModelView* RooWorkspace::addView(const char* name, const RooArgSet& observables) 
// {
//   RooModelView* newView = new RooModelView(*this,observables,name,name) ;
//   _views.Add(newView) ;
//   return newView ;
// }


// RooModelView* RooWorkspace::view(const char* name) 
// {
//   return (RooModelView*) _views.FindObject(name) ;
// }


// void RooWorkspace::removeView(const char* /*name*/) 
// {
// }


Bool_t RooWorkspace::CodeRepo::autoImportClass(TClass* tc, Bool_t doReplace) 
{
  oocxcoutD(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo(" << _wspace->GetName() << ") request to import code of class " << tc->GetName() << endl ;

  // Check if we already have the class
  if (!doReplace && _map.find(tc->GetName())!=_map.end()) {
    oocxcoutD(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo(" << _wspace->GetName() << ") code of class " << tc->GetName() << " already imported, skipping" << endl ;
    return kTRUE ;
  }

  string implfile = tc->GetImplFileName() ;
  string declfile = tc->GetDeclFileName() ;

  TString rootsys = gSystem->Getenv("ROOTSYS") ;
  char* implpath = gSystem->ConcatFileName(rootsys.Data(),implfile.c_str()) ;
  if (!gSystem->AccessPathName(implpath)) {
    oocxcoutD(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo(" << _wspace->GetName() << ") code of class " << tc->GetName() << "is in ROOT distribution, skipping " << endl ;
    delete[] implpath ;
    return kTRUE ;
  }
  delete[] implpath ;
  implpath=0 ;

  if (!tc->HasDefaultConstructor()) {
      oocoutW(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() << ") WARNING cannot import class " 
				      << tc->GetName() << " : it cannot be persisted because it doesn't have a default constructor. Please fix " << endl ;
      return kFALSE ;      
  }

  char* declpath = 0 ;
  if (gSystem->AccessPathName(declfile.c_str())) {

    // Check list of additional declaration paths
    list<string>::iterator diter = RooWorkspace::_classDeclDirList.begin() ;
    while(diter!= RooWorkspace::_classDeclDirList.end()) {
      
      declpath = gSystem->ConcatFileName(diter->c_str(),declfile.c_str()) ;      
      if (!gSystem->AccessPathName(declpath)) {
	// found declaration file
	break ;
      }
      // cleanup and continue ;
      delete[] declpath ;
      declpath=0 ;

      ++diter ;
    }
    
    if (!declpath) {
      oocoutW(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() << ") WARNING Cannot access code of class " 
				      << tc->GetName() << " because header file " << declfile << " is not found in current directory nor in $ROOTSYS" ;
      if (_classDeclDirList.size()>0) {
	ooccoutW(_wspace,ObjectHandling) << ", nor in the search path " ;
	diter = RooWorkspace::_classDeclDirList.begin() ;
	while(diter!= RooWorkspace::_classDeclDirList.end()) {
	  if (diter!=RooWorkspace::_classDeclDirList.begin()) {
	    ooccoutW(_wspace,ObjectHandling) << "," ;
	  }
	  ooccoutW(_wspace,ObjectHandling) << diter->c_str() ;
	  ++diter ;
	}
      }
      ooccoutW(_wspace,ObjectHandling) << ". To fix this problem add the required directory to the search "
				       << "path using RooWorkspace::addClassDeclDir(const char* dir)" << endl ;
      
      return kFALSE ;
    }
  }
  
  if (gSystem->AccessPathName(implfile.c_str())) {

    // Check list of additional declaration paths
    list<string>::iterator iiter = RooWorkspace::_classImplDirList.begin() ;
    while(iiter!= RooWorkspace::_classImplDirList.end()) {
      
      implpath = gSystem->ConcatFileName(iiter->c_str(),implfile.c_str()) ;      
      if (!gSystem->AccessPathName(implpath)) {
	// found implementation file
	break ;
      }
      // cleanup and continue ;
      delete[] implpath ;
      implpath=0 ;

      ++iiter ;
    }
     
    if (!implpath) {
      oocoutW(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() << ") WARNING Cannot access code of class " 
				      << tc->GetName() << " because implementation file " << implfile << " is not found in current directory nor in $ROOTSYS" ;
      if (_classDeclDirList.size()>0) {
	ooccoutW(_wspace,ObjectHandling) << ", nor in the search path " ;
	iiter = RooWorkspace::_classImplDirList.begin() ;
	while(iiter!= RooWorkspace::_classImplDirList.end()) {
	  if (iiter!=RooWorkspace::_classImplDirList.begin()) {
	    ooccoutW(_wspace,ObjectHandling) << "," ;
	  }
	  ooccoutW(_wspace,ObjectHandling) << iiter->c_str() ;
	  ++iiter ;
	}
      }
      ooccoutW(_wspace,ObjectHandling) << ". To fix this problem add the required directory to the search "
				       << "path using RooWorkspace::addClassImplDir(const char* dir)" << endl ;    
      return kFALSE ;
    }
  }
  
  char buf[1024] ;
  
  fstream fdecl(declpath?declpath:declfile.c_str()) ;
  if (!fdecl) {
    oocoutE(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() 
				    << ") ERROR opening declaration file " <<  declfile << endl ;
    return kFALSE ;      
  }

  oocoutI(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() 
				  << ") importing code of class " << tc->GetName() 
				  << " from " << (implpath?implpath:implfile.c_str()) 
				  << " and " << (declpath?declpath:declfile.c_str()) << endl ;
  
  
  string decl ;
  while(fdecl.getline(buf,1023)) {
    if (strstr(buf,"#include")) {
      // Process #include statements here
      char tmp[1024] ;
      strcpy(tmp,buf) ;
      strtok(tmp," <\"") ;
      //char* incfile = strtok(0," <\"") ;
    }
    decl += buf ;
    decl += '\n' ;
  }
  
  fstream fimpl(implpath?implpath:implfile.c_str()) ;
  if (!fimpl) {
    oocoutE(_wspace,ObjectHandling) << "RooWorkspace::autoImportClass(" << _wspace->GetName() 
				    << ") ERROR opening implementation file " <<  implfile << endl ;
    return kFALSE ;      
  }
  
  string impl ;
  while(fimpl.getline(buf,1023)) {
    // Process #include statements here

    // Look for include state of self
    Bool_t foundSelfInclude=kFALSE ;
    if (strstr(buf,"#include")) {
      // Process #include statements here
      char tmp[1024] ;
      strcpy(tmp,buf) ;
      strtok(tmp," <\"") ;
      char* incfile = strtok(0," <\"") ;

      if (strstr(incfile,tc->GetName())) {
	foundSelfInclude=kTRUE ;
      }
    } 

    if (foundSelfInclude) {
      // If include of self is found, substitute original include 
      // which may have directory structure with a plain include
      impl += Form("#include \"%s.h\"\n",tc->GetName()) ;
    } else {
      impl += buf ;
      impl += '\n' ;
    }
  }
  
  TString baseNameList ;
  TList* bl = tc->GetListOfBases() ;
  TIterator* iter = bl->MakeIterator() ;
  TBaseClass* base ;
  list<TClass*> bases ;
  while((base=(TBaseClass*)iter->Next())) {
    if (baseNameList.Length()>0) {
      baseNameList += "," ;
    }
    baseNameList += base->GetClassPointer()->GetName() ;
    bases.push_back(base->GetClassPointer()) ;
  }
  
  _map[tc->GetName()]._baseName = baseNameList ;
  _map[tc->GetName()]._hfile = decl ;
  _map[tc->GetName()]._cxxfile = impl ;   
  
  // Recursive store all base classes as well
  list<TClass*>::iterator biter = bases.begin() ;
  while(biter!=bases.end()) {
    autoImportClass(*biter,doReplace) ;
    ++biter ;
  }


  if (implpath) {
    delete[] implpath ;
  }
  if (declpath) {
    delete[] declpath ;
  }

  return kTRUE ;
}

Bool_t RooWorkspace::makeDir() 
{
  if (_dir) return kTRUE ;

  TString name = Form("%sDir",GetName()) ;
  TString title= Form("TDirectory representation of RooWorkspace %s",GetName()) ;
  _dir = new WSDir(name.Data(),title.Data(),this) ;

  TIterator* iter = componentIterator() ;
  RooAbsArg* arg ;
  while((arg=(RooAbsArg*)iter->Next())) {
    _dir->InternalAppend(arg) ;
  }

  return kTRUE ;
}


void RooWorkspace::Print(Option_t* /*opts*/) const 
{
  // Print contents of the workspace 
  cout << endl << "RooWorkspace(" << GetName() << ") " << GetTitle() << " contents" << endl << endl  ;

  RooAbsArg* arg ;

  RooArgSet pdfSet ;
  RooArgSet funcSet ;
  RooArgSet varSet ;

  // Split list of components in pdfs, functions and variables
  TIterator* iter = _allOwnedNodes.createIterator() ;
  while((arg=(RooAbsArg*)iter->Next())) {

    if (arg->IsA()->InheritsFrom(RooAbsPdf::Class())) {
      pdfSet.add(*arg) ;
    }

    if (arg->IsA()->InheritsFrom(RooAbsReal::Class()) && 
	!arg->IsA()->InheritsFrom(RooAbsPdf::Class()) && 
	!arg->IsA()->InheritsFrom(RooRealVar::Class())) {
      funcSet.add(*arg) ;
    }

    if (arg->IsA()->InheritsFrom(RooRealVar::Class())) {
      varSet.add(*arg) ;
    }
    if (arg->IsA()->InheritsFrom(RooCategory::Class())) {
      varSet.add(*arg) ;
    }

  }
  delete iter ;


  if (varSet.getSize()>0) {
    cout << "variables" << endl ;
    cout << "---------" << endl ;
    cout << varSet << endl ;
    cout << endl ;
  }

  if (pdfSet.getSize()>0) {
    cout << "p.d.f.s" << endl ;
    cout << "-------" << endl ;
    iter = pdfSet.createIterator() ;
    while((arg=(RooAbsArg*)iter->Next())) {
      arg->Print() ;
    }
    delete iter ;
    cout << endl ;
  }

  if (funcSet.getSize()>0) {
    cout << "functions" << endl ;
    cout << "--------" << endl ;
    iter = funcSet.createIterator() ;
    while((arg=(RooAbsArg*)iter->Next())) {
      arg->Print() ;
    }
    delete iter ;
    cout << endl ;
  }


  if (_dataList.GetSize()>0) {
    cout << "datasets" << endl ;
    cout << "--------" << endl ;
    iter = _dataList.MakeIterator() ;
    RooAbsData* data ;
    while((data=(RooAbsData*)iter->Next())) {
      cout << data->IsA()->GetName() << "::" << data->GetName() << *data->get() << endl ;
    }
    delete iter ;
    cout << endl ;
  }

  if (_classes.listOfClassNames().size()>0) {
    cout << "embedded class code" << endl ;
    cout << "-------------------" << endl ;
    cout << _classes.listOfClassNames() << endl ;
    cout << endl ;
  }



//   if (_views.GetSize()>0) {
//     cout << "views" << endl ;
//     cout << "-----" << endl ;
//     iter = _views.MakeIterator() ;
//     RooModelView* view ;
//     while((view=(RooModelView*)iter->Next())) {
//       view->Print() ;
//     }
//     delete iter ;
//   }

  return ;
}

void RooWorkspace::CodeRepo::Streamer(TBuffer &R__b)
{
  typedef ::RooWorkspace::CodeRepo thisClass;

   // Stream an object of class RooWorkspace::CodeRepo.
   if (R__b.IsReading()) {

     UInt_t R__s, R__c;
     R__b.ReadVersion(&R__s, &R__c); 
     
     // Stream contents of ClassCode map
     Int_t count(0) ;
     R__b >> count ;
     while(count--) {
       TString name,hfile,cxxfile ;
       name.Streamer(R__b) ;       
       _map[name]._baseName.Streamer(R__b) ;
       _map[name]._hfile.Streamer(R__b) ;
       _map[name]._cxxfile.Streamer(R__b) ;    
     }     
     R__b.CheckByteCount(R__s, R__c, thisClass::IsA());
 
     // Instantiate any classes that are not defined in current session
     _compiledOK = !compileClasses() ;

   } else {
     
     UInt_t R__c;
     R__c = R__b.WriteVersion(thisClass::IsA(), kTRUE);
     
     // Stream contents of ClassCode map
     R__b << _map.size() ;
     map<TString,ClassCode>::iterator iter = _map.begin() ;
     while(iter!=_map.end()) {
       TString key_copy(iter->first) ;
       key_copy.Streamer(R__b) ;
       iter->second._baseName.Streamer(R__b) ;
       iter->second._hfile.Streamer(R__b);
       iter->second._cxxfile.Streamer(R__b);

       ++iter ;
     }
     R__b.SetByteCount(R__c, kTRUE);
     
   }
}


std::string RooWorkspace::CodeRepo::listOfClassNames() const 
{
  string ret ;
  map<TString,ClassCode>::const_iterator iter = _map.begin() ;
  while(iter!=_map.end()) {
    if (ret.size()>0) {
      ret += ", " ;
    }
    ret += iter->first ;    
    ++iter ;
  }  
  
  return ret ;
}


Bool_t RooWorkspace::CodeRepo::compileClasses() 
{

  Bool_t haveDir=kFALSE ;
  string dirName = Form(_classFileExportDir.c_str(),_wspace->GetName()) ;

  // Process all class entries in repository
  map<TString,ClassCode>::iterator iter = _map.begin() ;
  while(iter!=_map.end()) {

    // If class is already known, don't load
    if (gClassTable->GetDict(iter->first.Data())) {
      oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() Embedded class " 
				      << iter->first << " already in ROOT class table, skipping" << endl ;
      ++iter ;
      continue ;
    }

    if (!haveDir) {
      // Make local directory to extract files 
      if (!gSystem->AccessPathName(dirName.c_str())) {
	oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() reusing code export directory " << dirName.c_str() 
					<< " to extract coded embedded in workspace" << endl ;
      } else {
	if (gSystem->MakeDirectory(dirName.c_str())==0) { 
	  oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() creating code export directory " << dirName.c_str() 
					  << " to extract coded embedded in workspace" << endl ;
	} else {
	  oocoutE(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() ERROR creating code export directory " << dirName.c_str() 
					  << " to extract coded embedded in workspace" << endl ;
	  return kFALSE ;
	}
      }
      haveDir=kTRUE ;
    }

    // Check if identical declaration file (header) is already written
    Bool_t needDeclWrite=kTRUE ;
    ifstream ifdecl(Form("%s/%s.h",dirName.c_str(),iter->first.Data())) ;
    if (ifdecl) {
      TString contents ;
      char buf[1024] ;
      while(ifdecl.getline(buf,1024)) {
	contents += buf ;
	contents += "\n" ;
      }      
      UInt_t crcFile = RooAbsArg::crc32(contents.Data()) ;
      UInt_t crcWS   = RooAbsArg::crc32(iter->second._hfile.Data()) ;
      needDeclWrite = (crcFile!=crcWS) ;
    }
  
    // Write declaration file if required 
    if (needDeclWrite) {
      oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() Extracting declaration code of class" << iter->first << endl ;
      ofstream fdecl(Form("%s/%s.h",dirName.c_str(),iter->first.Data())) ;
      if (!fdecl) {
	oocoutE(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() ERROR opening file" 
					<< Form("%s.h",iter->first.Data()) << " for writing" << endl ;
	return kFALSE ;
      }
      fdecl << iter->second._hfile ;
      fdecl.close() ;
    }

    // Check if identical implementation file is already written
    Bool_t needImplWrite=kTRUE ;
    ifstream ifimpl(Form("%s/%s.cxx",dirName.c_str(),iter->first.Data())) ;
    if (ifimpl) {
      TString contents ;
      char buf[1024] ;
      while(ifimpl.getline(buf,1024)) {
	contents += buf ;
	contents += "\n" ;
      }      
      UInt_t crcFile = RooAbsArg::crc32(contents.Data()) ;
      UInt_t crcWS   = RooAbsArg::crc32(iter->second._cxxfile.Data()) ;
      needImplWrite = (crcFile!=crcWS) ;
    }
  
    // Write implementation file if required
    if (needImplWrite) {
      oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() Extracting implementation code of class " << iter->first << endl ;
      ofstream fimpl(Form("%s/%s.cxx",dirName.c_str(),iter->first.Data())) ;
      if (!fimpl) {
	oocoutE(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() ERROR opening file" 
					<< Form("%s.cxx",iter->first.Data()) << " for writing" << endl ;
	return kFALSE ;
      }
      fimpl << iter->second._cxxfile ;
      fimpl.close() ;
    }

    // Compile class
    oocoutI(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() Compiling code of class " << iter->first << endl ;
    Bool_t ok = gSystem->CompileMacro(Form("%s/%s.cxx",dirName.c_str(),iter->first.Data()),"k") ;
    
    if (!ok) {
      oocoutE(_wspace,ObjectHandling) << "RooWorkspace::CodeRepo::compileClasses() ERROR compiling class " << iter->first.Data() << ", to fix this you can do the following: " << endl 
				      << "  1) Fix extracted source code files in directory " << dirName.c_str() << "/" << endl 
				      << "  2) In clean ROOT session compiled fixed classes by hand using '.x " << dirName.c_str() << "/ClassName.cxx+'" << endl
				      << "  3) Reopen file with RooWorkspace with broken source code in UPDATE mode. Access RooWorkspace to force loading of class" << endl
				      << "     Broken instances in workspace will _not_ be compiled, instead precompiled fixed instances will be used." << endl
				      << "  4) Reimport fixed code in workspace using 'RooWorkspace::importClassCode(\"*\",kTRUE)' method, Write() updated workspace to file and close file" << endl
				      << "  5) Reopen file in clean ROOT session to confirm that problems are fixed" << endl ;
	return kFALSE ;
    }
    
    ++iter ;
  }

  return kTRUE ;
}


void RooWorkspace::WSDir::InternalAppend(TObject* obj) 
{
  TDirectory::Append(obj) ; 
}

void RooWorkspace::WSDir::Add(TObject*) 
{
  coutE(ObjectHandling) << "RooWorkspace::WSDir::Add(" << GetName() << ") ERROR: Directory is read-only representation of a RooWorkspace, use RooWorkspace::import() to add objects" << endl ;
} 

void RooWorkspace::WSDir::Append(TObject*) 
{
  coutE(ObjectHandling) << "RooWorkspace::WSDir::Add(" << GetName() << ") ERROR: Directory is read-only representation of a RooWorkspace, use RooWorkspace::import() to add objects" << endl ;
}
