#include "utDataSetInfo.h"
using namespace std;
using namespace UnitTesting;
using namespace TMVA;

utDataSetInfo::utDataSetInfo()
{
   name = "name";
   expression = "expression"; 
   title  = "title";
   unit = "unit";
   min = 2.781828;
   max = 3.1416;
   vartype = 'D';
   varcounter = 123;
   normalized = kFALSE;
   external = &max;
   norm = "norm";
   classname = "classname";
   cut1 = "x<1.";
   cut2 = "y>2.";
   splitoption = "splitoption";
   matrix = new TMatrixD();
   histname = "histname";
   histtitle = "histtitle"; 
   weightexpr = "weightexpr";
   event = new Event();
   
   varinfo = VariableInfo( expression, title,  unit, varcounter, vartype, external, min, max, normalized);
   
   vecvarinfo.push_back(varinfo);
   vecvarinfo.push_back(varinfo);
}



void utDataSetInfo::run()
{
  testConstructor();
  testMethods();
}


void utDataSetInfo::testConstructor()
{
  datasetinfo  = new DataSetInfo(name);
  test_(datasetinfo->GetName() == name);
}


void utDataSetInfo::testMethods()
{
  // FIXME:this doesn't work straightforward
  // test_(datasetinfo->GetDataSet() == 0);

 // note: the "constructor"-like call is different from the /standard/ VariableInfo constructor
  datasetinfo->AddVariable(expression, title,  unit, min, max, vartype, normalized, external);
  datasetinfo->AddVariable(varinfo);
  // note: the method has a different argument list than the other two
  datasetinfo->AddTarget(expression, title,  unit, min, max, normalized, external);
  datasetinfo->AddTarget(varinfo);
  datasetinfo->AddSpectator(expression, title,  unit, min, max, vartype, normalized, external);
  datasetinfo->AddSpectator(varinfo);
  datasetinfo->AddClass(classname);

  test_((datasetinfo->GetVariableInfos()).size()  == vecvarinfo.size());
  test_((datasetinfo->GetTargetInfos()).size()    == vecvarinfo.size());
  test_((datasetinfo->GetSpectatorInfos()).size() == vecvarinfo.size());
  
  test_(datasetinfo->GetNVariables()  == vecvarinfo.size());
  test_(datasetinfo->GetNTargets()    == vecvarinfo.size());
  test_(datasetinfo->GetNSpectators() == vecvarinfo.size());

  datasetinfo->SetNormalization(norm);
  test_(datasetinfo->GetNormalization() == norm);
  
  // there is no comparison operator for VariableInfo, this will be broken if the implementation changes
  /*  
      for(vector<VariableInfo>::const_iterator varinf = vecvarinfo.begin(); varinf < vecvarinfo.end(); ++varinf)
      {
      unsigned int index = vecvarinfo.begin() - varinf;
      test_(datasetinfo->GetVariableInfo(index) == varinfo); // doesn't work!
      }
  */
  
  test_(datasetinfo->GetClassNameMaxLength() == classname.Length() );
  test_(datasetinfo->GetNClasses()           == 1);
  test_(datasetinfo->IsSignal(event)         == kFALSE);

  /*
  // classification information
  ClassInfo*         GetClassInfo( Int_t clNum ) const;
  ClassInfo*         GetClassInfo( const TString& name ) const;
  */

  
  test_(datasetinfo->HasCuts() == kFALSE); // why does this fail?
  
  datasetinfo->SetCut(cut1, classname);
  test_(datasetinfo->HasCuts() == kTRUE);

  test_(datasetinfo->GetCut(0) == cut1);
  //  test_(datasetinfo->GetCut(1) == cut2); // there is no guard against using the wrong index
  test_(datasetinfo->GetCut(classname) == cut1);
  datasetinfo->AddCut(cut2, classname);
  cut1 += cut2;
  test_(datasetinfo->GetCut(classname) == cut1);
  
  datasetinfo->SetSplitOptions(splitoption);
  test_(datasetinfo->GetSplitOptions() == splitoption);
  
  test_(datasetinfo->GetWeightExpression(0) == "");
  datasetinfo->SetWeightExpression(weightexpr);
  test_(datasetinfo->GetWeightExpression(0) == weightexpr);
 
  datasetinfo->SetCorrelationMatrix(classname,matrix);
  //  datasetinfo->PrintCorrelationMatrix(classname);// this is insecure!
  
  test_(datasetinfo->FindVarIndex(expression) == 0);

  vector<TString> vars = datasetinfo->GetListOfVariables();
  
  
  /*
  void               ClearDataSet() const;
  Int_t              FindVarIndex( const TString& )      const;
  std::vector<TString> GetListOfVariables() const;
  const TMatrixD*    CorrelationMatrix     ( const TString& className ) const;
  void               SetCorrelationMatrix  ( const TString& className, TMatrixD* matrix );
  TH2*               CreateCorrelationMatrixHist( const TMatrixD* m, const TString& hName, const TString& hTitle ) const;
  */

  
  /* 
  unclear to me, what to test:
  void               SetRootDir(TDirectory* d) { fOwnRootDir = d; }
  TDirectory*        GetRootDir() const { return fOwnRootDir; }
  */
      
}

