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
# @(#) $RCSfile: ptecms.awk,v $ $Revision: 4.2 $ (OSF) $Date: 1991/09/21 14:05:48 $
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
# 	ptecms.awk	1.2  com/cmd/acct,3.1,8943 10/24/89 10:58:22
#

BEGIN {
	MAXCPU = 20.0		# report if cpu usage greater than this
	MAXKCORE = 1000.0	# report if KCORE usage is greater than this
	}
NF == 4		{
			print "\t\t\t\t" $1 " Time Exception Command Usage Summary"
		}

NF == 3		{
			print "\t\t\t\tCommand Exception Usage Summary"
		}

NR == 1		{
			MAXCPU = MAXCPU + 0.0
			MAXKCORE = MAXKCORE + 0.0
			print "\t\t\t\tTotal CPU > " MAXCPU " or Total KCORE > " MAXKCORE
		}

NF <= 4 && length != 0	{
				next
			}

$1 == "COMMAND" || $1 == "NAME"		{
						print
						next
					}

NF == 10 && ( $4 > MAXCPU || $3 > MAXKCORE ) && $1 != "TOTALS"

NF == 13 && ( $5 + $6 > MAXCPU || $4 > MAXKCORE ) && $1 != "TOTALS"

length == 0


