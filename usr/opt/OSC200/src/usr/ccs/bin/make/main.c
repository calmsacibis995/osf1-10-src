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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/10/07 19:16:09 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 * main.c	4.10 (Berkeley) 87/11/15";
 */

/*
 * command make to update programs.
 *
 * Flags:
 *	'd'  print out debugging comments
 *	'p'  print out a version of the input graph
 *	's'  silent mode--don't print out commands
 *	'f'  the next argument is the name of the description file;
 *	     "makefile" is the default
 *	'F'  a description file is compulsory
 *	'i'  ignore error codes from the shell
 *	'S'  stop after any command fails (normally do parallel work)
 *	'n'  don't issue, just print, commands
 *	't'  touch (update time of) files but don't issue command
 *	'q'  don't do anything, but check if object is up to date;
 *	     returns exit code 0 if up to date, -1 if not
 *	'c'  don't check out RCS files
 *	'C'  check out RCS files (default)
 *	'u'  don't unlink RCS working files
 *	'U'  unlink RCS working files automatically checked out (default)
 *	'm'  look in machine specific subdirectory first
 */

#include "defs.h"
#include <signal.h>
#ifdef lint
#undef SIG_IGN
SIG_IGN(){}
#endif

#ifndef _BLD
#include <locale.h>
#include <NLctype.h>
#include <nl_types.h>
nl_catd catd;
#include "make_msg.h"
#define MSGSTR(Num,Str)   catgets(catd,MS_MAKE,Num,Str)
#else
#include <ctype.h>
#define MSGSTR(Num,Str)    Str
#endif

FSTATIC void intrupt();
FSTATIC int rddescf();
FSTATIC setcocmd();
FSTATIC printdesc();
FSTATIC printmac();
FSTATIC setmflgs();
FSTATIC setflags();
FSTATIC optswitch();
FSTATIC getmflgs();
FSTATIC getmdefs();
FSTATIC setmdefs();

struct chain *rmchain		= 0;
struct chain *deschain		= 0;
struct nameblock *mainname	= 0;
struct nameblock *firstname	= 0;
struct nameblock *inobjdir	= 0;
struct lineblock *co_cmd	= 0;
struct lineblock *rcstime_cmd	= 0;
struct lineblock *sufflist	= 0;
struct varblock *firstvar	= 0;
struct dirhdr *firstod		= 0;

int rmflag	= YES;
int coflag	= YES;
int nocmds	= NO;
int rcsdep	= NO;
char *RCSdir	= RCS;
char *RCSsuf	= RCS_SUF;

int sighvalue	= 0;
int sigivalue	= 0;
int sigqvalue	= 0;
int sigtvalue	= 0;

int argsinenv	= YES;
int envover	= NO;
int dbgflag	= NO;
int prtrflag	= NO;
int silflag	= NO;
int noexflag	= NO;
int keepgoing	= NO;
int descflag	= NO;
int noruleflag	= NO;
int noconfig	= NO;
int touchflag	= NO;
int questflag	= NO;
int machdep	= NO;
int ignerr	= NO;	/* default is to stop on error */
int botchflag	= YES;	/* we come pre-botched */
int okdel	= YES;
int ndocoms	= 0;
int inarglist	= 0;
int ininternal	= 0;
char *prompt	= "";	/* other systems -- pick what you want */
char *curfname	= "no description file";
char funny[128];
char options[26 + 1] = { '-' };
char Makeflags[] = "MAKEFLAGS";
char Makedefs[] = "MAKEDEFS";


int
main(argc, argv)
	int argc;
	char **argv;
{
	register struct nameblock *p;
	int i, descset, nfargs;
	time_t td;
	struct chain *ch;
	char *s;
	extern char *ctime();

#ifndef _BLD
	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_MAKE,NL_CAT_LOCALE);
#endif

	i = getdtablesize();
	while (i > 3)
		(void) close(--i);
#ifdef METERFILE
	meter(METERFILE);
#endif

	descset = 0;

	funny[0] = (META | TERMINAL);
	for (s = "=|^();&<>*?[]:$`'\"\\\n"; *s; ++s)
		funny[*s] |= META;
	for (s = "\n\t :;&>|()"; *s; ++s)
		funny[*s] |= TERMINAL;

	getmflgs();		/* Init $(MAKEFLAGS) variable */
	for (i = 1; i < argc; ++i)
		if (argv[i][0] == '-' && index(argv[i], 'd'))
			break;
	if (dbgflag || i < argc)
		entrybanner(argc, argv);
	setflags(argc, argv);

	getmdefs();
	inarglist = 1;
	for (i = 1; i < argc; ++i)
		if (argv[i] && argv[i][0] != '-'
		&& eqsign(argv[i], (char *) 0, 0)) {
			setmdefs(argv[i]);
			argv[i] = 0;
		}
	ininternal = 1;
	setvar("$", "$", 0);
/* 001 - Moved definition to rules.c */
/*	setvar("MACHINE", MACHINE, 0);*/
	ininternal = 0;
	inarglist = 0;

	/*
	 * Summary of Assigning Macros and Variables
	 * from lower to higher priority.
	 *
	 *	-e not specified	-e specified
	 *	--------------------------------------------
	 *	internal definitions	internal definintions
	 *	environment		makefile(s)
	 *	makefile(s)		environment
	 *	command line		command line
	 */

	/*
	 * Read internal definitions and rules
	 */
	if (!noruleflag) {
		if (dbgflag)
			printf(MSGSTR(RDINT,"Reading internal rules.\n"));
		ininternal = 1;
		rdd1((FILE *) 0, "internal rules");
		ininternal = 0;
	}

	/*
	 * Read environment args.  Let file args which follow override
	 * unless "envover" variable is set.
	 */
	inarglist = envover;
	readenv();
	inarglist = 0;

	/*
	 * export MAKEFLAGS
	 */
	setenviron(Makeflags, varptr(Makeflags)->varval, 1);

	/*
	 * MFLAGS=options to make
	 */
	if (strcmp(options, "-") == 0)
		*options = 0;
	setvar("MFLAGS", options, 0);

	if (!noconfig)
		makemove("Makeconf");

	setcocmd();  /* to possibly check-out description file */
	setvpath();
	mainname = 0;

	/*
	 * Read command line "-f" arguments.
	 */
	for (i = 1; i < argc; i++)
		if (argv[i] && strcmp(argv[i], "-f") == 0) {
			argv[i] = 0;
			if (i >= argc-1)
				fatal(MSGSTR(NODESC,
				     "No description argument after -f flag"));
			if (rddescf(argv[++i]))
				fatal(MSGSTR(CANTOPEN,"Cannot open %s"),
				       argv[i]);
			argv[i] = 0;
			++descset;
		}

	if (!descset) {
		if (rddescf("makefile") == 0 || rddescf("Makefile") == 0)
			(void) rddescf(".depend");
		else if (descflag)
			fatal(MSGSTR(NOFILEDES,"No description file"));
	}

	setvpath();
	setcocmd();  /* may have been redefined in description file */

	if (prtrflag)
		printdesc(NO);

	if (srchname(".IGNORE"))
		++ignerr;
	if (srchname(".SILENT"))
		silflag = 1;
	if (p = srchname(".SUFFIXES"))
		sufflist = p->linep;
	if (!sufflist)
		fprintf(stderr, MSGSTR(SUFLIST, "No suffix list.\n"));
	if (p = srchname(".EXPORT"))
		export(p);
	if (didchdir() && (inobjdir = srchname(".INOBJECTDIR")) != 0) {
		doruntime(1, inobjdir);
		doruntime(2, inobjdir);
	}

	sighvalue = (signal(SIGHUP, SIG_IGN) == SIG_IGN);
	sigivalue = (signal(SIGINT, SIG_IGN) == SIG_IGN);
	sigqvalue = (signal(SIGQUIT, SIG_IGN) == SIG_IGN);
	sigtvalue = (signal(SIGTERM, SIG_IGN) == SIG_IGN);
	enbint(intrupt);

	if (p = srchname(".INIT")) {
		ch = 0;
		(void) doname(p, 0, &td, &ch);
	}

	nfargs = 0;
        if(dbgflag)
        {
                char pathname[MAXPATHLEN];
                if(getwd(pathname) == 0)
                {
                        fprintf(stderr, MSGSTR(WORKINGDIR, 
                        "Error trying to get current working directory, %s\n"), pathname);
                } else {
                        printf(MSGSTR(DISDIR,
                        "Current working directory for make is %s\n"), pathname);
                }
        }
	for (i = 1; i < argc; ++i)
		if (s = argv[i]) {
			p = makename(s);
			++nfargs;
			ch = 0;
			(void) doname(p, 0, &td, &ch);
			if (prtrflag)
				printdesc(YES);
		}

	/*
	 * If no file arguments have been encountered, make the first
	 * name encountered that doesn't start with a dot
	 */

	if (nfargs == 0) {
		if (mainname == 0)
			fatal(MSGSTR(NOARGS,
				"No arguments or description file"));
		ch = 0;
		(void) doname(mainname, 0, &td, &ch);
		if (prtrflag)
			printdesc(YES);
	}

	quit(0);
	/*NOTREACHED*/
}


struct lineblock *
isdependent(p, np)
	register char *p;
	register struct nameblock *np;
{
	register struct lineblock *lp;
	register struct depblock *dp;

	if (np == 0)
		return 0;
	for (lp = np->linep; lp; lp = lp->nxtlineblock)
		for (dp = lp->depp; dp; dp = dp->nxtdepblock)
			if (dp->depname && !unequal(p, dp->depname->namep))
				return lp;
	return 0;
}


int
isprecious(p)
	struct nameblock *p;
{
	static struct nameblock *prec = 0;

	if (prec == 0)
		prec = srchname(".PRECIOUS");
	return (p && (p->isarch || isdependent(p->namep, prec)));
}


FSTATIC void
intrupt(s)
	int s;
{
	char *p;
	struct nameblock *q;
	struct stat sbuf;

	if (!questflag && !touchflag && !noexflag
	&& (p = varptr("@")->varval) && stat(p, &sbuf) == 0
	&& (sbuf.st_mode & S_IFMT) == S_IFREG
	&& ((q = srchname(p)) == 0 || q->modtime != sbuf.st_mtime)) {
		if (!okdel || isprecious(q))
			printf(MSGSTR(FLEINC,"*** `%s' may be inconsistent\n"), p);
		else {
			printf(MSGSTR(FLERMV,"*** `%s' removed\n"), p);
			(void) fflush(stdout);
			(void) unlink(p);
		}
	}
	quit(s);
}


enbint(k)
	void (*k)();
{
	if (sighvalue == 0)
		(void) signal(SIGHUP, k);
	if (sigivalue == 0)
		(void) signal(SIGINT, k);
	if (sigqvalue == 0)
		(void) signal(SIGQUIT, k);
	if (sigtvalue == 0)
		(void) signal(SIGTERM, k);
}


extern FILE *fin;

FSTATIC int
rddescf(descfile)
	char *descfile;
{
	register struct nameblock *np;
	FILE *k;

	/*
	 * read and parse description
	 */
	if (!unequal(descfile, "-")) {
		rdd1(stdin, "standard input");
		return 0;
	}

	np = rcsco(descfile);
	if ((k = fopen(np->alias ? np->alias : descfile, "r")) != NULL) {
		rdd1(k, descfile);
		(void) fclose(k);
		return 0;
	}

	return 1;
}


FSTATIC char MAKEFILE[] = "MAKEFILE";

rdd1(k, fn)
	FILE *k;
	char *fn;
{
	extern int yylineno;
	extern char *zznextc;

	setvar(MAKEFILE, copys(fn), 1);
	curfname = fn;
	fin = k;
	yylineno = 0;
	zznextc = 0;

	if (yyparse())
		fatal(MSGSTR(FILEERR,"%s: Description file error"), fn);
}


FSTATIC
setcocmd()
{
	struct nameblock *p;
	register struct lineblock *lp;

	if (p = srchname(".CO")) {
		for (lp = p->linep; lp; lp = lp->nxtlineblock)
			if (lp->shp) {
				co_cmd = lp;
				break;
			}
		if (co_cmd == 0)
			coflag = NO;
		else if (p = srchname(".RCSTIME"))
			for (lp = p->linep; lp; lp = lp->nxtlineblock)
				if (lp->shp) {
					rcstime_cmd = lp;
					break;
				}
	} else
		coflag = NO;
}


entrybanner(ac, av)
	int ac;
	char **av;
{
	register int i;
	time_t now;
	extern time_t prestime();
	extern char *ctime();

	now = prestime();
	printf(MSGSTR(MKENTRY,"MAKE ENTRY (%s"), av[0]);
	for (i = 1; i < ac; ++i)
		printf(" %s", av[i]);
	printf(MSGSTR(CTIME,") at %s"), ctime(&now));
}


exitbanner(cc)
	int cc;
{
	time_t now;
	extern time_t prestime();
	extern char *ctime();

	now = prestime();
	printf(MSGSTR(MKEXIT,"MAKE EXIT (%d) at %s"), cc, ctime(&now));
}


FSTATIC
printdesc(prntflag)
	int prntflag;
{
	struct nameblock *p;
	struct depblock *dp;
	struct dirhdr *od;
	struct shblock *sp;
	struct lineblock *lp;

	if (prntflag) {
		printf(MSGSTR(OPENDIR,"Opened directories:\n"));
		for (od = firstod; od; od = od->nxtopendir)
			printf("\t%s\n", od->dirn);
		printf("\n");
	}

	if (firstvar) {
		printf(MSGSTR(MACROS,"Macros:\n"));
		printmac(firstvar);
	}

	for (p = firstname; p; p = p->nxtnameblock) {
		printf("\n%s", p->namep);
		if (p->linep)
			printf(":");
		if (prntflag)
			printf(MSGSTR(DONE,"  done=%d"), p->done);
		if (p == mainname)
			printf(MSGSTR(MAINNM,"  (MAIN NAME)"));
		for (lp = p->linep; lp; lp = lp->nxtlineblock) {
			if (dp = lp->depp) {
				printf(MSGSTR(DEPENDSON,"\n depends on:"));
				for (; dp; dp = dp->nxtdepblock)
					if (dp->depname)
						printf(" %s ", dp->depname->namep);
			}

			if (sp = lp->shp) {
				printf(MSGSTR(CMDS,"\n commands:"));
				for (; sp; sp = sp->nxtshblock)
					printf("\n\t%s", sp->shbp);
			}
		}
	}
	printf("\n");
}


FSTATIC
printmac(vp)
	register struct varblock *vp;
{
	if (vp->lftvarblock)
		printmac(vp->lftvarblock);
	printf(" %9s = %s\n" , vp->varname , vp->varval);
	if (vp->rgtvarblock)
		printmac(vp->rgtvarblock);
}


/*
 * Handle options and MAKEFLAGS and MFLAGS.
 */
FSTATIC
setflags(ac, av)
	int ac;
	char **av;
{
	register int i, j;
	register char c;
	int flflg = 0;		/* flag to note `-f' option. */

	for (i = 1; i < ac; ++i) {
		if (flflg) {
			flflg = 0;
			continue;
		}
		if (av[i] && av[i][0] == '-') {
			if (index(av[i], 'f'))
				flflg++;
			for (j = 1; c = av[i][j]; ++j)
				optswitch(c);
			av[i] = flflg ? "-f" : 0;
		}
	}
}


/*
 * Handle a single char option.
 */
FSTATIC
optswitch(c)
	register char c;
{
	switch (c) {
	case 'c':
		coflag = NO;
		setmflgs(c);
		break;
	case 'C':
		coflag = YES;
		setmflgs(c);
		break;
	case 'E':
		argsinenv = NO;
		setmflgs(c);
		break;
	case 'e':
		envover = YES;
		setmflgs(c);
		break;
	case 'd':
		dbgflag = YES;
		setmflgs(c);
		break;
	case 'p':
		prtrflag = YES;
		break;
	case 's':
		silflag = YES;
		setmflgs(c);
		break;
	case 'i':
		ignerr = YES;
		setmflgs(c);
		break;
	case 'S':
		keepgoing = NO;
		setmflgs(c);
		break;
	case 'k':
		keepgoing = YES;
		setmflgs(c);
		break;
	case 'F':
		descflag = YES;
		setmflgs(c);
		break;
	case 'n':
		noexflag = YES;
		setmflgs(c);
		break;
	case 'r':
		noruleflag = YES;
		break;
	case 'N':
		noconfig = YES;
		setmflgs(c);
		break;
	case 't':
		touchflag = YES;
		setmflgs(c);
		break;
	case 'q':
		questflag = YES;
		setmflgs(c);
		break;
	case 'm':
		machdep = YES;
		setmflgs(c);
		break;
	case 'u':
		rmflag = NO;
		setmflgs(c);
		break;
	case 'U':
		rmflag = YES;
		setmflgs(c);
		break;
	case 'x':
		nocmds = YES;
		setmflgs(c);
		break;
	case 'y':
		rcsdep = YES;
		setmflgs(c);
		break;
	case 'f':
		/*
		 * Named makefile; already handled by setflags().
		 */
		break;
	case 'b':
		botchflag = YES;
		setmflgs(c);
		break;
	default:
		fatal(MSGSTR(BADFLAG,"Unknown flag argument %c"), c);
	}
}


/*
 * getmflgs() set the cmd line flags into an EXPORTED variable
 * for future invocations of make to read.
 */
FSTATIC
getmflgs()
{
	register struct varblock *vpr;
	register char **pe;
	register char *p;
	static char mflags[64];  /* XXX */
	extern char **environ;

	vpr = varptr(Makeflags);
	vpr->varval = mflags;
	vpr->varval[0] = 0;
	vpr->noreset = 1;
	vpr->builtin = 1;
	vpr->used = 1;
	for (pe = environ; *pe; pe++) {
		if (strncmp(*pe, "MAKEFLAGS=", 10) == 0) {
			for (p = (*pe) + 10; *p; p++)
				optswitch(*p);
			return;
		}
	}
}


/*
 * setmflgs(c) sets up the cmd line input flags for EXPORT.
 * Also adds them to "MFLAGS" for compatibility with 4.3 make.
 */
FSTATIC
setmflgs(c)
	register char c;
{
	register struct varblock *vpr;
	register char *p;

	for (p = options; *p; p++)
		if (*p == c)
			break;
	if (*p == 0) {
		*p++ = c;
		*p = 0;
	}
	vpr = varptr(Makeflags);
	for (p = vpr->varval; *p; p++)
		if (*p == c)
			return;
	*p++ = c;
	*p = 0;
}


/*
 * getmdefs() allocate space for command line definitions
 * which may then be passed on to recursive makes.  Note
 * that they are not automatically imported/exported.
 */
FSTATIC
getmdefs()
{
	register struct varblock *vpr;
	static char mdefs[BUFSIZ];  /* XXX */

	vpr = varptr(Makedefs);
	vpr->varval = mdefs;
	vpr->varval[0] = 0;
	vpr->noreset = 1;
	vpr->builtin = 1;
	vpr->used = 0;
}


/*
 * setmdefs(s) adds the command line definition to $(MAKEDEFS).
 */
FSTATIC
setmdefs(s)
	register char *s;
{
	register struct varblock *vpr;
	register char *p;

	vpr = varptr(Makedefs);
	for (p = vpr->varval; *p; p++)
		;
	if (p != vpr->varval)
		*p++ = ' ';
	*p++ = '\'';
	while (*s) {
		if (*s == '\'') {
			*p++ = *s;
			*p++ = '\\';
			*p++ = *s;
		}
		*p++ = *s++;
	}
	*p++ = '\'';
	*p = 0;
}

