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
static char	*sccsid = "@(#)$RCSfile: fgrep.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/11 17:08:22 $";
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

/* Modification History:
   01 Piyanai (6/25/91) Fix the problem with 
              "fgrep -[i,y] UPPERCASE_STRING file" like command 
              when if "string" is upper case this command will not work. 
*/

#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 26, 27
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
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 * 
 * fgrep.c	1.11  com/cmd/scan,3.1,9013 3/3/90 12:45:10
 */

/*
 * Search for all lines matching particular fixed patterns.
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */                                                                   


#include <stdio.h>
#include <sys/param.h>
#include <limits.h>
#include <ctype.h>
#include <nl_types.h>
#include "fgrep_msg.h"
#include <locale.h>
#include <NLctype.h>

#define	MSGstr(Num,Str) catgets(catd,MS_FGREP,Num,Str)
nl_catd	catd;
#define	MAXSIZ 6000
#define QSIZE 400

struct words {
	char 	inp;
	char	out;
	struct	words *nst;
	struct	words *link;
	struct	words *fail;
} w[MAXSIZ], *smax, *q;

long	lnum;
int	bflag, 		/* Print out block numbers */
	cflag, 		/* count matching lines    */
	lflag, 		/* list file names only    */
	fflag, 		/* get pattern from file   */
	nflag, 		/* print line numbers      */
	vflag, 		/* oppcase. everything but */
	xflag, 		/* match entire line       */
	eflag,		/* pattern specified       */
	iflag,		/* ignore case             */
	sflag,		/* print only errors       */
	hflag = 1;	/* don't print file names  */
int	retcode = 0;
int	Debugit=0;	/* turn debugging on       */
int	nfile;
long	blkno;
int	nsucc;
long	tln;
FILE	*wordf;
char	*argptr;
char 	low_case_buf[MAX_INPUT];	/* for the -i option */
int twochr[2];		/* Used for two character NLS and KJI conversions. */
int buffered_char=0;	/* flag two character NLS and KJI conversion.      */

extern	char *optarg;
extern	int optind;
static void	cgotofn();
static void	cfail();
static void	execute();

main(argc, argv)
char **argv;
int argc;
{
	register int c;
	char *usage;
	int errflg = 0;
	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_FGREP,NL_CAT_LOCALE);
	usage = MSGstr(USAGE,"Usage: fgrep [ -hsbcilnvx ] [ -e exp ] [ -f file ] [ strings ] [ file ] ...\n");

	while(( c = getopt(argc, argv, "Dhsbciye:f:lnvx")) != EOF)
		switch(c) {

		case 'h':		/* Don't print file names */
			hflag = 0;
			continue;

		case 's':		/* Silent mode            */
			sflag++;
			continue;

		case 'b':		/* print block numbers    */
			bflag++;
			continue;

		case 'y':
		case 'i':		/* ignore case            */
			iflag++;
			continue;

		case 'c':		/* count matches          */
			cflag++;
			continue;

		case 'e':		/* specify a pattern      */
			eflag++;
			argptr = optarg;
			continue;

		case 'f':		/* patterns are in a file */
			fflag++;
			wordf = fopen(optarg, "r");
			if (wordf==NULL) {
				fprintf(stderr, MSGstr(BADFILE,"fgrep: can't open %s\n"), optarg);
				exit(2);
			}
			continue;

		case 'l':		/* list file names once   */
			lflag++;
			continue;

		case 'n':		/* print line numbers     */
			nflag++;
			continue;

		case 'v':		/* the NOT case           */
			vflag++;
			continue;

		case 'x':		/* must match entire line */
			xflag++;
			continue;

		case 'D':		/* for debuging purposes  */
			Debugit++;
			continue;

		case '?':
			errflg++;
	}

	argc -= optind;
	if (errflg || ((argc <= 0) && !(fflag || eflag))) {
		fprintf(stderr, usage); /*MSG*/
		exit(2);
	}
	if ( !eflag  && !fflag ) {
		argptr = argv[optind];
		optind++;
		argc--;
	}
	cgotofn();
	cfail();
	nfile = argc;
	argv = &argv[optind];
	if (argc<=0) {
		if (lflag) exit(1);
		execute((char *)NULL);
	}
	else
		while ( --argc >= 0 ) {
			execute(*argv);
			argv++;
		}
	exit(retcode != 0 ? retcode : nsucc == 0);
}


/*
 * NAME: execute
 *                                                                    
 * FUNCTION:	For each line of the passed filename, search for a pattern
 *		using the threaded list array w.  The line number,
 *		offset, number of matches and file names are a tracked
 *		in order to print out user requested information.
 *		
 * DATA STRUCTURES: nsucc is modified if a match is found.
 *
 */  

static void
execute(file)
char *file;
{
	register char *p;
	register struct words *c;
	char buf[MAX_INPUT];
	FILE *f;
	int align;
	int line_lgth;						/* 002 */
	if (Debugit) print_array();
	if (file) {
		if ((f = fopen(file, "r")) == NULL) {
			fprintf(stderr, MSGstr(BADFILE,"fgrep: can't open %s\n"), file);
			retcode = 2;
			return;
		}
	}
	else f = stdin;
	lnum = 1;
	tln = 0;
	c = w;
	while ((p = fgets (buf,MAX_INPUT,f)) != NULL)
	{
		line_lgth = NLstrlen (buf);			/* 002 */
		if (buf [line_lgth-1] == '\n')			/* 002 */
			buf [line_lgth-1] = 0;			/* 002 */
		if (bflag)
			blkno = ftell (f);
		if (iflag)
		{
			int cnt;
			char twochr[2];
			NLchar tmp;
			char *p1;
			p1 = low_case_buf;
			while (*p != '\0')
			{
				twochr[0] = *p++;
				if (NCisshift((int)twochr[0]))
					twochr[1] = *p++;
				NCdecode (twochr,&tmp);
				if (NCisupper((int) tmp))
					tmp = _NCtolower((int) tmp);
				cnt = NCencode (&tmp, twochr);
				*p1++ = twochr[0];
				if (cnt == 2)
					*p1++ = twochr[1];
			}
			*p1 = *p;
			p=low_case_buf;
		}

		while (!c->out && *p) 
		{
			while (!c->out && c->inp == *p)
			{
				p++;
				align=1;
				c = c->nst;
			}
			if (!c->out)
			{
				if (xflag && c->inp == '\n' && (*p == '' )){
					c = c->nst;
					break;
				}
				if (c->link)
					c = c->link;	/* try next pattern */
				else if (c -> fail)
				{	
					if (xflag) break;
					c = c -> fail;
				}
				else
				{
					if (xflag) break;
					c = w;
					if (align) 
						align = 0;
					else
						p++;
				}
			}
		}
		if (c->out ^ vflag)	/* if either is set but not both */
		   {
			nsucc = 1;
			if (cflag) tln++;
			else if (sflag)
				;	/* great!	*/
			else if (lflag) {
				printf("%s\n", file);
				fclose(f);
				return;
			}
			else {
				if (nfile > 1 && hflag) printf("%s:", file);
				if (bflag) printf("%d:", (blkno + p - 
					(iflag ? low_case_buf : buf))/DEV_BSIZE);
				if (nflag) printf("%ld:", lnum);
				printf ("%s\n",buf);		/* 002 */
			}
		     }
		lnum++;
		c = w;
	}
	fclose(f);
	if (cflag) {
		if (nfile > 1)
			printf("%s:", file);
		printf("%ld\n", tln);
	}
}


/*
 * NAME: getargc
 *                                                                    
 * FUNCTION:	Return the next character in the pattern definition.
 *		This is obtained from either a file (-f option) or
 *		the argument parameter.
 *
 * RETURN VALUE DESCRIPTION: 
 *		The next character or EOF if non are available.
 *			    
 */  

int
getargc()
{
	NLchar	tmp;
	int	cnt;
	if (buffered_char)
	{
		buffered_char--;
		return (twochr[1]);
	}
	if (wordf)
		twochr[0] = getc (wordf);
	else
		if ((twochr[0] = *argptr++) == '\0')
			twochr[0] = EOF;
	if (iflag)
	{
		if (NCisshift (twochr[0]))
		{
			if (wordf)
				twochr[1] = getc (wordf);
			else
				twochr[1] = *argptr++;
		}
		NCdecode (twochr,&tmp);
		if (NCisupper ((int) tmp))
			tmp = _NCtolower ((int) tmp);
		cnt = NCencode (&tmp,twochr);        /* 01 */
		if (cnt == 2)
			buffered_char++;
	}
	return (twochr[0]);
}

/*
 * NAME: cgotofn
 *                                                                    
 * FUNCTION: 	Build a threaded list of patterns to search for.
 *		Each pattern ends with a 1 in the out field.
 */  

static void
cgotofn() {
	register int c;
	register struct words *s;

	s = smax = w;
nword:	for(;;) {
		c = getargc();
		if (c==EOF)
			return;
		if (c == '\n') {
			if (xflag) {
				for(;;) {
					if (s->inp == c) {
						s = s->nst;
						break;
					}
					if (s->inp == 0) goto nenter;
					if (s->link == 0) {
						if (smax >= &w[MAXSIZ -1]) overflo();
						s->link = ++smax;
						s = smax;
						goto nenter;
					}
					s = s->link;
				}
			}
			s->out = 1;
			s = w;
		} else {
		loop:	if (s->inp == c) {
				s = s->nst;
				continue;
			}
			if (s->inp == 0) goto enter;
			if (s->link == 0) {
				if (smax >= &w[MAXSIZ - 1]) overflo();
				s->link = ++smax;
				s = smax;
				goto enter;
			}
			s = s->link;
			goto loop;
		}
	}

	enter:
	do {
		s->inp = c;
		if (smax >= &w[MAXSIZ - 1]) overflo();
		s->nst = ++smax;
		s = smax;
	} while ((c = getargc()) != '\n' && c!=EOF);
	if (xflag) {
	nenter:	s->inp = '\n';
		if (smax >= &w[MAXSIZ -1]) overflo();
		s->nst = ++smax; 
	}
	smax->out = 1;
	s = w;
	if (c != EOF)
		goto nword;
}

overflo() {
	fprintf(stderr,MSGstr(OVERFLO,"wordlist too large\n"));
	exit(2);
}

/*
 * NAME: cfail
 *                                                                    
 * FUNCTION:	This subroutine finishes threading the list so one can
 *		efficiently search through for a match.  If you 
 *		are searching down a particular thread and you don't
 *		find a match, the fail path can be followed.
 *                                                                    
 */  

static void
cfail() {
	struct words *queue[QSIZE];
	struct words **front, **rear;
	struct words *state;
	register char c;
	register struct words *s;
	s = w;
	front = rear = queue;
init:	if ((s->inp) != 0) {
		*rear++ = s->nst;
		if (rear >= &queue[QSIZE - 1]) overflo();
	}
	if ((s = s->link) != 0) {
		goto init;
	}

	while (rear!=front) {
		s = *front;
		if (front == &queue[QSIZE-1])
			front = queue;
		else front++;
	cloop:	if ((c = s->inp) != 0) {
			*rear = (q = s->nst);
			if (front < rear)
				if (rear >= &queue[QSIZE-1])
					if (front == queue) overflo();
					else rear = queue;
				else rear++;
			else
				if (++rear == front) overflo();
			state = s->fail;
		floop:	if (state == 0) state = w;
			if (state->inp == c) {
			qloop:	q->fail = state->nst;
				if ((state->nst)->out == 1) q->out = 1;
				if ((q = q->link) != 0) goto qloop;
			}
			else if ((state = state->link) != 0)
				goto floop;
		}
		if ((s = s->link) != 0)
			goto cloop;
	}
}

/* Used for debugging only.  Prints out the threaded array where the
 * patterns are stored.  The undocumented -D option will do this.
 */
print_array()
{
	int i;
	printf ("addr\t index\tchar\tmatch\t\tnst\tlink\tfail\n");
	for (i=0;&w[i]<smax;i++)
	{
		printf ("%x [%x]:\t%c\t %x\t %x\t %x\t %x\n",&w[i],i,w[i].inp,
					w[i].out,w[i].nst,w[i].link,w[i].fail);
	
	}
}
