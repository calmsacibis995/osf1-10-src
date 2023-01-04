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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
static char rcsid[] = "@(#)$RCSfile: cron.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/10 14:12:36 $";
 */
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * cron.h   1.9  com/cmd/oper/cron,3.1,9021 4/18/90 09:01:52
 */

#ifndef	FALSE
#define FALSE		0
#endif
#ifndef	TRUE
#define TRUE		1
#endif
#define MINUTE		60L
#define HOUR		60L*60L
#define DAY		24L*60L*60L
#define	NQUEUE		26		/* number of queues available */
#define	ATEVENT		0		/* queue 'a' has regular(sh) at jobs */
#define BATCHEVENT	1		/* queue 'b' has batch jobs */
#define CRONEVENT	2		/* queue 'c' has cron jobs  */
#define SYNCEVENT	3		/* queue 'd' is a sync     */
#define KSHEVENT	4		/* queue 'e' has ksh at jobs  */
#define CSHEVENT	5		/* queue 'f' has csh at jobs  */

#define ADD		'a'
#define DELETE		'd'
#define	AT		'a'
#define CRON		'c'

#define ROOT		0		/* Root userid */

#define	FLEN	30
#define	LLEN	9
/*****
#define	FLEN	15
#define	LLEN	9
*******/

/* structure used for passing messages from the
   at and crontab commands to the cron			*/

struct	message {
	char	etype;
	char	action;
	char	fname[FLEN];
	char	logname[LLEN];
} msgbuf;

/* anything below here can be changed */

#define CRONDIR		"/var/spool/cron/crontabs"
#define ATDIR		"/var/spool/cron/atjobs"
#define ACCTFILE	"/var/adm/cron/log"
#define CRONALLOW	"/var/adm/cron/cron.allow"
#define CRONDENY	"/var/adm/cron/cron.deny"
#define ATALLOW		"/usr/lib/cron/at.allow"	/* XPG4 location */
#define ATDENY		"/usr/lib/cron/at.deny"		/* XPG4 location */
#define PROTO		"/var/adm/cron/.proto"
#define	QUEDEFS		"/var/adm/cron/queuedefs"
#define	FIFO		"/var/adm/cron/FIFO"

#define SHELL		"/usr/bin/sh"	/* shell to execute */
#define CSH		"/usr/bin/csh"
#define KSH		"/usr/bin/ksh"

#define CTLINESIZE	1000	/* max chars in a crontab line */
#define UNAMESIZE	20	/* max chars in a user name */

#define CRON_SORT_M	0x00000001	/* sorted by submission time */
#define CRON_COUNT_JOBS	0x00000002	/* count of number of jobs */
#define CRON_USER	0x00000004	/* all at jobs for user removed */
#define CRON_PROMPT	0x00000008	/* prompt to validate removal */
#define CRON_QUIET	0x00000010	/* no output is printed */
#define CRON_VERBOSE	0x00000020	/* display all of crontab file */
#define CRON_SORT_E	0x00000040	/* sort in order of execution */
#define CRON_NON_VERBOSE 0x00000080	/* no extra information */
#define AT_JOB_ID	0x00000100	/* a specific at job  */
