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
 *	@(#)$RCSfile: systrace.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/07/15 18:52:56 $
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
 
#ifndef _SYSTRACE_H_
#define _SYSTRACE_H_
/* 
 * derived from systrace.h	4.1  (ULTRIX)        7/2/90    
 */

#define TR_BUFSIZE 8192		/* must be multiple of 1024 and < MAXBSIZE */
#define FRACTION 3/5		/* FRACTION full buffer select wakes you up */
/* u.u_tracedev is the integer offset into tr_users[] */
#define TR_USRS 16		/* number of simultaneous users */
#define TR_PIDS 16		/* number of pids to trace at a time */
#define TR_UIDS 16		/* number of uids to trace at a time */
#define TR_PGRP 16		/* number of pgrps to trace at a time */
#define TR_SYSC 16		/* number of syscalls to trace at a time */
extern int tracelost;		/* count of lost records from buffer overflow */
extern int tracebsize;		/* trace buffer size */
extern int traceopens;		/* how many active opens */
	/* before and after flags for syscall_trace routine */
#define BEFORE 1
#define AFTER  0

#ifdef SYS_TRACE
struct trace {
/* leave out kernel structure users only need defines! */
	int pids[TR_PIDS];	/* pids we are tracing for this user */
	int uids[TR_UIDS];	/* uids we are tracing for this user */
	int sysc[TR_SYSC];	/* syscalls we are tracing for this user */
	int pgrp[TR_PGRP];	/* pgrps we are tracing for this user */
	int traceflag;		/* what to trace -- see flags below */
	int open;		/* is this pseudo-device open */
	struct buf *bp;		/* buffer pointer */
	int uid,ruid;		/* privilege controls set at open */
	struct proc *tr_proc;	/* proc addr for select */
} tr_user[TR_USRS];		/* one structure per user */
#endif /* SYS_TRACE */

/* bits for open */
#define OPEN	1
#define CLOSED	0
/* traceflags -- bit masks for each option -- used below in IOCTL's */
/* we only have 16 bits to play with */
#define OFF	0
#define ON	1
#define PIDS	(1<<1)
#define UIDS	(1<<2)
#define SYSC	(1<<3)
#define PGRP	(1<<4)
#define ALL	(1<<15)
/* hi two bits control in and out of parameters -- see ioctl.h */
/* low 7 bits of high 16 bits are size of parameters (128 bytes max = 2^7) */
#define GET	_IOC_OUT
#define SET	_IOC_IN
/*
 * if (0 <= u.u_tracedev < TR_USRS && tr_users[u.u_tracedev].open == 1)
 * this is this users structure
 */
/*
 * We can have TR_USRS simultaneous users tracing a list of up to TR_PIDS
 * process ids per user, TR_UIDS different uids (all procs) per user,
 * TR_SYSC different syscalls (all calls, all procs) per user,
 * and up to TR_PGRP different process groups (all calls, all procs) per user.
 * Of course for security reasons, only root can do all of these. Individual
 * users would be limited to their process groups and their pids and uid. 
 */
/*      Symbolic Name   CMD  IN/OUT SIZE				*/
#define IOTR_GETOFF	OFF  | GET | (sizeof(int) << 16)
#define IOTR_GETON	ON   | GET | (sizeof(int) << 16)
#define IOTR_GETALL	ALL  | GET | (sizeof(int) << 16)
#define IOTR_GETPIDS	PIDS | GET | ((TR_PIDS*sizeof(int)) << 16)
#define IOTR_GETUIDS	UIDS | GET | ((TR_UIDS*sizeof(int)) << 16)
#define IOTR_GETSYSC	SYSC | GET | ((TR_SYSC*sizeof(int)) << 16)
#define IOTR_GETPGRP	PGRP | GET | ((TR_PGRP*sizeof(int)) << 16)

#define IOTR_SETOFF	OFF  | SET | (sizeof(int) << 16)
#define IOTR_SETON	ON   | SET | (sizeof(int) << 16)
#define IOTR_SETALL	ALL  | SET | (sizeof(int) << 16)
#define IOTR_SETPIDS	PIDS | SET | ((TR_PIDS*sizeof(int)) << 16)
#define IOTR_SETUIDS	UIDS | SET | ((TR_UIDS*sizeof(int)) << 16)
#define IOTR_SETSYSC	SYSC | SET | ((TR_SYSC*sizeof(int)) << 16)
#define IOTR_SETPGRP	PGRP | SET | ((TR_PGRP*sizeof(int)) << 16)

#endif
