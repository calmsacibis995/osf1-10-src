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
# @(#)$RCSfile: Lists.sh,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/11/12 22:06:50 $
# 

Car()
{
	echo $1
}

Cdr()
{
	shift; echo $*
}

Length ()
{
	echo $#
}

# takes two lists. Returns 0 if $1 is contained by $2
# assumption: both lists have no duplicate items in them.

ListContained()
{(
	LIST1=$1
	LIST2=$2
	
	CNT_LIST1=`Length $LIST1`
	CNT_LIST2=`Length $LIST2`

	[ "$CNT_LIST1" -gt "$CNT_LIST2" ] &&
		return 1

	for I in $LIST1
	{
		Member $I $LIST2 ||
			return 1
	}
	
	return 0
)}

# takes two lists, echoes back items in list1 but not in list2

ListUniq()
{(
	LIST1=$1
	LIST2=$2

	for I in $LIST1
	{
		Member $I $LIST2 ||
			echo $I
	}
	
	return 0
)}

# takes a list, eliminates all duplicate items and echoes back the list.

ListNoDup()
{(
	INLIST="$*"
	OUTLIST=
	for I in $INLIST
	{
		Member $I $OUTLIST || 
			OUTLIST="$OUTLIST $I"
	}
	echo $OUTLIST
)}

# takes two lists. Returns 0 if the two lists are the same.
# assumption: both lists have no duplicate items in them.

ListEq()
{(
	LIST1=$1
	LIST2=$2
	
	CNT_LIST1=`Length $LIST1`
	CNT_LIST2=`Length $LIST2`

	[ "$CNT_LIST1" -eq "$CNT_LIST2" ] ||
		return 1

	for I in $LIST1
	{
		Member $I $LIST2 ||
			return 1
	}
	
	return 0
)}


# takes a list and echoes back its componets in reverse order.

ListReverse()
{(
	RLIST=

	for I in "$@"
	do
		RLIST="$I $RLIST"
		shift
	done

	echo $RLIST
)}


Member()
{(
	ITEM=$1;shift
	for K
	{	
		[ "$K" = "$ITEM" ] && return 0
	}
	return 1
)}

