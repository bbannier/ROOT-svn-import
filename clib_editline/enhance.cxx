#include "TROOT.h"
#include "el.h"
#include <ncurses.h>
#include "TRegexp.h"
#include "TClassTable.h"
#include "TDictionary.h"
#include <stack>
#include <set>

using namespace std;

// int values for colour pairs
#define COLOR_CLASS        4            // NCurses COLOR_BLUE
#define COLOR_TYPE         1            // NCurses COLOR_RED
#define COLOR_BRACKET      2            // NCurses COLOR_GREEN

void highlightKeywords(EditLine * el);
int matchParentheses(EditLine * el);
void colorWord(EditLine * el, int first, int num, int color);
void colorBrackets(EditLine * el, int open, int close, int color);

struct KeywordInLine {
   KeywordInLine(const TString& w, Long64_t hash, Ssiz_t pos):
      fWord(w), fHash(hash), fPosBegin(pos) {}
   TString fWord;
   Long64_t fHash;
   Ssiz_t fPosBegin;
};

/*
 *      Use gRoot to establish keywords known to root.
 *
 */
void highlightKeywords(EditLine * el)
{
   typedef std::set<int> HashSet_t;
   static HashSet_t sHashedKnownClasses;
   static HashSet_t sHashedKnownTypes;

   if (sHashedKnownClasses.empty()) {
      TString name;
      for (int iClass = 0; iClass < gClassTable->Classes(); ++iClass) {
         name = TClassTable::At(iClass);
         sHashedKnownClasses.insert(name.Hash());
      }
      TIter iType(gROOT->GetListOfTypes());
      TObject* type = 0;
      while ((type = iType())) {
         name = type->GetName();
         sHashedKnownTypes.insert(name.Hash());
      }
   }

   TString sBuffer(el->el_line.buffer, el->el_line.lastchar - el->el_line.buffer) ;

   TString keyword;
   Ssiz_t posNextTok = 0;
   Ssiz_t posPrevTok = 0;
   // regular expression inverse of match expression to find end of match
   while (sBuffer.Tokenize(keyword, posNextTok, "[^a-zA-Z0-9_]")) {
      Long64_t hash = keyword.Hash();
      HashSet_t::const_iterator iClassHash = sHashedKnownClasses.find(hash);
      if (iClassHash != sHashedKnownClasses.end()) {
         if (TClassTable::GetDict(keyword)) {
            Ssiz_t toklen = posNextTok - posPrevTok;
            if (posNextTok == -1)
               toklen = sBuffer.Length() - posPrevTok;
            TString tok = sBuffer(posPrevTok, toklen);
            Ssiz_t pos = posPrevTok + tok.Index(keyword);
            colorWord(el, pos, keyword.Length(), COLOR_CLASS);
            posPrevTok = posNextTok;
            continue;
         }
      }
      // don't bother with matching for type names if match already found
      if (gROOT->GetListOfTypes()->FindObject(keyword)) {
         Ssiz_t toklen = posNextTok - posPrevTok;
         if (posNextTok == -1)
            toklen = sBuffer.Length() - posPrevTok;
         TString tok = sBuffer(posPrevTok, toklen);
         Ssiz_t pos = posPrevTok + tok.Index(keyword);
         colorWord(el, pos, keyword.Length(), COLOR_TYPE);
         posPrevTok = posNextTok;
         continue;
      }
      posPrevTok = posNextTok;
   }
}

/** if buffer has content, check each char to see if it is an opening bracket,
    if so, check for its closing one and return the indices to both
    * alt:
    * check each char for a match against each type of open and close bracket
    * if open found, push index onto a seperate stack for each type of bracket
    * if close found, pop previous value off relevant stack
    * and pass both pointers to highlight()
    */
int matchParentheses(EditLine * el) {
   int index = -1;
   int amtBrackets = 3;
   char bTypes[amtBrackets][2];

   bTypes[0][0] = '(';
   bTypes[0][1] = ')';
   bTypes[1][0] = '{';
   bTypes[1][1] = '}';
   bTypes[2][0] = '[';
   bTypes[2][1] = ']';

   // CURRENT STUFF
   // create a string of the buffer contents
   TString sBuffer = "";
   for (char* c=el->el_line.buffer; c<el->el_line.lastchar; c++)
      {
         sBuffer.Append(*c);
      }

   // char* stack for pointers to locations of brackets
   stack<int> locBrackets;

   if (sBuffer.Length() > 0)
      {
         int cursorPos = el->el_line.cursor - el->el_line.buffer;
         int foundParenIdx = -1;

         // check against each bracket type
         int bIndex = 0;
         for (bIndex = 0; bIndex<amtBrackets; bIndex++)
            {
               // if current char is equal to opening bracket, push onto stack
               if (sBuffer[cursorPos] == bTypes[bIndex][0])
                  {
                     locBrackets.push(cursorPos);
                     foundParenIdx = 0;
                     break;
                  } else if (sBuffer[cursorPos] == bTypes[bIndex][1])
                  {
                     locBrackets.push(cursorPos);
                     foundParenIdx = 1;
                     break;
                  }
            }

         // current cursor char is not an open bracket, therefore no need to search
         if ( foundParenIdx == -1 )
            {
               // check for any highlighted brackets and remove colour info
               for (int i = 0; i < sBuffer.Length(); i++)
                  {
                     if ( el->el_line.bufcolor[i].foreColor == COLOR_BRACKET )
                        {
                           el->el_line.bufcolor[i] = -1;                // reset to default colours
                           term__repaint(el, i);
                        }
                  }
               return -1;
            }

         // iterate through remaining letters until find a matching closing bracket
         // if another open bracket of the same type is found, push onto stack
         // and pop on next closing bracket match
         int step = 1;
         if (foundParenIdx == 1) {
            step = -1;
         }

         for (int i=cursorPos + step; i >= 0 && i<sBuffer.Length(); i += step)
            {
               //if current char is equal to another opening bracket, push onto stack
               if (sBuffer[i] == bTypes[bIndex][foundParenIdx])
                  {
                     // push index of bracket
                     locBrackets.push(i);
                  }
               //if current char is equal to closing bracket
               else if (sBuffer[i] == bTypes[bIndex][1 - foundParenIdx])
                  {
                     // pop previous opening bracket off stack
                     locBrackets.pop();
                     // if previous opening was the last entry, then highlight match
                     if (locBrackets.empty())
                        {
                           colorBrackets(el, cursorPos, i, COLOR_BRACKET);
                           break;
                        }
                  }
            }
      }

   return index;
}


/**
 *      Highlight a word within the buffer.
 *      Requires the start and end index of the word, and the color pair index (class or type).
 *      Writes colour info for each char in range to el->el_line.bufcol.
 *      Background colour is set to the same as the current terminal background colour.
 *      Foreground (text) colour is set according to the type of word being highlighted (e.g. class or type).
 */
void colorWord(EditLine * el , int first, int num, int textColor)
{
   int bgColor = -1;            // default background

   // add colour information to el.
   for (int index = first; index < first + num; ++index)
      {
         el->el_line.bufcolor[index].foreColor = textColor;
         el->el_line.bufcolor[index].backColor = bgColor;       
         term__repaint(el, index);      
      }
}

/*
 *      Set the colour information in the editline buffer,
 *      Then call repaint to repaint the chars with the new colour information
 */
void colorBrackets(EditLine * el, int open, int close, int color)
{
   int bgColor = -1;            // default background

   el->el_line.bufcolor[open].foreColor = COLOR_BRACKET;
   el->el_line.bufcolor[open].backColor = bgColor;      
   term__repaint(el, open);     

   el->el_line.bufcolor[close].foreColor = COLOR_BRACKET;
   el->el_line.bufcolor[close].backColor = bgColor;
   term__repaint(el, close);    
}

