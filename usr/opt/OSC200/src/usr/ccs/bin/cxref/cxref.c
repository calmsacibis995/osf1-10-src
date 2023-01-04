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
static char	*sccsid = "@(#)$RCSfile: cxref.c,v $ $Revision: 4.2.6.4 $ (DEC) $Date: 1993/10/07 19:22:15 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * COMPONENT_NAME: (CMDPROG) Programming Utilites
 *
 * FUNCTIONS: main, callsys, dexit, doxasref, doxref, getnumb, m4file, mktmpfl,
	      prntflnm, prntfnc, prntlino, prntwrd, prtsort, quitmsg,
	      quitperror, scanline, setsig, sigout, sortfile, tmpscan, tunlink
 *
 * ORIGINS: 00 03 10 27 32
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * cxref.c	1.10  com/cmd/prog/cxref,3.1,9013 3/1/90 12:19:32";
 */

#include <stdio.h>
#include <locale.h>
#include <ctype.h>
#include <signal.h>
#include <varargs.h>
#include <nl_types.h>
#include "cxref_msg.h"
#define MSGSTR(Num, Str) catgets(catd, MS_CXREF, Num, Str)
nl_catd catd;

#define EXITFATAL 0100	/* Fatal error occurred */

#define MAXWORD (BUFSIZ / 2)
#define MAXRFW MAXWORD
#define MAXRFL MAXWORD

#define STDPREDEFS	(sizeof(stdpredefs) / sizeof(char *))
#define MAXDEFS 50	/* maximum number of preprocessor arguments */
#define MAXARGS (MAXDEFS + 10)
#define BASIZ	50	/* funny */
#define LPROUT	70	/* output */
#define TTYOUT	30	/* numbers */
#define YES	1
#define NO	0
#define LETTER	'a'
#define DIGIT	'0'

#include <sys/param.h>

extern char
	*calloc(),
	*strcat(),
	*strcpy(),
	*mktemp();

char	*cfile,	/* current file name */
	*tmp1,	/* contains preprocessor result */
	*tmp2,	/* contains cpp and xpass cross-reference */
	*tmp3,	/* contains transformed tmp2 */
	*tmp4;	/* contains sorted output */

char	*clfunc = "   --   ";	/* clears function name for ext. variables */

extern void exit();

char	xflnm[MAXWORD],	/* saves current filename */
	funcname[MAXWORD],	/* saves current function name */
	sv1[MAXRFL],	/* save areas used for output printing */
	sv2[MAXRFL],
	sv3[MAXRFL],
	sv4[MAXRFL],
	buf[BUFSIZ];

/* ----------- these paths are to be checked for accuracy---------- */
/* ----------- and some final path for these utilities should be */
/* ------------ decided and if neccessary changed to reflect the */
/* ------------ correct path before installing cxref --------------*/

static char	xcpp[] = "/lib/cpp",
	*xref = "/usr/ccs/lib/xpass",
	sort[] = "/bin/sort";



char	*arv[MAXARGS], *predefs[MAXDEFS];
char	*stdpredefs[] = {
  "-DLANGUAGE_C",
  "-Dunix",
  "-D__LANGUAGE_C__", 
  "-D__unix__", 
  "-D__osf__", 
  "-D__alpha", 
  "-D_LONGLONG",
  "-D_CFE",
  "-DSYSTYPE_BSD",
  "-D_SYSTYPE_BSD"
  };

int	cflg = NO,	/* prints all given files as one cxref listing */
	silent = NO,	/* print filename before cxref? */
	tmpmade = NO,	/* indicates if temp file has been made */
	inafunc = NO,	/* in a function? */
	fprnt = YES,	/* first printed line? */
	addlino = NO,	/* add line number to line?? */
	ddbgflg = NO,	/* debugging? */
	idbgflg = NO,
	bdbgflg = NO,
	tdbgflg = NO,
	edbgflg = NO,
	xdbgflg = NO,
	Nflag = NO,
	LSIZE = LPROUT,	/* default size */
	lsize = 0,	/* scanline: keeps track of line length */
	sblk = 0,	/* saves first block number of each new function */
	fsp = 0,	/* format determined by prntflnm */
	defs = 0,	/* number of -I, -D and -U flags */
	defslen = 0;	/* length of -I, -D and -U flags */
char   *Nstr;

int	nword,	/* prntlino: new word? */
	nflnm,	/* new filename? */
	nfnc,	/* new function name? */
	nlino;	/* new line number? */

main(argc,argv)
	int argc;
	char *argv[];
{
	int     c, i;

	extern int      optind, opterr;
	extern char     *optarg;

	setlocale(LC_ALL, "");
	catd = catopen(MF_CXREF, NL_CAT_LOCALE);

	if(argc == 1){
		fprintf(stderr, MSGSTR(M_MSG_13,
		"Usage: cxref [-cst] [-o File] [-w[Number]]\n\
                [[-DName[=Definition]] [-UName] [-IDirectory]] ...\n\
                [-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"));
		exit(1);
	}
  	AddStdPreDefs();	/* Add standard preprocessor defs */
	mktmpfl();		/* make temp files */
	for(i=1; i < argc; i++)
		if(!strcmp(argv[i], "-w"))
			argv[i] = "-w80";
	opterr = 0;		/* surpress error from getopt() */
	while ((c = getopt(argc, argv, "cdibTex:D:U:I:tw:so:N:")) != -1) {
		sblk = 0;
		switch (c) {
		/* prints cross reference for all files combined */
			case 'c':
				cflg = YES;
				continue;
			case 'd':
				ddbgflg = YES;
				continue;
			case 'i':
				idbgflg = YES;
				continue;
			case 'b':
				bdbgflg = YES;
				continue;
			case 'T':
				tdbgflg = YES;
				continue;
			case 'e':
				edbgflg = YES;
				continue;
			case 'x':
				if (strncmp(argv[optind], "xrfp=", 5) == 0) {
				  xref=(argv[optind]) + 5;
				} else
				  xdbgflg = YES;
				continue;
			case 'D': case 'U': case 'I':

				if (defs >= MAXDEFS-1)
					quitmsg(MSGSTR(M_MSG_3,
			      "Too many preprocessor parameters\n" ));
				defslen += strlen(predefs[defs++] =
							argv[optind-1]);
/* 001 - REMOVED		 /* space between flag and arg */               /* 001 */
/* 001 - REMOVED		if (strlen(*argv) == 2)                         /* 001 */
/* 001 - REMOVED		{                                               /* 001 */
/* 001 - REMOVED		   defslen += strlen(predefs[defs++] = *++argv);/* 001 */
/* 001 - REMOVED		   --argc;                                      /* 001 */
/* 001 - REMOVED		}                                               /* 001 */
				continue;
			/* length option when printing on terminal */
			case 't':
			case 'w':
				LSIZE = getnumb(&argv[optind-1][2]) - BASIZ;
				if (LSIZE <= 0)
				LSIZE = TTYOUT;
				continue;
			case 's':
				silent = YES;
				continue;
			/* output file */
			case 'o':
				if (freopen(argv[optind-1], "w",
					stdout) == NULL)
					quitperror(argv[optind]);
				continue;
			case 'N':
				if (Nflag)
				{
				    /* Allocate enough space for
				     * original string, plus new
				     * string, and Null the new
				     * string won't include the N */
				    Nstr = (char *) realloc(Nstr,
					(strlen(Nstr) + strlen(optarg))
						* sizeof(char));
				    strcat(Nstr, optarg);
				}
				else
				{
				    Nflag = YES;
				    /* Allocate enough space for Null
				     * and -N */
				    Nstr = (char *) malloc( (3 +
						strlen(optarg)) *
						sizeof(char));
				    strcpy(Nstr, "-N");
				    strcat(Nstr, optarg);
				}
				continue;
			default:
				fprintf(stderr, MSGSTR(M_MSG_14,
				  "Invalid option '%c' - ignored\n"),
				  argv[optind-1][1]);
		} /* end switch() */
	} /* end while() */

	/* Check there is at least one file argument left */
	if (!*(argv += optind)) {
		fprintf(stderr, MSGSTR(M_MSG_13,
		"Usage: cxref [-cst] [-o File] [-w[Number]]\n\
                [[-DName[=Definition]] [-UName] [-IDirectory]] ...\n\
                [-NdNumber] [-NlNumber] [-NnNumber] [-NtNumber] File ...\n"));
		exit(1);
	}

	/* Check to make sure they are all C files */
	for (c = 0; c < (argc - optind); c++)
		if (strcmp(argv[c] + strlen(argv[c]) - 2, ".c") != 0){
			fprintf(stderr, MSGSTR(M_MSG_13,	/* @@ NEED MODIFIED @@ */
			"cxref: file with unknown suffix: %s\n"), argv[c]);
		exit(1);
		}

	/* Start creating a xref */
        for (c = 0; c < (argc - optind); c++){
		cfile = argv[c];

		if (silent == NO)
			printf("%s:\n\n", cfile);

		doxref();		/* find variables in the input files */

		if (cflg == NO) {
			sortfile();	/* sorts temp file */
			prtsort();	/* print sorted temp file when -c
					 * option is not used */
			tunlink();	/* forget everything */
			mktmpfl();	/* and start over */
		}
	}
	if (cflg == YES) {
		sortfile();	/* sorts temp file */
		prtsort();	/* print sorted temp file when -c
				 * option is used */
	}
	tunlink();	 /* unlink temp files */
	exit(0);
	/*NOTREACHED*/
}

	/*
	 * This routine calls the program "xpass" which parses
	 * the input files, breaking the file down into
	 * a list with the variables, their definitions and references,
	 * the beginning and ending function block numbers,
	 * and the names of the functions---The output is stored on a
	 * temporary file and then read.  The file is then scanned
	 * by tmpscan(), which handles the processing of the variables
	 */
doxref()
{

	register int i,n;
	FILE *fp, *tfp;
	char cccmd[MAXPATHLEN];
	char line[BUFSIZ], s1[MAXRFW], s2[MAXRFW];

	/* set up cc command line to do preprocessing */
	n = 0;
	arv[n++] = "/lib/cpp";
	arv[n++] = "-E";

	for (i = 0; i < defs; i++)
		arv[n++] = predefs[i];
	arv[n++] = cfile;			/* infile */
	arv[n++] = tmp1;			/* outfile */
	arv[n] = 0;
	if ( callsys(xcpp, arv, NO) > 0) quitmsg(MSGSTR(M_MSG_6,
		"/lib/cpp failed on %s!\n"), cfile);

	i = 0;
	arv[i++] = "xpass";
	if (ddbgflg)
		arv[i++] = "-d";
	if (idbgflg)
		arv[i++] = "-I";
	if (bdbgflg)
		arv[i++] = "-b";
	if (tdbgflg)
		arv[i++] = "-t";
	if (edbgflg)
		arv[i++] = "-e";
	if (xdbgflg)
		arv[i++] = "-x";
	if (Nflag)
		arv[i++] = Nstr;
	arv[i++] = tmp1;
	arv[i++] = tmp2;
	arv[i] = 0;
	if ( callsys(xref, arv, NO) > 0) quitmsg(MSGSTR(M_MSG_7,
		"xpass failed on %s!\n"), cfile);

	/* open temp file produced by "xpass" for reading */
	if ((fp = fopen(tmp2, "r")) == NULL) quitperror(tmp2);
	else {
		setbuf(fp, buf);
		/*
		 * open a new temp file for writing
		 * the output from tmpscan()
		 */
		if ((tfp = fopen(tmp3, "a")) == NULL) quitperror(tmp3);
		else {

			/*
			 * read a line from tempfile 2,
			 * break it down and scan the
			 * line in tmpscan()
			 */

			while (fgets(line, BUFSIZ, fp) != NULL) {
				if (line[0] == '"') {
					/* removes quotes */
					for (i=0; line[i + 1] != '"'; i++) {
						xflnm[i] = line[i + 1];
					};
					xflnm[i] = '\0';
					continue;
				};
				sscanf(line, "%s%s", s1, s2);
				tmpscan(s1, s2, tfp);
			};
			fclose(tfp);
		};
		fclose(fp);
	};
}

	/*
	 * general purpose routine which does a fork
	 * and executes what was passed to it--
	 */

callsys(f, v, flag)
	char f[], *v[];
	int flag;	/* flag to redirect stdout to tmp1 */
{
	register int pid, w;
	int status;

	if (flag)
		fflush(stdout);

	if ((pid = fork()) == 0) {
		/* only the child gets here */
		if(flag) {
			if (freopen(tmp1, "w", stdout) == NULL)
				quitperror(tmp1);
		}
		execvp(f, v);
		quitperror(f);
	}
	else
		if (pid == -1)
			/* fork failed - tell user */
			quitperror(MSGSTR(M_MSG_8, "cxref"));
	/*
	 * loop while calling "wait" to wait for the child.
	 * "wait" returns the pid of the child when it returns or
	 * -1 if the child can not be found.
	 */
	while (((w = wait(&status)) != pid) && (w != -1));
	if (w == -1) 
		/* child was lost - inform user */
		quitperror(f);
	else {
		/* check system return status */
		if (((w = status & 0x7f) != 0) && (w != SIGALRM)) {
			/* don't complain if the user interrupted us */
			if (w != SIGINT) {
				fprintf(stderr, MSGSTR(M_MSG_15,
					"Fatal error in %s"), f);
				perror(" ");
			};
			/* remove temporary files */
			dexit();
		};
	};
	/* return child status only */
	return((status >> 8) & 0xff);
}

	/*
	 * general purpose routine which returns
	 * the numerical value found in a string
	 */

getnumb(ap)
	register char *ap;
{

	register int n, c;

	n = 0;
	while ((c = *ap++) >= '0' && c <= '9')
		n = n * 10 + c - '0';
	return(n);
}

tmpscan(s, ns, tfp)
	register char s[];
	char ns[];
	FILE *tfp;
{

	/* this routine parses the output of "xpass"*/

	/*
	 * D--variable defined; R--variable referenced;
	 * F--function name; B--block(function ) begins;
	 * E--block(function) ends
	 */

	register int lino = 0;
	char star;

	/*
	 * look for definitions and references of external variables;
	 * look for function names and beginning block numbers
	 */

	if (inafunc == NO) {
		switch (*s++) {
			case 'D':
				star = '*';
				goto ahead1;
			case 'R':
				star = ' ';
ahead1:				lino = getnumb(ns);
				fprintf(tfp, "%s\t%s\t%s\t%5d\t%c\n", s, xflnm,
					clfunc, lino, star);
				break;
			case 'F':
				strcpy(funcname, s);
				star = '$';
				fprintf(tfp, "%s\t%c\n", s, star);
				break;
			case 'B':
				inafunc = YES;
				sblk = getnumb(s);
				break;
			default:
				quitmsg(MSGSTR(M_MSG_9,
				  "SWITCH ERROR IN TMPSCAN: inafunc = no\n" ));
		};
	}
	else {
		/*
		 * in a function:  handles local variables
		 * and looks for the end of the function
		 */

		switch (*s++) {
			case 'R':
				star = ' ';
				goto ahead2;
				/* No Break Needed */
			case 'D':
				star = '*';
ahead2:				lino = getnumb(ns);
				fprintf(tfp, "%s\t%s\t%s\t%5d\t%c\n", s, xflnm,
					funcname, lino, star);
				break;
			case 'B':
				break;
			case 'E':
				lino = getnumb(s);
				/*
				 * lino is used to hold the ending block
				 * number at this point
				 *
				 * if the ending block number is the
				 * same as the beginning block number
				 * of the function, indicate that the
				 * next variable found will be external.
				 */

				if (sblk == lino) {
					inafunc = NO;
				}
				break;
			case 'F':
				star = '$';
				fprintf(tfp, "%s\t%c\n", s, star);
				break;
			default:
				quitmsg(MSGSTR(M_MSG_10,
				  "SWITCH ERROR IN TMPSCAN: inafunc = yes\n"));
		};
	};
}

AddStdPreDefs()
{
  register int sdefs;
  for(sdefs = 0; sdefs < STDPREDEFS; sdefs++) {
    defslen += strlen(predefs[defs++] = stdpredefs[sdefs]);
  }
}
    
mktmpfl()
{
  static char str1[] = "/usr/tmp/xr1XXXXXX";
  static char str2[] = "/usr/tmp/xr2XXXXXX";
  static char str3[] = "/usr/tmp/xr3XXXXXX";
  static char str4[] = "/usr/tmp/xr4XXXXXX";

	/* make temporary files */

	tmp1 = mktemp(str1);
	tmp2 = mktemp(str2);		/* output of "xpass" */
	tmp3 = mktemp(str3);		/* holds output of tmpscan()
					 * routine */
	tmp4 = mktemp(str4);		/* holds output of tempfile 3*/
	tmpmade = YES;	/* indicates temporary files have been made */
	setsig();
}

tunlink()
{

	/* unlink temporary files */

	if (tmpmade == YES) {	/* if tempfiles exist */
		unlink(tmp1);
		unlink(tmp2);
		unlink(tmp3);
		unlink(tmp4);
	};
}

dexit()
{
	/* remove temporary files and exit with error status */

	tunlink();
	exit(EXITFATAL);
}

quitperror(str) char *str; {perror(str); dexit();}
/* VARARGS */
quitmsg(va_alist) va_dcl {
    char *fmt,buf[200];
    va_list v; 
    va_start(v); fmt=va_arg(v,char *); vsprintf(buf,fmt,v); va_end(v);
    fprintf(stderr,buf);
    dexit();}

setsig()
{

	/* set up check on signals */

	void sigout(void);

	if (isatty(1)) {
		if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
			signal(SIGHUP, (void (*)(int))sigout);
		if (signal(SIGINT, SIG_IGN) == SIG_DFL)
			signal(SIGINT, (void (*)(int))sigout);
	}
	else {
		signal(SIGHUP, SIG_IGN);
		signal(SIGINT, SIG_IGN);
	};

	signal(SIGQUIT, (void (*)(int))sigout);
	signal(SIGTERM, (void (*)(int))sigout);
}

 void sigout(void)
{

	/* signal caught; unlink tmp files */

	tunlink();
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	dexit();
}

sortfile()
{
	/* sorts temp file 3 --- stores on 4 */

	register int status;

	arv[0] = "sort";
	arv[1] = "-o";
	arv[2] = tmp4;
	arv[3] = tmp3;
	arv[4] = 0;
	/* execute sort */
	if ((status = callsys(sort, arv, NO)) > 0)
		quitmsg(MSGSTR(M_MSG_11, "Sort failed with status %d\n"),
			status);
}

prtsort()
{

	/* prints sorted files and formats output for cxref listing */

	FILE *fp;
	char line[BUFSIZ];

	/* open tempfile of sorted data */
	if ((fp = fopen(tmp4, "r")) == NULL)
		quitmsg(MSGSTR(M_MSG_12, "CAN'T OPEN %s\n"), tmp4);
	else {
		if (silent == YES) {	/* assume called by "oprl" */
			fprintf(stdout, MSGSTR(M_MSG_16,
			  "SYMBOL\t\tFILE\t\t\tFUNCTION   LINE\n"));
		};
		while (fgets(line, BUFSIZ, fp) != NULL) {
			scanline(line);	/* routine to format output */
		};
		fprnt = YES;	/* reinitialize for next file */
		fclose(fp);
		putc('\n', stdout);
	};
}

scanline(line)
	char *line;
{

	/* formats what is to be printed on the output */

	register char *sptr1;
	register int och, nch;
	char s1[MAXRFL], s2[MAXRFL], s3[MAXRFL], s4[MAXRFL], s5[MAXRFL];
	
	/*
	 * break down line into variable name, filename,
	 * function name, and line number
	 */
	sscanf(line, "%s%s%s%s%s", s1, s2, s3, s4, s5);
	if (strcmp(s2, "$") == 0) {
		/* function name */
		if (strcmp(sv1, s1) != 0) {
			strcpy(sv1, s1);
			printf("\n%s()", s1);	/* append '()' to name */
			*sv2 = *sv3 = *sv4 = '\0';
			fprnt = NO;
		};
		return;
	};
	if (strcmp(s5, "*") == 0) {
		/* variable defined at this line number */
		*s5 = '\0';
		sptr1 = s4;
		och = '*';
		/* prepend a star '*' */
		for ( nch = *sptr1; *sptr1 = och; nch = *++sptr1)
			och = nch;
	}
	if (fprnt == YES) {
		/* if first line--copy the line to a save area */
		prntwrd( strcpy(sv1, s1) );
		prntflnm( strcpy(sv2, s2) );
		prntfnc( strcpy(sv3, s3) );
		prntlino( strcpy(sv4, s4) );
		fprnt = NO;
		return;
	}
	else {
		/*
		 * this part checks to see what variables have changed
		 */
		if (strcmp(sv1, s1) != 0) {
			nword = nflnm = nfnc = nlino = YES;
		}
		else {
			nword = NO;
			if (strcmp(sv2, s2) != 0) {
				nflnm = nfnc = nlino = YES;
			}
			else {
				nflnm = NO;
				if (strcmp(sv3, s3) != 0) {
					nfnc = nlino = YES;
				}
				else {
					nfnc = NO;
					nlino = (strcmp(sv4, s4) != 0) ? YES :
							NO;
					if (nlino == YES) {
						/*
						 * everything is the same
						 * except line number
						 * add new line number
						 */
						addlino = YES;
						prntlino( strcpy(sv4, s4) );
					};
					/*
					 * Want to return if we get to
					 * this point. Case 1: nlino
					 * is NO, then entire line is
					 * same as previous one.
					 * Case 2: only line number is
					 * different, add new line number
					 */
					return;
				};
			};
		};
	};

	/*
	 * either the word, filename or function name
	 * are different; this part of the routine handles  
	 * what has changed...
	 */

	addlino = NO;
	lsize = 0;
	if (nword == YES) {
		/* word different--print line */
		prntwrd( strcpy(sv1, s1) );
		prntflnm( strcpy(sv2, s2) );
		prntfnc( strcpy(sv3, s3) );
		prntlino( strcpy(sv4, s4) );
		return;
	}
	else {
		fputs("\n\t\t", stdout);
		if (nflnm == YES) {
			/*
			 * different filename---new name,
			 * function name and line number are
			 * printed and saved
			 */
			prntflnm( strcpy(sv2, s2) );
			prntfnc( strcpy(sv3, s3) );
			prntlino( strcpy(sv4, s4) );
			return;
		}
		else {
			/* prints filename as formatted by prntflnm()*/
			switch (fsp) {
			case 1:
				printf("%s\t\t\t", s2);
				break;
			case 2:
				printf("%s\t\t", s2);
				break;
			case 3:
				printf("%s\t", s2);
				break;
			case 4:
				printf("%s  ", s2);
			};
			if (nfnc == YES) {
				/*
				 * different function name---new name
				 * is printed with line number;
				 * name and line are saved
				 */
				prntfnc( strcpy(sv3, s3) );
				prntlino( strcpy(sv4, s4) );
			};
		};
	};
}

prntwrd(w)
	char *w;
{

	/* formats word(variable)*/
	register int wsize;	/*16 char max. word length */

	if ((wsize = strlen(w)) < 8) {
		printf("\n%s\t\t", w);
	}
	else
		if ((wsize >= 8) && (wsize < 16)) {
			printf("\n%s\t", w);
		}
		else {
			printf("\n%s  ", w);
		};
}

prntflnm(fn)
	char *fn;
{

	/* formats filename */
	register int fsize;	/*24 char max. fliename length */

	if ((fsize = strlen(fn)) < 8) {
		printf("%s\t\t\t", fn);
		fsp = 1;
	}
	else {
		if ((fsize >= 8) && (fsize < 16)) {
			printf("%s\t\t", fn);
			fsp = 2;
		}
		else
			if ((fsize >= 16) && (fsize < 24)) {
				printf("%s\t", fn);
				fsp = 3;
			}
			else {
				printf("%s  ", fn);
				fsp = 4;
			};
	};
}

prntfnc(fnc)
	char *fnc;
{

	/* formats function name */
	if (strlen(fnc) < 8) {
		printf("%s\t  ", fnc);
	}
	else {
		printf("%s  ", fnc);
	};
}


prntlino(ns)
	register char *ns;
{

	/* formats line numbers */

	register int lino, i;
	char star;

	i = lino = 0;

	if (*ns == '*') {
		star = '*';
		ns++;	/* get past star */
	}
	else {
		star = ' ';
	};
	lino = getnumb(ns);
	if (lino < 10)	/* keeps track of line width */
		lsize += (i = 3);
	else if ((lino >=10) && (lino < 100))
			lsize += (i = 4);
	else if ((lino >= 100) && (lino < 1000))
				lsize += (i = 5);
	else if ((lino >= 1000) && (lino < 10000))
					lsize += (i = 6);
	else /* lino > 10000 */
						lsize += (i = 7);
	if (addlino == YES) {
		if (lsize <= LSIZE) {
			/* line length not exceeded--print line number */
			fprintf(stdout, " %c%d", star, lino);
		}
		else {
			/* line to long---format lines overflow */
			fprintf(stdout, "\n\t\t\t\t\t\t   %c%d", star, lino);
			lsize = i;
		};
		addlino = NO;
	}
	else {
		fprintf(stdout, " %c%d", star, lino);
	};
}

