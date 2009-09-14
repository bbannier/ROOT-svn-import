#include "histedit.h"

void setKeywordColors(const char* colorTab, const char* colorBracket,
                      const char* colorBadBracket);
int selectColor(const char* str);
void highlightKeywords(EditLine* el);
int matchParentheses(EditLine* el);
void colorWord(EditLine* el, int first, int last, int color);
void colorBrackets(EditLine* el, int open, int close, int color);
