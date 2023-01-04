#!/usr/bin/sh
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
# @(#)$RCSfile: sccsdiff.sh,v $ $Revision: 4.1.3.2 $ (DEC) $Date: 1992/12/11 14:26:36 $
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# 
# 
# OSF/1 Release 1.0

#

#
# COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
#
# FUNCTIONS: N/A
#
# ORIGINS: 3, 10, 27
#
# This module contains IBM CONFIDENTIAL code. -- (IBM
# Confidential Restricted when combined with the aggregated
# modules for this product)
# OBJECT CODE ONLY SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# sccsdiff.sh    1.5 1/4/90 18:11:10
#

sid1= sid2= num= pipe= files=

for i in $@
do
	case $i in

	-*)
		case $i in

		-r*)
			if [ ! "$sid1" ]
			then
				sid1=$i
			elif [ ! "$sid2" ]
			then
				sid2=$i
			fi
			;;
		-s*)
			num=`expr "$i" : '-s\(.*\)'`
			;;
		-p*)
			pipe=yes
			;;
		*)
			files=
			break
			;;
		esac
		;;
	*s.*)
		files="$files $i"
		;;
	*)
		dspmsg sccs.cat 1 '%1$s: %2$s is not an SCCS file.\n\tSpecify an SCCS file.\n' $0 $i 1>&2
#		echo "$0: $i not an SCCS file" 1>&2
		;;
	esac
done

if [ -z "$files" ]
then
	dspmsg sccs.cat 2 'usage: %s -rSID1 -rSID2 \[-p\] \[-sNumber\] SCCSFile...\n\tCompares two versions of a Source Code Control System (SCCS) file.\n' $0 1>&2
#	echo "usage: $0 -r<sid1> -r<sid2> [-p] [-s<num-arg>] sccsfile ..." 1>&2
	exit 1
fi

trap "rm -f /tmp/get[abc]$$;exit 1" 0 1 2 3 15

for i in $files
do
	if get -s -p -k $sid1 $i > /tmp/geta$$
	then
		if get -s -p -k $sid2 $i > /tmp/getb$$
		then
			bdiff /tmp/geta$$ /tmp/getb$$ $num > /tmp/getc$$
		fi
	fi
	if [ ! -s /tmp/getc$$ ]
	then
		if [ -f /tmp/getc$$ ]
		then
#                  echo '%s: No differences.\n' $i >/tmp/getc$$
# 001 - Changed previous line to:
		  dspmsg sccs.cat 3 '%s: No differences.\n' $i >/tmp/getc$$
		else
		exit 1
		fi
	fi
	if [ "$pipe" ]
	then
#		msg=`'%1$s: %2$s versus %3$s\n' $i $sid1 $sid2`
# 001 - Changed previous line to:
		msg=`dspmsg sccs.cat 4 '%1$s: %2$s versus %3$s\n' $i $sid1 $sid2`
		pr -h "$msg" /tmp/getc$$
	else
		cat /tmp/getc$$
	fi
done

trap 0
rm -f /tmp/get[abc]$$
