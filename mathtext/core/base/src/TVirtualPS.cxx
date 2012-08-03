// @(#)root/base:$Id$
// Author: Rene Brun   05/09/99

/*************************************************************************
 * Copyright (C) 1995-2000, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

//______________________________________________________________________________
//
// TVirtualPS is an abstract interface to a Postscript, PDF and SVG drivers
//

#include "Riostream.h"
#include "Byteswap.h"
#include "TError.h"
#include "TClonesArray.h"
#include "TVirtualPS.h"
#include "fontembed.h"

TVirtualPS *gVirtualPS = 0;

const Int_t  kMaxBuffer = 250;

ClassImp(TVirtualPS)


//______________________________________________________________________________
TVirtualPS::TVirtualPS()
{
   // VirtualPS default constructor.

   fStream    = 0;
   fNByte     = 0;
   fSizBuffer = kMaxBuffer;
   fBuffer    = new char[fSizBuffer+1];
   fLenBuffer = 0;
   fPrinted   = kFALSE;
   fImplicitCREsc = 0;
}


//______________________________________________________________________________
TVirtualPS::TVirtualPS(const char *name, Int_t)
          : TNamed(name,"Postscript interface")
{
   // VirtualPS constructor.

   fStream    = 0;
   fNByte     = 0;
   fSizBuffer = kMaxBuffer;
   fBuffer    = new char[fSizBuffer+1];
   fLenBuffer = 0;
   fPrinted   = kFALSE;
   fImplicitCREsc = 0;
}


//______________________________________________________________________________
TVirtualPS::~TVirtualPS()
{
   // VirtualPS destructor

   if (fBuffer) delete [] fBuffer;
}


//______________________________________________________________________________
void TVirtualPS::PrintStr(const char *str)
{
   // Output the string str in the output buffer

   if (!str || !str[0])
      return;
   Int_t len = strlen(str);
   while (len) {
      if (str[0] == '@') {
         if (fLenBuffer) {
            fStream->write(fBuffer, fLenBuffer);
            fNByte += fLenBuffer;
            fLenBuffer = 0;
            fStream->write("\n", 1);
            fNByte++;
            fPrinted = kTRUE;
         }
         len--;
         str++;
      } else {
         Int_t lenText = len;
         if (str[len-1] == '@') lenText--;
         PrintFast(lenText, str);
         len -= lenText;
         str += lenText;
      }
   }
}


//______________________________________________________________________________
void TVirtualPS::PrintFast(Int_t len, const char *str)
{
   // Fast version of Print
   if (!len || !str) return;
   while ((len + fLenBuffer) > kMaxBuffer) {
      Int_t nWrite = kMaxBuffer;
      if (fImplicitCREsc) {
         if (fLenBuffer > 0) nWrite = fLenBuffer;
      } else {
         if ((len + fLenBuffer) > nWrite) {
            // Search for the nearest preceeding space to break a line, if there is no instruction to escape the <end-of-line>.
            while ((nWrite >= fLenBuffer) && (str[nWrite - fLenBuffer] != ' ')) nWrite--;
            if (nWrite < fLenBuffer) {
               while ((nWrite >= 0) && (fBuffer[nWrite] != ' ')) nWrite--;
            }
            if (nWrite <= 0) {
               // Cannot find a convenient place to break a line, so we just break at this location.
               nWrite = kMaxBuffer;
            }
         }
      }
      if (nWrite >= fLenBuffer) {
         if (fLenBuffer > 0) {
            fStream->write(fBuffer, fLenBuffer);
            fNByte += fLenBuffer;
            nWrite -= fLenBuffer;
            fLenBuffer = 0;
         }
         if (nWrite > 0) {
            fStream->write(str, nWrite);
            len -= nWrite;
            str += nWrite;
            fNByte += nWrite;
         }
      } else {
         if (nWrite > 0) {
            fStream->write(fBuffer, nWrite);
            fNByte += nWrite;
            memmove(fBuffer, fBuffer + nWrite, fLenBuffer - nWrite); // not strcpy because source and destination overlap
            fBuffer[fLenBuffer - nWrite] = 0; // not sure if this is needed, but just in case
            fLenBuffer -= nWrite;
         }
      }
      if (fImplicitCREsc) {
         // Write escape characters (if any) before an end-of-line is enforced.
         // For example, in PostScript the <new line> character must be escaped inside strings.
         Int_t crlen = strlen(fImplicitCREsc);
         fStream->write(fImplicitCREsc, crlen);
         fNByte += crlen;
      }
      fStream->write("\n",1);
      fNByte++;
   }
   if (len > 0) {
      strlcpy(fBuffer + fLenBuffer, str, len+1);
      fLenBuffer += len;
      fBuffer[fLenBuffer] = 0;
   }
   fPrinted = kTRUE;
}


//______________________________________________________________________________
void TVirtualPS::WriteInteger(Int_t n, Bool_t space )
{
   // Write one Integer to the file
   //
   // n: Integer to be written in the file.
   // space: If TRUE, a space in written before the integer.

   char str[15];
   if (space) {
      snprintf(str,15," %d", n);
   } else {
      snprintf(str,15,"%d", n);
   }
   PrintStr(str);
}


//______________________________________________________________________________
void TVirtualPS::WriteReal(Float_t z)
{
   // Write a Real number to the file

   char str[15];
   snprintf(str,15," %g", z);
   PrintStr(str);
}

//______________________________________________________________________________
void TVirtualPS::PrintRaw(Int_t len, const char *str)
{
	fNByte += len;
	if((len + fLenBuffer) > kMaxBuffer - 1) {
		fStream->write(fBuffer, fLenBuffer);
		while(len > kMaxBuffer-1) {
			fStream->write(str,kMaxBuffer);
			len -= kMaxBuffer;
			str += kMaxBuffer;
		}
		memcpy(fBuffer, str, len);
		fLenBuffer = len;
	}
	else {
		memcpy(fBuffer + fLenBuffer, str, len);
		fLenBuffer += len;
	}
	fPrinted = kTRUE;
}

//______________________________________________________________________________
void TVirtualPS::WriteASCII(Int_t /*len*/, const char */*buf*/)
{
}

//______________________________________________________________________________
void TVirtualPS::WriteASCIIHex(Int_t len, const char *buf)
{
	const int width = 64;
	int column = 0;

	for(int i = 0; i < len; i++) {
		char str[3];

		snprintf(str, 3, "%02hhX", buf[i]);
		PrintRaw(2, str);
		column += 2;
		if(column >= width) {
			PrintRaw(1, "\n");
			column = 0;
		}
	}
}

//______________________________________________________________________________
Int_t TVirtualPS::ASCII85LineCount(Int_t len, const char *buf) const
{
	const int width = 64;
	int column = 0;
	int line = 0;

	for(int i = 0; i < len - 3; i += 4) {
		unsigned int b = ((unsigned int *)buf)[i >> 2];

		if(b == 0) {
			column++;
			if(column == width - 1) {
				line++;
				column = 0;
			}
		}
		else {
			if(column + 5 >= width) {
				column += 5 - width;
				line++;
			}
			else
				column += 5;
		}
	}
	if(column + (len & 3) + 3 >= width)
		line++;

	return line;
}

//______________________________________________________________________________
void TVirtualPS::WriteASCII85(Int_t len, const char *buf)
{
	const int width = 64;
	int column = 0;

	for(int i = 0; i < len - 3; i += 4) {
		unsigned int dword = ((unsigned int *)buf)[i >> 2];

		if(dword == 0) {
			PrintRaw(1, "z");
			column++;
			if(column == width - 1) {
				PrintRaw(1, "\n");
				column = 0;
			}
		}
		else {
#ifdef R__BYTESWAP
			dword = Rbswap_32(dword);
#endif // R__BYTESWAP

			char str[5];

			str[4] = (char)(dword % 85 + '!'); dword /= 85;
			str[3] = (char)(dword % 85 + '!'); dword /= 85;
			str[2] = (char)(dword % 85 + '!'); dword /= 85;
			str[1] = (char)(dword % 85 + '!'); dword /= 85;
			str[0] = (char)(dword % 85 + '!');
			for(int j = 0; j < 5; j++) {
				PrintRaw(1, str + j);
				column++;
				if(column == width) {
					PrintRaw(1, "\n");
					column = 0;
				}
			}
		}
	}

	int k = len & 3;

	if(k > 0) {
		unsigned int dword = 0;

		memcpy(&dword, buf + (len & ~3), k);
#ifdef R__BYTESWAP
		dword = Rbswap_32(dword);
#endif // R__BYTESWAP

		char str[5];

		str[4] = (char)(dword % 85 + '!'); dword /= 85;
		str[3] = (char)(dword % 85 + '!'); dword /= 85;
		str[2] = (char)(dword % 85 + '!'); dword /= 85;
		str[1] = (char)(dword % 85 + '!'); dword /= 85;
		str[0] = (char)(dword % 85 + '!');
		for(int j = 0; j < k + 1; j++) {
			PrintRaw(1, str + j);
			column++;
			if(column == width) {
				PrintRaw(1, "\n");
				column = 0;
			}
		}

	}
	if(column > width - 2)
		PrintRaw(1, "\n");
	PrintRaw(2, "~>");
}

Bool_t TVirtualPS::ReadOTFCFFHeader(
	FILE *fp, TString &fontName, UInt_t &cffOffset,
	UInt_t &cffLength)
{
	const std::vector<unsigned char> fontData =
		mathtext::font_embed_t::read_font_data(fp);
	std::string fontNameStl;
	unsigned short cidEncodingId;
	Bool_t retval = mathtext::font_embed_t::parse_otf_cff_header(
		fontNameStl, cidEncodingId, cffOffset, cffLength, fontData);

	if(retval) {
		fontName = fontNameStl.c_str();
	}

	return retval;
}

Bool_t TVirtualPS::ReadTTFHeader(
	FILE *fp, TString &fontName, Double_t *fontBBox,
	UShort_t *encoding, TString *charStrings, UShort_t *cMap)
{
	const std::vector<unsigned char> fontData =
		mathtext::font_embed_t::read_font_data(fp);
	std::string fontNameStl;
	std::map<wchar_t, uint16_t> cMapStl;
	std::vector<std::string> charStringsStl;
	Bool_t retval = mathtext::font_embed_t::parse_ttf_header(
		fontNameStl, fontBBox, cMapStl, charStringsStl, fontData);

	if(retval) {
		fontName = fontNameStl.c_str();
		for(size_t i = 0; i < 65536; i++) {
			std::map<wchar_t, uint16_t>::const_iterator iterator =
				cMapStl.find(static_cast<wchar_t>(i));

			if(iterator != cMapStl.end()) {
				if(iterator->second < charStringsStl.size()) {
					encoding[i] = iterator->second;
					charStrings[i] =
						charStringsStl[iterator->second].c_str();
				}
				else {
					fprintf(stderr, "%s:%d: warning: %lu -> %hu "
							"(size = %lu)\n", __FILE__, __LINE__, i,
							iterator->second, charStringsStl.size());
				}
			}
		}

		size_t offset = 0;

		for(std::map<wchar_t, uint16_t>::const_iterator iterator =
				cMapStl.begin();
			iterator != cMapStl.end(); iterator++) {
			*reinterpret_cast<uint32_t *>(&(cMap[offset])) =
				iterator->first;
			cMap[offset + 2] = iterator->second;
			offset += 3;
		}
		*reinterpret_cast<uint32_t *>(&(cMap[offset])) = 0xffffffffU;
	}

	return retval;
}
