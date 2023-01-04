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
static char *rcsid = "@(#)$RCSfile: catman.c,v $ $Revision: 4.3.9.4 $ (DEC I18N) $Date: 1993/10/07 21:54:16 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: doit
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
 * catman.c	1.5  com/cmd/man,3.1,9021 1/15/90 15:33:01
 */

/*
 * NAME: catman [ -p ] [ -n ] [ -w ] [-M path] [sections]
 *
 * FUNCTION: 
 *	The catman command creates the preformatted versions of the on-line
 *	manual from the nroff input files.  Each manual page is examined and
 * 	those whose preformatted versions are missing or out of date are 
 *	recreated. If any changes are made, catman will recreate the whatis
 *  	database.
 * FLAGS:
 *	-n	prevents creations of the whatis database.		 *004*
 *	-p 	prints what would be done instead of doing it.
 *	-w	causes only the whatis database to be created. No manual *004*
 *		reformatting is done.
 *	-M	updates manual pages located in the set of directories
 *		specified by path.
 *
 */ 

/* Maintenance History
 *
 * 001	gws	(OSF_QAR 0116)
 *	add "8" to the list of sections formatted when no list of sections
 *	  is specified
 *
 * 002	gws	(OSF_QARs 1092, 1096)
 *	added support for multi-character subsections by removing check for
 *	  NULL character after ".[0-9][<alpha>]" in filename parsing code.
 *	Updated comments which describe valid filename syntax.
 *
 * 003	gws 	(OSF_QARs 0113, 1714)
 *	if -w flag specified, return from doit() after updating "whatis"
 *	  database
 *
 * 004	gws	(OSF_QARs 1550, 1671, 1672)
 *	changed to no longer ignore manpages whose 1st line contains
 *		.soman?*
 *	  	.so?*
 *	or	.so<blank>?*
 *	where:		?*  = anything other than "man"
 *	
 *	corrected "datebase" to "database" in all comments
 *
 * 005	gws	(OSF_QAR 1812)
 *	nroff now invoked with -Tlp option (for default printing device)
 *	so it is now consistent with the man(1) command.
 *	
 *	minor comment corrections
 *
 * 006	gws 11/04/91	(OSF_QAR 1695)
 *	manpages sources are now preprocessed through the tbl(1) and neqn(1)
 *	commands, and post-processed through the col(1) command with -p option
 *	If the nroff step has an error, no output file is created.
 *
 * 007	gws 11/27/91	(OSF_QARs 2217, 2253)
 *	remove postprocessing of manpages through "col -p", because
 *	  col(1) command counts the Escape codes as text that "-p" passes
 *	  through, so col miscalculates no. of tabs to generate when it
 *	  converts spaces into tabs by default.  Also, col is NOT required
 *	  to make tables vertically alligned, when the "-Tlp" device is
 *	  associated with a device that has reverse-linefeed AND the "-Tlp"
 *	  device is also the default terminal display device for the system.
 *	add the -h option to the nroff call, because this generates tabs,
 *	  and the "-Tlp" default device needs the tabs to step over left
 *	  columns in multicolumn tables containing text blocks, else the
 *	  left columns get wiped out.  This also makes catman consistent with
 *	  the man command, which was already using nroff -h for creating cat?
 *	  files.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/time.h>

#include <dirent.h>
#include <ctype.h>
#include <locale.h>

#include "catman_msg.h"
nl_catd catd;
#define MSGSTR(n, s) catgets(catd, MS_CATMAN, n, s)

#define USE() \
	printf(MSGSTR(USAGE, "usage: catman [-p] [-n] [-w] [-M path] [sections]\n")); \
	exit(-1)

#define WHATIS() \
	sprintf(buf, "%s %s", makewhatis, mandir); \
	if (pflag) \
		printf("%s\n", buf); \
	else if ((status = system(buf)) != 0) { \
		fprintf(stderr, MSGSTR(STATUS, "catman: %s: exit status %d\n"), buf, status); \
		exstat = 1; \
		}


char	buf[BUFSIZ];
char	pflag;
char	nflag;
char	wflag;
char	man[NAME_MAX+6] = "manx/";
int	exstat = 0;
char	cat[NAME_MAX+6] = "catx/";
char	lncat[NAME_MAX+9] = "../catx/";
#ifdef	MANPATH
char	*manpath = MANPATH;
#else	/* MANPATH */
char	*manpath = "/usr/share/man";
#endif	/* MANPATH */
char	*sections = "12345678";					/* 001 */
char	*makewhatis = "/usr/lbin/mkwhatis";
char	*index(), *rindex();
char	*NLstrcpy();

/*
 * Look for the supported format codes %L, %l, %t, %c & %%.
 * These format codes are then translated into :
 *      %L -> value of LC_MESSAGES environment variable
 *      %P -> value of LC_MESSAGES environment variable excluding the modifier
 *      %l -> language element of the LC_MESSAGES environment variable
 *      %t -> territory element of the LC_MESSAGES environment variable
 *      %c -> codeset element of the LC_MESSAGES environment variable
 * assuming that the value of LC_MESSAGES environment variable will be
 * in the format:
 *      language[_territory[.codeset]][@modifier]
 */
static char *
expand_path(path)
	char *path ;
{
	static char buf[BUFSIZ+1] ;
	char	    loc[BUFSIZ+1] ;
	char       *Pptr          ;    /* Path name pointer      */
	char       *lptr          ;    /* Language name pointer  */
	char       *tptr          ;    /* Territory name pointer */
	char       *cptr          ;    /* Codeset name pointer   */
	char	   *fptr	  ;    /* Format code pointer    */
	char	   *bptr	  ;
	char	   *eptr	  ;
	char	   *pptr	  ;
	int	    len		  ;

	if ((fptr = (char *)index(path, '%')) == NULL)
		return(path) ;	/* No format code, return */
	/*
	 * Fetch current locale name
	 */
	eptr = (char *)setlocale(LC_MESSAGES, NULL) ;
	if ((eptr == NULL) || (strcmp(eptr, "C") == 0))
		return(path) ;	/* No translation */

	strcpy(loc, eptr) ;
	/*
	 * The locale name is duplicated to isolate the language, territory
	 * and codeset component.
	 */
	Pptr = loc ;
	lptr = loc + strlen(loc) + 1 ;
	strcpy(lptr, eptr) ;
	if ((eptr = (char *)index(lptr, '@')) != NULL) {
		*eptr = '\0' ;	/* Remove modifier */
		Pptr  = lptr ;
		lptr  = eptr + 1   ;
		strcpy(lptr, Pptr) ;
	}
	if ((cptr = (char *)index(lptr, '.')) != NULL)
		*cptr++ = '\0';	/* Isolate codeset component */
	if ((tptr = (char *)index(lptr, '_')) != NULL)
		*tptr++ = '\0';	/* Isolate territory component */
	/*
	 * Expand the path name
	 */
	pptr = path ;
	bptr = buf  ;
	do {
		len = fptr - pptr ;
		if (len >= BUFSIZ - 20 - (bptr - buf))
			return(path)	;	/* Path too long */
		bcopy(pptr, bptr, len)	;
		pptr += len + 1		;
		bptr += len		;
		switch (*pptr++) {
		    case 'L':	strcpy(bptr, loc)    ;
				bptr += strlen(loc)  ;
				break		     ;
		    case 'P':	strcpy(bptr, Pptr)   ;
				bptr += strlen(Pptr) ;
				break		     ;
		    case 'l':	strcpy(bptr, lptr)   ;
				bptr += strlen(lptr) ;
				break		     ;
		    case 't':	if (tptr != NULL) {
				    strcpy(bptr, tptr)   ;
				    bptr += strlen(tptr) ;
				}
				break ;
		    case 'c':	if (cptr != NULL) {
				    strcpy(bptr, cptr)   ;
				    bptr += strlen(cptr) ;
				}
				break ;
		    case '%':   *bptr++ = '%' ;
				break 	      ;
		    default :	*bptr++ = '%' ;
				pptr--	      ;
				break	      ;
		}
	} while ((fptr = (char *)index(pptr, '%')) != NULL) ;
	strcpy(bptr, pptr) ;
	return(buf)	   ;
}

main(ac, av)
	int ac;
	char *av[];
{
	char *mp, *nextp;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_CATMAN, NL_CAT_LOCALE);

	if ((mp = getenv("MANPATH")) != NULL)
		manpath = mp;

	ac--, av++;
	while (ac > 0 && av[0][0] == '-') {
		switch (av[0][1]) {

		case 'p':
			pflag++;
			break;

		case 'n':
			nflag++;
			break;

		case 'w':
			wflag++;
			break;

		case 'M':
			if (ac <= 1) {
				fprintf(stderr, MSGSTR(MISSPATH, "catman: -M option missing path\n"));
				exit(1);
			}
			ac--, av++;
			manpath = *av;
			break;

		default:
			USE();
			break;
		}
		ac--, av++;
	}
	if (ac > 1) { USE(); }
	if (ac == 1)
		sections = *av;
	for (mp = manpath; mp && ((nextp = index(mp, ':')), 1); mp = nextp) {
		if (nextp)
			*nextp++ = '\0';

		if ((mp = expand_path(mp)) == NULL)
			continue ;
		/* start of kak-009 */
		/* test to see if directory mp exists */
		if (access(mp, F_OK) == -1) {
			sprintf(buf, MSGSTR(NOTDIR, "catman: %s: not a directory"),mp);
			perror(buf);
			exstat = 1;
			continue;
		}
		else if (access(mp, R_OK|W_OK|X_OK) == -1){/*check permissions*/
			sprintf(buf, MSGSTR(PERMISSION, "Check write permissions or su NFS permissions for %s"),mp);
			perror(buf);
			exstat = 1;
			continue;
		}
		/* end of kak-009 */
		doit(mp);
	}
	exit(exstat);
}

/*
 * NAME: doit
 *
 * FUNCTION: for each man file in each manual section , performs a nroff 
 *	     function on it.  The resulting files are cat files.	* 005 *
 *
 */
doit(mandir)
	char *mandir;
{
	register char *msp, *csp, *sp;
	int changed = 0;
	int status;

	if (wflag) {	/* update only "whatis" database */ 	/* 003 */
		WHATIS()					/* 003 */
		return;						/* 003 */
	}							/* 003 */

	if (chdir(mandir) < 0) {
		sprintf(buf, MSGSTR(MANDIR, "catman: %s"), mandir);
		perror(buf);
		return;
	}
	if (pflag)
		printf(MSGSTR(CDMANDIR, "cd %s\n"), mandir);
	msp = &man[5];
	csp = &cat[5];
	(void) umask((mode_t)0);

/* looks in a list of manual sections */
	for (sp = sections; *sp; sp++) {
		register DIR *mdir;
		register struct dirent *dir;
		struct stat sbuf;

		man[3] = cat[3] = *sp;
		*msp = *csp = '\0';

/* opens a man directory, makes a cat directory and checks its accessing modes */ 
		if ((mdir = opendir(man)) == NULL) {
			sprintf(buf, MSGSTR(OPENDIR, "catman: opendir: %s"), man);
			perror(buf);
			continue;
		}
		if (stat(cat, &sbuf) < 0) {
			register char *cp;

			(void) NLstrcpy(buf, cat);
		        cp = rindex(buf, '/');
			if (cp && cp[1] == '\0')
				*cp = '\0';
			if (pflag)
				printf(MSGSTR(MKDIR, "mkdir %s\n"), buf);
			else if (mkdir(buf, (mode_t)0777) < 0) {
				sprintf(buf, MSGSTR(MKDIRCAT, "catman: mkdir: %s"), cat);
				perror(buf);
				exstat = 1;
				continue;
			}
			(void) stat(cat, &sbuf);
		}
		if (access(cat, R_OK|W_OK|X_OK) == -1) {
			sprintf(buf, MSGSTR(CAT, "catman: %s"), cat);
			perror(buf);
			exstat = 1;
			continue;
		}
		if ((sbuf.st_mode & S_IFMT) != S_IFDIR) {
			fprintf(stderr, MSGSTR(NOTDIR, "catman: %s: not a directory\n"), cat);
			exstat = 1;
			continue;
		}

/* reads in each file in one manual section and creates the cat file for it */
		while ((dir = readdir(mdir)) != NULL) {
			time_t time;
			register char *tsp;
			FILE *inf;
			int  makelink;

			if ((dir->d_fileno == 0) || (0 == NLstrcmp(dir->d_name, ".")) || (0 == NLstrcmp(dir->d_name, "..")))
				continue;
			/*
			 * Make sure this is a man file, i.e., that it
			 * ends in .[0-9] , .[0-9][<alpha>] ,		**002**
			 * .[0-9][<alpha>][<anything>]* ,		**002**
			 * .[<section>] , .[<section>][<alpha>] ,	**002**
			 * or .[<section>][<alpha>][<anything>]* .	**002**
			 */
			tsp = rindex(dir->d_name, '.');
			if (tsp == NULL)
				continue;
			if (!isdigit((int)*++tsp) && *tsp != *sp)
				continue;
			if (*++tsp && !isalpha((int)*tsp))
				continue;
								/* 002 */
			(void) NLstrcpy(msp, dir->d_name);
			if ((inf = fopen(man, "r")) == NULL) {
				sprintf(buf, "catman: %s", man);      /* 008 */
				perror(buf);
				exstat = 1;
				continue;
			}

			makelink = 0;
			/* if first line of manpage starts with ".so"  * 004 */
			if (getc(inf) == '.' && getc(inf) == 's'
			    && getc(inf) == 'o') {
								      /* 004 */
				if ((lncat[3] = getc(inf)) == ' ')    /* 004 */
					fgets(lncat+3, (int)(sizeof(lncat)-3), inf);
								      /* 004 */
				else fgets(lncat+4, (int)(sizeof(lncat)-4), inf);
				if (lncat[strlen(lncat)-1] == '\n')
					lncat[strlen(lncat)-1] = '\0';
				if (strncmp(lncat+3, "man", 3) == 0) { /*004 */
								      /* 004 */
					bcopy("../cat", lncat, sizeof("../cat")-1);
					makelink = 1;
				}
			}
			fclose(inf);

			(void) NLstrcpy(csp, dir->d_name);
			if (stat(cat, &sbuf) >= 0) {
				time = sbuf.st_mtime;
				(void) stat(man, &sbuf);
				if (time >= sbuf.st_mtime)
					continue;
				(void) unlink(cat);
			}
			if (makelink) {
				/*
				 * Don't unlink a directory by accident.
				 */
				if (stat(lncat+3, &sbuf) >= 0 &&
				    (((sbuf.st_mode&S_IFMT)==S_IFREG) ||
				     ((sbuf.st_mode&S_IFMT)==S_IFLNK)))
					(void) unlink(cat);
				if (pflag)
					printf(MSGSTR(LNCAT, "ln -s %s %s\n"), lncat, cat);
				else
					if (symlink(lncat, cat) == -1) {
						sprintf(buf, MSGSTR(SYMLINK, "catman: symlink: %s"), cat);
						perror(buf);
						exstat = 1;
						continue;
					}
			}
			else {
							       /* -Tlp * 005 */
					/* tbl, neqn, col -p  added:   * 006 */
				 /* nroff "-h" added; col -p removed.  * 007 */
				sprintf(buf, MSGSTR(NROFF, "tbl %s | neqn | nroff -Tlp -man -h - > %s || (stat=$? ; rm %s ; exit $stat)"), man, cat, cat);
				if (pflag)
					printf("%s\n", buf);
				else if ((status = system(buf)) != 0) {
					fprintf(stderr, MSGSTR(BADMAN, "catman: nroff: %s: exit status %d\n")
					, cat, status);
					exstat = 1;
					continue;
				}
			}
			changed = 1;
		}
		closedir(mdir);
	}

/* updates the whatis database */				/* 004 */
	if (changed && !nflag) { WHATIS() }

	return;
}
