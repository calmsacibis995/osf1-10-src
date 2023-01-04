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
static char     *sccsid = "@(#)$RCSfile: ranny.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:39:51 $";
#endif
/*
 */

/*
 * Random number generator is:
 *
 *	Copyright 1988 by Rayan S. Zachariassen, all rights reserved.
 *	This will be free software, but only when it is finished.
 *
 * Used in xntp by permission of the author.  If copyright is
 * annoying to you, read no further.  Instead, look up the reference,
 * write me an equivalent to this and send it back to me.
 */

/*
 * Random number generator; see Knuth Vol 2. 2nd ed. p.27 (section 3.2.2)
 */
#include <sys/types.h>

/*
 * 55 random numbers, not all even.  Note we don't initialize ran_y
 * directly since I have had thoughts of putting this in an EPROM
 */
static u_int ran_y[55];

static u_int init_ran_y[55] = {
	1860909544, 231033423, 437666411, 1349655137, 2014584962,
	504613712, 656256107, 1246027206, 573713775, 643466871,
	540235388, 1630565153, 443649364, 729302839, 1933991552,
	944681982, 949111118, 406212522, 1065063137, 1712954727,
	73280612, 787623973, 1874130997, 801658492, 73395958,
	739165367, 596047144, 490055249, 1131094323, 662727104,
	483614097, 844520219, 893760527, 921280508, 46691708,
	760861842, 1425894220, 702947816, 2006889048, 1999607995,
	1346414687, 399640789, 1482689501, 1790064052, 1128943628,
	1269197405, 587262386, 2078054746, 1675409928, 1652325524,
	1643525825, 1748690540, 292465849, 1370173174, 402865384
};

static int ran_j;
static int ran_k;


/*
 * ranp2 - return a random integer in the range 0 .. (1<<m)-1
 */
u_int
ranp2(m)
	int m;
{
	u_int r;

	ran_y[ran_k] += ran_y[ran_j];	/* overflow does a mod */
	r = ran_y[ran_k];
	if (ran_k-- == 0) ran_k = 54;
	if (ran_j-- == 0) ran_j = 54;
	return (r & ((1<<m)-1));
}

#ifdef notdef
/*
 * ranny - return a random integer in the range 0 .. m-1
 */
u_int
ranny(m)
	u_int m;
{
	unsigned int r;

	ran_y[ran_k] += ran_y[ran_j];	/* overflow does a mod */
	r = ran_y[ran_k];
	if (ran_k-- == 0) ran_k = 54;
	if (ran_j-- == 0) ran_j = 54;
	return (r % m);
}
#endif /* notdef */

/*
 * init_random - do initialization of random number routine
 */
void
init_random()
{
	register int i;
	register time_t now;

	ran_j = 23;
	ran_k = 54;

	/*
	 * Randomize the seed array some more.  The time of day
	 * should be initialized by now.
	 */
	now = (time_t)(time((time_t *)0))|01;

	for (i = 0; i < 55; ++i)
		ran_y[i] = now * init_ran_y[i];	/* overflow does a mod */
}
