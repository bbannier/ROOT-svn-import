#include <stdexcept>
#include <cstring>
#include <memory>

#include "CocoaPrivate.h"
#include "TVirtualX.h"
#include "X11Buffer.h"

namespace ROOT {
namespace MacOSX {
namespace X11 {

//______________________________________________________________________________
Command::Command(Drawable_t wid, GContext_t gc)
            : fID(wid),
              fGC(gc)
{
}

//______________________________________________________________________________
Command::~Command()
{
}

//______________________________________________________________________________
DrawLine::DrawLine(Drawable_t wid, GContext_t gc, const Point_t &p1, const Point_t &p2)
            : Command(wid, gc),
              fP1(p1),
              fP2(p2)
{
}

//______________________________________________________________________________
void DrawLine::Execute()const
{
   gVirtualX->DrawLine(fID, fGC, fP1.fX, fP1.fY, fP2.fX, fP2.fY);
}

//______________________________________________________________________________
ClearArea::ClearArea(Window_t wid, const Rectangle_t &area)
             : Command(wid, GContext_t()),
               fArea(area)
{
}

//______________________________________________________________________________
void ClearArea::Execute()const
{
   gVirtualX->ClearArea(fID, fArea.fX, fArea.fY, fArea.fWidth, fArea.fHeight);   
}

//______________________________________________________________________________
CopyArea::CopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, const Rectangle_t &area, const Point_t &dstPoint)
               : Command(dst, gc),
                 fSrc(src),
                 fArea(area),
                 fDstPoint(dstPoint)
{
}

//______________________________________________________________________________
void CopyArea::Execute()const
{
   gVirtualX->CopyArea(fSrc, fID, fGC, fArea.fX, fArea.fY, fArea.fWidth, fArea.fHeight, fDstPoint.fX, fDstPoint.fY);
}

//______________________________________________________________________________
DrawString::DrawString(Drawable_t wid, GContext_t gc, const Point_t &point, const std::string &text)
               : Command(wid, gc),
                 fPoint(point),
                 fText(text)
{
}

//______________________________________________________________________________
void DrawString::Execute()const
{
   gVirtualX->DrawString(fID, fGC, fPoint.fX, fPoint.fY, fText.c_str(), fText.length());
}

//______________________________________________________________________________
FillRectangle::FillRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle)
                  : Command(wid, gc),
                    fRectangle(rectangle)
{
}

//______________________________________________________________________________
void FillRectangle::Execute()const
{
   gVirtualX->FillRectangle(fID, fGC, fRectangle.fX, fRectangle.fY, fRectangle.fWidth, fRectangle.fHeight);
}

//______________________________________________________________________________
DrawRectangle::DrawRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle)
                 : Command(wid, gc),
                   fRectangle(rectangle)
{
}

//______________________________________________________________________________
void DrawRectangle::Execute()const
{
   gVirtualX->DrawRectangle(fID, fGC, fRectangle.fX, fRectangle.fY, fRectangle.fWidth, fRectangle.fHeight);
}

//______________________________________________________________________________
CommandBuffer::CommandBuffer()
{
}

//______________________________________________________________________________
CommandBuffer::~CommandBuffer()
{
   ClearCommands();
}

//______________________________________________________________________________
void CommandBuffer::AddDrawLine(Drawable_t wid, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2)
{
   try {
      Point_t p1 = {}; 
      //I'd use .fX = x1 from standard C, but ... this is already C++0x + Obj-C :)
      //So, not to make it worse :)
      p1.fX = x1;
      p1.fY = y1;
      Point_t p2 = {};
      p2.fX = x2;
      p2.fY = y2;
      std::auto_ptr<DrawLine> cmd(new DrawLine(wid, gc, p1, p2));//if this throws, I do not care.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::AddClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<ClearArea> cmd(new ClearArea(wid, r));//Can throw, nothing leaks.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::AddCopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY)
{
   try {
      Rectangle_t area = {};
      area.fX = srcX;
      area.fY = srcY;
      area.fWidth = width;
      area.fHeight = height;
      Point_t dstPoint = {};
      dstPoint.fX = dstX;
      dstPoint.fY = dstY;
      std::auto_ptr<CopyArea> cmd(new CopyArea(src, dst, gc, area, dstPoint));//Can throw, nothing leaks.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::AddDrawString(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, const char *text, Int_t len)
{
   try {
      if (len < 0)//Negative length can come from caller.
         len = std::strlen(text);
      const std::string substr(text, len);//Can throw.
      Point_t p = {};
      p.fX = x;
      p.fY = y;
      std::auto_ptr<DrawString> cmd(new DrawString(wid, gc, p, substr));//Can throw.
      fCommands.push_back(cmd.get());//can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::AddFillRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<FillRectangle> cmd(new FillRectangle(wid, gc, r));
      fCommands.push_back(cmd.get());
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::AddDrawRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<DrawRectangle> cmd(new DrawRectangle(wid, gc, r));
      fCommands.push_back(cmd.get());
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void CommandBuffer::Flush(Details::CocoaPrivate * /*impl*/)
{
   //All magic is here.
}

//______________________________________________________________________________
void CommandBuffer::ClearCommands()
{
   for (auto cmd : fCommands)
      delete cmd;

   fCommands.clear();
}

}//X11
}//MacOSX
}//ROOT
