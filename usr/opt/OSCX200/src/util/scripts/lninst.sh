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

#
# This accepts bsd-style install arguments and simply makes symbolic links.
#

flags=""
dst=""
src=""
dostrip=""

while [ x$1 != x ]; do
    case $1 in 
	-c) shift
	    continue;;

	-[mog]) flags="$flags $1 $2 "
	    shift
	    shift
	    continue;;

	-s) dostrip="strip"
	    shift
	    continue;;

	*)  if [ x$src = x ] 
	    then
		src=$1
	    else
		dst=$1
	    fi
	    shift
	    continue;;
    esac
done

if [ x$src = x ] 
then
	echo "syminst:  no input file specified"
	exit 1
fi

if [ x$dst = x ] 
then
	echo "syminst:  no destination specified"
	exit 1
fi

if [ -d $dst ]; then
    rm -f $dst/`basename $src`
else
    rm -f $dst
fi

ln -s `pwd`/$src $dst
