// @(#)root/roostats:$Id$
// Author: Danilo Piparo   25/08/2009

/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//___________________________________________________
/*
BEGIN_HTML
<p>
HLFactory is an High Level model Factory allows you to 
describe your models in a configuration file 
(<i>datacards</i>) acting as an interface with the RooFactoryWSTool.
Moreover it provides tools for the combination of models and datasets.
</p>

END_HTML
*/

#include <iostream>
#include <fstream>

#include "RooStats/HLFactory.h"
#include "TFile.h"
#include "TObject.h"
#include "TObjArray.h"
#include "TObjString.h"

#include "RooSimultaneous.h"

ClassImp(RooStats::HLFactory) ;


using namespace RooStats;
using namespace RooFit;

//_______________________________________________________
HLFactory::HLFactory(const char *name,
                     const char *fileName,
                     bool isVerbose):
    TNamed(name,name),
    fComboCat(0),
    fComboBkgPdf(0),
    fComboSigBkgPdf(0),
    fComboDataset(0),
    fCombinationDone(false),
    fVerbose(isVerbose),
    fInclusionLevel(0),
    fOwnWs(true){
    // Constructor with the name of the config file to interpret and the 
    // verbosity flag. The extension for the config files is assumed to 
    // be ".rs".

    TString wsName(fileName);
    wsName.ReplaceAll(".rs","");
    fWs = new RooWorkspace(wsName,true);

    fSigBkgPdfNames.SetOwner();
    fBkgPdfNames.SetOwner();
    fDatasetsNames.SetOwner();

    // Start the parsing
    fReadFile(fileName);
}

//_______________________________________________________
HLFactory::HLFactory(const char* name,
                     RooWorkspace* externalWs,
                     bool isVerbose):
    TNamed(name,name),
    fComboCat(0),
    fComboBkgPdf(0),
    fComboSigBkgPdf(0),
    fComboDataset(0),
    fCombinationDone(false),
    fVerbose(isVerbose),
    fInclusionLevel(0),
    fOwnWs(false){
    // Constructor without a card but with an exrernal workspace.

    fWs=externalWs;
    fSigBkgPdfNames.SetOwner();
    fBkgPdfNames.SetOwner();
    fDatasetsNames.SetOwner();

}

//_______________________________________________________
HLFactory::HLFactory():
    TNamed("empty","empty"),
    fComboCat(0),
    fComboBkgPdf(0),
    fComboSigBkgPdf(0),
    fComboDataset(0),
    fCombinationDone(false),
    fVerbose(false),
    fInclusionLevel(0),
    fWs(0),
    fOwnWs(false){
    }

//_______________________________________________________
HLFactory::~HLFactory(){
    // destructor

    if (fComboSigBkgPdf!=NULL)
        delete fComboSigBkgPdf;
    if (fComboBkgPdf!=NULL)
        delete fComboBkgPdf;
    if (fComboDataset!=NULL)
        delete fComboDataset;
    if (fComboCat!=NULL)
        delete fComboCat;

    if (fOwnWs)
        delete fWs;
}

//_______________________________________________________
int HLFactory::AddChannel(const char* label,
                          const char* SigBkgPdfName,
                          const char* BkgPdfName,
                          const char* DatasetName){
    // Add a channel to the combination. The channel can be specified as:
    //  - A signal plus background pdf
    //  - A background only pdf
    //  - A dataset
    // Once the combination of the pdfs is done, no more channels should be 
    // added.

    if (fCombinationDone){
        std::cerr << "Cannot add anymore channels. "
                  << "Combination already carried out.\n";
        return -1;
        }

    if (SigBkgPdfName!=0){
        if (fWs->pdf(SigBkgPdfName)==NULL){
            std::cerr << "Pdf " << SigBkgPdfName << " not found in workspace!\n";
            return -1;
            }
        TObjString* name = new TObjString(SigBkgPdfName);
        fSigBkgPdfNames.Add(name);
        }

    if (BkgPdfName!=0){
        if (fWs->pdf(BkgPdfName)==NULL){
            std::cerr << "Pdf " << BkgPdfName << " not found in workspace!\n";
            return -1;
            }
        TObjString* name = new TObjString(BkgPdfName);
        fBkgPdfNames.Add(name);
        }

    if (DatasetName!=0){
        if (fWs->data(DatasetName)==NULL){
            std::cerr << "Dataset " << DatasetName << " not found in workspace!\n";
            return -1;
            }
        TObjString* name = new TObjString(DatasetName);
        fDatasetsNames.Add(name);
        }

    if (label!=0){
        TObjString* name = new TObjString(label);
        fLabelsNames.Add(name);
        }
    return 0;

}

//_______________________________________________________
RooAbsPdf* HLFactory::GetTotSigBkgPdf(){
    // Return the combination of the signal plus background channels.
    // The facory owns the object.

    if (fSigBkgPdfNames.GetSize()==0)
        return 0;

    if (fComboSigBkgPdf!=NULL)
        return fComboSigBkgPdf;

    if (not fNamesListsConsistent())
        return NULL;

    if (fSigBkgPdfNames.GetSize()==1){
        TString name(((TObjString*)fSigBkgPdfNames.At(0))->String());
        fComboSigBkgPdf=fWs->pdf(name);
        return fComboSigBkgPdf;
        }

    if (not fCombinationDone)
        fCreateCategory();

    RooArgList pdfs("pdfs");

    TIterator* it=fSigBkgPdfNames.MakeIterator();
    TObjString* ostring;
    TObject* obj;
    while ((obj = it->Next())){
        ostring=(TObjString*) obj;
        pdfs.add( *(fWs->pdf(ostring->String())) );
        }
    delete it;

    TString name(GetName());
    name+="_sigbkg";

    TString title(GetName());
    title+="_sigbkg";

    fComboSigBkgPdf=
      new RooSimultaneous(name,
                          title,
                          pdfs,
                          *fComboCat);

    return fComboSigBkgPdf;

    }
//_______________________________________________________
RooAbsPdf* HLFactory::GetTotBkgPdf(){
    // Return the combination of the background only channels.
    // If no background channel is specified a NULL pointer is returned.
    // The facory owns the object.

    if (fBkgPdfNames.GetSize()==0)
        return 0;

    if (fComboBkgPdf!=NULL)
        return fComboBkgPdf;

    if (not fNamesListsConsistent())
        return NULL;

    if (fBkgPdfNames.GetSize()==1){
        fComboBkgPdf=fWs->pdf(((TObjString*)fBkgPdfNames.First())->String());
        return fComboBkgPdf;
        }

    if (not fCombinationDone)
        fCreateCategory();

    RooArgList pdfs("pdfs");

    TIterator* it = fBkgPdfNames.MakeIterator();
    TObjString* ostring;
    TObject* obj;
    while ((obj = it->Next())){
        ostring=(TObjString*) obj;
        pdfs.add( *(fWs->pdf(ostring->String())) );
        }

    TString name(GetName());
    name+="_bkg";

    TString title(GetName());
    title+="_bkg";

    fComboBkgPdf=
      new RooSimultaneous(name,
                          title,
                          pdfs,
                          *fComboCat);

    return fComboBkgPdf;

}

//_______________________________________________________
RooDataSet* HLFactory::GetTotDataSet(){
   // Return the combination of the datasets.
   // If no dataset is specified a NULL pointer is returned.
   // The facory owns the object.

    if (fDatasetsNames.GetSize()==0)
        return 0;

    if (fComboDataset!=NULL)
        return fComboDataset;

    if (not fNamesListsConsistent())
        return NULL;

    if (fDatasetsNames.GetSize()==1){
        fComboDataset=(RooDataSet*)fWs->data(((TObjString*)fDatasetsNames.First())->String());
        return fComboDataset;
        }

    if (not fCombinationDone)
        fCreateCategory();

    TIterator* it = fDatasetsNames.MakeIterator();
    TObjString* ostring;
    TObject* obj = it->Next();
    ostring = (TObjString*) obj;
    fComboDataset = (RooDataSet*) fWs->data(ostring->String()) ;
    fComboDataset->Print();
    TString dataname(GetName());
    fComboDataset = new RooDataSet(*fComboDataset,dataname+"_TotData");
    int catindex=0;
    fComboCat->setIndex(catindex);
    fComboDataset->addColumn(*fComboCat);
    while ((obj = it->Next())){
        ostring=(TObjString*) obj;
        catindex++;
        RooDataSet* dummy = new RooDataSet(*(RooDataSet*)fWs->data(ostring->String()),"");
        fComboCat->setIndex(catindex);
        fComboCat->Print();
        dummy->addColumn(*fComboCat);
        fComboDataset->append(*dummy);
        delete dummy;
        }

    delete it;
    return fComboDataset;

}

//_______________________________________________________
RooCategory* HLFactory::GetTotCategory(){
   // Return the category.
   // The facory owns the object.

    if (fComboCat!=NULL)
        return fComboCat;

    if (not fNamesListsConsistent())
        return NULL;

    if (not fCombinationDone)
        fCreateCategory();

    return fComboCat;

    }

//_______________________________________________________
int HLFactory::ProcessCard(const char* filename){
    // Process an additional configuration file
    return fReadFile(filename,0);
    }

//_______________________________________________________
int HLFactory::fReadFile(const char*fileName, bool is_included){
    // Parses the configuration file. The objects can be specified following 
    // the rules of the RooFactoryWSTool, plus some more flexibility.
    //
    // The expected format for the datacards is ".rs".
    //
    // All the instructions end with a ";" (like in C++).
    //
    // Carriage returns and white lines are irrelevant (like in C++).
    //
    // The (Roo)ClassName::objname(description) can be replaced with the more
    // "pythonic" objname = (Roo)ClassName(description).
    //
    // The comments can be specified with // if on a single line or with /* */
    // if on multiple lines (like in C++).
    //
    // The "#include path/to/file.rs" statement triggers the inclusion of a 
    // configuration fragment.
    //
    // The "echo" statement prompts a message on screen.
    //

    // Check the deepness of the inclusion
    if (is_included) 
        fInclusionLevel+=1;
    else
        fInclusionLevel=0;

    const int maxDeepness=50;
    if (fInclusionLevel>maxDeepness)
        std::cerr << "The inclusion stack is deeper than " << maxDeepness 
                  << ". Is this a recursive inclusion?\n";


    // open the config file and go through it
    std::ifstream ifile(fileName);

    if(ifile.fail()){
        std::cerr<<"\nFile "<< fileName << " could not be opened.\n";
        return -1;
        }

    TString ifileContent("");
    ifileContent.ReadFile(ifile);
    ifile.close();

    // Strip the commented lines
    TString ifileContentStripped("");

    TObjArray* lines_array = ifileContent.Tokenize("\n");
    TIterator* lineIt=lines_array->MakeIterator();
    bool in_comment=false;
    TString line;
    TObject* line_o;

    while((line_o=(*lineIt)())){
        line = (static_cast<TObjString*>(line_o))->GetString();

        // Are we in a multiline comment?
        if (in_comment)
            if (line.EndsWith("*/")){
                in_comment=false;
                if (fVerbose) std::cout << "Out of comment /* */ ..." << std::endl;
                continue;
                }

        // Was line a single line comment?
        if (line.BeginsWith("/*") and line.EndsWith("*/") or 
            line.BeginsWith("//")){
            if (fVerbose) std::cout << "In single line comment..." << std::endl;
            continue;
            }

        // Did a multiline comment just begun?
        if (line.BeginsWith("/*")){
            in_comment=true;
            if (fVerbose) std::cout << "In comment /* */ ..." << std::endl;
            continue;
            }

        ifileContentStripped+=line+"\n";

        }

    delete lines_array;
    delete lineIt;

    lines_array = ifileContentStripped.Tokenize(";");
    lineIt=lines_array->MakeIterator();
    in_comment=false;

    const int nNeutrals=2;
    TString neutrals[nNeutrals]={"\t"," "};

    while((line_o=(*lineIt)())){

        line = (static_cast<TObjString*>(line_o))->GetString();

        line.Strip(TString::kBoth,' ');

        line.ReplaceAll("\n","");

        // Do we have an echo statement?
        if (line.BeginsWith("echo")){
            line = line(5,line.Length()-1);
            if (fVerbose) std::cout << "Echoing line " << line.Data() << std::endl;
            std::cout << "[" << GetName() << "] echo: " << line.Data() << std::endl;
            continue;
            }

        for (int i=0;i<nNeutrals;++i)
            line.ReplaceAll(neutrals[i],"");


        if (fVerbose) std::cout << "Reading -->" << line.Data() << "<--\n";

        // Was line a white space?
        if (line == ""){
            if (fVerbose) std::cout << "Empty line: skipping ..." << std::endl;
            continue;
            }

        // Do we have an include statement?
        if (line.BeginsWith("#include")){
            line.ReplaceAll("#include","");
            if (fVerbose) std::cout << "Reading included file..." << std::endl;
            fReadFile(line,true);
            continue;
            }

        // We parse the line
        if (fVerbose) std::cout << "Parsing the line..." << std::endl;
        fParseLine(line); 
        }

    delete lineIt;
    delete lines_array;

    return 0;
}


//_______________________________________________________
void HLFactory::fCreateCategory(){
    // Builds the category necessary for the mutidimensional models. Its name
    // will be <HLFactory name>_category and the types are specified by the 
    // model labels.

    fCombinationDone=true;

    TString name(GetName());
    name+="_category";

    TString title(GetName());
    title+="_category";

    fComboCat=new RooCategory(name,title);

    TIterator* it=fLabelsNames.MakeIterator();
    TObjString* ostring;
    TObject* obj;
    while ((obj = it->Next())){
        ostring=(TObjString*) obj;
        fComboCat->defineType(ostring->String());
        }

    }

//_______________________________________________________
bool HLFactory::fNamesListsConsistent(){
    // Check the number of entries in each list. If not the same and the list 
    // is not empty prompt an error.

    if ((fSigBkgPdfNames.GetEntries()==fBkgPdfNames.GetEntries() or fBkgPdfNames.GetEntries()==0) and
        (fSigBkgPdfNames.GetEntries()==fDatasetsNames.GetEntries() or fDatasetsNames.GetEntries()==0) and
        (fSigBkgPdfNames.GetEntries()==fLabelsNames.GetEntries() or fLabelsNames.GetEntries()==0))
        return true;
    else{
        std::cerr << "The number of datasets and models added as channels " 
                  << " is not the same!\n";
        return false;
        }
    }

//_______________________________________________________
int HLFactory::fParseLine(TString& line){
    // Parse a single line and puts the content in the RooWorkSpace

    if (fVerbose) std::cout << "\nParsing line: " << line.Data() << std::endl;

    TString new_line("");

    const int nequals = line.CountChar('=');
    if (nequals==1 or 
        (nequals>1 and line.Contains("SIMUL"))){ //build with the factory a pdf

        const int equal_index=line.First('=');
        const int par_index=line.First('(');
        TString o_name(line(0,equal_index));
        TString o_class(line(equal_index+1,par_index-equal_index-1));
        TString o_descr(line(par_index+1,line.Length()-par_index-2));

        if (fVerbose) std::cout << "\no_name=" << o_name.Data()
                                << " o_class=" << o_class.Data()
                                << " o_descr=" << o_descr.Data() << "\n\n";


        if (o_class.BeginsWith("import")){// import a generic TObject, for the moment a dataset in a WSpace
            o_class.ReplaceAll("import","");

            if (o_class!="DataSet"){
                std::cerr << "Import syntax not recognised...\n";
                return -1;
                }

            o_descr.ReplaceAll("(","");
            o_descr.ReplaceAll(")","");
            TString ifilename("");
            TString objname("");
            TString objinwsname("");
            TObjArray* lines_array = o_descr.Tokenize(",");
            const int n_lines=lines_array->GetEntries();

            if (n_lines != 3){
                std::cerr << "Import syntax not recognised...\n";
                return -1;
                }

            ifilename=static_cast<TObjString*>((*lines_array)[0])->GetString();
            objname=static_cast<TObjString*>((*lines_array)[1])->GetString();
            objinwsname=static_cast<TObjString*>((*lines_array)[2])->GetString();

            delete lines_array;

            // Open the file
            if (fVerbose) std::cout << "Opening " << ifilename.Data() << " ...\n";
            TFile* ifile=TFile::Open(ifilename);
            if (ifile==NULL){
                std::cerr << "Could not open " << ifilename.Data() << std::endl;
                return -1;
                }

            // Getting the object: it can well be a Workspace
            if (fVerbose) std::cout << "Getting " << objname.Data() << " from " 
                                    << ifilename.Data() << " ...\n";

            TObject* obj=ifile->Get(objname);

            //obj->Print();

            if (obj==NULL){
                std::cerr << "Could not find " << objname.Data() << " in file " 
                          <<  ifilename.Data() << std::endl;
                return -1;
                }

            if (fVerbose) std::cout << "Adding to workspace " 
                                    << objinwsname.Data() << " ...\n";

        // FIXME when generic TObject addition is in there AND RooWorkspace::merge
            TString classname(obj->ClassName());
            if (classname!="RooWorkspace"){
                std::cerr << "Object is not a RooWorkspace!\n";
                return -1;
                }

            RooDataSet data(*(RooDataSet*)(((RooWorkspace*)obj)->data(objinwsname)),o_name);
            fWs->import(data);
            return 0;
            }

        new_line=o_class+"::"+o_name+"("+o_descr+")";

        if (fVerbose){
            std::cout << "DEBUG: line: " << line.Data() << std::endl;
            std::cout << "DEBUG: new_line: " << new_line.Data() << std::endl;
            }

        fWs->factory(new_line);

        return 0;
        }

    else{//build with the factory a var or cat.
        new_line=line;
        fWs->factory(new_line);
        }

    return 0;

}






