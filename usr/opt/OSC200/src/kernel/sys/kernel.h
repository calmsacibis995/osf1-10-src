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
 *	@(#)$RCSfile: kernel.h,v $ $Revision: 4.3.3.2 $ (DEC) $Date: 1992/12/15 15:45:33 $
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/* 
 *  kernel.h
 *
 * Modification History:
 *
 * 4-Apr-91     Paula
 *      Add P1003.4 required extensions.  
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined  POSIX support routines are defined
 *
 */

/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef	_SYS_KERNEL_H_
#define _SYS_KERNEL_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>		/* for MAXHOSTNAMELEN */
#include <rt_timer.h>
/*
 * Global variables for the kernel
 */

extern long	rmalloc();

/* 1.1 */
/*
 * The hostname lock protects the hostname, hostid, domainname
 * and uname (SysV) variables.
 */
lock_data_t			hostname_lock;
#define	HOSTNAME_LOCKINIT()	lock_init2(&hostname_lock,TRUE,LTYPE_HOSTNAME)
#define	HOSTNAME_READ_LOCK()	lock_read(&hostname_lock)
#define	HOSTNAME_READ_UNLOCK()	lock_read_done(&hostname_lock)
#define	HOSTNAME_WRITE_LOCK()	lock_write(&hostname_lock)
#define	HOSTNAME_WRITE_UNLOCK()	lock_write_done(&hostname_lock)

extern int	hostid;
extern char	hostname[MAXHOSTNAMELEN];
extern int	hostnamelen;
extern char	domainname[MAXDOMNAMELEN];
extern int	domainnamelen;

/* 1.2 */
extern struct	timeval boottime;
extern struct	timeval time;
extern struct	timezone tz;		/* XXX */
extern int	hz;
extern int	phz;			/* alternate clock's frequency */
extern int	tick;
extern time_t	lbolt;			/* awoken once a second */
extern int	realitexpire();
#if RT_TIMER
extern int      psx4_tod_expire();
extern int      psx4_set_todtimer();
extern int      psx4_get_todtimer();
#endif
#define LSCALE	1000		/* scaling for "fixed point" arithmetic */
extern	long	avenrun[3];
extern	long	mach_factor[3];

#endif	/* _KERNEL */
#endif	/* _SYS_KERNEL_H_ */
