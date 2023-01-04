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
#	@(#)spellin.sh	3.1	(ULTRIX/OSF)	2/26/91
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



# COMPONENT_NAME: (CMDTEXT) Text Formatting Services
# 
# FUNCTIONS:
# 
# ORIGINS: 3,10,13,27
# 
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
# 
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# spellin.sh - front end to /usr/lbin/spell/spellinprg
# 
# This front end script provides for the overlapping functionality
# of the BSD and ATT versions (which are completely different).
# spellin.sh	1.5  com/bsd.d/spell.d,3.1,9011A 3/13/90 09:23:28
# 

PATH=:/usr/bin
export PATH

SPELL=/usr/lbin/spell
TMP=/tmp/spellin
TMPA=${TMP}A.$$
TMPB=${TMP}B.$$
TMPC=${TMP}C.$$

status=0

trap 'rm -f ${TMP}[ABC].$$ >/dev/null 2>&1; exit $status' 0
trap 'status=1;exit' 1 2 3 15

case A$1 in

A[0-9]*)	# spellin num < in > out

	$SPELL/spellinprg $1
	;;

A)		# spellin < in > out

# $SPELL/hashmake | sort -u +0n > $TMPC
	$SPELL/hashmake | sort -u > $TMPC
	$SPELL/spellinprg `wc -l < $TMPC` < $TMPC
	;;

A[!0-9]*)	# spellin list < in > out

	if [ -s $1 ] 
        then
           $SPELL/hashcheck < $1 > $TMPA &
# $SPELL/hashmake | sort -u +0n > $TMPB
	   $SPELL/hashmake | sort -u  > $TMPB
	   wait
	   if [ ! -s $TMPA ]
	   then
	      status=1
	      exit 1
	   fi
	   sort -mu $TMPA $TMPB > $TMPC
	   $SPELL/spellinprg `wc -l < $TMPC` < $TMPC
        else
	   dspmsg spell.cat 8 "spellin:  List file %s does not exist.\n" $1 >&2
	   status=1
	   exit 1
        fi
	;;

esac

status=$?
# trap 0 handles the exit code
