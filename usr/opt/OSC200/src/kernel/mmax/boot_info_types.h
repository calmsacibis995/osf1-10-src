/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
/*	
 *	@(#)$RCSfile: boot_info_types.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:39:55 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *        Copyright 1985, 1986 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Include file description:
 *	Data information corresponding to boot.h, for sysboot and sysparam
 *
 * Original Author: Sharon Krause	Created on: May 5, 1986
 */

/* this structure corresponds to boot.h, and must correspond exactly!! */

typedef struct boot_info {
	char	bi_expl[80];
	char	bi_name[32];
	int	bi_type;
	int	bi_min;
	int	bi_max;
	int	bi_def;
	int	bi_size;
	int	bi_special;
} BOOT_INFO;

/* definitions for the boot_info.h structure */

#define MINDYNSIZE	524288

#define BI_CHAR		1001
#define BI_DEV		1002
#define BI_HDR		1003
#define BI_INFO		1004
#define BI_INT		1005
#define BI_INV		1006
#define BI_HEX		1007

#define AUTO_SIZE	-5000

#define SV_NONE		-5999

/* these are the same on purpose */

#define SPECIAL_CASE	-6000
#define CALCULATED_VAL	-6000

#define SV_MAXCPUS	-6001
#define SV_MAXUSERS	-6002
#define SV_MAX_NBUFFERS	-6003
#define SV_WSSRATE	-6004
#define SV_MAX_NFSDESC	-6005
#define SV_MAX_NFILE	-6006
#define SV_INIT_NINODE	-6007
#define SV_MAX_NINODE	-6008
#define SV_MAX_NCYLGR	-6009
#define SV_BUF_HASHFSZ	-6010
#define SV_INODE_HASHFSZ	-6011
#define SV_CG_HASHFSZ	-6012
#define SV_INIT_NFILE	-6013
#define SV_NUM_CBLKS	-6014
#define SV_NUM_MSGS	-6015
#define SV_INIT_NBUFFERS	-6016
#define SV_DYNSIZE	-6017
#define SV_CBLK_RNDUP	-6018
#define SV_DAYLIGHT	-6019
#define SV_DEVICE	-6020
#define SV_MINUTES	-6021
