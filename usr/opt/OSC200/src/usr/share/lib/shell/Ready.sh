:	-Ready
#		affirm user readiness
#
#	given:	nothing
#	does:	wait for user to affirm readiness
#	return:	VOID
#
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
# @(#)$RCSfile: Ready.sh,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/29 14:40:21 $ 
# 
#

Ready()
{ (
	X=		# user input

	while :
	do
		sleep 2
		echo "Are you ready (y/n)? \c"
		read X
		X=`echo $X`
		[ "$X" = 'y' -o "$X" = 'Y' ] && return
	done
) }
