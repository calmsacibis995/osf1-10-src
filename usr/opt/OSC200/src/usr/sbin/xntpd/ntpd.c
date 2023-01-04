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
static char     *sccsid = "@(#)$RCSfile: ntpd.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1992/05/04 16:58:55 $";
#endif
/*
 * ntpd.c - main program for the fixed point NTP daemon
 */
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/in.h>

#include <ntp/ntp_syslog.h>
#include <ntp/ntp_fp.h>
#include <ntp/ntp.h>

#if defined(ULT_2_0)
#ifndef sigmask
#define	sigmask(m)	(1<<(m))
#endif
#endif

/*
 * Mask for blocking SIGIO and SIGALRM
 */
#define	BLOCKSIGMASK	(sigmask(SIGIO)|sigmask(SIGALRM))

/*
 * Signals we catch for debugging.  If not debugging we ignore them.
 */
#define	MOREDEBUGSIG	SIGUSR1
#define	LESSDEBUGSIG	SIGUSR2

/*
 * Signals which terminate us gracefully.
 */
#define	SIGDIE1		SIGHUP
#define	SIGDIE2		SIGINT
#define	SIGDIE3		SIGQUIT
#define	SIGDIE4		SIGTERM

/*
 * Scheduling priority we run at
 */
#define	NTPD_PRIO	(-12)

/*
 * Debugging flag
 */
int debug;

/* 
 * Boottime flag.  When this is set, xnptd will initialize the time
 * before running.
 */
int boottime=0;
int count_peers=0;
#define MAX_NTPDATE 3
char *save_peeraddr[MAX_NTPDATE];

/* -s, -x, -g, -l -n flags */
int dont_change_time = FALSE;
int allow_set_backward = FALSE;
int correct_any=FALSE;
int limit_log=FALSE;
int no_log=FALSE;

/*
 * Initializing flag.  All async routines watch this and only do their
 * thing when it is clear.
 */
int initializing;

/*
 * Version declaration
 */
extern char *Version;

/*
 * Alarm flag.  Imported from timer module
 */
extern int alarm_flag;


/*
 * Main program.  Initialize us, disconnect us from the tty if necessary,
 * and loop waiting for I/O and/or timer expiries.
 */
main(argc, argv)
	int argc;
	char *argv[];
{
	char *cp;
	int was_alarmed;
	struct recvbuf *rbuflist;
	struct recvbuf *rbuf;
	extern struct recvbuf *getrecvbufs();
	extern void receive();
	extern void getstartup();
	extern char *rindex();
#ifdef DEBUG
	void moredebug(), lessdebug();
#endif
#ifdef SIGDIE1
	void finish();
#endif	/* SIGDIE1 */

	initializing = 1;	/* mark that we are initializing */
	debug = 0;		/* no debugging by default */

	getstartup(argc, argv);	/* startup configuration, may set debug */
	if (boottime)
	  init_time();

#ifndef NODETACH
	/*
	 * Detach us from the terminal.  May need an #ifndef GIZMO.
	 */
#ifdef	DEBUG
	if (!debug) {
#endif
	  if (fork())
	    exit(0);

		{
			int s;
			for (s = getdtablesize(); s >= 0; s--)
				(void) close(s);
			(void) open("/", 0);
			(void) dup2(0, 1);
			(void) dup2(0, 2);
			/* (void) setpgrp(0, getpid()); */
			s = open("/dev/tty", 2);
			if (s >= 0) {
				(void) ioctl(s, (u_int) TIOCNOTTY, (char *) 0);
				(void) close(s);
			}
		}
#ifdef	DEBUG
	}
#endif
#endif /* NODETACH */

	/*
	 * Logging.  This may actually work on the gizmo board.  Find a name
	 * to log with by using the basename of argv[0]
	 */
	cp = rindex(argv[0], '/');
	if (cp == 0)
		cp = argv[0];
	else
		cp++;

#ifndef	LOG_DAEMON
	openlog(cp, LOG_PID);
#else

#ifndef	LOG_NTP
#define	LOG_NTP	LOG_DAEMON
#endif
	openlog(cp, LOG_PID | LOG_NDELAY, LOG_NTP);
#ifdef	DEBUG
	if (debug)
		setlogmask(LOG_UPTO(LOG_DEBUG));
	else
#endif	/* DEBUG */
		setlogmask(LOG_UPTO(LOG_INFO));
#endif	/* LOG_DAEMON */
	
	syslog(LOG_INFO, Version);


	/*
	 * Set the priority.
	 */
#if defined(NTPD_PRIO) && NTPD_PRIO != 0
	(void) setpriority(PRIO_PROCESS, 0, NTPD_PRIO);
#endif	/* ... */

	/*
	 * Set up signals we pay attention to locally.
	 */
#ifdef SIGDIE1
	(void) signal(SIGDIE1, finish);
#endif	/* SIGDIE1 */
#ifdef SIGDIE2
	(void) signal(SIGDIE2, finish);
#endif	/* SIGDIE2 */
#ifdef SIGDIE3
	(void) signal(SIGDIE3, finish);
#endif	/* SIGDIE3 */
#ifdef SIGDIE4
	(void) signal(SIGDIE4, finish);
#endif	/* SIGDIE4 */

#ifdef DEBUG
	(void) signal(MOREDEBUGSIG, moredebug);
	(void) signal(LESSDEBUGSIG, lessdebug);
#else
	(void) signal(MOREDEBUGSIG, SIG_IGN);
	(void) signal(LESSDEBUGSIG, SIG_IGN);
#endif 	/* DEBUG */

	/*
	 * Call the init_ routines to initialize the data structures.
	 * Note that init_systime() may run a protocol to get a crude
	 * estimate of the time as an NTP client when running on the
	 * gizmo board.  It is important that this be run before
	 * init_subs() since the latter uses the time of day to seed
	 * the random number generator.  That is not the only
	 * dependency between these, either, be real careful about
	 * reordering.
	 */
#ifdef DES_OK
	init_auth();
#endif /* DES_OK */
	init_util();
	init_restrict();
	init_loopfilter();
	init_mon();
	init_systime();
	init_timer();
	init_lib();
	init_random();
	init_request();
	init_leap();
	init_peer();
#ifdef REFCLOCK
	init_refclock();
#endif
	init_proto();
	init_io();


	/*
	 * Get configuration.  This (including argument list parsing) is
	 * done in a separate module since this will definitely be different
	 * for the gizmo board.
	 */
	getconfig(argc, argv);
	
	initializing = 0;

	/*
	 * Report that we're up to any trappers
	 */
	report_event(EVNT_SYSRESTART, (struct peer *)0);

	/*
	 * Done all the preparation stuff, now the real thing.  We block
	 * SIGIO and SIGALRM and check to see if either has occured.
	 * If not, we pause until one or the other does.  We then call
	 * the timer processing routine and/or feed the incoming packets
	 * to the protocol module.  Then around again.
	 */
	was_alarmed = 0;
	rbuflist = (struct recvbuf *)0;
	for (;;) {
		int omask;

		omask = sigblock(BLOCKSIGMASK);
		if (alarm_flag) {		/* alarmed? */
			was_alarmed = 1;
			alarm_flag = 0;
		}
		rbuflist = getrecvbufs();	/* get received buffers */

		if (!was_alarmed && rbuflist == (struct recvbuf *)0) {
			/*
			 * Nothing to do.  Wait for something.
			 */
			sigpause(omask);
			if (alarm_flag) {		/* alarmed? */
				was_alarmed = 1;
				alarm_flag = 0;
			}
			rbuflist = getrecvbufs();  /* get received buffers */
		}
		(void)sigsetmask(omask);

		/*
		 * Out here, signals are unblocked.  Call timer routine
		 * to process expiry.
		 */
		if (was_alarmed) {
			timer();
			was_alarmed = 0;
		}

		/*
		 * Call the data procedure to handle each received
		 * packet.
		 */
		while (rbuflist != (struct recvbuf *)0) {
			rbuf = rbuflist;
			rbuflist = rbuf->next;
			(rbuf->receiver)(rbuf);
			freerecvbuf(rbuf);
		}
		/*
		 * Go around again
		 */
	}
}


#ifdef SIGDIE1
/*
 * finish - exit gracefully
 */
void
finish()
{
	struct timeval tv;
	extern int dont_change_time;

	/*
	 * The only thing we really want to do here is make sure
	 * any pending time adjustment is terminated, as a bug
	 * preventative.  Also log any useful info before exiting.
	 */
	tv.tv_sec = tv.tv_usec = 0;
	if (!dont_change_time)
	  (void) adjtime(&tv, (struct timeval *)0);

#ifdef notdef
	log_exit_stats();
#endif
	exit(0);
}
#endif	/* SIGDIE1 */


#ifdef DEBUG
/*
 * moredebug - increase debugging verbosity
 */
void
moredebug()
{
	if (debug < 255)
		debug++;
}


/*
 * lessdebug - decrease debugging verbosity
 */
void
lessdebug()
{
	if (debug > 0)
		debug--;
}
#endif	/* DEBUG */
