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
# @(#)all.sh	3.6	(ULTRIX/OSF)	7/7/91
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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

if [ $# != 2 ]
then
    echo "all.sh <script dir> <log dir>"
    exit 1
fi
/bin/sh -x $1/setup.sh  > $2/setup.log  2>&1
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

/bin/sh -x $1/cmplrs.sh  > $2/cmplrs.log  2>&1
RC=$?
if [ ! $RC -eq 0 ] ; then exit $RC ; fi

#/bin/sh -x $1/kernel.sh > $2/kernel.log 2>&1 &
( /bin/sh -x $1/kernel.sh -setup > $2/kernel-setup.log 2>&1 ; \
	/bin/sh -x $1/kernel.sh -std > $2/kernel-std.log 2>&1 ; \
	/bin/sh -x $1/kernel.sh -rt > $2/kernel-rt.log 2>&1 ) &

sleep 60
( /bin/sh -x $1/cmds.sh   > $2/cmds.log   2>&1 ; /bin/sh -x $1/stand.sh all ""  > $2/stand.log   2>&1 ; /bin/sh -x $1/kdebug.sh > $2/kdebug.log   2>&1 ) 

wait

exit 0
