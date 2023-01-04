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
static char *rcsid = "@(#)$RCSfile: mkmultdir.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/07 14:29:45 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "mkmultdir_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MKMULTDIR,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_MAC /*{*/
/*
 * This program turns an existing directory into a multilevel directory.
 * The directory is created if the -c flag is used.  The -m flag will
 * set the mode explicitly.
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static int total_errors = 0;

static int oct_num();
static int make_dir();
static int make_mult();


extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */
extern int opterr;

extern int errno;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int file;
	register int mode;
	int option;
	int error = 0;
	int create = 0;
	int mode_set = 0;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_MKMULTDIR,NL_CAT_LOCALE);
#endif

	mode = (0777 & ~umask(~0777));

	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("multileveldir")) {
		fprintf(stderr, MSGSTR(MKMULTDIR_5,
			"%s need the multileveldir authorization\n"),
			command_name);
		exit(1);
	}

	/*
	 * Alter the umask set by set_auth_parameters to one usable
	 * by the program.
	 */
	(void) umask(~mode);

	opterr = 0;
	while ((option = getopt(argc, argv, "cm:")) != EOF)
		switch (option) {
		case 'c':
			create++;
			break;

		case 'm':
			mode_set++;
			mode = oct_num(optarg);
			break;

		case '?':
			error = 1;
			break;
	}

	if (error || (argc < optind + 1)) {
		fprintf(stderr, MSGSTR(MKMULTDIR_1,
			"usage:  %s [-c] [-m fmode] dir ...\n"),
			command_name);
		fprintf(stderr, MSGSTR(MKMULTDIR_2,
			"Where -c is create directory if needed\n"));
		fprintf(stderr, MSGSTR(MKMULTDIR_3,
			"      -m is set directory mode to fmode\n"));
		fprintf(stderr, MSGSTR(MKMULTDIR_4,
			"      dir is a directory to change/create\n"));
		exit(2);
	}

	/*
	 * Raise privilege needed to make directory multilevel.
	 * On SEC_ILB systems, also raise ilnofloat in case we are
	 * invoked to create a new directory.
	 * Also raise access control override privileges for which
	 * user is authorized.
	 */
	if (forceprivs(privvec(SEC_MULTILEVELDIR,
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
				-1), (priv_t *) 0) ||
	    enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
				SEC_ALLOWMACACCESS,
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(MKMULTDIR_6,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

	for (file = optind; file < argc; file++)  {
		if (create)  {
			if (make_dir(argv[file], mode))
				(void) make_mult(argv[file], 1, mode);
		}
		else
			(void) make_mult(argv[file], mode_set, mode);
	}

	return total_errors != 0;
}


/*
 * Take a string of octal digits and return the octal number.
 */
static int
oct_num(string)
	char *string;
{
	int collect_num = 0;

	while (*string != '\0')  {
		if (!isascii(*string) || !isdigit(*string) ||
		    (*string >= '8'))  {
			fprintf(stderr,
				MSGSTR(MKMULTDIR_7, "%s: mode contains non-octal digit '%c'\n"),
				command_name, *string);
			exit(2);
		}

		collect_num = (collect_num * 8) + (*string - '0');
		string++;
	}

	return collect_num;
}


/*
 * Make a new directory with given mode (allowing the kernel to use the
 * umask of the process).  Return a 1 if the directory can be made and
 * 0 if not.  Consider an existing directory a non-error so a string of
 * files under the -c option, some present and some not, will all work.
 */
static int
make_dir(file, mode)
	char *file;
	int mode;
{
	int status;

	/*
	 * The mode here is biased by the umask setting, but the chmod in
	 * make_mult will override the restriction.
	 */
	status = mkdir(file, mode);
	if ((status != 0) && (errno != EEXIST))  {
		total_errors++;
		fprintf(stderr, MSGSTR(MKMULTDIR_8,
			"%s: cannot create directory"), command_name);
		perror(file);
	}

	return (status == 0) || (errno == EEXIST);
}


/*
 * Turn a regular directory into a multilevel directory.  Return 1 if the
 * operation is successful, 0 if not (including slamming in the proper mode
 * for a new directory).
 */
static int
make_mult(file, use_mode, mode)
	char *file;
	int use_mode;
	int mode;
{
	int status;

	if (use_mode) {
		if (status = chmod(file, mode)) {
			total_errors++;
			fprintf(stderr, MSGSTR(MKMULTDIR_9,
				"%s: cannot change mode of "),
				command_name);
			perror(file);
		}
	} else
		status = 0;

	if (status == 0) {
		if (status = mkmultdir(file)) {
			total_errors++;
			fprintf(stderr, MSGSTR(MKMULTDIR_10,
				"%s: cannot set multilevel flag of "),
				command_name);
			psecerror(file);
		}
	}

	return status == 0;
}

#endif /*} SEC_MAC */
