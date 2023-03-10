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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ring.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/03/06 09:28:00 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */ 
#if !defined(lint) && !defined(_NOIDENT)

#endif
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
 */

#ifndef lint

#endif /* not lint */

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

#include	<stdio.h>
#include	<errno.h>

#ifdef	size_t
#undef	size_t
#endif

#include	<sys/types.h>
#include	<sys/ioctl.h>
#include	<sys/socket.h>

#include	"ring.h"
#include	"general.h"

/* Internal macros */

#if	!defined(MIN)
#define	MIN(a,b)	(((a)<(b))? (a):(b))
#endif	/* !defined(MIN) */

#define	ring_subtract(d,a,b)	(((a)-(b) >= 0)? \
					(a)-(b): (((a)-(b))+(d)->size))

#define	ring_increment(d,a,c)	(((a)+(c) < (d)->top)? \
					(a)+(c) : (((a)+(c))-(d)->size))

#define	ring_decrement(d,a,c)	(((a)-(c) >= (d)->bottom)? \
					(a)-(c) : (((a)-(c))-(d)->size))


/*
 * The following is a clock, used to determine full, empty, etc.
 *
 * There is some trickiness here.  Since the ring buffers are initialized
 * to ZERO on allocation, we need to make sure, when interpreting the
 * clock, that when the times are EQUAL, then the buffer is FULL.
 */
static u_int ring_clock = 0;


#define	ring_empty(d) (((d)->consume == (d)->supply) && \
				((d)->consumetime >= (d)->supplytime))
#define	ring_full(d) (((d)->supply == (d)->consume) && \
				((d)->supplytime > (d)->consumetime))





/* Buffer state transition routines */

ring_init(ring, buffer, count)
Ring *ring;
char *buffer;
int count;
{
    memset((char *)ring, 0, sizeof *ring);

    ring->size = count;

    ring->supply = ring->consume = ring->bottom = buffer;

    ring->top = ring->bottom+ring->size;

    return 1;
}

/* Mark routines */

/*
 * Mark the most recently supplied byte.
 */

void
ring_mark(ring)
Ring *ring;
{
    ring->mark = ring_decrement(ring, ring->supply, 1);
}

/*
 * Is the ring pointing to the mark?
 */

int
ring_at_mark(ring)
Ring *ring;
{
    if (ring->mark == ring->consume) {
	return 1;
    } else {
	return 0;
    }
}

/*
 * Clear any mark set on the ring.
 */

void
ring_clear_mark(ring)
Ring *ring;
{
    ring->mark = 0;
}

/*
 * Add characters from current segment to ring buffer.
 */
void
ring_supplied(ring, count)
Ring *ring;
int count;
{
    ring->supply = ring_increment(ring, ring->supply, count);
    ring->supplytime = ++ring_clock;
}

/*
 * We have just consumed "c" bytes.
 */
void
ring_consumed(ring, count)
Ring *ring;
int count;
{
    if (count == 0)	/* don't update anything */
	return;

    if (ring->mark &&
		(ring_subtract(ring, ring->mark, ring->consume) < count)) {
	ring->mark = 0;
    }
    ring->consume = ring_increment(ring, ring->consume, count);
    ring->consumetime = ++ring_clock;
    /*
     * Try to encourage "ring_empty_consecutive()" to be large.
     */
    if (ring_empty(ring)) {
	ring->consume = ring->supply = ring->bottom;
    }
}



/* Buffer state query routines */


/* Number of bytes that may be supplied */
int
ring_empty_count(ring)
Ring *ring;
{
    if (ring_empty(ring)) {	/* if empty */
	    return ring->size;
    } else {
	return ring_subtract(ring, ring->consume, ring->supply);
    }
}

/* number of CONSECUTIVE bytes that may be supplied */
int
ring_empty_consecutive(ring)
Ring *ring;
{
    if ((ring->consume < ring->supply) || ring_empty(ring)) {
			    /*
			     * if consume is "below" supply, or empty, then
			     * return distance to the top
			     */
	return ring_subtract(ring, ring->top, ring->supply);
    } else {
				    /*
				     * else, return what we may.
				     */
	return ring_subtract(ring, ring->consume, ring->supply);
    }
}

/* Return the number of bytes that are available for consuming
 * (but don't give more than enough to get to cross over set mark)
 */

int
ring_full_count(ring)
Ring *ring;
{
    if ((ring->mark == 0) || (ring->mark == ring->consume)) {
	if (ring_full(ring)) {
	    return ring->size;	/* nothing consumed, but full */
	} else {
	    return ring_subtract(ring, ring->supply, ring->consume);
	}
    } else {
	return ring_subtract(ring, ring->mark, ring->consume);
    }
}

/*
 * Return the number of CONSECUTIVE bytes available for consuming.
 * However, don't return more than enough to cross over set mark.
 */
int
ring_full_consecutive(ring)
Ring *ring;
{
    if ((ring->mark == 0) || (ring->mark == ring->consume)) {
	if ((ring->supply < ring->consume) || ring_full(ring)) {
	    return ring_subtract(ring, ring->top, ring->consume);
	} else {
	    return ring_subtract(ring, ring->supply, ring->consume);
	}
    } else {
	if (ring->mark < ring->consume) {
	    return ring_subtract(ring, ring->top, ring->consume);
	} else {	/* Else, distance to mark */
	    return ring_subtract(ring, ring->mark, ring->consume);
	}
    }
}

/*
 * Move data into the "supply" portion of of the ring buffer.
 */
void
ring_supply_data(ring, buffer, count)
Ring *ring;
char *buffer;
int count;
{
    int i;

    while (count) {
	i = MIN(count, ring_empty_consecutive(ring));
	memcpy(ring->supply, buffer, i);
	ring_supplied(ring, i);
	count -= i;
	buffer += i;
    }
}


#ifdef notdef
/*
 * Move data from the "consume" portion of the ring buffer
 */
void
ring_consume_data(ring, buffer, count)
Ring *ring;
char *buffer;
int count;
{
    int i;

    while (count) {
	i = MIN(count, ring_full_consecutive(ring));
	memcpy(buffer, ring->consume, i);
	ring_consumed(ring, i);
	count -= i;
	buffer += i;
    }
}
#endif
