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
#	@(#)prtacct.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# 	prtacct.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:58:03
#

#	"print daily/summary total accounting (any file in tacct.h format)"
#       "prtacct [-f FIELDS ] [-v] file [heading]"
#
#       "FIELDS is in the form of a field specification for acctmerg (1)"

FIELDS="1-2,5-6,11-15"
PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH
VFLAG=
while [ -n "$1" ]
do
	case "$1" in
	-f)	FIELDS="$2"; shift; shift ;;
	-v)     VFLAG="$1";  shift ;;
	*)      break ;;
	esac
done
if [ ! -r "$1" ]
then
	if ( dspmsg acct.cat 22 "Usage: prtacct [-f Specification ] [-v] File ['Heading']\n" >&2 )
	then :
	else echo "Usage: prtacct [-f Specification ] [-v] File ['Heading']\n" >&2
	fi
	exit 1
fi
# Take all arguments after the file as part of the heading.
# Unfortunately, getopt does not quote its arguments so "heading words"
# appears as two arguments.
(acctmerg -t $VFLAG -h"$FIELDS" <"$1"; acctmerg -p"$FIELDS" $VFLAG <"$1") |\
pr -h "$2"
exit 0
