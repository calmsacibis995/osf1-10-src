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

case "$1" in
"")	echo "Usage: $0 directory"; exit 1;;
esac

echo "Analyzing $1 for Incompatabilities with System V"

echo 'File names longer than 12 characters (excluding the doc directory):'
cd $1

dirlist=
for dir in `echo *`
do
	case "$dir" in
	doc)	;;
	*)	dirlist="$dirlist $dir";;
	esac
done

(
	find doc      -name '???????????????*' -print
	find $dirlist -name '?????????????*' -print
) | sort \
  | sed -e '/,v/d' \
		-e 's/^/	/'

echo 'Symbolic links:'
find . -type l -print | sed -e 's/^/	/'

