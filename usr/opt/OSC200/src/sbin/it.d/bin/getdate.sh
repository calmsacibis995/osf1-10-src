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
# @(#)$RCSfile: getdate.sh,v $ $Revision: 4.2.3.7 $ (DEC) $Date: 1993/01/21 16:25:14 $ 
#
############################################
# History
#
# 001	24-may-1991	Jon Wallace
#	First Implementation
#
# 002	24-Jun-1991	Jon Wallace
#	Put in code that asks user to confirm their choice.
#
############################################

NetSet()
{
	2>&1 /usr/sbin/rdate -s >/dev/null &&
	{
		echo "\nThe date and time has been set to `date`"
		return 0
	}
	return 1
}


ManSet()
{
	while : true
	do
 		echo "
The current date and time should be specified using the following
format:

	yymmddhhmm

Use two digits for year (yy), month (mm), day (dd), hour (hh), and
minute (mm).  Enter the time in 24-hour format.  For example, 11:30
p.m. on July 25, 1993 would be entered as:

	9307252330

Enter the date and time: \c"

		read DATE_TIME
		DATE_TIME=`echo $DATE_TIME`
		2>&1 date -n $DATE_TIME >/dev/null 
		case $? in 
		0 )
			echo "\nThe date and time has been set to `date`" 
			echo "Is this correct? (y/n) [y]: \c"
			read ans
			ans=`echo $ans`
			case $ans in
			[yY]* | "" )
				break
				;;
			* )
				;;
			esac
			;;
		* )
			echo "\n*** Invalid date/time entry ***"
			;;
		esac
	done
}


Main()
{
	NetSet ||
	{
		echo "\n*** DATE AND TIME SPECIFICATION ***"
		ManSet
	}
}


[ "$CHECK_SYNTAX" ] || Main
