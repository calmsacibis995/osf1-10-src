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
static char	*sccsid = "@(#)$RCSfile: setlevel.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:35:23 $";
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
 *   All rights reserved
 *
 * setlevel command for sensitivity, information and clearance levels.
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


#if SEC_MAC || SEC_ILB

#include <sys/types.h>
#include <stdio.h>
#include <sys/security.h>
#include <mandatory.h>
#include <prot.h>
#include <sys/errno.h>
#include <pwd.h>

extern struct passwd *getpwuid();
extern char *convert_er();
extern priv_t *privvec();

#if SEC_ILB

#define	usage	MSGSTR(SETLEVEL_1, "Usage: %s [-s sens-label] [-c clearance] [ -i info-label]\n")
#define	optstr	"c:s:i:"
#define	SL_CONVERT	convert_sl_er

extern char *convert_sl_er();

#else

#define	usage	MSGSTR(SETLEVEL_2, "Usage: %s [-s sens-label] [-c clearance]\n")
#define	optstr	"c:s:"
#define	SL_CONVERT	convert_er

#endif

#if SEC_ENCODINGS
extern char *convert_cl_er();
#endif

main (argc, argv)
int argc;
char *argv[];
{
	extern int optind, opterr;
	extern char *optarg;
	mand_ir_t *sl_ir;
	mand_ir_t *cl_ir;
	privvec_t saveprivs;
	int c, size;
	char *sl = (char *) 0;
	char *cl = (char *) 0;
	char errflag = 0, cflag = 0, sflag = 0, noflag = 1;
#if SEC_ILB
	ilb_ir_t *il_ir;
	char *il = (char *) 0;
	char iflag = 0;
#endif
	int ret;
	struct passwd *p;
#if SEC_ENCODINGS
	int rel;
#endif


#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();
#if SEC_MAC
	mand_init();
#endif

	opterr = 0;
	while ((c = getopt (argc, argv, optstr)) != -1)
		switch (c) {
		case '?':
			errflag++;
			break;
		case 'c':
			cflag++; noflag = 0;
			cl = optarg;
			break;
		case 's':
			sflag++; noflag = 0;
			sl = optarg;
			break;
#if SEC_ILB
		case 'i':
			iflag++; noflag = 0;
			il = optarg;
			break;
#endif
		}

	if (errflag || noflag) {
		fprintf(stderr, usage, command_name);
		exit(1);
	}

	if (sflag) {
		sl_ir = (mand_ir_t *) SL_CONVERT (sl, &size);
		if (sl_ir == (mand_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(SETLEVEL_3, "%s: cannot convert sensitivity label\n"),
			  command_name);
			exit (1);
		}
#if SEC_ENCODINGS
		/* Ensure that the requested sensitivity level dominates the
		 * system minimum.  Use the system minimum if not. */
		if (forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs)==0) {
			rel = mand_ir_relationship(sl_ir, mand_minsl);
			if ((rel & (MAND_EQUAL | MAND_SDOM)) == 0)
				sl_ir = mand_minsl;
			seteffprivs(saveprivs, (priv_t *) 0);
		}
#endif
	}
	if (cflag) {
#if SEC_ENCODINGS
		cl_ir = (mand_ir_t *) convert_cl_er (cl, &size);
#else
		cl_ir = (mand_ir_t *) SL_CONVERT (cl, &size);
#endif
		if (cl_ir == (mand_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(SETLEVEL_4, "%s: cannot convert clearance label\n"),
			  command_name);
			exit (1);
		}
#if SEC_ENCODINGS
		/* Ensure that the requested clearance dominates the system
		 * minimum.  Use the system minimum if it doesn't.  */
		if (forceprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs)==0) {
			rel = mand_ir_relationship(cl_ir, mand_minclrnce);
			if ((rel & (MAND_EQUAL | MAND_SDOM)) == 0)
				cl_ir = mand_minclrnce;
			seteffprivs(saveprivs, (priv_t *) 0);
		}
#endif
	}

#if SEC_ILB
	if (iflag) {
		il_ir = (ilb_ir_t *) convert_er (il, &size);
		if (il_ir == (ilb_ir_t *) 0) {
			fprintf (stderr,
			  MSGSTR(SETLEVEL_5, "%s: cannot convert information label\n"),
			  command_name);
			exit (1);
		}
	}
#endif

	if (cflag || sflag) {
		if (enableprivs(privvec(SEC_ALLOWMACACCESS, -1), saveprivs)) {
			fprintf(stderr, MSGSTR(SETLEVEL_6, "%s: insufficient privileges\n"),
				command_name);
			exit(1);
		}

		if (cflag) {
			ret = setclrnce(cl_ir);
			if (ret < 0) {
				fprintf(stderr,
					MSGSTR(SETLEVEL_7, "Cannot set process clearance.\n"));
				exit(1);
			}
		}

		if (sflag) {
			ret = setslabel(sl_ir);
			if (ret < 0) {
				fprintf(stderr,
					MSGSTR(SETLEVEL_8, "Cannot set process sensitivity label.\n"));
				exit(1);
			}
		}

		seteffprivs(saveprivs, (priv_t *) 0);
	}

#if SEC_ILB
	if (iflag) {
		if (enableprivs(privvec(SEC_ALLOWILBACCESS, -1), saveprivs)) {
			fprintf(stderr, MSGSTR(SETLEVEL_6, "%s: insufficient privileges\n"),
				command_name);
			exit(1);
		}
		ret = setilabel(il_ir);
		if (ret < 0) {
			fprintf(stderr,
				MSGSTR(SETLEVEL_9, "Cannot set process information label.\n"));
			exit (1);
		}
		seteffprivs(saveprivs, (priv_t *) 0);
	}
#endif

	/* execute the user's shell or the command given */

	if (optind == argc) {
		p = getpwuid (geteuid());
		execl (p->pw_shell, strrchr (p->pw_shell,'/') + 1, 0);
		printf (MSGSTR(SETLEVEL_10, "exec of %s failed\n"), p->pw_shell);
	} else {
		execvp (argv[optind], &argv[optind]);
		printf (MSGSTR(SETLEVEL_10, "exec of %s failed\n"), argv[optind]);
	}
	exit (1);
}
#endif
