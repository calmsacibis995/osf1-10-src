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
static char	*sccsid = "@(#)$RCSfile: aif_main.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:18 $";
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
 *   Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



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
 *  -p  if compiled with -DDEBUG, will print screens to stdout.
 *  -h  will print a list of help screens for all fields in all
 *      screens. Argument is name of directory to search for
 *      help.
 *  -l  logging level
 *  -S  normal signal-handling (don't catch them)
 *
 * The program now runs in totally raw mode; all control keys are
 * passed through to the program. See signals.c for signal processing.
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <grp.h>
#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "userif.h"
#include "kitch_sink.h"
#include "logging.h"
#define MAIN
#include "UIMain.h"
#undef MAIN

#if defined(DEBUG) || defined(PRINTSCR)

#include <stdio.h>
FILE *logfp;
#define	LOGFILE "logfile"
static char ARGS[] = "pSh:l:";
int scr_pag = 0;

#else /* DEBUG || PRINTSCR */

static char ARGS[] = "Sh:l:";

#endif /* DEBUG || PRINTSCR */


/* Definitions */
#define MESSAGE_FILE	"isso.msg"

extern void
InitializeMessageHandling(),
LoadMessage();

static char
**msg_error_1,
**msg_error_2,
**msg_main,
*msg_error_1_text,
*msg_error_2_text,
*msg_main_text;

static char message_file[64];

#define ISSO_PROGRAM "issoif"
#define SYSADMIN_PROGRAM "sysadmif"

extern Scrn_parms pm1_isso_scrn;
extern Scrn_parms pm1_sysadmin_scrn;
Scrn_parms *TopScrn;

extern void hup_catch();
extern void time_catch();

main (argc, argv)
int	argc;
char	*argv[];
{
	struct	group	*g, *getgrgid();
	extern	int optind;
	extern	char *optarg;
	int ret;
	int c;
	int _NormalSigs = 1;
	char *program_name;

	OPENLOG();

	/* Initialize security */
	set_auth_parameters(argc, argv);

	/* deciding how we've been called */

	program_name = strrchr(argv[0], '/');
	if (program_name == NULL)
		program_name = argv[0];
	else
		program_name++;

	SWLOG(0, "Setting up privileges", NULL, NULL, NULL);
	initprivs();
	if (forceprivs(privvec(SEC_OWNER,
			SEC_CHOWN, SEC_EXECSUID,
			SEC_SUSPEND_AUDIT, SEC_WRITE_AUDIT,
			SEC_CONFIG_AUDIT, SEC_ALLOWDACACCESS,
#if SEC_MAC
			SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
			SEC_ILNOFLOAT,
#endif
			-1), NULL)) {
		printf ("%s: insufficient privileges", argv[0]);
		exit (1);
	}
	SWLOG(0, "Privileges set up", NULL, NULL, NULL);


	while ((c = getopt(argc, argv, ARGS)) != -1)
		switch (c) {
		case 'h':
			SWLOG(0, "Help screens print", NULL, NULL, NULL);
			if (optarg[0] == '\0')
				print_help_scrns(NULL);
			else	print_help_scrns(optarg);
			exit (0);

		case 'l':
			_SWlogging = 1;
			if (optarg[0] != '\0') {
				_SWloglevel = atoi(optarg);
				if (_SWloglevel < 0)
					_SWloglevel = 0;
				else if (_SWloglevel > MaxSWLogLevel)
					_SWloglevel = MaxSWLogLevel;
			}
			SWLOG(0, "logging level = <%d>", _SWloglevel, 0, 0);
			break;

#if defined(DEBUG) || defined(PRINTSCR)
		case 'p':
			SWLOG(0, "Screen printing only", NULL, NULL, NULL);
			print_if_scrns();
			restorescreen();
			exit (0);
#endif /* DEBUG || PRINTSCR */
			
		case 'S':
			SWLOG(0, "Normal signal handling", NULL, NULL, NULL);
			_NormalSigs = 0;
			break;

		default:
			SWLOG(0, "Errorneous argv: <%s>", c, NULL, NULL);
			fprintf(stderr, "%s does not take any arguments.\n");
			exit (1);
		}


#ifdef DEBUG
	if (!logfp && (logfp = fopen(LOGFILE, "w")) == NULL) {
		fprintf(stderr, "Cannot open logfile \'%s\'\n", LOGFILE);
		exit (1);
	}
	fprintf(logfp, "Log file open\n");
	chmod(LOGFILE, 0666);
	setbuf(logfp, NULL);
#endif DEBUG

	/*
	 * Check user's authorizations and the program's name
	 */

	SWLOG(0, "Checking user auths", NULL, NULL, NULL);
	role_program = NULL;
	if (authorized_user("sysadmin")) {
		role_program = SYS_ADMIN;
		SWLOG(0, " Sysadmin", NULL, NULL, NULL);
	}
	if (authorized_user("isso")) {
		if (role_program == SYS_ADMIN) {
			role_program = ISSO_ADMIN;
			SWLOG(0, " ISSO & Sysadmin", NULL, NULL, NULL);
		} else {
			role_program = ISSO;
			SWLOG(0, " ISSO", NULL, NULL, NULL);

			/*
			 * If calling as ISSO only, need to be running issoif
			 */

			if (strcmp(program_name, ISSO_PROGRAM) != 0)
				role_program = NULL;
		}
	} else if (role_program == SYS_ADMIN) {

		/*
		 * If calling as SYS_ADMIN only, need to be running sysadmif
		 */

		if (strcmp(program_name, SYSADMIN_PROGRAM) != 0)
			role_program = NULL;
	}

	if (role_program == 0) {
		fprintf(stderr, "%s: need sysadmin and/or isso authorization\n",
			  command_name);
		exit(1);
	}

	/* Get this application's messages loaded and ready */
	sprintf (message_file, "%saif/%s", HelpDir, MESSAGE_FILE);
	InitializeMessageHandling(message_file);

	LoadMessage("msg_main", &msg_main, &msg_main_text);

	/* set up TopScrn and runner */

	init_vars();
	switch (role_program) {
	case ISSO:
		strcpy(gl_runner, sec_officer);
		TopScrn = &pm1_isso_scrn;
		break;
	case SYS_ADMIN:
		strcpy(gl_runner, sys_admin);
		TopScrn = &pm1_sysadmin_scrn;
		break;
	case ISSO_ADMIN:
		strcpy(gl_runner, hot_dog);
		TopScrn = &pm1_isso_scrn;
		break;
	}

	initcurses();
	(void) signal(SIGHUP, hup_catch);

#ifndef NO_SIGS
	if (_NormalSigs) {
		(void) setup_signals();
		(void) signal(SIGALRM, time_catch);
		(unsigned) alarm((unsigned) UPD_SECS);
	}
#endif /* NO_SIGS */

	/* walk the menus, restore the screen back to normal, and exit.
	 */

	ret = traverse(TopScrn, 1);  /* the first desc is a choice */

	restorescreen();

	CLOSELOG();

	exit (0);
}
