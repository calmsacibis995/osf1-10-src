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
static char *rcsid = "@(#)$RCSfile: screend.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/16 15:34:36 $";
#endif

/*
 * screend.c
 * IP gateway screening daemon
 *
 * Usage:
 *	screend [-d] [-c] [-l] [-f filename] [-s] [-r] [-L logfile]
 *
 * 21 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/errno.h>

#include <net/if.h>
#include <netinet/in.h>
#include "screentab.h"
#include <net/gw_screen.h>

#include <stdio.h>
#include <syslog.h>
#include <signal.h>

#define	DEFCONFIG "/etc/screend.conf"

#define	REPORT_INTERVAL	(60*60)		/* seconds between stat reports */

char *sourcefile = DEFCONFIG;

int debug = 0;
int semantic_errors = 0;
int logall = 0;
int checkonly = 0;
int use_syslog = 0;
int log_rule = 0;
int compress_log = 1;	/* do we even need a flag for this one? */
FILE *log_file = NULL;
char *logfilename = NULL;

extern int LastMatchRule;
extern int LastCacheHit;

int	TotalCacheHits;
int	TotalCacheMisses;

int	screen_sock;

void Restart();
void Reporter();

extern int errno;

main(argc, argv)
int argc;
char **argv;
{
	struct itimerval itv;

	while (argc > 1) {
	    if (argv[1][0] != '-') {
		Usage();
		exit(1);
	    }
	    switch (argv[1][1]) {
		case 'c':
		    checkonly++;
		    break;
		case 'd':
		    debug++;
		    break;
		case 'l':
		    logall++;
		    break;
		case 'L':
		    argc--;
		    argv++;
		    if (argc < 2) {
			fprintf(stderr, "-L: no filename argument\n");
			Usage();
			exit(1);
		    }
		    logfilename = argv[1];
		    break;
		case 'f':
		    argc--;
		    argv++;
		    if (argc < 2) {
			fprintf(stderr, "-f: no filename argument\n");
			Usage();
			exit(1);
		    }
		    sourcefile = argv[1];
		    break;
		case 's':
		    use_syslog = 1;
		    break;
		case 'r':
		    log_rule = 1;
		    break;
		default:
		    fprintf(stderr, "%s: unknown flag\n", argv[1]);
		    Usage();
		    exit(1);
		    break;
	    }
	    argc--;
	    argv++;
	}

	if (freopen(sourcefile, "r", stdin) == NULL) {
	    perror(sourcefile);
	    exit(1);
	}

	InitTables();
	InitFragCache();
	if (yyparse() || semantic_errors) {
	    fprintf(stderr, "%s: not correct\n", sourcefile);
	    exit(1);
	}
	fclose(stdin);	/* don't need config file any more */

	if (debug) {
	    DumpNetMaskTable();
	    DumpActionTable();
	}
	
	if (checkonly) {
	    exit(0);
	}

	if (debug == 0) {
	    int i;

	    if (fork()) {
		exit(0);
	    }

	    i = getdtablesize();
	    while (--i >= 0)
		(void)close(i);

	    (void) open("/dev/null", O_RDONLY);
	    (void) dup2(0, 1);
	    (void) dup2(0, 2);
	    i = open("/dev/tty", O_RDWR);
	    if (i >= 0) {
		(void) ioctl(i, TIOCNOTTY, NULL);
		(void) close(i);
	    }
	}

	if (use_syslog) {
	    if (openlog("screend", LOG_PID, LOG_DAEMON) < 0) {
		perror("syslog");
	    }
	    else {
		LogSyslog("started");
	    }
	}

	if (logfilename) {
	    log_file = fopen(logfilename, "a");
	    if (log_file == NULL) {
		perror(logfilename);
		exit(1);
	    }
	    LogToFileTS(log_file, "START");
	}

	screen_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (screen_sock < 0) {
		Myperror("screend: socket");
		exit(1);
	}

	(void)signal(SIGHUP, Restart);
	(void)signal(SIGALRM, Reporter);

	/*
	 * Set up a periodic timer for doing statistics reports;
	 * do the first report "real soon" so that we have a
	 * baseline.
	 */
	itv.it_interval.tv_sec = REPORT_INTERVAL;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	
	(void) setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

	InitReporter(screen_sock);

	Daemon(screen_sock);
}

Usage()
{
	fprintf(stderr,
"Usage: screend [-d] [-c] [-l] [-f filename] [-s] [-r] [-L logfile]\n");
}

/* Entered upon SIGHUP */
void Restart()
{
	Reporter();
	if (use_syslog)
	    syslog(LOG_INFO, "restarting due to SIGHUP", 0);
	else
	    syslog(LOG_INFO, "screend: restarting due to SIGHUP", 0);
	ReOpenLog(1);
	InitReporter(screen_sock);
	if (use_syslog)
	    syslog(LOG_INFO, "restart complete", 0);
	else
	    syslog(LOG_INFO, "screend: restart complete", 0);
}


struct	screen_data scdata;

Daemon(s)
register int s;
{
	static struct annotated_hdrs ah;
	register int actcode;

	scdata.sd_xid = 0;	/* make sure things start "clean" */
	scdata.sd_family = AF_INET;	/* make sure we get only IPs */
	scdata.sd_action = SCREEN_DROP;

	while (1) {
	    if (debug) {
		PrintScreenData(&scdata);
	    }
	    if (ioctl(s, SIOCSCREEN, (caddr_t)&scdata) < 0) {
		if (errno != EINTR) {
		    Myperror("ioctl (SIOCSCREEN)");
		    exit(1);
		}
	    }
	    scdata.sd_action = SCREEN_DROP;	/* default is DROP */

	    /* unpack this packet */
	    bzero(&ah, sizeof(ah));
	    if (UnpackIP(scdata.sd_data, scdata.sd_dlen, &(ah.hdrs)) == 0) {
		/* cannot parse this packet */
		continue;
	    }

	    if (debug) {
		PrintAnnotatedHdrs(&ah);
		printf("\n");
	    }
	    
	    /* Now see what we have */
	    actcode = ScreenIt(&ah);

	    if (LastCacheHit)
		TotalCacheHits++;
	    else
		TotalCacheMisses++;

	    if (actcode & ASACTION_ACCEPT)
		scdata.sd_action = SCREEN_ACCEPT;
	    else if (actcode & ASACTION_NOTIFY)
		scdata.sd_action |= SCREEN_NOTIFY;
	    if (logall || (actcode & ASACTION_LOG))
		LogIt(&scdata, actcode, &(ah.hdrs));
	}
}

PrintScreenData(sdp)
register struct screen_data *sdp;
{
	printf("(%d.%06d) af %d count %d dlen %d xid %x action %x",
		sdp->sd_arrival.tv_sec, sdp->sd_arrival.tv_usec,
		sdp->sd_family,
		sdp->sd_count, sdp->sd_dlen, sdp->sd_xid, sdp->sd_action);
	if (sdp->sd_action & SCREEN_ACCEPT)
		printf(" ACCEPT");
	else
		printf(" REJECT");
	if (sdp->sd_action & SCREEN_NOTIFY)
		printf(" NOTIFY");
	printf("\n");

/*	PrintIPHeader(&(sdp->sd_arrival), sdp->sd_data, sdp->sd_dlen); */
}

int origdrops;

InitReporter(s)
int s;
{
	struct screen_stats sstats;
	
	if (ioctl(screen_sock, SIOCSCREENSTATS, (caddr_t)&sstats) < 0) {
	    Myperror("ioctl (SIOCSCREENSTATS)");
	    origdrops = 0;
	    return;
	}
	
	origdrops = sstats.ss_nobuffer + sstats.ss_badsync + sstats.ss_stale;
	TotalCacheHits = 0;
	TotalCacheMisses = 0;
}

void Reporter()
{
	struct screen_stats sstats;
	
	if (ioctl(screen_sock, SIOCSCREENSTATS, (caddr_t)&sstats) < 0) {
	    Myperror("ioctl (SIOCSCREENSTATS)");
	    return;
	}
	
	LogStats(TotalCacheHits, TotalCacheMisses, origdrops, &sstats);

	/*
	 * Once an hour, close and open log file (if in use) so that
	 * renames of the log file will have some effect.
	 */
	ReOpenLog(0);
}
