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
 *	@(#)$RCSfile: ring.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/06 09:28:17 $
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
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

/*
 * This defines a structure for a ring buffer.
 *
 * The circular buffer has two parts:
 *(((
 *	full:	[consume, supply)
 *	empty:	[supply, consume)
 *]]]
 *
 */
typedef struct {
    char	*consume,	/* where data comes out of */
    		*supply,	/* where data comes in to */
		*bottom,	/* lowest address in buffer */
		*top,		/* highest address+1 in buffer */
		*mark;		/* marker (user defined) */
    int		size;		/* size in bytes of buffer */
    u_int	consumetime,	/* help us keep straight full, empty, etc. */
		supplytime;
} Ring;

/* Here are some functions and macros to deal with the ring buffer */


#if	defined(LINT_ARGS)

/* Initialization routine */
extern int
	ring_init(Ring *ring, char *buffer, int count);

/* Data movement routines */
extern void
	ring_supply_data(Ring *ring, char *buffer, int count);
#ifdef  notdef
extern void
	ring_consume_data(Ring *ring, char *buffer, int count);
#endif

/* Buffer state transition routines */
extern void
	ring_supplied(Ring *ring, int count),
	ring_consumed(Ring *ring, int count);

/* Buffer state query routines */
extern int
	ring_empty_count(Ring *ring),
	ring_empty_consecutive(Ring *ring),
	ring_full_count(Ring *ring),
	ring_full_consecutive(Ring *ring);

#else /* LINT_ARGS */
extern int
	ring_init();

extern void
    ring_supply_data();
#ifdef notdef
extern void
    ring_consume_data();
#endif

extern void
    ring_supplied(),
    ring_consumed();

extern void
    ring_clear_mark(),
    ring_mark();

extern int
    ring_empty_count(),
    ring_empty_consecutive(),
    ring_full_count(),
    ring_full_consecutive();
#endif	/* defined(LINT_ARGS) */
