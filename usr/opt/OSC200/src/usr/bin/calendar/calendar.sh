#!/usr/bin/sh -
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
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: calendar.sh,v $ $Revision: 4.3.10.3 $ (DEC) $Date: 1993/09/03 03:37:22 $
#
#
# HISTORY
#
# OSF/1 1.2
#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: calendar
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# calendar.sh	1.7  com/cmd/misc/calendar,3.1,9021 4/27/90 16:19:35
#

#	calendar.sh - calendar command, uses calprog (in /usr/lbin)

#  NAME: calendar [-]
#                                                                     
#  FUNCTION: Writes reminder messages to stdout.
# 	OPTIONS:
#	-	Calls calendar for everyone having calendar in their home 
#		directory and sends any reminders by mail.

PATH=/usr/bin:/usr/lbin:/usr/bin:/usr/lbin
_tmp=/tmp/cal$$
_mailmsg=/tmp/calmail2$$
_subj=/tmp/calmail1$$
trap "rm -f ${_tmp} ${_mailmsg} ${_subj}; exit 1" 1 2 13 15
calprog > ${_tmp}
case $# in

0)	if [ -d calendar ]; then
		dspmsg calendar.cat 3 "%s: %s/calendar is a directory rather than a file\n" $0 `pwd`
		rm -f ${_tmp}
		exit 1
	fi
	if [ -s calendar ]; then
		grep -E -f ${_tmp} calendar
	else
		dspmsg calendar.cat 1 "%s: %s/calendar not found\n" $0 \
			`pwd` 1>&2
		rm -f ${_tmp}
		exit 1
	fi;;
*)
	if [ $# != 1 -o $1 != "-" ]
	then
		dspmsg calendar.cat 4 "Usage: calendar [-]\n" 1>&2
		rm -f ${_tmp}
		exit 1
	fi
	cat /etc/passwd | \
		sed 's/\([^:]*\):.*:\(.*\):[^:]*$/_dir=\2 _user=\1/' | \
		while read _token; do
			eval ${_token}	# evaluates _dir= and _user=
			if [ -s ${_dir}/calendar ]; then
				grep -E -f ${_tmp} ${_dir}/calendar \
				 2>/dev/null > ${_mailmsg}
				if [ -s ${_mailmsg} ]; then
					_date=`date '+%D'`
					dspmsg calendar.cat 2 \
					 "Subject: Calendar for %s\n" ${_date}\
					 > ${_subj}
					cat ${_subj} ${_mailmsg} | mail ${_user}
				fi
			fi
		done;;
esac
rm -f ${_tmp} ${_mailmsg} ${_subj}
exit 0
