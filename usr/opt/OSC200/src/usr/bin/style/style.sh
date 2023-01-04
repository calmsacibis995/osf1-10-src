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
#	@(#)style.sh	3.1	(ULTRIX/OSF)	2/26/91
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

#

#
# COMPONENT_NAME: (CMDTEXT) Text Formatting Services
#
# FUNCTIONS:
#
# ORIGINS: 26,27,28
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
#	style.sh	4.5	(Berkeley)	82/11/06
#
L=/usr/lbin
B=/usr/bin
echo " " $*
sflag=-s
eflag=
Pflag=
nflag=
lflag=
lcon=
rflag=
rcon=
mflag=-me
mlflag=-ml
kflag=
for i in $*
do case $i in
-r) rflag=-r; shift; rcon=$1;shift;continue;;
-l)lflag=-l; shift; lcon=$1;shift;continue;;
-mm) mflag=-mm;shift;continue;;
-ms) mflag=-ms;shift;continue;;
-me) mflag=-me;shift;continue;;
-ma) mflag=-ma;shift;continue;;
-li|-ml) mlflag=-ml;shift;continue;;
+li|-tt)mlflag=;shift;continue;;
-p) sflag=-p;shift;continue;;
-a) sflag=-a;shift;continue;;
-e) eflag=-e;shift;continue;;
-P) Pflag=-P;shift;continue;;
-n) nflag=-n;shift;continue;;
-N) nflag=-N;shift;continue;;
-k) kflag=-k;shift;continue;;
-flags) dspmsg style.cat 1 "%s [-flags] [-r num] [-l num] [-e] [-p] [-n] [-N]\
 [-a] [-P] [-mm|-ms] [-li|+li] [file ...]\n" $0;exit;;
-*) dspmsg style.cat 2 "unknown style flag %s\n" $i; echo "Usage:"
    dspmsg style.cat 1 "%s [-flags] [-r num] [-l num] [-e] [-p] [-n] [-N]\
 [-a] [-P] [-mm|-ms] [-li|+li] [file ...]\n" $0;exit 1;;
*) break;;
esac
done
$B/deroff $kflag $mflag $mlflag $* | $L/style1 | $L/style2 | \
          $L/style3 $rflag $rcon $lflag $lcon $sflag $nflag $eflag $Pflag

