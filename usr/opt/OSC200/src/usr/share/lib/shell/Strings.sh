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
# @(#)$RCSfile: Strings.sh,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/01/29 14:40:30 $ 
# 
#
#	Strings.sh
#		strings manipulation routines
#
#	000	??-???-????	ccb



:	-Parse
#		separate into words using separator
#
#	given:	$1 separator characters to use
#		$2 - $n strings to separate
#	does:	uses the separators to break the arglist into words
#	return:	nothing

Parse()
{ (
	IFS=$1
	shift
	echo $*
) }


:	-ToLower
#		coerce to lower case
#
#	given:	$* - strings to be coerced
#	does:	coerces argument strings to upper case and places
#		results on stdout
#	return:	NIL

ToLower()
{
	echo $* | dd conv=lcase 2> /dev/null
}


:	-ToUpper
#		alias for Ucase

ToUpper()
{
	Ucase $*
}



:	-Ucase
#		echo arguments upcased
#
#	given:	$* - some text
#	does:	upcases the text and writes it to stdout
#	return:	ignore

Ucase() {	echo $* | dd conv=ucase 2> /dev/null;	}

