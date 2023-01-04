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
# @(#)$RCSfile: c_install.sh,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/04 20:29:33 $ 
#
###################################################
# c_install - performs setld -c INSTALL for subsets
#
# History
#
# 001	24-apr-1991	Jon Wallace
#	First Implementation
#
# 002	24-jun-1991	Jon Wallace
#	Removed logging.  Logging by 'it' is sufficient.
#
###################################################

DATA_FILE=/sbin/it.d/data/cinst.data

echo "\n\n*** SYSTEM CONFIGURATION ***\n"
if [ -s $DATA_FILE ]
then
	SUBSET_LIST=`cat $DATA_FILE`
else
	echo "c_install: Cannot find $DATA_FILE!\n\n"
	exit 0
fi

for SUBSET in $SUBSET_LIST
do
	setld -c $SUBSET INSTALL || 
	{
		echo "c_install: configuration of $SUBSET failed!\n"
		echo "           See the /var/adm/smlogs/it.log for error messages."
		echo "           After the system reboots and the problem has been corrected,"
		echo "           configure $SUBSET manually by executing:"
		echo "           'setld -c $SUBSET INSTALL'\n\n"
	}
done

rm -f $DATA_FILE
