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
static char	*sccsid = "@(#)$RCSfile: qsort.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 21:21:12 $";
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
 * FUNCTIONS: qsort
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
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
 * qsort.c	1.12  com/lib/c/gen,3.1,8943 10/16/89 09:20:05
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak qsort = __qsort
#endif
#endif
#include <stdlib.h>			/* for size_t */

/*
 * FUNCTION:	Qsort sorts a table using the "quicker-sort" algorithm.
 *                                                                    
 * NOTES:	'Base' is the base of the table, 'nmemb' is the number
 *		of elements in it, 'size' is the size of an element and
 *		'compar' is the comparision function to call to compare
 *		2 elements of the table.  'Compar' is called with 2
 *		arguments, each a pointer to an element to be compared.
 *		It must return:
 *			< 0	if the first argument is less than the second
 *			> 0	if the first argument is greater than the second
 *			= 0	if the first argument is equal to the second
 *
 *
 */  

struct qdata {
	size_t	qses;				     /* element size */
	int	(*qscmp)(const void *, const void *); /* comparison function */
};

static void qstexc( char *i, char *j, char *k, struct qdata *qd);
static void qsexc( char *i, char *j, struct qdata *qd);
static void qs1( char *a, char *l, struct qdata *qd);


void 
qsort(void *base, size_t nmemb, size_t size,
	 int(*compar)(const void *, const void *))
{
	struct qdata	qsdata;

	qsdata.qscmp = compar;	/* save off the comparison function	*/
	qsdata.qses = size;	/* save off the element size		*/

	qs1((char *)base, (char *)base + nmemb * size, &qsdata);
}

static void
qs1( char *a, char *l, struct qdata *qsd)
{
	char *i, *j;
	size_t es;
	char	*lp, *hp;
	int	c;
	unsigned n;

	es = qsd->qses;
start:
	if((n=l-a) <= es)
		return;
	n = es * (n / (2*es));
	hp = lp = a+n;
	i = a;
	j = l-es;
	while(1) {
		if(i < lp) {
			if((c = (*(qsd->qscmp))((void*)i, (void*)lp)) == 0) {
				qsexc(i, lp -= es, qsd);
				continue;
			}
			if(c < 0) {
				i += es;
				continue;
			}
		}

loop:
		if(j > hp) {
			if((c = (*(qsd->qscmp))((void*)hp, (void*)j)) == 0) {
				qsexc(hp += es, j, qsd);
				goto loop;
			}
			if(c > 0) {
				if(i == lp) {
					qstexc(i, hp += es, j, qsd);
					i = lp += es;
					goto loop;
				}
				qsexc(i, j, qsd);
				j -= es;
				i += es;
				continue;
			}
			j -= es;
			goto loop;
		}

		if(i == lp) {
			if(lp-a >= l-hp) {
				qs1(hp+es, l, qsd);
				l = lp;
			} else {
				qs1(a, lp, qsd);
				a = hp+es;
			}
			goto start;
		}

		qstexc(j, lp -= es, i, qsd);
		j = hp -= es;
	}
}

static void
qsexc( char *i, char *j, struct qdata *qsd)
{
	char *ri, *rj, c;
	size_t n;

	n = qsd->qses;
	ri = i;
	rj = j;
	do {
		c = *ri;
		*ri++ = *rj;
		*rj++ = c;
	} while(--n);
}

static void
qstexc( char *i, char *j, char *k, struct qdata *qsd)
{
	char *ri, *rj, *rk;
	int c;
	size_t n;

	n = qsd->qses;
	ri = i;
	rj = j;
	rk = k;
	do {
		c = *ri;
		*ri++ = *rk;
		*rk++ = *rj;
		*rj++ = c;
	} while(--n);
}
