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
 *	@(#)$RCSfile: tacct.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:01:03 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */ 
/* 

 */
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/* tacct.h	1.2  com/cmd/acct,3.1,8943 10/24/89 11:00:40 */

/*
 *	total accounting (for acct period), also for day
 */

/* Float arrays below contain prime and non-prime components */
/* NSZ is define in acctdef.h as (sizeof(((struct utmp *)0)->ut_user)) */
struct	tacct	{
	uid_t		ta_uid;		/* userid */
	char		ta_name[NSZ];	/* login name */
	double          ta_cpu[2];      /* cum. cpu time (mins) */
	double          ta_kcore[2];    /* cum kcore-mins */
	double          ta_io[2];       /* cum. chars xferred (512s) */
	double          ta_rw[2];       /* cum. blocks read/written */
	double          ta_con[2];      /* cum. connect time (mins) */
	double		ta_du;		/* cum. disk usage */
	long            ta_qsys;        /* queueing sys charges (pgs) */
	double		ta_fee; 	/* fee for special services */
	long		ta_pc;		/* count of processes */
	unsigned short	ta_sc;		/* count of login sessions */
	unsigned short	ta_dc;		/* count of disk samples */
};
