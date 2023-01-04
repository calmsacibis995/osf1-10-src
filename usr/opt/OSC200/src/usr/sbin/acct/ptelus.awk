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
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
#
# OSF/1 Release 1.0

# 
# 	@(#) $RCSfile: ptelus.awk,v $ $Revision: 4.2 $ (OSF) $Date: 1991/09/21 14:05:52 $
# 
# 	 COMPONENT_NAME: (CMDACCT) Command Accounting
# 
# 	 FUNCTIONS: none
# 
# 	 ORIGINS: 3, 9, 27
# 
# 	 (C) COPYRIGHT International Business Machines Corp. 1985, 1989
# 	 All Rights Reserved
# 	 Licensed Materials - Property of IBM
# 
# 	 US Government Users Restricted Rights - Use, duplication or
# 	 disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 	ptelus.awk	1.2  com/cmd/acct,3.1,8943 10/24/89 10:58:37
#

BEGIN	{
	MAXCPU = 20.		# report if cpu usage is greater than this
	MAXKCORE = 500.		# report is Kcore usage is greater than this
	MAXCONNECT = 120.	# report if connect time is greater than this
	}
NR == 1	 {
	MAXCPU = MAXCPU + 0
	MAXKCORE = MAXKCORE + 0
	MAXCONNECT = MAXCONNECT + 0
	printf "Logins with exceptional Prime/Non-prime Time Usage\n"
	printf ( "CPU > %d or KCORE > %d or CONNECT > %d\n\n\n", MAXCPU, MAXKCORE, MAXCONNECT)
	printf "\tLogin\t\tCPU (mins)\tKCORE-mins\tCONNECT-mins\tdisk"
	printf "\t# of\t# of\t# Disk\tfee\n"
	printf "UID\tName\t\tPrime\tNprime\tPrime\tNprime\t"
	printf "Prime\tNprime\tBlocks\tProcs\tSess\tSamples\n\n"
	}
$3 > MAXCPU || $4 > MAXCPU || $5 > MAXKCORE || $6 > MAXKCORE || $7 > MAXCONNECT || $8 > MAXCONNECT {
				printf("%d\t%-8.8s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", $1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
				}
