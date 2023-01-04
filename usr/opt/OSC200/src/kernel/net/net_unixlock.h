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
 *	@(#)$RCSfile: net_unixlock.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:45:34 $
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
 * Lock debugging aids for UNIX.
 *
 * Because of splnet/splimp and single threading, these locks
 * should always succeed. Assertions provided for debugging.
 */

#ifndef _KERN_LOCK_H_
#define _KERN_LOCK_H_

typedef struct slock {
	unsigned long sm_lock;
#define S_LCK	(unsigned long)(0x87654321)
#define S_ULCK	(unsigned long)(0x12345678)
} *simple_lock_t, simple_lock_data_t;

typedef struct lock {
	unsigned long rw_lock;
#define A_LCK	(unsigned long)(0x9abcdef0)
#define R_LCK	(A_LCK|1)
#define W_LCK	(A_LCK|2)
#define L_ULCK	(unsigned long)(0x0fedcba9)
	int	count;
} *lock_t, lock_data_t;

extern char _net_lock_format_[];
extern char _net_simple_lock_[],_net_simple_unlock_[];
extern char _net_lock_write_[],_net_lock_read_[],_net_lock_recursive_[];
extern char _net_lock_write_to_read_[],_net_lock_done_[];

#define simple_lock_init(slp)	  \
	((slp)->sm_lock = S_ULCK)

#define simple_lock(slp)	{ \
	LOCK_ASSERTL(_net_simple_lock_, ((slp)->sm_lock == S_ULCK)); \
	(slp)->sm_lock = S_LCK; \
}
#define simple_unlock(slp)	{ \
	LOCK_ASSERTL(_net_simple_unlock_, ((slp)->sm_lock == S_LCK)); \
	(slp)->sm_lock = S_ULCK; \
}

#define lock_init2(lp,a,c) {	  \
	(lp)->rw_lock = L_ULCK; \
	(lp)->count = 0; \
}
#define lock_islocked(lp)	  \
	(((lp)->rw_lock & ~0x3) == A_LCK)
#define lock_write(lp)		{ \
	if ((lp)->count == 0) { \
		LOCK_ASSERTL(_net_lock_write_, ((lp)->rw_lock == L_ULCK)); \
		(lp)->rw_lock = W_LCK; \
	} else { \
		LOCK_ASSERTL(_net_lock_write_, ((lp)->rw_lock == W_LCK)); \
		(lp)->count++; \
	} \
}
#define lock_read(lp)		{ \
	if ((lp)->count == 0) { \
		LOCK_ASSERTL(_net_lock_read_, ((lp)->rw_lock == L_ULCK)); \
		(lp)->rw_lock = R_LCK; \
	} else { \
		LOCK_ASSERTL(_net_lock_read_, ((lp)->rw_lock == W_LCK)); \
		(lp)->count++; \
	} \
}
#define lock_write_to_read(lp)	{ \
	LOCK_ASSERTL(_net_lock_write_to_read_, ((lp)->rw_lock == W_LCK && (lp)->count == 0)); \
	(lp)->rw_lock = R_LCK; \
}
#define lock_done(lp)		{ \
	LOCK_ASSERTL(_net_lock_done_, lock_islocked(lp)); \
	if ((lp)->count > 1) (lp)->count--; \
	else { (lp)->rw_lock = L_ULCK; (lp)->count = 0; } \
}
#define lock_set_recursive(lp)	{ \
	LOCK_ASSERTL(_net_lock_recursive_, ((lp)->rw_lock == W_LCK)); \
	if ((lp)->count == 0) (lp)->count = 1; \
}
#define lock_clear_recursive(lp){ \
	LOCK_ASSERTL(_net_lock_recursive_, ((lp)->rw_lock == W_LCK && (lp)->count)); \
	if ((lp)->count == 1) (lp)->count = 0; \
}
#endif
