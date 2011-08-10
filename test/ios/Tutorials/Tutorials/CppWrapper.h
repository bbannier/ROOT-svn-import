#ifndef ROOT_CppWrapper
#define ROOT_CppWrapper

#include <vector>
#include <memory>

#include <CoreGraphics/CGContext.h>
#include <CoreGraphics/CGImage.h>

#include "ResourceManagement.h"

//
//Wrapper for C++ code to be used in Objective-C++.
//ROOT classes use a macro with 'id' token, which is a keyword in Obj-C.
//So in Objective-C++ code I can include and use only classes without
//ClassDef macro. But classes, inheriting from core/base, have such
//base classes. So I need this simple 'wrapper' class to hide 
//ClassDef macro from Objective-C++ compiler.
//

namespace ROOT_iOS {

class FontManager;
class PadProxy;
class Painter;

}

class FontManagerWrapper : private ROOT_iOS::Util::NonCopyable {
public:
   FontManagerWrapper();
   ~FontManagerWrapper();
   
   ROOT_iOS::FontManager *GetFontManager() const {return fManager.get();}

private:
   std::auto_ptr<ROOT_iOS::FontManager> fManager;
};

class PainterWrapper : private ROOT_iOS::Util::NonCopyable {
public:
   PainterWrapper(ROOT_iOS::FontManager *fontManager);
   ~PainterWrapper();
   
   ROOT_iOS::Painter *GetPainter() const {return fPainter.get();}
   
   void SetContext(CGContextRef ctx);
private:
   std::auto_ptr<ROOT_iOS::Painter> fPainter;
};

struct PadParametersForEditor {
   unsigned fillColor;
   unsigned fillPattern;

   bool tickX;
   bool tickY;

   bool gridX;
   bool gridY;

   bool logX;
   bool logY;
   bool logZ;
};

class PadWrapper : private ROOT_iOS::Util::NonCopyable {
public:
   enum {
      pwButton1Down,
      pwButton1Motion,
      pwButton1Up
   };
   //w and h - are view sizes to pass to pad.
   PadWrapper(unsigned w, unsigned h, ROOT_iOS::Painter *painter, ROOT_iOS::FontManager *fontManager);
   ~PadWrapper();
   
   void cd();
   void Paint();
   void Clear();
   void SelectObject(unsigned x, unsigned y);
   void ExRot(Int_t ev, Int_t px, Int_t py);
   
   void Pick(Int_t px, Int_t py);
  
   ROOT_iOS::PadProxy *GetPad() const {return fPad.get();}
   
   bool Selected() const;
   void PaintSelected() const;
   void PaintShadowForSelected() const;
   
   PadParametersForEditor GetPadParams() const;
   void SetPadParams(PadParametersForEditor params);
   
private:
   void InitSelectionBuffer();
   void CopySelectionBuffer(CGImageRef im);

   std::auto_ptr<ROOT_iOS::PadProxy> fPad;
   ROOT_iOS::Painter * fPainter;
   
   std::vector<unsigned char> fBitmapData;
};

#endif
