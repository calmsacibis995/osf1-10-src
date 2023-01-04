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
static char	*sccsid = "@(#)$RCSfile: init.c,v $ $Revision: 4.4.16.5 $ (DEC) $Date: 1994/01/31 16:08:32 $";
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
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: init
 *
 * ORIGINS: 3, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*
 *	"init" is the general process spawning program.  It reads
 *	/etc/inittab for a script.
 *
 * Modifications:
 *   5/29/91 - Blake Van Thof
 *          Added code to support capacity limitation.
 */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/limits.h>
#define USE_NEW_TTY		/* temporary? */
#include	<sys/ioctl.h>
#include	<sys/reboot.h>
#include        <sys/sysinfo.h>
#include	<signal.h>
#include	<utmp.h>
#include	<errno.h>
#include	<ctype.h>
#include	<fcntl.h>
#include 	<pwd.h>
#include	<stdio.h>
#include	<setjmp.h>
#if SEC_BASE
#include        <sys/security.h>
#include        <protcmd.h>

extern priv_t   *privvec();
#endif

#ifdef	MSG
#include	"init_msg.h"
#endif

#include	"init.h"

char            SU[] = "/sbin/su";	/* Super-user for single user mode. */
char            SH[] = "/sbin/sh";	/* Standard Shell */
char 		UMOUNT[] = "/sbin/umount"; /* Performs dismount of file systems */
									   /* when shutdown() called */
/* 
 * if defctab is changed here it must also be changed in NLchar.h in 
 * /usr/include/sys
 */
char		defctab[]	= "/etc/nls/ctab/default";

struct termios  dflt_termios = {
	TTYDEF_IFLAG,
	TTYDEF_OFLAG,
	TTYDEF_CFLAG,
	TTYDEF_LFLAG,
	{0},		/* control chars -- filled in at runtime */
	TTYDEF_SPEED,	/* input speed */
	TTYDEF_SPEED	/* output speed */
};
struct termios  termios;
struct proc *proc_table;	/* Table of active processes */
struct proc dummy;		/* A zero table used when calling "account" */
				/* for non-process type accounting. */
int wakeup_flags = 0;
volatile pid_t cur_pid = PID_MAX + 1; /* Initialize out of range */
sigset_t emptysigs;	/* Signal mask with no sigs set */
sigset_t allsigs;	/* Signal mask with all sigs set */
unsigned int	spawncnt,pausecnt;
int	rsflag;		/* Set if a respawn has taken place */
pid_t	own_pid;	/* This is the value of our own pid.
			* If the value is SPECIALPID, then we fork
			* to interact with outside world.  */
int	n_prev[NSIG];	/* Number of times previously in state */
int	cur_state = -1;	/* Current state of "init" */
int	prev_state;	/* State "init" was in last time it woke */
int	new_state;	/* State user wants "init" to go to. */
int	prior_state;	/* temp var for runstate changes a-c */
int	op_modes = BOOT_MODES;	/* Current state of "init" */
int utmp_exists; /* make visible to SIGUSR1 handler */
				 /* to clear on a dismount */
char	*CONSOLE	=	"/dev/console";	/* virtual system console */
char	*SYSCON		=	"/dev/syscon"; /* link to "trusted console" */
#ifdef UDEBUG
char	*UTMP		=	"utmp";
char	*WTMP		=	"wtmp";
char	*INITTAB	=	"inittab";
char	*DBG_FILE	=	"debug";

#else

char	*UTMP	=	UTMP_FILE;	/* Snapshot record file */
char	*WTMP	=	WTMP_FILE;	/* Long term record file */
char	*INITTAB =	"/etc/inittab";	/* Script file for "init" */

#ifdef	DEBUGGER

char	*DBG_FILE	=	"/etc/debug";

#endif
#endif
jmp_buf shutpass;
extern int time_up;
struct proc *efork();

/********************/
/****    main    ****/
/********************/
main(argc, argv)
int             argc;
char          *argv[];
{
	extern char     level();
	extern unsigned int spawncnt, pausecnt;
	int             defaultlevel;	/* what the system will boot up into */
	int             chg_lvl_flag;	/* did somebody ask us to change? */
	int             howto;
	sigset_t	osigmsk;

        int             fd;
        int             x = 2;
        char            n;
        int             i;
        char            buf[1024];
        /* code /upgrade in ascii plus a constant 0x64 */
        static char     fpdata[] =
                        {0x93,0xd9,0xd4,0xcb,0xd6,0xc5,0xc8,0xc9,0x64,0x64};
        char            fpstring[256];

        for(i = 0; i < 9; i++) {                /* BVT */
          fpstring[i] = fpdata[i] - 0x64;
        }
        if((fd = open(fpstring, O_RDONLY)) >= 0) {
          i = read(fd,buf,sizeof(buf));
          if(i > 0) {
            n = buf[87];
            if(buf[87] == ((~buf[88]) - 5)
                && (buf[87] == (~buf[87 + n + 9]))) {
              x = n;
            }
          }
        }
        close(fd);
	if((setsysinfo(SSI_SLIMIT,0,x,0,0)) != 0) /* BVT */
		exit(1);        /* must be root to continue */


#ifdef XDEBUG
	system("/sbin/mount -u /");
	unlink(DBG_FILE);
#endif


#ifdef	UDEBUG
	if (argc == 1)
		SPECIALPID = getpid();
#endif
	/*
	 * Determine if we are process SPECIALPID (the main init) or a user
	 * invoked init, whose job it is to inform init to change levels or
	 * perform some other action. 
	 */
	sigemptyset(&emptysigs);
	sigfillset(&allsigs);
	howto = 0;
	if ((own_pid = getpid()) != SPECIALPID)
		userinit(argc, argv);
	else if (argc > 1 && argv[1][0] == '-') {
		char           *cp;

		/* howto = 0; */
		cp = &argv[1][1];
		while (*cp)
			switch (*cp++) {
			case 'a':
				howto |= RB_ASKNAME;
				break;
			case 's':
				howto |= RB_SINGLE;
				break;
			}
	}
#ifdef	XDEBUG
	debug("We are past userinit().\n");
#endif
	if (setsid() < 0)
#ifdef MSG
		console(NLgetamsg(MF_INIT, MS_INIT, M_SETSID, "setsid() failed: errno = %d\n"), errno);
#else
		console("setsid() failed: errno = %d\n", errno);
#endif
	/*
	 * Set up the initial states and see if there is a default level
	 * supplied in the "/etc/inittab" file. 
	 */
	defaultlevel = initialize();
	chg_lvl_flag = FALSE;

#ifdef	XDEBUG
	debug("Debug version of init starting-pid = %d\n", SPECIALPID);
#endif
	/*
	 * If there is no default level supplied, ask the user to supply one.
	 * Here we will also set up our runlevel indicators new_state (what
	 * are we going to) and defaultlevel (where we start at boot time).
	 * cur_stat* is where we are presently at, and prev_state is the last
	 * state we were in.
	 */
	if ((howto & RB_SINGLE) || defaultlevel == -1) {
		new_state = SINGLE_USER;
		if (defaultlevel == -1)
			defaultlevel = 0;
	}
	else if (defaultlevel == 0)
		new_state = getlvl();
	else
		new_state = defaultlevel;
#ifdef	XDEBUG
	debug("We have completed getting new_state.\n");
#endif

#if SEC_BASE
        init_authenticate(new_state == SINGLE_USER);
#endif

	if (new_state == SINGLE_USER) {
		single(defaultlevel);
		chg_lvl_flag = TRUE;
	}
	else {
		prev_state = cur_state;
		if (cur_state >= 0) {
			n_prev[cur_state]++;
			prior_state = cur_state;
		}
		cur_state = new_state;
	}

	account(BOOT_TIME, &dummy, NULL);  /* Put Boot Entry in "utmp" */
	account(RUN_LVL, &dummy, NULL);	   /* Make the run level entry */
	endutent();

#ifdef	XDEBUG
	debug("We are at the beginning of the main process loop.\n");
#endif
#if SEC_BASE
        init_secure_mask();
#endif

	/* Here is the beginning of the main process loop. */
	for (;;) {
		/*
		 * If "normal" mode, check all living processes and initiate
		 * kill sequence on those that should not be there anymore.
		 */
		if (op_modes == NORMAL_MODES && cur_state != LVLa
		    && cur_state != LVLb && cur_state != LVLc
			&& cur_state != SINGLE_USER)
			remove_proc();
#ifdef	XDEBUG
		debug("We have completed remove_proc().\n");
#endif
		/*
		 * If a change in run levels is the reason we awoke, now do
		 * the accounting to report the change in the utmp file. Also
		 * report the change on the system console.
		 */
		if (chg_lvl_flag) {
			chg_lvl_flag = FALSE;
			account(RUN_LVL, &dummy, NULL);
#ifdef MSG
			console(NLgetamsg(MF_INIT, MS_INIT, M_NEWLVL,
					  "New run level: %c\n"),
				level(cur_state));
#else
			console("New run level: %c\n", level(cur_state));
#endif
		}
#ifdef	XDEBUG
		debug("We have completed account().\n");
#endif
		/*
		 * Scan the inittab file and spawn and respawn processes that
		 * should be alive in the current state.
		 */
		spawn();
		if (rsflag) {
			rsflag = 0;
			spawncnt++;
		}
		if (cur_state == SINGLE_USER) {
#ifdef	XDEBUG
			debug("We have gone into SINGLE-USER mode.\n");
#endif
			wakeup_flags = 0;
			single(0);
			if (cur_state != prev_state && cur_state != LVLa
			    && cur_state != LVLb && cur_state != LVLc) {
				chg_lvl_flag = TRUE;
				continue;
			}
		}
		/*
		 * If powerfail signal was received during last sequence, set
		 * mode to powerfail. When "spawn" is entered the first thing
		 * it does is check "powerhit".  If it is in PF_MODES then it
		 * clears "powerhit" and does a powerfail.  If it is not in
		 * PF_MODES, it puts itself in PF_MODES and then clears
		 * "powerhit". If "powerhit" sets again while "spawn" is
		 * working on a powerfail, the following code will see that
		 * "spawn" tries to execute the sequence again.  This ensures
		 * that the powerfail sequence will be successfully completed
		 * before further processing.
		 */
		if (wakeup_flags & W_POWERFAIL) {
			op_modes = PF_MODES;
			/*
			 * Make sure that cur_state != prev_state so
			 * ONCE/WAIT types work.
			 */
			prev_state = 0;
			/*
			 * If "spawn" was not just called while in "normal"
			 * mode then set the mode to "normal" and call again
			 * to check normal states.
			 */
		}
		else if (op_modes != NORMAL_MODES) {
			/*
			 * If we have just finished a powerfail sequence
			 * (which had the prev_state == 0), set the
			 * prev_state = cur_state before the next pass
			 * through.
			 */
			if (op_modes == PF_MODES)
				prev_state = cur_state;
			op_modes = NORMAL_MODES;
		}
		/*
		 * "spawn was last called with "normal" modes. If it
		 * was a change of levels that awakened us and the
		 * new level is one of the demand levels, LVL[a-c],
		 * then reset the cur_state to the previous state and
		 * do another scan to take care of the usual
		 * "respawn" actions.
		 */
		else if (cur_state == LVLa || cur_state == LVLb || cur_state == LVLc) {
			if (cur_state >= 0)
				n_prev[cur_state]++;
			cur_state = prior_state;
			prior_state = prev_state;
			prev_state = cur_state;
			account(RUN_LVL, &dummy, NULL);
			/*
			 * At this point "init" is finished with all actions
			 * for the current wakeup.
			 */
		}
		else {
			prev_state = cur_state;
			/*
			 * Now pause until there is a signal of some sort.
			 * We must make sure there is no window in which
			 * a signal can come in that we could miss.
			 */
			sigprocmask(SIG_SETMASK, &allsigs, &osigmsk);
			if (wakeup_flags == 0) {
				sigsuspend(&emptysigs);
				pausecnt++;
			}
			sigprocmask(SIG_SETMASK, &osigmsk, NULL);
			/*
			 * Now install the new level, if a change in level
			 * happened and then allow signals again while we do
			 * our normal processing.
			 */
			if (wakeup_flags & W_SHUTDOWN) {
				if (setjmp(shutpass) == 0)
					shutdown();
			}
			else if (wakeup_flags & W_USRSIGNAL) {
#ifdef	XDEBUG
				debug("\nmain\tSignal-new: %c cur: %c prev: %c\n",
				      level(new_state),
				      level(cur_state), level(prev_state));
#endif
				/*
				 * Set flag so that we know to change run
				 * level in utmp file. All the old processes
				 * have been removed.  Do not set the flag if
				 * a "telinit {Q | a | b | c}" was done or a
				 * telinit to the same level at which init is
				 * already running (which is the same thing
				 * as a "telinit Q").
				 */
				if (new_state != cur_state)
					if (new_state == LVLa
					    || new_state == LVLb
					    || new_state == LVLc) {
						prev_state = prior_state;
						prior_state = cur_state;
						cur_state = new_state;
						account(RUN_LVL, &dummy, NULL);
					}
					else {
						prev_state = cur_state;
						if (cur_state >= 0) {
							n_prev[cur_state]++;
							prior_state = cur_state;
						}
						cur_state = new_state;
						chg_lvl_flag = TRUE;
					}
				new_state = 0;
			}
			/*
			 * If we awoke because of a powerfail, change the
			 * operating mode to powerfail mode.
			 */
			if (wakeup_flags & W_POWERFAIL)
				op_modes = PF_MODES;

			/* Clear all wakeup reasons. */
			wakeup_flags = 0;
		} /* else */
	} /* for */
}

/**********************/
/****    single    ****/
/**********************/

single(defaultlevel)
	int             defaultlevel;
{
	register struct proc *sh_process;
	extern int      errno;
	extern int      new_state, cur_state, prev_state;
	extern int      wakeup_flags;
	extern int      childeath(void);
	int             state;

#ifdef	XDEBUG
	debug("We have entered single(%d).\n", defaultlevel);
#endif
	for (;;) {
#ifdef MSG
		console(NLgetamsg(MF_INIT, MS_INIT, M_SUSER, "SINGLE-USER MODE\n"));
#else
		console("SINGLE-USER MODE\n");
#endif
		while ((sh_process = efork(NULLPROC, NOCLEANUP)) == NO_ROOM)
			pause();

		if (sh_process == NULLPROC) {
			(void) revoke(CONSOLE);
			openconsole();

			(void) ioctl(0, TIOCSCTTY, (char *) 0);
#if SEC_BASE
                        /* Invoke the single user shell */
                        init_shell(SH, 0, 0, "/");
#ifdef MSG
                        console(NLgetamsg(MF_INIT, MS_INIT_SEC, M_SECEXEC,
                           "failed to start single-user shell; errno = %d\n"),
                           errno);
#else
                        console(
                      "failed to start single-user shell; errno = %d\n",
                         errno);
#endif
#else /* !SEC_BASE */
			/* Execute the shell. */
			execl(SH, "-", (char *) 0);
#ifdef MSG
			console(NLgetamsg(MF_INIT, MS_INIT, M_EXEC,
			   "execlp of %s failed; errno = %d\n"), SH, errno);
#else
			console("execlp of %s failed; errno = %d\n", SH, errno);
#endif
#endif /* !SEC_BASE */
			timer(5);
			exit(1);
		}
		/*
		 * If we are the parent, wait around for the child to die or
		 * for "init" to be signaled to change levels.
		 */
		while (waitproc(sh_process) == FAILURE) {

			/*
			 * Did we waken because a change of levels?  If so,
			 * kill the child and then exit.
			 */
			if (wakeup_flags & W_USRSIGNAL) {
				if (new_state >= LVL0 && new_state <= LVL9) {
					kill(sh_process->p_pid, SIGKILL);
					prev_state = cur_state;
					if (cur_state >= 0) {
						n_prev[cur_state]++;
						prior_state = cur_state;
					}
					cur_state = new_state;
					new_state = 0;
					wakeup_flags = 0;
					sh_process->p_flags &= ~NOCLEANUP;
					return;
				}
			}
			/*
			 * All other reasons for waking are ignored when in
			 * SINGLE_USER mode.  The only child we are
			 * interested in is being waited for explicitly by
			 * "waitproc".
			 */
			wakeup_flags = 0;
		}
		freeproc(sh_process);
		/*
		 * Since the sh user process died and the level hasn't been
		 * changed by a signal, either request a new level from the
		 * user if default one wasn't supplied, or use the supplied
		 * default level.
		 */
		if (defaultlevel != 0)
			state = defaultlevel;
		else
			state = getlvl();

		if (state != SINGLE_USER) {
			/*
			 * If the new level is not SINGLE_USER, then exit,
			 * otherwise go back and make up a new "sh" process.
			 */
			prev_state = cur_state;
			if (cur_state >= 0) {
				n_prev[cur_state]++;
				prior_state = cur_state;
			}
			cur_state = state;
			wakeup_flags = 0;
			return;
		}
	}
}

/**********************/
/****  remove_proc  ****/
/**********************/

/*
 * "remove_proc" scans thru "proc_table" and performs cleanup. If
 * there is a process in the table, which shouldn't be here at
 * the current runlevel, then "remove_proc" kills the processes.
 */
remove_proc()
{
	register struct proc *p, *nextp;
	struct CMD_LINE cmd;
	char            cmd_string[MAXCMDL];
	int             change_level;
	extern char    *vetchar();

#ifdef	XDEBUG
	debug("We have entered remove_proc().\n");
#endif
	change_level = (cur_state != prev_state);
	/*
	 * Clear the TOUCHED flag on all entries so that when we have
	 * finished scanning /etc/inittab, we will be able to tell if
	 * we have any processes for which there is no entry in
	 * /etc/inittab.
	 */
	for (p = proc_table; p != NULL; p = p->p_next)
		p->p_flags &= ~TOUCHED;

	/* Scan all /etc/inittab entries. */
	while (getcmd(&cmd, cmd_string) == TRUE) {
		/*
		 * Scan for process which goes with this entry in
		 * /etc/inittab.
		 */
		for (p = proc_table; p != NULL; p = p->p_next) {
			/*
			 * Does this slot contain the process we are looking
			 * for?
			 */
			if (id_eq(p->p_id, cmd.c_id)) {
#ifdef	XDEBUG
				debug("remove_proc- id:%s pid: %d time: %lo %d %o %o\n",
				      vetchar(p->p_id), p->p_pid,
				      p->p_time, p->p_count,
				      p->p_flags, p->p_exit);
#endif
				/*
				 * Is the cur_state SINGLE_USER or is this
				 * process marked as "off" or was this
				 * process was started by some other mechanism
				 * than the LVLa, LVLb, LVLc mechanism, and
				 * the current level does not support this
				 * process?
				 */
				if ((cur_state == SINGLE_USER)
				    || (cmd.c_action == M_OFF)
				    || ((cmd.c_levels & mask(cur_state)) == 0
					&& (p->p_flags & DEMANDREQUEST) == 0)) {
					if (p->p_flags & LIVING) {
						/*
						 * Touch this entry so that we
						 * will know that we've treated
						 * it.
						 * NOTE: Processes which are
						 * already dead at this point,
						 * but should not be restarted
						 * are left untouched. This
						 * causes their slot to be
						 * freed later after accounting
						 * is performed.
						 */
						p->p_flags |= TOUCHED;
						/*
						 * If this process has already
						 * been killed before, but for
						 * some reason hasn't
						 * disappeared yet, don't kill
						 * it again. Only kill it if
						 * the KILLED flag hasn't been
						 * set.
						 */
						if ((p->p_flags & KILLED) == 0) {
							/*
							 * If this is a change
							 * of levels call, don't
							 * fork a killing
							 * process for each
							 * process that must
							 * die.  Send the first
							 * warning signal
							 * yourself and mark
							 * the process as
							 * warned. If any
							 * warned processes
							 * fail to die in TWARN
							 * seconds, then kill
							 * them.
							 */
							if (change_level) {
								p->p_flags |= WARNED;
								kill(p->p_pid, SIGTERM);
								/*
								 * If this isn't
								 * change of
								 * levels,
								 * then fork
								 * killing
								 * process which
								 * will worry
								 * about details
								 * of killing
								 * the specified
								 * process. This
								 * allows "init"
								 * to continue
								 * work instead
								 * of pausing
								 * for TWARN
								 * seconds each
								 * pass through
								 * this routine.
								 */
							}
							else
								killproc(p->p_pid);
							/*
							 * Mark the process as
							 * having been sent its
							 * kill signals. It
							 * should show up as
							 * dead shortly, but
							 * just to be safe....
							 */
							p->p_flags |= KILLED;
						}
					}
				}
				/*
				 * This process can exist at the current
				 * level.  If it is also still alive or a
				 * DEMANDREQUEST, TOUCH it so that will be left
				 * alone.  If it is dead and not a
				 * DEMANDREQUEST, leave it untouched so that it
				 * will be accounted and cleaned up later on in
				 * "remove_proc".  Dead DEMANDREQUESTS will be
				 * accounted, but not freed.
				 */
				else if (p->p_flags & (LIVING | NOCLEANUP | DEMANDREQUEST))
					p->p_flags |= TOUCHED;

				break;
			}
		}
	}
	/*
	 * If this was a change of levels call, scan through the process
	 * table for processes that were warned to die.  If any are found
	 * that haven't left yet, sleep for TWARN seconds and then send
	 * final terminations to any that haven't died yet.
	 */
	if (change_level) {
		/*
		 * Set the alarm for TWARN seconds on the assumption that
		 * there will be some that need to be waited for.  This won't
		 * harm anything except we are guaranteed to wakeup in TWARN
		 * seconds whether we need to or not.
		 */
		setimer(TWARN);
		/*
		 * Scan for processes which should be dying.  We hope they
		 * will die without having to be sent a SIGKILL signal.
		 */
		for (p = proc_table; p != NULL; p = p->p_next) {
			sigset_t sigmsk, osigmsk;
			/*
			 * If this process should die, hasn't yet, and the
			 * TWARN time hasn't expired yet, wait around for
			 * process to die or for timer to expire.
			 */
			sigmsk = emptysigs;
			sigaddset(&sigmsk, SIGCHLD);
			sigaddset(&sigmsk, SIGALRM);
			sigprocmask(SIG_BLOCK, &sigmsk, &osigmsk);
			while ((time_up == FALSE) && (p->p_flags &
				      (WARNED | LIVING)) == (WARNED | LIVING))
				sigsuspend(&emptysigs);
			sigprocmask(SIG_SETMASK, &osigmsk, NULL);
		}
		/* If we reached the end of the proc table without the timer
		 * expiring, then there are no processes which will have to
		 * be sent the SIGKILL signal.  If the timer has expired, then
		 * it is necessary to scan the table again and send signals to
		 * all processes which aren't going away nicely.
		 */
		if (time_up == TRUE)
			for (p = proc_table; p != NULL; p = p->p_next) {
				/*
				 * Is this a WARNED process that hasn't died
				 * yet?
				 */
				if ((p->p_flags & (WARNED | LIVING)) ==
				    (WARNED | LIVING))
					kill(p->p_pid, SIGKILL);
			}
		setimer(0);
	}
	/*
	 * Rescan the proc_table for two kinds of entry, those marked
	 * as LIVING, NAMED, and which don't have an entry in
	 * /etc/inittab (haven't been TOUCHED by the above scanning), and
	 * haven't been sent kill signals, and those entries marked as
	 * not LIVING, NAMED.  The former processes should be killed.
	 * The latter entries should have DEAD_PROCESS accounting done
	 * on them and the slot cleared.
	 */
	for (p = proc_table; p != NULL; ) {
		nextp = p->p_next;
		if ((p->p_flags & (LIVING | NAMED | TOUCHED | KILLED))
						== (LIVING | NAMED)) {
			killproc(p->p_pid);
			p->p_flags |= KILLED;
		}
		else if ((p->p_flags & (LIVING | NAMED)) == NAMED) {
			account(DEAD_PROCESS, p, NULL);
			/*
			 * If this named process hasn't been TOUCHED, then
			 * free the space. It has either died of its own accord,
			 * but isn't respawnable or was killed because it
			 * shouldn't exit at this level.
			 */
			if ((p->p_flags & TOUCHED) == 0)
				freeproc(p);
		}
		p = nextp;
	}
}

/***********************/
/****    shutdown   ****/
/***********************/

int	shutreset();

/*
 * Shutdown the system (to single user mode).  Initiated
 * by sending SIGTERM to init.  In contrast to simply
 * telling init to change the run level to single user
 * mode, shutting down causes init to kill ALL processes,
 * not just the ones it started.
 */
shutdown()
{
	register int i;
	int shutreset(void);

	signal(SIGHUP, SIG_IGN);
	new_state = cur_state = SINGLE_USER;
	op_modes = BOOT_MODES;
	account(RUN_LVL, &dummy, NULL);
	remove_proc();
	signal(SIGALRM, (void (*)(int))shutreset);
	signal(SIGCHLD, SIG_IGN);
	(void) kill(-1, SIGTERM);	/* one chance to catch it */
	sleep(8);
	alarm(40);
	for (i = 0; i < 5; i++)
		kill(-1, SIGKILL);
 	while (wait((int *)0) != -1)
		; 
	alarm(0);
	shutend();
}

shutreset()
{
#ifdef MSG
	console(NLgetamsg(MF_INIT, MS_INIT, M_WARNING, "WARNING: Something is hung (won't die); ps axl advised\n"));
#else
	console("WARNING: Something is hung (won't die); ps axl advised\n");
#endif /* MSG */
	sleep(5);
	shutend();
	longjmp(shutpass, 1);
}

shutend()
{
	register int i, f;
	struct sigaction action;
	int alarmclk(void);
	int childeath(void);
	int siglvl(int);
	pid_t pid;

	acct(0);
#ifdef notdef
	for (i = 0; i < 10; i++)
		close(i);
	f = open(wtmpf, O_WRONLY|O_APPEND);
	if (f >= 0) {
		SCPYN(wtmp.ut_line, "~");
		SCPYN(wtmp.ut_name, "shutdown");
		SCPYN(wtmp.ut_host, "");
		time(&wtmp.ut_time);
		write(f, (char *)&wtmp, sizeof(wtmp));
		close(f);
	}
#endif

	/* Tell account() to cache info in memory */

	utmp_exists = 0;
	 
 	/* Perform file system dismounts */
	/* -A: Remove all mounts; ufs, nfs auto/manual */
	/* -f: Remove nfs mounts without server confirmation */

	if((pid = fork()) < 0) { 
		perror("fork");
	} else if  (pid == 0) {
		execl(UMOUNT, "umount", "-Af", NULL);
		exit(-1);
	} 
	if(waitpid(pid, NULL, 0) < 0)
		perror("waitpid");

	action.sa_flags = action.sa_mask = 0;
	action.sa_handler = (void (*) (int)) alarmclk;
	sigaction(SIGALRM, &action, NULL);
	action.sa_handler = (void (*) (int)) childeath;
	sigaction(SIGCHLD, &action, NULL);
	action.sa_handler = (void (*) (int)) siglvl;
	sigaction(SIGHUP, &action, NULL);

}

jmp_buf idlebuf;

void
idlehup()
{
	longjmp(idlebuf, 1);
}

/*********************/
/****    idle    ****/
/*********************/
/*
 * Idle init.  Entered on receipt of the SIGTSTP
 * signal.  This signal is sent by halt and reboot
 * prior to their killing off all running processes,
 * in order to prevent init from trying to restart
 * the processes that die.
 */
void
idle()
{
	register int pid;
	register struct proc *p, *nextp;
	struct sigaction action, oaction;

	action.sa_flags = action.sa_mask = 0;
	action.sa_handler = (void (*) (int)) idlehup;
	sigaction(SIGHUP, &action, &oaction);
	for (;;) {
		if (setjmp(idlebuf) != 0) {
			sigaction(SIGHUP, &oaction, NULL);
			return;
		}
		sigsuspend(&emptysigs);
		for (p = proc_table; p != NULL; ) {
			nextp = p->p_next;
			/* Ignore NOCLEANUP here */
			if ((p->p_flags & LIVING) == 0) {
#ifdef	XDEBUG
				debug("idle- id:%s pid: %d time: %lo %d %o %o\n",
				      vetchar(p->p_id), p->p_pid, p->p_time,
				      p->p_count, p->p_flags, p->p_exit);
#endif
				/*
				 * Is this a named process?  If so, do the
				 * necessary bookkeeping. 
				 */
				if (p->p_flags & NAMED)
					account(DEAD_PROCESS, p, NULL);
				freeproc(p);
			}
			p = nextp;
		}
	}
}

/*********************/
/****    spawn    ****/
/*********************/
/*
 *	"spawn" scans /etc/inittab for entries which should be run at
 *	this mode.  If a process which should be running is found not
 *	to be running, then it is started.
 */
spawn()
{
	register struct proc *p;
	struct CMD_LINE cmd;
	char            cmd_string[MAXCMDL];
	short           lvl_mask;
	extern struct proc *findpslot();
#ifdef	XDEBUG
	extern char     level();
	extern char    *ctime(), *vetchar();
#endif

#ifdef	XDEBUG
	debug("We have entered spawn().\n");
#endif
	/*
	 * First check the "powerhit" flag.  If it is set, make sure
	 * the modes are PF_MODES and clear the "powerhit" flag.
	 * Avoid the possible race on the "powerhit" flag by disallowing
	 * a new powerfail interupt between the test of the powerhit
	 * flag and the clearing of it.
	 */
	if (wakeup_flags & W_POWERFAIL) {
		wakeup_flags &= ~W_POWERFAIL;
		op_modes = PF_MODES;
	}
	lvl_mask = mask(cur_state);

#ifdef	DEBUG1
	debug("spawn\tSignal-new: %c cur: %c prev: %c\n", level(new_state), level(cur_state), level(prev_state));
	debug("spawn- lvl_mask: %o op_modes: %o\n", lvl_mask, op_modes);
#endif


	/* 
	 * Scan through all the entries in /etc/inittab. 
	 * While scanning, the findpslot() function will allocate
	 * new proc structures, which must be freed if not
	 * valid.
	 */

	p = NULLPROC;
	while (getcmd(&cmd, cmd_string) == TRUE) {
		if ((p != NULLPROC) && (p->p_flags == 0)) {
			freeproc(p);
			p = NULLPROC;
		}
		/* 
		 *	Do not create any additional processes at this level
		 *  if a user signal has been received.
		 */
		if (wakeup_flags & W_USRSIGNAL) 
			continue;

		/*
		 * Find out if there is a process slot for this entry
		 * already.
		 */
		if ((p = findpslot(&cmd)) == NULLPROC) {
			/*
			 * Only generate an error message once every
			 * WARNFREQUENCY secondswhen the internal process
			 * table is full
			 */
			if (error_time(FULLTABLE))
#ifdef MSG
				console(NLgetamsg(MF_INIT, MS_INIT, M_PTAB,
				      "Internal process table is full.\n"));
#else
				console("Internal process table is full.\n");
#endif
			continue;
		}
		/*
		 * If there is an entry, and it is marked as DEMANDREQUEST,
		 * one of the levels a,b, or c is in its levels mask, and the
		 * action field is ONDEMAND and ONDEMAND is a permissible mode,
		 * and the process is dead, then respawn it.
		 */
		if (((p->p_flags & (LIVING | DEMANDREQUEST)) == DEMANDREQUEST)
		    && (cmd.c_levels & (MASKa | MASKb | MASKc))
		    && (cmd.c_action & op_modes) == M_ONDEMAND) {
			respawn(p, &cmd);
			/* Now finished with this entry. */
			continue;
		}
#ifdef	DEBUG1
		debug("process:\t%s\t%05d\n%s\t%d\t%o\t%o\n",
		      vetchar(p->p_id), p->p_pid,
		      ctime(&p->p_time), p->p_count,
		      p->p_flags, p->p_exit);
		debug("cmd:\t%s\t%o\t%o\n\"%s\"\n", vetchar(cmd.c_id),
		      cmd.c_levels, cmd.c_action, cmd.c_command);
#endif
		/*
		 * If the action is not an action we are interested in, skip
		 * the entry. 
		 */
		if ((cmd.c_action & op_modes) == 0) {
			continue;
		}
		if (p->p_flags & LIVING) {
			continue;
		}
		if ((cmd.c_levels & lvl_mask) == 0) {
			continue;
		}
		/*
		 * If the modes are the normal modes (ONCE, WAIT, RESPAWN, OFF,
		 * ONDEMAND) and the action field is either OFF or the action
		 * field is ONCE or WAIT and the current level is the same as
		 * the last level, then skip this entry.  ONCE and WAIT only get
		 * run when the level changes.
		 */
		if ((op_modes == NORMAL_MODES)
		    && (cmd.c_action == M_OFF || (cmd.c_action & (M_ONCE | M_WAIT))
			&& cur_state == prev_state)) {
			continue;
		}
		/*
		 * At this point we are interested in performing the action
		 * for this entry.  Actions fall into two categories, spinning
		 * off a process and not waiting, and spinning off a process
		 * and waiting for it to die.  If the action is ONCE, RESPAWN,
		 * ONDEMAND, POWERFAIL, or BOOT then spin off a process, but
		 * don't wait.
		 */
		if (cmd.c_action & (M_ONCE | M_RESPAWN | M_PF | M_BOOT))
			respawn(p, &cmd);
		/*
		 * The action must be WAIT, BOOTWAIT, or POWERWAIT, therefore
		 * spin off the process, but wait for it to die before
		 * continuing. 
		 */
		else {
			respawn(p, &cmd);
			while (waitproc(p) == FAILURE);
			account(DEAD_PROCESS, p, NULL);
			freeproc(p);
			p = NULLPROC;
		}
	}
	if ((p != NULLPROC) && (p->p_flags == 0))
		freeproc(p);
}

/* B-Add by Glenn Marcy */

void
_close_files()
{
	register int    i;

	for (i = getdtablesize() - 1; i >= 0; i--)
		close(i);
}

/* E-Add by Glenn Marcy */

/***********************/
/****    respawn    ****/
/***********************/

/*
 * "respawn" spawns a shell, inserts the information about the
 * process into the proc_table, and does the startup accounting.
 */
respawn(process, cmd)
	struct proc *process;
	struct CMD_LINE *cmd;
{
	int             modes;
	extern int      childeath(void);
	extern int      cur_state, errno;
	extern char    *strrchr(), *strchr();
	struct proc tmproc, *oprocess;
	time_t          now;
	static char    *envp[] = {"PATH=/sbin:/etc:/usr/sbin", (char *) 0};
	extern int      rsflag;
	struct utmp    *dmmy;	/* Used only to get size of ut_user */
	register char	*cp;
	static char     word[sizeof(dmmy->ut_user) + 1];

#ifdef	DEBUG1
	extern char    *vetchar();
	debug("**  respawn  **  id:%s\n", vetchar(process->p_id));
#endif
#ifdef	XDEBUG
	debug("We have entered respawn().\n");
#endif
	/*
	 * The modes to be sent to "efork" are 0 unless we are spawning
	 * a LVLa, LVLb, or LVLc entry or we will be waiting for the
	 * death of the child before continuing.
	 */
	modes = NAMED;
	if ((process->p_flags & DEMANDREQUEST) || cur_state == LVLa
	    || cur_state == LVLb || cur_state == LVLc)
		modes |= DEMANDREQUEST;
	if ((cmd->c_action & (M_SYSINIT | M_WAIT | M_BOOTWAIT | M_PWAIT)) != 0)
		modes |= NOCLEANUP;
	/*
	 * If this is a respawnable process, check the threshold
	 * information to avoid excessive respawns.
	 */
	if (cmd->c_action & M_RESPAWN) {
		/*
		 * Add the NOCLEANUP to all respawnable commands so that the
		 * information about the frequency of respawns isn't lost.
		 */
		modes |= NOCLEANUP;
		time(&now);
		/*
		 * If no time is assigned, then this is the first time this
		 * command is being processed in this series. Assign the
		 * current time. 
		 */
		if (process->p_time == 0)
			process->p_time = now;

		/* Have we just reached the respawn limit? */
		if (process->p_count++ == SPAWN_LIMIT) {
			/* If so, have we been respawning it too rapidly? */
			if ((now - process->p_time) < SPAWN_INTERVAL) {
				/*
				 * If so, generate an error message and
				 * refuse to respawn the process for now.
				 */
#ifdef MSG
				console(NLgetamsg(MF_INIT, MS_INIT, M_RAPID,
				    "Command is respawning too rapidly. Check for possible errors.\nid:%6s \"%s\"\n"),
				    cmd->c_id, &cmd->c_command[EXEC]);
#else
				console("Command is respawning too rapidly. Check for possible errors.\nid:%6s \"%s\"\n",
				    cmd->c_id, &cmd->c_command[EXEC]);
#endif
				return;
			}
			process->p_time = now;
			process->p_count = 0;
			/*
			 * If this process has been respawning too rapidly
			 * and the inhibit time limit hasn't expired yet,
			 * refuse to respawn. 
			 */
		}
		else if (process->p_count > SPAWN_LIMIT) {
			if ((now - process->p_time) < (SPAWN_INTERVAL + INHIBIT))
				return;
			process->p_time = now;
			process->p_count = 0;
		}
		rsflag = TRUE;
	}
	/* Spawn a child process to execute this command. */
	oprocess = process;
	while ((process = efork(oprocess, modes)) == NO_ROOM)
		pause();

	/*
	 * If we are the child, close up all the open files and set up the
	 * default standard input and standard outputs.
	 */
	if (process == NULLPROC) {
		int		i;
		FILE		*fp;
		char		*lastp;
		/*
		 * Make sure the child uses a different file pointer in the
		 * OS for its references to /etc/utmp.  If this isn't done, the
		 * seeks and reads of the child and parent will compete with
		 * each other.
		 */
		endutent();
		/*
		 * Perform the accounting for the beginning of a process.
		 * Note that all processes are initially "INIT_PROCESS"es.
		 * Getty will change the type to "LOGIN_PROCESS" and login will
		 * change it to "USER_PROCESS" when they run.
		 */
		strncpy(tmproc.p_id, cmd->c_id, IDENT_LEN);
		tmproc.p_pid = getpid();
		tmproc.p_exit = 0;
		cp = &cmd->c_command[EXEC];
		lastp = NULL;
		while (*cp != ' ' && *cp != '\t' && *cp != '\n' && *cp != '\0') {
			if (*cp++ == '/')
				lastp = cp;
		}
		if (lastp != NULL)
			cp = lastp;
		else
			cp = &cmd->c_command[EXEC];
		memcpy(word, cp, sizeof(word));
		account(INIT_PROCESS, &tmproc, word);
		/*
	 	 * If the completion of this process will block init, attach
		 * it to the terminal so that it may be aborted by the user
		 * if necessary rather than hanging init.
	 	 */
		if (cmd->c_action & (M_WAIT | M_BOOTWAIT | M_PWAIT)) {
			openconsole();
			ioctl(0, TIOCSCTTY, 0);
		}
		/*
		 * Now exec the command.  Pass it to a shell with -c if
		 * it contains meta characters, otherwise, exec it directly.
		 */
		_close_files();
#ifdef doexec
		if (strpbrk(cmd->c_command, "=|^();&<>*?[]:$`'\"\\\n") == NULL) {
			char *cmdargs[100];
			char *cmdname;
			extern char *strtok();
			int i;

			i = 0;
			cp = strtok(cmd->c_command, " \t");
			if (cp != NULL) {
				do {
					if (*cp == '#')
						break;
					/*
					 * Skip initial "exec" inserted by
					 * getcmd.
					 */
					if (i > 0 || strcmp(cp, "exec"))
						cmdargs[i++] = cp;
					cp = strtok(NULL, " \t");
				} while (cp != NULL);
			}
			cmdargs[i] = NULL;
			cmdname = cmdargs[0];
			cmdargs[0] = "-";
			execve(cmdname, cmdargs, envp);
		}
		else
#endif /* doexec */
			execle(SH, "initsh", "-c", cmd->c_command, (char *)0, envp);

		/* If the "exec" fails, print an error message. */
#ifdef MSG
		console(NLgetamsg(MF_INIT, MS_INIT, M_CMDFAIL, "Command\n\"%s\"\n failed to execute.  errno = %d (exec of shell failed)\n"), cmd->c_command, errno);
#else
		console("Command\n\"%s\"\n failed to execute.  errno = %d (exec of shell failed)\n", cmd->c_command, errno);
#endif
		/*
		 * Don't come back so quickly that "init" hasn't had a chance
		 * to complete putting this child in "proc_table".
		 */
		timer(20);
		exit(1);

	}
	else {
		/*
		 * We are the parent, therefore insert the necessary
		 * information in the proc_table.
		 */
		strncpy(process->p_id, cmd->c_id, IDENT_LEN);
	}
}

/************************/
/****    findpslot    ****/
/************************/

/*
 * findpslot() finds the existing entry in the process table for the
 * command with the given id, or it allocates a new entry.
 */
struct proc *
findpslot(cmd)
	register struct CMD_LINE *cmd;
{
	register struct proc *p;
	extern char *calloc();

#ifdef	XDEBUG
	debug("We have entered findpslot().\n");
#endif
	for (p = proc_table; p != NULL; p = p->p_next)
		if (id_eq(p->p_id, cmd->c_id))
			break;
	/*
	 * If there is no entry for this slot, then allocate a new entry
	 * and insert it into the linked list of processes.
	 * If there is no space available, we return NULL, and the caller
	 * will have to complain. 
	 */
	if (p == NULL) {
#ifdef	XDEBUG
		debug("findpslot: allocating new proc\n");
#endif
		p = (struct proc *)calloc(1, sizeof(*p));
		if (p != NULL) {
			if (proc_table != NULL)
				proc_table->p_prev = p;
			p->p_next = proc_table;
			proc_table = p;
		}
	}
	return(p);
}

/************************/
/****    freeproc    ****/
/************************/

freeproc(p)
register struct proc *p;
{
	if (p->p_next)
		p->p_next->p_prev = p->p_prev;
	if (p->p_prev)
		p->p_prev->p_next = p->p_next;
	else
		proc_table = p->p_next;
	free(p);
}

/********************/
/****    mask    ****/
/********************/

mask(lvl)
	int             lvl;
{
	register int    answer;
#ifdef	XDEBUG
	debug("We have entered mask().\n");
#endif
	switch (lvl) {
	case LVLQ:
		answer = 0;
		break;
	case LVL0:
		answer = MASK0;
		break;
	case LVL1:
		answer = MASK1;
		break;
	case LVL2:
		answer = MASK2;
		break;
	case LVL3:
		answer = MASK3;
		break;
	case LVL4:
		answer = MASK4;
		break;
	case LVL5:
		answer = MASK5;
		break;
	case LVL6:
		answer = MASK6;
		break;
	case LVL7:
		answer = MASK7;
		break;
	case LVL8:
		answer = MASK8;
		break;
	case LVL9:
		answer = MASK9;
		break;
	case SINGLE_USER:
		answer = MASKSU;
		break;
	case LVLa:
		answer = MASKa;
		break;
	case LVLb:
		answer = MASKb;
		break;
	case LVLc:
		answer = MASKc;
		break;
	default:
		answer = FAILURE;
		break;
	}
	return (answer);
}

/*********************/
/****    level    ****/
/*********************/

char
level(state)
	int             state;
{
	register char   answer;

#ifdef	XDEBUG
	debug("We have entered level().\n");
#endif
	switch (state) {
	case LVL0:
		answer = '0';
		break;
	case LVL1:
		answer = '1';
		break;
	case LVL2:
		answer = '2';
		break;
	case LVL3:
		answer = '3';
		break;
	case LVL4:
		answer = '4';
		break;
	case LVL5:
		answer = '5';
		break;
	case LVL6:
		answer = '6';
		break;
	case LVL7:
		answer = '7';
		break;
	case LVL8:
		answer = '8';
		break;
	case LVL9:
		answer = '9';
		break;
	case SINGLE_USER:
		answer = 'S';
		break;
	case LVLa:
		answer = 'a';
		break;
	case LVLb:
		answer = 'b';
		break;
	case LVLc:
		answer = 'c';
		break;
	default:
		answer = '?';
		break;
	}
	return (answer);
}

/************************/
/****    killproc    ****/
/************************/

/*
 *	"killproc" sends the SIGTERM signal to the specified process
 *	and then after TWARN seconds, the SIGKILL signal.
 */

killproc(pid)
	register pid_t  pid;
{
	extern int      childeath(void);
	register int	mypid;

#ifdef	XDEBUG
	debug("We have entered killproc().\n");
#endif
	while ((mypid = fork()) == -1)
		pause();
	/*
	 * If we are the child, send the signals to the process we are to
	 * kill. 
	 */
	if (mypid == 0) {
		kill(pid, SIGTERM);	/* Warn the process to quit.    */
		timer(TWARN);	/* Sleep TWARN seconds */
		kill(pid, SIGKILL);	/* Kill the process if still alive. */
		exit(0);
	}
	else
		waitpid(mypid, NULL, 0);
}

/**************************/
/****    initialize    ****/
/**************************/

/*
 *	Perform the initial state setup and look for an initdefault
 *	entry in the "inittab" file.
 */
initialize()
{
	struct CMD_LINE cmd;
	char            command[MAXCMDL];
	extern int      cur_state, op_modes;
	extern int      childeath(void);
	register int    msk, i;
	static int      states[] = {LVL0, LVL1, LVL2, LVL3, LVL4, LVL5, LVL6, LVL7, LVL8, LVL9, SINGLE_USER};
	FILE           *fp_syscon;
	char            device[sizeof("/dev/") + NAME_MAX];
	int             initstate, flag;
	register struct proc *p, *oprocess;
	extern struct proc *findpslot();

	/*
	 * Initialize state to "SINGLE_USER" "BOOT_MODES"
	 * At conception we are cur_state = -1 for safety
	 */
	if (cur_state >= 0) {
		n_prev[cur_state]++;
		prior_state = cur_state;
	}
	cur_state = SINGLE_USER;
	op_modes = BOOT_MODES;

	/* Set up all signals to be caught or ignored as is appropriate. */
	init_signals();
	initstate = 0;

	/* Get the ioctl settings for /dev/console */
	get_ioctl_console();
	/*
	 * Look for an "initdefault" entry in "/etc/inittab", which
	 * specifies the initial level to which "init" is to go at
	 * startup time.
	 */
	while ((flag = getcmd(&cmd, command)) == TRUE) {
		if (cmd.c_action == M_INITDEFAULT) {
			/*
			 * Look through the "c_levels" word, starting at the
			 * highest level.  The assumption is that there will
			 * only be one level specified, but if there is more
			 * than one, the system will come up at the highest
			 * possible level.
			 */
			for (msk = MASKSU, i = (sizeof(states) / sizeof(int)) - 1; msk > 0; msk >>= 1, i--) {
				if (msk & cmd.c_levels) {
					initstate = states[i];
				}
			}
			/*
			 * If the entry is for a system initialization
			 * command, execute it at once, and wait for it to
			 * complete.
			 */
		}
/* FIRST PASS CODE
*  There will be an "elseif" installed here in the second pass that will
*  look for a "CONSOLE" entry in /etc/inittab. If there is one, the
*  system console is not the default one and a flag will be set.
*/
		else if (cmd.c_action == M_SYSINIT) {
			if (p = findpslot(&cmd)) {
				for (oprocess = p;
				     (p = efork(oprocess, (NAMED | NOCLEANUP))) == NO_ROOM;)
					;
				if (p == NULLPROC) {
					FILE           *fp;
					/*
					 * Notice no bookkeeping is performed
					 * on these entries.  This is to avoid
					 * doing anything that would cause
					 * writes to the file system to take
					 * place.  No writing should be done
					 * until the operator has had the chance
					 * to decide whether the file system 
					 * needs checking or not.
					 */
					_close_files();
					execl(SH, "initsh", "-c", cmd.c_command, (char *) 0);
					exit(1);
				}
				else
					while (waitproc(p) == FAILURE);
#ifdef	ACCTDEBUG
				debug("SYSINIT- id: %.6s term: %o exit: %o\n",
				      cmd.c_id, (p->p_exit & 0xff),
				      (p->p_exit & 0xff00) >> 8);
#endif
				freeproc(p);
			}
		}
	}
#ifdef XDEBUG
	debug("initialize: initstate = %d\n", initstate);
#endif
	if (initstate)
		return (initstate);

/* FIRST PASS CODE
*  We assume in the first pass that the console is always /dev/console.
*  Future passes will include have the /dev/console driver remember
*  what the console is (inittab) and change to that console.
*    Essentially, nothing happens here UNLESS the console device is
*  supposed to be remote (not the trusted console), in which case we notify
*  the trusted console of the impending change.
*  BY THE WAY: I need to determine if /dev/console -> /dev/syscon, which
*  is a case that makes this code redundant.
*/
	/*
	 * If the system console is remote, put a message on the
	 * system tty warning anyone there that console is elsewhere.
	 */
/*
	while ((p = efork(NULLPROC,NOCLEANUP)) == NO_ROOM)
		;
	if (p == NULLPROC) {
		if ((fp_syscon = fopen(SYSCON,"r+")) != (FILE*)NULL) {
#ifdef MSG
			NLfprintf(fp_syscon, NLgetamsg(MF_INIT, MS_INIT, M_REMOTE, 
				"\nInit: system console is remote:\
				%s Type <DEL> to regain control.\n"), device);
#else
			NLfprintf(fp_syscon, "\nInit: system console is remote:\
				%s Type <DEL> to regain control.\n", device);
#endif
			fflush(fp_syscon);
			fclose(fp_syscon);
		}
		exit(0);
	}
	while(waitproc(p) == FAILURE);
	freeproc(p);
*/
	/*
	 * Since no "initdefault" entry was found, return 0.  This will
	 * have "init" ask the user at /dev/console to supply a level.
	 */
	return (flag);
}

/**********************/
/****    getlvl    ****/
/**********************/

/*	Get the new run level from /dev/console. */

int             fd_syscon;

int
getlvl()
{
	char            c;
	int             status;
	int             flag;
#ifdef	UDEBUG
	extern int      abort();
#endif
	FILE           *fp_tmp;
	register        p;
	extern int      fd_syscon;
	extern int      switchcon(int);
	static char     levels[] = {LVL0, LVL1, LVL2, LVL3, LVL4, LVL5, LVL6, LVL7, LVL8, LVL9, SINGLE_USER};
	struct sigaction action, oaction;

	/*
	 * Fork a child who will request the new run level from
	 * /dev/console.
	 */
#ifdef XDEBUG
	debug("We are entering getlvl()\n");
#endif

	while ((p = fork()) == -1)
		;
/*
 * ensure that this routine can wait for it's child and not
 * lose the signal to childeath - burns 7-12-91
 */
	action.sa_flags = 0;
	action.sa_mask = 0;
	action.sa_handler = SIG_IGN;
	sigaction(SIGCHLD, &action, &oaction);

	if (p == 0) {
		action.sa_flags = 0;
		action.sa_mask = 0;
		action.sa_handler = SIG_IGN;
		sigaction(SIGHUP, &action, (struct sigaction *) NULL);	/* ignore telinit q */
		/*
		 * Open /dev/syscon so that if someone types a <del>, we can
		 * be informed of the fact.
		 */
		if ((fp_tmp = fopen(SYSCON, "r+")) != NULL) {
			/*
			 * Make sure the file descriptor is greater than 2 so
			 * that it 
			 */
			/* won't interfere with the standard descriptors. */
			fd_syscon = fcntl((int) fileno(fp_tmp), F_DUPFD, 3);
			fdopen(fd_syscon, "r+");
			fclose(fp_tmp);
			/*
			 * Prepare to catch the interupt signal if <del>
			 * typed at /dev/syscon.
			 */
			action.sa_handler = (void (*) (int)) switchcon;
			sigaction(SIGINT, &action, (struct sigaction *) NULL);
			sigaction(SIGQUIT, &action, (struct sigaction *) NULL);
		}
#ifdef	UDEBUG
		action.sa_handler = (void (*) (int)) abort;
		sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
		sigaction(SIGUSR2, &action, (struct sigaction *) NULL);
#endif
		for (;;) {
			/*
			 * Close the current descriptors and open ones to
			 * /dev/console. 
			 */
			openconsole();
			/*
			 * Print something unimportant and pause, since
			 * reboot may be taking place over a line coming in
			 * over a modem or something stranger...
			 */
#ifdef MSG
			fprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_NEWLINE, "\n"));
#else
			fprintf(stdout, "\n");
#endif
			timer(2);

			flag = TRUE;
			while (flag) {
				/* Now read in the user response. */
#ifdef MSG
				NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_LVLPROMPT, "Enter run level (0-9, s or S): "));
#else
				NLfprintf(stdout, "Enter run level (0-9, s or S): ");
#endif
				fflush(stdout);
				/*
				 * Get a character from the user which isn't
				 * a space, tab or <cr>. 
				 */
				while ((fscanf(stdin, "%c", &c) != 1)
				|| (c == '\n') || (c == '\t') || (c == ' '));
				c &= 0x7f;
				/*
				 * If the character is a digit between 0 and
				 * 9 or the letter S,  exit with the level
				 * equal to the new desired state. 
				 */
				if (c >= '0' && c <= '9') {
					if(check_lvl(c - '0') < 0) {
#ifdef MSG
						NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_CHANGE,
							 "Run level %c not configured\n"), c);
#else
						NLfprintf(stdout, "Run level %c not configured\n", c);
#endif
						while ((fscanf(stdin, "%c", &c) != 1) || (c != '\n'))
							continue;
						continue;
					}
#ifdef MSG
					NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_CHANGE,
					   "will change to state %c\n"), c);
#else
					NLfprintf(stdout, "will change to state %c\n", c);
#endif
					exit(levels[c - '0']);
				}
				else if (c == 'S' || c == 's') {
#ifdef MSG
					NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_CHANGE,
					   "will change to state %c\n"), c);
#else
					NLfprintf(stdout, "will change to state %c\n", c);
#endif
					exit(levels[10]);
				}
				else {
#ifdef MSG
					NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_BADCHAR,
					 "\nbad character <%3.3o>\n\n"), c);
#else
					NLfprintf(stdout, "\nbad character <%3.3o>\n\n", c);
#endif
					while ((fscanf(stdin, "%c", &c) != 1) || (c != '\n'))
						continue;
				}
			}
		}
	}
	/* Wait for the child to die and return its status. */
	while (wait(&status) == -1);

/*
 * restore sigchild to original operation - to be caught
 * by childeath - burns 7-12-91
 */
	sigaction(SIGCHLD, &oaction, (struct sigaction *) NULL);

	/* Return the new run level to the caller. */
#ifdef XDEBUG
	debug("getlvl: status: %o exit: %o termination: %o\n", status, (status & 0xff00) >> 8, (status & 0xff));
#endif
	return ((status & 0xff00) >> 8);
}

/*********************/
/****    efork    ****/
/*********************/

/*
 *	"efork" forks a child and the parent inserts the process in
 *	its table of processes that are directly a result of forks
 *	that it has performed.  The child just changes the "global"
 *	with the process id for this process to its new value.
 *
 *	If "efork" is called with a pointer into the proc_table
 *	it uses that slot, otherwise it searches for a free slot.
 *	Whichever way it is called, it returns the pointer to the
 *	proc_table entry.
 */
struct proc *
efork(procp, modes)
	register struct proc *procp;
	int             modes;
{
	register pid_t  childpid;
	register struct proc *p, *nextp;
	int             i;
	extern char    *vetchar();
	extern int      childeath(void);
	extern char *calloc();
	struct sigaction action, oaction;
	sigset_t	sigset_chld, old_mask;
#ifdef	UDEBUG
	static void      (*oldsigs[NPROC]) ();

#endif

#ifdef	XDEBUG
	debug("We have entered efork().\n");
#endif
	/*
	 * Don't accept child death signals until the process database
	 * is consistent.
	 */
	sigemptyset(&sigset_chld);
	sigaddset(&sigset_chld, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigset_chld, &old_mask);

	/*
	 * Freshen up the proc_table, removing any entries for dead processes
	 * that don't have the NOCLEANUP set.  Perform the necessary
	 * accounting. 
	 */
	for (p = proc_table; p != NULL; ) {
		nextp = p->p_next;
		if (p != procp && (p->p_flags & (LIVING | NOCLEANUP)) == 0) {
#ifdef	XDEBUG
			debug("efork- id:%s pid: %d time: %lo %d %o %o\n",
			      vetchar(p->p_id), p->p_pid, p->p_time,
			      p->p_count, p->p_flags, p->p_exit);
#endif
			/*
			 * Is this a named process?  If so, do the necessary
			 * bookkeeping. 
			 */
			if (p->p_flags & NAMED)
				account(DEAD_PROCESS, p, NULL);
			freeproc(p);
		}
		p = nextp;
	}

	while ((childpid = fork()) == FAILURE) {
		/*
		 * Shorten the alarm timer in case someone else's child dies
		 * and free up a slot in the process table. 
		 */
		setimer(5);
		/*
		 * Wait for some children to die.
		 */
		sigsuspend(&emptysigs);
		setimer(0);
	}

	if (childpid != 0) {	/* parent */
		/*
		 * If a pointer into the process table was not specified,
		 * then search for one. 
		 */
		if (procp == NULLPROC) {
			procp = (struct proc *)calloc(1, sizeof(*procp));
			if (procp == NULL) {
				if (error_time(FULLTABLE))
#ifdef MSG
					console(NLgetamsg(MF_INIT, MS_INIT, M_PTAB,
							  "Internal process table is full.\n"));
#else
					console("Internal process table is full.\n");
#endif
				sigprocmask(SIG_SETMASK, &old_mask, NULL);
				return (NO_ROOM);
			}
			if (proc_table != NULL)
				proc_table->p_prev = procp;
			procp->p_next = proc_table;
			proc_table = procp;
			procp->p_time = 0;
			procp->p_count = 0;
		}
		bzero(procp->p_id, IDENT_LEN);
		procp->p_pid = childpid;
		procp->p_flags = (LIVING | modes);
		procp->p_exit = 0;
	}
	else {			/* child */
		/* Reset child's concept of its own process id.  */
		own_pid = getpid();
		/* start session and become process group leader */
		if (setsid() < 0)
			console("efork: setsid() failed errno = %d\n", errno);
		procp = NULLPROC;

		/* Reset all signals to the system defaults. */
		action.sa_handler = SIG_DFL;
		action.sa_mask = 0;
		action.sa_flags = SA_RESTART;
		for (i = SIGHUP; i <= SIGMAX; i++) {
			sigaction(i, &action, &oaction);
#ifdef	UDEBUG
/* BVT bugfix by changing oldsigs[] to oldsigs[] */
			oldsigs[i] = oaction.sa_handler;
#endif
		}
	}
	sigprocmask(SIG_SETMASK, &old_mask, NULL);
	return (procp);
}

/************************/
/****    waitproc    ****/
/************************/

/*
 * "waitproc" waits for a specified process to die.  For this
 * routine to work, the specified process must already be in
 * the proc_table.  "waitproc" returns the exit status of the
 * specified process when it dies.
 */
waitproc(p)
	register struct proc *p;
{
	int             answer;
	sigset_t        sigmsk;
	sigset_t        osigmsk;

#ifdef	XDEBUG
	struct proc *xp;
	debug("We have entered waitproc(id= %s, pid= %d, flags= %o).\n",
					p->p_id, p->p_pid, p->p_flags);
	for (xp = proc_table; xp != NULL; xp = xp->p_next)
		debug("\tproc %d id= %s\n", xp->p_pid, xp->p_id);
#endif

	/* make available to siglvl() */
	cur_pid = p->p_pid;

	/* Wait around until the process dies. */
	sigmsk = emptysigs;
	sigaddset(&sigmsk, SIGCHLD);
	sigprocmask(SIG_BLOCK, &sigmsk, &osigmsk);
	if (p->p_flags & LIVING)
		sigsuspend(&emptysigs);
	if (p->p_flags & LIVING)
		answer = FAILURE;
	else
		/*
		 * Make sure to only return 16 bits so that answer will always
		 * be positive whenever the process of interest really died.
		 */
		answer = (p->p_exit & 0xffff);

	sigprocmask(SIG_SETMASK, &osigmsk, NULL);
	return (answer);
}

/***********************/
/****    account    ****/
/***********************/

/*	"account" updates entries in /var/adm/utmp and appends new entries	*/
/*	to the end of /var/adm/wtmp (assuming /var/adm/wtmp exists).		*/

account(state, process, program)
	int             state;
	register struct proc *process;
	char           *program;/* Program Name in the case of INIT_PROCESSes
				 * else NULL */
{
	extern          cur_state;
	struct utmp     utmpbuf;
	register struct utmp *uptr, *oldu;
	extern struct utmp *getutid(), *pututline();
	extern char    *WTMP;
	FILE           *fp;
	int		fd, wfd;
	int             uopen_flags;
	char            level();
	char            utmplock[84];	/* utmp lock pathname */
	static struct ulist {
		struct ulist *ul_next;
		struct utmp  ul_data;
	} *savedut;
	static int 	trunc_flag = 0;

#ifdef	ACCTDEBUG
	extern char    *vetchar();
	debug("** account ** state: %d id:%s\n", state, vetchar(process->p_id));
#endif

#ifdef	XDEBUG
	debug("We have entered account(%d).\n", state);
#endif
	/* Set up the prototype for the utmp structure we want to write. */
	uptr = &utmpbuf;
	memset(uptr->ut_user, 0, sizeof(uptr->ut_user));
	memset(uptr->ut_line, 0, sizeof(uptr->ut_line));
	memset(uptr->ut_host, 0, sizeof(uptr->ut_host));

	/* Fill in the various fields of the utmp structure. */
	strncpy(uptr->ut_id, process->p_id, IDENT_LEN);
	uptr->ut_pid = process->p_pid;

	/* Fill the "ut_exit" structure. */
	uptr->ut_exit.e_termination = (process->p_exit & 0xff);
	uptr->ut_exit.e_exit = ((process->p_exit >> 8) & 0xff);
	uptr->ut_type = state;
	time(&uptr->ut_time);

	/* See if there already is such an entry in the "utmp" file. */
	setutent();		/* Start at beginning of utmp file. */
	if ((oldu = getutid(uptr)) != NULL) {
		/*
		 * Copy in the old "user" and "line" fields to our new
		 * structure. 
		 */
		memcpy(uptr->ut_user, oldu->ut_user, sizeof(uptr->ut_user));
		memcpy(uptr->ut_line, oldu->ut_line, sizeof(uptr->ut_line));
#ifdef	ACCTDEBUG
		debug("New entry in utmp file.\n");
#endif
	}
#ifdef	ACCTDEBUG
	else
		debug("Replacing old entry in utmp file.\n");
#endif
	/*
	 * Perform special accounting. Insert the special string into the
	 * ut_line array. For INIT_PROCESSes put in the name of the program
	 * in the "ut_user" field. 
	 */
	switch (state) {
	case RUN_LVL:
		uptr->ut_exit.e_termination = level(cur_state);
		uptr->ut_exit.e_exit = level(prior_state);
		uptr->ut_pid = n_prev[cur_state];
		sprintf(uptr->ut_line, RUNLVL_MSG, level(cur_state));
		break;
	case BOOT_TIME:
		strncpy(uptr->ut_line, BOOT_MSG, 12);
	/*	
	 * set to cause zeroing of /var/adm/utmp on bootup for new connect session. 
	 */
		trunc_flag = 1; 
		break;
	case INIT_PROCESS:
		strncpy(uptr->ut_user, program, sizeof(uptr->ut_user));
		break;
	case DEAD_PROCESS:
#if SEC_BASE
#if SEC_WIS
                killsession(process->p_pid, SIGKILL);
#endif
                init_update_tty(oldu);
#endif
		bzero(uptr->ut_user, sizeof(uptr->ut_user));
		break;
	default:
		break;
	}

	/*
	 * Set flags to open /var/adm/utmp at zero length during bootup.
	 * If /var/adm mounted but /var/adm/utmp does not exist, create it.
	 */ 

	uopen_flags =  (trunc_flag  ? O_CREAT | O_TRUNC | O_RDWR :
			O_CREAT | O_RDWR); 

	/*
	 * Write out the updated entry to utmp file, if it exists.
	 * If it doesn't exist, assume that it is on a file system
	 * that hasn't been mounted yet, and save the entry on a
	 * list, to be written out as soon as the utmp file appears.
	 */
	wfd = -1;
	if (!utmp_exists) {
		struct ulist *ulp;

		if ((fd = open(UTMP, uopen_flags, 0644)) < 0) {
			struct ulist *upp;
			extern char *malloc();

#ifdef XDEBUG
			debug("No utmp file yet\n");
#endif
			/* Still doesn't exist */
			ulp = (struct ulist *)malloc(sizeof(struct ulist));
			memcpy(&ulp->ul_data, uptr, sizeof(struct utmp));
			/* Insert at END of list */
			if (savedut == NULL)
				savedut = ulp;
			else for (upp = savedut; upp != NULL; upp = upp->ul_next)
				if (upp->ul_next == NULL) {
					upp->ul_next = ulp;
					break;
				}
			ulp->ul_next = NULL;
		}
		else {
			trunc_flag = 0;
#ifdef XDEBUG
			debug("utmp file exists\n");
#endif
			close(fd);
			sprintf(utmplock, "%s.lck", UTMP);
			unlink(utmplock);
			/* Write out all the saved entries */
			wfd = open(WTMP, O_WRONLY|O_APPEND);
			for (ulp = savedut; ulp != NULL; ulp = ulp->ul_next) {
				pututline(&ulp->ul_data);
				if (wfd != -1)
					write(wfd, (char *)&ulp->ul_data, sizeof(struct utmp));
			}
			for (ulp = savedut; ulp != NULL; ulp = ulp->ul_next)
				free(ulp);
			savedut = NULL;
			utmp_exists = 1;
		}
	}
	if (utmp_exists && pututline(uptr) == NULL)
#ifdef MSG
		console(NLgetamsg(MF_INIT, MS_INIT, M_UTMP,
				  "failed write of utmp entry: \"%2.2s\"\n"),
			uptr->ut_id);
#else
		console("failed write of utmp entry: \"%2.2s\"\n", uptr->ut_id);
#endif
        endutent();
	/*
	 * Now attempt to add to the end of the wtmp file.  Do not create if
	 * it doesn't already exist.
	 */
	if (wfd == -1 && (wfd = open(WTMP, O_WRONLY|O_APPEND)) < 0)
		return;
	write(wfd, (char *)uptr, sizeof(struct utmp));
	(void) close(wfd);
}

/**************************/
/****    error_time    ****/
/**************************/

/*
 * "error_time" keeps a table of times, one for each time of
 * error that it handles.  If the current entry is 0 or the
 * elapsed time since the last error message is large enough,
 * "error_time" returns TRUE, otherwise it returns FALSE.
 */
error_time(type)
	register int    type;
{
	time_t          curtime;
	extern struct ERRORTIMES err_times[];

#ifdef	XDEBUG
	debug("We have entered error_time().\n");
#endif
	time(&curtime);
	if (err_times[type].e_time == 0
	    || (curtime - err_times[type].e_time >= err_times[type].e_max)) {
		err_times[type].e_time = curtime;
		return (TRUE);
	}
	else
		return (FALSE);
}


/************************/
/****    userinit    ****/
/************************/

/*
 * Routine to handle requests from users to main init running as
 * process 1.
 */
userinit(argc, argv)
	int             argc;
	char          **argv;
{
	FILE           *fp;
	char           *ln;
	extern char    *ttyname();
	int             init_signal;

	/* We are a user invoked init.  Is there an argument and is it */
	/* a single character?  If not, print usage message and quit. */
#ifdef	XDEBUG
	debug("We have entered userinit().\n");
#endif

#if SEC_BASE
        set_auth_parameters(argc, argv);
        initprivs();

        if (!authorized_user("sysadmin")) {
#ifdef MSG
                NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT_SEC, M_NOTAUTH,
                       "Need sysadmin authorization to change run-level.\n"));
#else
                NLfprintf(stderr,
                        "Need sysadmin authorization to change run-level.\n");
#endif
                exit(1);
        }
#endif /* SEC_BASE */


	if (argc != 2 || *(*++argv + 1) != '\0') {
#ifdef MSG
		NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT, M_USAGE,
				    "Usage: init [0123456789SsMmQqabc]\n"));
#else
		NLfprintf(stderr, "Usage: init [0123456789SsMmQqabc]\n");
#endif
		exit(1);
	}
	else
		switch (**argv) {
		case 'Q':
		case 'q':
			init_signal = LVLQ;
			break;
		case '0':
			init_signal = LVL0;
			break;
		case '1':
			init_signal = LVL1;
			break;
		case '2':
			init_signal = LVL2;
			break;
		case '3':
			init_signal = LVL3;
			break;
		case '4':
			init_signal = LVL4;
			break;
		case '5':
			init_signal = LVL5;
			break;
		case '6':
			init_signal = LVL6;
			break;
		case '7':
			init_signal = LVL7;
			break;
		case '8':
			init_signal = LVL8;
			break;
		case '9':
			init_signal = LVL9;
			break;
		case 'S':
		case 's':
		case 'M':
		case 'm':
			ln = ttyname(0);	/* Get the name of tty */
			if (ln == (char *)0 || *ln == '\0') {
#ifdef MSG
				NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT, M_NOTTY,
					"Standard input not a tty line\n"));
#else
				NLfprintf(stderr, "Standard input not a tty line\n");
#endif
				exit(1);
			}
			if (strcmp(ln, CONSOLE) != 0) {
#ifdef MSG
				NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT, M_MOVCNTL,
				    "Moving control to CONSOLE device.\n"));
#else
				NLfprintf(stderr, "Moving control to CONSOLE device.\n");
#endif
				if ((fp = fopen(CONSOLE, "r+")) != NULL) {
#ifdef MSG
					NLfprintf(fp, NLgetamsg(MF_INIT, MS_INIT, M_SUENTER,
								"\n**** SINGLE-USER MODE ENTERED FROM %s ****\n"), ln);
#else
					NLfprintf(fp, "\n**** SINGLE-USER MODE ENTERED FROM %s ****\n", ln);
#endif
					fclose(fp);
				}
			}
			init_signal = SINGLE_USER;
			break;

		case 'a':
			init_signal = LVLa;
			break;
		case 'b':
			init_signal = LVLb;
			break;
		case 'c':
			init_signal = LVLc;
			break;

			/*
			 * If the argument was invalid, print the usage
			 * message. 
			 */
		default:
#ifdef MSG
			NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT, M_USAGE,
				    "Usage: init [0123456789SsMmQqabc]\n"));
#else
			NLfprintf(stderr, "Usage: init [0123456789SsMmQqabc]\n");
#endif
			exit(1);
		}

#if SEC_BASE
        if (forceprivs(privvec(SEC_KILL,
#if SEC_MAC
                                SEC_ALLOWMACACCESS,
#endif
                                -1), (priv_t *) 0)) {
#ifdef MSG
                NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT_SEC, M_INSUFFPRIV,
                        "init: insufficient privileges\n"));
#else
                NLfprintf(stderr, "init: insufficient privileges\n");
#endif
        }
#endif /* SEC_BASE */

	/* Now send signal to main init and then exit. */
	if (kill(SPECIALPID, init_signal) == FAILURE) {
#ifdef MSG
		NLfprintf(stderr, NLgetamsg(MF_INIT, MS_INIT, M_NOSIG,
				   "Could not send signal to \"init\".\n"));
#else
		NLfprintf(stderr, "Could not send signal to \"init\".\n");
#endif
		exit(1);
	}
	else
		exit(0);
}

#if SEC_BASE
/***************************/
/****    sub_process    ****/
/***************************/

/*
 * Spawn the init helper program to perform the requested function.
 * This hook depends completely on the underlying implementation of init,
 * and is therefore placed directly in the vendor code.  It should
 * mirror whatever the vendor code does to create a process and
 * wait for its completion.  If open_console is true, the child
 * process should open the console device and make the console its
 * controlling tty by whatever means the system uses to do that.
 */

sub_process(arg2, arg3, open_console)
	char *arg2, *arg3;
	int open_console;
{
	struct proc *p;
	extern int      childeath(void);
#ifdef	UDEBUG
	extern int      abort();
	struct sigaction action;
#endif
	extern char    *strrchr();

#ifdef	XDEBUG
	debug("We have entered sub_process().\n");
#endif
	while ((p = efork(NULLPROC, NOCLEANUP)) == NO_ROOM)
		timer(5);
	if (p == NULLPROC) {
		char *cp;
#ifdef	UDEBUG
		action.sa_handler = (void (*) (int)) abort;
		sigaction(SIGUSR1, &action, (struct sigaction *) NULL);
		sigaction(SIGUSR2, &action, (struct sigaction *) NULL);
#endif
		if (open_console) {
			openconsole();
			/*
			 * Set controlling tty.
			 */
			ioctl(0, TIOCSCTTY, (char *) 0);
		}
		cp = strrchr(INITCOND_PROGRAM, '/');
		if (cp)
			cp++;
		else
			cp = INITCOND_PROGRAM;

		execl(INITCOND_PROGRAM, cp, "init", arg2, arg3, NULL);
		exit(0);
	}
	else {
		while (waitproc(p) == FAILURE);
		freeproc(p);
	}
}

#endif /* SEC_BASE */

/***************************/
/****    check_lvl	****/
/***************************/
/*
 * Receive input from user in single user mode via getlvl(). Check 0-9 
 * response against /etc/inittab file for an entry representing a run
 * level script of the form /rc[0-9]. If an entry can be found matching
 * the user's input then return 0 otherwise return -1.
 */

check_lvl(new_level)
{
	char *cp, *lptr;
	FILE *fp;
	char line[MAXLN];
	extern char *INITTAB;

	if( (fp = fopen(INITTAB,"r")) == 0) {
#ifdef MSG
		NLfprintf(stdout, NLgetamsg(MF_INIT, MS_INIT, M_CHANGE,
		"Can't open %s\n"), INITTAB);
#else
		NLfprintf(stdout, "Can't open %s\n", INITTAB);
#endif
	} else {
		while ((lptr = fgets(line, MAXLN, fp)) != (char *)NULL) {
			if(*lptr == '#') {
                                continue;
                        }
                        if ((cp = (char *)strstr(lptr, "/rc")) != (char *)NULL) {
                                cp += 3;
                                if(*cp >= '0' && *cp <= '9') {
	                        	if(new_level == *cp - '0') {
						cp++;
						if(*cp == ' ') { 
						    fclose(fp);
	                                            return 0;
	                                        }
					}
                                }
                        }
                 } 
	}
	fclose(fp);
	return -1;

}	
