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
#	@(#)turnacct.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# COMPONENT_NAME: (CMDACCT) Command Accounting
#
# FUNCTIONS: none
#
# ORIGINS: 3, 9, 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# turnacct.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 11:00:59
#

#	"control process accounting (must be root)"
#	"turnacct on	makes sure it's on"
#	"turnacct off	turns it off"
#	"turnacct switch	switches pacct to pacct?, starts fresh one"
#	"/var/adm/pacct is always the current pacct file"
#

PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH
cd ${ACCTDIR:-/var/adm}
case "$1"  in
on)
	if test ! -r pacct
	then
		nulladm pacct
	fi
	accton pacct
	rc=$?
	;;
off)
	accton
	rc=$?
	;;
switch)
	if test -r pacct
	then
		_i=1
		while test -r pacct${_i}
		do
			_i="`expr ${_i} + 1`"
		done
		mv pacct pacct${_i}
	fi
	nulladm pacct
	accton
	accton pacct
	_rc=$?
	if test ${_rc} -ne 0; then
		echo "accton failed" >&2
		rm pacct
		mv pacct${_i} pacct
		exit ${_rc}
	fi
	;;
*)
	if ( dspmsg acct.cat 64 "Usage: turnacct on|off|switch\n" >&2 ); then :
	else echo "Usage: turnacct on|off|switch" >&2
	fi
	_rc=1
	;;
esac
exit ${_rc}
