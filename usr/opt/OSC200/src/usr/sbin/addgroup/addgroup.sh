#!/bin/sh
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
# @(#)$RCSfile: addgroup.sh,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/04 08:23:30 $
# 
# addgroup.sh
#
# Purpose:	Add a new group number to /etc/group
# Remarks:	Returns the group number.
# Background:	Port to DEC OSF/1 by Tom Peterson 2/19/92 based on
#		addgroup.sh v5.1 (ULTRIX) 3/30/91 - Copyright (c) 1984, 1989
# Usage:	addgroup
# Environment:	Bourne shell script
#
#

PATH=/sbin:/usr/sbin:/usr/bin
export PATH
umask 022

LOCKFILE=/etc/ptmp

#
# Make sure its root.
#
if test ! -w /etc/group
then
    dspmsg addgroup.cat 1 'Please su to root first.\n'
    exit 1
fi

#
# Set the traps and LOCKFILE only if not called from another script (adduser)
#
if test "${#}" -lt 1
then
# Trap signals
	trap 'rmdir ${LOCKFILE} ; exit 0' 0
	trap 'rmdir ${LOCKFILE} ; exit 1' 1 2

# Lock the passwd and group files
	if mkdir ${LOCKFILE}
	then
	    :
	else
	    dspmsg addgroup.cat 13 '\nError, the /etc/group file is busy.\n'
	    exit 1
	fi
fi

#
# See if this system is an NIS (formerly YP) client.
#
tail -1 /etc/group | grep "^[+-]:" > /dev/null
if [ $? -eq 0 ]
then
	dspmsg addgroup.cat 2 '\nNIS in use. Refer to the network documentation for adding new groups.\n'
	exit 1
fi

#
# Get new group name
#
while true
do
    if test "${#}" -lt 1
    then
	dspmsg addgroup.cat 3 'Enter a new group name or <Return> to exit: '
	if read GROUP
	then
	    true
	else
	    dspmsg addgroup.cat 14 'Could not read the new group name.\n'
	    dspmsg addgroup.cat 5 'New entry not created.\n'
	    exit 1
	fi
    else
	GROUP="${1}"
    fi
    case "${GROUP}"
      in
	*:*)
	    dspmsg addgroup.cat 4 'Error, illegal characters in group name.\n'
	    ;;
	'')
	    dspmsg addgroup.cat 5 'New entry not created.\n'
	    exit 0
	    ;;
	?????????*)
	    dspmsg addgroup.cat 6 'Error, the group name is too long (must be 8 characters or less).\n'
	    ;;
	*)
#
# See if group already exists in passwd file, if so exit
#
	    if grep -q "^${GROUP}:" /etc/group
	    then
		dspmsg addgroup.cat 7 'Error, group %1$s is already in the /etc/group file.\n' "${GROUP}"
	    else
		break
	    fi
	    ;;
    esac
    if test "${#}" -ge 1
    then
	exit 1
    fi
done
#
# Get the group number for the new user.  Sort the group file on gid,
# get the largest gid, validity check it as a valid number,
# then add 5 to it to get the new gid.
#
GID=`sed -n '/^[^:]*:[^:]*:[0-9][0-9]*:/p' < /etc/group | sort -nt: +2 -3 | tail -1 | cut -d: -f3`
#
# Check for valid $gid
#
if test ! "${GID}"
then
    dspmsg addgroup.cat 8 '\nError, the /etc/group file is corrupt.\n'
    dspmsg addgroup.cat 9 'Exiting %1$s.  New entry not created.\n' "${0}"
    exit 1
fi
DEFGID=`expr "${GID}" + 5`
#
while true
do
    dspmsg addgroup.cat 10 '\nEnter a new group number [%1$s]: ' "${DEFGID}"
    if read GNUM
    then
	case "${GNUM}"
	  in
	    '')
		GNUM="${DEFGID}"
		break
		;;
	    *)
		if expr "${GNUM}" : '[^0-9]' > /dev/null
		then
		     dspmsg addgroup.cat 11 '\nError, the group number must be all digits.\n'
		else
#
# See if group number already exists in group file, if so get another.
#
		    if grep -q "^[^:]*:[^:]*:${GNUM}:" /etc/group
		    then
			 dspmsg addgroup.cat 12 '\nError, the group number %1$s is already in the /etc/group file.\n' "${GNUM}"
		    else
			break
		    fi
		fi
		;;
	esac
    else
    	dspmsg addgroup.cat 5 'New entry not created.\n'
	exit 1
    fi
done
#
# Add the group to the /etc/group file
#
echo "${GROUP}:*:${GNUM}:" >> /etc/group
dspmsg addgroup.cat 15 '\nGroup %1$s was added to the /etc/group file.\n' "${GROUP}"
exit "${GNUM}"
