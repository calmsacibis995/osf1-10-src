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
static char rcsid[] = "@(#)$RCSfile: cat.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/10/11 20:07:13 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.1
 */
/*
 * COMPONENT_NAME: (CMDSCAN) commands that scan files
 *
 * FUNCTIONS:
 *
 *
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
 * cat.c  1.13  com/cmd/scan,3.1,9021 12/21/89 12:49:17
 */

#include	<stdio.h>
#include        <fcntl.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include 	<locale.h>
#include        <nl_types.h>
#include	<stdlib.h>
#include	<errno.h>
#include        "cat_msg.h"

#define BUFSIZE BUFSIZ	  /* BUFSIZ buffer for all others */
#define MSGSTR(num,str) catgets(catd,MS_CAT,num,str)  /*MSG*/
nl_catd catd;

/* Flags
 */
int	silent = 0;	/* don't warn about missing files */
int	bflag = 0;	/* don't count blank lines  */
int	eflag = 0;	/* put $ at the end of lines */
int	nflag = 0;	/* number lines */
int	rflag = 0;	/* squeeze blank lines to one */
int	tflag = 0;	/* display tabs as ^I */
int	vflag = 0;	/* display all control chars */

dev_t	dev;		/* details of output file */
ino_t	ino = -1;

static void	scat(char **), fcat(char **);

/*
 * There are just a few changes to make cat faster under certain
 * circumstances. The standard code (scat) is called, if the option
 * are used or input to cat is stdin. Switching between standard 
 * and the fast code is done by 'main()'!                           
 */

/*
 * NAME: cat
 *
 * FUNCTION:
 * 	Depending on the options passed, main() switches between the
 * 	standard cat (scat) and the faster version (fcat). 
 *
 * RETURN:
 *	Exits with 1 on command syntax errors,
 *	fcat/ scat exit with 0 success or, 2 failure.
 *                                                                    
 */
main(int argc, char **argv)
{
	register int	c;
	int		errflg = 0;
	struct stat	statb;
	extern int	optind;

	(void) setlocale (LC_ALL, "");
	catd = catopen(MF_CAT, NL_CAT_LOCALE);


#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("cat", &argv, 0);
#endif
	while ((c = getopt(argc, argv, "usrvbnte")) != EOF) {
		switch (c) {

		case 'b':	/* don't count blank lines  */
			bflag = nflag = 1;	/* -b implies -n */
			break;

		case 'e':	/* put $ at the end of lines */
			eflag = vflag = 1;	/* -e implies -v */
			break;

		case 'n':	/* number lines */
			nflag = 1;
			break;

		case 'r':	/* squeeze blank lines to one (was -s) */
			rflag = 1;
			break;

		case 's':	/* don't warn about missing files */
			silent = 1;
			break;

		case 't':	/* display tabs as ^I */
			tflag = vflag = 1;	/* -t implies -v */
			break;

		case 'u':	/* don't buffer output */
#ifndef STANDALONE
			setbuf(stdout, (char *) NULL);
#endif
			break;

		case 'v':	/* display all control chars */
			vflag = 1;
			break;

		case '?':
			errflg++;
			fprintf(stderr,
MSGSTR(EUSAGE, "usage: cat [-benrstuv] [-|file] ...\n"));
			exit(1);
		}
	}
	argv += optind;

	/* Preliminary sanity check.
	 */
	if (fstat((int)fileno(stdout), &statb) < 0) {
		fprintf(stderr,
				MSGSTR(ESTATOUT, "cat: Cannot stat stdout\n"));
		exit(2);
	}

	/* Save details of stdout away for later checking.
	 */
	statb.st_mode &= S_IFMT;
	if (statb.st_mode != S_IFCHR && statb.st_mode != S_IFBLK) {
		dev = statb.st_dev;
		ino = statb.st_ino;
	}

	/* If char processing is required call slow cat
	 * otherwise fast cat.
	 */
	if (nflag || rflag || vflag)	/* bflag, eflag, tflag are implicit */
		scat(argv);
	else
		fcat(argv);

	exit(-1);  /* should never get here */
}


/*
 * NAME: fcat
 *
 * FUNCTION:
 *	Fast cat (no char processing).
 *
 * RETURN VALUE DESCRIPTION: 
 *	Exits with status 0 if successful, 2 if failure.
 */
static void	
fcat(char **argv)
{
	register int	fd, cnt;
	char		buffer[BUFSIZE];
	char		*filename;
	struct stat	statb;
	int		status = 0;

	do {
		/* Extract input name.
		 */
		if (*argv) {
			if (!strcmp(*argv, "-"))
				fd = fileno(stdin);
			else if ((fd = open(*argv, O_RDONLY, 0)) < 0) {
				if (!silent)
					fprintf(stderr,
MSGSTR(EOPEN, "cat: cannot open %s\n"), *argv);
				status = 2;
				++argv;
				continue;
			}
			filename = *argv++;
		} else { /* no args - use stdin as input (implicit "-") */
			fd = fileno(stdin);
			filename = "-";
		}

		/* Open and check input.
		 */
		if (fstat(fd, &statb) < 0) {
			fprintf(stderr,
				MSGSTR(ESTAT, "cat: cannot stat %s\n"),
				filename);
			if (fd != fileno(stdin) && close(fd) != 0)
				fprintf(stderr,
					MSGSTR(ECLOSE, "cat: close error\n"));
			status = 2;
			continue;
		}
		if (statb.st_dev == dev && statb.st_ino == ino) {
			fprintf(stderr,
				MSGSTR(EINISOUT, "cat: input %s is output\n"),
				filename);
			if (fd != fileno(stdin) && close(fd) != 0)
				fprintf(stderr,
					MSGSTR(ECLOSE, "cat: close error\n"));
			status = 2;
			continue;
		}

		/* Transfer input to stdout (no buffering).
		 */
		while ((cnt = read(fd, buffer, BUFSIZE)) > 0) {
			if (cnt != write(fileno(stdout), buffer, cnt)) {
				perror("");
				fprintf(stderr,
					MSGSTR(EOUTPUT, "cat: output error\n"));
				status = 2;
				break;
			}
		}

		/* Check for partial reads.
		 */
		if (cnt < 0) {
		        fprintf(stderr,MSGSTR(EREAD, "cat: %s : %s\n"),
                                filename, strerror(errno));
			status = 2;
		}

		/* Close down input.
		 */
		if (fd != fileno(stdin) && close(fd) != 0) {
			fprintf(stderr, MSGSTR(ECLOSE, "cat: close error\n"));
			status = 2;
		}
	} while (*argv);

	exit(status);
}


/*
 * NAME: scat
 *
 * FUNCTION:
 *
 *	Slow cat. Handles all char processing options.
 *	Read file character by character and copy it to standard output.
 *	Translate special characters and number lines as specified by
 *	flags.
 *
 * RETURN VALUE DESCRIPTION: 
 *	Exits with status 0 if successful, 2 if failure.
 */
static void	
scat(char **argv)
{
	register FILE	*fp;
	wchar_t 	c;
	register int	i, j, k, cvt_cnt;
	register int	cnt;			/* counter for buffer */
	register int	pending;		/* pending char in buffer */
	char		buff[10];		/* buff for bytes read */
	char		tmp[10];
	int		mb_max=MB_CUR_MAX;	/* max mb size */
	int		spaced = 0;
	int		nline = 0;
	int		lno = 1;
	char		*filename;
	struct stat	statb;
	int		status = 0;

	do {
		/* Extract input name.
		 */
		if (*argv) {
			if (!strcmp(*argv, "-"))
				fp = stdin;
			else if ((fp = fopen(*argv, "r")) == NULL) {
				if (!silent)
					fprintf(stderr,
MSGSTR(EOPEN, "cat: cannot open %s\n"), *argv);
				status = 2;
				++argv;
				continue;
			}
			filename = *argv++;
		} else { /* no args - use stdin as input (implicit "-") */
			fp = stdin;
			filename = "-";
		}

		/* Open and check input.
		 */
		if (fstat((int)fileno(fp), &statb) < 0) {
			fprintf(stderr,
				MSGSTR(ESTAT, "cat: cannot stat %s\n"),
				filename);
			if (fp != stdin && fclose(fp) != 0)
				fprintf(stderr,
					MSGSTR(ECLOSE, "cat: close error\n"));
			status = 2;
			continue;
		}
		if (statb.st_dev == dev && statb.st_ino == ino) {
			fprintf(stderr,
				MSGSTR(EINISOUT, "cat: input %s is output\n"),
				filename);
			if (fp != stdin && fclose(fp) != 0)
				fprintf(stderr,
					MSGSTR(ECLOSE, "cat: close error\n"));
			status = 2;
			continue;
		}

		/* Transfer input to stdout processing char by char.
		 */
		pending=0;
		cnt=0;
		while (((k = getc(fp)) != EOF) || cnt) {
			if (k!=EOF) {
				buff[cnt++]=k&0xff;
				buff[cnt]='\0';
			}

			/* try convert to valid wc */
			if (((cvt_cnt=mbtowc(&c, buff, cnt))==-1) && 
			    (cnt<mb_max) && (k!=EOF)) continue;

			if ((cvt_cnt==-1) ||  	/* cant cvt, use 1st byte */
			    (!cvt_cnt)) {	/* get \0 in file */
				c=(wchar_t)buff[0]&0xff;
				bcopy(buff+1,tmp,cnt);
				bcopy(tmp,buff,cnt--);
			}
			else {
				cnt-=cvt_cnt;	/* cvt ok, reset buff */
				if (cnt) {	/* mv remaining to front */
					bcopy(buff+cvt_cnt,tmp,cnt+1);
					bcopy(tmp,buff,cnt+1);
				}
			}

			if (c == L'\n') {
				if (nline == 0) {
					if (rflag && spaced)
						continue;
					spaced = 1;
				}
				if (nflag && bflag == 0 && nline == 0)
					printf("%6d\t", lno++);
				if (eflag)
					putchar('$');
				putchar('\n');
				nline = 0;
				continue;
			}
			if (nflag && nline == 0)
				printf("%6d\t", lno++);
			nline = 1;
			if (vflag) {
				if (tflag == 0 && c == L'\t')
					putwchar(c);
				else if (isascii(c) && iswcntrl(c))
					printf("^%c", c == L'\177'
				  	? '?' : c | 0100);
				else if (iswprint(c))
					putwchar(c);
				else if (c>0xff) {	/* cvt'ed wc only */
					register int size;

					putchar('[');
					size=wctomb(tmp,c);
					for(j=0;j<size;j++)
						printf("%x",tmp[j]&0xff);
					putchar(']');
				}
				else if (iswcntrl(c=toascii(c))) /*not for wc*/
					printf("M-^%c", c == L'\177'
				           ? '?' : c | 0100);
				else
					printf("M-%c", c);
			} else
				putwchar(c);
			spaced = 0;

		}

		/* Check for partial reads.
		 */
		if (ferror(fp)) {
		        fprintf(stderr,MSGSTR(EREAD, "cat: %s : %s\n"),
                                filename, strerror(errno));
			status = 2;
		}

		/* Close down input.
		 */
		if (fp != stdin) {
			fflush(stdout);
			if (fclose(fp) != 0) {
				fprintf(stderr,
					MSGSTR(ECLOSE, "cat: close error\n"));
				status = 2;
			}
		}
	} while (*argv);

	fflush(stdout);
	if (ferror(stdout)) {
		fprintf(stderr, MSGSTR(EOUTPUT, "cat: output error\n"));
		status = 2;
	}

	exit(status);
}

