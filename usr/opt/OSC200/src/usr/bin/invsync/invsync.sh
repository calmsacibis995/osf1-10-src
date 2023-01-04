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
# @(#)$RCSfile: invsync.sh,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/01/29 17:46:39 $ 
#
#	invsync.sh -
#		inventory synchronization program
#
#       @(#)invsync.sh	3.2     (DEC OSF/1)    11/7/91
# *********************************************************************
# *                                                                   *
# *      Copyright (c) Digital Equipment Corporation, 1991, 1994      *
# *                                                                   *
# *                       All Rights Reserved.                        *
# *                                                                   *
# *********************************************************************
#
#	000	16-oct-1990	ccb
#		bugfixes. integrate udetect call
#
#	001	15-may-1991	ech
#		ported to OSF/1.
#

CDPATH= ;export CDPATH		# assure no surprises.
PATH=/isl:/usr/bin:/usr/lbin:/usr/sbin:/sbin:/usr/ucb

:	-Main
#		main routine
#

Main()
{
	cd  /usr/.smdb.

	# Establish a list of subsets which must be syncronized,
	#  taken in install-order.

	SYNCLIST=`ls -rt *.inv | sed 's/\.inv//g'`
	# determine which of these are installed
	INSTALLED=
	for S in $SYNCLIST
	{
		[ -f $S.lk ] &&
			INSTALLED="$INSTALLED $S"
	}

	set xxx $INSTALLED; shift


	# sort the initial inventory ascending by pathname
	Y=$1
	echo "Initial Subset: $Y"
	sort -o $Y.#syn +9 -10 $Y.inv

	# Iteratively synchronize inventories 2 at a time.
	#  the oldest two are synchronized into one which
	#  is in turn used as the oldest in the next iteration
	#

	while [ $# -gt 1 ]
	do
		# pop the first two from the list.
		X=$1 Y=$2
		shift

		mv $X.#syn $X.syn 2> /dev/null

		# get original copies the inventories
		#  if we haven't already and sort them
		[ -f $X.syn ] ||
		{
			sort -o $X.syn +9 -10 $X.inv &
			SORTPID=$!
		}

		[ -f $Y.syn ] ||
			sort -o $Y.syn +9 -10 $Y.inv

		(Wait SORTPID)

		# synchronize the inventory pair
		echo "Synchronize $Y...\c"
		usync $X.syn $Y.syn > $Y.#syn
		echo "done."

		rm -f $X.syn
	done

	mv $Y.#syn MSI
	(cd /;/usr/lbin/udetect) < MSI > CFI
}



:	-Wait
#		intelligent wait routine
#
#	given:	$1 - the name of the variable contianing the pid of the
#			process to wait for (yes, this uses call-by-reference)
#		[$2 - $n] - optional string to eval if wait fails
#	does:	wait on the specified PID if contents of $1 is not null
#		if wait returns !0 status, eval remainder of command line
#		clear contents of variable specified in $1
#	return:	exit status of the wait

Wait()
{
	VAR=$1
	shift

	# was there a first arg?
	[ ! "$VAR" ] && return 0

	eval VAL=\$$VAR

	# was the value of the named variable set?
	[ ! "$VAL" ] && return 0

	# clear the value from the named variable
	eval $VAR=

	wait $VAL ||
	{
		STAT=$?
		eval $*
		return $STAT
	}
}

ARGS=$*

[ "$INVSYNC_DEBUG" ] || Main $ARGS
echo OK

