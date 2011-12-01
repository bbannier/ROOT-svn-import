#include <Cocoa/Cocoa.h>

#include "TGQuartz.h"

ClassImp(TGQuartz)

//______________________________________________________________________________
TGQuartz::TGQuartz()
{
   NSLog(@"TGQuartz default ctor");
}

//______________________________________________________________________________
TGQuartz::TGQuartz(const char *name, const char *title)
            : TGCocoa(name, title)
{
   NSLog(@"TGQuartz was created %s %s", name, title);
}

//______________________________________________________________________________
void TGQuartz::DrawBox(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, EBoxMode /*mode*/)
{
   NSLog(@"-----------DRAW BOX-------------");
}

//______________________________________________________________________________
void TGQuartz::DrawCellArray(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/, Int_t /*nx*/, Int_t /*ny*/, Int_t */*ic*/)
{
}

//______________________________________________________________________________
void TGQuartz::DrawFillArea(Int_t /*n*/, TPoint * /*xy*/)
{
   NSLog(@"------------DRAWFILLAREA-----------");
}

//______________________________________________________________________________
void TGQuartz::DrawLine(Int_t /*x1*/, Int_t /*y1*/, Int_t /*x2*/, Int_t /*y2*/)
{
   NSLog(@"----------DRAWLINE----------");
}

//______________________________________________________________________________
void TGQuartz::DrawPolyLine(Int_t /*n*/, TPoint * /*xy*/)
{
   NSLog(@"---------DRAWPOLYLINE----------");
}

//______________________________________________________________________________
void TGQuartz::DrawPolyMarker(Int_t /*n*/, TPoint * /*xy*/)
{

}

//______________________________________________________________________________
void TGQuartz::DrawText(Int_t /*x*/, Int_t /*y*/, Float_t /*angle*/, Float_t /*mgn*/, const char *text, ETextMode /*mode*/)
{
   NSLog(@"--------DRAW TEXT %s-----", text);
}
