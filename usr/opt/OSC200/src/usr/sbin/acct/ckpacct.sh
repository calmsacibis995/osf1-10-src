#! /bin/sh
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
#	@(#)ckpacct.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# 

# 
# 	 COMPONENT_NAME: (CMDACCT) Command Accounting
# 
# 	 FUNCTIONS: none
# 
# 	 ORIGINS: 3, 9, 27
# 
# 	 (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# 	 All Rights Reserved
# 	 Licensed Materials - Property of IBM
# 
# 	 US Government Users Restricted Rights - Use, duplication or
# 	 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 	ckpacct.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:54:26
#

#
# 	      "periodically check the size of /var/adm/pacct"
# 	      "if over $1 blocks (500) default, execute turnacct switch"
# 	      "should be run as adm"
# 

# MAILCOM = "mail root adm" to send mail to root adm
MAILCOM=${MAILCOM:-':'}
_adm=${ACCTDIR:-/var/adm}

# (001) added /usr/sbin to path to find rcmgr
PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin:/usr/sbin
export PATH
trap "rm -f ${_adm}/cklock; exit 0" 0 1 2 3 9 15

_max=${1-500}
_MIN_BLKS=500
cd ${_adm}

# (001) added next 2 lines to test if accounting has been set to run.  -'zanne
ACCOUNTING=`rcmgr get ACCOUNTING`
if [ "$ACCOUNTING" != "YES" ]; then exit 1; fi

#	set up lock file to prevent simultaneous checking

if [ -f cklock ] ; then exit 1; fi
cp /dev/null cklock
chmod 400 cklock

#       If there are less than $_MIN_BLKS free blocks left on the /var/adm
#	file system, turn off the accounting (unless things improve
#	the accounting wouldn't run anyway).  If something has
#	returned the file system space, restart accounting.  This
#	feature relies on the fact that ckpacct is kicked off by the
#	cron at least once per hour.

_blocks=`df ${_adm} | awk '/\/dev\// { print $4 }' `

if [ "$_blocks" -lt $_MIN_BLKS   -a  -f /tmp/acctoff ];then
	if ( dspmsg acct.cat 3 "ckpacct: %s still low on space (%s blks); acctg still off\n" ${_adm} $_blocks >&2 )
	then
	( dspmsg acct.cat 3 "ckpacct: %s still low on space (%s blks); acctg still off\n" ${_adm} $_blocks ) | $MAILCOM
	else
	echo "ckpacct: ${_adm} still low on space ($_blocks blks); \c" >&2
	echo "acctg still off" >&2
	( echo "ckpacct: ${_adm} still low on space ($_blocks blks); \c"
	echo "acctg still off" ) | $MAILCOM
	fi
	exit 1
elif [ "$_blocks" -lt $_MIN_BLKS ];then
	if ( dspmsg acct.cat 4 "ckpacct: %s too low on space (%s blks);  turning acctg off\n" ${_adm} $_blocks >&2 )
	then
	( dspmsg acct.cat 4 "ckpacct: %s too low on space (%s blks);  turning acctg off\n" ${_adm} $_blocks ) | $MAILCOM
	else
	echo "ckpacct: ${_adm} too low on space ($_blocks blks); \c" >&2
	echo "turning acctg off" >&2
	( echo "ckpacct: ${_adm} too low on space ($_blocks blks); \c"
	echo "turning acctg off" ) | $MAILCOM
	fi
	nulladm /tmp/acctoff
	turnacct off
	exit 1
elif [ -f /tmp/acctoff ];then
	if (dspmsg acct.cat 8 "ckpacct: %s free space restored; turning acctg on\n" ${_adm} ) | $MAILCOM
	then :
	else
	echo "ckpacct: ${_adm} free space restored; turning acctg on" | \
		$MAILCOM
	fi
	rm /tmp/acctoff
	turnacct on
fi

_cursize="`du -s pacct | sed 's/	.*//'`"
if [ "${_max}" -lt "${_cursize}" ]; then
	turnacct switch
fi
