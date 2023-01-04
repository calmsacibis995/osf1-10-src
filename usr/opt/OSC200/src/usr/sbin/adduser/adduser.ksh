#!/bin/ksh
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
# @(#)$RCSfile: adduser.ksh,v $ $Revision: 1.1.8.4 $ (DEC) $Date: 1993/09/30 19:25:19 $ 
# 

PATH=/sbin:/usr/sbin:/usr/bin
export PATH

UID_MAX=60000
LOCKFILE=/etc/ptmp
SKEL=/usr/skel
DEFPARENT=/usr/users
DEFSHELL=/bin/sh
DEFGROUP=users
HOMEMODES=0751
PARENTMODES=0755
DEFPASSMAXLIFE=60
DEFPASSMINLIFE=0
umask 022
YES=`dspmsg adduser.cat 34 '[yY]*'`
NO=`dspmsg adduser.cat 35 '[nN]*'`

if test ! -w /etc/passwd
then
    dspmsg adduser.cat 1 'Please su to root first.\n'
    exit 1
fi

# Trap signals

trap 'rmdir ${LOCKFILE} ; exit 1' 1 2

# Lock the passwd and group files

if mkdir ${LOCKFILE}
then
#    trap 'rmdir -f ${LOCKFILE} ; exit 0' 0
    :
else
    dspmsg adduser.cat 36 'The /etc/passwd file is busy.  Try again later.\n'
    exit 1
fi

#
# See if this system is a NIS client.
#
tail -1 /etc/passwd | grep -q "^[+-]:"
yp_used="$?"
tail -1 /etc/group | grep -q "^[+-]:"
if [ $? -eq 0 ] || [ $yp_used -eq 0 ]
then
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 2 '\nNIS in use. Refer to the network documentation for adding users.\n'
	exit 5
fi

# Get new user's login name

while true
  do
    dspmsg adduser.cat 3 '\nEnter a login name for the new user (for example, john): '
    if read USER
      then
	case "${USER}"
	  in
	    *:*)
		dspmsg adduser.cat 13 'Error, name can not contain colons.\n'
		;;
	    '')
		;;
	    ?????????*)
		dspmsg adduser.cat 4 'Error, name cannot be longer than 8 characters.\n'
		;;
	    *)
		break
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 0
    fi
done

# See if user already exists in passwd file, if so exit.
if grep -q "^${USER}:" /etc/passwd
  then
    rmdir ${LOCKFILE}
    dspmsg adduser.cat 5 'User (%1$s) is already in the /etc/passwd file.\n' "${USER}"
    exit 5
fi

# Get the user ID for the new user.  Sort the passwd file on uid,
#   get the largest uid, validity check it as a valid number,
#   then add one to it to get the new uid.

UID=`egrep -v '^nobody:|^nobodyV:' /etc/passwd | grep '^[^:]*:[^:]*:[0-9][0-9]*:' | cut -d: -f3 | sort -n | tail -1`

# Check for valid ${UID}

if test ! "${UID}"
  then
    dspmsg adduser.cat 6 '\nThe /etc/passwd file may be corrupt.\n'
    dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
    rmdir ${LOCKFILE}
    exit 1
fi

if test "${UID}" -gt ${UID_MAX}
  then
    dspmsg adduser.cat 33 '\nA UID greater than the maximum allowed (%1$s) was found.\n' "${UID_MAX}"
    dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
    rmdir ${LOCKFILE}
    exit 1
fi

UID=`expr "${UID}" + 1`
# Verfify UID
while true
  do
    dspmsg adduser.cat 8 'Enter a UID for (%1$s) [%2$s]: ' "${USER}" "${UID}"
    if read X
      then
	case "${X}"
	  in
	    '')
		break
		;;
	    *)
		N=`expr "${X}" : '[^0-9]'`
		if [ ${N} -gt 0 ]
		  then
		    dspmsg adduser.cat 9 'Bad value for UID, must be an integer\n'
		else
		    if [ ${X} -gt ${UID_MAX} ]
		      then
			dspmsg adduser.cat 10 'UID must be less than %1$s\n' "${UID_MAX}"
		    else
			if grep -q "^[^:]*:[^:]*:${X}:" /etc/passwd
			  then
			    dspmsg adduser.cat 11 'There is another account with UID %1$s. Use UID %1$s ([y]/n)? ' "${X}"
			    if read Y
			      then
				case "${Y}"
				  in
				    ${YES}|'')
					;;
				    *)
					continue
					;;
				esac
			    else
				rmdir ${LOCKFILE}
				dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
				exit 1
			    fi
			fi
			UID=${X}
			break
		    fi
		fi
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
done

# Get new user's real name

while true
  do
    dspmsg adduser.cat 12 'Enter a full name for (%1$s): ' "${USER}"
    if read NAME
      then
	case "${NAME}"
	  in
	    *:*)
		dspmsg adduser.cat 13 'Error, name can not contain colons.\n'
		;;
	    '')
		;;
	    *)
		break
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
done

# Get the login group for the new user.

while true
do
while true
  do
    dspmsg adduser.cat 14 'Enter a login group for (%1$s) [%2$s]: ' "${USER}" "${DEFGROUP}"
    if read LOGGROUP
      then
	case "${LOGGROUP}"
	  in
	    *:*)
		dspmsg adduser.cat 13 'Error, name can not contain colons.\n'
		;;
	    '')
		LOGGROUP="${DEFGROUP}"
		break
		;;
	    *)
		break
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
done

#   Get the group ID for the new user

    GID=`grep "^${LOGGROUP}:" /etc/group | cut -d: -f3`
    if test ! "${GID}"
      then
	dspmsg adduser.cat 15 '\nThe group %1$s was not found.  The existing groups are:\n\n' "${LOGGROUP}"
	cut -d: -f1 < /etc/group | pr -t -4
	while true
	  do
	    dspmsg adduser.cat 16 '\nDo you want to add group %1$s to the /etc/group file ([y]/n)? ' "${LOGGROUP}"
	    if read ADDGROUP
	      then
		case "${ADDGROUP}"
		  in
		    ${YES}|'')
			dspmsg adduser.cat 17 '\nAdding group %1$s to the /etc/group file...\n' "${LOGGROUP}"
			addgroup "${LOGGROUP}"
    			GID=`grep "^${LOGGROUP}:" /etc/group | cut -d: -f3`
			break
			;;
		    ${NO})
			continue 2
			;;
		    *)
			dspmsg adduser.cat 23 'Error, you must answer either y or n.\n'
			;;
		esac
	    else
		rmdir ${LOCKFILE}
		dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
		exit 1
	    fi
	done
    fi

LOGGID=${GID}

# 001 - Add the user to the login group specified

	grep "^${LOGGROUP}:" /etc/group | cut -d: -f4 | grep -q -w "${USER}"
	if test $? -ne 0
	then
	    if cp /etc/group /etc/ptmp/group
	    then
		ed /etc/ptmp/group > /dev/null << EOF
		/^${LOGGROUP}:/s/\$/,${USER}/
		g/:,/s//:/
		w
		q
EOF
		mv /etc/ptmp/group /etc/group
	    else
		dspmsg adduser.cat 37 'Unable to create temporary group file, user (%1$s) not added to group %2$s.\n' "${USER}" "${LOGGROUP}"
		rmdir ${LOCKFILE}
		dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
		exit 1
	    fi
	    rm -f /etc/ptmp/group
        else
	    dspmsg adduser.cat 19 '\nUser (%1$s) is already a member of group %2$s.\n' "${USER}" "${LOGGROUP}"
	fi
	break
done

# Get other groups if this user is to be part of any others

while true
do
while true
  do
    dspmsg adduser.cat 18 '\nEnter another group that (%1$s) should be a member of.\n(<Return> only if none): ' "${USER}"
    if read GROUP
      then
	case "${GROUP}"
	  in
	    *:*)
		dspmsg adduser.cat 13 'Error, name can not contain colons.\n'
		;;
	    '')
		break 2
		;;
	    *)
		break
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
done

#   Get the group ID for the new user

    GID=`grep "^${GROUP}:" /etc/group | cut -d: -f3`
    if test ! "${GID}"
      then
	dspmsg adduser.cat 15 '\nThe group %1$s was not found.  The existing groups are:\n\n' "${GROUP}"
	cut -d: -f1 < /etc/group | pr -t -4
	while true
	  do
	    dspmsg adduser.cat 16 '\nDo you want to add group %1$s to the /etc/group file ([y]/n)? ' "${GROUP}"
	    if read ADDGROUP
	      then
		case "${ADDGROUP}"
		  in
		    ${YES}|'')
			dspmsg adduser.cat 17 '\nAdding group %1$s to the /etc/group file...\n' "${GROUP}"
			addgroup "${GROUP}"
    			GID=`grep "^${GROUP}:" /etc/group | cut -d: -f3`
			break
			;;
		    ${NO})
			continue 2
			;;
		    *)
			dspmsg adduser.cat 23 'Error, you must answer either y or n.\n'
			;;
		esac
	    else
		rmdir ${LOCKFILE}
		dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
		exit 1
	    fi
	done
    fi

# Add the user to each group as it is specified 

	grep "^${GROUP}:" /etc/group | cut -d: -f4 | grep -q -w "${USER}"
	if test $? -eq 0
	then 
	    dspmsg adduser.cat 19 '\nUser (%1$s) is already a member of group %2$s.\n' "${USER}" "${GROUP}"
	else
	    if cp /etc/group /etc/ptmp/group
	      then
		ed /etc/ptmp/group > /dev/null << EOF
		/^${GROUP}:/s/\$/,${USER}/
		g/:,/s//:/
		w
		q
EOF
		mv /etc/ptmp/group /etc/group
	    else
		dspmsg adduser.cat 37 'Unable to create temporary group file, user (%1$s) not added to group %2$s.\n' "${USER}" "${GROUP}"
	    fi
	    rm -f /etc/ptmp/group
	fi

done

while true
  do
    dspmsg adduser.cat 20 'Enter a parent directory for (%1$s) [%2$s]: ' "${USER}" "${DEFPARENT}"
    if read PARENT
      then
	case "${PARENT}" in
	    '') PARENT="${DEFPARENT}"
		;;
	esac
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
    if test \! -d "${PARENT}"
      then
	while true
	  do
	    dspmsg adduser.cat 21 '%1$s was not found, do you want to create it ([y]/n)? ' "${PARENT}"
	    if read ADDDIR
	      then
		case "${ADDDIR}"
		  in
		    ${YES}|'')
			if mkdir -m ${PARENTMODES} -p "${PARENT}"
			  then
			    chgrp "${LOGGROUP}" "${PARENT}"
			    break 2
			else
			    dspmsg adduser.cat 22 'Unable to create %1$s.\n' "${PARENT}"
			    break
			fi
			;;
		    ${NO})
			break
			;;
		    *)
			dspmsg adduser.cat 23 'Error, you must answer either y or n.\n'
			;;
		esac
	    else
		rmdir ${LOCKFILE}
		dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
		exit 1
	    fi
	done
    else
	break
    fi
done

# Get the users login shell.

while true
  do
    dspmsg adduser.cat 24 'The shells are:\n\n'
    pr -4 -t /etc/shells
    echo
    dspmsg adduser.cat 25 'Enter a login shell for (%1$s) [%2$s]: ' "${USER}" "${DEFSHELL}"
    if read LSHELL
      then
        if [ ! -f "${LSHELL}" ]
          then
            LSHELL=`grep "/${LSHELL}$" /etc/shells | tail -1l`
        fi
        if [ ! -n "${LSHELL}" ]
          then
            LSHELL="${DEFSHELL}"
        fi
	if [ "${LSHELL}" = /bin/sh ]
	  then
	    LSHELL=''
	fi
    else
	rmdir ${LOCKFILE}
	dspmsg adduser.cat 7 'Exiting %1$s.  New entry not created.\n' "${0}"
	exit 1
    fi
    break
done

dspmsg adduser.cat 32 '\nAdding new user...\n'

# Make sure parent directories for everything exist.

if test \! -d /usr/spool/mail
then
    mkdir -p /usr/spool/mail
    # give mail spool directory the proper protection. QAR 6466
    chmod 1777 /usr/spool/mail
fi

# Add the user to the password file.

echo "${USER}:Nologin:${UID}:${LOGGID}:${NAME}:${PARENT}/${USER}:${LSHELL}" >> /etc/passwd
if [ -f /etc/passwd.pag -o -f /etc/passwd.dir ]
  then
    dspmsg adduser.cat 26 'Rebuilding the password database...\n'
    ( cd /etc ; mkpasswd passwd )
   else
     dspmsg adduser.cat 39 'The hashed password database does not exist.  Do you want to create it ([y]/n)? '
     if read Y
     then
     	case "${Y}"
 	in
 	   ${YES}|'')
 	   	dspmsg adduser.cat 26 'Rebuilding the password data base...\n'
 		( cd /etc ; mkpasswd passwd )
 		;;
 	esac
     fi
fi

# Unlock the password and group files

rmdir ${LOCKFILE}
trap - 1 2

# Add the user to the auth database

if [ -d /tcb/files/auth ]
then
    ULETTER=`expr "${USER}" : '\(.\)'`
    AUTHDIR="/tcb/files/auth/${ULETTER}"
    if [ ! -d "${AUTHDIR}" ]
      then
	mkdir -p "${AUTHDIR}"
	chown auth "${AUTHDIR}"
	chgrp auth "${AUTHDIR}"
	chmod 660 "${AUTHDIR}"
    fi

    if [ -f "${AUTHDIR}/${USER}" ]
      then
	dspmsg adduser.cat -s 2 1 'An auth database entry already exists for (%1$s)\n' "${USER}"
    else
    cat > "${AUTHDIR}/${USER}" <<EOF
${USER}:u_name=${USER}:u_id#${UID}:\\
	:u_pwd=Nologin:\\
	:u_lock@:chkent:
EOF
    fi
    chown auth "${AUTHDIR}/${USER}"
    chgrp auth "${AUTHDIR}/${USER}"
    chmod 660 "${AUTHDIR}/${USER}"
    dspmsg adduser.cat -s 2 5 'Do you wish to edit the auth file entry for this user (y/[n])? '
    if read X
      then
	case "${X}"
	  in
	    ${YES})
                dspmsg adduser.cat 40 'Invoking ed editor ......\n\n'
		ed "${AUTHDIR}/${USER}"
		;;
	esac
    fi
fi

# Create home and bin directories, and set-up files

dspmsg adduser.cat 27 'Creating home directory...\n\n'
cd "${PARENT}"
if [ "${?}" -ne 0 ]
  then
    dspmsg adduser.cat 28 'Unable to cd to parent directory %1$s\n' "${PARENT}"
    exit 1
fi
if [ -d "${USER}" ]
  then
    dspmsg adduser.cat 29 '%1$s/%2$s already exists.\n' "${PARENT}" "${USER}"
else
    mkdir -m ${HOMEMODES} "${USER}"
    chown "${USER}" "${USER}"
    chgrp "${LOGGROUP}" "${USER}"
fi 
# cin 001 begin #
( cd "${SKEL}"
  for I in `ls -A`
    do
      if [ ! -f "${PARENT}/${USER}/${I}" ]
        then
          cp -pR -- "${I}" "${PARENT}/${USER}" && \
          chown "${USER}" "${PARENT}/${USER}/${I}" && \
          chgrp "${LOGGROUP}" "${PARENT}/${USER}/${I}"
      fi
  done )
# cin 001 end #


# Set a password

dspmsg adduser.cat 30 'You must enter a new password for (%1$s).\n' "${USER}"

if passwd "${USER}"
  then
    :
else
    dspmsg adduser.cat 31 'Warning, the password for (%1$s) was not set.\n' "${USER}"
fi
dspmsg adduser.cat 38 'Finished adding user account for (%1$s).\n' "${USER}"
exit 0
