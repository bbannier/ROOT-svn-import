

// todo: write config tests for these:
#define HAVE_GETLINE 1
#define HAVE_FLOCKFILE 1

////////////////////////////////////////////////////////////////////////
// most of this #ifdef code is to work around my own preference of
// #if FOO, instead of #ifdef FOO, in client code. Since toc
// does a define to 0 on false (which i think is sane), we need
// to UNSET those vars which toc sets to zero, to accomodate
// this source tree.
#ifndef HAVE_GETLINE
#  define HAVE_GETLINE @HAVE_GETLINE@
#endif
#if (0 == HAVE_GETLINE)
#  undef HAVE_GETLINE
#endif

#ifndef HAVE_FLOCKFILE
#  define HAVE_FLOCKFILE @HAVE_FLOCKFILE@
#endif
#if (0 == HAVE_FLOCKFILE)
#  undef HAVE_FLOCKFILE
#endif

#ifndef HAVE_SYS_TYPES_H
#  define HAVE_SYS_TYPES_H 1
#endif
#if (0 == HAVE_SYS_TYPES_H)
#  undef HAVE_SYS_TYPES_H
#endif

#ifndef HAVE_SYS_CDEFS_H
#  define HAVE_SYS_CDEFS_H 1
#endif
#if (0 == HAVE_SYS_CDEFS_H)
#  undef HAVE_SYS_CDEFS_H
#endif

#ifndef HAVE_LIMITS_H
#  define HAVE_LIMITS_H 1
#endif
#if (0 == HAVE_LIMITS_H)
#  undef HAVE_LIMITS_H
#endif

#ifndef HAVE_MALLOC_H
#  define HAVE_MALLOC_H 1
#endif
#if (0 == HAVE_MALLOC_H)
#  undef HAVE_MALLOC_H
#endif

#ifndef HAVE_SYS_IOCTL_H
#  define HAVE_SYS_IOCTL_H 1
#endif
#if (0 == HAVE_SYS_IOCTL_H)
#  undef HAVE_SYS_IOCTL_H
#endif

#ifndef HAVE_UNISTD_H
#  define HAVE_UNISTD_H 1
#endif
#if (0 == HAVE_UNISTD_H)
#  undef HAVE_UNISTD_H
#endif

#ifndef HAVE_VIS_H
#  define HAVE_VIS_H 0
#endif
#if (0 == HAVE_VIS_H)
#  undef HAVE_VIS_H
#endif


#ifndef HAVE_FGETLN
#  define HAVE_FGETLN 0
#endif
#if (0 == HAVE_FGETLN)
#  undef HAVE_FGETLN
#endif

#ifndef HAVE_ISSETUGID
#  define HAVE_ISSETUGID 0
#endif
#if (0 == HAVE_ISSETUGID)
#  undef HAVE_ISSETUGID
#endif

#ifndef HAVE_STRLCPY
#  define HAVE_STRLCPY 0
#endif
#if (0 == HAVE_STRLCPY)
#  undef HAVE_STRLCPY
#endif
