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
# This accepts bsd-style install arguments and makes the appropriate calls
# to the System V install.
#

flags=""
dst=""
src=""
dostrip=""
owner=""
mode=""

while [ x$1 != x ]; do
    case $1 in 
	-c) shift
	    continue;;

	-m) flags="$flags $1 $2 "
	    mode="$2"
	    shift
	    shift
	    continue;;

	-o) flags="$flags -u $2 "
	    owner="$2"
	    shift
	    shift
	    continue;;

	-g) flags="$flags $1 $2 "
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

case "$mode" in
"")
	;;
*)
	case "$owner" in
	"")
		flags="$flags -u root"
		;;
	esac
	;;
esac

if [ x$src = x ] 
then
	echo "bsdinst:  no input file specified"
	exit 1
fi

if [ x$dst = x ] 
then
	echo "bsdinst:  no destination specified"
	exit 1
fi


# set up some variable to be used later

rmcmd=""
srcdir="."

# if the destination isn't a directory we'll need to copy it first

if [ ! -d $dst ]
then
	dstbase=`basename $dst`
	cp $src /tmp/$dstbase
	rmcmd="rm -f /tmp/$dstbase"
	src=$dstbase
	srcdir=/tmp
	dst="`echo $dst | sed 's,^\(.*\)/.*$,\1,'`"
	if [ x$dst = x ]
	then
		dst="."
	fi
fi


# If the src file has a directory, copy it to /tmp to make install happy

srcbase=`basename $src`

if [ "$src" != "$srcbase" -a "$src" != "./$srcbase" ] 
then
	cp $src /tmp/$srcbase
	src=$srcbase
	srcdir=/tmp
	rmcmd="rm -f /tmp/$srcbase"
fi

# do the actual install

if [ -f /usr/sbin/install ]
then
	installcmd=/usr/sbin/install
elif [ -f /etc/install ]
then
	installcmd=/etc/install
else
	installcmd=install
fi

# This rm is commented out because some people want to be able to
# install through symbolic links.  Uncomment it if it offends you.
# rm -f $dst/$srcbase
(cd $srcdir ; $installcmd -f $dst $flags $src)

if [ x$dostrip = xstrip ]
then
	strip $dst/$srcbase
fi

# and clean up

$rmcmd

