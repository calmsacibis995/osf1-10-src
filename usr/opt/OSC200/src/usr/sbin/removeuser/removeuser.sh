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
# @(#)$RCSfile: removeuser.sh,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/12/09 20:35:33 $
# 
#
# removeuser.sh
#
# Purpose:	To remove a user from the passwd file
# Remarks:
#	Removes user from /etc/passwd file, from /etc/group file, and
#	optionally deletes the user's home directory, all sub-directories
#	and files.
#
# Background:	Port to OSF/1 by Tom Peterson 2/19/92 based on
#		removeuser.sh v5.1 (ULTRIX) 3/30/91 - Copyright (c) 1984, 1989 1990
# Usage:	removeuser
# Environment:	Bourne shell script
# 
#
PATH=/sbin:/usr/sbin:/usr/bin
export PATH
umask 022
YES=`dspmsg removeuser.cat 27 '[yY]*'`
NO=`dspmsg removeuser.cat 28 '[nN]*'`

LOCKFILE=/etc/ptmp
MAIL=/usr/spool/mail

# Trap signals
trap 'rmdir ${LOCKFILE} ; exit 0' 0
trap 'rmdir ${LOCKFILE} ; exit 1' 1 2

# Lock the passwd and group files
if mkdir ${LOCKFILE}
then
    :
else
    dspmsg removeuser.cat 15 '\nError, the /etc/group or /etc/passwd file is busy.\n'
    exit 1
fi

# See if this system is an NIS (formerly YP) client.
tail -1 /etc/passwd | grep "^[+-]:" > /dev/null
yp_used="$?"
tail -1 /etc/group | grep "^[+-]:" > /dev/null
if [ $? -eq 0 ] || [ $yp_used -eq 0 ]
then
	dspmsg removeuser.cat 1 '\nNIS in use. Refer to the network documentation for adding users.\n'
	exit 1
fi

# Get the user's login name
dspmsg removeuser.cat 2 '\nEnter a login name to be removed or <Return> to exit: '
read USER
case "$USER" in
"")	dspmsg removeuser.cat 18 '\nNo user removed.\n'
	exit 0
	;;
*)	USERENTRY=`grep "^$USER:" /etc/passwd` ||
	{
		dspmsg removeuser.cat 3 'User (%1$s) was not found in the /etc/passwd file.\n' "$USER"
		exit 0
	}
	;;
esac

# Display user_entry record from passwd file and confirm it with user.
dspmsg removeuser.cat 4 'This is the entry for (%1$s) in the /etc/passwd file:\n\n %2$s\n' "$USER" "$USERENTRY"
while :
do
	dspmsg removeuser.cat 5 '\nIs this the entry you want to delete (y/n)? '
	read _X_
        case "$_X_" in
        ${YES})
		break
                ;;
        ${NO})
		dspmsg removeuser.cat 6 '\nUser (%1$s) was not removed.\n' "$USER"
		exit 0
                ;;
        esac
done

dspmsg removeuser.cat 7 'Working ...'

# Remove the user from /etc/passwd
if cp /etc/passwd /etc/ptmp/passwd
then
ed - /etc/ptmp/passwd <<EOF
/^${USER}:/d
w
q
EOF
else
	dspmsg removeuser.cat 16 '\nCould not copy the /etc/passwd file to /etc/ptmp/passwd.'
	dspmsg removeuser.cat 6 '\nUser (%1$s) was not removed.\n' "$USER"
	exit 1
fi
mv /etc/ptmp/passwd /etc/passwd

# Remove any auth entry
if [ -d /tcb/files/auth ]
  then
    ULETTER=`expr "${USER}" : '\(.\)'`
    AUTHDIR="/tcb/files/auth/${ULETTER}"
    rm -f "${AUTHDIR}/${USER}"
fi

if [ -f /etc/passwd.dir -o -f /etc/passwd.pag ]		# GA001
then
	dspmsg removeuser.cat 8 'Rebuilding the password database...'
	cd /etc
	mkpasswd passwd
fi

# Remove the user /etc/group
if cp /etc/group /etc/ptmp/group
then
ed - /etc/ptmp/group <<EOF
g/$/s//,/
g/:$USER,/s/$USER,//
g/,$USER,/s//,/
g/,$/s///
w
q
EOF
else
	dspmsg removeuser.cat 17 '\nCould not copy the /etc/group file to /etc/ptmp/group.'
	dspmsg removeuser.cat 6 '\nUser (%1$s) was not removed.\n' "$USER"
	exit 1
fi
mv /etc/ptmp/group /etc/group

dspmsg removeuser.cat 9 '\nEntry for (%1$s) removed.' "$USER"

# set trap values back to default values
trap 0 1 2

# free up the password file.
rmdir /etc/ptmp

HOMEPATH=`echo $USERENTRY|awk -F: '{print $6}'`


# Search for misc entries of user in administrative dirs/files
dspmsg removeuser.cat 19 \
   '\nSearching relevant directories and files for user (%1$s) ...\n' "$USER"
found=0

# search for /etc/exports entries (ignoring comment lines)
if [ -s /etc/exports ]
then
cat /etc/exports | sed -e "/^#/d" | grep -q "$HOMEPATH"
if [ "${?}" -eq 0 ]
then
	dspmsg removeuser.cat 20 \
	  '- Entries containing the home directory %1$s\n  for this user were found in the /etc/exports file.\n' "$HOMEPATH"
	found=1
fi
fi

# search for alias entries (ignoring comment lines)
if [ -s /var/adm/sendmail/aliases ]
then
cat /var/adm/sendmail/aliases | sed -e "/^#/d" | grep -q "$USER"
if [ "${?}" -eq 0 ]
then
	dspmsg removeuser.cat 21 \
	  '- Entries for (%1$s) were found in the /var/adm/sendmail/aliases file.\n' "$USER"
	found=1
fi
fi
if [ -s /usr/lib/mh/MailAliases ]
then
cat /usr/lib/mh/MailAliases | sed -e "/^#/d" | grep -q "$USER"
if [ "${?}" -eq 0 ]
then
	dspmsg removeuser.cat 22 \
	  '- Entries for (%1$s) were found in the /usr/lib/mh/MailAliases file.\n' "$USER"
	found=1
fi
fi

# search for cron files
if [ -d /var/spool/cron/crontabs ]
then
ls -l /var/spool/cron/crontabs | tr -s ' ' ' ' | cut -d" " -f3 | grep -q "$USER"
if [ "${?}" -eq 0 ]
then
	dspmsg removeuser.cat 23 \
	  '- Entries for (%1$s) were found in the /var/spool/cron/crontabs directory.\n' "$USER"
	found=1
fi
fi
if [ -d /var/spool/cron/atjobs ]
then
ls -l /var/spool/cron/atjobs | tr -s ' ' ' ' | cut -d" " -f3 | grep -q "$USER"
if [ "${?}" -eq 0 ]
then
	dspmsg removeuser.cat 24 \
	  '- Entries for (%1$s) were found in the /var/spool/cron/atjobs directory.\n' "$USER"
	found=1
fi
fi
if [ "$found" -eq 1 ]
then
	dspmsg removeuser.cat 25 \
	  'You may want to remove the entries for (%1$s) from these directories/files.\n' "$USER"
else
	dspmsg removeuser.cat 26 'None found.\n'
fi

while :
do
dspmsg removeuser.cat 10 \
'\nDo you want to remove the home directory, all subdirectories, \nfiles and mail for (%1$s) (y/n)? ' "$USER"
	read _X_
	case "${_X_}" in
	${YES})	while :
		do
			dspmsg removeuser.cat 11 \
			'The files for (%1$s) will be lost if not backed up.\n' "$USER"
			dspmsg removeuser.cat 12 \
			'Are you sure you want to remove these files (y/n)? '
			read _X_
			case "${_X_}" in
			${YES})
				dspmsg removeuser.cat 13 '\nRemoving %1$s\n' "$HOMEPATH"
				dspmsg removeuser.cat 13 '\nRemoving %1$s\n' "$MAIL/$USER"
				rm -rf $HOMEPATH $MAIL/$USER 2>&1 > /dev/null
				dspmsg removeuser.cat 29 '\nFinished removing user account for (%1$s)\n' "$USER"
				exit 0
				;;
			${NO})	break 2
				;;
			esac
		done
		;;
	${NO})	break
		;;
	esac
done

# 'no'
dspmsg removeuser.cat 14 '\n%1$s not removed.\n' "$HOMEPATH"
dspmsg removeuser.cat 29 '\nFinished removing user account for (%1$s)\n' "$USER"
exit 0
