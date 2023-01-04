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
# @(#)$RCSfile: Wait.sh,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/29 14:40:48 $ 
# 
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
#
#

Wait()
{
	_Wait_VAR=$1
	shift

	# was there a first arg?
	[ ! "$_Wait_VAR" ] && return 0

	eval _Wait_VAL=\$$_Wait_VAR

	# was the value of the named variable set?
	[ ! "$_Wait_VAL" ] && return 0

	# clear the value from the named variable
	eval $_Wait_VAR=

	wait $_Wait_VAL ||
	{
		_Wait_STAT=$?
		eval $*
		return $_Wait_STAT
	}
}
