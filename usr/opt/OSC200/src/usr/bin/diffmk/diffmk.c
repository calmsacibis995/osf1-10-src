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
static char	*sccsid = "@(#)$RCSfile: diffmk.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 16:12:09 $";
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
 *
 * COMPONENT_NAME: (CMDTEXT) Text formatting services
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * diffmk.c	1.2  com/diffmk.d,3.1,8952 12/21/89 13:09:32
 */
#include <stdio.h>
#include <locale.h>


#include <nl_types.h>
#include "diffmk_msg.h"
#define MSGSTR(num,str) catgets(catd,MS_DIFFMK,num,str)  /*MSG*/

#define LBUF 256   /* some size larger than any line */

#define OKRET 0    /* normal return */
#define BADRET 1   /* error return */

/*
 *      diffmk
 *       [-abstring -aestring -cbstring -cestring -dbstring -destring -b]
 *      oldfile newfile outfile
 *
 *      enhanced to allow user-specifiable "diff"
 *      and to insert the marks itself rather than writing an "ed" script.
 */

char    *code[6] = {    /* code names for mark strings, correspond to mark */
	"ab",		/* append begin */
	"ae",		/* append end */
	"cb",		/* change begin */
	"ce",		/* change end */
	"db",		/* delete begin */
	"de"		/* delete end */
};

char *mark[6] = {       /* default marks, corresponding to code array */
	".mc |",	/* ab */
	".mc",		/* ae */
	".mc |",	/* cb */
	".mc",		/* ce */
	".mc *",	/* db */
	".mc"		/* de */
};

char    *file1 = NULL;  /* first input, can be stdin */
char    *file2 = NULL;  /* second input */
char    *file3 = NULL;  /* output */
char    *bflag = "";    /* pass null switch */
FILE    *ifile;         /* new file, for changes */
FILE    *dfile;         /* news from bdiff */
FILE    *ofile = stdout;        /* output file, default to filter */
FILE    *popen();
nl_catd catd;		/* message catalog file descriptor */

char    in_line[LBUF];   /* input line buffer */
char    filine[LBUF];   /* file line buffer */
unsigned int lineno;    /* file line number */
char *difname, *getenv(), *getline();
char *CSloca(), *CSlocc();

main(argc,argv)
int argc;
char **argv;
{
	register char *p;
	unsigned int n1, n2;
	int ccode;      /* command code */

	setlocale(LC_ALL, "");

	catd = catopen(MF_DIFFMK,NL_CAT_LOCALE);

	getargs(argc,argv);
	if((difname = getenv("DIFFMARK")) == NULL)
		difname = "diff";
	sprintf(in_line, "exec %s %s %s %s", difname, bflag, file1, file2);
# ifdef DEBUG
	fflush(stdout);
	fprintf(stderr,"Command: %s\n", in_line);
	fflush(stderr);
# endif

	if(file1[0] != '-')
		if(access(file1, 4) != 0) {
			perror(file1);
			exit(BADRET);
		}
	if((ifile = fopen(file2, "r")) == NULL) {
		perror(file2);
		exit(BADRET);
	}
	if(file3 && (ofile = fopen(file3, "w")) == NULL) {
		perror(file3);
		exit(BADRET);
	}
	if((dfile = popen(in_line, "r")) == NULL) {
		perror(in_line);
		exit(BADRET);
	}
	while(getline(in_line, sizeof(in_line), dfile) != NULL) {
		switch(*in_line) {
	case '<': case '>': case '-':
			continue;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
			break;
	default:
			fflush(stdout);
			fprintf(stderr, "??: %s\n", in_line);
			fflush(stderr);
			continue;
		}
		/* get the command code */
		p = CSloca(in_line, "acd");
		ccode = *p++;
		n1 = n2 = atoi(p);
		p = CSlocc(p, ',');
		if(*p++)
			n2 = atoi(p);
# ifdef DEBUG
		fflush(stdout);
		fprintf(stderr, "%s = %c/%d/%d\n", in_line, ccode, n1, n2);
		fflush(stderr);
# endif
		switch (ccode) {
	default:
			fflush(stdout);
			fprintf(stderr, "???: %s\n", in_line);
			fflush(stderr);
			break;
	case 'a':
			readto(n1-1);
			out(0);
			readto(n2);
			out(1);
			break;
	case 'c':
			readto(n1-1);
			out(2);
			readto(n2);
			out(3);
			break;
	case 'd':
			readto(n1);
			out(4);
			out(5);
			break;
		}
	}
	readto((unsigned)-1);   /* read the rest of the input file */
	pclose(dfile);
	exit(OKRET);
}

/* getargs: scan all arguments (6 codes, 2 file names, -b maybe) */
getargs(argc,argv)
int argc;
char **argv;
{
	register char *p,*pc;
	register int i;

	argv++;
	while(--argc) {
		p = *argv++;
		if (*p != '-' || p[1] == 0)
			if(!file1)
				file1 = p;
			else if(!file2)
				file2 = p;
			else
				file3 = p;
		else {
			p++;	/* skip over - */
			if(*p == 'b') {         /* support -b flag */
				bflag = "-b";
				continue;
			}
			for(i = 0;i <= 5; i++) {
				pc = code[i];
				if (*p == *pc && p[1] == pc[1]) {
					mark[i] = p+2;
					break;
				}
			}
		}
	}
	if (file1 == NULL || file2 == NULL) {
		printf(MSGSTR(USAGE, "usage:diffmk [-abX] [-aeX] [-b] [-cbX] [-ceX] [-dbX] [-deX] File1 File2 [File3]\n"));
		exit(1);
	}
	if ((file3 != NULL) && 
	   ((strcmp(file1, file3) == 0) || (strcmp(file2, file3) == 0))) {
		printf(MSGSTR(SAMEFILE, "The output file must be different from the input files.\n"));
		exit(1);
	}
}

/* this function is identical to CSgetl (gets a full line from the file)
   except that it does not break after reading a formfeed               */
char *
getline(s, n, iop)
char *s;
register FILE *iop;
{       register char *cs;
      register c;

      cs = s;
      n--;        /* save one space for terminating NUL */
      for (;;)
      {   if ((c = getc(iop)) < 0)
	  {   if (s == cs)
		  return(0);
	      break;
	  }
	  if (c=='\n')
	      break;
	  if (n > 0)
	  {   *cs++ = c;
	      n--;
	  }
      }
      *cs = '\0';
      return(cs);
}



/* out - put out a string */
out(n)
{
	fprintf(ofile, "%s\n", mark[n]);
}

/* read to appropriate place in input file */
readto(n)
unsigned n;
{
	while(lineno < n && fgets(filine, sizeof(filine), ifile) != NULL) {
		fputs(filine, ofile);
		lineno++;
	}
}



/*
 * CSloca() CSlocc() both are yanked from the (not supported) libIN.
 * They do some special NLS character processing.
 *
 */

#include <NLchar.h>

/*
 * Find the location of one of the charaters in "set" in the string "str"
 */
char *
CSloca(str, set)
	register char *str, *set; {

	if (str && set) {
		while (1) {
			NLchar strch;
			register char *srchset;

			NCdec (str, &strch); /* grab successive characters */

			if (strch == '\0') /* end of string */
				return (str);

			srchset = set;

			while (1) {
				NLchar ch;

				NCdec (srchset, &ch); /* check next character */

				if (ch == '\0') /* end of search set */
				    break;

				if (strch == ch)
				    return (str);

				srchset += NCchrlen (ch);
			}

			str += NCchrlen (strch);
		}
	}
	return str;
}

/*
 * Find the location of the charater value "chr" in the string "str".
 */
char *
CSlocc(str, chr)
	register char *str;
	register chr; {
	register c;

	if (str) {
		if (NCchrlen (chr) == 1) {
			while (c = *str++)
				/* two byte character != one byte character */
				if (NCisshift (c)) {
					if (*str)
						str++; /* eat second byte */
				} else
					if (c == chr)
						break;
		} else {
			while (c = *str++) {
				register char *nstr;
				NLchar ch;

				if (NCisshift (c)) {
					/* get pointer to the NLchar */
					nstr = str-1;
					NCdec (nstr, &ch);
					if (ch == chr)
					    break;
					if (*str)
					    str++; /* next character */
				}
			}
		}
		--str;
	}
	return str;
}
