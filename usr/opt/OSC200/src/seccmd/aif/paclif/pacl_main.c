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
static char *rcsid = "@(#)$RCSfile: pacl_main.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/10/07 15:23:25 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	pacl_main.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:04:53  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:39  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:05  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:01  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 *   Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */

/* #ident "@(#)pacl_main.c	1.1 11:17:34 11/8/91 SecureWare" */

/*
 * NOTE - if program hangs for any reason, kill -HUP pid will
 *        kill it, leaving the tty in its original mode.
 */


/* main routine that drives the traversal of menu screens.
 * The menus are linked together via the menu_scrn structure.
 * For each menu, there is a table that describes what happens
 * when each item on the menu is chosen.
 * That action can be to run a program, to call a routine, or to
 * popup a menu.  For menuprompt screens, the fields that are
 * on the same line as the menu choice are passed to the program
 * as arguments.
 *
 * Arguments:
 *  -p  if compiled with -DPRINTSCR, will print screens to stdout.
 *  -h  will print a list of help screens for all fields in all
 *      screens. Argument is name of directory to search for
 *      help.
 *  -t  turn off time and date reporting (useful for testing)
 *  -l  logging level
 *  -S  normal signal-handling (don't catch them)
 *
 * The program now runs in totally raw mode; all control keys are
 * passed through to the program. See signals.c for signal processing.
 */

#define MAIN

#include <stdio.h>
#include <sys/secdefines.h>
#include <pwd.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"
#include <locale.h>

#ifdef PRINTSCR
static char ARGS[] = "pSth:l:";
#else
static char ARGS[] = "Sth:l:";
#endif

/* Definitions */
#ifdef OSF
#define MESSAGE_FILE	"/usr/share/lib/sechelp/paclif/paclif.msg"
#else
#define MESSAGE_FILE	"/etc/menuhelp/paclif/paclif.msg"
#endif

extern void InitializeMessageHandling();

extern Scrn_parms *TopScrn;

extern void hup_catch();
extern void time_catch();

extern Scrn_hdrs acc_hdrs, pacl_hdrs;
extern char *gl_runner;

int	no_date_and_time = 0;

main (argc, argv)
int	argc;
char	*argv[];
{
	extern	int optind;
	extern	char *optarg;
	int ret;
	int c;
	int _NormalSigs = 1;
	char *program_name;
	char *runner;

	/* Open the Catalog */

        (void) setlocale(LC_ALL, "" );
        catd = catopen(MF_AIF, NL_CAT_LOCALE);

	OPENLOG();

	/* Initialize security */
	set_auth_parameters(argc, argv);

	/* deciding how we've been called */

	program_name = strrchr(argv[0], '/');
	if (program_name == NULL)
		program_name = argv[0];
	else
		program_name++;

	SWLOG(0, MSGSTR(AIF_MAIN_1, "Setting up privileges"), 
	      NULL, NULL, NULL);

	initprivs();
#if I_MUCKED_UP_PRIVS
	if (forceprivs(privvec(SEC_OWNER,
			SEC_CHOWN, SEC_EXECSUID,
			SEC_SUSPEND_AUDIT, SEC_WRITE_AUDIT,
/*			SEC_ALLOWDACACCESS, */
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
			-1), NULL)) {
		fprintf(stderr, MSGSTR(AIF_MAIN_2, "%s: insufficient privileges\n"), argv[0]);
		exit (1);
	}
#endif

	enablepriv (SEC_CHOWN);
	enablepriv (SEC_SETPROCIDENT);
	enablepriv (SEC_CHMODSUGID);
	enablepriv (SEC_ALLOWDACACCESS);
#if SEC_MAC
	enablepriv (SEC_ALLOWMACACCESS);
#endif
	enablepriv (SEC_OWNER);

	SWLOG(0, MSGSTR(AIF_MAIN_3, "Privileges set up"), NULL, NULL, NULL);

	/*
	 * Check user's authorizations and the program's name
	 */

	InitializeMessageHandling(MESSAGE_FILE);

	/* initialize curses */

	initcurses();

	/* initialize buffer variables used by the program */

	init_vars();

#if SEC_MAC
	/* initialize mandatory access control for mand_syslo and mand_syshi */

	mand_init();
#endif
	
	while ((c = getopt(argc, argv, ARGS)) != -1)
		switch (c) {
		case 'h':
			SWLOG(0, MSGSTR(AIF_MAIN_6, "Help screens print"), NULL, NULL, NULL);
			if (optarg[0] == '\0')
				print_help_scrns(NULL);
			else	print_help_scrns(optarg);
			exit (0);

#ifdef LOGGING
		case 'l':
			_SWlogging = 1;
			if (optarg[0] != '\0') {
				_SWloglevel = atoi(optarg);
				if (_SWloglevel < 0)
					_SWloglevel = 0;
				else if (_SWloglevel > MaxSWLogLevel)
					_SWloglevel = MaxSWLogLevel;
			}
			SWLOG(0, MSGSTR(AIF_MAIN_7, "logging level = <%d>"), _SWloglevel, 0, 0);
			break;
#endif

#ifdef PRINTSCR
		case 'p':
			SWLOG(0, MSGSTR(AIF_MAIN_8, "Screen printing only"), NULL, NULL, NULL);
			print_if_scrns();
			restorescreen();
			exit (0);
#endif /* PRINTSCR */

		case 'S':
			SWLOG(0, MSGSTR(AIF_MAIN_9, "Normal signal handling"), NULL, NULL, NULL);
			_NormalSigs = 0;
			break;

		case 't':		/* turn off the time and date */
			no_date_and_time = 1;
			break;

		default:
			SWLOG(0, MSGSTR(AIF_MAIN_10, "Bad argument switch: <%c>"), c, NULL, NULL);
			fprintf(stderr, MSGSTR(AIF_MAIN_11, "%s does not take any arguments.\n"),
			  argv[0]);
			restorescreen();
			exit (1);
		}

	/* set up TopScrn and runner */

	if (runner = pw_idtoname (getluid()))
		strcpy (gl_runner, runner);
	else
		strcpy (gl_runner, "Unknown");

	(void) signal(SIGHUP, hup_catch);

#ifndef NO_SIGS
	if (_NormalSigs) {
		(void) setup_signals();
		(void) signal(SIGALRM, time_catch);
		(unsigned) alarm((unsigned) UPD_SECS);
	}
#endif /* NO_SIGS */

	/* translate all the intitialized messages in global variables */
#if I_NEED_PAIN
	translate();
#endif

	/* walk the menus, restore the screen back to normal, and exit */
	ret = traverse(TopScrn, -1); 

	restorescreen();

	CLOSELOG();

	if (no_date_and_time) {
		fflush (stdout);
		sleep (3);
	}
	exit (0);
}
