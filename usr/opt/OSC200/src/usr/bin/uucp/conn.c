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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: conn.c,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/09/07 16:04:30 $";
#endif
/* 
 * COMPONENT_NAME: UUCP conn.c
 * 
 * FUNCTIONS: alarmtr, chat, classmatch, conn, expect, fdig, finds, 
 *            getProto, getto, ifdate, nap, notin, protoString, 
 *            rddev, sendthem, take_a_nap, wrchar 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
conn.c	1.5  com/cmd/uucp,3.1,9013 11/30/89 10:07:11";
*/
/*	uucp:conn.c	1.15.1.1
*/
#include "uucp.h"
#include <sys/time.h>

/* VERSION( conn.c	5.3 -  -  ); */

static char _ProtoStr[20] = "";	/* protocol string from Systems file entry */
extern jmp_buf Sjbuf;

int alarmtr();
static void getProto();
/* static getto(), finds();   PUT this back after altconn is included */
extern getto();
static finds();
extern char *strchr();

static notin(), ifdate(), classmatch();

extern struct caller caller[];

/* Needed for cu for force which line will be used */
#ifdef STANDALONE
extern char *Myline;
#endif

int nap_time = 0;	/* accumulator for time to pause (nap) in HZ units. */

/*
 * conn - place a telephone call to system and login, etc.
 *
 * return codes:
 *	FAIL - connection failed
 *	>0  - file no.  -  connect ok
 * When a failure occurs, Uerror is set.
 */

conn(system)
char *system;
{
	int nf, fn = FAIL;
	char *flds[F_MAX+1];
	FILE *fsys;

	CDEBUG(4, MSGSTR(MSG_CONN_CD1, "conn(%s)\n"), system);
	fsys = fopen(SYSFILE, "r");
	ASSERT(fsys != NULL, Ct_OPEN, SYSFILE, errno);
	Uerror = 0;
	while ((nf = finds(fsys, system, flds, F_MAX)) > 0) {
		fn = getto(flds);
		CDEBUG(4, MSGSTR(MSG_CONN_CD2, "getto ret %d\n"), fn);
		if (fn < 0)
		    continue;

#ifdef STANDALONE
		fclose(fsys);
		return(fn);
#else
		if (chat(nf - F_LOGIN, flds + F_LOGIN, fn,"","") == SUCCESS) {
			fclose(fsys);
			return(fn); /* successful return */
		}

		/* login failed */
		DEBUG(6, "close caller (%d)\n", fn);
		close(fn);
		if (Dc[0] != NULLCHAR) {
			DEBUG(6, "ttyunlock(%s)\n", Dc);
			ttyunlock(Dc);
		}
#endif
	}

	/* finds or getto failed */
	fclose(fsys);
	CDEBUG(1, MSGSTR(MSG_CONN_CD3,"Call Failed: %s\n"), UERRORTEXT);
	return(FAIL);
}

/*
 * getto - connect to remote machine
 *
 * return codes:
 *	>0  -  file number - ok
 *	FAIL  -  failed
 */

getto(flds)
char *flds[];
{
	FILE *dfp;
	char *dev[D_MAX+2], devbuf[BUFSIZ];
	register int status;
	register int dcf = -1;
	int reread = 0;
	int tries = 0;	/* count of call attempts - for limit purposes */
#ifndef STANDALONE
	extern char *LineType;

	LineType = flds[F_TYPE];
#endif
	CDEBUG(1, MSGSTR(MSG_CONN_CD4, "Device Type %s wanted\n"),flds[F_TYPE]);
	dfp = fopen(DEVFILE, "r");
	ASSERT(dfp != NULL, Ct_OPEN, DEVFILE, errno);
	Uerror = 0;
	while (tries < TRYCALLS) {
		if ((status=rddev(dfp, flds[F_TYPE], dev, devbuf, D_MAX)) == FAIL) {
			if (tries == 0 || ++reread >= TRYCALLS)
				break;
			rewind(dfp);
			continue;
		}
		/* check class, check (and possibly set) speed */
		if (classmatch(flds, dev) != SUCCESS)
			continue;
		if ((dcf = processdev(flds, dev)) >= 0)
			break;

		switch(Uerror) {
		case SS_CANT_ACCESS_DEVICE:
		case SS_DEVICE_FAILED:
		case SS_LOCKED_DEVICE:
			break;
		default:
			tries++;
			break;
		}
	}
	(void) fclose(dfp);
	if (status == FAIL && !Uerror) {
		CDEBUG(1, MSGSTR(MSG_CONN_CD5, 
		"Requested Device Type Not Found\n"), 0);
		Uerror = SS_NO_DEVICE;
	}
	return(dcf);
}

/*
 * classmatch - process 'Any' in Devices and Systems and
 * 	determine the correct speed, or match for ==
 */

static int
classmatch(flds, dev)
char *flds[], *dev[];
{
	/* check class, check (and possibly set) speed */
	if (EQUALS(flds[F_CLASS], "Any")
	   && EQUALS(dev[D_CLASS], "Any")) {
		dev[D_CLASS] = flds[F_CLASS] = DEFAULT_BAUDRATE;
		return(SUCCESS);
	} else if (EQUALS(dev[F_CLASS], "Any")) {
		dev[D_CLASS] = flds[F_CLASS];
		return(SUCCESS);
	} else if (EQUALS(flds[F_CLASS], "Any")) {
		flds[D_CLASS] = dev[F_CLASS];
		return(SUCCESS);
	} else if (EQUALS(flds[F_CLASS], dev[D_CLASS]))
		return(SUCCESS);
	else
		return(FAIL);
}


/***
 *	rddev - find and unpack a line from device file for this caller type 
 *	lines starting with whitespace of '#' are comments
 *
 *	return codes:
 *		>0  -  number of arguments in vector - succeeded
 *		FAIL - EOF
 ***/

rddev(fp, type, dev, buf, devcount)
FILE *fp;
char *type;
char *dev[];
char *buf;
{
	int na;

	while (fgets(buf, BUFSIZ, fp) != NULL) {
		if (buf[0] == ' ' || buf[0] == '\t'
		    ||  buf[0] == '\n' || buf[0] == '\0' || buf[0] == '#')
			continue;
		na = getargs(buf, dev, devcount);
		ASSERT(na >= D_CALLER, MSGSTR(MSG_CONN_A1,"BAD LINE"), buf, na);

/* For cu -- to force the requested line to be used */
#ifdef STANDALONE
		if ((Myline != NULL) && (!EQUALS(Myline, dev[D_LINE])) )
		    continue;
#endif

		bsfix(dev);	/* replace \X fields */
		if (EQUALS(dev[D_TYPE], type))
			return(na);
	}
	return(FAIL);
}


/*
 * finds	- set system attribute vector
 *
 * input:
 *	fsys - open Systems file descriptor
 *	sysnam - system name to find
 * output:
 *	flds - attibute vector from Systems file
 *	fldcount - number of fields in flds
 * return codes:
 *	>0  -  number of arguments in vector - succeeded
 *	FAIL - failed
 * Uerror set:
 *	0 - found a line in Systems file
 *	SS_BADSYSTEM - no line found in Systems file
 *	SS_TIME_WRONG - wrong time to call
 */

static
finds(fsys, sysnam, flds, fldcount)
char *sysnam, *flds[];
FILE *fsys;
{
	static char info[BUFSIZ];
	int na;

	/* format of fields
	 *	0 name;
	 *	1 time
	 *	2 acu/hardwired
	 *	3 speed
	 *	etc
	 */
	while (fgets(info, BUFSIZ, fsys) != NULL) {
		if (info[0] == '#')
			continue;
		na = getargs(info, flds, fldcount);
		bsfix(flds);	/* replace \X fields */
		if ( !EQUALSN(sysnam, flds[F_NAME], SYSNSIZE))
			continue;
#ifndef STANDALONE
		if (ifdate(flds[F_TIME])) {
			/*  found a good entry  */
			getProto(flds[F_TYPE]);
			Uerror = 0;
			return(na);	/* FOUND OK LINE */
		}
		CDEBUG(1, MSGSTR(MSG_CONN_CD6,"Wrong Time To Call: %s\n"),
		 flds[F_TIME]);
		Uerror = SS_TIME_WRONG;
#else
			Uerror = 0;
			return(na);	/* FOUND OK LINE */
#endif
	}
	if (!Uerror)
		Uerror = SS_BADSYSTEM;
	return(FAIL);
}

/*
 * getProto - get the protocol letters from the input string.
 * input:
 *	str - string from Systems file (flds[F_TYPE])--the ,
 *		delimits the protocol string
 *		e.g. ACU,g or DK,d
 * output:
 *	str - the , (if present) will be replaced with NULLCHAR
 *	global ProtoStr will be modified
 * return:  none
 */

static
void
getProto(str)
char *str;
{
	register char *p;
	if ( (p=strchr(str, ',')) != NULL) {
		*p = NULLCHAR;
		(void) strcpy(_ProtoStr, p+1);
		DEBUG(7, "ProtoStr = %s\n", _ProtoStr);
	}
	else
		*_ProtoStr = NULLCHAR;
}

/*
 * check for a specified protocol selection string
 * return:
 *	protocol string pointer
 *	NULL if none specified for LOGNAME
 */
char *
protoString()
{
	return(_ProtoStr[0] == NULLCHAR ? NULL : _ProtoStr);
}

static int _Echoflag;
/*
 * chat -	do conversation
 * input:
 *	flds - fields from Systems file
 *	nf - number of fields in flds array
 *	dcr - write file number
 *	phstr1 - phone number to replace \D
 *	phstr2 - phone number to replace \T
 *
 *	return codes:  0  |  FAIL
 */

chat(nf, flds, fn, phstr1, phstr2)
char *flds[], *phstr1, *phstr2;
int nf, fn;
{
	char *want, *altern;
	extern char *index();
	int k, ok;

	_Echoflag = 0;
	for (k = 0; k < nf; k += 2) {
		want = flds[k];
		ok = FAIL;
		while (ok != 0) {
			altern = index(want, '-');
			if (altern != NULL)
				*altern++ = NULLCHAR;
			ok = expect(want, fn);
			if (ok == 0)
				break;
			if (altern == NULL) {
				Uerror = SS_LOGIN_FAILED;
				logent(UERRORTEXT,MSGSTR(MSG_CONN_L1,"FAILED"));
				return(FAIL);
			}
			want = index(altern, '-');
			if (want != NULL)
				*want++ = NULLCHAR;
			sendthem(altern, fn, phstr1, phstr2);
		}
		sleep(2);
		if (flds[k+1])
		    sendthem(flds[k+1], fn, phstr1, phstr2);
	}
	return(0);
}

#define MR 300

/***
 *	expect(str, fn)	look for expected string
 *	char *str;
 *
 *	return codes:
 *		0  -  found
 *		FAIL  -  lost line or too many characters read
 *		some character  -  timed out
 */

expect(str, fn)
char *str;
int fn;
{
	static char rdvec[MR];
	char *rp = rdvec;
	register int kr, c;
	char nextch;
	extern	errno;

	*rp = 0;

	CDEBUG(4, MSGSTR(MSG_CONN_CD7,"expect: ("), 0);
	for (c=0; kr=str[c]; c++)
		if (kr < 040) {
			CDEBUG(4, "^%c", kr | 0100);
		} else
			CDEBUG(4, "%c", kr);
	CDEBUG(4, ")\n", 0);

	if (EQUALS(str, "\"\"")) {
		CDEBUG(4, MSGSTR(MSG_CONN_CD8,"got it\n"), 0);
		return(0);
	}
	if (setjmp(Sjbuf)) {
		return(FAIL);
	}
	(void) signal(SIGALRM, (void(*)(int)) alarmtr);
	alarm(MAXEXPECTTIME);
	while (notin(str, rdvec)) {
		errno = 0;
		kr = read(fn, &nextch, 1);
		if (kr <= 0) {
			alarm(0);
			CDEBUG(4, MSGSTR(MSG_CONN_CD9,
			"lost line errno - %d\n"), errno);
			logent(MSGSTR(MSG_CONN_L2,"LOGIN"), 
			 MSGSTR(MSG_CONN_L3,"LOST LINE"));
			return(FAIL);
		}
		c = nextch & 0177;
		CDEBUG(4, "%s", c < 040 ? "^" : "");
		CDEBUG(4, "%c", c < 040 ? c | 0100 : c);
		if ((*rp = nextch & 0177) != NULLCHAR)
			rp++;
		if (rp >= rdvec + MR) {
			CDEBUG(4, MSGSTR(MSG_CONN_CD10,"enough already\n"), 0);
			alarm(0);
			return(FAIL);
		}
		*rp = NULLCHAR;
	}
	alarm(0);
	CDEBUG(4, MSGSTR(MSG_CONN_CD8,"got it\n"), 0);
	return(0);
}


/***
 *	alarmtr()  -  catch alarm routine for "expect".
 */

alarmtr()
{
	CDEBUG(6, MSGSTR(MSG_CONN_CD12,"timed out\n"), 0);
	longjmp(Sjbuf, 1);
}


/***
 *	sendthem(str, fn, phstr1, phstr2)	send line of chat sequence
 *	char *str, *phstr;
 *
 *	return codes:  none
 */

sendthem(str, fn, phstr1, phstr2)
char *str, *phstr1, *phstr2;
int fn;
{
	int sendcr = 1;
	register char	*sptr, *pptr;

	if (PREFIX("BREAK", str)) {
		/* send break */
		CDEBUG(5, MSGSTR(MSG_CONN_CD11,"BREAK\n"), 0);
		genbrk(fn);
		return;
	}

	if (EQUALS(str, "EOT")) {
		CDEBUG(5, MSGSTR(MSG_CONN_CD13,"EOT\n"), 0);
		(void) write(fn, EOTMSG, strlen(EOTMSG));
		return;
	}

	if (EQUALS(str, "\"\"")) {
		CDEBUG(5, "\"\"\n", 0);
		str += 2;
	}

	CDEBUG(5, MSGSTR(MSG_CONN_CD14, "sendthem ("), "");
	if (setjmp(Sjbuf))	/* Timer so echo check doesn't last forever */
		goto err;
	(void) signal(SIGALRM, (void(*)(int)) alarmtr);
	alarm(MAXEXPECTTIME);
	for (sptr = str; *sptr; sptr++) {
		if (*sptr == '\\')
			switch(*++sptr) {
			case 'D':
				take_a_nap();	/* nap for nap_time */
				for (pptr=phstr1; *pptr; pptr++)
					if (wrchar(fn, pptr))
						goto err;
				continue;
			case 'T':
				take_a_nap();	/* nap for nap_time */
				for (pptr=phstr2; *pptr; pptr++)
					if (wrchar(fn, pptr))
						goto err;
				continue;
			case 'N':
				*sptr = 0;
				break;
			case 'd':
				CDEBUG(5, MSGSTR(MSG_CONN_CD15,"DELAY\n"), 0);
				/* pause for approximately 2 seconds */
				nap_time += (HZ * 2);	/* do it later */
				continue;
			case 'c':
				if (*(sptr+1) == NULLCHAR) {
				CDEBUG(5, MSGSTR(MSG_CONN_CD16,"<NO CR>"), 0);
					sendcr = 0;
					continue;
				}
				CDEBUG(5, MSGSTR(MSG_CONN_CD17,
				 "<NO CR - MIDDLE IGNORED>\n"), 0);
				continue;
			case 's':
				*sptr = ' ';
				break;
			case 'p':
				CDEBUG(5, MSGSTR(MSG_CONN_CD18,"PAUSE\n"), 0);
				/* pause for approximately 1/4 second */
				nap_time += (HZ / 4);	/* do it later */
				continue;
			case 'E':
				CDEBUG(5,MSGSTR(MSG_CONN_CD19,
					"ECHO CHECK ON\n"), 0);
				_Echoflag = 1;
				continue;
			case 'e':
				CDEBUG(5, MSGSTR(MSG_CONN_CD20,
				    "ECHO CHECK OFF\n"), 0);
				_Echoflag = 0;
				continue;
			case 'K':
				CDEBUG(5, MSGSTR(MSG_CONN_CD11,"BREAK\n"), 0);
				take_a_nap();	/* nap for nap_time */
				genbrk(fn);
				continue;
			case '\\':
				/* backslash escapes itself */
				break;
			default:
				/* send the backslash */
				--sptr;
				break;
			}
		take_a_nap();	/* nap for any accumulated nap time. */
		if (wrchar(fn, sptr))
			goto err;
	}

	if (sendcr) {
		CDEBUG(5, "^M", 0);
		(void) write(fn, "\r", 1);
	}
err:
	alarm(0);
	CDEBUG(5, ")\n", 0);
	return;
}

take_a_nap()
{
	if (nap_time > 0)
		nap(nap_time);
	nap_time = 0;
	return;
}

wrchar(fn, sptr)
register int fn;
register char *sptr;
{
	if (GRPCHK(getgid()))	/* protect Systems file */
	{
		CDEBUG(5, "%s", *sptr < 040 ? "^" : "");
		CDEBUG(5, "%c", *sptr < 040 ? *sptr | 0100 : *sptr);
	}
	if ((write(fn, sptr, 1)) != 1)
		return(FAIL);
	if (_Echoflag) {
		char chr;
		while(1) {	/* Should set a timer here */
			if (read(fn, &chr, 1) != 1)
				return(FAIL);
			chr &= 0177;
			CDEBUG(4, "%s", chr < 040 ? "^" : "");
			CDEBUG(4, "%c", chr < 040 ? chr | 0100 : chr);
			if (chr == ((*sptr) & 0177))
				break;
		}

	}
	return(SUCCESS);
}


/***
 *	notin(sh, lg)	check for occurrence of substring "sh"
 *	char *sh, *lg;
 *
 *	return codes:
 *		0  -  found the string
 *		1  -  not in the string
 */

static
notin(sh, lg)
char *sh, *lg;
{
	while (*lg != NULLCHAR) {
		if (PREFIX(sh, lg))
			return(0);
		else
			lg++;
	}
	return(1);
}


/*******
 *	ifdate(s)
 *	char *s;
 *
 *	ifdate  -  this routine will check a string (s)
 *	like "MoTu0800-1730" to see if the present
 *	time is within the given limits.
 *	SIDE EFFECT - Retrytime is set to number following ";"
 *
 *	String alternatives:
 *		Wk - Mo thru Fr
 *		zero or one time means all day
 *		Any - any day
 *
 *	return codes:
 *		0  -  not within limits
 *		1  -  within limits
 */

static
ifdate(s)
char *s;
{
	static char *days[] = {
		"Su", "Mo", "Tu", "We", "Th", "Fr", "Sa", 0
	};
	time_t	clock;
	int	t__now;
	char	*p, *rindex(), *index();
	struct tm	*tp;

	time(&clock);
	tp = localtime(&clock);
	t__now = tp->tm_hour * 100 + tp->tm_min;	/* "navy" time */

	/*
	 *	pick up retry time for failures
	 *	global variable Retrytime is set here
	 */
	if ((p = rindex(s, ';')) != NULL)
	    if (isdigit(p[1])) {
		if (sscanf(p+1, "%d", &Retrytime) < 1)
			Retrytime = 5;	/* 5 minutes is error default */
		Retrytime  *= 60;
		*p = NULLCHAR;
	    }

	while (*s) {
		int	i, dayok;

		for (dayok = 0; (!dayok) && isalpha(*s); s++) {
			if (PREFIX("Any", s))
				dayok = 1;
			else if (PREFIX("Wk", s)) {
				if (tp->tm_wday >= 1 && tp->tm_wday <= 5)
					dayok = 1;
			} else
				for (i = 0; days[i]; i++)
					if (PREFIX(days[i], s))
						if (tp->tm_wday == i)
							dayok = 1;
		}

		if (dayok) {
			int	t__low, t__high;

			while (isalpha(*s))	/* flush remaining day stuff */
				s++;

			if ((sscanf(s, "%d-%d", &t__low, &t__high) < 2)
			 || (t__low == t__high))
				return(1);

			/* 0000 crossover? */
			if (t__low < t__high) {
				if (t__low <= t__now && t__now <= t__high)
					return(1);
			} else if (t__low <= t__now || t__now <= t__high)
				return(1);

			/* aim at next time slot */
			if ((s = index(s, ',')) == NULL)
				break;
		}
		s++;
	}
	return(0);
}

/***
 *	char *
 *	fdig(cp)	find first digit in string
 *
 *	return - pointer to first digit in string or end of string
 */

char *
fdig(cp)
char *cp;
{
	char *c;

	for (c = cp; *c; c++)
		if (*c >= '0' && *c <= '9')
			break;
	return(c);
}


#ifdef FASTTIMER
/*	Sleep in increments of 60ths of second.	*/
nap (time)
register int time;
{
	static int fd;

	if (fd == 0)
		fd = open (FASTTIMER, 0);

	read (fd, 0, time);
}

#endif

#if ! defined FASTTIMER && defined BSD4_2

	/* nap(n) -- sleep for 'n' ticks of 1/60th sec each. */
	   than 1 second, so we use BSD gettimeofday() to make
	   sure that we have paused long enough. - jjhnsn */
	/* Previously, this function used select(), but
	   select rounds to the nearest second on AIX V2
	   so it didn't pause at all. - jjhnsn */

nap(n)
unsigned n;
{
	struct timeval tim;
	struct timezone timz;
	int sec;
	int start_sec;
	int start_usec;
	int goal_usec;
	int nap_time;
	int t_usec = 0;

	CDEBUG(5, "nap(%d) ", n);
	if (n == 0)  {
		CDEBUG(9, "nap paused for %d cycles.\n", n);
		return;
	}
	/* get the starting time */
	timz.tz_minuteswest = 0;  /* we don't care about time zones */
	timz.tz_dsttime = 0;
	if (gettimeofday(&tim, &timz)) {
		CDEBUG(5,"gettimeofday(), errno = %d.\n", errno);
		start_sec = 0;
	}
	else  {
		start_sec = tim.tv_sec;
		start_usec = tim.tv_usec;
	}
	sec = ( n / HZ );	/* calc. seconds to sleep */
	if (sec)  {
		sleep(sec);
		if ((n % HZ) == 0)
			start_sec = 0;
	}
	if(start_sec == 0)
		return;
	goal_usec = (((2 * n) - 1) * 1000000L + HZ) / (2 * HZ) + tim.tv_usec;
	while (goal_usec > t_usec)  {
		if (gettimeofday(&tim,&timz))  {
			CDEBUG(5,"gettimeofday(), errno = %d.\n", errno);
			t_usec = goal_usec;
		}
		else
			t_usec = tim.tv_usec + 
				(1000000L * (tim.tv_sec - start_sec));
	}
	nap_time = (HZ * (t_usec - start_usec) + 500000L) / 1000000L;
	return;
}

#endif

#if ! defined  FASTTIMER && !  defined BSD4_2

/*	nap(n) where n is ticks
 *
 *	loop using n/HZ part of a second
 *	if n represents more than 1 second, then
 *	use sleep(time) where time is the equivalent
 *	seconds rounded off to full seconds
 *	NOTE - this is a rough approximation and chews up
 *	processor resource!
 */

nap(n)
unsigned n;
{
	struct tms	tbuf;
	clock_t endtime;
	int i;

	if (n > HZ) {
		/* > second, use sleep, rounding time */
		sleep( (int) (((n)+HZ/2)/HZ) );
		return;
	}

	/* use timing loop for < 1 second */
	endtime = times(&tbuf) + 3*n/4;	/* use 3/4 because of scheduler! */
	while (times(&tbuf) < endtime) {
	    for (i=0; i<1000; i++, i*i)
		;
	}
	return;
}


#endif /* ! FASTTIMER */
