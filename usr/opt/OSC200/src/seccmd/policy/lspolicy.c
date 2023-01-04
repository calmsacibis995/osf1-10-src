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
static char	*sccsid = "@(#)$RCSfile: lspolicy.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:35:11 $";
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
 * ls{policy} command driver, which prints out security attributes of files
 *   Usage:  ls{policy} [-d] [-R] [-a] file . . .
 *   If no arguments, lists level of all files in current directory.
 *   -d means list all files as directories.
 *   -r means recursive.
 *   -a means all files.
 * Compile with different files which implement print_er routines
 * for different policies.  The print_er routine prints a security
 * attribute on stdout, possibly using the support routines in printbuf.c.
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

#if SEC_ARCH
#include <sys/types.h>
#include <dirent.h>

typedef struct dirent DIRENTRY;

#include <sys/stat.h>
#include <stdio.h>
#include <sys/security.h>
#include <prot.h>

char	dflag, rflag, aflag;

static int	nodotdirs();
static int	dotonly();
static void	dodir();
static void	freenl();
static int	scompar();
static void	output_file();
void	mem_error();

extern int	alphasort();
extern void	print_er();
extern char	*malloc();
extern char	*realloc();

main (argc, argv)
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

	set_auth_parameters(argc, argv);

	while ((c = getopt (argc, argv, "dRa")) != EOF) {
		switch (c) {
		case 'd':
			dflag++;
			break;
		case 'R':
			rflag++;
			break;
		case 'a':
			aflag++;
			break;
		case '?':
			errflag++;
		}
	}

	if (optind == argc) { /* no arguments: do current directory */
		/* look at current directory only */
		if (dflag)
			output_file (".", 1, (DIRENTRY **) 0, 0);
		else
			dodir ("", "");
		exit (0);
	}

	/* collect files into file and directory lists */
	firstopt = optind;
	filetab = (char **) calloc (argc - optind, sizeof (char *));
	dirtab  = (char **) calloc (argc - optind, sizeof (char *));
	if (filetab == (char **) 0 || dirtab == (char **) 0)
		mem_error(MSGSTR(LSPOLICY_1, "file list"));
	nfiles = 0;
	ndirs = 0;
	widest_file = 0;
	for (optind = firstopt; optind < argc; optind++) {
		register char *this = argv[optind];
		int	len;

		/* for each directory, look at it or its contents */
		ret = stat (this, &sb);
		if (ret == -1) {
			perror (this);
			errflag++;
			continue;
		}
		if ((sb.st_mode & S_IFMT) == S_IFDIR && !dflag) {
			dirtab[ndirs++] = argv[optind];
		} else {
			filetab[nfiles++] = this;
			len = strlen (this);
			if (len > widest_file)
				widest_file = len;
		}
	}
	/* print files */
	if (nfiles > 0) {
		qsort (filetab, nfiles, sizeof (char *), scompar);
		for (i = 0; i < nfiles; i++)
			output_file (filetab[i], widest_file, 0, 0);
	}
	
	/* do directories */
	if (ndirs > 0) {
		qsort (dirtab, ndirs, sizeof (char *), scompar);
		for (i = 0; i < ndirs; i++) {
			if (nfiles + ndirs != 1)
				printf ("\n%s:\n", dirtab[i]);
			dodir ("", dirtab[i]);
		}
	}

	exit (0);
}

static int
nodotdirs (d)
register DIRENTRY *d;
{
	if (d->d_name[0] == '.')
		return (0);
	else	return (1);
}

static void
dodir (pre_name, name)
char *pre_name;
char *name;
{
	int entries;
	DIRENTRY **namelist;
	char *dirname;
	int fd;
	int widest;
	int thislen;
	char *filename;
	int i;
	struct stat sb;

	dirname = malloc (strlen (pre_name) + strlen (name) + 2);
	if (dirname == (char *) 0)
		mem_error (pre_name);
	if (pre_name[0] == '\0')
		if (name[0] == '\0') {
			dirname[0] = '.';
			dirname[1] = '\0';
		} else
			strcpy (dirname, name);
	else if (pre_name[strlen(pre_name) - 1] == '/')
		sprintf (dirname, "%s%s", pre_name, name);
	else
		sprintf (dirname, "%s/%s", pre_name, name);
	entries = scandir (dirname, &namelist,
			aflag ? NULL : nodotdirs, alphasort);
	if (entries == -1) {
		fd = open (dirname, 0);
		if (fd < 0) {
			fprintf (stderr, MSGSTR(LSPOLICY_2, "%s unreadable\n"), dirname);
			free (dirname);
			return;
		}
		else {
			(void) close (fd);
			mem_error (dirname);
			/* never returns */
		}
	}
	if (entries > 0)
		output_file (dirname, 0, namelist, entries);
	if (rflag && entries > 0) {
		widest = 0;
		for (i = 0; i < entries; i++) {
			thislen = strlen (namelist[i]->d_name);
			if (thislen > widest)
				widest = thislen;
		}
		filename = malloc (widest + strlen (dirname) + 1);
		if (filename == (char *) 0)
			mem_error (dirname);
		for (i = 0; i < entries; i++) {
			if (dirname[strlen(dirname)-1] == '/')
				sprintf (filename, "%s%s", dirname,
				  namelist[i]->d_name);
			else
				sprintf (filename, "%s/%s", dirname,
				  namelist[i]->d_name);
			if (stat (filename, &sb) < 0)
				continue;
			if (((sb.st_mode & S_IFMT) == S_IFDIR) &&
				(strcmp(namelist[i]->d_name, ".") != 0) &&
				(strcmp(namelist[i]->d_name, "..") != 0)) {
				printf ("\n%s:\n", filename);
				dodir (dirname, namelist[i]->d_name);
			}
		}
		free (filename);
	}
	free (dirname);
	freenl (namelist, entries);
}

void
mem_error (name)
{
	(void) fprintf (stderr, MSGSTR(LSPOLICY_3, "%s: memory allocation error on %s.\n"),
	  command_name, name);
	exit (1);
}

static void
freenl (nl, entries)
DIRENTRY	*nl[];
int	entries;
{
	int	i;

	for (i = 0; i < entries; i++)
		free ((char *) nl[i]);
	if (nl != (DIRENTRY **) 0)
		free ((char *) nl);
}

static int
scompar (sp1, sp2)
char **sp1, **sp2;
{

	return (strcmp (*sp1, *sp2));
}

/* print a list of files to standard output, by padding out all names to
 * the widest name plus 3 columns and calling print_er for each file name.
 */

static void
output_file (name, widest_file, nl, entries)
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
			len = strlen (nl[i]->d_name);
			if (len > widest_file)
				widest_file = len;
		}
		/* need spaces for intervening '/' and '\0' */
		bufsize = widest_file + strlen (name) + 2;
		if (bufsize > filebuflen) {
			filebuf = (filebuflen == 0) ?
			  malloc  (bufsize) :
			  realloc (filebuf, bufsize);
			if (filebuf == (char *) 0)
				mem_error (MSGSTR(LSPOLICY_4, "buffer allocation"));
			filebuflen = bufsize;
		}
	}

	if (entries == 0)
		print_er (name, name, widest_file + 3);
	else {
		for (i = 0; i < entries; i++) {
			strcpy (filebuf, name);
			if (name[strlen(name)-1] != '/')
				strcat (filebuf, "/");
			strcat (filebuf, nl[i]->d_name);
			print_er (filebuf, nl[i]->d_name, widest_file + 3);
		}
	}
	return;
}
#endif /* SEC_ARCH */
