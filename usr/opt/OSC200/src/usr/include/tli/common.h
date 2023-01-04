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
 * @(#)$RCSfile: common.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/05/12 19:40:41 $
 */
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
 /** Copyright (c) 1988  Mentat Inc.
  ** common.h 1.2, last change 11/9/89
  **/

#ifndef	_TLI_COMMON_H
#define	_TLI_COMMON_H

#include <sys/types.h>
#include <sys/errno.h>
#include <tli/tlistate.h>

#ifdef XTI
#include <xti.h>
#else
#include <tiuser.h>
#endif

#ifdef XTIDBG
#include <tli/tdbg.h>
#endif

#define	staticf		static

#define	noop
#define	fallthru

typedef	int		boolean;
#define true		((boolean)1)
#define	false		((boolean)0)

#define A_CNT(arr)	(sizeof(arr)/sizeof(arr[0]))
#define	A_END(arr)	(&arr[A_CNT(arr)])
#define	A_LAST(arr)	(&arr[A_CNT(arr)-1])

#define getarg(ac,av)	(optind < ac ? av[optind++] : nilp(char))
#define newa(t,cnt)     ((t *)calloc(cnt, sizeof(t)))
#define	noshare
#define ERR		(-1)

#define	nilp(t)		((t *)0)
#define	nil(t)		((t)0)

#define reg		register

#define	stream_open	open
#define	stream_close	close
#define	stream_ioctl	ioctl
#define	stream_read	read
#define	stream_write	write

#define TLI_STACK_BUF_SIZE	512
#define _DEFAULT_STRMSGSZ	(STRMSGSZ ? STRMSGSZ : 4096)
#define _DEFAULT_STRCTLSZ	(STRCTLSZ ? STRCTLSZ : 4096)


#define TLI_NUM_STATES	9
#define TLI_NUM_EVENTS	25

extern char tli_state[TLI_NUM_EVENTS][TLI_NUM_STATES];

#define TLI_BAD_STATE	-1
#define TLI_TSTATECHNG	8

#define TLI_OPEN	0
#define TLI_BIND	1
#define TLI_OPTMGMT	2
#define TLI_UNBIND	3
#define TLI_CLOSE	4
#define TLI_CONNECT1	5
#define TLI_CONNECT2	6
#define TLI_RCVCONN	7
#define TLI_LISTEN	8
#define TLI_ACCEPT1	9
#define TLI_ACCEPT2	10
#define TLI_ACCEPT3	11
#define TLI_SND		12
#define TLI_RCV		13
#define TLI_SNDDIS1	14
#define TLI_SNDDIS2	15
#define TLI_RCVDIS1	16
#define TLI_RCVDIS2	17
#define TLI_RCVDIS3	18
#define TLI_SNDREL	19
#define TLI_RCVREL	20
#define TLI_PASSCON	21
#define TLI_SNDUDATA	22
#define TLI_RCVUDATA	23
#define TLI_RCVUDERR	24


#define TLI_NEXTSTATE(tli, x) \
	if (((tli)->tlis_state == TSTATECHNG) || \
	   ((tli)->tlis_state == TLI_TSTATECHNG) || \
	   (((tli)->tlis_state = tli_state[x] [(tli)->tlis_state]) == -1)) { \
	  	TLI_UNLOCK(tli); \
		TLI_TSYNC(tli->tlis_fd); \
	}


#if defined(_THREAD_SAFE) || defined(_REENTRANT)
/*
 * Per thread t_errno is provided by the threads provider. Both the extern int
 * and the per thread value must be maintained buy the threads libarary.
 */
#include <lib_lock.h>
#include <lib_data.h>

extern lib_lock_functions_t	__t_lock_funcs;

#define t_errno	(*_terrno())

#define IOSTATE_LOCK()		lib_mutex_lock(__t_lock_funcs, &__t_mutex);
#define IOSTATE_UNLOCK()	lib_mutex_unlock(__t_lock_funcs, &__t_mutex);


#define TLI_LOCK(tli) \
		if ((tli) != NULL && (tli)->tlis_lock != NULL)  \
			lib_mutex_lock(__t_lock_funcs, (tli)->tlis_lock)
		
#define TLI_UNLOCK(tli)  \
		if ((tli) != NULL && (tli)->tlis_lock != NULL) \
                	lib_mutex_unlock(__t_lock_funcs, (tli)->tlis_lock)

#define TLI_LOCKCREATE(tli) \
		lib_mutex_create(__t_lock_funcs, &(tli)->tlis_lock)

#define TLI_LOCKDELETE(tli) \
		if ((tli) != NULL) \
			(void)lib_mutex_delete(__t_lock_funcs, &(tli)->tlis_lock)

#define TLI_LOOK(fd, tli)	__t_look(fd, tli)
#ifdef XTI
#define TLI_ILOOK(fd, tli)	__t_ilook(fd, tli)
#else
#define TLI_ILOOK(fd, tli)	__t_look(fd, tli)
#define t_ilook t_look
#endif
#define TLI_TSYNC(fd)		iostate_sw(fd, IOSTATE_SYNC)

#else 

#define TLI_TSYNC		t_sync

#define TLI_LOOK(fd, tli)	t_look(fd)
#ifdef XTI
#define TLI_ILOOK(fd, tli)	t_ilook(fd)
#else
#define TLI_ILOOK(fd, tli)	t_look(fd)
#define t_ilook t_look
#endif

#define IOSTATE_LOCK()
#define IOSTATE_UNLOCK()

#define TLI_LOCK(tli)
#define TLI_UNLOCK(tli)
#define TLI_LOCKCREATE(tli)	(1)
#define TLI_LOCKDELETE(tli)

#endif	/* _REENTRANT */

extern int t_errno;

#endif	/* _TLI_COMMON_H */
