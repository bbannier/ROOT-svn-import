// @(#)root/graf:$Name:  $:$Id: TGraph.h,v 1.37 2004/09/06 06:54:55 brun Exp $
// Author: Rene Brun, Olivier Couet   12/12/94

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TGraph
#define ROOT_TGraph


//////////////////////////////////////////////////////////////////////////
//                                                                      //
// TGraph                                                               //
//                                                                      //
// Graph graphics class.                                                //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_TNamed
#include "TNamed.h"
#endif
#ifndef ROOT_TH1
#include "TH1.h"
#endif

class TBrowser;
class TF1;
class TVector;
class TVectorD;
class TSpline;

class TGraph : public TNamed, public TAttLine, public TAttFill, public TAttMarker {

protected:
    Int_t       fMaxSize;    //!Current dimension of arrays fX and fY
    Int_t       fNpoints;    //Number of points <= fMaxSize
    Double_t   *fX;          //[fNpoints] array of X points
    Double_t   *fY;          //[fNpoints] array of Y points
    TList      *fFunctions;  //Pointer to list of functions (fits and user)
    TH1F       *fHistogram;  //Pointer to histogram used for drawing axis
    Double_t    fMinimum;    //Minimum value for plotting along y
    Double_t    fMaximum;    //Maximum value for plotting along y

    static void      SwapValues(Double_t* arr, Int_t pos1, Int_t pos2);
    virtual void     SwapPoints(Int_t pos1, Int_t pos2);

    virtual Double_t **Allocate(Double_t **newarrays, Int_t newsize);
    virtual Bool_t CopyPoints(Double_t **newarrays, Int_t ibegin, Int_t iend,
                              Int_t obegin);
    virtual void CopyAndRelease(Double_t **newarrays,
                                      Int_t ibegin, Int_t iend, Int_t obegin);
    virtual Double_t **ExpandAndCopy(Double_t **newarrays,
                                     Int_t size, Int_t iend);
    virtual Double_t **ShrinkAndCopy(Double_t **newarrays,
                                     Int_t size, Int_t iend);

public:
    // TGraph status bits
    enum {
       kClipFrame     = BIT(10),  // clip to the frame boundary
       kNotEditable   = BIT(18),  // bit set if graph is non editable
       kFitInit       = BIT(19)
    };

        TGraph();
        TGraph(Int_t n);
        TGraph(Int_t n, const Int_t *x, const Int_t *y);
        TGraph(Int_t n, const Float_t *x, const Float_t *y);
        TGraph(Int_t n, const Double_t *x, const Double_t *y);
        TGraph(const TGraph &gr);
        TGraph(const TVector  &vx, const TVector  &vy);
        TGraph(const TVectorD &vx, const TVectorD &vy);
        TGraph(const TH1 *h);
        TGraph(const TF1 *f, Option_t *option="");
        TGraph(const char *filename, const char *format="%lg %lg", Option_t *option="");

        virtual ~TGraph();
        virtual void     Apply(TF1 *f);
        virtual void     Browse(TBrowser *b);
	static Bool_t    CompareX(const TGraph* gr, Int_t left, Int_t right);
	static Bool_t    CompareY(const TGraph* gr, Int_t left, Int_t right);
	static Bool_t    CompareRadius(const TGraph* gr, Int_t left, Int_t right);
                void     ComputeLogs(Int_t npoints, Int_t opt);
        virtual void     ComputeRange(Double_t &xmin, Double_t &ymin, Double_t &xmax, Double_t &ymax) const;
        virtual Int_t    DistancetoPrimitive(Int_t px, Int_t py);
        virtual void     Draw(Option_t *chopt="");
        virtual void     DrawGraph(Int_t n, const Int_t *x, const Int_t *y, Option_t *option="");
        virtual void     DrawGraph(Int_t n, const Float_t *x, const Float_t *y, Option_t *option="");
        virtual void     DrawGraph(Int_t n, const Double_t *x=0, const Double_t *y=0, Option_t *option="");
        virtual void     DrawPanel(); // *MENU*
        virtual Double_t Eval(Double_t x, TSpline *spline=0, Option_t *option="") const;
        virtual void     ExecuteEvent(Int_t event, Int_t px, Int_t py);
        virtual void   Expand(Int_t newsize);
        virtual void   Expand(Int_t newsize, Int_t step);
        virtual TObject *FindObject(const char *name) const;
        virtual TObject *FindObject(const TObject *obj) const;
        virtual Int_t    Fit(const char *formula ,Option_t *option="" ,Option_t *goption="", Axis_t xmin=0, Axis_t xmax=0); // *MENU*
        virtual Int_t    Fit(TF1 *f1 ,Option_t *option="" ,Option_t *goption="", Axis_t xmin=0, Axis_t xmax=0); // *MENU*
        virtual void     FitPanel(); // *MENU*
                Bool_t   GetEditable() const;
        virtual Double_t GetErrorX(Int_t bin) const;
        virtual Double_t GetErrorY(Int_t bin) const;
        TF1             *GetFunction(const char *name) const;
        TH1F            *GetHistogram() const;
        TList           *GetListOfFunctions() const { return fFunctions; }
        virtual Double_t GetCorrelationFactor() const;
        virtual Double_t GetCovariance() const;
        virtual Double_t GetMean(Int_t axis=1) const;
        virtual Double_t GetRMS(Int_t axis=1) const;
        Int_t            GetMaxSize() const {return fMaxSize;}
        Int_t            GetN() const {return fNpoints;}
        Double_t        *GetX() const {return fX;}
        Double_t        *GetY() const {return fY;}
        TAxis           *GetXaxis() const ;
        TAxis           *GetYaxis() const ;
        virtual void     GetPoint(Int_t i, Double_t &x, Double_t &y) const;
        virtual void     InitExpo(Int_t first=0, Int_t last=0);
        virtual void     InitGaus(Int_t first=0, Int_t last=0);
        virtual void     InitPolynom(Int_t first=0, Int_t last=0);
        virtual Int_t    InsertPoint(); // *MENU*
        virtual Bool_t   IsEditable() const {return !TestBit(kNotEditable);}
        virtual void     LeastSquareFit(Int_t m, Double_t *a, Int_t first=0, Int_t last=0);
        virtual void     LeastSquareLinearFit(Int_t n, Double_t &a0, Double_t &a1, Int_t &ifail, Int_t first, Int_t last);
        virtual void     Paint(Option_t *chopt="");
        virtual void     PaintFit(TF1 *fit);
        virtual void     PaintGraph(Int_t npoints, const Double_t *x, const Double_t *y, Option_t *option="");
        virtual void     PaintGrapHist(Int_t npoints, const Double_t *x, const Double_t *y, Option_t *option="");
        virtual void     Print(Option_t *chopt="") const;
        virtual Int_t    RemovePoint(); // *MENU*
        virtual Int_t    RemovePoint(Int_t ipoint);
        virtual void     SavePrimitive(ofstream &out, Option_t *option);
        virtual void     SetEditable(Bool_t editable=kTRUE); // *TOGGLE* *GETTER=GetEditable
        virtual void     SetHistogram(TH1 *h) {fHistogram = (TH1F*)h;}
        virtual void     SetMaximum(Double_t maximum=-1111); // *MENU*
        virtual void     SetMinimum(Double_t minimum=-1111); // *MENU*
        virtual void     Set(Int_t n);
        virtual void     SetPoint(Int_t i, Double_t x, Double_t y);
        virtual void     SetTitle(const char *title="");    // *MENU*
                void     Smooth(Int_t npoints, Double_t *x, Double_t *y, Int_t drawtype);
        virtual void     Sort(Bool_t (*greater)(const TGraph*, Int_t, Int_t)=&TGraph::CompareX,
                              Bool_t ascending=kTRUE, Int_t low=0, Int_t high=-1111);
        virtual void     UseCurrentStyle();
                void     Zero(Int_t &k,Double_t AZ,Double_t BZ,Double_t E2,Double_t &X,Double_t &Y
                          ,Int_t maxiterations);

        ClassDef(TGraph,4)  //Graph graphics class
};

#endif


