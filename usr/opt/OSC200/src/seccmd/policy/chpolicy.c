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

/* Copyright (c) 1988-90, SecureWare, Inc.
 *   All rights reserved.
 *
 * Change a security attribute on a file.
 *   chlevel [ -f file | "level" ] file . . .
 *
 *   -f means attribute itself is stored in a file
 * Note that this driver routine can be used with any of the conv_ files
 * to create a chxxxxx command for any security attribute.
 */

/* #ident "@(#)chpolicy.c	3.2 12:31:20 6/15/90 SecureWare" */
/*
 * Based on:
 *   "@(#)chpolicy.c	2.1 11:51:31 1/25/89"
 */

#include <stdio.h>
#include <sys/security.h>
#include <prot.h>
#include <locale.h>
#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#ifdef SEC_ACL_POSIX

/* Indicate to filetobuf that it is dealing with labels */

int acl_or_label = 0;
#endif

extern char *file_to_buf();
extern char *malloc, *realloc();
extern int change_attr();
extern char *convert_er();
extern char *convert_ir();

main (argc, argv)
int	argc;
char	*argv[];
{
	int	c;
	extern	int optind, opterr;
	extern	char *optarg;
	char *ir;
	int ret;
	int errflag = 0, fflag = 0;
	char *attr_buf;		/* holds security attribute */
	int size;

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);

	set_auth_parameters(argc, argv);
	initprivs();

	opterr = 0;
	while ((c = getopt (argc, argv, "f:")) != -1) {
		switch (c) {
		case '?':
			errflag++;
			break;
		case 'f':
			fflag++;
			attr_buf = file_to_buf (optarg);
			if (attr_buf == (char *) 0) {
				fprintf (stderr, MSGSTR(CHPOLICY_1,
				  "%s: contents of %s unusable\n"),
				  command_name, optarg);
				exit (1);
			}
		}
	}

	if (!fflag)
		attr_buf = argv[optind++];

	if (errflag || optind >= argc) {
		fprintf (stderr, MSGSTR(CHPOLICY_2,
			"Usage: %s [ -f file | \"level\" ] file . . .\n"),
		  command_name);
		exit (1);
	}

	ir = convert_er (attr_buf, &size);
	if (ir == NULL && size == 0) {
		fprintf(stderr, MSGSTR(CHPOLICY_3, "%s: \"%s\" is unrecognizable\n"),
			command_name, attr_buf);
		exit(1);
	}

	for ( ; optind < argc; optind++) {
		ret = change_attr (argv[optind], ir, size);
		if (ret < 0) {
			psecerror (argv[optind]);
			errflag++;
		}
	}
	exit (errflag);
}
