#!/sbin/sh
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
# @(#)$RCSfile: genufi.sh,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/06/15 17:17:08 $ 
# 
#	genufi -
#		Takes the name of an inventory file as a command line
#		argument and generates a list of files resident on the
#		system which are not listed in this inventory.
#
#	000	Designed by ccb
#		Coded by jon		15-Aug-1990
#
#	001	16-oct-1990	ccb
#		bugfixes, integrate fsmount
#
#	002	15-may-1991	ech
#		ported to OSF/1
#
#	003	09-oct-1991	ech
#		refine Filter routine, use find -mount

CDPATH=
PATH=/usr/lbin:/usr/bin:/sbin:/usr/sbin:/usr/ucb
export CDPATH PATH

SHELL_LIB=${SHELL_LIB:-/usr/share/lib/shell}
. $SHELL_LIB/Strings

Constants()
{
	TDIR=/usr/tmp
	TMP1=$TDIR/ufitmp1
	TMP2=$TDIR/ufitmp2
	TMP3=$TDIR/ufitmp3
	SPLITS=/usr/tmp/ufisplits
	UFI=/usr/.smdb./UFI
}



Filter()
{
	# filter out components which are known to exist outside of
	#  any subset.

	HOST=`hostname`
	set xx `Parse . $HOST`; shift
	SHOST=$1
	KHOST=`Ucase $SHOST`

	# Directories which are to be ignored
	egrep -v '^\./dev/' | 
		egrep -v '^\./usr/sys/'$KHOST/ | 
		egrep -v '^\./tmp/|^\./usr/\.smdb\./|^\..*/var/rwho/' |
		egrep -v '^\..*/var/tmp/|^\..*/var/adm/syslog.dated/' |
		egrep -v '^\..*/var/adm/smlogs/' | 

	# individual files which are to be ignored
		egrep -v '^\./osf_boot$' |
		egrep -v '^\..*/core$|^\..*/.dummy$' |
		egrep -v '^\..*/var/adm/binary\.errlog$' |
		egrep -v '^\..*/var/adm/xdm/xdm-pid$' |

	# fuzzy matches
		egrep -v '^\./.*vmunix|^\..*/vmcore' |
		egrep -v '^\..*/\.new\.\.|^\..*/\.proto\.\.' 
}


Main()
{
	cd /
	INFILE=/usr/.smdb./MSI

	echo "Working.\c"
	
	# Place list of all files in TMP1
	find ./ -mount ! -type d ! -type s -print > $TMP1
	for MOUNT in `fsmount < $INFILE | awk '{print $3}'`
	{
		find .$MOUNT -mount ! -type d ! -type s -print >> $TMP1
	}
	echo ".\c"

	# Filter out unwanted components
	Filter < $TMP1 > $TMP2
	# sort the list
	sort -o $TMP1 $TMP2
	echo ".\c"

	# Change format of INFILE
	cut -f10 $INFILE > $TMP3

	# Get files listed in TMP1 that are not listed in INFILE
	comm -23 $TMP1 $TMP3 > $TMP2

	echo ".\c"

	# Back to inventory format
	ils -f $TMP2 > $UFI

	# Cleanup the tmp files
	rm $TMP1 $TMP2 $TMP3
	echo done.
}


Ucase()
{
	echo $*| dd conv=ucase 2> /dev/null
}


ARGS=$*


Constants
[ "$GENUFI_DEBUG" ] || Main $ARGS

