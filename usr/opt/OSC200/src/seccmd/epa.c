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
static char	*sccsid = "@(#)$RCSfile: epa.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/07 14:28:06 $";
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
 * This program sets up environment control for primordial processes, i.e.,
 * those processes stemming from inittab or rc.  The IDs, security levels
 * and privileges can each or all be crafted and a program run from it.
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "epa_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_EPA,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_BASE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <ctype.h>
#include <pwd.h>
#include <grp.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>


#define	LOGIN_TYPE	"INITSESSION"
#define	SHELL		"/sbin/sh"



static struct pr_passwd *pr;
static struct passwd p;

static int has_real_uid = 0;
static int has_login_uid = 0;
static int has_real_gid = 0;
#if SEC_MAC
static int has_clearance = 0;
static int has_sec_level = 0;
#endif
#if SEC_NCAV
static int has_caveats = 0;
#endif
static int has_auths = 0;
static int has_privs = 0;

static int real_uid = 0;
static int login_uid = 0;
static int real_gid = 0;
#if SEC_MAC
static mand_ir_t *clearance = (mand_ir_t *) 0;
static mand_ir_t *sec_level = (mand_ir_t *) 0;
#endif
#if SEC_NCAV
static ncav_ir_t *ncav_ir = (ncav_ir_t *) 0;
#endif
static privvec_t auths;
static privvec_t privs;
static char *envp[] = {
		"PATH=/tcb/bin:/sbin:/usr/sbin",
		"HOME=/",
		"IFS= \t",
		"PS1=$",
		"PS2=>",
		"SHELL=/sbin/sh",
		(char *) 0 };


extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */


static int uid();
static int gid();
static void leave();
#if SEC_MAC
static mand_ir_t *well_known_er();
#endif


extern char *strchr();
extern struct passwd *getpwnam();
extern struct group *getgrnam();
extern void endpwent();
extern void endgrent();

#define MAXOPTS "u:l:g:c:s:a:p:v:"
#define GENOPTS "u:l:g:a:p:"
#if SEC_MAC
#define MACOPTS "c:s:"
#endif
#if SEC_NCAV
#define NCAVOPTS "v:"
#endif


int
main(argc, argv)
	int argc;
	char *argv[];
{
	int option;
	int err_code;
	int error = 0;
	int verbose = 0;
	char cmd[NCARGS];
	char optstring[sizeof(MAXOPTS)];
#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_EPA,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
#if SEC_MAC
	mand_init();
#endif

	strcat(optstring, GENOPTS);
#if SEC_NCAV
	strcat(optstring, NCAVOPTS);
#endif
#if SEC_MAC
	strcat(optstring, MACOPTS);
#endif

	while ((option = getopt(argc, argv, optstring)) != EOF)
		switch (option)  {
		case 'u':
			has_real_uid++;
			real_uid = uid(optarg);
			break;

		case 'l':
			has_login_uid++;
			login_uid = uid(optarg);
			break;

		case 'g':
			has_real_gid++;
			real_gid = gid(optarg);
			break;

#if SEC_MAC
		case 'c':
			has_clearance++;
			clearance = well_known_er(optarg);
			if (clearance == (mand_ir_t *) 0)
				clearance = mand_er_to_ir(optarg);
			if (clearance == (mand_ir_t *) 0)
				leave(MSGSTR(EPA_1, "Cannot locate clearance '%s'\n"), optarg);
			break;

		case 's':
			has_sec_level++;
			sec_level = well_known_er(optarg);
			if (sec_level == (mand_ir_t *) 0)
				sec_level = mand_er_to_ir(optarg);
			if (sec_level == (mand_ir_t *) 0)
				leave(MSGSTR(EPA_2, "Cannot locate sec level '%s'\n"), optarg);
			break;
#endif

#if SEC_NCAV
		case 'v':
			has_caveats++;
			ncav_ir = ncav_er_to_ir(optarg);
			if (ncav_ir == (ncav_ir_t *) 0)
				leave(MSGSTR(EPA_3, "Cannot locate caveat set '%s'\n"),optarg);
			break;
#endif

		case 'a':
			has_auths++;
			if (strtoprivs(optarg, ",", auths) != -1)
				leave(MSGSTR(EPA_4, "Unknown kernel authorization\n"));
			break;

		case 'p':
			has_privs++;
			if (strtoprivs(optarg, ",", privs) != -1)
				leave(MSGSTR(EPA_5, "Unknown base privilege\n"));
			break;

		case '?':
			error = 1;
			break;
	}

	if (error || (argc != optind + 1)) {
		fprintf(stderr,
		  MSGSTR(EPA_6, "usage:  %s [-u RUID] [-l LUID] [-g RGID] "),
		   command_name);
		fprintf(stderr, MSGSTR(EPA_7, "[-a auth] [-p base_privs] "));

#if SEC_MAC
		fprintf(stderr, MSGSTR(EPA_9,
			"[-c clearance] [-s sens_level] "));
#endif
#if SEC_NCAV
		fprintf(stderr, MSGSTR(EPA_10, "[-v caveats] "));
#endif
		fprintf(stderr, MSGSTR(EPA_11, "command\n"));
		fprintf(stderr, MSGSTR(EPA_12, "Where RUID is the real user ID\n"));
		fprintf(stderr, MSGSTR(EPA_13, "      LUID is the login user ID\n"));
		fprintf(stderr, MSGSTR(EPA_14, "      RGID is the real group ID\n"));
		fprintf(stderr, MSGSTR(EPA_15,
			"      auth is the set of kernel authorizations\n"));
		fprintf(stderr, MSGSTR(EPA_16,
			"      privs is the set of base privileges\n"));
#if SEC_MAC
		fprintf(stderr, MSGSTR(EPA_17,
			"      clearance is the user's clearance\n"));
		fprintf(stderr, MSGSTR(EPA_18,
			"      sens_level is the process sensitivity level\n"));
#endif
#if SEC_NCAV
		fprintf(stderr, MSGSTR(EPA_19,
			"      caveats is the process caveat set\n"));
#endif
		exit(1);
	}


	/*
	 * The order of operations is important, since some sets of
	 * operations are related.  For instance, the login UID must be
	 * set prior to any other identity change, the clearance
	 * must be set prior to the security level, and privilege
	 * restrictions should occur last (the other operations are
	 * privileged).
	 */

	if (has_login_uid && setluid(login_uid) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_21, "Cannot set login UID"));
	}

	if (has_real_gid && setgid(real_gid) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		perror(MSGSTR(EPA_22, "Cannot set real GID"));
	}

	if (has_real_uid && setuid(real_uid) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		perror(MSGSTR(EPA_23, "Cannot set real UID"));
	}

#if SEC_MAC
	if (has_clearance && setclrnce(clearance) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_24, "Cannot set clearance"));
	}

	if (has_sec_level && setslabel(sec_level) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_25, "Cannot set sensitivity level"));
	}
#endif

#if SEC_NCAV
	if (has_caveats && setncav(ncav_ir) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_26, "Cannot establish caveat set"));
	}
#endif

#if SEC_ILB
	if (has_clearance && has_sec_level && setilabel(mand_syslo) != 0) {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_27, "Cannot set information label"));
	}
#endif

	if (has_auths && setpriv(SEC_MAXIMUM_PRIV, auths) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_28, "Cannot set kernel authorizations"));
	}

	if (has_privs && setpriv(SEC_BASE_PRIV, privs) != 0)  {
		err_code = errno;
		fprintf(stderr, MSGSTR(EPA_20, "%s: Warning: "), command_name);
		errno = err_code;
		psecerror(MSGSTR(EPA_29, "Cannot set base privileges"));
	}


#if SEC_MAC
	/*
	 * Prior to executing the command, close all extra open files.
	 */
	mand_end();
#endif

	/*
	 * For a single command, exec it to avoid a parent shell.  For
	 * multiple commands, we need the parent shell to coordinate
	 * activity.
	 */
	if (strchr(argv[optind], ';') == (char *) 0)
		strcpy(cmd, "exec ");
	else
		cmd[0] = '\0';
	strcat(cmd, argv[optind]);

	(void) execle(SHELL, "PRIMORD", "-c", cmd, (char *) 0, envp);
	leave(MSGSTR(EPA_30, "cannot execute %s\n"), SHELL);
}

#if SEC_MAC
/*
 * See if the passed string is one of the well-known
 * sensitivity label aliases: syslo or syshi
 */
static mand_ir_t *
well_known_er(string)
	char *string;
{
	register mand_ir_t	*ir = mand_alloc_ir();

	if (ir)
		if (strcmp(string, "syslo") == 0)
			mand_copy_ir(mand_syslo, ir);
		else if (strcmp(string, "syshi") == 0)
			mand_copy_ir(mand_syshi, ir);
		else {
			mand_free_ir(ir);
			ir = (mand_ir_t *) 0;
		}
	return ir;
}
#endif


/*
 * Take an argument and return the UID for the ASCII representation for
 * the UID (all digits) or the login name (starts with non-digit).
 */
static int
uid(string)
	char *string;
{
	int collect_num = 0;
	struct passwd *p;

	if (isascii(*string) && isdigit(*string))
		while (*string != '\0')  {
			if (!isascii(*string) || !isdigit(*string) ||
			    (*string > '9'))
				leave(MSGSTR(EPA_31,
					"UID contains non-digit `%c'\n"),
					*string);

			collect_num =
				(collect_num * 10) + (int) (*string - '0');
			string++;
		}
	else  {
		p = getpwnam(string);
		if (p == (struct passwd *) 0)
			leave(MSGSTR(EPA_32, "Unknown user `%s'\n"), string);
		collect_num = p->pw_uid;
		endpwent();
	}

	return collect_num;
}



/*
 * Take an argument and return the UID for the ASCII representation for
 * the GID (all digits) or the group name (starts with non-digit).
 */
static int
gid(string)
	char *string;
{
	int collect_num = 0;
	struct group *g;

	if (isascii(*string) && isdigit(*string))
		while (*string != '\0')  {
			if (!isascii(*string) || !isdigit(*string) ||
			    (*string > '9'))
				leave(MSGSTR(EPA_33,
					"GID contains non-digit `%c'\n"),
					*string);

			collect_num =
				(collect_num * 10) + (int) (*string - '0');
			string++;
		}
	else  {
		g = getgrnam(string);
		if (g == (struct group *) 0)
			leave(MSGSTR(EPA_34, "Unknown group `%s'\n"), string);
		collect_num = g->gr_gid;
		endgrent();
	}

	return collect_num;
}


/*
 * Error has occurred. Exit the program.
 */
static void
leave(fmt, a, b, c, d, e, f)
	char *fmt;
	int a, b, c, d, e, f;
{
	fflush(stdout);

	fprintf(stderr, fmt, a, b, c, d, e, f);
	fflush(stderr);
	exit(1);
}

#endif
