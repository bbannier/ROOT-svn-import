//Author: Timur Pocheptsov 29/02/2012
#ifndef ROOT_X11Buffer
#define ROOT_X11Buffer

#include <vector>
#include <string>

#ifndef ROOT_GuiTypes
#include "GuiTypes.h"
#endif

//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
// Unfortunately, TGCocoa's drawing methods can be called in a                  //
// "wrong" time and place: not from QuartzView -drawRect.                       //
// For example, on mouse move. This is bad and unnatural for Cocoa application, //
// since I expect GUI to draw only when I'm ready == ... called from drawRect.  //
// In X11 commands are buffered and this buffer is flushed at some points.      //
// I'm trying to emulate this, just to make GUI happy.                          //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

namespace ROOT {
namespace MacOSX {

class X11Command {
protected:
   const Drawable_t fID;
   const GContext_t fGC;
   
public:
   X11Command(Drawable_t wid, GContext_t gc);
   virtual ~X11Command();

   virtual void Execute()const = 0;
   
   X11Command(const X11Command &rhs) = delete;
   X11Command &operator = (const X11Command &rhs) = delete;
};

class X11DrawLine : public X11Command {
private:
   const Point_t fP1;
   const Point_t fP2;

public:
   X11DrawLine(Drawable_t wid, GContext_t gc, const Point_t &p1, const Point_t &p2);
   void Execute()const;
};

class X11ClearArea : public X11Command {
private:
   const Rectangle_t fArea;

public:
   X11ClearArea(Window_t wid, const Rectangle_t &area);
   void Execute()const;
};

class X11CopyArea : public X11Command {
private:
   const Drawable_t  fSrc;
   const Rectangle_t fArea;
   const Point_t     fDstPoint;

public:
   X11CopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, const Rectangle_t &area, const Point_t &dstPoint);
   void Execute()const;
};

class X11DrawString : public X11Command {
private:
   const Point_t     fPoint;
   const std::string fText;

public:
   X11DrawString(Drawable_t wid, GContext_t gc, const Point_t &point, const std::string &text);
   void Execute()const;
};

class X11FillRectangle : public X11Command {
private:
   const Rectangle_t fRectangle;

public:
   X11FillRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle);
   void Execute()const;
};

class X11DrawRectangle : public X11Command {
private:
   Rectangle_t fRectangle;

public:
   X11DrawRectangle(Drawable_t wid, GContext_t gc, const Rectangle_t &rectangle);
   void Execute()const;
};

namespace Details {

class CocoaPrivate;

}

class X11CommandBuffer {
private:
   X11CommandBuffer(const X11CommandBuffer &rhs) = delete;
   X11CommandBuffer &operator = (const X11CommandBuffer &rhs) = delete;
   
   std::vector<X11Command *> fCommands;

public:
   ~X11CommandBuffer();

   void AddDrawLine(Drawable_t wid, GContext_t gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2);
   void AddClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h);
   void AddCopyArea(Drawable_t src, Drawable_t dst, GContext_t gc, Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY);
   void AddDrawString(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, const char *text, Int_t len);
   void AddFillRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h);
   void AddDrawRectangle(Drawable_t wid, GContext_t gc, Int_t x, Int_t y, UInt_t w, UInt_t h);

   void Flush(Details::CocoaPrivate *impl);
private:
   void ClearCommands();

};

}
}

#endif
