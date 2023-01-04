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
static char *rcsid = "@(#)$RCSfile: identity.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/04/01 20:23:45 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (c) 1988-90 SecureWare, Inc.  All rights reserved.
 */


/*
 * Based on:

 */

/*LINTLIBRARY*/

/*
 * This file contains a set of routines used to make programs
 * more secure.  Specifically, routines that handle identity
 * are found here and are intended to be used with
 * policies and designs in SecureWare's security products.
 */

#include <sys/secdefines.h>
#include "libsecurity.h"

/* #if SEC_BASE */ /*{*/

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>

#define	SINK_HOLE	"/dev/null"
#define	CNTL_TERM	"/dev/tty"


/*
 * Used to hold prior ID values to reset to after protected
 * devices are obtained.
 */
static uid_t this_process_luid = -1;
static uid_t this_process_ruid = -1;
static gid_t this_process_rgid = -1;
static uid_t this_process_euid = -1;
static gid_t this_process_egid = -1;

/*
 * Boolean to denote authentication is done (=1) or not yet
 * done (=0).  Initially, of course, it is not done.
 */
static int auth_done = 0;

/*
 * Boolean to denote privileged user (=1) or non-privileged
 * user (=0).  Until explicitly set elsewhere, we assume the
 * user is non-privileged.
 */
static int has_privilege = 0;
static char know_privilege = 0;

/*
 * The return value of the signal() system call differs between systems.
 */

#if defined(SYSV_3) || defined(_OSF_SOURCE)
typedef void ((*SIGNAL_T)());
#else
typedef int ((*SIGNAL_T)());
#endif

/*
 * Used to retain specific signals.
 * The ones we keep are the ones that can be generated externally
 * (through the keyboard or through fiddling with the path between
 * the terminal and the TCB).  We need to stop SIGSYS since a user
 * can use stopio to disable his terminal (which we may use as stderr)
 * which sends SIGSYS to the process.
 */
SIGNAL_T old_hup = SIG_IGN;
SIGNAL_T old_int = SIG_IGN;
SIGNAL_T old_quit = SIG_IGN;
SIGNAL_T old_sys = SIG_IGN;
SIGNAL_T old_alarm = SIG_IGN;

static int alarm_left = 0;
static int time_when_entered = (time_t) 0;


static int quiet = 0;


/*
 * Used to catch SIGSYS signal while we test for the presence of
 * the security() set of calls in the kernel.
 */
static void catch_no_sec_sys_calls();


char *command_name;
char *command_line;

#ifndef STANDALONE
extern uid_t getluid();
#endif


/*
 * This routine must be called absolutely first.  It saves the identity
 * parameters for the current process (effective/real/login UID/GIDs).
 * It also records the command name.
 */
void
set_auth_parameters(argc, argv)
	int argc;
	char *argv[];
{
	SIGNAL_T old_sys = SIG_IGN;
	struct stat stat_buf;
	int i;

	if (!auth_done)  {
		/*
		 * Note we have done the lookup so that we don't mistakenly
		 * reset values (especially after a setuid() or setgid()
		 * has been done.  Set it now to avoid recursive calls here.
		 */
		auth_done = 1;

		/*
		 * Make sure the program was called with the standard
		 * file descriptors (stdin, stdout, stderr) open.
		 * If they are not open, open /dev/null (or failing
		 * that, /dev/tty) in their place.  If the files cannot
		 * be open, stop immediately rather than running the program
		 * assuming operations on the files are working (or are
		 * going to the right place.
		 */
		if (fstat(fileno(stdin), &stat_buf) != 0)  {
			if ((freopen(SINK_HOLE, "r+", stdin) == (FILE *) 0) &&
			    (freopen(CNTL_TERM, "r+", stdin) == (FILE *) 0))
				exit(1);
		}
		if (fstat(fileno(stdout), &stat_buf) != 0)  {
			if ((freopen(SINK_HOLE, "r+", stdout) == (FILE *) 0) &&
			    (freopen(CNTL_TERM, "r+", stdout) == (FILE *) 0))
				exit(1);
		}
		if (fstat(fileno(stderr), &stat_buf) != 0)  {
			if ((freopen(SINK_HOLE, "r+", stderr) == (FILE *) 0) &&
			    (freopen(CNTL_TERM, "r+", stderr) == (FILE *) 0))
				exit(1);
		}

		/*
		 * Save command name passed here so that we can use it
		 * for auditing and won't need to pass it across every
		 * Authentication subroutine call.  First, we must guarantee
		 * that we are called with at least one argument.
		 */
		if (argc < 1)  {
			(void) fprintf(stderr,
			MSGSTR(IDENTITY_1, "Security command was invoked without ANY arguments\n"));
			(void) fflush(stderr);
			exit(0);
		}

		command_name = strrchr(argv[0], '/');
		if (command_name == (char *) 0)
			command_name = argv[0];
		else
			command_name++;

		/* 
		 * Save the command line passed here so that we can use it
		 * for auditing. This is required only in case of subsystem
		 * auditing.
		 */
		command_line = malloc(strlen(argv[0]) + 3);
        	strcpy(command_line, argv[0]);
        	for (i=1; i<argc; i++) {
                	strcat(command_line, " ");
                	if ( (command_line = 
			     realloc(command_line, 
				strlen(command_line) + strlen(argv[i]) + 3)) == NULL)
			   {
			      fprintf(stderr,MSGSTR(IDENTITY_4,"Realloc failed\n"));
			      exit(1);
			   }
                	strcat(command_line, argv[i]);
        	}

		/*
		 * Disable any pending alarms to this process.  Without
		 * this, we could be interrupted in the middle of a
		 * security transition.
		 */
		(void) alarm(0);

		/*
		 * Trap a bad system call before we try the code below.
		 */
		old_sys = signal(SIGSYS, catch_no_sec_sys_calls);

		/*
		 * Store the real UID and GID which we use later in figuring
		 * privilege, resetting any special privileges, and determining
		 * the identify of the process before outputting information.
		 *
		 * The getluid() will cause a SIGSYS if the security() system
		 * calls are not installed.  If not, we print a message rather
		 * than dying ungracefully.
		 */
#ifndef STANDALONE
		this_process_luid = getluid();
#else
		this_process_luid = getuid();
#endif
		this_process_ruid = getuid();
		this_process_rgid = getgid();
		this_process_euid = geteuid();
		this_process_egid = getegid();

		(void) signal(SIGSYS, old_sys);

		/*
		 * Set the umask to be restrictive in case it was opened
		 * up prior to the call here.
		 */
		(void) umask(~SEC_DEFAULT_MODE);
	}
}


/*
 * This routine is called first on every authentication library external
 * referenced routine.  It verifies that set_auth_parameters() has been
 * invoked.
 */
void
check_auth_parameters()
{
	if (!auth_done)  {
		(void) fflush(stdout);
		(void) fprintf(stderr,
			MSGSTR(IDENTITY_2, "Authentication database use not initialized first\n"));
		(void) fflush(stderr);
		exit(1);
	}
}


/*
 * Return 1 if the argument UID is the same as the LUID we obtained
 * for this process.  Return 0 otherwise.
 */
int
is_starting_luid(uid)
	uid_t uid;
{
	check_auth_parameters();

	return uid == this_process_luid;
}


/*
 * Return 1 if the argument UID is the same as the RUID we obtained
 * for this process.  Return 0 otherwise.
 */
int
is_starting_ruid(uid)
	uid_t uid;
{
	check_auth_parameters();

	return uid == this_process_ruid;
}


/*
 * Return 1 if the argument UID is the same as the EUID we obtained
 * for this process.  Return 0 otherwise.
 */
int
is_starting_euid(uid)
	uid_t uid;
{
	check_auth_parameters();

	return uid == this_process_euid;
}

/*
 * Return 1 if the argument GID is the same as the RGID we obtained
 * for this process.  Return 0 otherwise.
 */
int
is_starting_rgid(gid)
	gid_t gid;
{
	check_auth_parameters();

	return gid == this_process_rgid;
}

/*
 * Return 1 if the argument GID is the same as the EGID we obtained
 * for this process.  Return 0 otherwise.
 */
int
is_starting_egid(gid)
	gid_t gid;
{
	check_auth_parameters();

	return gid == this_process_egid;
}


/*
 * Return the LUID when the set_auth_parameters() routine was first
 * called (which should be when the program first starts up but can be
 * as late as now.
 */
uid_t
starting_luid()
{
	check_auth_parameters();

	return this_process_luid;
}


/*
 * Return the RUID when the set_auth_parameters() routine was first
 * called (which should be when the program first starts up but can be
 * as late as now.
 */
uid_t
starting_ruid()
{
	check_auth_parameters();

	return this_process_ruid;
}


/*
 * Return the EUID when the set_auth_parameters() routine was first
 * called (which should be when the program first starts up but can be
 * as late as now.
 */
uid_t
starting_euid()
{
	check_auth_parameters();

	return this_process_euid;
}


/*
 * Return the RGID when the set_auth_parameters() routine was first
 * called (which should be when the program first starts up but can be
 * as late as now.
 */
gid_t
starting_rgid()
{
	check_auth_parameters();

	return this_process_rgid;
}


/*
 * Return the EGID when the set_auth_parameters() routine was first
 * called (which should be when the program first starts up but can be
 * as late as now.
 */
gid_t
starting_egid()
{
	check_auth_parameters();

	return this_process_egid;
}


/*
 * Set the appropriate signals to a state that prevents keyboard
 * interruption of program actions.  Do nothing if this is called
 * multiple times without an intervening exit_quiet_zone().  If an
 * alarm has been set, we save the time left to go.
 */
void
enter_quiet_zone()
{
	check_auth_parameters();

	if (++quiet == 1)  {
		old_hup = signal(SIGHUP, SIG_IGN);
		old_int = signal(SIGINT, SIG_IGN);
		old_quit = signal(SIGQUIT, SIG_IGN);
		old_sys = signal(SIGSYS, SIG_IGN);
		old_alarm = signal(SIGALRM, SIG_IGN);
		alarm_left = alarm(0);
		time_when_entered = time((time_t *) 0);
	}
}


/*
 * Restore the signals to their previous state.  We make sure we do not
 * call this reentrant since it uses a static area.  Restore a pending
 * alarm clock too.
 */
void
exit_quiet_zone()
{
	time_t time_in_zone;

	check_auth_parameters();

	if (--quiet == 0)  {
		(void) signal(SIGHUP, old_hup);
		(void) signal(SIGINT, old_int);
		(void) signal(SIGQUIT, old_quit);
		(void) signal(SIGSYS, old_sys);
		(void) signal(SIGALRM, old_alarm);

		/*
		 * If we took more time in the quiet zone than the
		 * alarm was set for, cause an alarm VERY soon.
		 * Otherwise, adjust the alarm time and resume the
		 * alarm clock.  Note the test os for >=, not > because
		 * the subtraction that follows would leave a 0 for == and
		 * disable the alarm.
		 */
		if (alarm_left != 0)  {
			time_in_zone = time((time_t *) 0) - time_when_entered;
			if (time_in_zone >= alarm_left)
				(void) alarm(1);
			else
				(void) alarm(alarm_left - time_in_zone);
		}

	}
	else  
	   if(quiet<0) {
		(void) fflush(stdout);
		quiet = 0;
	   }
}


static void
catch_no_sec_sys_calls()
{
	(void) fflush(stdout);
	(void) fprintf(stderr,
		MSGSTR(IDENTITY_3, "%s: Kernel is not configured for this security command.\n"),
		       command_name);
	(void) fflush(stderr);
	exit(0);
}

#ifdef STANDALONE
/* in the case of standalone operation, authorized_user returns TRUE */

int
authorized_user (subsys)
char *subsys;
{
	return(1);
}
#endif

/* #endif */ /*} SEC_BASE */
