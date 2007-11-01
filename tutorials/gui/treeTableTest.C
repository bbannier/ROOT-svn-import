// A simple example that shows the usage of a TGSplitButton. The
// checkbutton is used to change the split state of the button.
//
// author, Roel Aaij 13/07/2007

#include <iostream>
#include <TApplication.h>
#include <TGClient.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGWindow.h>
#include <TString.h>
#include <TGTable.h>
#include <TTreeTableInterface.h>
#include <TFile.h>
#include <TNtuple.h>

// A little class to automatically handle the generation of unique
// widget ids.
class IDList {
private:
   Int_t nID ;               // Generates unique widget IDs.
public:
   IDList() : nID(0) {}
   ~IDList() {}
   Int_t GetUnID(void) { return ++nID ; }
} ;

class TableTest : public TGMainFrame {

private:
   IDList   fIDs ;      // Generator for unique widget IDs.
   UInt_t   fNTableRows;
   UInt_t   fNTableColumns;
   TGTable *fTable;
   TFile   *fFile;

   TTreeTableInterface *fInterface;

public:
   TableTest(const TGWindow *p, UInt_t ntrows, UInt_t ntcols, 
             UInt_t w = 100, UInt_t h = 100) ;
   virtual ~TableTest() ;

   void DoExit() ;

   TGTable *GetTable() { return fTable; }
   TTreeTableInterface *GetInterface() { return fInterface; }

   ClassDef(TableTest, 0)
};
                          
TableTest::TableTest(const TGWindow *p, UInt_t ntrows, UInt_t ntcols, 
                     UInt_t w, UInt_t h) 
   : TGMainFrame(p, w, h),  fNTableRows(ntrows), fNTableColumns(ntcols), 
     fTable(0)
{
   SetCleanup(kDeepCleanup) ;
   
   Connect("CloseWindow()", "TableTest", this, "DoExit()") ;
   DontCallClose() ;
   
   //   TGTextButton *tb = new TGTextButton(fVL, new TGHotString("&Exit"), 
   //                                       fIDs.GetUnID());
   //   tb->Connect("Clicked()", "TableTest", this, "DoExit()");
   
   // Open root file for the tree
//    fFile = new TFile("$ROOTSYS/tutorials/hsimple.root");
   fFile = new TFile("$ROOTSYS/tutorials/gui/staff.root");


   if (!fFile || fFile->IsZombie()) {
      printf("Please run <ROOT location>/tutorials/tree/staff.C first.");
      return;
   }
//    TNtuple *ntuple = (TNtuple *)fFile->Get("ntuple");
   TTree *tree = (TTree *)fFile->Get("T");
//    TString varexp = "px:py:pz:random:sin(px):log(px/py):log(pz)";
   TString varexp = "*";
//    TString select = "px>0 && py>0 && pz>0";
   TString select = "";
   TString options = "";
//    fInterface = new TTreeTableInterface(ntuple, varexp.Data(), select.Data(),
//                                         options.Data());
   fInterface = new TTreeTableInterface(tree, varexp.Data(), select.Data(),
                                        options.Data());
   
   fTable = new TGTable(this, fIDs.GetUnID(), fInterface, fNTableRows, 
                                fNTableColumns);
   AddFrame(fTable, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
 
//    fInterface->AddColumn("px+py", 3);
//    fInterface->RemoveColumn(5);
//    fInterface->RemoveColumn(2);
//    fTable->Update();
  
   SetWindowName("Tree Table Test") ;
   MapSubwindows() ;
   Layout();
   Resize() ;
   MapWindow() ;
   
} ;

TableTest::~TableTest()
{
   // Destructor
   delete fInterface;
   fFile->Close();
   Cleanup() ;
}
 
 void TableTest::DoExit()
{
   // Exit this application via the Exit button or Window Manager.
   // Use one of the both lines according to your needs.
   // Please note to re-run this macro in the same ROOT session,
   // you have to compile it to get signals/slots 'on place'.
   
   DeleteWindow();            // to stay in the ROOT session
   //   gApplication->Terminate();   // to exit and close the ROOT session   
}

TGTable *treeTableTest(UInt_t ntrows = 50, UInt_t ntcols = 10) {
   TableTest *test = new TableTest(0, ntrows, ntcols, 500, 200);
   return test->GetTable();
}
