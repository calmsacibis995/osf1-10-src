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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* @(#)$RCSfile: utmp.h,v $ $Revision: 4.2.5.2 $ (OSF) $Date: 1993/06/08 01:15:14 $ */
/*
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 */ 
/* utmp.h	1.8  com/inc,3.1,8943 10/11/89 17:45:38 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	utmp.h	5.1 (Berkeley) 5/30/85
 */
#ifndef _UTMP_H_
#define _UTMP_H_
#include <standards.h>

#define UTMP_FILE       "/var/adm/utmp"
#define WTMP_FILE       "/var/adm/wtmp"

#include <sys/types.h>	/* for pid_t, time_t */

/*
 * Structure of utmp and wtmp files.
 *
 * Assuming these numbers is unwise.
 */


#define ut_name ut_user 		/* compatability */
struct utmp {
	char	ut_user[32];		/* User login name */
	char	ut_id[14];		/* /etc/inittab id- IDENT_LEN in init */
	char	ut_line[32];		/* device name (console, lnxx) */
	short	ut_type; 		/* type of entry */
	pid_t	ut_pid;			/* process id */
	struct exit_status {
	    short	e_termination;	/* Process termination status */
	    short	e_exit;		/* Process exit status */
	} ut_exit;			/* The exit status of a process
					 * marked as DEAD_PROCESS.
					 */
	time_t	ut_time;		/* time entry was made */
	char	ut_host[64];		/* host name same as MAXHOSTNAMELEN */
};


/* Definitions for ut_type						*/

#define	EMPTY		0
#define	RUN_LVL		1
#define	BOOT_TIME	2
#define	OLD_TIME	3
#define	NEW_TIME	4
#define	INIT_PROCESS	5	/* Process spawned by "init" */
#define	LOGIN_PROCESS	6	/* A "getty" process waiting for login */
#define	USER_PROCESS	7	/* A user process */
#define	DEAD_PROCESS	8
#define	ACCOUNTING	9

#define	UTMAXTYPE	ACCOUNTING	/* Largest legal value of ut_type */

/*	Special strings or formats used in the "ut_line" field when	*/
/*	accounting for something other than a process.			*/
/*	No string for the ut_line field can be more than 11 chars +	*/
/*	a NULL in length.						*/

#define RUNLVL_MSG      "run-level %c"
#define	BOOT_MSG	"system boot"
#define	OTIME_MSG	"old time"
#define	NTIME_MSG	"new time"

extern struct utmp	*getutent __((void));
extern struct utmp	*getutid __((struct utmp *));
extern struct utmp	*getutline __((struct utmp *));
extern struct utmp	*pututline __((struct utmp *));
extern void		setutent __((void));
extern void		endutent __((void));
extern void		utmpname __((char *));

#if defined(_REENTRANT) || defined(_THREAD_SAFE)

struct utmp_data {
	int		ut_fd;
	long		loc_utmp;
	struct utmp	ubuf;
};

extern int getutent_r __((struct utmp **, struct utmp_data *));
extern int getutid_r __((struct utmp *, struct utmp **, struct utmp_data *));
extern int getutline_r __((struct utmp *, struct utmp **, struct utmp_data *));
extern int pututline_r __((struct utmp *, struct utmp_data *));
extern void setutent_r __((struct utmp_data *));
extern void endutent_r __((struct utmp_data *));

#endif	/* _REENTRANT || _THREAD_SAFE */

#endif  /* _UTMP_H_ */
