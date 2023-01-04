#!/sbin/sh
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
# @(#)$RCSfile: Avl.sh,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/16 17:09:06 $ 
#
#	AvlMgr.sh
#		Attribute Value List Mgt
#		manages files that contain records of the form:
#			VAR=VAL
#			export VAR
#
#		
#	AvlMgr set variable value
#
#	AvlMgr get variable
#
#	AvlMgr get variable default_value
#
#
#		Digital Equipment Corporation, Maynard, MA
#			All rights reserved.
#
#	This software is furnished under a license and may be used and
#	copied  only  in accordance with the terms of such license and
#	with the  inclusion  of  the  above  copyright  notice.   This
#	software  or  any  other copies thereof may not be provided or
#	otherwise made available to any other person.  No title to and
#	ownership of the software is hereby transferred.		
#
#	The information in this software is subject to change  without
#	notice  and should not be construed as a commitment by Digital
#	Equipment Corporation.					
#
#	Digital assumes no responsibility for the use  or  reliability
#	of its software on equipment which is not supplied by Digital.
#
#	000	?		jps
#	001	30-may-1990	ccb
#		adapt for use in ISL

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/Error



:	-AvlExists
#		Checks to see if an attribute exists
#
#	given:	$1 - attribute name
#	does:	checks file described in global AVL_FILE for existence of
#			the attribute
#	return:	0 if attr exists, 1 if not, 2 on error.

AvlExists()
{ (
	[ $# = 1 ] ||
	{
		Error "usage: Avl exists <var>"
		return 2
	}
	AVL_VAR=$1

	egrep -s '^'$AVL_VAR= $AVL_FILE
) }



:	-AvlGet
#		get a value for an attribute
#
#	given:	$1 - an attribute name
#		$2 .. $n - default values for the attribute
#	does:	look up the attribute name
#	return:	NIL

AvlGet()
{ (
	if [ $# -ge 1 ]; then
	{
		AVL_VAR=$1
	
		AVL_LINE=`egrep '^'${AVL_VAR}= $AVL_FILE` 2>/dev/null
		if [ -z "$AVL_LINE" ]; then
		{
			[ $# -ge 2 ] &&
			{	# default values provided
				shift
				eval echo $*
			}
		}
		else
		{
			AVL_LINE="`echo $AVL_LINE | sed 's/=/ /'`"
			set $AVL_LINE
			shift; 
			eval echo $*
		}; fi
	}
	else
	{
		Error "USAGE: AvlMgr get {variable} "
	}; fi
) }



:	-AvlSet
#		set the value of an AVL
#
#	given:	$1 - the attribute name
#		$2 - the value to set
#	does:

AvlSet()
{ (
	[ $# -lt 2 ] &&
	{
		Error "usage: AvlSet <var> <val>"
		return 2
	}

	AVL_VAR=$1
	shift

	#! it's likely that we can quote these things even it they're
	#  singletons
	
	AVL_VAL=\"$*\"

	if AvlExists $AVL_VAR; then
	{
		# existing entry - replace it
		#! this edit fails if there are commas in
		#!  the value

		ed - $AVL_FILE <<xxEOFxx
/^$AVL_VAR=/d
i
$AVL_VAR=$AVL_VAL
.
w
q
xxEOFxx
	}
	else
	{	# create new entry
		echo "$AVL_VAR=$AVL_VAL
export $AVL_VAR"   >> $AVL_FILE

	}; fi
) }



:	Avl
#		general interface, used to be Main
#
#	

Avl()
{ (
	AVL_FILE=$1
	AVL_CMD=$2
	shift 2

	[ -f $AVL_FILE ] ||
	{
		Error "Avl: file ($AVL_FILE) does not exist"
		return 2
	}

	case $AVL_CMD in
	set)	AvlSet "$@"
		;;
	get)	AvlGet "$@"
		;;
	exists)	AvlExists "$@"
		;;
	*)	Error "Avl(): Invalid command 'AvlMgr $AVL_FILE ($AVL_CMD)'
usage: Avl file [get | set | exists] <variable> [value]"

	esac
) }


