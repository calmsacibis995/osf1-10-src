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
 *	@(#)$RCSfile: stdio_lock.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/08 00:38:07 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Thread safe locking extenstions for stdio.h
 *
 * OSF/1 Release 1.0
 */
/*
 * stdio_lock.h
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#ifndef	_STDIO_LOCK_H
#define	_STDIO_LOCK_H

#include "rec_mutex.h"

typedef	rec_mutex_t	filelock_t;	/* pointer to rec_mutex struct */

#define	_funlockfile(filelock) \
		(((filelock) == (rec_mutex_t) NULL) ? (void)0 : \
					_rec_mutex_unlock(filelock))

/* old version left here for reference
#define _funlockfile(filelock)	if (filelock) rec_mutex_unlock(filelock);
*/

#define	_flockfile(iop)	\
		(filelock_t)(((iop)->_lock == (rec_mutex_t) NULL) ? NULL : \
			(rec_mutex_lock((iop)->_lock), (iop)->_lock))

#define	_ftestfile(iop)	\
		(filelock_t)(((iop)->_lock == (rec_mutex_t) NULL) ? NULL : \
			 (rec_mutex_trylock((iop)->_lock) ? (iop)->_lock : NULL))

#endif /* _STDIO_LOCK_H */
