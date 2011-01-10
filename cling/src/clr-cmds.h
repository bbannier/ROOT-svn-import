// @(#)root/cint:$Id$
// Author: Zdenek Culik   16/10/2010

#ifndef __CLR_CMDS_H__
#define __CLR_CMDS_H__

#include "Reflex/Reflex.h"

#include <fstream> // clas std::istream
#include <vector>
#include <map>
// #include <exception>

using std::string;

/* ---------------------------------------------------------------------- */

class TTextInput
{
public:

   enum EToken {
      StartToken,

      IdentToken,     // identifier
      NumberToken,    // number
      CharToken,      // character in single quotes
      StringToken,    // string in double quotes
      SeparatorToken, // single character separator

      EofToken        // end of file
   };

   typedef unsigned char byte_t;

   static const int BufSize = 4 * 1024;
   static const int DuplMax = 256;

protected:
   std::istream * fInpStream;

   size_t fBufPos;
   size_t fBufLen;
   byte_t fBuf [BufSize];

   string fFileName;

   bool fReleaseStream;

private:
   int fLinePos; // line counter
   int fColPos;  // column counter
   int fCharPos; // character counter

   int fTokenLinePos;
   int fTokenColPos;
   int fTokenCharPos;

   char fInpCh;  // current inpput character
   char fPrevCh; // previous character (for end of line detection)
   bool fPrevEol; // end of line detected (but LinePos will be incremented with next character)
   bool fInpEof; // end of source text

   int  fDuplLen;
   char fDuplBuf [DuplMax + 1]; // copy of current token

   int  fValueLen;
   char fValueBuf [DuplMax + 1]; // string value

public:
   EToken fTokenKind;

   string fTokenTxt; // original token
   string fTokenVal; // value of string

private:
   void Init();

   void ReadBuf();
   void ReadData(void * adr, size_t size);

public:
   void Open(std::istream & stream);
   void Open(string name);

   void Close();

   string GetFileName() { return fFileName; }
   void SetFileName(const string p_filename) { fFileName = p_filename; }

   int GetTokenLine() { return fTokenLinePos; }
   int GetTokenCol() { return fTokenColPos; }

   void Info(const string msg);
   void Error(const string msg);

private:
   void GetCh();

public:
private:
   void NextCh();

   void StoreCharacter(char c);
   char Character();

   void GetIdent();
   void GetNumber();
   void GetChar();
   void GetString();

   void StoreSeparator(char ch);
   void GetSeparator();

   void SkipLine();
   void SkipWhiteSpace();

public:
   void GetToken();

   bool IsIdent() { return fTokenKind == IdentToken; }
   bool IsNumber() { return fTokenKind == NumberToken; }
   bool IsChar() { return fTokenKind == CharToken; }
   bool IsString() { return fTokenKind == StringToken; }
   bool IsSeparator() { return fTokenKind == SeparatorToken; }
   bool IsEof() { return fTokenKind == EofToken;}

   bool IsIdent(const string par);
   bool IsSeparator(char par);

   void CheckIdent(const string par);
   void CheckSeparator(char par);
   void CheckEof();

   string ReadIdent();
   char   ReadChar();
   string ReadNumber();
   string ReadSignedNumber();  // including minus sign
   string ReadString();

   TTextInput();
   virtual ~ TTextInput();
};

/* ---------------------------------------------------------------------- */

class TReflexCommands : public TTextInput {
private:

   struct TLocalObject
   {
       Reflex::Type type;
       Reflex::Object obj;
   };

   typedef std::map<string,TLocalObject*> name_table_t;

   typedef void* param_t;
   typedef std::vector<param_t> param_list_t;

private:
   name_table_t fNames;

private:
   TLocalObject * FindName(string name);

   void Parameter(param_list_t& list);
   void Parameters(param_list_t& list);
   // void Clean(param_list_t& list);

   void AssignStatement(string name);
   void CallStatement(string name);
   void Statement();

public:
   void Statements();
};

/* ---------------------------------------------------------------------- */

#endif /* __CLR_CMDS_H__ */
