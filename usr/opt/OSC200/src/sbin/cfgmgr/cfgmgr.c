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
static char	*sccsid = "@(#)$RCSfile: cfgmgr.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1992/05/05 13:47:05 $";
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
 *  cfgmgr.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */

#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <loader.h>
#include <AFdefs.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <locale.h>

#include "cfgmgr.h"
#include "cm.h"

extern int flock ();

/*
 *	CFGMGR:	static global data
 */
cmgr_common_t	CMGR;
void		cmgr_reset();
void		cmgr_mainexit();
void		cmgr_client_state();


/*
 *	CFGMGR:	command line usage
 */
void
cmgr_usage()
{
	fprintf(stderr, cm_msg(MSG_CFGMGR), CMGR.progname);
	exit(2);
}


void
cmgr_init(pname)
	char *	pname;
{
	/*
	 *	Initialize global data
	 */
        CMGR.progname = strrchr(pname,'/') ? strrchr(pname,'/') +1: pname;
	CMGR.database	= CFGMGR_DATABASE;
	CMGR.maxrec	= CFGMGR_MAXRECSIZ;
	CMGR.maxatr	= CFGMGR_MAXATRNUM;
	CMGR.dflg	= FALSE;
	CMGR.vflg	= FALSE;
	CMGR.fflg	= FALSE;
	CMGR.lflg	= FALSE;
	CMGR.reset	= FALSE;
	CMGR.exitval	= 0;
	CMGR.pidfd	= -1;
}

/*
 *	CFGMGR:	option parsing and initialization
 */
void
cmgr_options(argc, argv)
	int	argc;
	char *	argv[];
{
	register int	c;
	register int	i;
	extern char *	optarg;
	extern int	optind;

	while ((c = getopt(argc, argv, "flvdc:R:A:")) != EOF) {
		switch (c) {
		case 'f':
			CMGR.fflg = TRUE;
			break;
		case 'l':	/* load automatics */
			CMGR.lflg = TRUE;
			break;
		case 'v':
			CMGR.vflg = TRUE;
			break;
		case 'd':
			CMGR.dflg = TRUE;
			break;
		case 'c':
			CMGR.database = optarg;
			break;
		case 'R':
			if ((i=atoi(optarg)) > CFGMGR_MAXRECSIZ)
				CMGR.maxrec = i;
			break;
		case 'A':
			if ((i=atoi(optarg)) > CFGMGR_MAXATRNUM)
				CMGR.maxatr = i;
			break;
		case '?':
		default:
			cmgr_usage();
		}
	}
	if (optind != argc)
		cmgr_usage();
}


/*
 *	CFGMGR:	error log
 */
void
cfgmgr_log( int level, char * format, ...)
{
	va_list 	args;
        char    	message[BUFSIZ +1];
	static int	syslog_opened = FALSE;


	if (!syslog_opened) {
		openlog(CFGMGR_SYSLOG,CMGR.dflg ? (LOG_PID|LOG_CONS) : LOG_PID, 
			LOG_DAEMON);
		syslog_opened = TRUE;
	}

	va_start(args, format);
	vsprintf(message, format, args);
	va_end(args);

	if (CMGR.fflg && CMGR.dflg && stderr != NULL) {
		fprintf(stderr, message);
		fflush(stderr);
	}

	syslog(level, message);
}


/*
 *	CFGMGR:	lock pid run file
 *
 *	The lock file is open exclusive which limits us to one 
 *	cfgmgr running at a time.
 */
void
cmgr_lock_pid_file()
{
	struct flock flock;

	/*
 	 *	Open pid file
	 */
	if ((CMGR.pidfd = open(CMGR_PID_PATHNAME, (O_CREAT|O_WRONLY),
		0644)) == -1) {
		cfgmgr_log(LOG_ERR, "%s: %s: %s\n", CMGR.progname,
			CMGR_PID_PATHNAME, strerror(errno));
		CMGR.exitval = 1;
		cmgr_mainexit();
	}

	/*
 	 *	See if the lock is taken
	 */
	flock.l_type = F_WRLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;

	if (fcntl(CMGR.pidfd, F_GETLK, &flock) == -1) {
		cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.progname,
			cm_msg(MSG_ALREADYRUNNING));
		(void) close(CMGR.pidfd);
		CMGR.pidfd = -1;
		CMGR.exitval = 1;
		cmgr_mainexit();
	}
	if (flock.l_type != F_UNLCK) {
		cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.progname,
			cm_msg(MSG_ALREADYRUNNING));
		(void) close(CMGR.pidfd);
		CMGR.pidfd = -1;
		CMGR.exitval = 1;
		cmgr_mainexit();
	}
}


/*
 *	CFGMGR: Write PID to pid file (warning if we can't write PID out)
 *	Also take an exclusive lock.
 */
void
cmgr_write_pid_file()
{
	struct flock flock;
	char buf[32];

	/*
 	 *	Exclusive lock pid file
	 */
	flock.l_type = F_WRLCK;
	flock.l_whence = SEEK_SET;
	flock.l_start = 0;
	flock.l_len = 0;
	flock.l_pid = getpid();

	if (fcntl(CMGR.pidfd, F_SETLK, &flock) == -1) {
		cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.progname,
			cm_msg(MSG_ALREADYRUNNING));
		(void) close(CMGR.pidfd);
		CMGR.pidfd = -1;
		CMGR.exitval = 1;
		cmgr_mainexit();
	}


	/* convert pid to ascii .... */
	(void)sprintf(buf, "%d\n", (int) getpid());

	/* ... and write it */
	if (write(CMGR.pidfd, buf, strlen(buf)) == -1) {
		cfgmgr_log(LOG_ERR, "%s: %s: %s\n", CMGR.progname,
			CMGR_PID_PATHNAME, strerror(errno));
	}
}


/*
 *	CFGMGR:	unlock pid run file, if we have it locked (opened)
 */
void
cmgr_unlock_pid_file()
{
	if (CMGR.pidfd >= 0) {
		(void) close(CMGR.pidfd);
		(void) unlink(CMGR_PID_PATHNAME);
	}
}


/*
 *	CFGMGR:	check priv
 */
cmgr_check_priv()
{
        if (getuid() && geteuid()) {
		fprintf(stderr, "%s: %s\n", CMGR.progname,
			cm_msg(MSG_MUSTBEROOT));
                exit(1);
        }
}




/*
 *	CFGMGR:	detach from controlling terminal
 */
void
cmgr_detach()
{
	register int	f;

	cfgmgr_log(LOG_INFO, cm_msg(MSG_DETACHING), CMGR.progname); 

	if (fork())
		exit(0);

	cmgr_write_pid_file();

	(void)setsid();

	for (f = 0; f < 3; f++)
		(void) close(f);
	(void) open("/dev/null", O_RDONLY);
	(void) open("/dev/null", O_WRONLY);
	(void) dup(1);
}


/*
 *	CFGMGR:	main exit point
 */
void
cmgr_mainexit()
{
	cfgmgr_log(LOG_ERR, "%s: %s\n", CMGR.progname, cm_msg(MSG_EXITING));
	cmgr_unlock_pid_file();
	exit(CMGR.exitval);
}

/*
 *	CFGMGR:	reset point
 */
void
cmgr_reset()
{
	CMGR.reset = TRUE;
	return;
}

/*
 *	CFGMGR:	client has gone away
 */
void
cmgr_client_state()
{
	CMGR.client = FALSE;
	return;
}

/*
 *	CFGMGR:	main entry point
 */
main(argc, argv)
	int		argc;
	char *		argv[];
{
	int			rc;
	struct sigaction	sigact_reset;
	struct sigaction	sigact_abort;
	struct sigaction	sigact_pipe;
	extern void		startup_mode();
	extern void		request_mode();

	cmgr_init(argv[0]);

	setlocale(LC_ALL, "");

	cmgr_check_priv();

	cmgr_options(argc, argv);

	if(CMGR.lflg)
		sleep(3);

	cmgr_lock_pid_file();

	/*
	 *	Set to reread database on receit of SIGHUP
	 */
	sigact_reset.sa_handler = (void (*)())&cmgr_reset;
	sigfillset(&sigact_reset.sa_mask);
	sigact_reset.sa_flags = 0;
	sigaction(SIGHUP, &sigact_reset, (struct sigaction *)NULL);

	/*
	 *	Abort process on receit of SIGINT or SIGQUIT
	 */
	sigact_abort.sa_handler = (void (*)())&cmgr_mainexit;
	sigfillset(&sigact_abort.sa_mask);
	sigact_abort.sa_flags = 0;
	sigaction(SIGTERM, &sigact_abort, (struct sigaction *)NULL);
	sigaction(SIGINT, &sigact_abort, (struct sigaction *)NULL);
	sigaction(SIGQUIT, &sigact_abort, (struct sigaction *)NULL);

	/*
	 *	Handle pipe errors between sysconfig and cfgmgr
	 */
	sigact_pipe.sa_handler = (void (*)())&cmgr_client_state;
	sigfillset(&sigact_pipe.sa_mask);
	sigact_pipe.sa_flags = 0;
	sigaction(SIGPIPE, &sigact_pipe, (struct sigaction *)NULL);

	startup_mode();

	if (CMGR.fflg == FALSE)
	    	cmgr_detach();
	else
		cmgr_write_pid_file();

	request_mode();

	cmgr_mainexit();
}


