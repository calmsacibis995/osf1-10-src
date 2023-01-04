#!/bin/sh
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

if [ x$1 = x ]; then
	echo "Usage: Apply patch_name"
	echo "only do one at a time with this script, paranoid you know."
	exit
fi
echo "The following files need to be patched:"
echo ""
cat $1 | awk '/^--- [a-zA-Z]/{print $2}'
echo ""
echo "They need to be checked out already."
echo "Are you ready (y or n)"
read a
if [ x$a != xy ]; then
	echo "Get ready and then try again."
	exit
fi

cat $1 | /usr/local/bin/patch -d.. -p > .scratch 2>&1 

grep failed .scratch > /dev/null 2>&1
if [ $? = 0 ]; then
	echo "Some failures occurred. They are listed in .scratch."
	exit
fi
grep "can't" .scratch > /dev/null 2>&1
if [ $? = 0 ]; then
	echo "File may not have been check out."
	echo "Some failures occurred. They are listed in .scratch."
	exit
fi
#rm .scratch
echo "No failures occurred."
seq=`cat $1 | awk '/^Subject/{print $3}' | awk 'BEGIN{FS=")"} {print $1}'`
echo "Please modify the headers to denote that sequence $seq was added."
