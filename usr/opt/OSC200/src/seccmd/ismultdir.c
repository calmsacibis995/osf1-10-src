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
static char	*sccsid = "@(#)$RCSfile: ismultdir.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:28:22 $";
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
 *	Copyright (c) 1989-90, SecureWare, Inc.
 *	All rights reserved.
 *
 *	ismultdir(1M)-this utility outputs the directory name along with an
 *	indicator of whether the directory is a multilevel directory or not.
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "ismultdir_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_ISMULTDIR,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_MAC /*{*/

#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/security.h>
#include <prot.h>

extern char *sys_errlist[];
extern int errno;

main(argc, argv)
int argc;
char *argv[];
{
	register int file;
	register int ret = 0;

	set_auth_parameters(argc, argv);
	initprivs();

	/*
	 * Must have at least two arguments.
	 */

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_ISMULTDIR,NL_CAT_LOCALE);
#endif

	if(argc < 2) {
		fprintf(stderr, MSGSTR(ISMULTDIR_1,
			"usage: %s dir ...\n"), command_name);
		exit(1);
	}

	/*
	 * Raise the privilege required to prevent diversion.
	 * Also raise the access control override privileges for
	 * which the user is authorized.
	 */

	if (forceprivs(privvec(SEC_MULTILEVELDIR, -1), (priv_t *) 0) ||
	    enableprivs(privvec(SEC_ALLOWDACACCESS, SEC_ALLOWMACACCESS,
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(ISMULTDIR_1,
			"%s: insufficient privileges\n"), command_name);
		exit(1);
	}

	/*
	 * For each directory, perform the system call.
	 */

	for (file = 1; file < argc; file++)  {
		ret = ismultdir(argv[file]);
		if (ret < 0)
			psecerror(argv[file]);
		else
			printf("%s\t%s\n", argv[file],
				ret ? MSGSTR(YES, "yes") : MSGSTR(NO, "no"));
	}

	exit(0);
}

#endif /*} SEC_MAC */
