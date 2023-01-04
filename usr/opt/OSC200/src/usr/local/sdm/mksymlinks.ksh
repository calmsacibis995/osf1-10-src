#!/usr/bin/ksh
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
# @(#)$RCSfile: mksymlinks.ksh,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/17 15:14:18 $
# 
#
# Script to make symbolic links in the release area.
#
# Program:  mksymlinks <tostage> <from_path[s]> <to_path[s]> <file[s]>
#
# Where 
#	tostage		-  Where the path name of the release directory.
#
#	from_path[s] 	-  Is the path names to make the symbolic links from.
#			   If more than one, the number has to match
#			   the number of to_paths.
#
#	to_path[s]	-  The path names of the symbolic links to be
#			   created.
#
#	file[s]		-  Names of the symbolic links
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

TOSTAGE=$1

set -A FROM_PATH -- $2
set -A TO_PATH -- $3
set -A FILES -- $4

if [ ${#FROM_PATH[*]} -gt 1 ]
then
	if [ ${#FROM_PATH[*]} -ne ${#TO_PATH[*]} ]
	then
		print -u2 "error: invalid pathname mapping."
		exit 2
	fi
else
	TMP_FROMPATH=${FROM_PATH}
	COUNTER=${#TO_PATH[*]}
	while [ ${COUNTER} -gt 0 ]
	do
		TMP_ARRAY="${TMP_ARRAY:+${TMP_ARRAY} }${TMP_FROMPATH}"
		COUNTER=$((COUNTER - 1))
	done
	set -A FROM_PATH -- ${TMP_ARRAY}
fi

COUNTER=0
MAX_TIMES=${#TO_PATH[*]}
while [ COUNTER -lt MAX_TIMES ]
do
	TODIR=${TO_PATH[${COUNTER}]}
	FROMDIR=${FROM_PATH[${COUNTER}]}
	FILE=${FILES[${COUNTER}]}
	if [ "${FILE}" != "nolink" ] 
	then	
		if [ ! -d ${TOSTAGE}${TODIR} ]
		then
			print -u2 "creating directory ${TOSTAGE}${TODIR}"
			mkdir -p ${TOSTAGE}${TODIR}
		fi
		if [ -L ${TOSTAGE}${TODIR}/${FILE} ]
		then
			print -u2 "removing previous symbolic link"
			rm -f ${TOSTAGE}${TODIR}/${FILE}
		fi
		print "ln -s ${FROMDIR} ${TOSTAGE}${TODIR}/${FILE}"
		ln -s ${FROMDIR} ${TOSTAGE}${TODIR}/${FILE}
	fi
	COUNTER=$((COUNTER + 1))
done
