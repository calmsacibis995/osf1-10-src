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
static char *rcsid = "@(#)$RCSfile: getlevel.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/10/07 14:35:01 $";
#endif
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* Copyright (c) 1988 SecureWare, Inc.
 *   All rights reserved
 *
 * getlevel command for levels and clearance.
 *
 * getlevel [-c] [-s] [-a]
 *
 * Information label system (SEC_ILB) also allows -i option
 */

#ident "@(#)getlevel.c	5.2 09:50:28 8/30/90 SecureWare"
/*
 * Based on:
 *   "@(#)getlevel.c	10.1.1.4 15:21:22 8/8/90 SecureWare"
 */

#include <sys/secdefines.h>



#if SEC_MAC /*{*/

#include <sys/types.h>
#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <stdio.h>
#include <sys/errno.h>

#include <locale.h>
#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)

#if SEC_ILB

#define	optstr	"csia"
#define	usage	MSGSTR(GETLEVEL_1, "Usage: %s [-c] [-s] [-i] [-a]\n")
#define	IR_TYPE	ilb_ir_t
#define	ALLOC_IR	ilb_alloc_ir
#define	SL_CONVERT	convert_sl_ir

extern char *convert_sl_ir();

#else /* SEC_ILB */

#define	optstr	"csa"
#define	usage	MSGSTR(GETLEVEL_2, "Usage: %s [-c] [-s] [-a]\n")
#define	IR_TYPE	mand_ir_t
#define	ALLOC_IR	mand_alloc_ir
#define	SL_CONVERT	convert_ir

#endif /* SEC_ILB */

#if SEC_ENCODINGS
extern char *convert_cl_ir();
#endif

#ifdef SEC_ACL_POSIX
/* Indicate to filetobuf that it is dealing with labels. */

int acl_or_label = 0;
#endif

extern char *convert_ir();

main (argc, argv)
int argc;
char *argv[];
{
	extern int optind;
	extern char *optarg;
	IR_TYPE *ir;
	int c;
	char *sl;
	char *cl;
#if SEC_ILB
	char *il;
	char iflag = 0;
#endif
	char errflag = 0, cflag = 0, sflag = 0;
	int ret;


        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);

	set_auth_parameters(argc, argv);
	initprivs();
	if (mand_init())
	        exit(1);

	while ((c = getopt (argc, argv, optstr)) != -1)
		switch (c) {
		case '?':
			errflag = 1;
			break;
		case 'a':
			cflag = sflag = 1;
#ifdef SEC_ILB
			iflag = 1;
#endif
			break;
		case 'c':
			cflag = 1;
			break;
		case 's':
			sflag = 1;
			break;
#if SEC_ILB
		case 'i':
			iflag = 1;
			break;
#endif
		}
	if (errflag) {
		fprintf (stderr, usage, command_name);
		exit (1);
	}

#if SEC_ILB
	if (!cflag && !sflag && !iflag)
#else
	if (!cflag && !sflag)
#endif
		sflag = 1;

	ir = ALLOC_IR();
	if (ir == (IR_TYPE *) 0) {
		fprintf (stderr, MSGSTR(GETLEVEL_3, "%s: can't allocate label buffer\n"),
			command_name);
		exit (1);
	}

	if (cflag) {
		ret = getclrnce (ir);
		if (ret < 0) {
			fprintf (stderr, MSGSTR(GETLEVEL_4, "Process clearance not set yet.\n"));
			exit (1);
		}
#if SEC_ENCODINGS
		cl = convert_cl_ir (ir, 0);
#else
		cl = SL_CONVERT (ir, 0);
#endif
		if (cl == (char *) 0) {
			fprintf (stderr, MSGSTR(GETLEVEL_5, "%s: cannot convert clearance\n"),
				command_name);
			exit (1);
		}

		printf (MSGSTR(GETLEVEL_6, "Process clearance: %s\n"), cl);
	}

	if (sflag) {
		ret = getslabel (ir);
		if (ret < 0) {
			fprintf (stderr,
				MSGSTR(GETLEVEL_7, "Process sensitivity level not set yet.\n"));
			exit (1);
		}
		/* Convert to external representation */
		sl = SL_CONVERT (ir, 0);
		if (sl == (char *) 0) {
			fprintf (stderr,
			  MSGSTR(GETLEVEL_8, "%s: cannot convert sensitivity label\n"),
			    command_name);
			exit (1);
		}
		printf (MSGSTR(GETLEVEL_9, "Process sensitivity level: %s\n"), sl);
	}

#if SEC_ILB
	if (iflag) {
		ret = getilabel (ir);
		if (ret != -1) {
			il = convert_ir (ir, 0);
			if (il == (char *) 0) {
				fprintf(stderr,
					MSGSTR(GETLEVEL_10, "%s: can't convert information label\n"),
					command_name);
				exit(1);
			}
		}
		fprintf (stderr, MSGSTR(GETLEVEL_11, "Process information level: %s\n"),
			ret == -1 ? "WILDCARD" : il);
	}
#endif
	
	exit (0);
}

#endif /*} SEC_MAC */
