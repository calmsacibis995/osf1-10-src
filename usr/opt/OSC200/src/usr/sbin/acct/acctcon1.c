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
static char	*sccsid = "@(#)$RCSfile: acctcon1.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/07 21:53:29 $";
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
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: bootshut, fixup, iline, loop, nomem, prctmp, printlin,
 *            printrep, sortty, upall, update, valid, comptty, wread
 *
 * ORIGINS: 3,9,27
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
 *	acctcon1 [-p] [-t] [-l file] [-o file] <wtmp-file >ctmp-file
 *	-p	print input only, no processing
 *	-t	test mode: use latest time found in input, rather than
 *		current time when computing times of lines still on
 *		(only way to get repeatable data from old files)
 *	-l file	causes output of line usage summary
 *	-o file	causes first/last/reboots report to be written to file
 *      reads input (normally /var/adm/wtmp), produces
 *	list of sessions, sorted by ending time in ctmp.h/ascii format
 *	A_TSIZE is max # distinct ttys
 */

#include <sys/types.h>
#include "acctdef.h"
#include <sys/acct.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "ctmp.h"
#include "table.h"

#include <locale.h>
#include "acct_msg.h"
nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

struct  utmp	wb;	/* record structure read into */
struct	ctmp	cb;	/* record structure written out of */

struct tbuf {
	char	tline[LSZ];	/* /dev/* */
	char	tname[NSZ];	/* user name */
	time_t	ttime;		/* start time */
	dev_t	tdev;		/* device */
	int	tlsess;		/* # complete sessions */
	int	tlon;		/* # times on (ut_type of 7) */
	int	tloff;		/* # times off (ut_type != 7) */
	long	ttotal;		/* total time used on this line */
	ushort  tchain;         /* used entries chain */
	ushort  thchain;        /* hash chain */
	ushort  tstatus;        /* status of `user/dead process' occurrences */

} *tbuf, t0;
unsigned short tbufstart;
struct  table tbuftable = INITTABLE(tbuf, A_TSIZE);

#define NSYS	20
int	nsys;
struct sys {
	char	sname[LSZ];	/* reasons for ACCOUNTING records */
	char	snum;		/* number of times encountered */
} sy[NSYS];

time_t	datetime;	/* old time if date changed, otherwise 0 */
time_t	firstime;
time_t	lastime;
int	ndates;		/* number of times date changed */
int	exitcode;
char	*report	= NULL;
char	*replin = NULL;
char    *prog;
int	printonly;
int	tflag;

char	timbuf[BUFSIZ];
struct tm *localtime();
long	ftell();
uid_t	namtouid();
dev_t	lintodev();

static int	wread(), valid(), fixup();
static int	loop(), bootshut(), nomem();
static int	upall(), update(), sortty(), comptty();
static int	printrep(), printlin(), prctmp();
static unsigned	iline();

main(int argc, char **argv) 
{
	register int i;

	prog = argv[0];

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_ACCT,NL_CAT_LOCALE);

	while (--argc > 0 && **++argv == '-')
		switch(*++*argv) {
		case 'l':
			if (--argc > 0)
				replin = *++argv;
			continue;
		case 'o':
			if (--argc > 0)
				report = *++argv;
			continue;
		case 'p':
			printonly++;
			continue;
		case 't':
			tflag++;
			continue;
		}

	if (printonly) {
		while (wread()) {
			if (valid()) {
				(void)printf("%.*s\t%.*s\t%lu",
					LSZ, wb.ut_line,
					NSZ, wb.ut_user,
					wb.ut_time);
				(void)strftime(timbuf, BUFSIZ, "%c %Z %n", 
						localtime(&wb.ut_time));
				(void)printf("\t%s", timbuf);
			} else 
				fixup(stdout);
			
		}
		exit(exitcode);
	}
	/* Allocate initial tbuf table and clear hash section */
	if (extend(&tbuftable) == NULL) {
		nomem();
	}
	for (i = 0; i <= THASH; i++)
		tbuf[i] = t0;

	while (wread()) {
		if (firstime == 0)
			firstime = wb.ut_time;
		if (valid())
			loop();
		else 
			fixup(stderr);
	}
	wb.ut_user[0] = '\0';
	(void)strcpy(wb.ut_line, "acctcon1");
	wb.ut_type = ACCOUNTING;
	if (tflag)
		wb.ut_time = lastime;
	else
		(void)time(&wb.ut_time);
	loop();
	if (report != NULL)
		printrep();
	if (replin != NULL)
		printlin();
	exit(exitcode);
	return(0);
}

static
wread()
{
	return( fread(&wb, sizeof(wb), 1, stdin) == 1 );
	
}

/*
 * valid: check input wtmp record, return 1 if looks OK
 */
static
valid()
{
	register i, c;

	for (i = 0; i < NSZ; i++) {
		c = wb.ut_user[i];
		if (isprint(c))
			continue;
		else if (c == '\0')
			break;
		else
			return(0);
	}

	if((wb.ut_type >= EMPTY) && (wb.ut_type <= UTMAXTYPE))
		return(1);

	return(0);
}

/*
 *	fixup assumes that V6 wtmp (16 bytes long) is mixed in with
 *	V7 records (20 bytes each)
 *
 *	Starting with Release 5.0 of UNIX, this routine will no
 *	longer reset the read pointer.  This has a snowball effect
 *	On the following records until the offset corrects itself.
 *	If a message is printed from here, it should be regarded as
 *	a bad record and not as a V6 record.
 */
static
fixup(stream)
register FILE *stream;
{
	(void)fprintf(stream, MSGSTR( BADWTMP, "bad wtmp: offset %lu.\n"), 
		ftell(stdin)-sizeof(wb));
	(void)fprintf(stream, MSGSTR( WTMPREC, "bad record is:  %.*s\t%.*s\t%lu"),
		LSZ, wb.ut_line,
		NSZ, wb.ut_user,
		wb.ut_time);
	(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&wb.ut_time));
	(void)fprintf( stream, "\t%s", timbuf);
#ifdef	V6
	fseek(stdin, (long)-4, 1);
#endif
	exitcode = 1;
}

static
loop()
{
	register unsigned ti;
	register time_t timediff;
	register struct tbuf *tp;

	if( wb.ut_line[0] == '\0' )	/* It's an init admin process */
		return;			/* no connect accounting data here */
	switch(wb.ut_type) {
	case OLD_TIME:
		datetime = wb.ut_time;
		return;
	case NEW_TIME:
		if(datetime == 0)
			return;
		timediff = wb.ut_time - datetime;
		for (ti = tbufstart; ti; ti = tp->tchain) {
			tp = &tbuf[ti];
			tp->ttime += timediff;
		}
		datetime = 0;
		ndates++;
		return;
	case BOOT_TIME:
		upall();
	case ACCOUNTING:
	case RUN_LVL:
		lastime = wb.ut_time;
		bootshut();
		return;
	case LOGIN_PROCESS:
	case INIT_PROCESS:
	case EMPTY:
		return;
	case USER_PROCESS:       /* user logged on */
 	case DEAD_PROCESS:       /* first one indicates user logged off */
		ti = iline();
		update(&tbuf[ti]);
		return;
	default:
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", 
					localtime(&wb.ut_time));
		(void)fprintf(stderr, 
		 MSGSTR( CON1BADTYPE, "acctcon1: invalid type %d for %.*s %.*s %s"),
			wb.ut_type, NSZ, wb.ut_user, LSZ, wb.ut_line, timbuf);
	}
}

/*
 * bootshut: record reboot (or shutdown)
 * bump count, looking up wb.ut_line in sy table
 */
static
bootshut()
{
	register i;

	for (i = 0; i < nsys && !EQN(wb.ut_line, sy[i].sname); i++)
		;
	if (i >= nsys) {
		if (++nsys > NSYS) {
			(void)fprintf(stderr, MSGSTR( CON1TOOSMALL, 
				     "acctcon1: recompile with larger NSYS\n"));
			nsys = NSYS;
			return;
		}
		(void)CPYN(sy[i].sname, wb.ut_line);
	}
	sy[i].snum++;
}

/*
 * iline: look up/enter current line name in tbuf, return index
 * (used to avoid system dependencies on naming)
 */
static unsigned
iline()
{       register struct tbuf *tp;
	register unsigned t, th;
	static tused = THASH;

	/* Hash line name and look it up */
	for (t = th = 0; t < LSZ; t++) {
		th *= 61;
		th += wb.ut_line[t];
	}
	t = th = th%THASH + 1;
	do {
		tp = &tbuf[t];
		if (EQN(wb.ut_line, tp->tline))
			return(t);
	} while (t = tp->thchain);

	/* If already an entry in this slot (first hash), get another slot.
	 * An empty slot is one that has a null linename.
	 */
	if (tp->tline[0]) {
		if ((tused += 1) > tbuftable.tb_nel) {
			tbuftable.tb_nel += 32;
			if (extend(&tbuftable) == NULL)
				nomem();
		}
		tp = &tbuf[t = tused];
		*tp = t0;
	} else
		t = th;
	if (t != th) {  /* If not first entry, link on at head of chain */
		tp->thchain = tbuf[th].thchain;
		tbuf[th].thchain = t;
	}
	/* Add this entry to the list of all lines */
	tp->tchain = tbufstart;
	tbufstart = t;
	(void)CPYN(tp->tline, wb.ut_line);
	tp->tdev = lintodev(wb.ut_line);
	return(t);
}

static
nomem()
{       (void)fprintf(stderr,MSGSTR( NOMEM, "%s: Cannot allocate memory\n"), prog);
	exit(2);
}

static
upall()
{
	register unsigned ti;
	register struct tbuf *tp;

	wb.ut_type = DEAD_PROCESS;	/* fudge a logoff for reboot record */
	for (ti = tbufstart; ti; ti = tp->tchain)
		update(tp = &tbuf[ti]);
}

/*
 * update tbuf with new time, write ctmp record for end of session
 */
static
update(tp)
register struct tbuf *tp;
{
	time_t	told,	/* last time for tbuf record */
		tnew;	/* time of this record */
			/* Difference is connect time */

	told = tp->ttime;
	tnew = wb.ut_time;
	if (told > tnew) {
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&told));
		(void)fprintf(stderr,
			MSGSTR( BADTIMEOLD, "%s: bad times: old: %s"),
			prog, timbuf);
		(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&tnew));
		(void)fprintf(stderr,MSGSTR( BADTIMENEW, "new: %s"), timbuf);
		exitcode = 1;
		tp->ttime = tnew;
		return;
	}
	tp->ttime = tnew;
	switch(wb.ut_type) {
	case USER_PROCESS:
		tp->tlsess++;
		if(tp->tname[0] != '\0') { /* Someone logged in without */
					   /* logging off. Put out record. */
			cb.ct_tty = tp->tdev;
			(void)CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			prctmp(&cb);
			tp->ttotal += tnew-told;
		}
		else	/* Someone just logged in */
			tp->tlon++;
		tp->tstatus = LOGGED_ON;   /* mark as logged on */
		(void)CPYN(tp->tname, wb.ut_user);
		break;
	case DEAD_PROCESS:
		tp->tloff++;
		if(tp->tname[0] != '\0' && tp->tstatus == LOGGED_ON) { 
		/* Someone logged off */
			/* Set up and print ctmp record */
			cb.ct_tty = tp->tdev;
			(void)CPYN(cb.ct_name, tp->tname);
			cb.ct_uid = namtouid(cb.ct_name);
			cb.ct_start = told;
			pnpsplit(cb.ct_start, tnew-told, cb.ct_con);
			prctmp(&cb);
			tp->ttotal += tnew-told;
			tp->tname[0] = '\0';
			tp->tstatus = LOGGED_OFF;   /* mark as logged off */
		}
	}
}

static
printrep()
{
	register i;

	(void)freopen(report, "w", stdout);
	(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&firstime));
	(void)printf(MSGSTR( CON1FROM, "from %s"), timbuf);
	(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&lastime));
	(void)printf(MSGSTR( CON1TO, "to   %s"), timbuf);
	if (ndates)
		if (ndates>1)
			(void)printf(MSGSTR(DATECHGS, "%d\tdate changes\n"),ndates);
		else
			(void)printf(MSGSTR( DATECHG, "%d\tdate change\n"),ndates);
	for (i = 0; i < nsys; i++)
		(void)printf("%d\t%.12s\n", sy[i].snum, sy[i].sname);
}

/*
 *	print summary of line usage
 *	accuracy only guaranteed for wtmp file started fresh
 */
static
printlin()
{
	register struct tbuf *tp;
	register int ti, ntty;
	double timet, timei;
	double ttime;
	int tsess, ton, toff;

	(void)freopen(replin, "w", stdout);
	ttime = 0.0;
	tsess = ton = toff = 0;
	timet = MINS(lastime-firstime);
	(void)printf(MSGSTR( CON1TOT1, "TOTAL DURATION: %.0f MINUTES\n\n"), timet);
	(void)printf(MSGSTR( CON1TOT2, 
			     "LINE\tMINUTES\tPERCENT\t# SESS\t# ON\t# OFF\n"));
	ntty = sortty();
	for (ti = 0; ti < ntty; ti++) {
		tp = &tbuf[ti];
		timei = MINS(tp->ttotal);
		ttime += timei;
		tsess += tp->tlsess;
		ton += tp->tlon;
		toff += tp->tloff;
		(void)printf("%.8s\t%.0f\t%.0f\t%d\t%d\t%d\n",
			tp->tline,
			timei,
			(timet > 0.)? 100*timei/timet : 0.,
			tp->tlsess,
			tp->tlon,
			tp->tloff);
	}
	(void)printf(MSGSTR( CON1TOT3, "TOTALS\t%.0f\t--\t%d\t%d\t%d\n"), ttime, tsess, ton, toff);
}

static
prctmp(t)
register struct ctmp *t;
{

	(void)printf("%u\t%lu\t%.8s\t%lu\t%lu\t%lu",
		t->ct_tty,
		t->ct_uid,
		t->ct_name,
		t->ct_con[0],
		t->ct_con[1],
		t->ct_start);
	(void)strftime(timbuf, BUFSIZ, "%c %Z %n", localtime(&t->ct_start));
	(void)printf("\t%s", timbuf);
}

/* Sort tbuf entries.  Links (tchain, thchain) no longer valid afterwards. */
static
sortty()
{       register struct tbuf *tempty, *tp;
	register unsigned ti;
	int ntty = 0;
	int comptty();

	tempty = &tbuf[0];

	for (ti = tbufstart; ti; ti = tp->tchain) {
		tp = &tbuf[ti];
		++ntty;

		/* Find next empty slot before this one, if any */
		while (tempty < tp && tempty->tline[0])
			tempty++;

		/* If we found one, transfer this entry */
		if (tempty->tline[0] == '\0') {
			*tempty = *tp;
			tp->tline[0] = '\0';
		}
	}
	qsort(tbuf, ntty, sizeof (*tbuf), comptty);
	return(ntty);
}

static
comptty(t1, t2)
struct tbuf *t1, *t2;
{       register char *n1 = t1->tline;
	register char *n2 = t2->tline;

	/* No end check necessary as there are no duplicates */
	while (*n1 == *n2) ++n1, ++n2;
	return(*n1 - *n2);
}
