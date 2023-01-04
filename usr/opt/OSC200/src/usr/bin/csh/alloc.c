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
static char	*sccsid = "@(#)$RCSfile: alloc.c,v $ $Revision: 4.2.4.7 $ (DEC) $Date: 1993/01/08 16:00:32 $";
#endif 

/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS:
 *
 * ORIGINS: 10,26,27
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
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

#include <sys/types.h>
#include "local.h"

#ifdef debug
#define ASSERT(p) if(!(p))botch("p");else
static void
botch(uchar_t *s)
{
	csh_printf("assertion botched: %s\n",s);
	abort();
}
#else
#define ASSERT(p)
#endif

/*	avoid break bug */
#define GRANULE 0
/*	C storage allocator
 *	circular first-fit strategy
 *	works with noncontiguous, but monotonically linked, arena
 *	each block is preceded by a ptr to the (pointer of) 
 *	the next following block
 *	blocks are exact number of words long 
 *	aligned to the data type requirements of ALIGN
 *	pointers to blocks must have BUSY bit 0
 *	bit in ptr is 1 for busy, 0 for idle
 *	gaps in arena are merely noted as busy blocks
 *	last block of arena (pointed to by alloct) is empty and
 *	has a pointer to first
 *	idle blocks are coalesced during space search
 *
 *	a different implementation may need to redefine
 *	ALIGN, NALIGN, BLOCK, BUSY, INT
 *	where INT is integer type to which a pointer can be cast
*/
#define INT long
#define ALIGN long
#define NALIGN 1
#define WORD sizeof(union store)
#define BLOCK 1024	/* a multiple of WORD*/
#define BUSY 1
#ifndef NULL
#define NULL (void *)0
#endif
#define testbusy(p) ((INT)(p)&BUSY)
#define setbusy(p) (union store *)((INT)(p)|BUSY)
#define clearbusy(p) (union store *)((INT)(p)&~BUSY)

union store { union store *ptr;
	      ALIGN dummy[NALIGN];
	      int calloc;	/*calloc clears an array of integers*/
};

static	union store allocs[2];	/*initial arena*/
static	union store *allocp;	/*search ptr*/
static	union store *alloct;	/*arena top*/
static	union store *allocx;	/*for benefit of realloc*/
uchar_t	*sbrk();

uchar_t *
malloc(unsigned nbytes)
{
	register union store *p, *q;
	register int nw;
	static int temp;	/*coroutines assume no auto*/

	if(allocs[0].ptr==0) {	/*first time*/
		allocs[0].ptr = setbusy(&allocs[1]);
		allocs[1].ptr = setbusy(&allocs[0]);
		alloct = &allocs[1];
		allocp = &allocs[0];
	}
	nw = (nbytes+WORD+WORD-1)/WORD;
	ASSERT(allocp>=allocs && allocp<=alloct);
	ASSERT(allock());
	for(p=allocp; ; ) {
		for(temp=0; ; ) {
			if(!testbusy(p->ptr)) {
				while(!testbusy((q=p->ptr)->ptr)) {
					ASSERT(q>p&&q<alloct);
					p->ptr = q->ptr;
				}
				if(q>=p+nw && p+nw>=p)
					goto found;
			}
			q = p;
			p = clearbusy(p->ptr);
			if(p>q)
				ASSERT(p<=alloct);
			else if(q!=alloct || p!=allocs) {
				ASSERT(q==alloct&&p==allocs);
				return(NULL);
			} else if(++temp>1)
				break;
		}
		temp = ((nw+BLOCK/WORD)/(BLOCK/WORD))*(BLOCK/WORD);
		q = (union store *)sbrk(0);
		if(q+temp+GRANULE < q) {
			return(NULL);
		}
		q = (union store *)sbrk(temp*WORD);
		if((INT)q == -1) {
			return(NULL);
		}
		ASSERT(q>alloct);
		alloct->ptr = q;
		if(q!=alloct+1)
			alloct->ptr = setbusy(alloct->ptr);
		alloct = q->ptr = q+temp-1;
		alloct->ptr = setbusy(allocs);
	}
found:
	allocp = p + nw;
	ASSERT(allocp<=alloct);
	if(q>allocp) {
		allocx = allocp->ptr;
		allocp->ptr = p->ptr;
	}
	p->ptr = setbusy(allocp);
	return((uchar_t *)(p+1));
}

/*	freeing strategy tuned for LIFO allocation
*/
void
free(register uchar_t *ap)
{
	register union store *p = (union store *)ap;

	if (ap == (uchar_t *) 0)
		return;

	ASSERT(p>clearbusy(allocs[1].ptr)&&p<=alloct);
	ASSERT(allock());
	allocp = --p;
/* 	ASSERT(testbusy(p->ptr)); */
	p->ptr = clearbusy(p->ptr);
	ASSERT(p->ptr > allocp && p->ptr <= alloct);
}

/*	realloc(p, nbytes) reallocates a block obtained from malloc()
 *	and freed since last call of malloc()
 *	to have new size nbytes, and old content
 *	returns new location, or 0 on failure
*/

uchar_t *
realloc(register union store *p, unsigned nbytes)
{
	register union store *q;
	union store *s, *t;
	register unsigned nw;
	unsigned onw;

	if(testbusy(p[-1].ptr))
		free((uchar_t *)p);
	onw = p[-1].ptr - p;
	q = (union store *)malloc(nbytes);
	if(q==NULL || q==p)
		return((uchar_t *)q);
	s = p;
	t = q;
	nw = (nbytes+WORD-1)/WORD;
	if(nw<onw)
		onw = nw;
	while(onw--!=0)
#ifdef	V6
		copy(t++, s++, sizeof (*t));
#else
		*t++ = *s++;
#endif
	if(q<p && q+nw>=p)
		(q+(q+nw-p))->ptr = allocx;
	return((uchar_t *)q);
}

#ifdef debug
allock()
{
#ifdef longdebug
	register union store *p;
	int x;
	x = 0;
	for(p= &allocs[0]; clearbusy(p->ptr) > p; p=clearbusy(p->ptr)) {
		if(p==allocp)
			x++;
	}
	ASSERT(p==alloct);
	return(x==1|p==allocp);
#else
	return(1);
#endif
}
#endif

void
showall(uchar_t **v)
{
	register union store *p, *q;
	int used = 0, free = 0, i;

	for (p = clearbusy(allocs[1].ptr); p != alloct; p = q) {
	    q = clearbusy(p->ptr);
	    if (v[1]) {
		csh_printf("%lx %08x %s\n", p,
		    ((unsigned long) q - (unsigned long) p),
		    testbusy(p->ptr) ? "BUSY" : "FREE");
	    }
	    i = ((unsigned long) q - (unsigned long) p);
	    if (testbusy(p->ptr)) used += i; else free += i;
	}
	csh_printf("%d used, %d free, %#lx end\n", used, free, clearbusy(alloct));
}
