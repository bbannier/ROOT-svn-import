#include "TGLFontManager.h"

#include "TSystem.h"
#include "TEnv.h"
#include "TObjString.h"
#include "TGLUtil.h"

#include "FTFont.h"
#include "FTGLExtrdFont.h"
#include "FTGLOutlineFont.h"
#include "FTGLPolygonFont.h"
#include "FTGLTextureFont.h"
#include "FTGLPixmapFont.h"
#include "FTGLBitmapFont.h"

//______________________________________________________________________________
// TGLFont
//
// A wrapper class for FTFont.
// Holds pointer to FTFont object and its description: face size, font file
// and class ID. It  wraps Render and BBox functions.
//

ClassImp(TGLFont);

//______________________________________________________________________________
TGLFont::TGLFont(Int_t size, Int_t font, EMode mode, const FTFont* f):
   fSize(size), fFile(font), fMode(mode), fFont(f), fDepth(1)
{
   // Constructor.
}

//______________________________________________________________________________
TGLFont::TGLFont(const TGLFont& o) :
   fSize(o.fSize), fFile(o.fFile), fMode(o.fMode), fFont(o.fFont), fDepth(o.fDepth)
{
   // Copy constructor.
}

//______________________________________________________________________________
TGLFont& TGLFont::operator=(const TGLFont& o)
{
   // Assignment operator.

   fSize  = o.fSize;
   fFile  = o.fFile;
   fMode  = o.fMode;
   fFont  = o.fFont;
   fDepth = o.fDepth;
   return *this;
}


/******************************************************************************/

//______________________________________________________________________________
void TGLFont::BBox(const char* txt, Float_t& llx, Float_t& lly, Float_t& llz, Float_t& urx, Float_t& ury, Float_t& urz) const
{
   // Get bounding box.

   // FTGL is not const correct.
   const_cast<FTFont*>(fFont)->BBox(txt, llx, lly, llz, urx, ury, urz);
}

//______________________________________________________________________________
void TGLFont::Render(const char* txt) const
{
   // Render text.

   Bool_t scaleDepth = (fMode == kExtrude && fDepth != 1.0f);

   if (scaleDepth) {
      glPushMatrix();
      // !!! 0.2*fSize is hard-coded in TGLFontManager::GetFont(), too.
      glTranslatef(0.0f, 0.0f, 0.5f*fDepth * 0.2f*fSize);
      glScalef(1.0f, 1.0f, fDepth);
   }

   // FTGL is not const correct.
   const_cast<FTFont*>(fFont)->Render(txt);

   if (scaleDepth) {
      glPopMatrix();
   }
}

//______________________________________________________________________________
void TGLFont::PreRender(Bool_t autoLight, Bool_t lightOn) const
{
   // Set-up GL state before FTFont rendering.

   switch (fMode)
   {
      case kBitmap:
      case kPixmap:
         glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
         glEnable(GL_ALPHA_TEST);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glAlphaFunc(GL_GEQUAL, 0.0625);
         break;
      case kTexture:
         glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
         glEnable(GL_TEXTURE_2D);
         glDisable(GL_CULL_FACE);
         glEnable(GL_ALPHA_TEST);
         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
         glAlphaFunc(GL_GEQUAL, 0.0625);
         break;
      case kOutline:
      case kPolygon:
      case kExtrude:
         glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT);
         glEnable(GL_NORMALIZE);
         glDisable(GL_CULL_FACE);
         break;
      default:
         Warning("TGLFont::PreRender", "Font mode undefined.");
         glPushAttrib(GL_LIGHTING_BIT);
         break;
   }

   if ((autoLight && fMode > TGLFont::kOutline) || (!autoLight && lightOn))
      glEnable(GL_LIGHTING);
   else
      glDisable(GL_LIGHTING);
}

//______________________________________________________________________________
void TGLFont::PostRender() const
{
   // Reset GL state after FTFont rendering.

   glPopAttrib();
}


//______________________________________________________________________________
//
// A FreeType GL font manager.
//
// Each GL rendering context has an instance of FTGLManager.
// This enables FTGL fonts to be shared same way as textures and display lists.

ClassImp(TGLFontManager);

TObjArray   TGLFontManager::fgFontFileArray;
TGLFontManager::FontSizeVec_t TGLFontManager::fgFontSizeArray;
Bool_t  TGLFontManager::fgStaticInitDone = kFALSE;

//______________________________________________________________________________
TGLFontManager::~TGLFontManager()
{
   // Destructor.

   std::map<TGLFont, Int_t>::iterator it = fFontMap.begin();
   while (it != fFontMap.end()) {
      delete it->first.GetFont();
      it++;
   }
   fFontMap.clear();
}

//______________________________________________________________________________
const TGLFont& TGLFontManager::GetFont(Int_t size, Int_t fileID, TGLFont::EMode mode)
{
   // Provide font with given size, file and FTGL class.

   if (fgStaticInitDone == kFALSE) InitStatics();

   std::map<TGLFont, Int_t>::iterator it = fFontMap.find(TGLFont(size, fileID, mode));
   if (it == fFontMap.end())
   {
      TString ttpath;
# ifdef TTFFONTDIR
      ttpath = gEnv->GetValue("Root.TTGLFontPath", TTFFONTDIR );
# else
      ttpath = gEnv->GetValue("Root.TTGLFontPath", "$(ROOTSYS)/fonts");
# endif
      TObjString* name = (TObjString*)fgFontFileArray[fileID];
      const char *file = gSystem->Which(ttpath.Data(), Form("%s.ttf", name->GetString().Data()));

      FTFont* ftfont = 0;
      switch (mode)
      {
         case TGLFont::kBitmap:
            ftfont = new FTGLBitmapFont(file);
            break;
         case TGLFont::kPixmap:
            ftfont = new FTGLPixmapFont(file);
            break;
         case TGLFont::kOutline:
            ftfont = new FTGLOutlineFont(file);
            break;
         case TGLFont::kPolygon:
            ftfont = new FTGLPolygonFont(file);
            break;
         case TGLFont::kExtrude:
            ftfont = new FTGLExtrdFont(file);
            ftfont->Depth(0.2*size);
            break;
         case TGLFont::kTexture:
            ftfont = new FTGLTextureFont(file);
            break;
         default:
            Error("TGLFontManager::GetFont", "invalid FTGL type");
            break;
      }
      ftfont->FaceSize(size);
      return fFontMap.insert(std::make_pair(TGLFont(size, fileID, mode, ftfont), 1)).first->first;
   }
   else
   {
      it->second = it->second;
      return it->first;
   }
}

//______________________________________________________________________________
Bool_t TGLFontManager::ReleaseFont(const TGLFont& font)
{
   // Release font with given attributes. Returns false if font has
   // not been found in the managers font set.

   std::map<TGLFont, Int_t>::iterator it = fFontMap.find(font);

   if (it != fFontMap.end()) {
      it->second = it->second -1;
      if (it->second == 0) {
         delete it->first.GetFont();
         fFontMap.erase(it);
      }
      return kTRUE;
   }
   return kFALSE;
}

//______________________________________________________________________________
TObjArray* TGLFontManager::GetFontFileArray()
{
   // Get id to file name map.

   if (fgStaticInitDone == kFALSE) InitStatics();
   return &fgFontFileArray;
}

//______________________________________________________________________________
TGLFontManager::FontSizeVec_t* TGLFontManager::GetFontSizeArray()
{
   // Get valid font size vector.

   if (fgStaticInitDone == kFALSE) InitStatics();
   return &fgFontSizeArray;
}

//______________________________________________________________________________
void TGLFontManager::InitStatics()
{
   // Create a list of available font files and allowed font sizes.

   const char *ttpath = gEnv->GetValue("Root.TTFontPath",
# ifdef TTFFONTDIR
                                       TTFFONTDIR);
# else
                                       "$(ROOTSYS)/fonts");
# endif

   void *dir = gSystem->OpenDirectory(ttpath);
   const char* name = 0;
   TString s;
   while ((name = gSystem->GetDirEntry(dir))) {
      s = name;
      if (s.EndsWith(".ttf")) {
         s.Resize(s.Sizeof() -5);
         fgFontFileArray.Add(new TObjString(s.Data()));
      }
   }
   fgFontFileArray.Sort();
   gSystem->FreeDirectory(dir);


   // font sizes
   for (Int_t i = 8; i <= 20; i+=2)
      fgFontSizeArray.push_back(i);
   for (Int_t i = 24; i <= 64; i+=4)
      fgFontSizeArray.push_back(i);
   for (Int_t i = 72; i <= 120; i+=8)
      fgFontSizeArray.push_back(i);

   fgStaticInitDone = kTRUE;
}

