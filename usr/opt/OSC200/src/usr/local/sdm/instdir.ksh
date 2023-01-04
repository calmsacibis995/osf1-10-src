#!/usr/bin/ksh -p
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
# @(#)$RCSfile: instdir.ksh,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/03/15 20:58:36 $
# 
#
# Script to make directories in the release area.
#
# DEFAULT VALUES:

	OPTIONS="m:g:o:i:"
	PWD=$(/bin/pwd)			# Can't use environment variable 
					# not all users use ksh.

	case $(uname) in
		OSF1)	
			CHOWN="/usr/bin/chown"
			MKDIR="/usr/bin/mkdir"
			CHGRP="/usr/bin/chgrp"
			CHMOD="/usr/bin/chmod"
			;;
		*)
			CHOWN="/etc/chown"
			MKDIR="/bin/mkdir"
			CHGRP="/bin/chgrp"
			CHMOD="/bin/chmod"
			;;
	esac
#
# ENVIRONMENT VARIABLES
#
#	DEBUG		- Used to debug this program.
#
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

test ${DEBUG} && { 			# Test to see if environment 
	 PS4='$LINENO+ '		# debugging turned on.
	 set -x
}

if [ "${RELEASE_OPTIONS}" != ""  ]
then
	IDFILE=$(eval print -- ${RELEASE_OPTIONS} | awk '{ print $2}' )
fi

set -- $(getopt $OPTIONS $*)
if [ $? != 0 ]
then
	print -u2 "Usage: instdir <-m mode> <-o owner> <-g group> dir"
	exit 2
fi
for i in $*
do
	case $i in
	-m )	MODE=$2; shift 2;;
	-o )	if [ "${IDFILE}" != "" ]
		then
			OWNER=$(grep 'uid	'${2}'[ |	]' ${IDFILE} |
				awk '{ print $3 }' )
		else
			OWNER=$2
		fi
		shift 2;;
	-g )	if [ "${IDFILE}" != "" ]
		then
			GROUP=$(grep 'gid	'${2}'[ |	]' ${IDFILE} |
				awk '{ print $3 }' )
		else
			GROUP=$2
		fi
		shift 2;;
		--) shift; break;;
	esac
done

if [ $# -ne 1 ]			 # Check to see if there were any
then				 # arguments.
	print -u2 "Usage: instdir <-m mode> <-o owner> <-g group> dir"
	exit
fi

DIR="$*"

${MKDIR} -p ${DIR} > /dev/null 2>&1
${CHOWN} ${OWNER} ${DIR} || exit
${CHGRP} ${GROUP} ${DIR} || exit
${CHMOD} ${MODE} ${DIR}  || exit
