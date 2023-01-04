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
static char	*sccsid = "@(#)$RCSfile: look.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/06/03 16:27:24 $";
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
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
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
 * look.c      1.15  com/cmd/scan,3.1,9021 3/21/90 17:15:26
 */                                                                   

#include <stdio.h>
#include <locale.h>

#include <NLctype.h>

#include "look_msg.h" 
#define MSGSTR(n,s) NLgetamsg(MF_LOOK, MS_LOOK, n, s) 

static void canon();

FILE *dfile;
char *filenam  = "/usr/share/dict/words";

int fold=0;
int dict=0;
int tab;
#define		BIG_BUF		250
char entry[BIG_BUF];
char word [BIG_BUF];
char key  [BIG_BUF];

main(argc,argv)
char **argv;
int argc;
{
	register int c;
	long top,bot,mid;

	(void) setlocale (LC_ALL,"");

	while(argc>=2 && *argv[1]=='-') {
		for(;;) {
			switch(*++argv[1]) {
			case 'd':
				dict++;
				continue;
			case 'f':
				fold++;
				continue;
			case 't':
				tab = argv[1][1];
				if(tab)
					++argv[1];
				continue;
			case 0:
				break;
			default:
				continue;
			}
			break;
		}
		argc --;
		argv++;
	}
	if(argc<=1)
		usage(); /* 001 - prints usage message if no arguments */
	if(argc==2) {
		dict++;
		fold++;
	}
	else
		filenam = argv[2];
	dfile = fopen(filenam,"r");
	if(dfile==NULL) {
		fprintf(stderr,MSGSTR(CANTOPEN, "look: can't open %s\n"),filenam); /*MSG*/
		exit(2);
	}
	canon(argv[1],key);
	bot = 0;
	fseek(dfile,0L,SEEK_END);
	top = ftell(dfile);
	for(;;) {
		mid = (top+bot)/2;
		fseek(dfile,mid,SEEK_SET);
		do {
			c = getc(dfile);
			mid++;
		} while(c!=EOF && c!='\n');
		if(!getword(entry))
			break;
		canon(entry,word);
		switch(compare(key,word)) {
		case -2:
		case -1:
		case 0:
			if(top<=mid)
				break;
			top = mid;
			continue;
		case 1:
		case 2:
			bot = mid;
			continue;
		}
		break;
	}
	fseek(dfile,bot,SEEK_SET);
	while(ftell(dfile)<top) {
		if(!getword(entry))
			return;
		canon(entry,word);
		switch(compare(key,word)) {
		case -2:
			return;
		case -1:
		case 0:
			puts(entry);
			break;
		case 1:
		case 2:
			continue;
		}
		break;
	}
	while(getword(entry)) {
		canon(entry,word);
		switch(compare(key,word)) {
		case -1:
		case 0:
			puts(entry);
			continue;
		}
		break;
	}
	exit(0);
}

/* 001 - added usage function to print usage message */
usage()
{
	fprintf(stderr,MSGSTR(USAGE, "Usage: look [-df] [-tcharacter] String [File...]\n"));
	exit(2);
}

/*
 *  NAME:  compare
 *
 *  FUNCTION:  string compare two strings.
 *	      
 *  RETURN VALUE:  	 0   - strings are equal
 *			 1   - t < s
 *			-1   - s < t
 *			-2   - t < s
 *			 2   - s < t
 */

compare(s,t)
register char *s,*t;
{
	NLchar sc, tc;
	short scu, tcu;		/* uniq collating values */

	for (;;) {
		if (!*s && !*t)
			return(0);
		if (!*s)
			return(-1);
		if (!*t)
			return(1);

		s += NCdecode(s, &sc);
		t += NCdecode(t, &tc);
#ifdef OLD
		if (NCcolval(sc) != NCcolval(tc))
			return( (NCcolval(sc) < NCcolval(tc)) ? -2 : 2 );
#else
		if ((scu = colval(sc)) != (tcu = colval(tc)))
			return( (scu < tcu) ? -2 : 2 );
#endif
	}
}

#ifndef OLD
colval(c)		/* get uniq colval for c */
wchar_t c;
{
	char buf[3], *bp = buf;
	short int cu, co, tcu;

	buf[0] = buf[1] = buf[2] = '\0';
	_NCe2(c,buf[0], buf[1]);
	cu = NCcoluniq(c);
	if ( ((co = NCcollate(c) < 0 )) &&
	      (co = _NLxcolu(co, buf, 0, &tcu)) ) ;
	return (int)cu;
}
#endif

/*
 *  NAME:  	getword
 *
 *  FUNCTION:  	Read up until a new line character or EOF is found from
 *		file "dfile".  Characters read in are placed in memory
 *		pointed to by the first parameter.
 *
 *  RETURN VALUE: 	0 is end-of-file
 *			1 a string has been read in
 */

getword(w)
char *w;
{
	register int c;
	int len=BIG_BUF;
	for(;;) {
		c = getc(dfile);
		if(c==EOF)
			return(0);
		if(c=='\n')
			break;
		if (--len == 0) {
			printf (MSGSTR(TOOLONG,"Line too long.  Truncating\n"));
			break;
		}
			
		*w++ = c;
	}
	*w = 0;
	return(1);
}

/*
 *  NAME:  canon
 *
 *  FUNCTION:	Take a given string and set the pointer (new)
 *		to the first word (null terminates it).
 *
 *  RETURN VALUE:  None
 *
 */

static void
canon(old,new)
char *old,*new;
{
	register int c;
	int len=BIG_BUF;
	NLchar	k;
	int	dbl=0;

	for(;;) {
		if (NCisshift(*old)) {
			dbl++;
			old += NCdecode (old,&k);
			c = k;
		}
		else *new = c = *old++;
		if (--len <= 0)
		{
			printf (MSGSTR(WORDFILE,"Wordfile line too long. Truncating\n"));
			*new = 0;
			break;
		}
		if(c==0||c==tab) {
			*new = 0;
			break;
		}

		if(dict) {

			if(!NCisalnum((NLchar)c)) {
				dbl=0;
				continue;
			}
		}
		if(fold) {
			if(NCisupper((NLchar)c))
				*new = k = NCtolower((NLchar)c); 
		}

		if (dbl) {
			new += NCencode (&k,new);
			dbl = 0;
		}
		else new++;
	}
}
