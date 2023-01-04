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
static char rcsid[] = "@(#)$RCSfile: time.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/10 17:24:27 $";
#endif
/*
 * HISTORY
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: settimes dotime ptimes
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 */ 

#include <langinfo.h>
#include "sh.h"
#undef NOSTR

#define MICRO	1000000L
#define HEC	100L
#define HS 	(long)(MICRO/HEC)


void
settimes(void)
{
	struct rusage ruch;

	(void)gettimeofday(&time0, (struct timezone *)0);
	(void) getrusage(RUSAGE_SELF, &ru0);
	(void) getrusage(RUSAGE_CHILDREN, &ruch);
	ruadd(&ru0, &ruch);
}

/*
 * dotime is only called if it is truly a builtin function and not a
 * prefix to another command
 */
void
dotime(void)
{
	struct timeval timedol;
	struct rusage ru1, ruch;

	(void) getrusage(RUSAGE_SELF, &ru1);
	(void) getrusage(RUSAGE_CHILDREN, &ruch);
	ruadd(&ru1, &ruch);
	(void)gettimeofday(&timedol,(struct timezone *)0);
	prusage(&ru0, &ru1, &timedol, &time0);
}

void
ruadd(register struct rusage *ru, register struct rusage *ru2)
{
	register long *lp, *lp2;
	register int cnt;

	tvadd(&ru->ru_utime, &ru2->ru_utime);
	tvadd(&ru->ru_stime, &ru2->ru_stime);
	if (ru2->ru_maxrss > ru->ru_maxrss)
		ru->ru_maxrss = ru2->ru_maxrss;
	cnt = &ru->ru_last - &ru->ru_first + 1;
	lp = &ru->ru_first; lp2 = &ru2->ru_first;
	do
		*lp++ += *lp2++;
	while (--cnt > 0);
}

void
prusage(register struct rusage *r0,
	register struct rusage *r1,
	struct timeval *e,
	struct timeval *b)
{
	register time_t t =
	    (r1->ru_utime.tv_sec-r0->ru_utime.tv_sec)*HEC+
	    (r1->ru_utime.tv_usec-r0->ru_utime.tv_usec)/HS+
	    (r1->ru_stime.tv_sec-r0->ru_stime.tv_sec)*HEC+
	    (r1->ru_stime.tv_usec-r0->ru_stime.tv_usec)/HS;
	register uchar_t *cp;
	register int i;
	register struct varent *vp = adrof((uchar_t *)"time");
	time_t ms = 
	    (e->tv_sec-b->tv_sec)*HEC + (e->tv_usec-b->tv_usec)/HS;

	cp = (uchar_t *)"%Uu %Ss %E %P %X+%Dk %I+%Oio %Fpf+%Ww";
	if (vp && vp->vec[0] && vp->vec[1])
		cp = vp->vec[1];
	for (; *cp; cp++) {
		if (*cp != '%') {
			display_char(*cp);
		}
		else if (cp[1]) switch(*++cp) {

		case 'U':
			pdeltat(&r1->ru_utime, &r0->ru_utime);
			break;

		case 'S':
			pdeltat(&r1->ru_stime, &r0->ru_stime);
			break;

		case 'E':
			psecs((long)(ms / HEC));
			break;

		case 'P':
			csh_printf("%d%%", (int) (t*HEC / ((ms ? ms : 1))));
			break;

		case 'W':
			i = r1->ru_nswap - r0->ru_nswap;
			csh_printf("%d", i);
			break;

		case 'X':
			csh_printf("%d", 
				t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
			break;

		case 'D':
			csh_printf("%d", t == 0 ? 0 :
			    (r1->ru_idrss+r1->
				ru_isrss-(r0->ru_idrss+r0->ru_isrss))/t);
			break;

		case 'K':
			csh_printf("%d", t == 0 ? 0 :
			((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
			    (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
			break;

		case 'M':
			csh_printf("%d", r1->ru_maxrss);
			break;

		case 'F':
			csh_printf("%d", r1->ru_majflt-r0->ru_majflt);
			break;

		case 'R':
			csh_printf("%d", r1->ru_minflt-r0->ru_minflt);
			break;

		case 'I':
			csh_printf("%d", r1->ru_inblock-r0->ru_inblock);
			break;

		case 'O':
			csh_printf("%d", r1->ru_oublock-r0->ru_oublock);
			break;
		}
	}
	csh_printf("\n");
}

void
pdeltat(struct timeval *t1, struct timeval *t0)
{
	struct timeval td;
	char *radix = nl_langinfo(RADIXCHAR);

	tvsub(&td, t1, t0);

	csh_printf("%d%s%02d", td.tv_sec, (*radix)?radix:".", td.tv_usec/HS);
}

void
tvadd(struct timeval *tsum, struct timeval *t0)
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > MICRO)
		tsum->tv_sec++, tsum->tv_usec -= MICRO;
}

void
tvsub(struct timeval *tdiff, struct timeval *t1, struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += MICRO;
}
