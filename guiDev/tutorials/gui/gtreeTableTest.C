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
#include <TGTreeTable.h>
#include <TFile.h>
#include <TNtuple.h>
#include <TTree.h>

// A little class to automatically handle the generation of unique
// widget ids.
class IDList {
private:
   Int_t nID;               // Generates unique widget IDs.
public:
   IDList() : nID(0) {}
   ~IDList() {}
   Int_t GetUnID(void) { return ++nID ; }
};

class TableTest : public TGMainFrame {

private:
   IDList        fIDs;      // Generator for unique widget IDs.
   UInt_t        fNTableRows;
   UInt_t        fNTableColumns;
   TGTreeTable  *fTreeTable;
   TFile        *fFile;
   
public:
   TableTest(const TGWindow *p, UInt_t ntrows, UInt_t ntcols, 
             UInt_t w = 100, UInt_t h = 100);
   virtual ~TableTest() ;

   void DoExit() ;

   TGTreeTable *GetTable() { return fTreeTable; }

   ClassDef(TableTest, 0)
};
                          
TableTest::TableTest(const TGWindow *p, UInt_t ntrows, UInt_t ntcols, 
                     UInt_t w, UInt_t h) 
   : TGMainFrame(p, w, h), fNTableRows(ntrows), fNTableColumns(ntcols), 
     fTreeTable(0)
{
   SetCleanup(kDeepCleanup) ;
   
   Connect("CloseWindow()", "TableTest", this, "DoExit()") ;
   DontCallClose() ;
   
   //   TGTextButton *tb = new TGTextButton(fVL, new TGHotString("&Exit"), 
   //                                       fIDs.GetUnID());
   //   tb->Connect("Clicked()", "TableTest", this, "DoExit()");
   
   fFile = new TFile("$ROOTSYS/tutorials/hsimple.root");
   if (!fFile || fFile->IsZombie()) {
      printf("Please run <ROOT location>/tutorials/hsimple.C before.");
      return;
   }
   TNtuple *ntuple = (TNtuple *)fFile->Get("ntuple");
   TString varexp = "px:py:pz:random:sin(px):log(px/py):log(pz)";
//    TString varexp = "*";
   TString select = "px>0 && py>0 && pz>0";
//    TString select = "";
   TString options = "";
   
   fTreeTable = new TGTreeTable(this, fIDs.GetUnID(), ntuple, varexp.Data(), 
                                select.Data(), options.Data(), 100, 7);
   AddFrame(fTreeTable, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   
   SetWindowName("TGTreeTable Test") ;
   MapSubwindows() ;
   Layout();
   Resize() ;
   MapWindow() ;
   
} ;

TableTest::~TableTest()
{
   // Destructor
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

TGTreeTable *gtreeTableTest(UInt_t ntrows = 50, UInt_t ntcols = 10) {
   TableTest *test = new TableTest(0, ntrows, ntcols, 500, 200);
   return test->GetTable();
}
