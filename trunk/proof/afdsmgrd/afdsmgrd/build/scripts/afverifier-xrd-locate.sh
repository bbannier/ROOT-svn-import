#!/bin/bash

#
# afverifier-xrd-locate.sh -- by Dario Berzano <dario.berzano@cern.ch>
#
# This file is part of afdsmgrd -- see http://code.google.com/p/afdsmgrd
#
# Wrapper around xrd locateall command, see macros/Verify.C for info on output.
#

# Get the configuration variables from the first configuration file found
declare -a CONF_FILES
CONF_FILES=(
  "/usr/local/etc/sysconfig/afdsmgrd" \
  "/usr/local/etc/default/afdsmgrd" \
  "/etc/sysconfig/afdsmgrd" \
  "/etc/default/afdsmgrd" \
  "$HOME/.afdsmgrd.cf"
)

for CF in "${CONF_FILES[@]}"; do
  if [ -r "$CF" ]; then
    source "$CF"
    break
  fi
done

# Classic xrootd (in EXTCMD_LIBS) and ROOT environment variables
export LD_LIBRARY_PATH="$AFDSMGRD_EXTCMD_LIBS:$ROOTSYS/lib:$LD_LIBRARY_PATH"
export PATH="$ROOTSYS/bin:$AFDSMGRD_EXTCMD_PATH:$PATH"

#
# Parse command-line options
#

Args=$(getopt -o '' \
  --long 'no-zipcheck,xrd-prefix:,no-remote,force-zip' \
  -n`basename "$0"` -- "$@")
[ $? != 0 ] && exit 1

eval set -- "$Args"

# Default option values
ZipCheck=1
ForceZip=0
DoSshIfNeeded=1
XrdPrefix=/

while [ "$1" != '--' ] ; do

  case "$1" in

    --no-zipcheck)
      ZipCheck=0
      shift 1
    ;;

    --xrd-prefix)
      XrdPrefix="$2/"
      shift 2
    ;;

    --force-zip)
      ForceZip=1
      shift 1
    ;;

    --no-remote)
      DoSshIfNeeded=0
      shift 1
    ;;

    *)
      # Should never happen
      echo "Ignoring unknown option: $1"
      shift 1
    ;;

  esac

done

shift # --

# Remaining option is the URL
URL="$1"

# Split into host:port, filename and anchor; note that queries (?...) are not
# supported!
if [[ "$URL" =~ ^root://([^/]+)/([^\#\?]+)(.*)?$ ]]; then
  HOSTPORT=${BASH_REMATCH[1]}
  FILENAME=${BASH_REMATCH[2]}
  REMAIN=${BASH_REMATCH[3]}
  CLEANURL="root://$HOSTPORT/$FILENAME"
else
  echo "FAIL $URL Reason: cant_parse_url"
  exit 0
fi

# Launch xrd
TMPOUT=$(mktemp /tmp/.afverifier-xrd-locate-XXXXX)
xrd $HOSTPORT locateall $FILENAME > $TMPOUT 2>&1
#echo $?

if [ $? != 0 ]; then
  rm $TMPOUT
  echo "FAIL $CLEANURL Reason: xrd_error"
  exit 0
fi

# Get Location: line
LOCATION=$(cat $TMPOUT | grep 'Location: ' | head -n1)

# Parse location server
if [[ $LOCATION =~ Location:\\s\'(.*)\' ]]; then

  ENDP_URL="root://${BASH_REMATCH[1]}/${FILENAME}"

  # Get file extension to lowercase
  EXT="${FILENAME##*.}"
  EXT=$(echo "$EXT" | tr '[:upper:]' '[:lower:]')

  # With option --no-zipcheck as second argument, zip check might be skipped
  if [ $ForceZip == 1 ] || [[ "$EXT" == 'zip' && $ZipCheck == 1 ]] ; then

    #
    # Additional zip integrity check if it is a zipfile
    #

    echo 'Performing zip check...'

    # Error values
    ERR_ZIP=42
    ERR_XRD=43

    # Script to execute on the remote host, kept inside a variable
    CHECKSCRIPT=$(cat <<EOF
#!/bin/bash
echo "Check script running on \$(hostname -f)"
export PATH="$PATH"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
TMPXRD=\$(mktemp /var/tmp/afverifier-xrd-zip-test.XXXXX)

LocalFileName='`readlink -m "$XrdPrefix/$FILENAME"`'
RETVAL=0
if [ -r "\$LocalFileName" ]; then
  echo 'File found locally! No need to run xrdcp...'
  ln -nfs "\$LocalFileName" \$TMPXRD
else
  echo 'File not accessible locally, copying it with xrdcp...'
  xrdcp -f '$ENDP_URL' \$TMPXRD || RETVAL=$ERR_XRD
fi

if [ \$RETVAL == 0 ] && [ -s \$TMPXRD ]; then
  zip -T "\$TMPXRD" || RETVAL=$ERR_ZIP
fi

rm -f "\$TMPXRD"

exit \$RETVAL
EOF
)

    #echo "--- Generated check script follows ---"
    #echo "$CHECKSCRIPT"
    #echo "--- End of generated check script ---"

    # Get remote host
    REMOTE_HOST=$(echo ${BASH_REMATCH[1]} | cut -d: -f1)

    # If it is an IP, convert it to hostname
    REMOTE_HOST=$(getent hosts $REMOTE_HOST | awk '{print $2}')

    # Do we have to perform SSH? Disable SSH with --no-remote
    if [[ $DoSshIfNeeded == 0 ||
          $REMOTE_HOST == $(hostname -f) ||
          $REMOTE_HOST == $(hostname -s) ||
          $REMOTE_HOST == 'localhost' ||
          $REMOTE_HOST == '127.0.0.1' ]]
    then

      # No SSH needed: same machine
      echo "--> Running check on this machine"
      echo "$CHECKSCRIPT" | bash
      RETVAL=$?

    else

      # SSH needed
echo ${BASH_REMATCH[1]}
      echo "--> Running check remotely via SSH on $REMOTE_HOST"
      echo "$CHECKSCRIPT" | ssh -oStrictHostKeyChecking=no $REMOTE_HOST
      RETVAL=$?

    fi

    # Result of check
    if [ $RETVAL == $ERR_XRD ]; then
      echo "FAIL $CLEANURL Reason: xrdcp_error"
      exit 0
    elif [ $RETVAL == $ERR_ZIP ]; then
      echo "FAIL $CLEANURL Reason: zip_error"
      exit 0
    elif [ $RETVAL != 0 ]; then
      echo "FAIL $CLEANURL Reason: zipcheck_unknown_err_$RETVAL"
      exit 0
    fi

  else
    echo 'Skipping zip check...'
  fi

  # Append the rest of endpoint URL (queries, anchors...)
  ENDP_URL="${ENDP_URL}${REMAIN}"
  echo "OK $CLEANURL EndpointUrl: $ENDP_URL"
else

  # File is not staged (empirically... bad xrd, bad...)
  cat $TMPOUT | grep 'matching files' > /dev/null 2>&1
  if [ $? == 0 ]; then
    echo "FAIL $CLEANURL Reason: not_staged"
  else
    # No reason given, generic error (server down, etc.)
    echo "FAIL $CLEANURL"
  fi

fi

rm -f $TMPOUT
