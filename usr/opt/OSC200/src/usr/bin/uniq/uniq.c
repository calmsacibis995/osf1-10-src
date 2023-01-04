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
static char rcsid[] = "@(#)$RCSfile: uniq.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/10/11 19:30:52 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: uniq
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.10  com/cmd/files/uniq.c, bos320,9130320g 7/23/91 10:02:27
 */

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <stdlib.h>
#include "uniq_msg.h"

#define MSGSTR(n,s)	catgets(catd,MS_UNIQ,n,s)

/* For <blank> detection: define macros for isblank() for SBCS and
 * iswblank() for wchar with MBCS. POSIX defines function of -f option
 * in terms of <blank> character class. Standard bindings do not provide
 * isblank() or iswblank(), so uniq must provide its own.
 */
static wctype_t blank_wctype;
#define isblank(c)    iswctype(c, blank_wctype)
#define iswblank(wc)  iswctype(wc, blank_wctype)

int mb_cur_max; /* max number of bytes per character in current locale */
int mbcodeset;  /* 0=current locale SBCS, 1=current locale MBCS */

nl_catd catd;

int	fields = 0;
int	letters = 0;
int	repeats = 0;
int	cflag, dflag, uflag;
char	*skip();
FILE	*file();

/*
 * NAME: uniq [-cdu] [-f m] [-s n] [+n] [-m]] [InFile [OutFile]]
 *                                                                    
 * FUNCTION: Deletes repeated lines in a file.  Repeated lines must be
 *           in consecutive lines in order for them to be found.
 *         -c     Precedes each output line with the number of times it
 *                appear in the file.
 *         -d     Displays only the repeated lines.
 *         -u     Displays only the unrepeated lines.
 *	   -f num Skips over the first num fields.
 *         -num   Equivalent to  -f num.
 *         -s num Skips over the first num characters.
 *         +num   Equivalent to  -s num.
 *                                                                    
 */  
main(argc, argv)
int argc;
char *argv[];
{
	FILE *ifp, *ofp; /* input and output file pointers */
	char *arg;	/* next option or operand string */
	int c;		/* option character */
	int numval;	/* numeric value of option value */
	char *cp;	/* for processing numeric option values */
	char optname[] = "-?";	/* for diagnostics */
	char *prevline, *thisline;
	register char *t1, *t2;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_UNIQ,NL_CAT_LOCALE);
	mb_cur_max = MB_CUR_MAX;
	blank_wctype = wctype("blank");
	mbcodeset = (mb_cur_max > 1);

		/* Command parsing: follow POSIX guidelines and also
		 * allow old POSIX "obsolescent" versions.
		 */
	do {
		/* Two-stage processing of each potential option character:
		 * 1. If it is the first character of an obsolescent option
		 *    that does not follow getopt() conventions, process it
		 *    manually and update getopt() pointers to next possible
		 *    option character.
		 * 2. Otherwise process it through getopt() for normal
		 *    option processing.
		 */
		if(optind > argc)
			c = EOF;
		else {
			arg = argv[optind];
			c = arg ? arg[0]: EOF;
		};
		if ( c == '+') {
			/* Part 1 of 3: Obsolescent +m option */
			cp = &arg[1];
			numval = (int)strtoul(cp,&cp,10);
			if (*cp == '\0')
				letters = numval;			
			else {
				fprintf(stderr,MSGSTR(BADNUM,"uniq: bad number for %s\n"),"+");
				usage();
			}
			optarg = argv[optind++];
		} else if (c == '-' && strlen(arg) > 1 && iswdigit((wchar_t)arg[1])) {
			/* Part 2 of 3: Obsolescent -n option */
			cp = &arg[1];
			numval = (int)strtoul(cp,&cp,10);
			if (*cp == '\0')
				fields = numval;			
			else {
				fprintf(stderr,MSGSTR(BADNUM,"uniq: bad number for %s\n"),"-");
				usage();
			}
			optarg = argv[optind++];
		} else {
			/* Part 3 of 3: Normal POSIX command syntax option */
			/* Implemented to POSIX 1003.2/Draft 10 conventions*/
			c = getopt(argc,argv,"cdf:us:");
			switch (c) {
			case 'c':
				cflag = 1;
				break;
			case 'd':
				dflag = 1;
				break;
			case 'u':
				uflag = 1;
				break;
			case 'f':
			case 's':
				cp = optarg;
				numval = (int)strtoul(cp,&cp,10);
				if (*cp == '\0')
					if (c == (int)'f') fields = numval;
					else letters = numval;
				else {
					optname[1] = (char)c;
					fprintf(stderr,MSGSTR(BADNUM,
						"uniq: bad %s number\n"),optname);
					usage();
				}
				break;
			case EOF:
				break;
			default:
				usage();
			} /* end switch(c) */
		} /* end POSIX syntax option */
	} /* end do */
	while (c != EOF);

	/* if no flags are set, default is -d -u */
	if (cflag) {
		if (dflag || uflag)
			dflag = uflag = 0;
	} else if (!dflag && !uflag)
		dflag = uflag = 1;

	argc -= optind;
	argv += optind;

	switch(argc) {
		case 0:
			ifp = stdin;
			ofp = stdout;
			break;
		case 1:
			ifp = file(argv[0], "r");
			ofp = stdout;
			break;
		case 2: 
			ifp = file(argv[0], "r");
			ofp = file(argv[1], "w");
			break;
		default:
			fprintf(stderr,
			       MSGSTR(BADOPT,"uniq: too many files\n"),optname);
			usage();
	} 

	prevline = malloc(LINE_MAX + 2);
	thisline = malloc(LINE_MAX + 2);

	(void)fgets(prevline, LINE_MAX, ifp);

	while (fgets(thisline, LINE_MAX, ifp)) {
		/* if requested get the chosen fields + character offsets */
		if (fields || letters) {
			t1 = skip(thisline);
			t2 = skip(prevline);
		} else {
			t1 = thisline;
			t2 = prevline;
		}

		/* if different, print; set previous to new value */
		if (strcmp(t1, t2)) {
			show(ofp, prevline);
			t1 = prevline;
			prevline = thisline;
			thisline = t1;
			repeats = 0;
		}
		else
			++repeats;
	}
	show(ofp, prevline);
	exit(0);
}

/*
 * show --
 *	output a line depending on the flags and number of repetitions
 *	of the line.
 */
show(ofp, str)
	FILE *ofp;
	char *str;
{
	if (cflag)
		(void)fprintf(ofp, "%4d %s", repeats + 1, str);
	if (dflag && repeats || uflag && !repeats)
		(void)fprintf(ofp, "%s", str);
}

/*
 * NAME: skip
 *                                                                    
 * FUNCTION:  Skip num fields or num characters.
 *
 * RETURN VALUE DESCRIPTION:  return the rest of the string.
 *   
 */  
char *
skip(s)
char *s;
{
	int nf, nl;
	wchar_t ws;
	wchar_t *wsp = &ws;
	int chrlen;

	nf = nl = 0;
	if (mbcodeset) { /* skip fields (-f) and letters (-s) in MBCS */
		while (nf++ < fields) {
			for (;;) {
				chrlen = mbtowc(wsp,s,MB_CUR_MAX);
				if (chrlen == 0 || ws == '\0')
					return(s);
				s += chrlen;
				if (!iswblank(ws))
					break;
			}
			for (;;) {
				chrlen = mbtowc(wsp,s,MB_CUR_MAX);
				if (chrlen == 0 || ws == '\0')
					return(s);
				if (iswblank(ws))
					break;
				s += chrlen;
			}
		}
		while (nl++ < letters) {
			chrlen = mbtowc(wsp,s,MB_CUR_MAX);
			if (chrlen == 0 || ws == '\0')
				break;
			s += chrlen;
		}
	} else {	 /* skip fields (-f) and letters (-s) in SBCS */
		while (nf++ < fields) {
			for (;;) {
				if (*s == '\0')
					return(s);
				if (!isblank(*s++))
					break;
			}
			for (;;) {
				if (*s == '\0')
					return(s);
				if (isblank(*s))
					break;
				s++;
			}
		}
		while (*s != 0 && nl++ < letters)
			s++;
	}
	return(s);
}

/*
 * NAME: file
 *                                                                    
 * FUNCTION:  Open file with requested mode.
 *
 * RETURN VALUE DESCRIPTION:  a pointer to the open file
 * 
 */
FILE *
file(name, mode)
	char *name, *mode;
{
	FILE *fp;

	if ((*mode == 'r') && (strcmp(name, "-") == 0))
		return(stdin);

	if (!(fp = fopen(name, mode))) {
		(void)fprintf(stderr, MSGSTR(OPERR, "uniq: can't open %s.\n"), 
			      name);
		exit(1);
	}
	return(fp);
}

/*
 * NAME: usage
 *                                                                    
 * FUNCTION:  Print usage message and exit.
 *
 * RETURN VALUE DESCRIPTION:  none
 */
usage()
{
	(void)fprintf(stderr,MSGSTR(USAGE,
"Usage: uniq [-cdu] [-f Fields] [-s Chars] [-Fields] [+Chars] [InFile [OutFile]]\n"));
	exit(1);
}
