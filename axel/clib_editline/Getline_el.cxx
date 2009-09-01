/* @(#)root/clib:$Id$ */
/* Author: */

/*
 * Copyright (C) 1991, 1992 by Chris Thewalt (thewalt@ce.berkeley.edu)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * copyright notice and this permission notice appear in supporting
 * documentation.  This software is provided "as is" without express or
 * implied warranty.
 */
/*
*************************** Motivation **********************************

Many interactive programs read input line by line, but would like to
provide line editing and history functionality to the end-user that
runs the program.

The input-edit package provides that functionality.  As far as the
programmer is concerned, the program only asks for the next line
of input. However, until the user presses the RETURN key they can use
emacs-style line editing commands and can traverse the history of lines
previously typed.

Other packages, such as GNU's readline, have greater capability but are
also substantially larger.  Input-edit is small, since it uses neither
stdio nor any termcap features, and is also quite portable.  It only uses
\b to backspace and \007 to ring the bell on errors.  Since it cannot
edit multiple lines it scrolls long lines left and right on the same line.

Input edit uses classic (not ANSI) C, and should run on any Unix
system (BSD or SYSV), PC's with the MSC compiler, or Vax/VMS (untested by me).
Porting the package to new systems basicaly requires code to read a
character when it is typed without echoing it, everything else should be OK.

I have run the package on:

        DECstation 5000, Ultrix 4.2 with cc and gcc
        Sun Sparc 2, SunOS 4.1.1, with cc
        SGI Iris, IRIX System V.3, with cc
        PC, DRDOS 5.0, with MSC 6.0

The description below is broken into two parts, the end-user (editing)
interface and the programmer interface.  Send bug reports, fixes and
enhancements to:

Chris Thewalt (thewalt@ce.berkeley.edu)
2/4/92

PS: I don't have, and don't want to add, a vi mode, sorry.

************************** End-User Interface ***************************

Entering printable keys generally inserts new text into the buffer (unless
in overwrite mode, see below).  Other special keys can be used to modify
the text in the buffer.  In the description of the keys below, ^n means
Control-n, or holding the CONTROL key down while pressing "n". M-B means
Meta-B (or Alt-B). Errors will ring the terminal bell.

^A/^E   : Move cursor to beginning/end of the line.
^F/^B   : Move cursor forward/backward one character.
^D      : Delete the character under the cursor.
^H, DEL : Delete the character to the left of the cursor.
^K      : Kill from the cursor to the end of line.
^L      : Redraw current line.
^O      : Toggle overwrite/insert mode. Initially in insert mode. Text
          added in overwrite mode (including yanks) overwrite
          existing text, while insert mode does not overwrite.
^P/^N   : Move to previous/next item on history list.
^R/^S   : Perform incremental reverse/forward search for string on
          the history list.  Typing normal characters adds to the current
          search string and searches for a match. Typing ^R/^S marks
          the start of a new search, and moves on to the next match.
          Typing ^H or DEL deletes the last character from the search
          string, and searches from the starting location of the last search.
          Therefore, repeated DEL's appear to unwind to the match nearest
          the point at which the last ^R or ^S was typed.  If DEL is
          repeated until the search string is empty the search location
          begins from the start of the history list.  Typing ESC or
          any other editing character accepts the current match and
          loads it into the buffer, terminating the search.
^T      : Toggle the characters under and to the left of the cursor.
^U      : Kill from beginning to the end of the line.
^Y      : Yank previously killed text back at current location.  Note that
          this will overwrite or insert, depending on the current mode.
M-F/M-B : Move cursor forward/backward one word.
M-D     : Delete the word under the cursor.
^SPC    : Set mark.
^W      : Kill from mark to point.
^X      : Exchange mark and point.
TAB     : By default adds spaces to buffer to get to next TAB stop
          (just after every 8th column), although this may be rebound by the
          programmer, as described below.
NL, CR  : returns current buffer to the program.

DOS and ANSI terminal arrow key sequences are recognized, and act like:

  up    : same as ^P
  down  : same as ^N
  left  : same as ^B
  right : same as ^F

************************** Programmer Interface ***************************

The programmer accesses input-edit through five functions, and optionally
through three additional function pointer hooks.  The five functions are:

char *Getline(const char *prompt)

        Prints the prompt and allows the user to edit the current line. A
        pointer to the line is returned when the user finishes by
        typing a newline or a return.  Unlike GNU readline, the returned
        pointer points to a static buffer, so it should not be free'd, and
        the buffer contains the newline character.  The user enters an
        end-of-file by typing ^D on an empty line, in which case the
        first character of the returned buffer is '\0'.  Getline never
        returns a NULL pointer.  The getline function sets terminal modes
        needed to make it work, and resets them before returning to the
        caller.  The getline function also looks for characters that would
        generate a signal, and resets the terminal modes before raising the
        signal condition.  If the signal handler returns to getline,
        the screen is automatically redrawn and editing can continue.
        Getline now requires both the input and output stream be connected
        to the terminal (not redirected) so the main program should check
        to make sure this is true.  If input or output have been redirected
        the main program should use buffered IO (stdio) rather than
        the slow 1 character read()s that getline uses (note: this limitation
        has been removed).

char *Getlinem(int mode, const char *prompt)

        mode: -1 = init, 0 = line mode, 1 = one char at a time mode, 2 = cleanup

        More specialized version of the previous function. Depending on
        the mode, it behaves differently. Its main use is to allow
        character by character input from the input stream (useful when
        in an X eventloop). It will return NULL as long as no newline
        has been received. Its use is typically as follows:
        1) In the program initialization part one calls: Getlinem(-1,"prompt>")
        2) In the X inputhandler: if ((line = Getlinem(1,NULL))) {
        3) In the termination routine: Getlinem(2,NULL)
        With mode=0 the function behaves exactly like the previous function.

void Gl_config(const char *which, int value)

        Set some config options. Which can be:
          "noecho":  do not echo characters (used for passwd input)
          "erase":   do erase line after return (used for text scrollers)

void Gl_setwidth(int width)

        Set the width of the terminal to the specified width. The default
        width is 80 characters, so this function need only be called if the
        width of the terminal is not 80.  Since horizontal scrolling is
        controlled by this parameter it is important to get it right.

void Gl_histinit(char *file)

        This function reads a history list from file. So lines from a
        previous session can be used again.

void Gl_histadd(char *buf)

        The Gl_histadd function checks to see if the buf is not empty or
        whitespace, and also checks to make sure it is different than
        the last saved buffer to avoid repeats on the history list.
        If the buf is a new non-blank string a copy is made and saved on
        the history list, so the caller can re-use the specified buf.

The main loop in testgl.c, included in this directory, shows how the
input-edit package can be used:

extern char *Getline();
extern void  Gl_histadd();
main()
{
    char *p;
    Gl_histinit(".hist");
    do {
        p = Getline("PROMPT>>>> ");
        Gl_histadd(p);
        fputs(p, stdout);
    } while (*p != 0);
}

In order to allow the main program to have additional access to the buffer,
to implement things such as completion or auto-indent modes, three
function pointers can be bound to user functions to modify the buffer as
described below.  By default Gl_in_hook and Gl_out_hook are set to NULL,
and Gl_tab_hook is bound to a function that inserts spaces until the next
logical tab stop is reached.  The user can reassign any of these pointers
to other functions.  Each of the functions bound to these hooks receives
the current buffer as the first argument, and must return the location of
the leftmost change made in the buffer.  If the buffer isn't modified the
functions should return -1.  When the hook function returns the screen is
updated to reflect any changes made by the user function.

int (*Gl_in_hook)(char *buf)

        If Gl_in_hook is non-NULL the function is called each time a new
        buffer is loaded. It is called when getline is entered, with an
        empty buffer, it is called each time a new buffer is loaded from
        the history with ^P or ^N, and it is called when an incremental
        search string is accepted (when the search is terminated). The
        buffer can be modified and will be redrawn upon return to Getline().

int (*Gl_out_hook)(char *buf)

        If Gl_out_hook is non-NULL it is called when a line has been
        completed by the user entering a newline or return. The buffer
        handed to the hook does not yet have the newline appended. If the
        buffer is modified the screen is redrawn before getline returns the
        buffer to the caller.

int (*Gl_tab_hook)(char *buf, int prompt_width, int *cursor_loc)

        If Gl_tab_hook is non-NULL, it is called whenever a tab is typed.
        In addition to receiving the buffer, the current prompt width is
        given (needed to do tabbing right) and a pointer to the cursor
        offset is given, where a 0 offset means the first character in the
        line.  Not only does the cursor_loc tell the programmer where the
        TAB was received, but it can be reset so that the cursor will end
        up at the specified location after the screen is redrawn.

int (*Gl_beep_hook)()
        Called if \007 (beep) is about to be printed. Return !=0 if handled.
*/

extern "C" {

/* forward reference needed for Gl_tab_hook */
static int gl_tab(char *buf, int offset, int *loc);

/********************* exported interface ********************************/


char   *Getline(const char *prompt); /* read a line of input */
char   *Getlinem(int mode, const char *prompt); /* allows reading char by char */
void    Gl_config(const char *which, int value); /* set some options */
void    Gl_setwidth(int w);          /* specify width of screen */
void    Gl_windowchanged();          /* call after SIGWINCH signal */
void    Gl_histinit(char *file); /* read entries from old histfile */
void    Gl_histadd(char *buf);       /* adds entries to hist */

int             (*Gl_in_hook)(char *buf) = 0;
int             (*Gl_out_hook)(char *buf) = 0;
int             (*Gl_tab_hook)(char *buf, int prompt_width, int *loc) = gl_tab;
int             (*Gl_beep_hook)() = 0;
int             (*Gl_in_key)(int ch) = 0;
}

/******************** imported interface *********************************/

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

/** newer imported interfaces **/
#include "editline.h"

char * hist_file; // file name for the command history (read and write)

struct WriteHistoryTrigger {
	~WriteHistoryTrigger() {
		write_history(hist_file);
	}
};



/******************** internal interface *********************************/

#define BUF_SIZE 1024

extern "C" {

static int      gl_init_done = -1;      /* terminal mode flag  */
static int      gl_notty = 0;           /* 1 when not a tty */
static int      gl_eof = 0;             /* 1 when not a tty and read() == -1 */
static int      gl_termw = 80;          /* actual terminal width */
static int      gl_scroll = 27;         /* width of EOL scrolling region */
static int      gl_width = 0;           /* net size available for input */
static int      gl_extent = 0;          /* how far to redraw, 0 means all */
static int      gl_overwrite = 0;       /* overwrite mode */
static int      gl_no_echo = 0;         /* do not echo input characters */
static int      gl_passwd = 0;          /* do not echo input characters */
static int      gl_erase_line = 0;      /* erase line before returning */
static int      gl_pos, gl_cnt = 0;     /* position and size of input */
static char     gl_buf[BUF_SIZE];       /* input buffer */
static char     gl_killbuf[BUF_SIZE]=""; /* killed text */
static const char *gl_prompt;           /* to save the prompt string */
static char     gl_intrc = 0;           /* keyboard SIGINT char */
static char     gl_quitc = 0;           /* keyboard SIGQUIT char */
static char     gl_suspc = 0;           /* keyboard SIGTSTP char */
static char     gl_dsuspc = 0;          /* delayed SIGTSTP char */
static int      gl_search_mode = 0;     /* search mode flag */
static int      gl_savehist = 0;        /* # of lines to save in hist file */
static char     gl_histfile[256];       /* name of history file */

static void     gl_init();              /* prepare to edit a line */
static void     gl_cleanup();           /* to undo gl_init */
static void     gl_char_init();         /* get ready for no echo input */
static void     gl_char_cleanup();      /* undo gl_char_init */

static void     gl_addchar(int c);      /* install specified char */
static void     gl_del(int loc);        /* del, either left (-1) or cur (0) */
static void     gl_error(char *buf);    /* write error msg and die */
static void     gl_fixup(const char *p, int c, int cur); /* fixup state variables and screen */
static int      gl_getc();              /* read one char from terminal */
static void     gl_kill();              /* delete to EOL */
static void     gl_newline();           /* handle \n or \r */
static void     gl_putc(int c);         /* write one char to terminal */
static void     gl_puts(const char *buf); /* write a line to terminal */
static void     gl_redraw();            /* issue \n and redraw all */
static void     gl_transpose();         /* transpose two chars */
static void     gl_yank();              /* yank killed text */

static int      is_whitespace(char c);  /* "whitespace" very loosely interpreted */
static void     gl_back_1_word();       /* move cursor back one word */
static void     gl_kill_1_word();       /* kill to end of word */
static void     gl_kill_back_1_word();  /* kill to begin of word */
static void     gl_kill_region(int i, int j); /* kills from i to j */
static void     gl_fwd_1_word();        /* move cursor forward one word */
static void     gl_set_mark();          /* sets mark to be at point */
static void     gl_exch();              /* exchanges point and mark */
static void     gl_wipe();              /* kills from mark to point */
static int      gl_mark = -1;           /* position of mark. gl_mark<0 if not set */

static void     hist_init();            /* initializes hist pointers */
static char    *hist_next();            /* return ptr to next item */
static char    *hist_prev();            /* return ptr to prev item */
static char    *hist_save(char *p);     /* makes copy of a string, without NL */

static void     search_addchar(int c);  /* increment search string */
static void     search_term();          /* reset with current contents */
static void     search_back(int s);     /* look back for current string */
static void     search_forw(int s);     /* look forw for current string */
} // extern "C"

/************************ nonportable part *********************************/

#ifdef MSDOS
#include <bios.h>
#endif

#ifdef WIN32
#  define MSDOS
#  include <io.h>
#  include <windows.h>
#endif /* WIN32 */

#if defined(_AIX) || defined(__Lynx__) || defined(__APPLE__) || defined(__OpenBSD__)
#define unix
#endif

#if defined(__hpux) || defined(__osf__)       /* W.Karig@gsi.de */
#ifndef unix
#define unix
#endif
#endif

#ifdef unix
#include <unistd.h>
#if !defined(__osf__) && !defined(_AIX)       /* W.Karig@gsi.de */
#include <sys/ioctl.h>
#endif

#if defined(__linux__) && defined(__powerpc__)
#   define R__PPCLINUX      /* = linux on PPC(64) */
#endif
#if defined(__linux__) && defined(__alpha__)
#   define R__ALPHALINUX    /* = linux on Alpha */
#endif
#if defined(__linux__) && defined(__mips) /* cholm@nbi.dk */
#   define R__MIPSLINUX    /* = linux on mips */
#endif

#if defined(TIOCGETP) && !defined(__sgi) && !defined(R__PPCLINUX) && \
    !defined(R__ALPHALINUX)  && !defined(R__MIPSLINUX) /* use BSD interface if possible */
#include <sgtty.h>
struct sgttyb   new_tty, old_tty;
struct tchars   tch;
struct ltchars  ltch;
#else
#ifdef SIGTSTP          /* need POSIX interface to handle SUSP */
#include <termios.h>
#if defined(__sun) || defined(__sgi) || defined(R__PPCLINUX) || \
    defined(R__ALPHALINUX) || defined(R__MIPSLINUX)
#undef TIOCGETP         /* Solaris and SGI define TIOCGETP in <termios.h> */
#undef TIOCSETP
#endif
struct termios  new_termios, old_termios;
#else                   /* use SYSV interface */
#include <termio.h>
struct termio   new_termio, old_termio;
#endif
#endif
#endif  /* unix */

#ifdef VMS
#include <descrip.h>
#include <ttdef.h>
#include <iodef.h>
#include <starlet.h>
#include <unistd.h>
#include unixio

static int   setbuff[2];             /* buffer to set terminal attributes */
static short chan = -1;              /* channel to terminal */
struct dsc$descriptor_s descrip;     /* VMS descriptor */
#endif

extern "C" {

void
Gl_config(const char *which, int value)
{
   if (strcmp(which, "noecho") == 0)
      gl_no_echo = value;
   else if (strcmp(which, "erase") == 0)
      gl_erase_line = value;
   else
      printf("gl_config: %s ?\n", which);
}

static void
gl_char_init()                  /* turn off input echo */
{
    if (gl_notty) return;
#ifdef unix
#ifdef TIOCGETP                 /* BSD */
    ioctl(0, TIOCGETC, &tch);
    ioctl(0, TIOCGLTC, &ltch);
    gl_intrc = tch.t_intrc;
    gl_quitc = tch.t_quitc;
    gl_suspc = ltch.t_suspc;
    gl_dsuspc = ltch.t_dsuspc;
    ioctl(0, TIOCGETP, &old_tty);
    new_tty = old_tty;
    new_tty.sg_flags |= RAW;
    new_tty.sg_flags &= ~ECHO;
    ioctl(0, TIOCSETP, &new_tty);
#else
#ifdef SIGTSTP                  /* POSIX */
    tcgetattr(0, &old_termios);
    gl_intrc = old_termios.c_cc[VINTR];
    gl_quitc = old_termios.c_cc[VQUIT];
#ifdef VSUSP
    gl_suspc = old_termios.c_cc[VSUSP];
#endif
#ifdef VDSUSP
    gl_dsuspc = old_termios.c_cc[VDSUSP];
#endif
    new_termios = old_termios;
    new_termios.c_iflag &= ~(BRKINT|ISTRIP|IXON|IXOFF);
    new_termios.c_iflag |= (IGNBRK|IGNPAR);
    new_termios.c_lflag &= ~(ICANON|ISIG|IEXTEN|ECHO);
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_termios);
#else                           /* SYSV */
    ioctl(0, TCGETA, &old_termio);
    gl_intrc = old_termio.c_cc[VINTR];
    gl_quitc = old_termio.c_cc[VQUIT];
    new_termio = old_termio;
    new_termio.c_iflag &= ~(BRKINT|ISTRIP|IXON|IXOFF);
    new_termio.c_iflag |= (IGNBRK|IGNPAR);
    new_termio.c_lflag &= ~(ICANON|ISIG|ECHO);
    new_termio.c_cc[VMIN] = 1;
    new_termio.c_cc[VTIME] = 0;
    ioctl(0, TCSETA, &new_termio);
#endif
#endif
#endif /* unix */

#ifdef MSDOS
    gl_intrc = 'C' - '@';
    gl_quitc = 'Q' - '@';
/*    gl_suspc = ltch.t_suspc; */
#endif /* MSDOS */

#ifdef vms
    descrip.dsc$w_length  = strlen("tt:");
    descrip.dsc$b_dtype   = DSC$K_DTYPE_T;
    descrip.dsc$b_class   = DSC$K_CLASS_S;
    descrip.dsc$a_pointer = "tt:";
    (void)sys$assign(&descrip,&chan,0,0);
    (void)sys$qiow(0,chan,IO$_SENSEMODE,0,0,0,setbuff,8,0,0,0,0);
    setbuff[1] |= TT$M_NOECHO;
    (void)sys$qiow(0,chan,IO$_SETMODE,0,0,0,setbuff,8,0,0,0,0);
#endif /* vms */
}

static void
gl_char_cleanup()               /* undo effects of gl_char_init */
{
    if (gl_notty) return;
#ifdef unix
#ifdef TIOCSETP         /* BSD */
    ioctl(0, TIOCSETP, &old_tty);
#else
#ifdef SIGTSTP          /* POSIX */
    tcsetattr(0, TCSANOW, &old_termios);
#else                   /* SYSV */
    ioctl(0, TCSETA, &old_termio);
#endif
#endif
#endif /* unix */

#ifdef vms
    setbuff[1] &= ~TT$M_NOECHO;
    (void)sys$qiow(0,chan,IO$_SETMODE,0,0,0,setbuff,8,0,0,0,0);
    sys$dassgn(chan);
    chan = -1;
#endif
}

#if defined(MSDOS) && !defined(WIN32)
#  include <conio.h>
   int pause_()
   {
      int first_char;
        first_char = _getch();
        if (first_char == 0 || first_char == 0xE0) first_char = -_getch();
        return first_char;
   }
#endif

#if defined(MSDOS) && defined(WIN32)
int pause_()
{
 static HANDLE hConsoleInput = NULL;
 static iCharCount = 0;
 static int chLastChar = 0;

 DWORD cRead;

 INPUT_RECORD pirBuffer;
 KEY_EVENT_RECORD *KeyEvent= (KEY_EVENT_RECORD *)&(pirBuffer.Event);

 if (!hConsoleInput) hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

 if (iCharCount) iCharCount--;      /* Whether several symbols had been read */
 else {
   chLastChar = 0;
   while (chLastChar == 0) {
     if (!ReadConsoleInput(hConsoleInput,       /* handle of a console input buffer    */
                           &pirBuffer,          /* address of the buffer for read data */
                           1,                   /* number of records to read           */
                           &cRead               /* address of number of records read   */
        )) return 0;

     if (pirBuffer.EventType == KEY_EVENT  && KeyEvent->bKeyDown == TRUE){
         iCharCount = KeyEvent->wRepeatCount - 1;
         chLastChar = ((int) (KeyEvent->uChar).AsciiChar & 0xffff);
         if (chLastChar)
              OemToCharBuff((char const *)&chLastChar,(char *)&chLastChar,1);
         else
              chLastChar = - (KeyEvent->wVirtualScanCode);
/*            chLastChar = - (KeyEvent->wVirtualKeyCode); */
     }
   }
 }
 return chLastChar;

}
#endif


static int
gl_getc()
{ }

static void
gl_putc(int c)
{ }

/******************** fairly portable part *********************************/

static void
gl_puts(const char *buf)
{
    int len = strlen(buf);

    if (gl_notty) return;
#ifdef WIN32
    {
     char *OemBuf = (char *)malloc(2*len);
     CharToOemBuff(buf,OemBuf,len);
     write(1, OemBuf, len);
     free(OemBuf);
    }
#else
    write(1, buf, len);
#endif
}

static void
gl_error(char *buf)
{
    int len = strlen(buf);

    gl_cleanup();
#ifdef WIN32
    {
      char *OemBuf = (char *)malloc(2*len);
      CharToOemBuff(buf,OemBuf,len);
      write(2, OemBuf, len);
      free(OemBuf);
    }
#else
    write(2, buf, len);
#endif
    exit(1);
}

static void
gl_init()
/* set up variables and terminal */
{
    if (gl_init_done < 0) {             /* -1 only on startup */
        hist_init();
    }
    if (isatty(0) == 0 || isatty(1) == 0)
        gl_notty = 1;
    gl_char_init();
    gl_init_done = 1;
}

static void
gl_cleanup()
/* undo effects of gl_init, as necessary */
{
    if (gl_init_done > 0)
        gl_char_cleanup();
    gl_init_done = 0;
}

void
Gl_setwidth(int w)
{
    if (w > 20) {
        gl_termw = w;
        gl_scroll = w / 3;
    } else {
        gl_error("\n*** Error: minimum screen width is 21\n");
    }
}

void
Gl_windowchanged()
{
#ifdef TIOCGWINSZ
   if (isatty(0)) {
      static char lenv[32], cenv[32];
      struct winsize wins;
      ioctl(0, TIOCGWINSZ, &wins);

      if (wins.ws_col == 0) wins.ws_col = 80;
      if (wins.ws_row == 0) wins.ws_row = 24;

      Gl_setwidth(wins.ws_col);

      sprintf(lenv, "LINES=%d", wins.ws_row);
      putenv(lenv);
      sprintf(cenv, "COLUMNS=%d", wins.ws_col);
      putenv(cenv);
   }
#endif
}

/* The new and hopefully improved Getlinem method! 
 * Uses readline() from libeditline. 
 * History and editing are also handled by libeditline.
 * Modes: -1 = init, 0 = line mode, 1 = one char at a time mode, 2 = cleanup
 */
char *
Getlinem(int mode, const char *prompt)
{
   static char sprompt[80] = {0};
   char * input_buffer;
   rl_tab_hook = Gl_tab_hook;

   	static int getline_initialized = 0;
	if (getline_initialized == 0)
	{
		//rl_initialize();		// - is rl_initialize already being called by history_stifle()?
		read_history(hist_file);
		getline_initialized = 1;
	}

	// mode 2 = cleanup
	if (mode == 2) {
		rl_reset_terminal(0);
        }

	// mode -1 = init
	if (mode == -1) {
		if (prompt)
			strcpy(sprompt, prompt);
		input_buffer = readline(sprompt, true /*newline*/);

		return input_buffer;
        }

	// mode 1 = one char at a time
	if (mode == 1) {
		if (prompt)
			strcpy(sprompt, prompt);

		// note: input_buffer will be null unless complete line entered
		input_buffer = readline(sprompt, false /*no newline*/);

		// if complete line is entered, add to history and return buffer, otherwise return null
		char * ch = input_buffer;
		
		if (input_buffer)
		{
			while ( *ch != '\a' ) 
			{
				if (*ch == '\n')		// line complete!
				{
					add_history(input_buffer);
					return input_buffer;
				}
				++ch;
			}
		}
	}
	return NULL;
}

int
Gl_eof()
{ }

char *
Getline(const char *prompt)
{
   return Getlinem(0, prompt);
}

static void
gl_addchar(int c)
{ }

static void
gl_yank()
{ }

static void
gl_transpose()
{ }

static void
gl_newline()
{ }

static void
gl_del(int loc)
{ }

static void
gl_kill()
{ }

static void
gl_redraw()
{ }

static void setCursorPosition(int x)
{ }

static void
gl_fixup(const char *prompt, int change, int cursor)
{ }

static int
gl_tab(char *buf, int offset, int *loc)
{
/* default tab handler, acts like tabstops every 8 cols */
    int i, count, len;

    len = strlen(buf);
    count = 8 - (offset + *loc) % 8;
    for (i=len; i >= *loc; i--)
        buf[i+count] = buf[i];
    for (i=0; i < count; i++)
        buf[*loc+i] = ' ';
    i = *loc;
    *loc = i + count;
    return i;
}

/******************* History stuff **************************************/

static void
hist_init() { }

void
Gl_histsize(int size, int save)
{
	stifle_history(save);
}

void
Gl_histinit(char *file)
{ 
	static WriteHistoryTrigger history_write_trigger;
	hist_file = file;
}

void
Gl_histadd(char *buf)
{ }

static char *
hist_prev()
{ }

static char *
hist_next()
{ }

static char *
hist_save(char *p)
{ }

/******************* Search stuff **************************************/

static void
search_update(int c)
{ }

static void
search_addchar(int c)
{ }

static void
search_term()
{ }

static void
search_back(int new_search)
{ }

static void
search_forw(int new_search)
{ }



/*****************************************************************************/
/* Extra routine provided by Christian Lacunza <lacunza@cdfsg5.lbl.gov>      */
/*****************************************************************************/

static void gl_back_1_word( void )
{ }

static void gl_kill_back_1_word( void )
{ }

static void gl_kill_1_word( void )
{ }

static void gl_kill_region( int i, int j )
{ }

static void gl_fwd_1_word( void )
{ }

static int is_whitespace( char c )
{ }

static void gl_set_mark( void )
{ }

static void gl_wipe( void )
{ }

static void gl_exch( void )
{ }

} // extern "C"
