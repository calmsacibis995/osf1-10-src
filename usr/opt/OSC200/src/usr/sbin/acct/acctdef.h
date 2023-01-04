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
 *	@(#)$RCSfile: acctdef.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 05:59:40 $
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
/* acctdef.h	1.4  com/cmd/acct,3.1,8943 10/24/89 10:50:33 */

/*
 *	defines, typedefs, etc. used by acct programs
 */

/*
 *	acct only typedefs
 */

#include <utmp.h>

#define NSZ	(sizeof(((struct utmp *)0)->ut_user))	/* sizeof login name */
#define ISZ	(sizeof(((struct utmp *)0)->ut_id))	/* sizeof /etc/inittab id */
#define LSZ	(sizeof(((struct utmp *)0)->ut_line))	/* sizeof line name */
#define HSZ	(sizeof(((struct utmp *)0)->ut_host))	/* sizeof host name */
#define PSZ	5		/* pid size for input/output format */
#define TSZ	2		/* type size for input/output format */
#define ETSZ	4		/* term. status size for input/output format */
#define EESZ	4		/* exit status size for input/output format */
#define TISZ	10		/* time size for input/output format */
#define P	0	/* prime time */
#define NP	1	/* nonprime time */

/*
 *	limits which may have to be increased if systems get larger
 */
#define A_SSIZE	1000	/* max number of sessions in 1 acct run */
#define A_TSIZE	100	/* max number of line names in 1 acct run */
#define A_USIZE	500	/* max number of distinct login names in 1 acct run */

#define UHASH   401     /* User hash size < USIZE, works best if prime */
#define THASH   79      /* Terminal hash size < TSIZE, works best if prime */

#define EQN(s1, s2)	(strncmp(s1, s2, sizeof(s1)) == 0)
extern char *strncpy();
#define CPYN(s1, s2)	strncpy(s1, s2, sizeof(s1))

#define SECSINDAY	86400L
#define MINS(secs)	((double) secs)/60

/*	convert clicks to Kbytes (ac_mem, see kernel/bsd/kern_acct.c) */
#define KCORE(clicks)	((double) (clicks << CLSIZELOG2)*(getpagesize()/1024))

#define LOGGED_ON       0
#define LOGGED_OFF      1

#define PRECISION	0.01	/* precision for rounding double values */

extern double expacct();
