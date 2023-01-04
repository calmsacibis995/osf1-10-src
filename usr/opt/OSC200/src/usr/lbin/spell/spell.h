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
 *	@(#)$RCSfile: spell.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/03 10:15:56 $
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


/*	spell.h	4.1	12/18/82	*/

#include <stdio.h>
#include <ctype.h>

#ifndef unix
#define SHIFT	5
#define TABSIZE (int)(400000/(1<<SHIFT))
int	*tab;	/*honeywell loader deficiency*/
#else
#define Tolower(c)	(isupper(c)?tolower(c):c) /* ugh!!! */
#define SHIFT	4
#define TABSIZE 25000	/*(int)(400000/(1<<shift))--pdp11 compiler deficiency*/
short	tab[TABSIZE];
#endif
int	p[] = {
	399871,
	399887,
	399899,
	399911,
	399913,
	399937,
	399941,
	399953,
	399979,
	399983,
	399989,
};
#define	NP	(sizeof(p)/sizeof(p[0]))
#define	NW	30

/*
* Hash table for spelling checker has n bits.
* Each word w is hashed by k different (modular) hash functions, hi.
* The bits hi(w), i=1..k, are set for words in the dictionary.
* Assuming independence, the probability that no word of a d-word
* dictionary sets a particular bit is given by the Poisson formula
* P = exp(-y)*y**0/0!, where y=d*k/n.
* The probability that a random string is recognized as a word is then
* (1-P)**k.  For given n and d this is minimum when y=log(2), P=1/2,
* whence one finds, for example, that a 25000-word dictionary in a
* 400000-bit table works best with k=11.
*/

int	pow2[NP][NW];

prime(argc, argv) register char **argv;
{
	int i, j;
	int h;
	register int *lp;

#ifndef unix
	if ((tab = (int *)calloc(sizeof(*tab), TABSIZE)) == NULL)
		return(0);
#endif
	if (argc > 1) {
		FILE *f;
		if ((f = fopen(argv[1], "ri")) == NULL)
			return(0);
		if (fread((char *)tab, sizeof(*tab), TABSIZE, f) != TABSIZE)
			return(0);
		fclose(f);
	}
	for (i=0; i<NP; i++) {
		h = *(lp = pow2[i]) = 1<<14;
		for (j=1; j<NW; j++)
			h = *++lp = (h<<7) % p[i];
	}
	return(1);
}

#define get(h)	(tab[h>>SHIFT]&(1<<((int)h&((1<<SHIFT)-1))))
#define set(h)	tab[h>>SHIFT] |= 1<<((int)h&((1<<SHIFT)-1))
