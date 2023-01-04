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
 * @(#)$RCSfile: ts_supp.h,v $ $Revision: 1.1.10.4 $ (DEC) $Date: 1993/07/20 22:48:06 $
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */


/*
 * Macros for thread safe work.
 *
 * The idea is to make the shared library code easier to read
 * and maintain by avoiding the loathsome #ifdef.
 * Sometimes this works.
 */

#ifndef _TS_SUPP_H_
#define	_TS_SUPP_H_

#if defined(_THREAD_SAFE) || defined(_REENTRANT)

#include	<errno.h>

#define	TS_LOCK(lock)		_rec_mutex_lock(lock)
#define	TS_TRYLOCK(lock)	_rec_mutex_trylock(lock)
#define	TS_UNLOCK(lock)		_rec_mutex_unlock(lock)
#define	TS_READLOCK(lock)	__rec_mutex_readlock(lock)
#define	TS_READUNLOCK(lock)	__rec_mutex_readunlock(lock)

#define	TS_FDECLARELOCK(lock)	filelock_t lock;
#define	TS_FLOCK(lock, iop)	(lock = _flockfile(iop))
#define	TS_FTRYLOCK(lock, iop)	(lock = _ftestfile(iop))
#define	TS_FUNLOCK(lock)	_funlockfile(lock)

#define	TS_EINVAL(arg)		if (arg) return (_Seterrno(EINVAL), -1)
#define	TS_ERROR(e)		_Seterrno(e)
#define	TS_SETERR(e)		seterrno(e)
#define	TS_GETERR()		geterrno()
#define	TS_RETURN(ts, nts)	return (ts)

#define	TS_SUCCESS		0
#define	TS_FAILURE		-1
#define	TS_FOUND(ret)		(TS_SUCCESS)
#define	TS_NOTFOUND		(_Seterrno(ESRCH), TS_FAILURE)

#else

#define	TS_LOCK(lock)
#define	TS_TRYLOCK(lock)	1
#define	TS_UNLOCK(lock)
#define	TS_READUNLOCK(lock)	1	/* unlock succeeded */
#define	TS_READLOCK(lock)

#define	TS_FDECLARELOCK(lock)
#define	TS_FLOCK(lock, iop)
#define	TS_FTRYLOCK(lock, iop)	1
#define	TS_FUNLOCK(lock)

#define	TS_EINVAL(arg)
#define	TS_ERROR(e)
#define	TS_SETERR(e)		(errno=e)
#define	TS_GETERR()		errno
#define	TS_RETURN(ts, nts)	return (nts)

#define	TS_SUCCESS		1
#define	TS_FAILURE		0
#define	TS_FOUND(ret)		(ret)
#define	TS_NOTFOUND		(TS_FAILURE)

#endif	/* _THREAD_SAFE || _REENTRANT */

#endif	/* _TS_SUPP_H_ */
