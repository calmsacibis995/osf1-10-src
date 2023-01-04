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
#	@(#)lint.sh	3.1	(ULTRIX/OSF)	2/26/91
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#
# COMPONENT_NAME: (CMDPROG) Programming Utilities
#
# FUNCTIONS:
#
# ORIGINS: 00 03 10 27 32
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# Lint shell script.  Interacts with options from cc(1).
#

LLDIR=/usr/ccs/lib	# location of lint executables
#Get lint and cpp binaries from $COMP_HOST_ROOT if it is defined
if [ "$COMP_HOST_ROOT" ] 
then
	COMP_HOST_ROOT=`echo $COMP_HOST_ROOT | sed 'sX/$XX`
	LCPP=$COMP_HOST_ROOT/usr/ccs/lib/cpp
	LINT1=$COMP_HOST_ROOT/usr/ccs/lib/lint1
	LINT2=$COMP_HOST_ROOT/usr/ccs/lib/lint2
else
	LINT1=$LLDIR/lint1
	LINT2=$LLDIR/lint2
	LCPP=/lib/cpp
fi

TMP=/usr/tmp/tlint.$$		# preprocessor scratch file
TOUT=				# combined input for second pass
LIBA=$LLDIR/llib-lansi.ln	# standard ANSI library
LIBE=$LLDIR/llib-lc.ln		# default EXTD library
POPT="-E -C -Dlint -DLINT"	# options for the preprocessor
PONLY="-D__osf__ -D__alpha -DLANGUAGE_C -Dunix -D__LANGUAGE_C__" #preprocessor only options
ALPHAOPT=			# options for the migration 
ALPHAPRS=0			# Parse control flag for migration switches
L1OPT=				# options for the lint pass1 only
L2OPT=				# options for lint pass2 only
CFILES=				# the *.c files in order
LFILES=				# the *.ln files in order
LLIB=				# lint library file to create
WARNS="-wA"			# lint warning level
CONLY=				# set for ``compile only''
MANSI=				# set if ANSI mode
LOOK=				# set for echo commands only
RC=0				# Value of 2 to signal failure of lint1
#set -x
trap "rm -f $TMP $TOUT; exit 2" 1 2 3 15

#
# Process each of the arguments, building lists of options.
#
for OPT in "$@"
do
	case "$OPT" in
	*.c)	CFILES="$CFILES $OPT";
		TOUT=/usr/tmp/olint.$$;;	# combined input for second pass
	*.i)	CFILES="$CFILES $OPT";
		TOUT=/usr/tmp/olint.$$;;	# combined input for second pass
	*.ln)	LFILES="$LFILES $OPT";;
	-*)	OPT=`echo $OPT | sed s/-//`
		while [ "$OPT" ]
		do
			O=`echo $OPT | sed 's/\\(.\\).*/\\1/'`	# option letter
			OPT=`echo $OPT | sed s/.//`	# option argument
			case $O in
			\#)	LOOK=1;;		# echo commands only
			c)	CONLY=1;;	# lint1 only, make .ln files
			p)	L2OPT="$L2OPT -$O";
				LIBE="$LLDIR/llib-port.ln";;	# extreme portability
			n)	LIBA=				# no libraries
				LIBE=;;
			v)	L1OPT="$L1OPT -v";;	# parameter usage check
			a)	WARNS="${WARNS}l";;     # warning message options

			w)	WARNS="$WARNS$OPT"    # warning message options
				break;;
			b)	WARNS="${WARNS}R";;    # warning message options
			u)	WARNS="${WARNS}U";;    # warning message options
			h)	WARNS="$WARNS$O";;    # warning message options
			x)	WARNS="${WARNS}D";;    # warning message options
			Q)	ALPHAOPT="-Q$OPT"   # Alpha migration switch
				break;;

			[N])	L1OPT="$L1OPT -$O$OPT"	# valid cc(1) options
				break;;
			[X])	L1OPT="$L1OPT -$O$OPT"	# X<args> in L1
				if [ "$OPT" = "dollar" ]
				then
					L2OPT="$L2OPT -$O$OPT"	# X only in L2
				else
					L2OPT="$L2OPT -$O"	# X only in L2
				fi
				break;;
			M)	L1OPT="$L1OPT -$O$OPT"	# valid cc(1) options
				L2OPT="$L2OPT -$O$OPT"
				MANSI=1
				break;;
			l)	if [ "$OPT" ]	# include a lint library
				then
				    if [ "$OPT" = "curses" ]
				    then
					LFILES="$LLDIR/llib-lcrses.ln $LFILES"
				    else
					LFILES="$LLDIR/llib-l$OPT.ln $LFILES"
				    fi
				else
					dspmsg lint.cat 1 "improper usage of option: %s\n" $O
				fi
				break;;
			o)	if [ "$OPT" ]		# make a lint library
				then
					OPT=`basename $OPT`
					LLIB="llib-l$OPT.ln"
				else
					dspmsg lint.cat 1 "improper usage of option: %s\n" $O
				fi
				break;;
			[DU])	if [ "$OPT" ]		# preprocessor options
				then
					POPT="$POPT -$O$OPT"
				else
					dspmsg lint.cat 1 "improper usage of option: %s\n" $O
				fi
				break;;

			[I])	POPT="$POPT -$O$OPT"
				break;;

			[P])	POPT="$POPT -$OPT"
				break;;

			*)	dspmsg lint.cat 2 "lint: bad option ignored: %s\n" $O;;
			esac
		done;;
	*)	if [ "$OPT" ]
		then
			dspmsg lint.cat 3 "lint: file with unknown suffix ignored: %s\n" $OPT
		fi
	esac
done
#
# Make sure we have valid cfiles and lfiles
# 
if [ "$CFILES" = "" ] && [ "$LFILES" = "" ]
then
	dspmsg lint.cat 4 "usage: lint [options] file ...\n"
	exit 2	
fi

#
# Check for full ANSI library.
#
if [ "$MANSI" ]
then
	LFILES="$LIBA $LFILES"		# standard ANSI library
else
	LFILES="$LFILES $LIBE"		# standard EXTD library
fi

#
# Run the file through lint1 (lint2).
#
if [ "$CONLY" ]		# run lint1 on *.c's only producing *.ln's
then
	for i in $CFILES
	do
		case "$i" in
			*.i) IFILE=1		# TRUE, skip CFE
			     T=`basename $i .i`.ln;;
			*.c) IFILE=		# FALSE, preprocess
			     T=`basename $i .c`.ln;;

		esac
		rm -f $TMP $T
		if [ "$LOOK" ]
		then	
			if [ "$IFILE" ]
			then
				echo "$LINT1 $WARNS $ALPHAOPT $L1OPT $i -L$T ) 2>&1"
			else
				echo "( $LCPP $POPT $PONLY $i > $TMP"
				echo "$LINT1 $WARNS $ALPHAOPT $L1OPT $TMP -L$T ) 2>&1"
			fi
		else
			if [ "$IFILE" ]
			then
				$LINT1 $WARNS $ALPHAOPT $L1OPT $i -L$T 2>&1
			else
				( $LCPP $POPT  $PONLY $i > $TMP
				$LINT1 $WARNS $ALPHAOPT $L1OPT $TMP -L$T ) 2>&1
			fi
		fi
	done
else			# send all *.c's through lint1 run all through lint2
	if [ "$CFILES" ]
	then
		rm -f $TOUT; touch $TOUT
	fi
	for i in $CFILES
	do
		if [ $RC -eq 2 ]
		then
			rm -f $TMP $TOUT;
			exit $RC;
		fi
		rm -f $TMP

		case "$i" in

			*.i) IFILE=1;;		# TRUE, skip CFE
			*)   IFILE=;;		# FALSE, preprocess
		esac
		if [ "$LOOK" ]
		then	
			if [ "$IFILE" ]
			then
				echo "$LINT1 $WARNS $ALPHAOPT $L1OPT $i -L$TOUT ) 2>&1"
			else
				echo "( $LCPP $POPT  $PONLY $i > $TMP"
				echo "$LINT1 $WARNS $ALPHAOPT $L1OPT $TMP -L$TOUT ) 2>&1"
			fi
		else
			if [ "$IFILE" ]
			then
				$LINT1 $WARNS $L1OPT $ALPHAOPT $i -L$TOUT 2>&1
			else
				( $LCPP $POPT  $PONLY $i > $TMP
				$LINT1 $WARNS $L1OPT $ALPHAOPT $TMP -L$TOUT ) 2>&1
			fi
			RC=$?
		fi
	done

	if [ ! $RC -eq 2 ]	# 2 means lint1 failed without recovering
	then
		if [ "$LOOK" ]
		then	
			echo "$LINT2 $WARNS $ALPHAOPT $L2OPT $TOUT $LFILES"
		else
			$LINT2 $WARNS $ALPHAOPT $L2OPT $TOUT $LFILES
		fi

		if [ "$LLIB" ]	# make a library of lint1 results
		then
			mv $TOUT $LLIB
		fi
	else
		rm -f $TMP $TOUT;
		exit $RC;
	fi
fi
rm -f $TMP $TOUT
