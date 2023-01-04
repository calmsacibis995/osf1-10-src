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
#	@(#)dodisk.sh	3.1	(ULTRIX/OSF)	2/26/91
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
# FUNCTIONS: none
# 
# ORIGINS: 3, 9, 27
# 
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# 	 US Government Users Restricted Rights - Use, duplication or
# 	 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 	dodisk.sh	1.2  com/cmd/acct,3.1,8943 10/24/89 10:55:51
#
# 	'perform disk accounting'
# 

_adm=${ACCTDIR:-/var/adm}
_nite=${_adm}/acct/nite
PATH=/usr/sbin/acct:/bin:/usr/bin:/sbin
export PATH
set -- `getopt o $*`
if [ $? -ne 0 ]
then
	if ( dspmsg acct.cat 9  "Usage: dodisk [ -o ] [ File ... ]\n" >&2 )
	then :
	else echo "Usage: dodisk [ -o ] [ File ... ]" >&2
	fi
	exit 1
fi
for i in $*; do
	case $i in
	-o)	SLOW=1; shift;;
	--)	shift; break;;
	esac
done

cd ${_adm}
date

if [ "$SLOW" = "" ]
then
	if [ $# -lt 1 ]
	then
		args=`awk '/\/dev\// { print $1 } ' /etc/fstab | sed 's/^\/dev\//\/dev\/r/'`
	else
		args="$*"
	fi
	diskusg $args > dtmp
else
	if [ $# -lt 1 ]
	then
		args="/"
	else
		args="$*"
	fi
	for i in $args; do
		if [ ! -d $i ]
		then
			echo "dodisk: $i \c" >&2
			if ( dspmsg acct.cat 10 "is not a directory -- ignored\n" >&2 )
			then :
			else echo "is not a directory -- ignored" >&2
			fi
		else
			dir="$i $dir"
		fi
	done
	if [ "$dir" = "" ]
	then
		if ( dspmsg acct.cat 11 "dodisk: No data\n" >&2 )
		then :
		else echo "dodisk: No data" >&2
		fi
		cp /dev/null dtmp
	else
		find $dir -xdev -print | acctdusg > dtmp
	fi
fi

date
sort +0n +1 -o dtmp dtmp
acctdisk <dtmp >disktmp
chmod 644 disktmp
chown adm disktmp
rm -f dtmp
mv disktmp ${_nite}/dacct
