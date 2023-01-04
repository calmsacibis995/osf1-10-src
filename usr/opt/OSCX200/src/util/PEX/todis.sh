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
# runs each named file through the sed scripts, 
# after renaming original to file.predis

# set to directory containing sed scripts
progpath=`dirname $0`   

for f in $*
do
	mv $f $f.predis
	echo === converting file $f - original in $f.predis ===
	exec sed -f $progpath/todis1.sed "$f.predis" | \
	sed -f $progpath/todis2.sed | \
	sed -f $progpath/todis3.sed | \
	sed -f $progpath/todis4.sed | \
	sed -f $progpath/todis5.sed | \
	sed -f $progpath/todis6.sed > $f
done
