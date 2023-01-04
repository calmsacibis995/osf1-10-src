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
# @(#)$RCSfile: Dialog.sh,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/20 09:22:53 $
# 
#		Prompt for, and accept, user input.
#
#	Given:	$1 = prompt string
#		$2 = name of variable to be returned with user input
#		$3 = OPTIONAL default value for input
#
#	Does:	Displays $1 as a prompt.  If $3 is present, displays
#		$3 within brackets as default value.  Accepts user
#		input.
#
#	Return:	Variable bearing name of $2 containing user's input.
#		If null input, returns contents of $3 (null if no $3).

:	-Dialog
Dialog ()
{
	echo "$1\c"
	[ "$3" ] && echo " [$3]\c"
	echo ": \c"
	read aZ
	[ "$aZ" ] || aZ="$3"
	eval $2='$aZ'
}
