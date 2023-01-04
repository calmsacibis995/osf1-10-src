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
# syminst - install with a symbolic link back to the build tree
#

# set DOITPROG to echo to test this script

doit="${DOITPROG-}"


# put in absolute paths if you don't have them in your path; or use env. vars.

lnprog="${LNPROG-ln -s}"
rmprog="${RMPROG-rm}"

instcmd="$lnprog"
rmcmd="$rmprog -f"
srcdir=`pwd`/
src=""
dst=""

while [ x"$1" != x ]; do
    case $1 in
	-c) shift
	    continue;;

	-m) shift
	    shift
	    continue;;

	-o) shift
	    shift
	    continue;;

	-g) shift
	    shift
	    continue;;

	-s) shift
	    continue;;

	-DIR) srcdir=`echo $2 | sed 's;/\./;/;g'`/
	      shift
              shift
              continue;;

	*)  if [ x"$src" = x ]
	    then
		src=$1
	    else
		dst=$1
	    fi
	    shift
	    continue;;
    esac
done

if [ x"$src" = x ]
then
	echo "syminst:  no input file specified"
	exit 1
fi

if [ x"$dst" = x ]
then
	echo "syminst:  no destination specified"
	exit 1
fi


# if destination is a directory, append the input filename; if your system
# does not like double slashes in filenames, you may need to add some logic

if [ -d $dst ]
then
	dst="$dst"/`basename $src`
fi

case $src in
    /*) srcdir=""
	instcmd=cp;;
esac

# get rid of the old one and mode the new one in

$doit $rmcmd $dst
$doit $instcmd $srcdir$src $dst

exit 0
