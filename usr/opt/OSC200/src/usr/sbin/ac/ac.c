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
static char	*sccsid = "@(#)$RCSfile: ac.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/10/14 22:23:30 $";
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
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: loop, print, upall, update, among, newday, pdate 
 *
 * ORIGINS: 9, 26, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
#ifndef lint
static char sccsid[] = "ac.c	1.2  com/cmd/acct/ac,3.1,8943 10/24/89 10:29:19";
#endif
 */
/*
 * ac [ -w wtmp ] [ -d ] [ -p ] [ people ]
 */

/*		History
 * 001	1991-10-31	zanne
 *	Changed length of array timestring from six to seven and changed it 
 *	to a constant called PDATESIZE.  The lenght of timestring did not
 *	account for the null terminater from function strftime.  The
 *	null character was being written over the last character of
 *	the date string for screen output.
 */

#include <stdio.h>
#include <ctype.h>
#include <utmp.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/fcntl.h>
#include <locale.h>

#include "ac_msg.h" 
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd,MS_AC,Num,Str) 

#define NMAX sizeof(ibuf.ut_name)
#define LMAX sizeof(ibuf.ut_line)

#define TSIZE  6242
#define	USIZE	500

#define PDATESIZE	7  /* 001 */

struct  utmp ibuf;

struct ubuf {
	char	uname[NMAX];
	long	utime;
} ubuf[USIZE];

struct tbuf {
	struct	ubuf	*userp;
	long	ttime;
} tbuf[TSIZE];

char	*wtmp;
int	pflag, byday;
long	dtime;
long	midnight;
long	lastime;
long	day	= 86400L;
int	pcount;
char	**pptr;
#define USAGE_STR	"Usage: %s [-dp] [-w filename] [user(s)...]\n"

#include <NLchar.h>
#include <NLctype.h>

main(argc, argv) 
char **argv;
{
        int bytesread, wf;
	int fl;
	int c;
	register i;
	extern char *optarg;
	extern int optind, opterr;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_AC,NL_CAT_LOCALE);

	wtmp = WTMP_FILE;
	while ((c=getopt(argc, argv, "dpw:")) != EOF) {
		switch(c) {
		case 'd':
			byday++;
			break;

		case 'p':
			pflag++;
			break;

		case 'w':
			if ( optarg ) {
				wtmp = optarg;
			} else {
				fprintf(stderr, MSGSTR(NOFILENM,
						"ac: Missing filename\n")); 
				exit(1); 
			}
			break;

		default:
		    	fprintf(stderr, MSGSTR(ACUSAGE, USAGE_STR), argv[0]);
			exit(1);
		}
	}
	pcount = argc - optind;
	pptr = &argv[optind];
        if ((wf=open(wtmp, O_RDONLY)) < 0) {
                (void)fprintf(stderr, MSGSTR(CANTOPEN,
                                        "ac: Can't open %s\n"), wtmp);
                exit(1);
        }

        for(;;) {
                bytesread = read(wf, (char *)&ibuf, sizeof(ibuf));
                if (bytesread == 0)                 /* eof */
                  break;
                if (bytesread < sizeof(ibuf)) {     /* invalid record read */
                  (void)fprintf(stderr, MSGSTR(BADDATA,
                                "ac: Invalid record found in %s.  Process aborted.\n"), wtmp);
                  exit(1);
                }

		fl = 0;
		for (i=0; i<NMAX; i++) {
			c = ibuf.ut_name[i];
			if (isprint(c) && !isspace(c)) {
				if (fl)
					goto skip;
				continue;
			}
			if (isspace(c) || c=='\0') {
				fl++;
				ibuf.ut_name[i] = '\0';
			} else
				goto skip;
		}
		loop();
    skip:;
	}
	ibuf.ut_name[0] = '\0';
	ibuf.ut_line[0] = '~';
	(void)time(&ibuf.ut_time);
	loop();
	print();
	return(0);
}

loop()
{
	register i;
	register struct tbuf *tp;
	register struct ubuf *up;

	if(ibuf.ut_line[0] == '|') {
		dtime = ibuf.ut_time;
		return;
	}
	if(ibuf.ut_line[0] == '{') {
		if(dtime == 0)
			return;
		for(tp = tbuf; tp < &tbuf[TSIZE]; tp++)
			tp->ttime += ibuf.ut_time-dtime;
		dtime = 0;
		return;
	}
	if (lastime>ibuf.ut_time || lastime+(1.5*day)<ibuf.ut_time)
		midnight = 0;
	if (midnight==0)
		newday();
	lastime = ibuf.ut_time;
	if (byday && ibuf.ut_time > midnight) {
		upall(1);
		print();
		newday();
		for (up=ubuf; up < &ubuf[USIZE]; up++)
			up->utime = 0;
	}
	if (ibuf.ut_line[0] == '~') {
		ibuf.ut_name[0] = '\0';
		upall(0);
		return;
	}
	/*
	if (ibuf.ut_line[0]=='t')
		i = (ibuf.ut_line[3]-'0')*10 + (ibuf.ut_line[4]-'0');
	else
		i = TSIZE-1;
	if (i<0 || i>=TSIZE)
		i = TSIZE-1;
	*/

	/*
	 * Correction contributed by Phyllis Kantar @ Rand-unix
	 *
	 * Fixes long standing problem with tty names other than 00-99
	 */
	if (ibuf.ut_line[0]=='t') {
		i = (ibuf.ut_line[3]-'0');
		if(ibuf.ut_line[4])
			i = i*79 + (ibuf.ut_line[4]-'0');
	} else
		i = TSIZE-1;
	if (i<0 || i>=TSIZE) {
		i = TSIZE-1;
		(void)printf(MSGSTR(BADTTY, "ac: Bad tty name: %s\n"), 
							ibuf.ut_line);
	}

	tp = &tbuf[i];
	update(tp, 0);
}

print()
{
	int i;
	long ttime, t;

	ttime = 0;
	for (i=0; i<USIZE; i++) {
		if(!among(i))
			continue;
		t = ubuf[i].utime;
		if (t>0)
			ttime += t;
		if (pflag && ubuf[i].utime > 0) {
			(void)printf("\t%-*.*s %6.2f\n", NMAX, NMAX,
			    ubuf[i].uname, ubuf[i].utime/3600.);
		}
	}
	if (ttime > 0) {
		pdate();
		(void)printf(MSGSTR(TOTAL, "\ttotal %9.2f\n"), ttime/3600.);
	}
}

upall(f)
{
	register struct tbuf *tp;

	for (tp=tbuf; tp < &tbuf[TSIZE]; tp++)
		update(tp, f);
}

update(tp, f)
struct tbuf *tp;
{
	int j;
	struct ubuf *up;
	long t, t1;

	if (f)
		t = midnight;
	else
		t = ibuf.ut_time;
	if (tp->userp) {
		t1 = t - tp->ttime;
		if (t1 > 0)
			tp->userp->utime += t1;
	}
	tp->ttime = t;
	if (f)
		return;
	if (ibuf.ut_name[0]=='\0') {
		tp->userp = 0;
		return;
	}
	for (up=ubuf; up < &ubuf[USIZE]; up++) {
		if (up->uname[0] == '\0')
			break;
		for (j=0; j<NMAX && up->uname[j]==ibuf.ut_name[j]; j++);
		if (j>=NMAX)
			break;
	}
	for (j=0; j<NMAX; j++)
		up->uname[j] = ibuf.ut_name[j];
	tp->userp = up;
}

among(i)
{
	register j, k;
	register char *p;

	if (pcount==0)
		return(1);
	for (j=0; j<pcount; j++) {
		p = pptr[j];
		for (k=0; k<NMAX; k++) {
			if (*p == ubuf[i].uname[k]) {
				if (*p++ == '\0' || k == NMAX-1)
					return(1);
			} else
				break;
		}
	}
	return(0);
}

newday()
{
	long ttime;
	struct timeb tb;
	struct tm *localtime();

	(void)time(&ttime);
	if (midnight == 0) {
		ftime(&tb);
		midnight = 60*(long)tb.timezone;
		if (localtime(&ttime)->tm_isdst)
			midnight -= 3600;
	}
	while (midnight <= ibuf.ut_time)
		midnight += day;
}

pdate()
{
	long x;
	char timestring[PDATESIZE];		/* 001 */
	struct tm	*tmp;

	if (byday==0)
		return;
	x = midnight-1;

	tmp = localtime(&x);
	strftime(timestring, PDATESIZE, "%sD", tmp);	/* 001 */
	(void)printf("%s ", timestring);
}
