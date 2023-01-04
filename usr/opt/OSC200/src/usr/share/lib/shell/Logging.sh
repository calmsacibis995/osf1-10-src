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
#	Logging.sh
#
#		routines to support logfiles
#
#	000	04-mar-1991	ccb
#		culled from setld
#

#	Routines supported:
#
#		Log(), WriteLog()
#
#	Requires Global Variable LOGFILE
#
#

LOGFILE=/dev/null

:	-Log
#		write entry to log file
#
#	given:	an entry to write
#	does:	writes entry to logfile named in global LOGFILE
#	return:	nothing

Log()
{
	echo "$1" >> $LOGFILE
}


:	-WriteLog
#		xfer stdin to logfile
#
#	given:	no args, reads stdin
#	does:	xfer stdin to logfile $LOGFILE
#	return:	ignore

WriteLog()
{ (
	2>&1 cat > $LOGFILE
) }

