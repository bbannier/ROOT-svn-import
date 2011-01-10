// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/10/2010

#include "clr-cmds.h"

#include <string.h> // function memset
#include <assert.h>
#include <iostream> // variable std::cerr
#include <stdexcept> // class std::runtime_error

using std::ifstream;

const char cr = '\r';
const char lf = '\n';
const char tab = '\t';
const char quote1 = '\'';
const char quote2 = '\"';
const char backslash = '\\';

inline bool IsLetter (char ch)
{
   return (ch >= 'A' && ch <= 'Z') ||
          (ch >= 'a' && ch <= 'z') ||
          (ch == '_');
}

inline bool IsDigit (char ch)
{
   return ch >= '0' && ch <= '9';
}

inline bool IsOctal (char ch)
{
   return ch >= '0' && ch <= '7';
}

inline bool IsHexDigit (char ch)
{
   return (ch >= '0' && ch <= '9') ||
          (ch >= 'a' && ch <= 'f') ||
          (ch >= 'A' && ch <= 'F');
}

inline bool IsLetterOrDigit (int ch)
{
   return IsLetter (ch) || IsDigit (ch);
}

inline string CharToStr (char c)
// convert one character to string
{
   return string (1, c);
}

/******************************* TEXT INPUT *******************************/

//______________________________________________________________________________
TTextInput::TTextInput() :
      fInpStream(NULL),
      fBufPos(0),
      fBufLen(0),
      // ignore fBuf
      fFileName(""),
      fReleaseStream(false),

      fLinePos(0),
      fColPos(0),
      fCharPos(0),

      fTokenLinePos(0),
      fTokenColPos(0),
      fTokenCharPos(0),

      fInpCh(0),
      fPrevCh(0),
      fPrevEol(false),
      fInpEof(false),

      fDuplLen(0),
      fDuplBuf(),

      fValueLen(0),
      fValueBuf(),

      fTokenKind(StartToken),

      fTokenTxt(""),
      fTokenVal("")
{
}

//______________________________________________________________________________
TTextInput::~TTextInput()
{
   if (fReleaseStream && fInpStream != NULL) {
      delete fInpStream;
      fInpStream = NULL;
   }
}

//______________________________________________________________________________
void TTextInput::Init()
{
   fBufPos = 0;
   fBufLen = 0;
   // do not change already initialized fInpStream, fFileName and fReleaseStream

   fInpCh = 0;
   fPrevCh = 0;
   fPrevEol = false;
   fInpEof = false;

   fLinePos = 1;
   fColPos = 0;
   fCharPos = 0;

   fTokenLinePos = 1;
   fTokenColPos = 0;
   fTokenCharPos = 0;

   // not necessary
   fDuplLen = 0;
   memset(fDuplBuf, 0, sizeof(fDuplBuf));

   fValueLen = 0;
   memset(fValueBuf, 0, sizeof(fValueBuf));

   fTokenKind = StartToken;
   fTokenTxt = "";
   fTokenVal = "";

   // read first character
   GetCh();
}

/********************************** OPEN **********************************/

//______________________________________________________________________________
void TTextInput::Open(std::istream & stream)
{
   fInpStream = & stream;
   assert(fInpStream != NULL);
   Init();
}

//______________________________________________________________________________
void TTextInput::Open(string name)
{
   SetFileName(name);

   ifstream * tmp = new ifstream;
   tmp->open(name.c_str());

   if (!tmp->good()) {
      Error("Cannot open " + name);
      delete tmp;
   } else {
      fReleaseStream = true;
      Open(*tmp);
   }
}

//______________________________________________________________________________
void TTextInput::Close()
{
}

//______________________________________________________________________________
void TTextInput::Info(const string msg)
{
   // !? !!
   std::cerr << "CLR " << msg << std::endl;
}

//______________________________________________________________________________
void TTextInput::Error(const string msg)
{
   // !? !!
   std::cerr << "CLR " << msg << std::endl;
   throw new std::runtime_error ("error: " + msg);
}

//______________________________________________________________________________
void TTextInput::ReadBuf()
{
   assert(fInpStream != NULL);
   if (! fInpStream->good()) {  // important for string streams
      fBufLen = 0;
   } else {
      fInpStream->read(static_cast <char *>(static_cast <void *>(& fBuf)), BufSize);
      fBufLen = fInpStream->gcount();
   }
   fBufPos = 0;
}

//______________________________________________________________________________
void TTextInput::ReadData(void * adr, size_t size)
{
   size_t step;
   assert(fBufPos >= 0);
   assert(fBufPos <= fBufLen);

   assert(fBufLen >= 0);
   assert(fBufLen <= BufSize);

   while (size > 0) {
      if (fBufPos >= fBufLen) {
         ReadBuf();
         if (fBufLen == 0)
            Error("End of file");
      }

      step = fBufLen - fBufPos;
      if (step > size)
         step = size;

      assert(step > 0);
      assert(step <= size);

      assert(fBufPos >= 0);
      assert(fBufPos + step <= fBufLen);

      assert(fBufLen >= 0);
      assert(fBufLen <= BufSize);

      memcpy(adr, & fBuf[fBufPos], step);
      fBufPos += step;
      size -= step;

      adr = (void *)((char *)(adr) + step);

      assert(size >= 0);
   }
}

/******************************* CHAR INPUT *******************************/

//______________________________________________________________________________
void TTextInput::GetCh()
{
   // assert (fBufPos >= 0);
   // assert (fBufPos <= fBufLen);

   // assert (fBufLen >= 0);
   // assert (fBufLen <= BufSize);

   if (fBufPos >= fBufLen) {
      if (fInpEof) {
         Error("End of file");
      } else {
         ReadBuf();
         fInpEof = (fBufLen == 0);
      }
   }

   if (fInpEof)
      fInpCh = 0;
   else {
      // assert (fBufPos >= 0);
      // assert (fBufPos < fBufLen);

      // assert (fBufLen >= 0);
      // assert (fBufLen <= BufSize);

      fInpCh = (char)(fBuf[fBufPos]);
      fBufPos ++;
   }

   // assert (fBufPos >= 0);
   // assert (fBufPos <= fBufLen);

   // assert (fBufLen >= 0);
   // assert (fBufLen <= BufSize);

   if (fPrevEol) {
      fLinePos ++;
      fColPos = 0;
      fPrevEol = false;
   }

   if (fInpCh == cr) {
      fPrevEol = true;
      fColPos ++;
   } else if (fInpCh == lf) {
      if (fPrevCh != cr) {
         fPrevEol = true;
         fColPos ++;
      }
   } else {
      fColPos ++;
   }

   fCharPos ++;
   fPrevCh = fInpCh;
}

//______________________________________________________________________________
void TTextInput::NextCh()
{
   if (fDuplLen >= DuplMax) {
      Error("Token too long");
   } else {
      fDuplLen ++;
      fDuplBuf [fDuplLen-1] = fInpCh;
   }

   GetCh();
}

//______________________________________________________________________________
void TTextInput::StoreCharacter(char c)
{
   if (fValueLen >= DuplMax) {
      Error("Token too long");
   } else {
      fValueLen ++;
      fValueBuf [fValueLen-1] = c;
   }
}

//______________________________________________________________________________
void TTextInput::GetIdent()
{
   fDuplLen = 0;

   while (IsLetterOrDigit(fInpCh))
      NextCh();

   // set length
   fDuplBuf [fDuplLen] = 0;

   fTokenTxt = string(fDuplBuf);
   fTokenVal = "";
   fTokenKind = IdentToken;
}

//______________________________________________________________________________
void TTextInput::GetNumber()
{
   bool hex = false;
   bool any_digit = false;
   bool floating_point = false;

   fDuplLen = 0;

   if (fInpCh == '0') {
      NextCh();  // first digit
      any_digit = true;

      if (fInpCh == 'x' || fInpCh == 'X') {
         NextCh();  // store 'x'
         hex = true;
         while (IsHexDigit(fInpCh))
            NextCh();
      }
   }

   if (! hex) {
      while (IsDigit(fInpCh)) {
         any_digit = true;
         NextCh();
      }

      if (fInpCh == '.') {
         NextCh();  // decimal point
         floating_point = true;

         while (IsDigit(fInpCh)) {
            any_digit = true;
            NextCh();
         }
      }

      if (any_digit && (fInpCh == 'E' || fInpCh == 'e')) {
         NextCh();
         floating_point = true;

         if (fInpCh == '+' || fInpCh == '-')
            NextCh();

         if (! IsDigit(fInpCh))
            Error("Digit expected");

         while (IsDigit(fInpCh))
            NextCh();
      }
   }

   if (any_digit) {
      if (floating_point)
         while (fInpCh == 'f' || fInpCh == 'F' || fInpCh == 'l' || fInpCh == 'L')
            NextCh();
      else
         while (fInpCh == 'u' || fInpCh == 'U' || fInpCh == 'l' || fInpCh == 'L')
            NextCh();
   }

   // set length
   fDuplBuf [fDuplLen] = 0;

   if (any_digit) {
      fTokenTxt = string(fDuplBuf);
      fTokenVal = "";
      fTokenKind = NumberToken;
   } else {
      StoreSeparator('.');
   }
}

//______________________________________________________________________________
char TTextInput::Character()
// read one C-style character
{
   if (fInpCh != backslash) {           // simple character
      char last = fInpCh;
      NextCh();
      return last;
   } else {                             // escape sequence
      NextCh();                        // skip backslash

      if (IsOctal(fInpCh)) {            // octal
         int n = 0;
         int cnt = 1;
         while (IsOctal(fInpCh) && cnt <= 3) {
            n = n * 8 + fInpCh - '0';
            cnt++;
            NextCh();
         }
         return char(n);
      }

      else if (fInpCh == 'x' || fInpCh == 'X') {   // hex
         NextCh();
         int n = 0;
         while (IsHexDigit(fInpCh)) {
            int d;
            if (fInpCh >= 'A' && fInpCh <= 'F')
               d = fInpCh - 'A' + 10;
            else if (fInpCh >= 'a' && fInpCh <= 'f')
               d = fInpCh - 'a' + 10;
            else
               d = fInpCh - '0';
            n = n * 16 + d;
            NextCh();
         }
         return char(n);
      } else {
         char last = fInpCh;
         NextCh();
         switch (last) {               // other
            case 'a':
               return '\a';
            case 'b':
               return '\b';
            case 'f':
               return '\f';
            case 'n':
               return '\n';
            case 'r':
               return '\r';
            case 't':
               return '\t';
            case 'v':
               return '\v';

            case quote1:
               return last;
            case quote2:
               return last;
            case backslash:
               return last;
            case '?':
               return last;

            default:
               return last;
         }
      }
   }
}

//______________________________________________________________________________
void TTextInput::GetChar()
{
   fDuplLen = 0;
   fValueLen = 0;
   NextCh();  // skip quote

   // if (fInpCh == quote1)
   //    Error ("Empty character constant");

   while (fInpCh != quote1) {
      if (fInpEof || fInpCh == cr || fInpCh == lf)
         Error("Character constant exceeds line");

      char c = Character();
      StoreCharacter(c);
   }

   NextCh();  // skip quote

   // set length
   fDuplBuf [fDuplLen] = 0;
   fValueBuf [fValueLen] = 0;

   fTokenTxt = string(fDuplBuf);  // with quotes and escape sequences
   fTokenVal = string(fValueBuf);  // value without quotes and escape sequences
   fTokenKind = CharToken;
}

//______________________________________________________________________________
void TTextInput::GetString()
{
   fDuplLen = 0;
   fValueLen = 0;
   NextCh();  // skip quote

   while (fInpCh != quote2) {
      if (fInpEof || fInpCh == cr || fInpCh == lf)
         Error("String exceeds line");

      char c = Character();
      StoreCharacter(c);
   }

   NextCh();  // skip quote

   // set length
   fDuplBuf [fDuplLen] = 0;
   fValueBuf [fValueLen] = 0;

   fTokenTxt = string(fDuplBuf);  // string with quotes and escape sequences
   fTokenVal = string(fValueBuf);  // value without quotes and escape sequences
   fTokenKind = StringToken;
}

//______________________________________________________________________________
void TTextInput::StoreSeparator(char ch)
{
   fTokenTxt = ch;
   fTokenVal = "";
   fTokenKind = SeparatorToken;
}

//______________________________________________________________________________
void TTextInput::GetSeparator()
{
   char c = fInpCh;
   NextCh();
   StoreSeparator(c);
}

//______________________________________________________________________________
void TTextInput::SkipLine()
{
   while (! fInpEof && fInpCh != cr && fInpCh != lf)
      GetCh();

   if (! fInpEof && fInpCh == cr)
      GetCh();  // skip cr

   if (! fInpEof && fInpCh == lf)
      GetCh();  // skip lf
}

//______________________________________________________________________________
void TTextInput::SkipWhiteSpace()
{
again:

   while (! fInpEof && fInpCh <= ' ')        // WHITE SPACE
      GetCh();

   if (! fInpEof && fInpCh == '/') {
      GetCh();  // skip slash

      if (! fInpEof && fInpCh == '/') {      // SINGLE LINE COMMENT
         SkipLine();
         goto again;
      }

      else if (! fInpEof && fInpCh == '*') { // COMMENT
         if (! fInpEof)
            GetCh();  // skip asterisk

         char prev = ' ';
         while (! fInpEof && !(prev == '*' && fInpCh == '/')) {
            prev = fInpCh;
            GetCh();
         }

         if (! fInpEof)
            GetCh();  // skip slash

         goto again;
      }

      else StoreSeparator('/');            // SLASH
   }
}

//______________________________________________________________________________
void TTextInput::GetToken()
{
   SkipWhiteSpace();

   fTokenTxt = "";
   fTokenVal = "";

   if (fInpEof)                                 // END OF FILE
      fTokenKind = EofToken;

   else if (IsLetter(fInpCh))                   // IDENTIFIER
      GetIdent();

   else if (IsDigit(fInpCh))                    // NUMBER
      GetNumber();

   else if (fInpCh == '.')                      // NUMBER or PERIOD
      GetNumber();

   else if (fInpCh == quote1)                   // CHARACTER
      GetChar();

   else if (fInpCh == quote2)                   // STRING
      GetString();

   else                                        // SEPARATOR
      GetSeparator();
}

//______________________________________________________________________________
bool TTextInput::IsIdent(string par)
{
   return (fTokenKind == IdentToken) && (fTokenTxt == par);
}

//______________________________________________________________________________
bool TTextInput::IsSeparator(char par)
{
   return (fTokenKind == SeparatorToken) && (fTokenTxt[0] == par);
}

//______________________________________________________________________________
void TTextInput::CheckIdent(string par)
{
   if (fTokenKind != IdentToken || fTokenTxt != par)
      Error(par + " expected");
   GetToken();
}

//______________________________________________________________________________
void TTextInput::CheckSeparator(char par)
{
   if (fTokenKind != SeparatorToken || fTokenTxt[0] != par) {
      // Error (CharToStr (par) + " expected");
      Error(CharToStr(par) + " expected and found " + quote2 + fTokenTxt + quote2);
   }
   GetToken();
}

//______________________________________________________________________________
void TTextInput::CheckEof()
{
   if (fTokenKind != EofToken)
      Error("End of file expected");
}

//______________________________________________________________________________
string TTextInput::ReadIdent()
{
   string result;
   if (fTokenKind != IdentToken)
      Error("Identifier expected");
   result = fTokenTxt;
   GetToken();
   return result;
}

//______________________________________________________________________________
string TTextInput::ReadNumber()
{
   string result;
   if (fTokenKind != NumberToken)
      Error("Number expected");
   result = fTokenTxt;
   GetToken();
   return result;
}

//______________________________________________________________________________
string TTextInput::ReadSignedNumber()
{
   bool minus;
   string result;

   minus = false;
   if (fTokenKind == SeparatorToken && fTokenTxt [0] == '-') {
      minus = true;
      GetToken();
   }

   result = ReadNumber();

   if (minus)
      result = '-' + result;
   return result;
}

//______________________________________________________________________________
char TTextInput::ReadChar()
{
   char result;
   if (fTokenKind != CharToken)
      Error("Character constant expected");
   result = fTokenTxt [0];
   GetToken();
   return result;
}

//______________________________________________________________________________
string TTextInput::ReadString()
{
   string result;
   if (fTokenKind != StringToken)
      Error("String expected");
   result = fTokenVal;
   GetToken();
   return result;
}

/********************************** COMMANDS ***********************************/

//______________________________________________________________________________
TReflexCommands::TLocalObject * TReflexCommands::FindName(string name)
{
   TLocalObject * result = NULL;

   if (fNames.find (name) != fNames.end ())
       result = fNames[name];
   else
      Error ("Unknown identifier " + name);

   return result;
}

//______________________________________________________________________________
void TReflexCommands::Parameter(param_list_t& list)
{
   param_t value = NULL;

   if (IsIdent()) {
      string id = ReadIdent ();
      TLocalObject * container = FindName(id);
      value = & container->obj;
   } else if (IsNumber()) {
      ReadNumber ();
   } else if (IsChar()) {
      char c = ReadChar ();
   } else if (IsString()) {
      string s = ReadString ();
      value = strdup (s.c_str ()); // !? delete
   } else Error("Parameter expected");

   list.push_back(value);
}

//______________________________________________________________________________
void TReflexCommands::Parameters(param_list_t& list)
{
   CheckSeparator('(');

   if (! IsSeparator(')')) {
      Parameter(list);

      while (IsSeparator(',')) {
         GetToken();
         Parameter(list);
      }
   }

   CheckSeparator(')');
}

//______________________________________________________________________________
void TReflexCommands::AssignStatement(string name)
{
   CheckIdent("new");
   string type_name = ReadIdent();
   param_list_t params;
   if (IsSeparator('('))
      Parameters(params);

   Info ("clr-cmds: Calling " + name + " = new " + type_name);

   TLocalObject * container = new TLocalObject;

   container->type = Reflex::Type::ByName(type_name);
   if (container->type)
      container->obj = container->type.Construct();
   else
      Info ("clr-cmds:  UNKNOWN class " + type_name);

   // store identifier
   fNames[name] = container;

   Info ("clr-cmds: Finished " + name + " = new " + type_name);
}

//______________________________________________________________________________
void TReflexCommands::CallStatement(string name)
{
   string func = ReadIdent();
   param_list_t params;
   Parameters(params);
   Info ("clr-cmds: Calling " + name + "." + func);

   TLocalObject * container = FindName(name);

   Reflex::Member m = container->type.MemberByName(func);
   if (m)
      m.Invoke(container->obj);
   else
      Info ("clr-cmds:  UNKNOWN method " + name + "." + func);

   Info ("clr-cmds: Finished " + name + "." + func);
}

//______________________________________________________________________________
void TReflexCommands::Statement()
{
   string name = ReadIdent();
   if (IsSeparator('=')) {
      GetToken();
      AssignStatement(name);
   } else if (IsSeparator('.')) {
      GetToken();
      CallStatement(name);
   } else Error("'=' or '.' expected");
   CheckSeparator(';');
}

//______________________________________________________________________________
void TReflexCommands::Statements()
{
   GetToken(); // read first token

   while (!IsEof())
      Statement();
}
