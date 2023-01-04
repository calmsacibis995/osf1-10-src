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
#	@(#)chargefee.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# 	chargefee.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:53:59
#

#
# 	      "chargefee login-name number"
# 	"emits tacct.h/ascii record to charge name $number"
# 

_adm=${ACCTDIR:-/var/adm}
cd ${_adm}
PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH

if test $# -lt 2; then
	if dspmsg acct.cat 65 "Usage: chargefee User Number\n" >&2
	then :
	else echo "Usage: chargefee User Number" >&2
	fi
	exit 1
fi

_entry="`printpw -u  | grep \^$1`"
if test -z "${_entry}"; then
	if dspmsg acct.cat 1 "can't find login name " >&2
	then :
	else echo "can't find login name " >&2
	fi
	echo "$1" >&2
	exit 1
fi

amt="`expr "$2".00 : '\(-\{0,1\}[0-9]*\.[0-9][0-9]*\)\.\{0,1\}[0-9]*$'`"
if test -z "$amt" ; then
	if dspmsg acct.cat 2 "charge invalid: " >&2
	then :
	else echo "charge invalid: " >&2
	fi
	echo "$2" >&2
	exit 1
fi

if test ! -r fee; then
	nulladm fee
fi

_userid=`echo "${_entry}" | cut -d: -f2`  # get the UID
echo  "${_userid} $1 0 0 0 0 0 0 0 0 0 0 0 0 $2 0 0 0" >>fee
exit 0
