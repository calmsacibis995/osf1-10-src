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
# @(#)$RCSfile: gitout.sh,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/13 00:16:59 $
# 


GITOUT_MSG="
	To restart the DEC OSF/1 AXP(tm) Factory-Installed
	Software startup procedure, reboot your system.\n\n"

MSG="
	***********************************************************
	*  Welcome to the DEC OSF/1 AXP(tm) Factory-Installed     * 
	*  Software Startup Procedure.                            *
	***********************************************************

	During this startup procedure, you will be prompted to enter 
	the following information:

		o  a system name 

		o  a new superuser password 

		o  your local time zone 

		o  the date and time

	To discontinue this procedure and halt your system,
	enter n at the following prompt.
"
QRY="\tWould you like to continue? (y/n): \c"



echo "$MSG"
while :
do
	echo "$QRY"
	read resp
	case $resp in
	[Yy]* ) exit 0 ;;
	[Nn]* ) echo "$GITOUT_MSG"; exit 1 ;;
	esac
done
