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
static char rcsid[] = "@(#)$RCSfile: split.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 19:11:56 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <math.h>
#include <sys/limits.h>
#include <sys/dir.h>
#include <sys/file.h>
#include "split_msg.h"

static nl_catd	catd;
#define MSGSTR(id, ds) catgets(catd, MS_SPLIT, id, ds)

/*
 * Definitions 
 */
#define LNCNT	1000		/* defualt number of lines */
#define DEFSUF	2		/* default suffix length */

#define	ERR	-1		/* error value */
#define NO	0		/* no/false */
#define YES	1		/* yes/true */

/*
 * Globals
 */
unsigned long 	line_count = LNCNT;	/* default number of lines per file */
unsigned long	byte_count;		/* byte count to split on */
int		suffix_len = DEFSUF;	/* number of suffix letters */

char 		*prefix;		/* prefix other than 'x' */
char		*filename;		/* input filename */

int 		ifd = ERR;		/* input file descriptor */
int		ofd = ERR;		/* output file descriptor */
int		file_open;		/* if a file open */

char		bfr[BUFSIZ];		/* input buffer */
char		fname[PATH_MAX+1];	/* output file name */

/*
 * Functions
 */
int 	split1(void);
int 	split2(void);
void 	newfile(void);
unsigned long getbytes(char *);
void 	usage(void);
void 	wrerror(void);


/*
 * NAME: split [-num] [-a suffix_len] [file [name]]
 * 	 split [[-l line_count] | [-b n[k|m]]] [-a suffix_len] [file [name]]
 *                                                                    
 * FUNCTION: Splits a file into pieces.  Default line count = 1000
 *           -num   changes the number of lines that go into each file
 *            name  prefix name not longer than NAME_MAX - 2 characters.
 * 	     -b	    number of bytes in each file
 * 
 * RETURN VALUES:  0 if no errors
 *                 1 if error
 */  
int
main(int argc, char **argv)
{
	int c, err=0;
	unsigned long count = 0;
	char *tail;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_SPLIT, NL_CAT_LOCALE);

	while ((c = getopt(argc, argv, "a:b:l:0123456789")) != -1)
	switch (c) {
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9':          /* get number of lines */
			count = (count *10) + (c - '0');
			break;

		case 'a':
			suffix_len = atoi(optarg);
			if (suffix_len == 0) {
				fprintf(stderr,MSGSTR(BADLENGTH,
					"split: Bad suffix length.\n"));
				exit(1);
			}
			break;
		case 'b':
			byte_count = getbytes(optarg);
			break;
		case 'l':
			count = atol(optarg);
			if (count == 0) {
				fprintf(stderr,MSGSTR(BADLINES,
					"split: Bad number of lines\n"));
				exit(1);
			}
			break;
		default:
			usage();
		
	}

	argc -= optind;
	argv += optind;

	if (byte_count && count)	/* can't have -l and -b */
		usage();

	if (count > 0)
		line_count = count;

	switch (argc) {
	case 2:
		prefix = argv[1];
		/* FALL THROUGH */
	case 1:
		filename = argv[0];
		if (strcmp(filename, "-") == 0)
			ifd = STDIN_FILENO;
		break;
	case 0:
		ifd = STDIN_FILENO;		/* default */
		break;
	default:
		usage();
	}


	if (ifd == ERR) {
		if((ifd = open(filename, O_RDONLY, 0)) < 0) {
			fprintf(stderr,MSGSTR(INPOPNER, 
				"split: Cannot open input file\n"));
			exit(1);
		}
	}

	if(prefix == NULL)
		prefix = "x";

	if ((tail = strrchr(prefix, '/')) == NULL)
		tail = prefix;
	else
		tail++;

	if(strlen(tail) + suffix_len > NAME_MAX) {
		fprintf(stderr, MSGSTR(OUTNMSIZ, 
	"split: More than %d characters in output file name\n"),NAME_MAX);
		exit(1); 
	}

	if (byte_count)
		err = split1();
	else
		err = split2();
	exit (err);
}

/*
 * split1 --
 *	split by bytes
 */
int
split1(void)
{
	register long bcnt;
	register int dist, len;
	register char *C;

	for (bcnt = 0;;)
		switch(len = read(ifd, bfr, BUFSIZ)) {
		case 0:
			return(0);
		case ERR:
			perror("split: read");
			return(1);
		default:
			if (!file_open) {
				newfile();
				file_open = YES;
			}
			if (bcnt + len >= byte_count) {
				dist = byte_count - bcnt;
				if (write(ofd, bfr, dist) != dist)
					wrerror();
				len -= dist;
				for (C = bfr + dist; len >= byte_count; len -= byte_count, C += byte_count) {
					newfile();
					if (write(ofd, C, (int)byte_count) != byte_count)
						wrerror();
				}
				if (len) {
					newfile();
					if (write(ofd, C, len) != len)
						wrerror();
				}
				else
					file_open = NO;
				bcnt = len;
			}
			else {
				bcnt += len;
				if (write(ofd, bfr, len) != len)
					wrerror();
			}
		}
}

/*
 * split2 --
 *	split by lines
 */
int
split2(void)
{
	register char *Ce, *Cs;
	register long lcnt;
	register int len, bcnt;

	for (lcnt = 0;;)
		switch(len = read(ifd, bfr, BUFSIZ)) {
		case 0:
			return(0);
		case ERR:
			perror("split: read");
			return(1);
		default:
			if (!file_open) {
				newfile();
				file_open = YES;
			}
			for (Cs = Ce = bfr; len--; Ce++)
				if (*Ce == '\n' && ++lcnt == line_count) {
					bcnt = Ce - Cs + 1;
					if (write(ofd, Cs, bcnt) != bcnt)
						wrerror();
					lcnt = 0;
					Cs = Ce + 1;
					if (len)
						newfile();
					else
						file_open = NO;
				}
			if (Cs < Ce) {
				bcnt = Ce - Cs;
				if (write(ofd, Cs, bcnt) != bcnt)
					wrerror();
			}
		}
}

/*
 * newfile --
 *	open a new file
 */
void
newfile(void)
{
	static long fnum;
	static char *fpnt;
	int i;

	if (ofd == ERR) {			/* first time */
		strcpy(fname, prefix);
		fpnt = fname + strlen(fname);
		for (i=0; i < suffix_len; i++)
			fpnt[i] = 'a';
		ofd = fileno(stdout);
	}

	if (fnum > 0) {
		fpnt[suffix_len-1]++;
		for (i = suffix_len - 1; i >= 0; i--)
			if (fpnt[i] > 'z') {
				if (i == 0) {
					fputs(MSGSTR(ABRTSPLT, 
					  "split: too many files.\n"), stderr);
					exit(1);
				} else {
					fpnt[i] = 'a';
					fpnt[i-1]++;
				}
			}
	}
	++fnum;
	if (!freopen(fname, "w", stdout)) {
		fprintf(stderr, MSGSTR(OUTCRTERR, 
				"split: unable to write to %s.\n"), fname);
		exit(1);
	}
}

/*
 * getbytes - parse byte value for -b
 */

unsigned long
getbytes(char *bytes)
{
	unsigned long 	number;
	char		*endptr;

	number = strtoul(bytes, &endptr, 10);
	if (number == 0) {
		fprintf(stderr, MSGSTR(BADBYTES, 
				"split: Bad number of bytes.\n"));
		usage();
	}

	switch (*endptr) {
		case 'k':
			number *= 1024;		/* 1 kilobyte */
			break;
		case 'm':
			number *= 1048576;	/* 1 meg */
			break;
		case '\0':
			break;
		default:
			usage();
	}

	return (number);
}

/*
 * Usage
 */
void
usage(void)
{
	fprintf(stderr,MSGSTR(USAGE, 
"Usage: split [[-b n[k|m] ] | [-l lines]] [-a suffix_length] [file [name]]\n"));
	exit(1);
}

/*
 * wrerror --
 *	write error
 */
void
wrerror(void)
{
	perror("split: write");
	exit(1);
}
