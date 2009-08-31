/* Some documentation! 
 * Integrate NCurses!
*/
#include "TROOT.h"
#include "el.h"
#include <ncurses.h>
#include "TRegexp.h"
#include <stack>

using namespace std;

// int values for colour pairs
#define COLOR_CLASS	   4		// NCurses COLOR_BLUE
#define COLOR_TYPE	   1		// NCurses COLOR_RED
#define COLOR_BRACKET  2		// NCurses COLOR_GREEN

void highlightKeywords(EditLine * el);
int matchParentheses(EditLine * el);
void colorWord(EditLine * el, int first, int last, int color);
void colorBrackets(EditLine * el, int open, int close, int color);


/*
 *	Use gRoot to establish keywords known to root. 
 *	
 */
void highlightKeywords(EditLine * el) {	

	TString sBuffer = "";
	for (char* c=el->el_line.buffer; c<el->el_line.lastchar; c++)
	{
		sBuffer.Append(*c);
	}

	// regular expression to enable the buffer to be split into potential class names
	TRegexp regex = TRegexp("[a-zA-Z0-9_]+");

	// regular expression inverse of match expression to find end of match
	//TRegexp antiRegex = TRegexp("^[a-zA-Z0-9_]");
	TRegexp antiRegex = TRegexp("[^a-z^A-Z^0-9^_]");

	TString keyword;
	Ssiz_t index=0, nindex=0, start=0;
	bool matchFound = false;
	
	// only check against keywords if a complete word has been entered
	// i.e. terminated by a character not matching the regular expression
	nindex = sBuffer.Index(antiRegex, start);
	if (nindex > -1)
	{
		// while still more substrings match the pattern:
		// keyword is the substring from the index of the match until there is no more match
		// the new start is the index of the non-matching char after the match.
		while (index > -1)
		{
			nindex = sBuffer.Index(antiRegex, index);
			if (nindex > -1)
			{	
				// keyword is the substring from the regex index to the non regex index
				keyword = sBuffer(index, nindex-index);

				// don't really want to do this stuff every time: static initialise?
				// get list of class names known to ROOT
				TIter iterClasses(gROOT->GetListOfClasses());
				//TClassTable::
				TObject * classLine;
				TString className;
				// get list of type names known to ROOT
				TIter iterTypes(gROOT->GetListOfTypes());
				TObject * typeLine;
				TString typeName;

				// check each keyword against being a class or type
				// If performing search on a per-word basis, break on match
				while (!matchFound && (classLine = (TObject *) iterClasses.Next()) )
				{
					className = classLine->GetName();

					index = sBuffer.Index(className);
					if ( index > -1)
					{
						colorWord(el, index, className.Length()+index, COLOR_CLASS);
						matchFound = true;
					}
				}
				// don't bother with matching for type names if match already found
				while (!matchFound && (typeLine = (TObject *) iterTypes.Next()) )
				{
					typeName = typeLine->GetName();

					// need to trim to after last ::
					/*TString split = "::";
					int splitPos=-1, lastSplit=-1;
					do {
						lastSplit = splitPos;
						// the next line doesnt work yet, need Ssiz_t instead of int, plus case stuff.
						//splitPos = typeName.Index(split, len, lastSplit);
					} while ( splitPos > -1 );
					if ( lastSplit > -1 )
					{
						// split the string at this position, otherwise leave it be
						int start = lastSplit + split.Length();
						typeName = typeName(start, typeName.Length()-start);
					}*/


					index = sBuffer.Index(typeName);
					if ( index > -1)
					{
						colorWord(el, index, typeName.Length()+index, COLOR_TYPE);
						matchFound = true;
					}
				}

				// start from this point on next iteration
				start = nindex;
			}
			else
			{
				// if there are no more non-matching chars, the keyword lasts until the end of the buffer
				keyword = sBuffer(index, sBuffer.Length()-index);
				start = sBuffer.Length();	// hope this doesnt crash it!
			}
			// get the start of the next matching character after the previous non-matching one
			index = sBuffer.Index(regex, start);
		}
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

	/* CURRENT STUFF
	// create the substring from the currently selected char to the end of the buffer
	TString sBuffer = "";
	for (char* c=el->el_line.cursor; c<el->el_line.lastchar; c++)
	{
		sBuffer.Append(*c);
	}

	// char* stack for pointers to locations of brackets
	stack<int> locBrackets;

	if (sBuffer.Length() > 0)
	{
		int openIndex = el->el_line.cursor - el->el_line.buffer;
		bool openFound = false;

		// check against each bracket type
		int bIndex = 0;
		for (bIndex = 0; bIndex<amtBrackets && !openFound; bIndex++)
		{
			// if current char is equal to opening bracket
			if (sBuffer[openIndex] == bTypes[bIndex][0])
			{
				locBrackets.push(openIndex);
				openFound = true;
			}
		}

		// current cursor char is not an open bracket, therefore no need to search
		if ( !openFound ) 
		{
			return -1;
		}

		// iterate through remaining letters until find a matching closing bracket 
		// if another open bracket of the same type is found, push onto stack 
		// and pop on next closing bracket match
		for (int i=openIndex; i<sBuffer.Length(); i++)
		{
			//if current char is equal to another opening bracket, push onto stack
			if (sBuffer[i] == bTypes[bIndex][0])
			{
				// push index of bracket
				locBrackets.push(i);
			}
			//if current char is equal to closing bracket
			else if (sBuffer[i] == bTypes[bIndex][1])
			{
				// pop previous opening bracket off stack
				locBrackets.pop();
				// if previous opening was the last entry, then highlight match
				if (locBrackets.empty())
				{
					colorBrackets(el, openIndex, i, COLOR_BRACKET);
					break;
				}
			}
		}
	}
*/
	return index;
}


/**
 *	Highlight a word within the buffer. 
 *	Requires the start and end index of the word, and the color pair index (class or type). 
 *	Writes colour info for each char in range to el->el_line.bufcol.
 *	Background colour is set to the same as the current terminal background colour.
 *	Foreground (text) colour is set according to the type of word being highlighted (e.g. class or type).
 */
void colorWord(EditLine * el , int first, int last, int textColor) 
{	
	int bgColor = -1;		// default background

	// add colour information to el.
	for (int index = first; index <= last; ++index)
	{
		el->el_line.bufcolor[index].foreColor = textColor;
		el->el_line.bufcolor[index].backColor = bgColor;	
		term__repaint(el, index);	
	}
}

/*
 *	Set the colour information in the editline buffer, 
 *	Then call repaint to repaint the chars with the new colour information
 */
void colorBrackets(EditLine * el, int open, int close, int color)
{
	int bgColor = -1;		// default background

	el->el_line.bufcolor[open].foreColor = COLOR_BRACKET;
	el->el_line.bufcolor[open].backColor = bgColor;	
	term__repaint(el, open);	

	el->el_line.bufcolor[close].foreColor = COLOR_BRACKET;
	el->el_line.bufcolor[close].backColor = bgColor;
	term__repaint(el, close);	
}

