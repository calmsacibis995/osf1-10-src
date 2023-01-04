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
static char	*sccsid = "@(#)$RCSfile: copy.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/04/10 15:56:25 $";
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
 * DEC HISTORY
 *	10/03/91 - Tom Peterson
 *		added changes from OSF/1 1.0.2 code which avoid NULL pointer
 *		referencing which caused seg. fault and core dump.
 *		See section marked by comment containing 001.
 *
 * PREVIOUS HISTORY
 */

/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: copy, pgets, trim, scopy, cmp, wordb, unhook, backstep, fcopy 
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * copy.c	1.3  com/cmd/man/learn,3.1,9021 11/28/89 16:10:29
 */

#include "stdio.h"
#include "signal.h"
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

char togo[50];
char last[LEN_MAX];
char logf[LEN_MAX];
char subdir[LEN_MAX];
extern char *NLctime();
#define CTIME NLctime
extern int review;
int noclobber;

copy(prompt, fin)
int prompt;
FILE *fin;
{
	FILE *fout, *f;
	char s[MAX_LEN], t[LEN_MAX], s1[LEN_MAX], nm[LEN_L];
	char *r, *tod, c;
	int *p, tv[2];
	extern int intrpt(void), *action();
	extern char *wordb();
	int nmatch = 0;
	long mark;

	if (subdir[0]==0)
		sprintf(subdir, "%s/%s", dname, sname);
	for (;;) {
		if (pgets(s, prompt, fin) == 0)
			if (fin == stdin) {
				fprintf(stderr, MSGSTR(LTYPEBYE, "Type \"bye\" if you want to leave learn.\n")); /*MSG*/
				fflush(stderr);
				clearerr(stdin);
				continue;
			} else
				break;
		trim(s);		/* trim newline */
		/* change the sequence %s to lesson directory */
		/* if needed */
		for (r = s; *r; r++)
			if (*r == '%') {
				sprintf(s1, s, subdir, subdir, subdir);
				strcpy(s, s1);
				break;
			}
		r = wordb(s, t);	/* t = first token, r = rest */
		p = action(t);		/* p = token class */
		if (p && *p == ONCE) {	/* some actions done only once per script */
					/* 001 */
					/* checks p first - avoids NULL referencing */ 
			if (wrong && !review) {	/* we are on 2nd time */
				scopy(fin, NULL);
				continue;
			}
			strcpy(s, r);
			r = wordb(s, t);
			p = action(t);
		}
		if (p == 0) {
			if (comfile >= 0) {	/* if #pipe in effect ... */
				write(comfile, s, strlen(s));
				write(comfile, "\n", 1);
			}
			else {		/* else must be UNIX command ... */
				signal(SIGINT, SIG_IGN);
				status = mysys(s);
				signal(SIGINT, (void (*)(int))intrpt);
			}
			if (incopy) {
				fprintf(incopy, "%s\n", s);
				strcpy(last, s);
			}
			continue;
		}
		switch (*p) {
		case READY:
			if (incopy && r) {
				fprintf(incopy, "%s\n", r);
				strcpy(last, r);
			}
			return;
		case PRINT:
			if (wrong)
				scopy(fin, NULL);	/* don't repeat message */
			else if (r)
				list(r);
			else
				scopy(fin, stdout);
			break;
		case HINT:
			mark = ftell(scrin);
			if (r)
				rewind(scrin);
			while ((int)(c=fgetc(scrin)) != EOF)
				putchar(c);
			fflush(stdout);
			fseek(scrin, mark, 0);
			break;
		case NOP:
			break;
		case MATCH:
			if (nmatch > 0)	/* we have already passed */
				scopy(fin, NULL);
			else if ((status = STRCMP(r, last)) == 0) {	/* did we pass this time? */
				nmatch++;
				scopy(fin, stdout);
			} else
				scopy(fin, NULL);
			break;
		case BAD:
			if (STRCMP(r, last) == 0) {
				scopy(fin, stdout);
			} else
				scopy(fin, NULL);
			break;
		case SUCCEED:
			scopy(fin, (status == 0) ? stdout : NULL);
			break;
		case FAIL:
			scopy(fin, (status != 0) ? stdout : NULL);
			break;
		case CREATE:
			if (noclobber)
				fout = NULL;
			else
				fout = fopen(r, "w");
			scopy(fin, fout);
			if (!noclobber)
				fclose(fout);
			break;
		case CMP:
			status = cmp(r);	/* contains two file names */
			break;
		case MV:
			sprintf(nm, "%s/L%s.%s", subdir, todo, r);
			fcopy(r, nm);
			break;
		case USER:
		case NEXT:
			if (noclobber)
				noclobber = 0;
			more = 1;
			return;
		case AGAIN:
			review = 0;
			if (!r) {
				r = todo;
				noclobber = 1;
				review = 1;
			}
			again = 1;
			strcpy(togo, r);
			unhook();
			return;
		case SKIP:
			skip = 1;
			unhook();
			return;
		case COPYIN:
			incopy = fopen(".copy", "w");
			break;
		case UNCOPIN:
			fclose(incopy);
			incopy = NULL;
			break;
		case COPYOUT:
			teed = maktee();
			break;
		case UNCOPOUT:
			untee();
			teed = 0;
			break;
		case PIPE:
			comfile = makpipe();
			break;
		case UNPIPE:
			close(comfile);
			wait(0);
			comfile = -1;
			break;
		case YES:
		case NO:
			if (incopy) {
				fprintf(incopy, "%s\n", s);
				strcpy(last, s);
			}
			return;
		case WHERE:
			PRINTF(MSGSTR(LINLESSON, "You are in lesson %s of \"%s\" with a speed rating of %d.\n"), todo, sname, speed); /*MSG*/
			PRINTF(MSGSTR(LHAVECOMPLETED, "You have completed %d out of a possible %d lessons.\n"), sequence-1, total); /*MSG*/
			if (r)
				tellwhich();
			fflush(stdout);
			break;
		case BYE:
			more=0;
			return;
		case CHDIR:
			printf(MSGSTR(LNOCD, "cd not allowed\n")); /*MSG*/
			fflush(stdout);
			break;
		case LEARN:
			printf(MSGSTR(LALRDYLRN, "You are already in learn.\n")); /*MSG*/
			fflush(stdout);
			break;
		case LOG:	/* logfiles should be created mode 666 */
			if (!logging)
				break;
			if (logf[0] == 0)
				sprintf(logf, "%s/log/%s", dname, sname);
			f = fopen((r ? r : logf), "a");
			if (f == NULL)
				break;
			time(tv);
			tod = CTIME(tv);
			tod[24] = 0;
			if (status)
				FPRINTF(f, MSGSTR(LFAIL, "%s L%-6s fail %2d %s\n"), tod, /*MSG*/
				todo, speed, pwline);
			else
				FPRINTF(f, MSGSTR(LPASS, "%s L%-6s pass %2d %s\n"), tod, /*MSG*/
				todo, speed, pwline);
			fclose(f);
			break;
		}
	}
	return;
}

pgets(s, prompt, f)
char *s;
int prompt;
FILE *f;
{
	if (prompt) {
		if (comfile < 0)
			fputs("% ", stdout);
		fflush(stdout);
	}
	if (fgets(s, MAX_LEN,f))
		return(1);
	else
		return(0);
}

trim(s)
char *s;
{
	char *old = s;

	while (*s)
		s++;
	backstep(old, &s);
	if (*s == '\n')
		*s=0;
}

scopy(fi, fo)	/* copy fi to fo until a line with #
		 * sequence "#\n" means a line not ending with \n
		 * control-M's are filtered out */
FILE *fi, *fo;
{
	int c;

	while ((c = getwc(fi)) != '#' && c != EOF) {
		do {
			if (c == '#')   {
				c = getwc(fi);
				if (c == '\n')
					break;
				if (c == EOF)   {
					if (fo != NULL)
						fflush(fo);
					return;
				}
				if (fo != NULL)
					putc('#', fo);
			}
			if (c == '\r')
				break;
			if (fo != NULL)
				putwc((NLchar)c, fo);
			if (c == '\n')
				break;
		} while ((c = getwc(fi)) != EOF);
	}
	if (c == '#')
		ungetwc((NLchar)c, fi);
	if (fo != NULL)
		fflush(fo);
}

cmp(r)	/* compare two files for status; #cmp f1 f2 [ firstnlinesonly ] */
char *r;
{
	char *s, *h;
	FILE *f1, *f2;
	int c1, c2, stat, n;

	for (s = r; *s != ' ' && *s != '\0'; s++)
		;
	*s++ = 0;	/* r contains file 1 */
	while (*s == ' ')
		s++;
	for (h = s; *h != ' ' && *h != '\0'; h++)
		;
	if (*h) {
		*h++ = 0;
		while (*h == ' ')
			h++;
		n = atoi(h);
	}
	else
		n = 077777;
	f1 = fopen(r, "r");
	f2 = fopen(s, "r");
	if (f1 == NULL || f2 == NULL)
		return(1);	/* failure */
	stat = 0;
	for (;;) {
		c1 = getc(f1);
		c2 = getc(f2);
		if (c1 != c2) {
			stat = 1;
			break;
		}
		if (*h && c1 == '\n')
			if (--n)
				break;
		if (c1 == EOF || c2 == EOF)
			break;
	}
	fclose(f1);
	fclose(f2);
	return(stat);
}

char *
wordb(s, t)	/* in s, t is prefix; return tail */
char *s, *t;
{
	int c;

	while (c = *s++) {
		if (c == ' ' || c == '\t')
			break;
		*t++ = c;
	}
	*t = 0;
	while (*s == ' ' || *s == '\t')
		s++;
	return(c ? s : NULL);
}

unhook()
{
	if (incopy) {
		fclose(incopy);
		incopy = NULL;
	}
	if (comfile >= 0) {
		close(comfile);
		wait(0);
		comfile = -1;
	}
	if (teed) {
		teed = 0;
		untee();
	}
	fclose(scrin);
	scrin = NULL;
}


int backstep(bufstart, bufptr)
char *bufstart;
char **bufptr;
{
	char *p;
	
	if (*bufptr <= bufstart)
		return(NULL);

	*bufptr -= 1;

	if (NCisshift(**bufptr))
		*bufptr -= 1;
	else {
		for (p = *bufptr; (p > bufstart) && NCisshift(p[-1]); --p)
			;

		if ((int)(*bufptr-p) & 1)
			*bufptr -= 1;
	}
	return(1);
}



fcopy(new,old)
char *new, *old;
{
	char b[BUFSIZ];
	int n, fn, fo;
	fn = creat(new, 0666);
	fo = open(old,0);
	if (fo<0) return;
	if (fn<0) return;
	while ( (n=read(fo, b, BUFSIZ)) > 0)
		write(fn, b, n);
	close(fn);
	close(fo);
}
