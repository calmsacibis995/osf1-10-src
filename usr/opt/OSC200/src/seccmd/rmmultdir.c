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
static char	*sccsid = "@(#)$RCSfile: rmmultdir.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 15:06:51 $";
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
/*
 * Copyright (c) 1989-90, SecureWare, Inc.
 *   All rights reserved.
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "rmmultdir_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_RMMULTDIR,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif
static int remove_mult(char *);


#if SEC_MAC /*{*/

/*
 * This program turns a multilevel directory back into a regular directory.
 * The directory is totally removed if the -r flag is used.
 */

#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static int total_errors = 0;

extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */
extern int opterr;

extern int errno;
extern int sec_errno;
extern char *sys_errlist[];


int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int file;
	register int mode;
	int option;
	int error = 0;
	int remove = 0;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_RMMULTDIR,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("multileveldir")) {
		fprintf(stderr, MSGSTR(RMMULTDIR_5,
			"%s: need multileveldir authorization\n"),
		       command_name);
		exit(1);
	}

	opterr = 0;
	while ((option = getopt(argc, argv, "r")) != EOF)
		switch (option) {
		case 'r':
			remove++;
			break;

		case '?':
			error = 1;
			break;
	}

	if (error || (argc < optind + 1)) {
		fprintf(stderr, MSGSTR(RMMULTDIR_1,
			"usage:  %s [-r] dir ...\n"), command_name);
		fprintf(stderr, MSGSTR(RMMULTDIR_2,
			"Where -r means totally remove\n"));
		fprintf(stderr, MSGSTR(RMMULTDIR_3,
			"      (no -r leaves a regular directory)\n"));
		fprintf(stderr, MSGSTR(RMMULTDIR_4,
			"      dir is a directory to change/remove\n"));
		exit(2);
	}

	/*
	 * Raise the privileges needed to make the directory not multilevel.
	 * On systems with SEC_ILB, also turn on ilnofloat in case we are
	 * invoked with the -r flag.
	 * Also raise any access control override privileges for which
	 * the user is authorized.
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
		fprintf(stderr, MSGSTR(RMMULTDIR_6,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);

	for (file = optind; file < argc; file++)  {
		if (remove)  {
			if (rmdir(argv[file]) != 0)  {
				total_errors++;
				fprintf(stderr, MSGSTR(RMMULTDIR_7,
					"%s: cannot remove "),
					command_name);
				perror(argv[file]);
			}
		} else
			(void) remove_mult(argv[file]);
	}

	exit(total_errors != 0);
}


/*
 * Turn a multilevel directory back into a regular directory.  Return 1 if the
 * operation is successful, 0 if not.
 */
remove_mult(file)
	char *file;
{
	int status;

	status = rmmultdir(file);
	if (status != 0)  {
		total_errors++;
		fprintf(stderr, MSGSTR(RMMULTDIR_8,
			"%s: cannot remove multilevel flag of "),
			command_name);
		psecerror(file);
	}

	return status == 0;
}

#endif /*} SEC_MAC */
