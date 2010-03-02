#include "TGFrame.h"

class TGTextEntry;
class TGHProgressBar;
class TGTextButton;

class TF1;

class RooWorkspace;

class TRooFitPanel: public TGVerticalFrame {

public: 
   TRooFitPanel(const TGWindow* p);
   virtual ~TRooFitPanel();

protected:
   TGTextEntry         *fExpRoo;           // RooFit expression 
   TGHProgressBar      *fProgRoo;          // shows the progression of the TF1 creation
   TGTextButton        *fGenRoo;           // starts the TF1 generation
   RooWorkspace        *fWorkspace;        // RooWorkspace of the FitPanel session

protected:
   TF1 *       CreateRooFitPdf(const char * expr, bool norm = false);

// Public interface
public:
   void ConnectSlots();
   void DisconnectSlots();

// Event controllers
public:
   virtual void   DoGenerateRooFit();
};
