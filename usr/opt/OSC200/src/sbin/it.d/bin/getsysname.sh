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
# @(#)$RCSfile: getsysname.sh,v $ $Revision: 4.2.3.5 $ (DEC) $Date: 1992/11/06 17:02:06 $ 
#
##############################################
#
# getsysname - system name program
#
# History
# %W%	(DEC OSF/1)	%G%
#
##############################################

[ "$ISL_ADVFLAG" ] ||
{
	case `whoami` in
	root ) ;;
	* )
		echo "
You must have root priviledges to change the name of your system."
		exit 1
		;;
	esac
}

RCMGR=/usr/sbin/rcmgr
HOSTNAME=`$RCMGR get HOSTNAME`

if [ "$HOSTNAME" ]
then
	echo "\nThe system name assigned to your machine is '${HOSTNAME}'".
else
	echo "\n*** SYSTEM NAME SPECIFICATION *** "
	while : true
	do
		echo "
Select the name of your system using alphanumeric characters. 
The first character must be a letter.   For example, tinker. 
Enter your system name:  \c"

		read HOSTNAME
		HOSTNAME=`echo $HOSTNAME`
		check=`expr $HOSTNAME : '\([a-zA-Z][a-zA-Z0-9.]*\)' \
			2> /dev/null`
		case $HOSTNAME in
		"" )
			continue
			;;
		$check  )
			;;
		* )
			echo "
You specified '${HOSTNAME}' which is an invalid system name.\n"
			continue
			;;
		esac

		echo "
You selected '${HOSTNAME}' as the name of your system.
Is this correct? (y/n) [y]: \c"
		read resp
		resp=`echo $resp`
		case $resp in
		y* | Y* | "")
			break
			;;
		*)
		;;
		esac
	done

	$RCMGR set HOSTNAME $HOSTNAME
fi

/bin/hostname $HOSTNAME > /dev/null

exit 0
