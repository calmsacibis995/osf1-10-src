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
# @(#)$RCSfile: Client.sh,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/16 16:00:39 $
# 

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}

. $SHELL_LIB/Dialog
. $SHELL_LIB/Error
. $SHELL_LIB/Strings

#
# Given: $1 is an ethernet or FDDI address to be validated syntactically
# $1 must be in the form of x-x-x-x-x-x or x:x:x:x:x:x, where x is a one or
# two digit hexidecimal number to be syntactically correct.
Ckether()
{
	ans=$1
	set -- `Parse ":-" $ans`
	ETHER=
	SEP=
	for i in $*
	do
		case $i in
		[0-9A-Fa-f] )
			i="0$i"
			;;
		[0-9A-Fa-f][0-9A-Fa-f] )
			;;
		* )
			Error "$ans is an invalid Ethernet or FDDI address."
			return 1
			;;
		esac
		ETHER="$ETHER$SEP$i"
		SEP=-
	done
	case $ETHER in
	[0-9A-Fa-f][0-9A-Fa-f]-[0-9A-Fa-f][0-9A-Fa-f]-[0-9A-Fa-f][0-9A-Fa-f]\
-[0-9A-Fa-f][0-9A-Fa-f]-[0-9A-Fa-f][0-9A-Fa-f]-[0-9A-Fa-f][0-9A-Fa-f])
		echo "$ETHER"
		return 0
		;;
	* )
		Error "$ans is an invalid Ethernet or FDDI address."
		return 1
		;;
	esac
}

: Getether
# Prompt for and validate an ethernet or FDDI address
#
Getether()
{
	CLINET=$1
	while :
	do
		Dialog "
Enter the client processor's hardware Ethernet or FDDI address.  For
example, 08-00-2b-02-67-e1" ans $CLINET
		CLINET=`Ckether $ans` && break
	done
}


: Ckname
# Test for valid hostname syntax and arp for CLIENT.  If both
# tests don't pass, invalid name.
Ckname()
{
	CLIENT=$1
	case $CLIENT in
	[A-Za-z]* )
		ARPRET=`arp "$CLIENT" 2>/dev/null`
		case $ARPRET in
		"" )
			Error "arp failed on hostname \"$CLIENT\""
			return 1
		esac
		return 0
		;;
	* ) 	Error "\"$CLIENT\" is an invalid hostname."
	esac
	return 1
}

: Getname
#
# Prompts for a valid CLIENT name.
# Inputs: If $1 is "existing", $2 is the path name of a database to find
# CLIENT in.  If CLIENT is not in $2, CLIENT is invalid.  If $1 is "new",
# Getname calls Ckname to validate the name.
Getname()
{
	while :
	do
		Dialog "
Enter the client processor's hostname" CLIENT
		case $1 in
		existing )
			ENT=`grep "^$CLIENT:" $2 2>/dev/null` && break
			Error "\
There is no entry for \"$CLIENT\" in the `basename $2` file."
			;;
		new )
			Ckname $CLIENT && break ;;
		esac
	done
}

