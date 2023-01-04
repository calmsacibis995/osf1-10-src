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
static char	*sccsid = "@(#)$RCSfile: chpriv.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:34:59 $";
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
/* Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * Change the privilege sets on a file
 *   chpriv -p -g [ -a privs ] [ -r privs ] [ -i privs ] file . . .
 *
 *   -p		change the potential set
 *   -g		change the granted set
 *   -a		absolute -- privs that follow become the new set
 *   -r		remove   -- privs that follow are removed
 *   -i		increase -- privs that follow are added to the set
 *
 * If neither -p or -g are specified, both sets are changed.
 * The process must possess the new set of privileges on the file
 * in order to change it.
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
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include <stdio.h>
#include <sys/stat.h>

privvec_t iprivs;	/* -i argument */
privvec_t rprivs;	/* -r argument */
privvec_t aprivs;	/* -a argument */
int errflag=0, pflag=0, gflag=0, aflag=0, rflag=0, iflag=0;

extern char *privstostr();
extern priv_t *checksysauths(), *privvec();

extern int opterr;

main (argc, argv)
	int	argc;
	char	*argv[];
{
	int	c;
	extern	int optind;
	extern	char *optarg;
	int ret;


#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();

	/* make sure user is authorized to set file privileges */
	if (!authorized_user("chpriv")) {
		fprintf(stderr, MSGSTR(CHPRIV_1, "%s: need chpriv authorization\n"),
			command_name);
		exit(1);
	}

	if (forceprivs(privvec(SEC_CHPRIV, -1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(CHPRIV_2, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	opterr = 0;
	while ((c = getopt (argc, argv, "pga:r:i:")) != -1) {
		switch (c) {
		case '?':
			errflag++;

			break;
		case 'p':
			pflag++;
			break;
		case 'g':
			gflag++;
			break;
		case 'a':
			aflag++;
			if (rflag || iflag)
				errflag++;
			if (strtoprivs(optarg, " \t,", aprivs) >= 0) {
				fprintf(stderr,
					MSGSTR(CHPRIV_3, "%s: invalid privileges after -a\n"),
					command_name);
				exit(1);
			}
			break;
		case 'r':
			rflag++;
			if (aflag)
				errflag++;
			if (strtoprivs(optarg, " \t,", rprivs) >= 0) {
				fprintf(stderr,
					MSGSTR(CHPRIV_4, "%s: invalid privileges after -r\n"),
					command_name);
				exit(1);
			}
			break;
		case 'i':
			iflag++;
			if (aflag)
				errflag++;
			if (strtoprivs(optarg, " \t,", iprivs) >= 0) {
				fprintf(stderr,
					MSGSTR(CHPRIV_5, "%s: invalid privileges after -i\n"),
					command_name);
				exit(1);
			}
			break;
		}
	}

	if (errflag || optind >= argc ||
	    (aflag == 0 && rflag == 0 && iflag == 0)) {
		fprintf(stderr,
	MSGSTR(CHPRIV_6, "Usage: %s [ -g | -p ] [ -a privs | -r privs | -i privs ] file . . .\n"),
		  command_name);
		exit(1);
	}
	
	/* check that the privileges we're adding or specifying are within
	 * the maximum set of the process.
	 */
	if (check_privs())
		exit(1);
	
	for ( ; optind < argc; optind++)
		if (change_priv(argv[optind]) < 0)
			errflag++;
	exit(errflag);
}

/* Compute the new privileges on the file based on the current privileges
 * on the file, the maximum privileges on the process, and the changes
 * requested.  Returns 0 on success, non-zero on failure.
 */

check_privs()
{
	int i;
	int first;
	int errors;
	priv_t *missing;

	/*
	 * make sure process is authorized for privileges to be set
	 */

	if (aflag && (missing = checksysauths(aprivs)) ||
	    iflag && (missing = checksysauths(iprivs))) {
		fprintf(stderr, MSGSTR(CHPRIV_7, "Not authorized to set: %s\n"),
			privstostr(missing, " "));
		return 1;
	}

	return 0;
}

/* Change the privileges on a file.
 * If necessary, retrieve the existing privilege set(s) from the file.
 */

change_priv (filename)
char *filename;
{
	struct stat sb;
	privvec_t fpprivs;	/* potential set on file */
	privvec_t fgprivs;	/* granted set on file */
	privvec_t saveprivs;	/* saved privilege set before enable */
	int i;
	int error = -1;

	/*
	 * Raise any access control override privileges for which
	 * the user is authorized.
	 */
	if (enableprivs(privvec(SEC_OWNER, SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), saveprivs)) {
		fprintf(stderr, MSGSTR(CHPRIV_2, "%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	/*
	 * Make sure the target is a regular file
	 */
	if (stat (filename, &sb) < 0) {
		fprintf(stderr, MSGSTR(CHPRIV_8, "%s: unable to retrieve file status of %s\n"), 
			command_name, filename);
		goto out;
	}
	if ((sb.st_mode & S_IFMT) != S_IFREG) {
		fprintf(stderr, MSGSTR(CHPRIV_9, "%s: %s is not a regular file\n"),
			command_name, filename);
		goto out;
	}

	/*
	 * If adding or removing privileges, get the current sets
	 */
	if (iflag || rflag) {
		/* if not setting granted, are setting potential or both */
		if (gflag == 0 || pflag) {
			if (statpriv(filename, SEC_POTENTIAL_PRIV,
					fpprivs) < 0) {
				psecerror(filename);
				goto out;
			}
			if (iflag)
				for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
					fpprivs[i] |= iprivs[i];
			if (rflag)
				for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
					fpprivs[i] &= ~rprivs[i];
		}
		/* if not setting potential, are setting granted or both */
		if (pflag == 0 || gflag) {
			if (statpriv(filename, SEC_GRANTED_PRIV, fgprivs) < 0) {
				psecerror(filename);
				goto out;
			}
			if (iflag)
				for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
					fgprivs[i] |= iprivs[i];
			if (rflag)
				for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
					fgprivs[i] &= ~rprivs[i];
		}
	} else { /* must be aflag */
		for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
			fgprivs[i] = aprivs[i];
		for (i = 0; i < SEC_SPRIVVEC_SIZE; i++)
			fpprivs[i] = aprivs[i];
	}

	/*
	 * Drop the suspendaudit privilege to allow the chpriv
	 * syscalls to be audited.
	 */
	disablepriv(SEC_SUSPEND_AUDIT);

	if (gflag == 0 || pflag) {
		if (chpriv(filename, SEC_POTENTIAL_PRIV, fpprivs) < 0) {
			psecerror(filename);
			goto out;
		}
	}
	if (pflag == 0 || gflag) {
		if (chpriv(filename, SEC_GRANTED_PRIV, fgprivs) < 0) {
			psecerror(filename);
			goto out;
		}
	}
	error = 0;

out:
	seteffprivs(saveprivs, (priv_t *) 0);
	return error;
}
#endif /* SEC_PRIV */
