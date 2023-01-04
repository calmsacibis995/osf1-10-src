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
#	@(#)monacct.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# 	monacct.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:56:39
#
# 	"this procedure should be run periodically ( by month or fiscal )"
# 

_adm=${ACCTDIR:-/var/adm}
_sum=${_adm}/acct/sum
_fiscal=${_adm}/acct/fiscal
PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH


#if test $# -ne 1; then
#	echo "usage: monacct fiscal-number"
#	exit 1
#fi

_period=${1-`date +%m`}

cd ${_adm}

#	"move summary tacct file to fiscal directory"
if [ -f ${_sum}/tacct ]
then
	mv ${_sum}/tacct ${_fiscal}/tacct${_period}
fi

#	"delete the daily tacct files"
rm -f ${_sum}/tacct????

#	"restart summary tacct file"
nulladm ${_sum}/tacct

#	"move summary cms file to fiscal directory
if [ -f ${_sum}/cms ]
then
	mv ${_sum}/cms ${_fiscal}/cms${_period}
fi

#	"restart summary cms file"
nulladm ${_sum}/cms

#	"remove old prdaily reports"
rm -f ${_sum}/rprt*

#	"produce monthly reports"
prtacct ${_fiscal}/tacct${_period} > ${_fiscal}/fiscrpt${_period}

if _hperiod="`dspmsg acct.cat 66 'TOTAL COMMAND SUMMARY FOR FISCAL'`"
then :
else
	_hperiod="TOTAL COMMAND SUMMARY FOR FISCAL"
fi

if _hlogin="`dspmsg acct.cat 67 'LAST LOGIN'`"
then :
else
	_hlogin="LAST LOGIN"
fi

acctcms -a -s ${_fiscal}/cms${_period} |  \
pr -h "${_hperiod} ${_period}" >> ${_fiscal}/fiscrpt${_period}
pr -h "${_hlogin}" -3 ${_sum}/loginlog >> ${_fiscal}/fiscrpt${_period}

#	"add commands here to do any charging of fees, etc"
exit 0
