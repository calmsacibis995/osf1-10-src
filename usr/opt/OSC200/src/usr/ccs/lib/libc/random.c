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
static char	*sccsid = "@(#)$RCSfile: random.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/09/23 18:29:54 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: random.c,v $ $Revision: 4.2.7.3 $ (OSF) $Date: 1993/09/23 18:29:54 $";
#endif
/*
 * FUNCTIONS: random, srandom, setstate, initstate
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 * 
 * random.c	1.4  com/lib/c/gen,3.1,8943 10/16/89 09:21:05
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak initstate_r = __initstate_r
#pragma weak random_r = __random_r
#pragma weak setstate_r = __setstate_r
#pragma weak srandom_r = __srandom_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak initstate = __initstate
#pragma weak random = __random
#pragma weak setstate = __setstate
#pragma weak srandom = __srandom
#endif
#endif
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "ts_supp.h"

/*
 * FUNCTION: random number generation operations
 *
 */

#include "libc_msg.h"
#define MSGSTR(n,s)	catgets(catd,MS_LIBC,n,s)

/*
 * random.c:
 * An improved random number generation package.  In addition to the standard
 * rand()/srand() like interface, this package also has a special state info
 * interface.  The initstate() routine is called with a seed, an array of
 * bytes, and a count of how many bytes are being passed in; this array is then
 * initialized to contain information for random number generation with that
 * much state information.  Good sizes for the amount of state information are
 * 32, 64, 128, and 256 bytes.  The state can be switched by calling the
 * setstate() routine with the same array as was initiallized with initstate().
 * By default, the package runs with 128 bytes of state information and
 * generates far better random numbers than a linear congruential generator.
 * If the amount of state information is less than 32 bytes, a simple linear
 * congruential R.N.G. is used.
 * Internally, the state information is treated as an array of longs; the
 * zeroeth element of the array is the type of R.N.G. being used (small
 * integer); the remainder of the array is the state information for the
 * R.N.G.  Thus, 32 bytes of state information will give 7 longs worth of
 * state information, which will allow a degree seven polynomial.  (Note: the 
 * zeroeth word of state information also has some other information stored
 * in it -- see setstate() for details).
 * The random number generation technique is a linear feedback shift register
 * approach, employing trinomials (since there are fewer terms to sum up that
 * way).  In this approach, the least significant bit of all the numbers in
 * the state table will act as a linear feedback shift register, and will have
 * period 2^deg - 1 (where deg is the degree of the polynomial being used,
 * assuming that the polynomial is irreducible and primitive).  The higher
 * order bits will have longer periods, since their values are also influenced
 * by pseudo-random carries out of the lower bits.  The total period of the
 * generator is approximately deg*(2**deg - 1); thus doubling the amount of
 * state information has a vast influence on the period of the generator.
 * Note: the deg*(2**deg - 1) is an approximation only good for large deg,
 * when the period of the shift register is the dominant factor.  With deg
 * equal to seven, the period is actually much longer than the 7*(2**7 - 1)
 * predicted by this formula.
 */


/*
 * For each of the currently supported random number generators, we have a
 * break value on the amount of state information (you need at least this
 * many bytes of state info to support this random number generator), a degree
 * for the polynomial (actually a trinomial) that the R.N.G. is based on, and
 * the separation between the two lower order coefficients of the trinomial.
 */

#define	TYPE_0		0		/* linear congruential */
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0

#define	TYPE_1		1		/* x**7 + x**3 + 1 */
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

#define	TYPE_2		2		/* x**15 + x + 1 */
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

#define	TYPE_3		3		/* x**31 + x**3 + 1 */
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

#define	TYPE_4		4		/* x**63 + x + 1 */
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1


/*
 * Array versions of the above information to make code run faster -- relies
 * on fact that TYPE_i == i.
 */

#define	MAX_TYPES	5		/* max number of types above */

static int	degrees[ MAX_TYPES ]	= { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 };

static int	seps[ MAX_TYPES ]	= { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 };


#ifdef _THREAD_SAFE

#define	RANDOM()	random_r(&junk, rand_data)
#define	SRANDOM(s)	srandom_r(s, rand_data)

#define	RETURN(x)	return (*retval = x, 0)

#define	FPTR		(rand_data->fptr)
#define	RPTR		(rand_data->rptr)
#define	STATE		(rand_data->state)
#define	RAND_TYPE	(rand_data->rand_type)
#define	RAND_DEG	(rand_data->rand_deg)
#define	RAND_SEP	(rand_data->rand_sep)
#define	END_PTR		(rand_data->end_ptr)

#else	/* _THREAD_SAFE */

#define	RANDOM()	random()
#define	SRANDOM(s)	srandom(s)

#define	RETURN(x)	return (x)

#define	FPTR		fptr
#define	RPTR		rptr
#define	STATE		state
#define	RAND_TYPE	rand_type
#define	RAND_DEG	rand_deg
#define	RAND_SEP	rand_sep
#define	END_PTR		end_ptr

/*
 * Initially, everything is set up as if from :
 *		initstate( 1, &randtbl, 128 );
 * Note that this initialization takes advantage of the fact that srandom()
 * advances the front and rear pointers 10*rand_deg times, and hence the
 * rear pointer which starts at 0 will also end up at zero; thus the zeroeth
 * element of the state information, which contains info about the current
 * position of the rear pointer is just
 *	MAX_TYPES*(rptr - state) + TYPE_3 == TYPE_3.
 */

static int		randtbl[ DEG_3 + 1 ]	= { TYPE_3,
				0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342, 
				0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb, 
				0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd, 
				0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86, 
				0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7, 
				0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc, 
				0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b, 
					0xf5ad9d0e, 0x8999220b, 0x27fb47b9 };

/*
 * fptr and rptr are two pointers into the state info, a front and a rear
 * pointer.  These two pointers are always rand_sep places aparts, as they cycle
 * cyclically through the state information.  (Yes, this does mean we could get
 * away with just one pointer, but the code for random() is more efficient this
 * way).  The pointers are left positioned as they would be from the call
 *			initstate( 1, randtbl, 128 )
 * (The position of the rear pointer, rptr, is really 0 (as explained above
 * in the initialization of randtbl) because the state table pointer is set
 * to point to randtbl[1] (as explained below).
 */

static int		*fptr			= &randtbl[ SEP_3 + 1 ];
static int		*rptr			= &randtbl[ 1 ];


/*
 * The following things are the pointer to the state information table,
 * the type of the current generator, the degree of the current polynomial
 * being used, and the separation between the two pointers.
 * Note that for efficiency of random(), we remember the first location of
 * the state information, not the zeroeth.  Hence it is valid to access
 * state[-1], which is used to store the type of the R.N.G.
 * Also, we remember the last location, since this is more efficient than
 * indexing every time to find the address of the last element to see if
 * the front and rear pointers have wrapped.
 */

static int		*state			= &randtbl[ 1 ];

static int		rand_type		= TYPE_3;
static int		rand_deg		= DEG_3;
static int		rand_sep		= SEP_3;

static int		*end_ptr		= &randtbl[ DEG_3 + 1 ];

#endif	/* _THREAD_SAFE */


/*
 * srandom:
 * Initialize the random number generator based on the given seed.  If the
 * type is the trivial no-state-information type, just remember the seed.
 * Otherwise, initializes state[] based on the given "seed" via a linear
 * congruential generator.  Then, the pointers are set to known locations
 * that are exactly rand_sep places apart.  Lastly, it cycles the state
 * information a given number of times to get rid of any initial dependencies
 * introduced by the L.C.R.N.G.
 * Note that the initialization of randtbl[] for default usage relies on
 * values produced by this routine.
 */

#ifdef _THREAD_SAFE
int
srandom_r(unsigned x, struct random_data *rand_data)
{
	int	junk;
#else
int
srandom(unsigned x)
{
#endif	/* _THREAD_SAFE */

	register int	i;

	TS_EINVAL((rand_data == 0) || (rand_data->state == 0));

	if (RAND_TYPE == TYPE_0) {
		STATE[0] = x;
	} else {
		STATE[0] = x;
		for ( i = 1; i < RAND_DEG; i++ ) {
			STATE[i] = 1103515245*STATE[i - 1] + 12345;
		}
		FPTR = &STATE[RAND_SEP];
		RPTR = &STATE[0];
		for ( i = 0; i < 10*RAND_DEG; i++ )
			RANDOM();
	}
	return (0);
}


/*
 * initstate:
 * Initialize the state information in the given array of n bytes for
 * future random number generation.  Based on the number of bytes we
 * are given, and the break values for the different R.N.G.'s, we choose
 * the best (largest) one we can and set things up for it.  srandom() is
 * then called to initialize the state information.
 * Note that on return from srandom(), we set state[-1] to be the type
 * multiplexed with the current value of the rear pointer; this is so
 * successive calls to initstate() won't lose this information and will
 * be able to restart with setstate().
 * Note: the first thing we do is save the current state, if any, just like
 * setstate() so that it doesn't matter when initstate is called.
 * Returns a pointer to the old state.
 */

#ifdef _THREAD_SAFE
int
initstate_r(unsigned seed, char *arg_state, int n,
		char **retval, struct random_data *rand_data)
#else
char *
initstate(
	unsigned	seed,		/* seed for R. N. G. */
	char		*arg_state,	/* pointer to state array */
	int		n)		/* # bytes of state info */
#endif	/* _THREAD_SAFE */
{
	register char	*ostate;
	nl_catd		catd;

	TS_EINVAL((arg_state == 0) || (retval == 0) || (rand_data == 0));

#ifdef _THREAD_SAFE
	if (!STATE)
		ostate = NULL;
	else
#endif	/* _THREAD_SAFE */

	{
		ostate = (char *)(&STATE[ -1 ]);

		if (RAND_TYPE == TYPE_0)
			STATE[ -1 ] = RAND_TYPE;
		else
			STATE[ -1 ] = MAX_TYPES*(RPTR - STATE) + RAND_TYPE;
	}
	if (n < BREAK_1) {
		if (n < BREAK_0) {
			catd = catopen( MF_LIBC, NL_CAT_LOCALE);
			fprintf(stderr, MSGSTR(M_INITST, 
				"initstate: not enough state (%d bytes)" \
				" with which to do operation; ignored.\n"), n);
			catclose( catd );
			return (TS_FAILURE);
		}
		RAND_TYPE = TYPE_0;
		RAND_DEG = DEG_0;
		RAND_SEP = SEP_0;

	} else if (n < BREAK_2) {
		RAND_TYPE = TYPE_1;
		RAND_DEG = DEG_1;
		RAND_SEP = SEP_1;

	} else if (n < BREAK_3) {
		RAND_TYPE = TYPE_2;
		RAND_DEG = DEG_2;
		RAND_SEP = SEP_2;

	} else if (n < BREAK_4) {
		RAND_TYPE = TYPE_3;
		RAND_DEG = DEG_3;
		RAND_SEP = SEP_3;

	} else {
		RAND_TYPE = TYPE_4;
		RAND_DEG = DEG_4;
		RAND_SEP = SEP_4;
	}
	STATE = &(( (int *)arg_state )[1]);	/* first location */
	END_PTR = &STATE[ RAND_DEG ];	/* must set end_ptr before srandom */
	SRANDOM(seed);
	if (RAND_TYPE == TYPE_0)
		STATE[ -1 ] = RAND_TYPE;
	else
		STATE[ -1 ] = MAX_TYPES*(RPTR - STATE) + RAND_TYPE;

	RETURN (ostate);
}



/*
 * setstate:
 * Restore the state from the given state array.
 * Note: it is important that we also remember the locations of the pointers
 * in the current state information, and restore the locations of the pointers
 * from the old state information.  This is done by multiplexing the pointer
 * location into the zeroeth word of the state information.
 * Note that due to the order in which things are done, it is OK to call
 * setstate() with the same state as the current state.
 * Returns a pointer to the old state information.
 */

#ifdef _THREAD_SAFE
int
setstate_r(char *arg_state,
		char **retval, struct random_data *rand_data)
#else
char *
setstate(char *arg_state)
#endif	/* _THREAD_SAFE */
{
	register int		*new_state	= (int *)arg_state;
	register int		type		= new_state[0]%MAX_TYPES;
	register int		rear		= new_state[0]/MAX_TYPES;
	char			*ostate;

	TS_EINVAL((arg_state == 0) || (retval == 0) || (rand_data == 0));

#ifdef _THREAD_SAFE
	if (!STATE)
		ostate = NULL;
	else {
#endif	/* _THREAD_SAFE */
		ostate = (char *)( &STATE[ -1 ] );

		if (RAND_TYPE == TYPE_0)
			STATE[ -1 ] = RAND_TYPE;
		else
			STATE[ -1 ] = MAX_TYPES*(RPTR - STATE) + RAND_TYPE;
#ifdef _THREAD_SAFE
	}
#endif	/* _THREAD_SAFE */
	switch (type) {
	case TYPE_0:
	case TYPE_1:
	case TYPE_2:
	case TYPE_3:
	case TYPE_4:
		RAND_TYPE = type;
		RAND_DEG = degrees[ type ];
		RAND_SEP = seps[ type ];
		break;

	default:
		{ nl_catd catd;
		catd = catopen( MF_LIBC, NL_CAT_LOCALE);
		fprintf( stderr, MSGSTR(M_SETST,
				"setstate: state info has been damaged;" \
				" not changed.\n") );
		catclose( catd );
		return (TS_FAILURE);
		}
	}
	STATE = &new_state[ 1 ];
	if (RAND_TYPE != TYPE_0) {
		RPTR = &STATE[ rear ];
		FPTR = &STATE[ (rear + RAND_SEP)%RAND_DEG ];
	}
	END_PTR = &STATE[ RAND_DEG ];		/* set end_ptr too */
	RETURN (ostate);
}



/*
 * random:
 * If we are using the trivial TYPE_0 R.N.G., just do the old linear
 * congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
 * same in all ther other cases due to all the global variables that have been
 * set up.  The basic operation is to add the number at the rear pointer into
 * the one at the front pointer.  Then both pointers are advanced to the next
 * location cyclically in the table.  The value returned is the sum generated,
 * reduced to 31 bits by throwing away the "least random" low bit.
 * Note: the code takes advantage of the fact that both the front and
 * rear pointers can't wrap on the same call by not testing the rear
 * pointer if the front one has wrapped.
 * Returns a 31-bit random number.
 */

#ifdef _THREAD_SAFE
int
random_r(int *retval, struct random_data *rand_data)
#else
int
random(void)
#endif	/* _THREAD_SAFE */
{
	int		i;
	
	TS_EINVAL((retval == 0) || (rand_data == 0));

	if (RAND_TYPE == TYPE_0) {
		i = STATE[0] = ( STATE[0]*1103515245 + 12345 )&0x7fffffff;
	} else {
		*FPTR += *RPTR;
		i = (*FPTR >> 1)&0x7fffffff;	/* chucking least random bit */
		if (++FPTR >= END_PTR) {
			FPTR = STATE;
			++RPTR;
		} else {
			if (++RPTR >= END_PTR)
				RPTR = STATE;
		}
	}

	RETURN (i);
}
