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

@class QuartzView;

namespace ROOT {
namespace MacOSX {

namespace Details {
class CocoaPrivate;
}

namespace X11 {

class Command {
   friend class CommandBuffer;

protected:
   const Drawable_t fID;
   const GCValues_t fGC;
   
public:
   Command(Drawable_t wid);
   Command(Drawable_t wid, const GCValues_t &gc);
   virtual ~Command();

   virtual bool HasOperand(Drawable_t drawable)const;

   virtual void Execute()const = 0;
   
   Command(const Command &rhs) = delete;
   Command &operator = (const Command &rhs) = delete;
};

class DrawLine : public Command {
private:
   const Point_t fP1;
   const Point_t fP2;

public:
   DrawLine(Drawable_t wid, const GCValues_t &gc, const Point_t &p1, const Point_t &p2);
   void Execute()const;
};

class ClearArea : public Command {
private:
   const Rectangle_t fArea;

public:
   ClearArea(Window_t wid, const Rectangle_t &area);
   void Execute()const;
};

class CopyArea : public Command {
private:
   const Drawable_t  fSrc;
   const Rectangle_t fArea;
   const Point_t     fDstPoint;

public:
   CopyArea(Drawable_t src, Drawable_t dst, const GCValues_t &gc, const Rectangle_t &area, const Point_t &dstPoint);

   bool HasOperand(Drawable_t drawable)const;

   void Execute()const;
};

class DrawString : public Command {
private:
   const Point_t     fPoint;
   const std::string fText;

public:
   DrawString(Drawable_t wid, const GCValues_t &gc, const Point_t &point, const std::string &text);
   void Execute()const;
};

class FillRectangle : public Command {
private:
   const Rectangle_t fRectangle;

public:
   FillRectangle(Drawable_t wid, const GCValues_t &gc, const Rectangle_t &rectangle);
   void Execute()const;
};

class DrawRectangle : public Command {
private:
   Rectangle_t fRectangle;

public:
   DrawRectangle(Drawable_t wid, const GCValues_t &gc, const Rectangle_t &rectangle);
   void Execute()const;
};

class UpdateWindow : public Command {
private:
   QuartzView *fView;

public:
   UpdateWindow(QuartzView *view);
   void Execute()const;
};

class CommandBuffer {
private:
   CommandBuffer(const CommandBuffer &rhs) = delete;
   CommandBuffer &operator = (const CommandBuffer &rhs) = delete;
   
   std::vector<Command *> fCommands;

public:
   typedef std::vector<Command *>::size_type size_type;

   CommandBuffer();
   ~CommandBuffer();

   void AddDrawLine(Drawable_t wid, const GCValues_t &gc, Int_t x1, Int_t y1, Int_t x2, Int_t y2);
   void AddClearArea(Window_t wid, Int_t x, Int_t y, UInt_t w, UInt_t h);
   void AddCopyArea(Drawable_t src, Drawable_t dst, const GCValues_t &gc,  Int_t srcX, Int_t srcY, UInt_t width, UInt_t height, Int_t dstX, Int_t dstY);
   void AddDrawString(Drawable_t wid, const GCValues_t &gc, Int_t x, Int_t y, const char *text, Int_t len);
   void AddFillRectangle(Drawable_t wid, const GCValues_t &gc, Int_t x, Int_t y, UInt_t w, UInt_t h);
   void AddDrawRectangle(Drawable_t wid, const GCValues_t &gc, Int_t x, Int_t y, UInt_t w, UInt_t h);
   void AddUpdateWindow(QuartzView *view);

   void Flush(Details::CocoaPrivate *impl);
   void RemoveOperationsForDrawable(Drawable_t wid);
   
   size_type BufferSize()const
   {
      return fCommands.size();
   }
private:
   void ClearCommands();
};

}//X11
}//MacOSX
}//ROOT

#endif
