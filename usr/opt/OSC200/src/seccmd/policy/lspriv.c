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
static char	*sccsid = "@(#)$RCSfile: lspriv.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:35:18 $";
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
/* Copyright (c) 1988-90 SecureWare, Inc.
 *   All rights reserved.
 *
 * lspriv command, which prints privilege sets of files
 *   Usage:  lspriv [-R] [-p] [-g] [-a] file . . .
 *   If no arguments, lists both privilege sets of files in current directory.
 *   If directories are given, list privilege sets of files in the directory.
 *   -R -- Recurse
 *   -p -- potential set only
 *   -g -- granted set only
 *   -a -- do all files (including those which begin with "."
 */



/*
 * Based on:

 */

#include <sys/secdefines.h>


#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "policy_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_PRIV

#include <sys/types.h>
#include <dirent.h>
typedef struct dirent DIRENTRY;

#include <sys/stat.h>
#include <stdio.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static int	nodotdirs();
static int	isregfile();
static void	dodir();
static void	freenl();
static int	scompar();
static void	output_file();
void	mem_error();

extern int	alphasort();
void	print_er();
extern char	*malloc();
extern char	*realloc();
extern char	*privstostr();

char	*program;
char	pflag, gflag, aflag, rflag;
/* rock to shove the current directory under for
 * the sort routine called from scandir
 */
char	*CurrentDir;

main(argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	extern char *optarg;
	extern int optind, opterr;
	char	errflag = 0;
	DIRENTRY **namelist;
	struct	stat	sb;
	int	firstopt;
	int	ret;
	int	nfiles;
	char	**filetab;
	int	ndirs;
	char	**dirtab;
	int	widest_file;
	int	i;


#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);
#endif

	program = argv[0];
        set_auth_parameters (argc, argv);
        while ((c = getopt(argc, argv, "pgaR")) != EOF) {
		switch (c) {
		case 'p':
			pflag++;
			break;
		case 'g':
			gflag++;
			break;
		case 'a':
			aflag++;
			break;
                case 'R':
                        rflag++;
                        break;
		case '?':
			errflag++;
			break;
		}
	}

	/* do both if didn't specify either */
	if (pflag == 0 && gflag == 0) {
		pflag++;
		gflag++;
	}
	if (optind == argc) { /* no arguments: do current directory */
		/* look at current directory only */
		dodir("", "");
		exit(0);
	}

	/* collect files into file and directory lists */
	firstopt = optind;
	filetab = (char **) calloc(argc - optind, sizeof(char *));
	dirtab  = (char **) calloc(argc - optind, sizeof(char *));
	if (filetab == (char **) 0 || dirtab == (char **) 0)
		mem_error(MSGSTR(LSPRIV_1, "file list"));
	nfiles = 0;
	ndirs = 0;
	widest_file = 0;
	for (optind = firstopt; optind < argc; optind++) {
		register char *this = argv[optind];
		int	len;

		/* for each directory, look at it or its contents */
		ret = stat(this, &sb);
		if (ret == -1) {
			perror(this);
			errflag++;
			continue;
		}
		if ((sb.st_mode & S_IFMT) == S_IFDIR) {
			dirtab[ndirs++] = argv[optind];
		} else {
			filetab[nfiles++] = this;
			len = strlen(this);
			if (len > widest_file)
				widest_file = len;
		}
	}
	/* print files */
	if (nfiles > 0) {
		qsort(filetab, nfiles, sizeof (char *), scompar);
		for (i = 0; i < nfiles; i++)
			output_file(filetab[i], widest_file, 0, 0);
	}
	
	/* do directories */
	if (ndirs > 0) {
		qsort(dirtab, ndirs, sizeof(char *), scompar);
		for (i = 0; i < ndirs; i++) {
			if (nfiles + ndirs != 1)
				printf("\n%s:\n", dirtab[i]);
			dodir("", dirtab[i]);
		}
	}

	exit(0);
}

/* weed out any files that start with "." */
static int
nodotdirs(d)
register DIRENTRY *d;
{
	if (d->d_name[0] == '.')
		return(0);
	return(isregfile(d));
}

/* weed out any files which aren't regular */
static int
isregfile(d)
register DIRENTRY *d;
{
	static char *pathname = (char *) 0;
	static int pathlen = 0;
	int len;
	char *comp = d->d_name;
	struct stat sb;

	len = strlen(CurrentDir) + strlen(comp) + 2;

	if (pathlen == 0) {
		pathname = malloc(len);
		pathlen = len;
	} else if (len > pathlen) {
		pathname = realloc(pathname, len);
		pathlen = len;
	}
	if (pathname == (char *) 0) {
		fprintf(stderr,
	MSGSTR(LSPRIV_2, "Memory allocation error on file %s in directory %s\n"),
		  comp, CurrentDir);
		exit(1);
	}
	sprintf(pathname, "%s/%s", CurrentDir, comp);
	if (stat(pathname, &sb) < 0)
		return(0);
        if (((sb.st_mode & S_IFMT) != S_IFREG) &&
             ((sb.st_mode & S_IFMT) != S_IFDIR))
		return(0);
	return(1);
}

static void
dodir(pre_name, name)
char *pre_name;
char *name;
{
	int entries;
	DIRENTRY **namelist = (DIRENTRY **) 0;
	char *dirname;
	int fd;
	int widest;
	int thislen;
	char *filename;
	int i;
	struct stat sb;

	dirname = malloc(strlen(pre_name) + strlen(name) + 2);
	if (dirname == (char *) 0)
		mem_error(pre_name);
	if (pre_name[0] == '\0')
		if (name[0] == '\0') {
			dirname[0] = '.';
			dirname[1] = '\0';
		} else
			strcpy(dirname, name);
	else if (pre_name[strlen(pre_name) - 1] == '/')
		sprintf(dirname, "%s%s", pre_name, name);
	else
		sprintf(dirname, "%s/%s", pre_name, name);
	/* set global variable so file type can be checked in isregfile */
	CurrentDir = dirname;
	/* search out regular files only */
	entries = scandir(dirname, &namelist,
                        aflag ? NULL : nodotdirs, alphasort);
	if (entries == -1) {
		fd = open(dirname, 0);
		if (fd < 0) {
			fprintf(stderr, MSGSTR(LSPRIV_3, "%s unreadable\n"), dirname);
			free(dirname);
			return;
		}
		else {
			(void) close(fd);
			mem_error(dirname);
			/* never returns */
		}
	}
	if (entries > 0)
		output_file(dirname, 0, namelist, entries);
	if (rflag && entries > 0) {
		widest = 0;
		for (i = 0; i < entries; i++) {
			thislen = strlen(namelist[i]->d_name);
			if (thislen > widest)
				widest = thislen;
		}
		filename = malloc(widest + strlen(dirname) + 2);
		if (filename == (char *) 0)
			mem_error(dirname);
		for (i = 0; i < entries; i++) {
			if (dirname[strlen(dirname)-1] == '/')
				sprintf(filename, "%s%s", dirname,
				  namelist[i]->d_name);
			else
				sprintf(filename, "%s/%s", dirname,
				  namelist[i]->d_name);
			if (stat(filename, &sb) < 0)
				continue;
                        if (((sb.st_mode & S_IFMT) == S_IFDIR) &&
                                (strcmp(namelist[i]->d_name, ".") != 0) &&
                                (strcmp(namelist[i]->d_name, "..") != 0)) {
				printf("\n%s:\n", filename);
				dodir(dirname, namelist[i]->d_name);
			}
		}
		free(filename);
	}
	free(dirname);
	freenl(namelist, entries);
}

void
mem_error(name)
{
	(void) fprintf(stderr, MSGSTR(LSPRIV_4, "%s: memory allocation error on %s.\n"),
	  program, name);
	exit(1);
}

static void
freenl(nl, entries)
DIRENTRY	*nl[];
int	entries;
{
	int	i;

	if (nl) {
		for (i = 0; i < entries; i++)
			free((char *) nl[i]);
		free((char *) nl);
	}
}

static int
scompar(sp1, sp2)
char **sp1, **sp2;
{

	return(strcmp(*sp1, *sp2));
}

/* print a list of files to standard output, by padding out all names to
 * the widest name plus 3 columns and calling print_er for each file name.
 */

static void
output_file(name, widest_file, nl, entries)
char *name;
register DIRENTRY *nl[];
int	entries;
int	widest_file;
{
	register int	i;
	int	len;
	static	char	*filebuf = (char *) 0;
	static	int	 filebuflen = 0;
	int	bufsize;

	/* Decide how big the buffer needs to be and either malloc or
	 * realloc one.
	 */
	if (entries != 0) {
		for (i = 0; i < entries; i++) {
			len = strlen(nl[i]->d_name);
			if (len > widest_file)
				widest_file = len;
		}
		/* need spaces for intervening '/' and '\0' */
		bufsize = widest_file + strlen(name) + 2;
		if (bufsize > filebuflen) {
			filebuf = (filebuflen == 0) ?
			  malloc(bufsize) :
			  realloc(filebuf, bufsize);
			if (filebuf == (char *) 0)
				mem_error(MSGSTR(LSPRIV_5, "buffer allocation"));
			filebuflen = bufsize;
		}
	}

	if (entries == 0)
		print_er(name, name, widest_file + 3);
	else {
		for (i = 0; i < entries; i++) {
			sprintf(filebuf, "%s/%s", name, nl[i]->d_name);
			print_er(filebuf, nl[i]->d_name, widest_file + 3);
		}
	}
	return;
}

/* print routine for the privilege sets */

void
print_er(pathname, name, columns)
char *pathname;
char *name;
int columns;
{
	privvec_t	privs;

	printf("%s:", name);
	printf("%*s", columns - strlen(name) - 1, "");
	if (pflag) {
		printf(MSGSTR(LSPRIV_6, "potential "));
		if (statpriv(pathname, SEC_POTENTIAL_PRIV, privs) < 0)
			printf(MSGSTR(LSPRIV_7, "unavailable\n"));
		else
			print_privvec(privs, columns + 10);
		if (gflag)
			printf("%-*s", columns, "");
	}
	if (gflag) {
		printf(MSGSTR(LSPRIV_8, "granted   "));
		if (statpriv(pathname, SEC_GRANTED_PRIV, privs) < 0)
			printf(MSGSTR(LSPRIV_7, "unavailable\n"));
		else
			print_privvec(privs, columns + 10);
	}
		
}

print_privvec(privs, columns)
priv_t *privs;
int columns;
{
	char *privbuf;

	privbuf = privstostr(privs, ",");
	if (privbuf == (char *) 0)
		printf(MSGSTR(LSPRIV_9, "cannot convert\n"));
	else {
		printbuf(privbuf, columns, ",");
		if (privbuf[0] == '\0')
			putchar('\n');
	}
}
#endif /* SEC_PRIV */
