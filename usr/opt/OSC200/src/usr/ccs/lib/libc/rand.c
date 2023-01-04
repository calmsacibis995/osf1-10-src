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
static char	*sccsid = "@(#)$RCSfile: rand.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 21:21:48 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: rand.c,v $ $Revision: 4.2.5.2 $ (OSF) $Date: 1993/06/07 21:21:48 $";
#endif
/*
 * FUNCTIONS: rand, srand
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any
 * actual or intended publication of such source code.
 *
 * rand.c	1.11  com/lib/c/gen,3.1,8943 9/9/89 09:27:25
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak rand_r = __rand_r
#endif
#endif
#include <stdlib.h>
#include <errno.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _rand_rmutex;
#endif	/* _THREAD_SAFE */

#define	MAGIC(x)	(((x = x * 1103515245L + 12345)>>16) & RAND_MAX)

static long randx = 1;

/*
 * NAME:	srand
 *
 * FUNCTION:	The srand function uses the arguement as a seed for a new
 *		sequence of pseudo-random numbers to be returned by 
 *		subsequent calls to rand.
 *
 * RETURN VALUE DESCRIPTION:	
 *		returns no value
 *
 */
void	
srand(unsigned int seed)
{
	TS_LOCK(&_rand_rmutex);

	randx = seed;

	TS_UNLOCK(&_rand_rmutex);
}


/*
 * NAME:	rand
 *
 * FUNCTION:	The rand function computes a sequence of pseudo-random
 *		integers in the range 0 to RAND_MAX.
 *
 * RETURN VALUE DESCRIPTION:	
 *		returns a psuedo-random integer
 *
 */
int 
rand(void)
{
	int	retval;

	TS_LOCK(&_rand_rmutex);

	retval = MAGIC(randx);

	TS_UNLOCK(&_rand_rmutex);
	return (retval);
}


#ifdef _THREAD_SAFE
int
rand_r(unsigned int *seed, int *randval)
{
	if ((randval == NULL) || (seed == NULL)) {
		_Seterrno(EINVAL);
		return(-1);
	}
	*randval = MAGIC(*seed);

	return (0);
}
#endif	/* _THREAD_SAFE */
