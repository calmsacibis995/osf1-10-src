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
static char	*sccsid = "@(#)$RCSfile: init_sec.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/04/17 12:22:08 $";
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
 * Copyright (c) 1988-1990 SecureWare, Inc.  All rights reserved.
 *
 * This Module contains Proprietary Information of SecureWare, Inc.
 * and should be treated as Confidential.
 */

#if SEC_BASE


/*

 */

#include	<sys/types.h>
#include	<sys/security.h>
#include	<protcmd.h>
#include	<signal.h>
#include	<stdio.h>
#include	<errno.h>
#include	<fcntl.h>
#include	<utmp.h>

#define	IGNORE_CONSOLE	0
#define	NEED_CONSOLE	1

#ifdef	MSG
#include	"init_msg.h"
#endif

#if BSD_INIT

/* The Berkeley utmp format has a ut_name field in place of ut_user */
#undef ut_user
#define ut_user	ut_name

extern char	ctty[];
static char	init_script[] = "/tcb/files/sysinitrc";
static char	init_sh[] = "/bin/sh";

/*
 * Berkley-based systems do not have anything like inittab that can
 * be used to spawn the security policy daemon(s).  This hook is used
 * to run a shell script before any other processing is done. This
 * shell script contains the commands that are used to start those
 * processes that are needed prior to the single user shell being
 * created.
 */
void
init_sysinitrc()
{
	int	child_pid, wait_pid;

	child_pid = fork();
	if (child_pid == 0) {
		open("/", O_RDONLY);
		dup2(0, 1);
		dup2(0, 2);
		execl(init_sh, init_sh, init_script, (char *) 0);
		exit(1);
	}

	while ((wait_pid = wait((int *) 0)) != child_pid)
		if (wait_pid == -1 && errno == ECHILD)
			break;
}

#else /* !BSD_INIT */

/*
 * Use more private inittab and su files for the secure system.
 */
void
init_file_sources(inittab, su)
	char **inittab;
	char **su;
{
	if (inittab != (char **) 0)
		*inittab = INIT_INITTAB_LOCATION;

	if (su != (char **) 0)
		*su = INIT_SU_LOCATION;

}


/*
 * This routine knows the proper way to leave the mode in the
 * system after a data file used for init has been created.
 */
int
init_data_file_mask()
{
	return umask(~INIT_DATA_FILE_MODE);
}


/*
 * This routine knows the proper way to leave the mode in the
 * system after a file used for init has been created.
 */
void
init_secure_mask()
{
	(void) umask(~SEC_DEFAULT_MODE);
}

#endif /* !BSD_INIT */


/*
 * At startup, have a subprocess authenticate the booting user and verify
 * his authorization to boot the system.  The subprocess returns only if
 * 1) it cannot access the Authentication database, or 2) it authenticates
 * an authorized user.
 */
void
init_authenticate(single)
	int	single;
{
	if (!security_is_on())
		return;

	sub_process(single ? "single" : (char *) 0, (char *) 0, NEED_CONSOLE);
}

/*
 * Invoke initcond to establish the single-user shell.
 * We are called in a subprocess with the console
 * already open.
 */
void
init_shell(cmd, singleuid, singlegid, singledir)
	char	*cmd;
	int	singleuid;
	int	singlegid;
	char	*singledir;
{
	char	user[16], group[16];

	if (security_is_on()) {
	   sprintf(user, "%d", singleuid);
	   sprintf(group, "%d", singlegid);

	   execl(INITCOND_PROGRAM, INITCOND_PROGRAM, "init_shell", cmd, user,
		group, singledir, (char *) 0);
	} else
	   execl(cmd, "-", 0);

	exit(1);
}

/*
 * Update the terminal control database with information from this dead
 * process.  Deal with entries referring to USER_PROCESS, LOGIN_PROCESS
 * or INIT_PROCESS types (System V only),
 * for these could be sessions that are ending.
 * We don't write anything if there is no tty name in the utmp record.
 */
void
init_update_tty(u)
	register struct utmp *u;
{
	register int end_of_buf;
	char line[sizeof(u->ut_line) + 1];
	char user[sizeof(u->ut_user) + 1];

	if (!security_is_on())
		return;

#if BSD_INIT
	if (u && u->ut_line[0] != '\0')
#else
	if (u && u->ut_line[0] != '\0' && u->ut_type == USER_PROCESS)
#endif
	{
		/*
		 * Make sure we have null terminated strings to execl().
		 */
		end_of_buf = sizeof(u->ut_line);
		(void) strncpy(line, u->ut_line, end_of_buf);
		line[end_of_buf] = '\0';

		end_of_buf = sizeof(u->ut_user);
		(void) strncpy(user, u->ut_user, end_of_buf);
		user[end_of_buf] = '\0';

		sub_process(line, user, IGNORE_CONSOLE);
	}
}


#ifdef notdef /*{*/
/*
 * The reason some actions need to be in a subprocess
 * is two-fold.  First, we don't want the master init (PID==1) to open
 * a character special file or that will be the controlling terminal for
 * EVERY process, since it is inherited and not changed.  Second, the
 * code that will look into the Authentication database is quite large,
 * and carrying it around in the parent would be wasteful.  So, we not
 * only fork(), but we execl() INITCOND_PROGRAM also.
 */

/*
 * HOOK NOTE:
 *
 * This routine is EXTREMELY system-dependent.  Therefore, it has
 * been moved into the vendor code so that dependencies on the
 * structure of subprocess forking can be made explicit.  For
 * aiding in porting effort, the BSD and System V versions of these
 * hooks are included inside the #ifdef notdef directive here.
 * However, when this module needs to be delivered to a customer,
 * use unifdef(1) to remove this.  When doing a port to a specific
 * UNIX version, model the code after the way the console() routine
 * works.
 */

/* BSD VERSION OF THE ROUTINE: */

#if BSD_INIT

static void
sub_process(arg2, arg3, tty_support)
	char	*arg2;
	char	*arg3;
	int	tty_support;
{
	register int	child_pid, wait_pid, sig;

	do {
		child_pid = fork();
		if (child_pid == 0) {	/* child process */

			/* Reset signals to default */
			for (sig = 1; sig <= NSIG; ++sig)
				(void) signal(sig, SIG_DFL);
#ifdef SIGTSTP
			/*
			 * Ignore job control signals
			 */
			(void) signal(SIGTSTP, SIG_IGN);
			(void) signal(SIGTTIN, SIG_IGN);
			(void) signal(SIGTTOU, SIG_IGN);
#endif
			if (tty_support == NEED_CONSOLE) {
				(void) open(ctty, O_RDWR);
				(void) dup2(0, 1);
				(void) dup2(0, 2);
#ifdef _OSF_SOURCE
				/*
				 * Become a session leader and establish
				 * the controlling tty.
				 */
				(void) setsid();
				(void) ioctl(0, TIOCSCTTY, (char *) 0);
#endif
			}
			execl(INITCOND_PROGRAM, INITCOND_PROGRAM, "init",
				arg2, arg3, (char *) 0);
			exit(0);
		}
		/* parent process */
		while ((wait_pid = wait((int *) 0)) != child_pid)
			if (wait_pid == -1 && errno == ECHILD)
				break;
	} while (child_pid == -1);
}

#else /* !BSD_INIT */

/*
 * SYSTEM V VERSION OF THE ROUTINE.
 * The hpux #ifdefs show a version that supports BSD signals.
 * System V.3 added another argument to the efork() function.
 */

static void
sub_process(arg2, arg3, tty_support)
	char *arg2;
	char *arg3;
	int tty_support;
{
	register struct PROC_TABLE *process;
	register int status;
	register int fd_tmp;
	register char *cmd_end;
#ifdef hpux
	struct termio stterm;
	extern int systtycf;
#endif

	/*
	 * Fork a child who will do the dirty work.
	 */
#ifdef hpux
	process = efork(NULLPROC, 0);
#else
	(void) signal(SIGCHLD, SIG_DFL);

	do  {
#ifdef SYSV_3
		process = efork(M_OFF, NULLPROC, NOCLEANUP);
#else
		process = efork(NULLPROC, NOCLEANUP);
#endif
	}
	while (process == NO_ROOM);

	(void) signal(SIGCHLD, childeath);
#endif

	if (process == NULLPROC) {

		if (tty_support == NEED_CONSOLE)  {
			/*
			 * Open /dev/systty so that if someone types a <del>,
			 * can be informed of the fact.
			 */
			fd_tmp = open(SYSTTY, 2);
			if (fd_tmp != FAILURE)  {

#ifdef hpux
				/* Modeled after all other sysinit processes */
				stterm = dflt_termio;
				stterm.c_cflag = systtycf;
				ioctl(fd_tmp, TCSETA, &stterm);
#else
				/*
				 * Make sure the system tty is not RAW.
				 */
				(void) ioctl(fd_tmp, TCSETA, &dflt_termio);
#endif

				/*
				 * Make sure the file descriptor is greater than
				 * 2 so that it won't interfere with the
				 * standard descriptors.
				 */
				fd_systty = fcntl(fd_tmp, 0, 3);
				(void) close(fd_tmp);
	
				/*
				 * Prepare to catch the interrupt signal if <del>
				 * typed at /dev/systty.
				 */
				(void) signal(SIGINT, switchcon);
				(void) signal(SIGQUIT, switchcon);
			}
#ifdef	UDEBUG
			(void) signal(SIGUSR1, abort);
			(void) signal(SIGUSR2, abort);
#endif
			/*
			 * Close the current descriptors and open ones to
			 * /dev/syscon.
			 */
			opensyscon();
		}
		else  {

			/*
			 * For those cases where the console should not be
			 * opened, use /dev/null as a substitute terminal.
			 */
			(void) fclose(stdin);
			(void) fclose(stdout);
			(void) fclose(stderr);
			(void) close(0);
			(void) close(1);
			(void) close(2);
			(void) freopen("/dev/null", "r+", stdin);
			(void) freopen("/dev/null", "r+", stdout);
			(void) freopen("/dev/null", "r+", stderr);
		}

		cmd_end = strrchr(AUX_CMD, '/');
		if (cmd_end == (char *) 0)
			cmd_end = AUX_CMD;
		else
			cmd_end++;

		execl(AUX_CMD, cmd_end, "init", arg2, arg3, (char *) 0);
                        console(NLgetamsg(MF_INIT, MS_INIT_SEC, M_EXECL_FAIL,
			"execl of %s failed; errno = %d\r\n"),
			cmd_end, errno);
		timer(5);
		exit(1);
	}

	/*
	 * Wait for the child to die and return its status.
	 */
	do  {
		status = waitproc(process);
	}
	while (status == FAILURE);

	/*
	 * Ignore any signals such as powerfail when in "init_authenticate".
	 */
	wakeup.w_mask = 0;

#ifdef	DEBUG
	debug("sub_process: status: %o exit: %o termination: %o\n",
	       status, (status & 0xff00) >> 8, (status & 0xff));
#endif
}
#endif /* !BSD_INIT */
#endif /*} notdef */

#endif /* SEC_BASE */
