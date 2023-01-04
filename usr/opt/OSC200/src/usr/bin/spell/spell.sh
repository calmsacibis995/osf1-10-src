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
# @(#)$RCSfile: spell.sh,v $ $Revision: 4.2.12.2 $ (DEC) $Date: 1993/09/30 18:48:21 $
# 
# 
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
# 
# OSF/1 Release 1.0
# 
# COMPONENT_NAME: (CMDTEXT) Text Formatting Services
# 
# FUNCTIONS:
# 
# ORIGINS: 3,10,27
# 
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# spell program
# Variables from environment (else defaulted):
#   D_SPELL dictionary, S_SPELL stop, H_SPELL history, P_SPELL program
# Local variables:
#   B flags
#   F input files
#   V data for -v
#   D -l/-i handling for forming deroff arguments
#   M non-null if multiple -i/-l flags
#   LOCAL file containing locally acceptable words (via + argument)
#   POSTPROC post-processing commands (for LOCAL list and history)
#
#   spell.sh	1.6  3/3/90 14:40:46
# 
H_SPELL=${H_SPELL-/usr/lbin/spell/spellhist}
P_SPELL=${P_SPELL-/usr/lbin/spell/spellprog}
# 
#   Only deroff V5.5 and up know about -u flag.
DEROFF="deroff -u"
V=/dev/null
STATUS=0
F=
B=
D=
M=

next="F="
trap "rm -f /tmp/spell.$$; exit \$STATUS" 0 1 2 13 15
for A in $*
do
	case $A in
	-v)     if [ -x /bin/pdp11 ] && /bin/pdp11
		then	ERRMSG=`dspmsg spell.cat 1 "-v option not supported on pdp11"`
			if [ "$ERRMSG" = "" ]
			then	echo -v option not supported on pdp11 >&2
			else	echo $ERRMSG >&2
			fi
			EXIT_SPELL=exit
			STATUS=1
		else    B="$B -v"
			V=/tmp/spell.$$
		fi ;;
	-a)	: ;;
	-b) 	D_SPELL=${D_SPELL-/usr/lbin/spell/hlistb}
		B="$B -b" ;;
	-x)     B="$B -x" ;;
	+*)	if [ "$FIRSTPLUS" = "+" ]
		then	ERRMSG=`dspmsg spell.cat 2 "multiple + options in spell, all but the last are ignored"`
			if [ "$ERRMSG" = "" ]
			then	echo "multiple + options in spell, all but the last are ignored" >&2
			else	echo $ERRMSG >&2
			fi
		fi;
		FIRSTPLUS="$FIRSTPLUS"+
		if  LOCAL=`expr $A : '+\(.*\)' 2>/dev/null`;
		then if test ! -r $LOCAL;
			then ERRMSG=`dspmsg spell.cat 3 "spell cannot read %s" $LOCAL`
			if [ "$ERRMSG" = "" ]
			then echo "spell cannot read $LOCAL" >&2; EXIT_SPELL=exit; STATUS=1;
			else	echo $ERRMSG >&2; EXIT_SPELL=exit; STATUS=1;
			fi
		     fi
		else ERRMSG=`dspmsg spell.cat 4 "spell cannot identify local spell file"`
			if [ "$ERRMSG" = "" ]
			then echo "spell cannot identify local spell file" >&2; EXIT_SPELL=exit; STATUS=1;
			else	echo $ERRMSG >&2; EXIT_SPELL=exit; STATUS=1;
			fi
		fi ;;
	-l|-i)  M="$D"
		D="$A" ;;
	-d)	# alternate dictionary (not american or british)
		next="D_SPELL="
		;;
	-s)	# alternate stoplist (must be hashed!)
		next="S_SPELL="
		;;
	-h)	# alternate histlist
		next="H_SPELL="
		;;
	-*)	# bad option
		ERRMSG=`dspmsg spell.cat 5 "spell: unknown flag '%s'" $A`
		if [ "$ERRMSG" = "" ]
		then	echo "spell: unknown flag '$A'" 2>&1
		else	echo $ERRMSG 2>&1
		fi
		ERRMSG=`dspmsg spell.cat 6 "usage: spell [-v] [-a|-b] [-x] [-l] [-i] [+hlist] [-d hlist] [-s hstop] [-h sphist] [file ...]"`
		if [ "$ERRMSG" = "" ]
		then	echo "usage: spell [-v] [-a|-b] [-x] [-l] [-i] [+hlist] [-d hlist] [-s hstop] [-h sphist] [file ...]" 2>&1
		else	echo $ERRMSG 2>&1
		fi
		STATUS=1; exit
		;;
	*)	# option arguments or filenames

                # add - spell cmd should check for file existence before calling deroff. -xz
                if [ ! -f "$A" -o ! -r "$A" ]
                then ERRMSG=`dspmsg spell.cat 3 "spell: cannot find or open %s" $A`
                      if [ "$ERRMSG" = "" ]
                      then echo "spell: cannot find or open $A"  2>&1; exit
                      else echo  $ERRMSG 2>&1; exit
                      fi
                fi

		eval $next"$A"
		next="F=$F@"
		;;
	esac
	done
${EXIT_SPELL-:}
IFS="${IFS}@"

if [ -n "$M" ]
then  ERRMSG=`dspmsg spell.cat 7 "multiple -i/-l options in spell, all but the last are ignored"`
	if [ "$ERRMSG" = "" ]
	then	echo "multiple -i/-l options in spell, all but the last are ignored" >&2
	else	echo $ERRMSG 2>&1
	fi
fi

# -l flag to spell means no special flags to deroff (follow ALL .so/.nx)
# default (no -l or -i flag to spell) means don't include /usr/lbin*: deroff -l
# -i flag to spell (follow NO .so/.nx) is passed through to deroff
case "$D" in
	-l)     D="" ;;
	"")     D="-l" ;;
esac

POSTPROC=
if [ -n "$LOCAL" ]
then
	POSTPROC=" | sort | comm -23 - $LOCAL"
fi

if [ -w "$H_SPELL" ]
then
	POSTPROC="$POSTPROC |tee -a $H_SPELL; who am i >>$H_SPELL 2>/dev/null"
fi

eval "$DEROFF -w $D $F |  sort -u +0 |  \
 $P_SPELL ${S_SPELL-/usr/lbin/spell/hstop} 1 | \
 $P_SPELL ${D_SPELL-/usr/lbin/spell/hlista} $V $B \
 $POSTPROC "
case $V in
/dev/null)	exit
esac
sed '/^\./d' $V | sort -u +1f +0
