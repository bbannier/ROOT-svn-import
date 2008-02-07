#ifndef ROOT_TGLFontManager
#define ROOT_TGLFontManager

#include "TObjArray.h"
#include <vector>
#include <map>

class FTFont;

class TGLFont
{
public:
   enum EMode { kUndef = -1, kBitmap, kPixmap, kTexture, kOutline, kPolygon, kExtrude}; // FTGL class

private:
   TGLFont& operator=(const TGLFont&); // not implemented

protected:
   Int_t    fSize;   // free-type face size
   Int_t    fFile;   // free-type file name
   EMode    fMode;   // free-type FTGL class id

   const  FTFont* fFont;  // FTGL font

public:
   TGLFont(): fSize(-1), fFile(1), fMode(kUndef),fFont(0) {}
   TGLFont(Int_t size, Int_t font, EMode mode, FTFont* f=0);
   TGLFont(const TGLFont& o) : fSize(o.GetSize()), fFile(o.GetFile()), fMode(o.GetMode()), fFont(fFont = o.GetFont()){}
   virtual ~TGLFont(){};

   Int_t GetSize()const { return fSize;} 
   Int_t GetFile()const { return fFile;} 
   EMode GetMode()const { return fMode;}

   const FTFont* GetFont() const {return fFont;}

   // FTGL wrapper functions
   void   BBox  ( const char* txt, Float_t& llx, Float_t& lly, Float_t& llz, Float_t& urx, Float_t& ury, Float_t& urz) const;
   void   Render(const char* txt) const;

   // helper gl draw functions
   virtual void   PreRender();
   virtual void   PostRender();

   Bool_t operator< (const TGLFont& o) const
   {
      if (fSize == o.fSize)
      {
         if(fFile == o.fFile)
         {
            return fMode < o.fMode;
         }
         return fFile < o.fFile;
      }
      return fSize < o.fSize;
   }

   ClassDef(TGLFont, 0); // A wrapper class for FTFont.
};

/******************************************************************************/
/******************************************************************************/

class TGLFontManager
{
public:
   typedef std::vector<Int_t> FontSizeVec_t;

private:
   TGLFontManager(const TGLFontManager&);            // Not implemented
   TGLFontManager& operator=(const TGLFontManager&); // Not implemented

   std::map<TGLFont, Int_t>  fFontMap;        // map of created fonts

   static TObjArray     fgFontFileArray;      // map font-id to ttf-font-file
   static FontSizeVec_t fgFontSizeArray;      // map of valid font-size
   static Bool_t        fgStaticInitDone;     // global initialization flag
   static void          InitStatics();

public:
   TGLFontManager(){}
   virtual ~TGLFontManager();

   const  TGLFont* GetFont(Int_t size, Int_t file, TGLFont::EMode mode);
   Bool_t ReleaseFont(TGLFont* font);

   static TObjArray*        GetFontFileArray();
   static FontSizeVec_t*    GetFontSizeArray();

   ClassDef(TGLFontManager, 0); // A FreeType GL font manager.
};

#endif
