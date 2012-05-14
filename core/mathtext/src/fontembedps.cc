// mathtext - A TeX/LaTeX compatible rendering library. Copyright (C)
// 2008-2012 Yue Shi Lai <ylai@users.sourceforge.net>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301 USA

#include "fontembed.h"
#include <cstring>
#include <cstdio>

// ROOT integration
#include "RConfig.h"
#ifdef R__BYTESWAP
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif // LITTLE_ENDIAN
#include "Byteswap.h"
#define bswap_16(x)	Rbswap_16((x))
#define bswap_32(x)	Rbswap_32((x))
#else // R__BYTESWAP
#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif // LITTLE_ENDIAN
#endif // R__BYTESWAP

// References:
//
// Adobe Systems, Inc., PostScript language Document Structuring
// Convention specification (Adobe Systems, Inc., San Jose, CA, 1992),
// version 3.0, section 5.1.
//
// Adobe Systems, Inc., PostScript language reference manual
// (Addison-Wesley, Reading, MA, 1999), 3rd edition, section 5.8.1.
//
// Adobe Systems, Inc., Adobe Type 1 Font Format (Addison-Wesley,
// Reading, MA, 1993), version 1.1
//
// Adobe Systems, Inc., The Compact Font Format specification, Adobe
// Technical Note 5176 (Adobe, Mountain View, CA, 2003), 4 December
// 2003 document
//
// Adobe Systems, Inc., Type 2 charstring format, Adobe Technical Note
// 5177 (Adobe, San Jose, CA, 2000), 16 March 2000 document

namespace mathtext {

	unsigned int font_embed_postscript_t::ascii85_line_count(
		const uint8_t *buffer, const size_t length)
	{
		const unsigned int width = 64;
		unsigned int column = 0;
		unsigned int line = 0;

		if (length >= 4) {
			for (size_t i = 0; i < length - 3; i += 4) {
				unsigned int b = reinterpret_cast<
					const unsigned int *>(buffer)[i >> 2];

				if (b == 0) {
					column++;
					if (column == width - 1) {
						line++;
						column = 0;
					}
				}
				else {
					if (column + 5 >= width) {
						column += 5 - width;
						line++;
					}
					else {
						column += 5;
					}
				}
			}
		}
		if (column + (length & 3) + 3 >= width) {
			line++;
		}

		return line;
	}

	void font_embed_postscript_t::append_ascii85(
		std::string &ascii, const uint8_t *buffer,
		const size_t length)
	{
		const int width = 64;
		int column = 0;

		if (length >= 4) {
			for (size_t i = 0; i < length - 3; i += 4) {
				unsigned int dword = reinterpret_cast<
					const unsigned int *>(buffer)[i >> 2];

				if (dword == 0) {
					ascii.append(1, 'z');
					column++;
					if (column == width - 1) {
						ascii.append(1, '\n');
						column = 0;
					}
				}
				else {
#ifdef LITTLE_ENDIAN
					dword = bswap_32(dword);
#endif // LITTLE_ENDIAN

					char str[5];

					str[4] = static_cast<char>(dword % 85 + '!');
					dword /= 85;
					str[3] = static_cast<char>(dword % 85 + '!');
					dword /= 85;
					str[2] = static_cast<char>(dword % 85 + '!');
					dword /= 85;
					str[1] = static_cast<char>(dword % 85 + '!');
					dword /= 85;
					str[0] = static_cast<char>(dword % 85 + '!');
					for (size_t j = 0; j < 5; j++) {
						ascii.append(1, str[j]);
						column++;
						if(column == width) {
							ascii.append(1, '\n');
							column = 0;
						}
					}
				}
			}
		}

		int k = length & 3;

		if(k > 0) {
			unsigned int dword = 0;

			memcpy(&dword, buffer + (length & ~3), k);
#ifdef LITTLE_ENDIAN
			dword = bswap_32(dword);
#endif // LITTLE_ENDIAN

			char str[5];

			str[4] = static_cast<char>(dword % 85 + '!');
			dword /= 85;
			str[3] = static_cast<char>(dword % 85 + '!');
			dword /= 85;
			str[2] = static_cast<char>(dword % 85 + '!');
			dword /= 85;
			str[1] = static_cast<char>(dword % 85 + '!');
			dword /= 85;
			str[0] = static_cast<char>(dword % 85 + '!');
			for(int j = 0; j < k + 1; j++) {
				ascii.append(1, str[j]);
				column++;
				if(column == width) {
					ascii.append(1, '\n');
					column = 0;
				}
			}

		}
		if(column > width - 2)
			ascii.append(1, '\n');
		ascii.append("~>");
	}

	std::string font_embed_postscript_t::font_embed_type_2(
		std::string &font_name,
		const std::vector<unsigned char> &font_data)
	{
		// Embed an OpenType CFF (Type 2) file in ASCII85 encoding
		// with the PostScript syntax

		unsigned short cid_encoding_id;
		unsigned int cff_offset;
		unsigned int cff_length;

		if(!parse_otf_cff_header(font_name, cid_encoding_id,
								 cff_offset, cff_length,
								 font_data))
			return std::string();

		std::vector<unsigned char> cff;

		cff.resize(cff_length + 10);
		memcpy(&cff[0], "StartData\r", 10);
		memcpy(&cff[10], &font_data[cff_offset], cff_length);

		char linebuf[BUFSIZ];
		std::string ret;

		snprintf(linebuf, BUFSIZ, "%%%%BeginResource: FontSet (%s)\n",
				 font_name.c_str());
		ret.append(linebuf);
		ret.append("%%VMusage: 0 0\n");
		ret.append("/FontSetInit /ProcSet findresource begin\n");
		snprintf(linebuf, BUFSIZ, "%%%%BeginData: %u ASCII Lines\n",
				 ascii85_line_count(&cff[0], cff_length) + 2);
		ret.append(linebuf);
		snprintf(linebuf, BUFSIZ,
				 "/%s %u currentfile /ASCII85Decode filter cvx exec\n",
				 font_name.c_str(), cff_length);
		ret.append(linebuf);
		append_ascii85(ret, &cff[0], cff_length + 10);
		ret.append(1, '\n');
		ret.append("%%EndData\n");
		ret.append("%%EndResource\n");

		return ret;
	}

}
