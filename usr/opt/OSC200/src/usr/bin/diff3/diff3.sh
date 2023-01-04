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
#
# (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# @(#)$RCSfile: diff3.sh,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/10 18:40:53 $
#
# 
# HISTORY
#
# OSF/1 1.2
#
# COMPONENT_NAME: (CMDFILES) commands that manipulate files
#
# FUNCTIONS: diff3
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# diff3.sh	1.10  com/cmd/files/diff,3.1,9013 3/5/90 16:31:39
#                                                                   
#  diff3  [-ex3EX] file1 file2 file3 
#  compares three files
#  this shell script calls /usr/lib/diff3prog to do the work.
e=
case $1 in
-*)
	e=$1
	shift;;
esac
if test $# -ne 3
then
	if [ -x /usr/bin/dspmsg ]; then
	dspmsg -s 1 diff3.cat 2 'usage: diff3 [-e | -x | -E | -X | -3] file1 file2 file3\n' 1>&2
	else	msg='usage: diff3 [-exEX3] file1 file2 file3\n';
	eval echo $msg 1>&2; fi
	exit 1
fi
if [ ! -f $1 -o ! -f $2 -o ! -f $3 ]
then
	if [ -x /usr/bin/dspmsg ]; then
	dspmsg -s 1 diff3.cat 1 'file not found\n' 1>&2
	else	msg='file not found\n';
	eval echo $msg 1>&2; fi
	exit 1
fi
trap "rm -f /tmp/d3[ab]$$; trap '' 0; exit" 1 2 13 15
diff $1 $3 >/tmp/d3a$$
diff $2 $3 >/tmp/d3b$$
/usr/lbin/diff3prog $e /tmp/d3[ab]$$ $1 $2 $3
if [ $? != 0 ]
then rm -f /tmp/d3[ab]$$
     exit 1
else rm -f /tmp/d3[ab]$$
fi
