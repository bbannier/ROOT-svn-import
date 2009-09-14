#ifdef __sun
# include R__CURSESHDR
extern "C" {
   // cannot #include term.h because it #defines move() etc
char *tparm(char*, long, long, long, long, long, long, long, long, long);
char *tigetstr(char*);
char *tgoto(char*, int, int);
int   tputs(char*, int, int (*)(int));
int tgetflag(char*);
int tgetnum(char*);
char* tgetstr(char*, char**);
int tgetent(char*, const char*);
}
// un-be-lievable.
# undef erase
# undef move
# undef clear
# undef del
# undef key_end
# undef key_clear
# undef key_print
#else
# include R__CURSESHDR
# include <termcap.h>
# include <termcap.h>
extern "C" int setupterm(const char* term, int fd, int* perrcode);
#endif
