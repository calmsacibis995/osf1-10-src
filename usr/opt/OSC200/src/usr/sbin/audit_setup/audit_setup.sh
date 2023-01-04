#! /bin/sh
#
# *****************************************************************
# *                                                               *
# *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
# *                                                               *
# *   All Rights Reserved.  Unpublished rights  reserved  under   *
# *   the copyright laws of the United States.                    *
# *                                                               *
# *   The software contained on this media  is  proprietary  to   *
# *   and  embodies  the  confidential  technology  of  Digital   *
# *   Equipment Corporation.  Possession, use,  duplication  or   *
# *   dissemination of the software and media is authorized only  *
# *   pursuant to a valid written license from Digital Equipment  *
# *   Corporation.                                                *
# *                                                               *
# *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
# *   by the U.S. Government is subject to restrictions  as  set  *
# *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
# *   or  in  FAR 52.227-19, as applicable.                       *
# *                                                               *
# *****************************************************************
#
# HISTORY
# 
# @(#)$RCSfile: audit_setup.sh,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/20 21:21:51 $
# 
					
PATH="/usr/bin:/sbin:/etc"

AUDITD=/usr/sbin/auditd
AUDITALIAS=/etc/sec/event_aliases
AUDITDCLNT=/etc/sec/auditd_clients
AUDITDLOC=/etc/sec/auditd_loc
AUDITEVNTS=/etc/sec/audit_events
AUDITMASK=/usr/sbin/auditmask
DFLTCONS=auditd_cons
DFLTDIR=/var/audit
DFLTLOG=auditlog
DOCONFIG=/usr/sbin/doconfig
ED=/sbin/ed
ECHO=/bin/echo
EXPR=/sbin/expr
GREP=/sbin/grep
MKDIR=/sbin/mkdir
RCCONF=/etc/rc.config
RCMGR=/usr/sbin/rcmgr

HOSTNAME="`hostname | tr a-z A-Z | sed 's/\..*//'`"
CONFIG="/sys/conf/$HOSTNAME"
CONFIGMARK=0                    # to indicate config file updated



# -------------------------
# trap handler
#
QUIT='
    $RCMGR set AUDITD_FLAG ""
    $GREP -qs DEC_AUDIT $CONFIG
    [ $? = 0 -a $CONFIGMARK = 1 ] && {
        $ED - $CONFIG << END > /dev/null
/^options		DEC_AUDIT
d
w
q
END
    }
    $ECHO
    $ECHO "  ***** AUDIT SETUP TERMINATED ***** "
    $ECHO
    exit 1
'



# -------------------------
# setup: uid check, trap handler, set editor
#
case `whoami` in
"root") ;;
*)      $ECHO "  You must be root to run audit_setup."
        exit 1
        ;;
esac


trap 'eval "$QUIT"' 1 2 3 15

EDITOR=${EDITOR:-$ED}

umask 077


[ "$HOSTNAME" = "" ] && {
    $ECHO "    This machine's hostname has not yet been set.
    Please run this script again after setting your system hostname."
    exit 1
}


[ ! -f $CONFIG ] && {
    $ECHO "    Configuration file [$CONFIG] not found."
    exit 1
}
[ ! -w $CONFIG ] && {
    $ECHO "    Configuration file [$CONFIG] not writable."
    exit 1
}



# -------------------------
# intro
#
$ECHO "
********************************************************************
*								   *
*		Audit Subsystem Setup Script			   *
*								   *
********************************************************************

"

$ECHO "
  The following steps will be taken to set up audit:
    1) establish startup flags for the audit daemon,
    2) establish startup flags for the auditmask,
    3) create the /dev/audit device (if needed),
    4) configure a new kernel (if needed).

"

while true
do
    $ECHO -n "  Do you wish to have security auditing enabled as part of
  system initialization (answer 'n' to disable) ([y]/n)?  "
    read ANS

    case $ANS in
    [yY]*)  break   ;;
    [nN]*)  $RCMGR set AUDITD_FLAG ""
            $ECHO "
  ***** AUDIT SETUP COMPLETE *****
            "
            exit    ;;
    "")     break   ;;
    esac
done



# -------------------------
# auditd
#
$ECHO "


    ----------------------------
     Audit Daemon Startup Flags
    ----------------------------

  Some of the options to 'auditd' control:
    1) destination of audit data,
    2) destination of auditd messages,
    3) action to take on an overflow condition,
    4) enable accepting audit data from remote auditd's.

"

# get auditd startup flags
#
while true
do
    MISC=

    # destination
    #
    $ECHO -n "  Destination of audit data (file|host:) [$DFLTDIR/$DFLTLOG]?  "
    read DEST

    # check for host:, default, and dir
    #
    [ "$DEST" = "" ] && {
        DEST=$DFLTDIR/$DFLTLOG
    }
    [ -d "$DEST" ] && {
        DEST="$DEST"/$DFLTLOG
    }
    LEN1=`$EXPR "$DEST" : ".*"`
    LEN2=`$EXPR "$DEST" : '.*\:'`
    HOST=0
    [ $LEN1 = $LEN2 ] && {
        HOST=1
    }

    # if dir doesn't exist, allow create of dir
    #
    DIR=`$EXPR "$DEST" : '\(.*\)/' '|' "$DEST"`
    [ ! -d "$DIR" -a $HOST = 0 ] && {
        while true
        do
            $ECHO -n "    Directory $DIR does not exist; create it now (y/[n])?  "
            read ANS
            case $ANS in
            [yY]*)  $MKDIR -p $DIR
                    break   ;;
            [nN]*)  break   ;;
            "")     break   ;;
            esac
        done        
    }
    $ECHO


    # console msgs
    #
    $ECHO -n "  Destination of auditd messages [$DFLTDIR/$DFLTCONS]?  "
    read MSG
    [ "$MSG" = "" ] && {
        MSG=$DFLTDIR/$DFLTCONS
    }
    [ -d $MSG ] && {
        MSG="$MSG"/$DFLTCONS
    }

    # if dir doesn't exist, allow create of dir
    #
    DIR=`$EXPR "$MSG" : '\(.*\)/' '|' "$MSG"`
    [ ! -d "$DIR" ] && {
        while true
        do
            $ECHO -n "    Directory $DIR does not exist; create it now (y/[n])?  "
            read ANS
            case $ANS in
            [yY]*)  $MKDIR -p $DIR
                    break   ;;
            [nN]*)  break   ;;
            "")     break   ;;
            esac
        done        
    }
    $ECHO


    # overflow action
    #
    $ECHO "  Action to take on an overflow condition may be one of:
    1)  change audit data location according to '$AUDITDLOC'
    2)  suspend auditing until space becomes available
    3)  overwrite the current auditlog
    4)  terminate auditing
    5)  halt the system
    "

    OVERFLOW=
    while [ -z "$OVERFLOW" ]
    do
        $ECHO -n "  Action (1-5) [1]?  "
        read ANS
        case "$ANS" in
        "1")    OVERFLOW="changeloc"    ;;
        "2")    OVERFLOW="suspend"      ;;
        "3")    OVERFLOW="overwrite"    ;;
        "4")    OVERFLOW="kill"         ;;
        "5")    OVERFLOW="halt"         ;;
        "")     OVERFLOW="changeloc"    ;;
        esac
    done

    [ "$OVERFLOW" = "changeloc" ] && {
        $ECHO "
    Don't forget to list in '$AUDITDLOC' the alternate directories
    in which to store audit data.
        "
        MISC=-r

        while true
        do
            $ECHO -n "    Do you wish to edit $AUDITDLOC now (y/[n])?  "
            read ANS
            case $ANS in
            [yY]*)  trap "" 2
                    "$EDITOR" $AUDITDLOC
                    trap 'eval "$QUIT"' 2
                    break   ;;
            [nN]*)  break   ;;
            "")     break   ;;
            esac
        done        
    }
    $ECHO


    # net server
    #
    while true
    do
        $ECHO -n "  Accept data from remote auditd's (y/[n])?  "
        read ANS
        case $ANS in
        [yY]*)  MISC="$MISC -s"
                $ECHO "
    Don't forget to place names of remote hosts from which data
    may be accepted into '$AUDITDCLNT'.
                "
                while true
                do
                    $ECHO -n "    Do you wish to edit $AUDITDCLNT now (y/[n])?  "
                    read ANS
                    case $ANS in
                    [yY]*)  trap "" 2
                            "$EDITOR" $AUDITDCLNT
                            trap 'eval "$QUIT"' 2
                            break   ;;
                    [nN]*)  break   ;;
                    "")     break   ;;
                    esac
                done        
                break   ;;

        [nN]*)  break   ;;
        "")     break   ;;
        esac
    done
    $ECHO


    # miscellaneous
    #
    $ECHO -n "  Further options are available for advanced users of the audit system
  (please refer to the auditd manpage).  If you wish to specify any further
  options you may do so now (<cr> for none):  "
    read ANS
    [ "$ANS" != "" ] && {
        MISC="$MISC $ANS"
    }
    $ECHO


    # final confirmation
    #
    $ECHO "
  Startup flags for 'auditd' set to:
    -l "$DEST -c $MSG -o $OVERFLOW $MISC"
    "
    while true
    do
        $ECHO -n "  Is this correct ([y]/n)?  "
        read ANS
        case $ANS in
        [yY]*)  break 2 ;;
        [nN]*)  break 1 ;;
        "")     break 2 ;;
        esac
    done
    $ECHO

done


# set AUDITD_FLAG in /etc/rc.config
#
$RCMGR set AUDITD_FLAG "-l $DEST -c $MSG -o $OVERFLOW $MISC"



# -------------------------
# auditmask
#
$ECHO "


    -------------------------
     Auditmask Startup Flags
    -------------------------

  The auditmask establishes which events get audited.  This can be specified
  by:
    1) having the auditmask read a list of events from a file,
      -or-
    2) specifying a list of events on the command line.

  Events can refer to syscalls, trusted events, site-defined events, or
  alias names.


  The file '$AUDITEVNTS' contains a list of all auditable system
  calls and trusted (application) events.  You may either modify this file
  or use it as a template.

  The file '$AUDITALIAS' contains a set of aliases by which
  logically related groupings of events may be constructed.  You may modify
  this set of aliases to suit your site's requirements.

"

# get auditmask startup flags
#
while true
do

    # redirect events from file or command line
    #
    while true
    do
        INPUT=
        $ECHO -n "  Enter filename containing event list or * to indicate that events
  will be listed on the command line (<cr> for no events):  "
        read ANS

        case $ANS in
        \*)     $ECHO -n "
    Enter event list:  "
                read EVNT
                break   ;;

        "")     break   ;;

        *)      EVNT=$ANS
                INPUT=1
                [ ! -f "$EVNT" ] && {
                    $ECHO "    $EVNT does not yet exist."
                }
                $ECHO
                while true
                do
                    $ECHO -n "    Do you wish to edit "$EVNT" now (y/[n])?  "
                    read ANS
                    case $ANS in
                    [yY]*)  trap "" 2
                            "$EDITOR" $EVNT
                            trap 'eval "$QUIT"' 2
                            break   ;;
                    [nN]*)  break   ;;
                    "")     break   ;;
                    esac
                done        
                break   ;;

        esac
    done


    $ECHO "

  The auditmask also sets various style flags such as:
    1) 'exec_argp'   - audit argument vector to exec system calls
    2) 'exec_envp'   - audit environment vector to exec system calls
    3) 'login_uname' - audit recorded username in failed login events

    "

    AUDSTYL=
    $ECHO -n "  Enable exec_argp ([y]/n)?  "
    read DEST
    [ "$DEST" = "" -o "$DEST" = "y" -o "$DEST" = "Y" ] && {
        AUDSTYL="$AUDSTYL -s exec_argp"
    }
    $ECHO

    $ECHO -n "  Enable exec_envp (y/[n])?  "
    read DEST
    [ "$DEST" = "y" -o "$DEST" = "Y" ] && {
        AUDSTYL="$AUDSTYL -s exec_envp"
    }
    $ECHO

    $ECHO -n "  Enable login_uname ([y]/n)?  "
    read DEST
    [ "$DEST" = "" -o "$DEST" = "y" -o "$DEST" = "Y" ] && {
        AUDSTYL="$AUDSTYL -s login_uname"
    }
    $ECHO


    # final confirmation
    #
    $ECHO "
  Startup flags for 'auditmask' set to:"
    if [ "$INPUT" = "1" ]
    then
        $ECHO "    $AUDSTYL < $EVNT"
    else
        $ECHO "    $AUDSTYL $EVNT"
    fi
    $ECHO

    while true
    do
        $ECHO -n "  Is this correct ([y]/n)?  "
        read ANS
        case $ANS in
        [yY]*)  break 2 ;;
        [nN]*)  break 1 ;;
        "")     break 2 ;;
        esac
    done
    $ECHO

done

# set AUDITMASK_FLAG in /etc/rc.config
if [ "$INPUT" = "1" ]
then
    $RCMGR set AUDITMASK_FLAG "$AUDSTYL < $EVNT"
else
    $RCMGR set AUDITMASK_FLAG "$AUDSTYL $EVNT"
fi



# -------------------------
# check /dev/audit
#
[ ! -f /dev/audit ] && {
    (cd /dev; /dev/MAKEDEV audit > /dev/null)
}



# -------------------------
# rebuild kernel
#
$ECHO "


    ----------------------
     System Configuration
    ----------------------
"

$GREP -qs DEC_AUDIT $CONFIG

# audit already configured
#
if [ $? = 0 ]
then
    $ECHO "  $HOSTNAME is already configured for security auditing ($CONFIG).
"

#   start auditd, set auditmask
#
    STARTAUDIT=
    while [ -z "$STARTAUDIT" ]
    do
        $ECHO -n "  Would you like to start audit now ([y]/n)?  "
        read ANS
        case $ANS in
        [yY]*)  STARTAUDIT=1    ;;
        [nN]*)  STARTAUDIT=0    ;;
        "")     STARTAUDIT=1    ;;
        esac
    done

    [ "$STARTAUDIT" = 1 ] && {
        $AUDITD `$RCMGR get AUDITD_FLAG`
        if [ "$INPUT" = "1" ]
        then
            $AUDITMASK $AUDSTYL < $EVNT
        else
            $AUDITMASK $AUDSTYL $EVNT
        fi
        $ECHO "
    '$AUDITD' started.
    '$AUDITMASK' set."
    }


# audit to be configured
#
else

#   update config file
#
    trap "" 2
    $ECHO "  Updating configuration file '$CONFIG' for security auditing..."
    $ED - $CONFIG << END > /dev/null
?^options
a
options		DEC_AUDIT
.
w
q
END
    CONFIGMARK=1
    $ECHO "  Configuration file '$CONFIG' updated.
    "
    trap 'eval "$QUIT"' 2


#   config system
#
    while true
    do
        $ECHO -n "  Would you like to have a kernel built now (y/[n])?  "
        read ANS
        case $ANS in
        [yY]*)  $ECHO -n "    Running '$DOCONFIG -c $HOSTNAME'...."
                $DOCONFIG -c $HOSTNAME
                $ECHO "    Configuration complete.  
    You may move /sys/$HOSTNAME/vmunix into place and reboot."
                break   ;;

        [nN]*)  $ECHO "    Your config file has been updated.  You may 'doconfig' when ready."
                break   ;;
        "")     $ECHO "    Your config file has been updated.  You may 'doconfig' when ready."
                break   ;;
        esac
    done
fi

$ECHO "
  ***** AUDIT SETUP COMPLETE *****
"
exit 0
