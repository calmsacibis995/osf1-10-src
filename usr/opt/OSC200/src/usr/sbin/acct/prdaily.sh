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
#	@(#)prdaily.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# 	prdaily.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:57:45
#

#	"prdaily	prints daily report"
#	"last command executed in runacct"
#	"if given a date mmdd, will print that report"
#

PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH

set -- `getopt cl $*`
if [ $? != 0 ]
then
	if ( dspmsg acct.cat 15  "Usage: prdaily [[-l] [mmdd]] | [-c]\n" >&2 )
	then :
	else echo  "Usage: prdaily [[-l] [mmdd]] | [-c]" >&2
	fi
	exit 2
fi
for i in $*
do
	case $i in
	-c)	CMDEXCPT=1; shift;;
	-l)	LINEEXCPT=1; shift;;
	--)	shift; break;;
	esac
done

if [ "$CMDEXCPT" = "1" -a "$LINEEXCPT" = "1" ]
then
	if  ( dspmsg acct.cat 15  "Usage: prdaily [[-l] [mmdd]] | [-c]\n" >&2 )
	then :
	else echo  "Usage: prdaily [[-l] [mmdd]] | [-c]" >&2
	fi
	exit 2
fi

date=`date +%m%d`
_sysname="`hostname`"
_sbin=/usr/sbin/acct
_adm=${ACCTDIR:-/var/adm}
_nite=${_adm}/acct/nite
_sum=${_adm}/acct/sum

cd ${_nite}
if [ `expr "$1" : [01][0-9][0-3][0-9]` -eq 4 -a "$1" != "$date" ]; then
	if [ "$CMDEXCPT" = "1" ]
	then
		date1=`date '+%h %d'`
		if ( dspmsg acct.cat 16  "Cannot print command exception reports except for %s\n" $date1 >&2 )
		then :
		else echo "Cannot print command exception reports except for $date1" >&2
		fi
		exit 5
	fi
	if [ "$LINEEXCPT" = "1" ]
	then
		acctmerg -a < ${_sum}/tacct$1 | awk -f ${_sbin}/ptelus.awk
		exit $?
	fi
	cat ${_sum}/rprt$1
	exit 0
fi

if [ "$CMDEXCPT" = 1 ]
then
	acctcms -a -s ${_sum}/daycms | awk -f ${_sbin}/ptecms.awk
fi
if [ "$LINEEXCPT" = 1 ]
then
	acctmerg -a < ${_sum}/tacct${date} | awk -f ${_sbin}/ptelus.awk
fi
if [ "$CMDEXCPT" = 1 -o "$LINEEXCPT" = 1 ]
then
	exit 0
fi
if HDR=`dspmsg acct.cat 17 'DAILY REPORT FOR %s\n' ${_sysname}`; then :
else HDR="DAILY REPORT FOR ${_sysname}\n"; fi
if HDR1=`dspmsg acct.cat 18 'DAILY USAGE REPORT FOR %s\n' ${_sysname}`; then :
else HDR1="DAILY USAGE REPORT FOR ${_sysname}\n"; fi
if HDR2=`dspmsg acct.cat 19 'DAILY COMMAND SUMMARY\n'`; then :
else HDR2="DAILY COMMAND SUMMARY\n"; fi
if HDR3=`dspmsg acct.cat 20 'MONTHLY TOTAL COMMAND SUMMARY\n'`; then :
else HDR3="MONTHLY TOTAL COMMAND SUMMARY\n"; fi
if HDR4=`dspmsg acct.cat 21 'LAST LOGIN\n'`; then :
else HDR4="LAST LOGIN\n"; fi

(cat reboots; echo ""; cat lineuse) | pr -h "$HDR"

prtacct daytacct "$HDR1"
pr -h "$HDR2" daycms
pr -h "$HDR3" cms
pr -h "$HDR4" -3 ../sum/loginlog  
exit 0
