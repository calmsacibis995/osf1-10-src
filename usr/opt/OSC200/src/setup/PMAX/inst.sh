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
# 	@(#)inst.sh	3.1	(ULTRIX/OSF)	2/26/91
#
#
#

if [ $# != 2 -a $# != 3 ]
then
    echo "inst.sh <targetroot> <configuration_name> [<log dir>]"
    exit 1
fi

if [ $# = 3 ]
then
    logdir=$3
else
    logdir=../logs
fi


/bin/sh -x setup/PMAX/install.sh $1  > $logdir/install.log  2>&1
/bin/sh -x setup/PMAX/installk.sh $1 $2 > $logdir/installk.log 2>&1 
/bin/sh -x setup/PMAX/stand.sh install $1 > $logdir/inst_stand.log   2>&1 

exit 0
