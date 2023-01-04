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
static char *rcsid = "@(#)$RCSfile: log.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/08/14 12:25:15 $";
#endif

/*
 * log.c
 * Logging routines for
 *	IP gateway screening daemon
 *
 * 21 April 1989	Jeffrey Mogul/DECWRL
 *	Created (extracted from screend.c)
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

static char MsgBuffer[1024];
static char MsgBufferX[1024];

extern int debug;
extern int log_rule;
extern int use_syslog;
extern int compress_log;
extern FILE *log_file;
extern char *logfilename;

extern int LastMatchRule;
extern int LastCacheHit;

int lcdebug = 0;	/* debug log cache code */

LogIt(sdp, act, uhp)
register struct screen_data *sdp;
int act;
struct unpacked_hdrs *uhp;
{
	static char rulebuf[16];
	char *lch;

	if (compress_log && LogCacheEnter(uhp))
	    return;	/* already in the log */

	if (log_rule) {
	    if (LastCacheHit)
		lch = " +";
	    else
		lch = " -";
	    if (LastMatchRule >= 0)
		sprintf(rulebuf, " (rule %d%s)", LastMatchRule, lch);
	    else
		sprintf(rulebuf, " (def rule%s)", lch);
	    }
	else
	    rulebuf[0] = 0;

	FormatIPHeader(sdp->sd_data, sdp->sd_dlen, MsgBufferX);
	if (sdp->sd_action & SCREEN_ACCEPT)
	    sprintf(MsgBuffer, "ACCEPT: %s%s", MsgBufferX, rulebuf);
	else if (sdp->sd_action & SCREEN_NOTIFY)
	    sprintf(MsgBuffer, "REJECTN: %s%s", MsgBufferX, rulebuf);
	else	/* REJECT but not NOTIFY */
	    sprintf(MsgBuffer, "REJECT: %s%s", MsgBufferX, rulebuf);
	if (use_syslog)
		LogSyslog(MsgBuffer);
	if (log_file)
		LogToFile(log_file, MsgBuffer);
}

LogToFile(lf, msg)
register FILE *lf;
register char *msg;
{
	fprintf(lf, "%s\n", msg);
	fflush(lf);
}

LogToFileTS(lf, msg)
register FILE *lf;
register char *msg;
{
	int now;
	char *ctime();

	now = time(0);
	fprintf(lf, "%s: %s", msg, ctime(&now));
	fflush(lf);
}

LogSyslog(msg)
register char *msg;
{
	syslog(LOG_INFO, msg, 0);
}

Myperror(msg)
register char *msg;
{
	if (debug) {	/* stderr still exists */
	    perror(msg);
	}
	else {
	    syslog(LOG_WARNING, "screend: %s: %m", msg);
	}
}

ReOpenLog(is_restart)
int is_restart;
{
	if (logfilename) {
	    if (log_file)
		fclose(log_file);
	    log_file = fopen(logfilename, "a");
	    if (log_file == NULL) {
		syslog(LOG_WARNING, "screend: logfile %s: %m", logfilename);
	    }
	    else {
		if (is_restart)
		    LogToFileTS(log_file, "RESTART");
		else
		    LogToFileTS(log_file, "REOPEN");
	    }
	}
}

LogStats(t_hits, t_misses, origdrops, sstatsp)
int t_hits;
int t_misses;
int origdrops;
struct screen_stats *sstatsp;
{
	int totalpkts = t_hits + t_misses;
	int totaldrops;
	float hitrate;

	totaldrops =
		sstatsp->ss_nobuffer + sstatsp->ss_badsync + sstatsp->ss_stale;

	hitrate = (t_hits * 100.0) / (totalpkts + 1.0);

	sprintf(MsgBuffer, "%d pkts (%.2f%% hits), %d drops",
				totalpkts,
				hitrate,
				totaldrops - origdrops);

	if (use_syslog)
	    syslog(LOG_INFO, "%s", MsgBuffer);	/* "screend" already there */
	else
	    syslog(LOG_INFO, "screend: %s", MsgBuffer);

	if (log_file) {
	    sprintf(MsgBufferX, "STATS: %s", MsgBuffer);
	    LogToFileTS(log_file, MsgBufferX);
	}
}

/*
 * Keep a small cache of recent log records to avoid spewing in the log
 */

#define	LOGCACHESIZE	16
#define	PRINTDELAY	(10*60)	/* seconds between printing even on hits */

struct LogCache_entry {
	struct unpacked_hdrs	uph;
	int			count;
	int			age;
	int			lastPrinted;
};

struct LogCache_entry LogCache[LOGCACHESIZE];
		/* ASSUMPTION: initialized to all zeros */

u_int logCache_time = 0;	/* pseudo-age for LRU */

/*
 * LogCacheEnter enters the specified into the Log cache.
 * Returns "true" IFF it was already there.
 * If an entry is replaced, a summary log entry is generated.
 */
LogCacheEnter(uhp)
register struct unpacked_hdrs *uhp;
{
	register int i;
	register struct LogCache_entry *cep;
	register struct LogCache_entry *eldest;
	int now;
	
	if (lcdebug)
	    printf("checking log cache:\n");
	cep = LogCache;
	eldest = LogCache;
	for (i = 0; i < LOGCACHESIZE; i++, cep++) {
	    /* keep track of oldest entry */
	    if (cep->age < eldest->age)
		eldest = cep;

	    if (lcdebug) {
		printf("\tage %d ", cep->age);
		PrintUnpackedHdrs(&(cep->uph));
		printf("\n");
	    }
	    /* check for match on all components */
	    if (cep->uph.src.addr.s_addr != uhp->src.addr.s_addr)
		continue;
	    if (cep->uph.dst.addr.s_addr != uhp->dst.addr.s_addr)
		continue;
	    if (cep->uph.proto != uhp->proto)
		continue;
	    /* next two may not always be valid but they always must match */
	    if (cep->uph.src.port != uhp->src.port)
		continue;
	    if (cep->uph.dst.port != uhp->dst.port)
		continue;

	    if (lcdebug) {
		printf("hit\n");
	    }
	    /* hit */
	    cep->age = logCache_time++;	/* update LRU timer */
	    cep->count++;
	    now = time(0);
	    if ((cep->lastPrinted + PRINTDELAY) < now) {
		/* Force printout every once in a while */
		Summarize(cep);
		cep->lastPrinted = now;
		cep->count = 1;
	    }
	    return(1);
	}
	
	if (lcdebug) {
	    printf("miss\n");
	}
	/* replace eldest cache entry */
	if (eldest->count > 1) {
	    /* Generate summary log record if we suppressed anything */
	    Summarize(eldest);
	}
	eldest->uph = *uhp;
	eldest->age = logCache_time++;
	eldest->count = 1;
	eldest->lastPrinted = time(0);
	
	return(0);
}

Summarize(cep)
register struct LogCache_entry *cep;
{
	FormatUnpackedIPHeader(&(cep->uph), MsgBufferX);
	sprintf(MsgBuffer, "SUMMARY: %d of %s", cep->count, MsgBufferX);
	if (use_syslog)
		LogSyslog(MsgBuffer);
	if (log_file)
		LogToFile(log_file, MsgBuffer);
}
