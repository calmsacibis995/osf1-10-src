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
# @(#)$RCSfile: cc.sh,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/09 20:29:46 $ 
# 
# 1-jun-1991: dave scoda
#	original implementation
#
# 20-aug-1991: vipul patel
#	support for shared library. s5path usage.
#
# 27-Nov-1991: Alaa Zeineldine
#	Change s5 to svid2
#

static=0
CC=/usr/bin/cc
SVID2PATH=`cat /etc/svid2_path`
LDINITFL=-Wl,-init,__sys5init_

for i in $*
do
	if test $i = "-non_shared"
	then
		cmd="$CC $LDINITFL -I$SVID2PATH/usr/include -u _habitat_id -L$SVID2PATH/usr/lib"
		static=1
	fi
done

# If we are using shared library,
# give a SVID2 habitat archieved libm
# instead of the default libm.a
# A shared libm is not supported yet.
#
if [ "$static" -ne 1 ]
then
	cmd="$CC -I$SVID2PATH/usr/include -u _habitat_id -L$SVID2PATH/usr/shlib -L$SVID2PATH/usr/lib -L/usr/shlib"
fi

for i in $*
do
	if test $i = "-lc"
	then
		cmd="$cmd -lsys5 -lc"
	else
		cmd="$cmd $i"
	fi
done
cmd="$cmd -lsys5"

$cmd
