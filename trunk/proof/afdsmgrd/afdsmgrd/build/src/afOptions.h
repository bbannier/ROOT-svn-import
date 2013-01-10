/**
 * afOptions.h -- by Dario Berzano <dario.berzano@cern.ch>
 *
 * This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
 *
 * Compile and configure options to build afdsmgrd. Version number is defined
 * here too.
 */

#ifndef AFOPTIONS_H
#define AFOPTIONS_H

/** Build options (unused).
 */
//#ifndef __CINT__
/* #undef WITH_APMON */
//#endif

/** Version number is in the format MAJOR.MINOR.PATCHES (or "trunk").
 *  See here for versioning information: http://apr.apache.org/versioning.html
 */
//#define AF_VERSION_DEVEL "trunk"
#define AF_VERSION "1.0.6"

/** Version banner which includes the version number with build date and time.
 */
#ifdef AF_VERSION_DEVEL
#define AF_VERSION_BANNER "*** " AF_PROG_NAME " " AF_VERSION_DEVEL \
  " (towards v" AF_VERSION ") compiled " __DATE__ " " __TIME__ \
  " -- by Dario Berzano <dario.berzano@cern.ch> ***"
#else
#define AF_VERSION_BANNER "*** " AF_PROG_NAME " v" AF_VERSION \
  " compiled " __DATE__ " " __TIME__ \
  " -- by Dario Berzano <dario.berzano@cern.ch> ***"
#endif

/** Number of extra regular expressions supported in configuration file, in
 *  addition to the first one (dsmgrd.urlregex). Extra regular expressions are
 *  named from 2 on (dsmgrd.urlregex.2, dsmgrd.urlregex.3...)
 */
#define AF_NUM_EXTRA_REGEX 4

#endif
