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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: hist.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/06/10 16:47:12 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: savehist enthist hfree dohist dohist1 phist
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 *
 *	1.5  com/cmd/csh/hist.c, bos320 5/10/91 15:37:55";
 */ 

#include "sh.h"

void
savehist(struct wordent *sp)
{
	register struct Hist *hp, *np;
	int histlen;
	register uchar_t *cp;

	/* throw away null lines */
	if (sp->next->word[0] == '\n')
		return;

	cp = value((uchar_t *)"history");
	if (*cp == 0)
		histlen = 0;
	else {
		while (*cp && digit(*cp))
			cp++;
		if (*cp)
			set((uchar_t *)"history", (uchar_t *)"10");
		histlen = (int)getn(value((uchar_t *)"history"));   /* 001 */
	}
	for (hp = &Histlist; np = hp->Hnext;)
		if (eventno - np->Href >= histlen || histlen == 0)
			hp->Hnext = np->Hnext, hfree(np);
		else
			hp = np;
	enthist(++eventno, sp, 1);
}

struct Hist *
enthist(int event, register struct wordent *lp, bool docopy)
{
	register struct Hist *np;

	np = (struct Hist *)calloc(1, sizeof *np);
	np->Hnum = np->Href = event;
	if (docopy)
		copylex(&np->Hlex, lp);
	else {
		np->Hlex.next = lp->next;
		lp->next->prev = &np->Hlex;
		np->Hlex.prev = lp->prev;
		lp->prev->next = &np->Hlex;
	}
	np->Hnext = Histlist.Hnext;
	Histlist.Hnext = np;
	return (np);
}

void
hfree(register struct Hist *hp)
{

	freelex(&hp->Hlex);
	xfree((uchar_t *)hp);
}

void
dohist(uchar_t **vp)
{
	int n, rflg = 0, hflg = 0;
	if ((int)getn(value((uchar_t *)"history")) == 0)        /* 001 */
		return;
	if (setintr)
		sigrelse(SIGINT);
	vp++;
	while (*vp && *vp[0] == '-') {
		if (*vp && EQ(*vp, "-h")) 
			hflg++;
		else if (*vp && EQ(*vp, "-r")) 
			rflg++;
		else {
			flush ();
			haderr = 1;
			csh_printf (MSGSTR(M_OPTION,"Unknown option : %s \n"), *vp);
			error(NULL);
		}
		vp++;
	}
	if (*vp)
		n = (int)getn(*vp);                              /* 001 */
	else {
		n = (int)getn(value((uchar_t *)"history"));      /* 001 */
	}
	dohist1(Histlist.Hnext, &n, rflg, hflg);
}

void
dohist1(struct Hist *hp, int *np, int rflg, int hflg)
{
	bool print = (*np) > 0;
top:
	if (hp == 0)
		return;
	(*np)--;
	hp->Href++;
	if (rflg == 0) {
		dohist1(hp->Hnext, np, rflg, hflg);
		if (print)
			phist(hp, hflg);
		return;
	}
	if (*np >= 0)
		phist(hp, hflg);
	hp = hp->Hnext;
	goto top;
}

void
phist(register struct Hist *hp, int hflg)
{

	if (hflg == 0)
		csh_printf("%6d\t", hp->Hnum);
	prlex(&hp->Hlex);
}
