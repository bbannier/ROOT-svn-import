#ifndef __LIBEDIT_COMPATH_H
#define __LIBEDIT_COMPATH_H


// On Darwin systems, __RCSID() and __COPYRIGHT() conflict
// with macros in system headers. We don't need them, so
// we're going to nuke them. (Thanks to Alexey Zakhlestin for
// reporting this.)
#ifdef __RCSID
# undef __RCSID
#endif
#ifdef __COPYRIGHT
# undef __COPYRIGHT
#endif

#define  __RCSID(x)
#define  __COPYRIGHT(x)

#include "compat_conf.h"

#ifndef HAVE_VIS_H
/* string visual representation - may want to reimplement */
# define strvis(d, s, m) strcpy(d, s)
# define strunvis(d, s) strcpy(d, s)
#endif

#ifndef HAVE_FGETLN
# include "fgetln.h"
#endif

#ifndef HAVE_ISSETUGID
# define issetugid() (getuid() != geteuid() || getegid() != getgid())
#endif

#ifndef HAVE_STRLCPY
# include "strlcpy.h"
#endif

#if HAVE_SYS_CDEFS_H
# include <sys/cdefs.h>
#endif

#ifndef __P
# if defined(__STDC__) || defined(__cplusplus)
#  define __P(x) x
# else
#  define __P(x) ()
# endif
#endif

#endif
