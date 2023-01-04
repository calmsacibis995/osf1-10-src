#! /usr/bin/ksh
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
 
#
# Find files in sandbox hierarchy
#
#	 Usage: genloc files ...
#
# Developed by Michael D. Fairbrother.
#
# DEFAULT VALUES:

	OPTIONS="d"
	PWD=${PWD:=$(/bin/pwd)}		# Can't use environment variable 
					# not all users use ksh.
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


test ${WORKON} || {			# Since this program depends on 
					# environment variables that are set
					# when the user invokes workon().

					# Simulate workon environment
					# for non-backed non-sandbox case
	test ${SOURCEBASE} || {
	    print -u2 "error: SOURCEBASE not defined in environment"
	exit 1
	}

	WORKON=1
	BACKED_SOURCEDIR=${SOURCEBASE}
	export WORKON BACKED_SOURCEDIR
}


set -- $(getopt $OPTIONS $*)
if [ $? != 0 ]
then
	print -u2 "genloc [-d] files"
	exit 2
fi

for i in $*
do
	case $i in
		-d ) DIR_ONLY=1; shift ;;
		--) shift; break;;
	esac
done

test $# || {				# Check to see if there were any
					# arguments.

	print -u2 "Usage: genloc [-d] files"
	exit
}
FILES="$*"
for FILE in ${FILES}
do
	case ${FILE} in 
		\/*)
			SB_DIR=$(dirname ${FILE})
			CSD=${SB_DIR##*$(dirname ${SOURCEBASE})}	
			FILE=$(basename ${FILE})
			;;

		*)	# No path given assume they want to find the file in
			# the src directory. Strip down to find current 
			# sandbox dir.
			case ${PWD} in
				*src*) CSD=${PWD##*$(dirname ${SOURCEBASE})}
				;;
				
				*) CSD=/src${MAKEDIR##*$(dirname ${SOURCEBASE})}
				;;
			esac
			;;
	esac

	#
	# Strip path field separator.
	# Strip off src and add in current sandbox dir.
	#

	SEARCH_PATH=$(print "${BACKED_SOURCEDIR}" | \
		sed -e 's+:+ +'g | \
		sed -e 's+/src+'${CSD}' +'g )

	for DIR in ${SEARCH_PATH}
	do
		if [[ -r ${DIR}/${FILE} ]]
		then
			if [[ ${DIR_ONLY:=0} -eq 1 ]]
			then
				dirname ${DIR}/${FILE}
			else
				print ${DIR}/${FILE}
			fi
			break
		fi
	done
done
