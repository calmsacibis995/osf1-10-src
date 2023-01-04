#! /bin/sh
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
# @(#)$RCSfile: diction.sh,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/12/22 10:09:27 $
# 
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0
#	diction.sh	4.5	(Berkeley)	82/11/06
#

D=/usr/bin
B=/usr/lbin
echo $*
rest=
flag=
nflag=
mflag=-me
lflag=-ml
kflag=
file=
for i
do case $i in
 -f) flag=-f;shift; file=$1; shift; continue;;
-n) nflag=-n;shift; continue;;
-k) kflag=-k;shift; continue;;
 -mm) mflag=$1; shift; continue;;
-ms) mflag=$1;shift;continue;;
-me) mflag=$1;shift;continue;;
-ma) mflag=$1;shift;continue;;
-ml) lflag=$1;shift;continue;;
-*)  dspmsg diction.cat 1 \
     "Usage: diction [-fpattern_file] [-k] [-ma] [-me] [-ml] [-ms] [-n] [file ...]\n";
     exit 2;;
*) rest=$*; break;;
esac
done
 $D/deroff $kflag $lflag $mflag $rest | $B/dprog -d $nflag $flag $file








