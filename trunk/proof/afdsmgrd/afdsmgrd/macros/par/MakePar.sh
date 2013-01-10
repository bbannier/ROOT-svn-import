#!/bin/bash

#
# MakePar.sh -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# Utility to create PARFiles out of directories. It launches a tar gzip command
# using an option to exclute versioning control files from being included in the
# archive.
#
# PARFiles are created in the same directory that contains MakePar.sh.
#

#
# Functions
#

# Returns a full normalized form of the given path
function NormalizePath() {
  local FN=$(basename "$1")
  local DN=$(dirname "$1")
  DN=$(cd "$DN" ; pwd)
  DN="$DN/$FN"

  while [ "${DN:0:2}" == "//" ]; do
    DN="${DN:1}"
  done

  echo $DN
}

#
# Entry point
#

if [ "$1" == "" ]; then
  echo "To create a PARFile, do: $0 <dirname> [<par_name>]"
  exit 1
fi

if [ "$2" != "" ]; then
  BASEPAR="$2"
else
  BASEPAR=$(basename "$1")
fi

DIR=$(NormalizePath "$1")

if [ "$PAR" == "/" ]; then
  echo "Cannot create PARFiles from the root directory!"
  exit 2
fi

PAR=$(dirname "$0")/"$BASEPAR".par
PAR=$(NormalizePath "$PAR")
TMP=$(mktemp -d)

echo -e "\033[1;35mInput dir\033[m  : $DIR"
echo -e "\033[1;35mOutput PAR\033[m : $PAR"
#echo -e "\033[1;35mWork dir\033[m   : $TMP"

if [ ! -d "$DIR" ]; then
  echo "Can't access input directory $DIR"
  exit 2
fi

rsync -a "$DIR" "$TMP"
cd "$TMP"
mv $(basename "$DIR") "$BASEPAR" 2> /dev/null

tar czf "$PAR" "$BASEPAR" --exclude '.svn' # --exclude-vcs not ubiquitous :(

#rm -rf "$TMP"
