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
 *	@(#)$RCSfile: thread_swap.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 17:46:16 $
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
 *	File:	kern/thread_swap.h
 *
 *	Declarations of thread swap_states and swapping routines.
 */

/*
 *	Swap states for threads.
 */

#ifndef	_KERN_THREAD_SWAP_H_
#define _KERN_THREAD_SWAP_H_

#define TH_SW_UNSWAPPABLE	1	/* not swappable */
#define TH_SW_IN		2	/* swapped in */
#define TH_SW_GOING_OUT		3	/* being swapped out */
#define TH_SW_WANT_IN		4	/* being swapped out, but should
					   immediately be swapped in */
#define TH_SW_OUT		5	/* swapped out */
#define TH_SW_COMING_IN		6	/* queued for swapin, or being
					   swapped in */

/*
 *	exported routines
 */
extern void	thread_swapper_init();
extern void	thread_swapin( /* thread_t thread */ );
extern void	swapin_thread();
extern void	swapout_threads();
extern void	swapout_thread();
extern void	thread_swappable( /* thread_t thread, boolean_t swappable */ );
extern void	thread_doswapin( /* thread_t thread */ );

#endif	/* _KERN_THREAD_SWAP_H_ */
