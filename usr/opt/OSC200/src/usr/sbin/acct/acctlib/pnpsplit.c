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
static char	*sccsid = "@(#)$RCSfile: pnpsplit.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 02:36:38 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * pnpsplit splits interval into prime & nonprime portions
 * ONLY ROUTINE THAT KNOWS ABOUT HOLIDAYS AND DEFN OF PRIME/NONPRIME
 */
#include "acctdef.h"
#include <stdio.h>
#include <time.h>

#ifdef MSG
#include	"acct_msg.h"
#define	MSGSTR(Num, Str)	NLgetamsg(MF_ACCT, MS_ACCT, Num, Str)
#else
#define MSGSTR(Num, Str)	Str
#endif	

#define	NHOLIDAYS	50	/* max number of company holidays per year */

/* validate that hours and minutes of prime/non-prime read in
 * from holidays file fall within proper boundaries.
 * Time is expected in the form and range of 0000-2359.
 */
#define	okay(time)	((time/100>=0) && (time/100<=24) \
				 && (time%100>=0) && (time%100<60))

static int	thisyear = 1970;	/* this is changed by holidays file */
static int	holidays[NHOLIDAYS];	/* holidays file day-of-year table */
static char	holfile[] = "/usr/sbin/acct/holidays\0";

/*
 *	prime(0) and nonprime(1) times during a day
 *	for BTL, prime time is 9AM to 5PM
 */
static struct hours {
	int	h_sec;		/* normally always zero */
	int	h_min;		/* initialized from holidays file (time%100) */
	int	h_hour;		/* initialized from holidays file (time/100) */
	int	h_type;		/* prime/nonprime of previous period */
} h[4];
int	daysend[] = {60, 59, 23};	/* the sec, min, hr of the day's end */

struct tm *localtime();

/*
 * split interval of length etime, starting at start into prime/nonprime
 * values, return as result
 * input values in seconds
 */
pnpsplit(start, etime, result)
long start, etime, result[2];
{
	struct tm cur, end;
	long tcur, tend;
	long tmp;
	register sameday;
	register struct hours *hp;

	extern int	tmless();
	extern long	tmsecs();
	extern char  	*copyn();
	if (thisyear)	/* once holidays file is read, this is zero */
		checkhol();

	tcur = start;
	tend = start+etime;
	(void)copyn(&end, localtime(&tend), sizeof(end));
	result[P] = 0;
	result[NP] = 0;

	while (tcur < tend) {	/* one iteration per day or part thereof */
		(void)copyn(&cur, localtime(&tcur), sizeof(cur));
		sameday = cur.tm_yday == end.tm_yday;
		if (ssh(&cur)) {	/* ssh:only NP */
			if (sameday) {
				result[NP] += tend-tcur;
				break;
			} else {
				tmp = tmsecs(&cur, (struct tm *)daysend);
				result[NP] += tmp;
				tcur += tmp;
			}
		} else {	/* working day, P or NP */
			for (hp = h; (hp < &h[4]) && tmless((struct tm *)hp, &cur); hp++);
			for (; (hp < &h[4]) && (hp->h_sec >= 0); hp++) {
				if (sameday && tmless(&end, (struct tm *)hp)) {
					/* PTM P9849 @rec@ */
					result[hp->h_type] += tend-tcur;
					tcur = tend;
					break;	/* all done */
				} else {	/* time to next P/NP change */
					tmp = tmsecs(&cur, (struct tm *)hp);
					result[hp->h_type] += tmp;
					tcur += tmp;
					cur.tm_sec = hp->h_sec;
					cur.tm_min = hp->h_min;
					cur.tm_hour = hp->h_hour;
				}
			}
			if( hp == &h[4] ) break;
		}
	}
}

/*
 *	Starting day after Christmas, complain if holidays not yet updated.
 *	This code is only executed once per program invocation.
 */
checkhol()
{
	register struct tm *tp;
	long t;
	extern int time();

	if(inithol() == 0) {
		(void)fprintf(stderr, MSGSTR( PNPHOLTBLERR, "pnpsplit: holidays table setup failed\n"));
		thisyear = 0;
		holidays[0] = -1;
		return;
	}
	(void)time(&t);
	tp = localtime(&t);
	tp->tm_year += 1900;
	if ((tp->tm_year == thisyear && tp->tm_yday > 359)
		|| tp->tm_year > thisyear)
		(void)fprintf(stderr,
			MSGSTR( UPDTHOL, "***UPDATE %s WITH NEW HOLIDAYS***\n"), holfile);
	thisyear = 0;	/* checkhol() will not be called again */
}

/*
 * ssh returns 1 if Sat, Sun, or Holiday
 */
ssh(ltp)
register struct tm *ltp;
{
	register i;

	if (ltp->tm_wday == 0 || ltp->tm_wday == 6)
		return(1);
	for (i = 0; holidays[i] >= 0; i++) {
		if (ltp->tm_yday > holidays[i])
			continue;
		else if (ltp->tm_yday == holidays[i])
			return(1);
		else if (ltp->tm_yday < holidays[i])
			break;	/* holidays is sorted */
	}
	return(0);
}

/*
 * inithol - read from an ascii file and initialize the "thisyear"
 * variable, the times that prime and non-prime start, and the
 * holidays array.
 */
inithol()
{
	FILE		*fopen(), *holptr;
	char		*fgets(), holbuf[128];
	register int	line = 0,
			holindx = 0,
			errflag = 0;
	void		sort();
	int		pstart, npstart;
	int		doy;	/* day of the year */
	extern void perror();

	if((holptr=fopen(holfile, "r")) == NULL) {
		perror(holfile);
		(void)fclose(holptr);
		return(0);
	}
	while(fgets(holbuf, sizeof(holbuf), holptr) != NULL) {
		if(holbuf[0] == '*')	/* Skip over comments */
			continue;
		else if(++line == 1) {	/* format: year p-start np-start */
			if(sscanf(holbuf, "%4d %4d %4d",
				&thisyear, &pstart, &npstart) != 3) {
				(void)fprintf(stderr,
					MSGSTR( BADYEARCONV, "%s: bad {yr ptime nptime} conversion\n"),
					holfile);
				errflag++;
				break;
			}

			/* validate year */
			if(thisyear < 1970 || thisyear > 2000) {
				(void)fprintf(stderr, MSGSTR( BADYEAR, "pnpsplit: invalid year: %d\n"),
					thisyear);
				errflag++;
				break;
			}

			/* validate prime/nonprime hours */
			if((! okay(pstart)) || (! okay(npstart))) {
				(void)fprintf(stderr,
					MSGSTR( PNPINVAL, "pnpsplit: invalid p/np hours\n"));
				errflag++;
				break;
			}

			/* Set up start of prime time; 2400 == 0000 */
			h[0].h_sec = 0;
			h[0].h_min = pstart%100;
			h[0].h_hour = (pstart/100==24) ? 0 : pstart/100;
			h[0].h_type = NP;

			/* Set up start of non-prime time; 2400 == 0000 */
			h[1].h_sec = 0;
			h[1].h_min = npstart%100;
			h[1].h_hour = (npstart/100==24) ? 0 : npstart/100;
			h[1].h_type = P;

			/* This is the end of the day */
			h[2].h_sec = daysend[0];
			h[2].h_min = daysend[1];
			h[2].h_hour = daysend[2];
			h[2].h_type = NP;

			/* The end of the array */
			h[3].h_sec = -1;

			continue;
		}
		else if(holindx >= NHOLIDAYS) {
			(void)fprintf(stderr, MSGSTR( PNPHOL1, "pnpsplit: too many holidays, "));
			(void)fprintf(stderr, MSGSTR( PNPHOL2, "recompile with larger NHOLIDAYS\n"));
			errflag++;
			break;
		}

		/* Fill up holidays array from holidays file */
		(void)sscanf(holbuf, "%d	%*s %*s	%*[^\n]\n", &doy);
		if(doy < 1 || doy > 366) {
			(void)fprintf(stderr,
				MSGSTR( PNPBADAY, "pnpsplit: invalid day of year %d\n"), doy);
			errflag++;
			break;
		}
		holidays[holindx++] = (doy - 1);
	}
	(void)fclose(holptr);
	if(!errflag && holindx < NHOLIDAYS) {
		sort(holidays, holindx - 1);
		holidays[holindx] = -1;
		return(1);
	}
	else
		return(0);
}

/*
 * sort - a version of the bubblesort algorithm from BAASE, Alg 2.1
 *
 * sorts the holidays array into nondecreasing order
 */
void
sort(array, nitems)
int	*array;		/* a pointer to the holidays array */
int	nitems;		/* the number of elements in the array */
{
	register int	index,	/* index going though holidays array */
			flag,	/* flag > 0 if more sorting is needed */
			k;

	flag = nitems;
	while(flag > 0) {
		k = flag - 1;
		flag = 0;
		for(index=0; index <= k; ++index) {
			if(array[index] > array[index+1]) {
				flag = array[index];	/* briefly use "flag"*/
				array[index] = array[index+1];
				array[index+1] = flag;
				flag = index;
			}
		}
	}

}
