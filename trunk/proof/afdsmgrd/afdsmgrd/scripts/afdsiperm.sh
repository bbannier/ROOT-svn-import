#!/bin/bash

#
# afdsiperm.sh -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# Bash script that relies on inotify to "fix" permissions of the dataset
# repository recursively. It has been created to allow afdsmgrd to run as an
# unprivileged user, and to work around some glitches in permissions of datasets
# created by PROOF.
#
# This script must be run as root and takes as arguments the path to the dataset
# repository to monitor and the main group of the Unix user running afdsmgrd.
#
# The script logs on syslog.
#

export Prog=`basename "$0"`
export InotifyWaitPrefix='/tmp/inot' # no final dash
export InotifyWait="${InotifyWaitPrefix}-$$" # basename shorter than 15 chars!!!
export DatasetGroup
export DatasetPath

# Echoes message on stderr and /var/log/messages (or analogous)
function Msg() {
  echo "$Prog: $1" >&2
  logger -t "$Prog" "$1"
}

#
# Initial checks
#

if [ `whoami` != 'root' ] ; then
  Msg 'must be run as root, aborting'
  exit 1
fi

if ! which inotifywatch > /dev/null 2>&1 ; then
  Msg "can't find inotifywatch in \$PATH, aborting"
  exit 3
fi

# Fixes the given entry only if needed, to avoid recursive calls made by
# inotify
function FixEntry() {
  local File Perm GroupPerm GroupName
  File="$1"
  Perm=`stat "$File" -c "%A %G" 2> /dev/null`
  GroupPerm=${Perm:4:3}
  GroupName=${Perm:11}

  if [ -d "$File" ] ; then
    [ "$GroupPerm" != 'rwx' ] && chmod g=rwx "$File" 2> /dev/null
  elif [ -f "$File" ] ; then
    [ "$GroupPerm" != 'rw-' ] && chmod g=rw "$File" 2> /dev/null
  fi

  if [ "$GroupName" != "$DatasetGroup" ] ; then
    chgrp "$DatasetGroup" "$File" 2> /dev/null
  fi

  #Msg "$File"
}

# Fixes a directory and its contents. This is performed because 'mkdir -p' calls
# make inotifywait ignore directories created after the topmost
function FixDirRecursive() {
  local File
  [ ! -d "$1" ] && return 1

  # Fix ROOT files and directories - "$1" is included by '-type d'
  find "$1" \( -type f -and -name "*.root" \) -or -type d | \
  while read File ; do
    FixEntry "$File"
  done

}

# The main fix loop: first performs an overall fix, then goes into inotify mode
function FixLoop() {

  # Initial fix of existing stuff
  FixDirRecursive "$DatasetPath"

  # Fix based on inotify events
  "$InotifyWait" --format %w%f -qrm \
    -e modify,attrib,moved_to,moved_from,move,create,delete_self \
    "$DatasetPath" 2> /dev/null | \
  while read File ; do

    File=`readlink -m "$File"`

    # Maybe, it is the main directory!
    if [ "$File" == "$DatasetPath" ] && [ ! -d "$DatasetPath" ] ; then
      Msg "dataset path $DatasetPath disappeared!"
      pkill -15 `basename "$InotifyWait"`
      return 1
    fi

    # Get extension
    Ext=${File##*.}
    [ "$Ext" == "$File" ] && Ext=''

    if [ -d "$File" ] ; then
      FixDirRecursive "$File"  # avoid 'mkdir -p' problem
    elif [ -f "$File" ] && [ "$Ext" == 'root' ] ; then
      FixEntry "$File"
    fi

  done

}

# The main function
function Main() {

  #local Tmp Pid
  local Tmp

  # Cleanup of old leftovers
  ls -1 "${InotifyWaitPrefix}-"* 2> /dev/null | \
  while read Tmp ; do
    rm -f "$Tmp"
    #Pid=${Tmp##*-}
    #if [ "$Pid" != '' ] && [ `stat -c %U "$Tmp"` == 'root' ] ; then
    #  kill -9 "$Pid" 2> /dev/null
    #  rm -f "$Tmp"
    #fi
  done
  kill -9 `pgrep "$Prog" | grep -v "$$"`

  # Are we only here to kill?
  [ "$1" == '--kill' ] && return 0

  # Check arguments
  if [ "$1" == '' ] || [ "$2" == '' ] ; then
    Msg "missing arguments: use $Prog <dspath> <dsgroup> (aborting)"
    return 4
  fi

  DatasetPath=`readlink -m "$1"`
  DatasetGroup="$2"

  # Nasty trick to get PID of inotifywait via pgrep/pkill
  ln -nfs `which inotifywait` "$InotifyWait"

  # Auto-cleanup of our stuff on any form of exit (except SIGKILL)
  trap 'rm -f "$InotifyWait" ; Msg "quit requested, exiting"' 0

  Msg "started with PID $$"
  Msg "fixing dataset directory $DatasetPath for group $DatasetGroup"

  # Infinite loop
  while true ; do
    FixLoop
    sleep 1
  done

}

#
# Entry point
#

Main "$@"
exit $?
