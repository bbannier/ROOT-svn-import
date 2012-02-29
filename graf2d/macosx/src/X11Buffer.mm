#include <stdexcept>
#include <cstring>
#include <memory>

#include "CocoaPrivate.h"
#include "TVirtualX.h"
#include "X11Buffer.h"

namespace ROOT {
namespace MacOSX {

//______________________________________________________________________________
X11Command::X11Command(Drawable_t wid, GContext_t gc)
               : fID(wid),
                 fGC(gc)
{
}

//______________________________________________________________________________
X11Command::~X11Command()
{
}

//______________________________________________________________________________
X11DrawLine::X11DrawLine(Drawable_t wid, GContext_t gc, const Point_t &p1, const Point_t &p2)
               : X11Command(wid, gc),
                 fP1(p1),
                 fP2(p2)
{
}

//______________________________________________________________________________
void X11DrawLine::Execute()const
{
   gVirtualX->DrawLine(fID, fGC, fP1.fX, fP1.fY, fP2.fX, fP2.fY);
}

//______________________________________________________________________________
X11ClearArea::X11ClearArea(Window_t wid, const Rectangle_t &area)
                : X11Command(wid, GContext_t()),
                  fArea(area)
{
}

//______________________________________________________________________________
void X11ClearArea::Execute()const
{
   gVirtualX->ClearArea(fID, fArea.fX, fArea.fY, fArea.fWidth, fArea.fHeight);   
}

//______________________________________________________________________________
X11CopyArea::X11CopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, const Rectangle_t &area, const Point_t &dstPoint)
               : X11Command(dst, gc),
                 fSrc(src),
                 fArea(area),
                 fDstPoint(dstPoint)
{
}

//______________________________________________________________________________
void X11CopyArea::Execute()const
{
   gVirtualX->CopyArea(fSrc, fID, fGC, fArea.fX, fArea.fY, fArea.fWidth, fArea.fHeight, fDstPoint.fX, fDstPoint.fY);
}

//______________________________________________________________________________
X11DrawString::X11DrawString(Drawable_t wid, GContext_t gc, const Point_t &point, const std::string &text)
                  : X11Command(wid, gc),
                    fPoint(point),
                    fText(text)
{
}

//______________________________________________________________________________
void X11DrawString::Execute()const
{
   gVirtualX->DrawString(fID, fGC, fPoint.fX, fPoint.fY, fText.c_str(), fText.length());
}

//______________________________________________________________________________
X11FillRectangle::X11FillRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle)
                     : X11Command(wid, gc),
                       fRectangle(rectangle)
{
}

//______________________________________________________________________________
void X11FillRectangle::Execute()const
{
   gVirtualX->FillRectangle(fID, fGC, fRectangle.fX, fRectangle.fY, fRectangle.fWidth, fRectangle.fHeight);
}

//______________________________________________________________________________
X11DrawRectangle::X11DrawRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle)
                     : X11Command(wid, gc),
                       fRectangle(rectangle)
{
}

//______________________________________________________________________________
void X11DrawRectangle::Execute()const
{
   gVirtualX->DrawRectangle(fID, fGC, fRectangle.fX, fRectangle.fY, fRectangle.fWidth, fRectangle.fHeight);
}

//______________________________________________________________________________
X11CommandBuffer::~X11CommandBuffer()
{
   ClearCommands();
}

//______________________________________________________________________________
void X11CommandBuffer::AddDrawLine(Drawable_t wid, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2)
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
      std::auto_ptr<X11DrawLine> cmd(new X11DrawLine(wid, gc, p1, p2));//if this throws, I do not care.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::AddClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<X11ClearArea> cmd(new X11ClearArea(wid, r));//Can throw, nothing leaks.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::AddCopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY)
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
      std::auto_ptr<X11CopyArea> cmd(new X11CopyArea(src, dst, gc, area, dstPoint));//Can throw, nothing leaks.
      fCommands.push_back(cmd.get());//this can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::AddDrawString(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, const char *text, Int_t len)
{
   try {
      if (len < 0)//Negative length can come from caller.
         len = std::strlen(text);
      const std::string substr(text, len);//Can throw.
      Point_t p = {};
      p.fX = x;
      p.fY = y;
      std::auto_ptr<X11DrawString> cmd(new X11DrawString(wid, gc, p, substr));//Can throw.
      fCommands.push_back(cmd.get());//can throw.
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::AddFillRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<X11FillRectangle> cmd(new X11FillRectangle(wid, gc, r));
      fCommands.push_back(cmd.get());
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::AddDrawRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h)
{
   try {
      Rectangle_t r = {};
      r.fX = x;
      r.fY = y;
      r.fWidth = w;
      r.fHeight = h;
      std::auto_ptr<X11DrawRectangle> cmd(new X11DrawRectangle(wid, gc, r));
      fCommands.push_back(cmd.get());
      cmd.release();
   } catch (const std::exception &) {
      throw;
   }
}

//______________________________________________________________________________
void X11CommandBuffer::Flush(Details::CocoaPrivate * /*impl*/)
{
   //All magic is here.
}

//______________________________________________________________________________
void X11CommandBuffer::ClearCommands()
{
   for (auto cmd : fCommands)
      delete cmd;

   fCommands.clear();
}

}//MacOSX
}//ROOT
