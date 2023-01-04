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
static char *rcsid = "@(#)$RCSfile: privs.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/07 15:06:42 $";
#endif
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
 * This program restricts users privileges and sets up a shell or
 * command in which to run the context.
 */


/*
 * Based on:

 */

#include <sys/secdefines.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef MSG
#include "privs_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRIVS,n,s)
#else
#define MSGSTR(n,s) s
#endif

#if defined(KJI) || defined(NLS)
#include <NLchar.h>
#include <NLctype.h>
#endif

#if SEC_BASE /*{*/

#include <sys/types.h>
#include <stdio.h>
#include <pwd.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

static void reviseprivs();
static void do_command();

extern char *optarg;	/* arg pointer for getopt */
extern int optind;	/* option index for getopt */
extern int opterr;

int verbose = 0;

extern char *strrchr();
extern char *getenv();
extern char *privstostr();
extern priv_t *checksysauths();

main(argc, argv)
	int argc;
	char *argv[];
{
	register int option;
	register char *req_privs;
	register int mode = 0;
	int error = 0;
	char *command = (char *) 0;
	int kern_auth = 0;
	int base = 0;
	int eff = 0;
	privvec_t base_mask;
	privvec_t kern_auth_mask;
	privvec_t eff_mask;
	privvec_t priv_mask;

#ifdef NLS
        (void) setlocale( LC_ALL, "" );
#endif

#ifdef MSG
        catd = catopen(MF_PRIVS,NL_CAT_LOCALE);
#endif

	set_auth_parameters(argc, argv);
	initprivs();

	opterr = 0;
	while ((option = getopt(argc, argv, "a:c:i:r:vmkb")) != EOF)
		switch (option) {
		    case 'a':
		    case 'r':
		    case 'i':
			if (mode)
				goto usage;
			req_privs = optarg;
			mode = option;
			break;

		    case 'c':
			command = optarg;
			break;

		    case 'v':
			verbose = 1;
			break;

		    case 'm':
		    case 'k':
			kern_auth = 1;
			break;

		    case 'b':
			base = 1;
			break;

		    case '?':
			error = 1;
			break;
		}

	if (error || optind < argc) {
usage:
		fprintf(stderr, MSGSTR(PRIVS_1,
			"usage: %s [-v] [-k] [-b] [{-a|-r|-i} privs] [-c cmd]\n"),
			command_name);
		exit(1);
	}

	/*
	 * Determine a default privilege set to use.
	 */
	if (!base)
		kern_auth = 1;

	/*
	 * Obtain the current privilege settings from the kernel.
	 */
	if (getpriv(SEC_MAXIMUM_PRIV, kern_auth_mask) < 0) {
		fprintf(stderr, MSGSTR(PRIVS_3,
			"%s: cannot get kernel authorizations\n"),
			command_name);
		exit(1);
	}

	if (getpriv(SEC_BASE_PRIV, base_mask) < 0) {
		fprintf(stderr, MSGSTR(PRIVS_4,
			"%s: cannot get base privileges\n"),
			command_name);
		exit(1);
	}

	/*
	 * No action arguments.  Just print the current privileges.
	 */
	if (!mode) {
		if (kern_auth)  {
			req_privs = privstostr(kern_auth_mask, " ");
			printf(MSGSTR(PRIVS_6,
				"Kernel authorizations:\n    "));
			if (*req_privs == '\0')
				req_privs = "none";
			printbuf(req_privs, 4, " ");
		}
		if (base)  {
			req_privs = privstostr(base_mask, " ");
			printf(MSGSTR(PRIVS_7, "Base privileges:\n    "));
			if (*req_privs == '\0')
				req_privs = "none";
			printbuf(req_privs, 4, " ");
		}
		return 0;
	}

	if ((error = strtoprivs(req_privs, ",", priv_mask)) >= 0) {
		fprintf(stderr, MSGSTR(PRIVS_10,
			"Privilege specification invalid: %s\n"),
			&req_privs[error]);
		exit(1);
	}
			
	if (kern_auth)
		reviseprivs(SEC_MAXIMUM_PRIV, MSGSTR(PRIVS_11,
			"kernel authorizations"),
			kern_auth_mask, priv_mask, mode);
	if (base)
		reviseprivs(SEC_BASE_PRIV, MSGSTR(PRIVS_12,
			"base privileges"),
			base_mask, priv_mask, mode);

	do_command(command);

	/*NOTREACHED*/
}

/*
 * Modify the appropriate privilege set accoring to the option
 * chosen.
 */
static void
reviseprivs(privtype, name, curr, cmdline_mask, mode)
	int privtype;
	char *name;
	priv_t *curr;
	priv_t *cmdline_mask;
	int mode;
{
	register int i;
	register int incr = 0;
	privvec_t new_mask;
	priv_t *missing;
	char *str;

	switch (mode) {
	    case 'a':	/* absolute */
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			new_mask[i] = cmdline_mask[i];
			if (cmdline_mask[i] & ~curr[i])
				incr = 1;
		}
		break;
	    case 'i':	/* increase */
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i) {
			new_mask[i] = curr[i] | cmdline_mask[i];
			if (cmdline_mask[i])
				incr = 1;
		}
		break;
	    case 'r':	/* decrease */
		for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
			new_mask[i] = curr[i] & ~cmdline_mask[i];
	}

	if (incr && (missing = checksysauths(cmdline_mask))) {
		fprintf(stderr, MSGSTR(PRIVS_2,	"%s: not authorized for %s\n"),
			command_name, privstostr(missing, " "));
		exit(1);
	}

	/*
	 * If privileges are being increased, print a message on the tty
	 * and force the user to confirm.  This is to prevent a trojan
	 * horse program from acquiring base privileges for which the
	 * user is authorized.
	 */
	if (incr)
		confirm();

	switch (privtype) {
	    case SEC_BASE_PRIV:
		i = setbaseprivs(new_mask);
		break;
	    case SEC_MAXIMUM_PRIV:
		i = setsysauths(new_mask);
		break;
	}

	if (i < 0) {
		fprintf(stderr, MSGSTR(PRIVS_18,
			"%s: cannot set new %s\n"), command_name, name);
		exit(1);
	}

	if (verbose)  {
		printf(MSGSTR(PRIVS_19, "New %s:\n    "), name);
		str = privstostr(new_mask, " ");
		if (*str == '\0')
			str = "none";
		printbuf(str, 4, " ");
	}
}


/*
 * Replace this program with the user's shell, as found in /etc/passwd.
 * If a command were specified on the command line, supply that as
 * an argument.  The shell should understand the '-c <command>' syntax,
 * because those are the arguments passed when the -c optin to privs
 * is used.
 */
static void
do_command(cmd)
	char *cmd;
{
	register struct passwd *p;
	register char *name;
	char *shell;

	/*
	 * Check first for SHELL environment variable, then in /etc/passwd
	 * entry; otherwise, use /bin/sh.
	 */

	if ((shell = getenv("SHELL")) == (char *) 0) {
		if ((p = getpwuid(getuid())) == (struct passwd *) 0)
			shell = "/sbin/sh";
		else
			shell = p->pw_shell;
	}

	if (shell == (char *) 0 || shell[0] == '\0')
		shell = "/sbin/sh";

	/*
	 * Compute argv[0].
	 */
	name = strrchr(shell, '/');
	if (name == (char *) 0)
		name = shell;
	else
		name++;

	if (cmd == (char *) 0)
		(void) execl(shell, name, (char *) 0);
	else
		(void) execl(shell, name, "-c", cmd, (char *) 0);

	fprintf(stderr, MSGSTR(PRIVS_20, "%s: No shell.\n"), command_name);
	exit(1);
}

/*
 * Request confirmation before increasing base privileges
 */
confirm()
{
	int	cnt, ttyf = open("/dev/tty", 2);
	char	buf[512];

	if (ttyf < 0) {
		fprintf(stderr, MSGSTR(PRIVS_5,
			"%s: cannot confirm increased privileges\n"),
			command_name);
		exit(1);
	}
	sprintf(buf, MSGSTR(PRIVS_8,
		"%s: Type Yes to raise base privileges, anything else to abort: "),
		command_name);
	write(ttyf, buf, strlen(buf));
	cnt = read(ttyf, buf, sizeof buf);
	if (cnt > 0 && buf[--cnt] == '\n')
		buf[cnt] = '\0';
	if (cnt <= 0 || strcmp(buf, MSGSTR(YES, "Yes")) != 0) {
		fprintf(stderr, MSGSTR(PRIVS_9, "%s: command aborted\n"),
			command_name);
		exit(1);
	}
	close(ttyf);
}
#endif /* SEC_BASE */
