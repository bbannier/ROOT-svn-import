#include "TGFrame.h"

class TGTextEntry;
class TGHProgressBar;
class TGTextButton;
class TGComboBox;

class TString;
class TF1;

class RooWorkspace;

#include <map>

class TRooFitPanel: public TGVerticalFrame {

public: 
   TRooFitPanel(const TGWindow* p);
   virtual ~TRooFitPanel();

protected:
   TGTextEntry         *fExpRoo;           // RooFit expression  
   TGTextButton        *fGenRoo;           // starts the TF1 generation
   TGHProgressBar      *fProgRoo;          // shows the progression of the TF1 creation
   TGComboBox          *fNameRoo;          // Shows the list of functions stored in the workspace
   RooWorkspace        *fWorkspace;        // RooWorkspace of the FitPanel session

   std::map<const TString, TString> fDefinedFunctions;

protected:
   TF1 *         CreateRooFitPdf(const char * expr, bool norm = false);
   const TString GetLastCreatedFunctionName();
   void          UpdateListOfFunctions();

// Public interface
public:
   void ConnectSlots();
   void DisconnectSlots();
   inline RooWorkspace* GetRooWorkspace() {return fWorkspace;};
   const char* GetFunctionDefinition(const char* functionName) const;

// Event controllers
public:
   virtual void   DoNameSel(Int_t);
   virtual void   DoGenerateRooFit();
};
