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
static char	*sccsid = "@(#)$RCSfile: syslog.c,v $ $Revision: 4.2.9.4 $ (DEC) $Date: 1993/10/05 21:03:25 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

#if !defined(lint) && !defined(_NOIDENT)
#endif

/*
 */ 
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: syslog, setlogmask, openlog, closelog
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *  "syslog.c	1.7  com/lib/c/gen,3.1,9021 4/18/90 17:28:02";
 */

/*
 * FUNCTION: print message on log file
 */

/*
 * SYSLOG -- print message on log file
 *
 * This routine looks a lot like printf, except that it
 * outputs to the log file instead of the standard output.
 * Also:
 *	adds a timestamp,
 *	prints the module name in front of the message,
 *	has some other formatting types (or will sometime),
 *	adds a newline on the end of the message.
 *
 * The output of this routine is intended to be read by /etc/syslogd.
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#undef NLcatgets
#if defined(_THREAD_SAFE)
#pragma weak closelog_r = __closelog_r
#pragma weak openlog_r = __openlog_r
#pragma weak setlogmask_r = __setlogmask_r
#pragma weak syslog_r = __syslog_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak closelog = __closelog
#pragma weak openlog = __openlog
#pragma weak setlogmask = __setlogmask
#pragma weak syslog = __syslog
#endif
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include "libc_msg.h"
#include <time.h>
#include <errno.h>

extern int sys_nerr;
extern char *sys_errlist[];

/* tap: 08/04/93: Bumped up MAXLINE from 1024 to handle long messages from rshd */
#define	MAXLINE	LINE_MAX * 2       /* max message size */

#define PRIMASK(p)	(1 << ((p) & LOG_PRIMASK))
#define PRIFAC(p)	(((p) & LOG_FACMASK) >> 3)
#define IMPORTANT 	LOG_ERR

static char	logname[] = "/dev/log";
static char	ctty[] = "/dev/console";

#ifdef _THREAD_SAFE

#define	OPENLOG(i,s,f)	openlog_r(i, s, f, syslog_data)

#define LogFile 	(syslog_data->log_file)
#define LogStat 	(syslog_data->log_stat)
#define LogTag 		(syslog_data->log_tag)
#define LogMask 	(syslog_data->log_mask)
#define LogFacility 	(syslog_data->log_facility)
#define SyslogAddr 	(syslog_data->syslog_addr)

#else

#define	OPENLOG	openlog

static int	LogFile = -1;		/* fd for log */
static int	LogStat	= 0;		/* status bits, set by openlog() */
static char	*LogTag = "syslog";	/* string to tag the entry with */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static int	LogFacility = LOG_USER;	/* default facility code */
static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */

#endif /* _THREAD_SAFE */


#ifdef _THREAD_SAFE
int
syslog_r(int pri, struct syslog_data *syslog_data, const char *fmt, ...)

#else
int
syslog(int pri, const char *fmt, ...)
#endif	/* _THREAD_SAFE */
{
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	register char *b, *o;
	register const char *f;
	register int c;
	time_t   now;
	pid_t pid;
	int olderrno = _Geterrno();
	va_list arg_list;
#ifdef _THREAD_SAFE
	char tmpstr[64];
#endif	/* _THREAD_SAFE */

	/* see if we should just throw out this message */
	if (pri < 0 || PRIFAC(pri) >= LOG_NFACILITIES || (PRIMASK(pri) & LogMask) == 0)
		return (-1);
	if (LogFile < 0)
		OPENLOG(LogTag, LogStat | LOG_NDELAY, 0);

	/* set default facility if none specified */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the message */
	o = outline;
	sprintf(o, "<%d>", pri);
	o += strlen(o);
	time(&now);
#ifdef _THREAD_SAFE
	sprintf(o, "%.15s ", (ctime_r(&now, tmpstr, 64), tmpstr + 4));
#else
	sprintf(o, "%.15s ", ctime(&now) + 4);
#endif	/* _THREAD_SAFE */
	o += strlen(o);
	if (LogTag) {
		strcpy(o, LogTag);
		o += strlen(o);
	}
	if (LogStat & LOG_PID) {
		sprintf(o, "[%d]", getpid());
		o += strlen(o);
	}
	if (LogTag) {
		strcpy(o, ": ");
		o += 2;
	}

	/* f is the first element of the variable list
	   the rest of the variable list is stored in arg_list
	 */

	va_start(arg_list, fmt);
	
	f = fmt;

	b = buf;

	while ((c = *f++) != '\0' && c != '\n' && b < &buf[MAXLINE]) {
		if (c != '%') {
			*b++ = c;
			continue;
		}
		if ((c = *f++) != 'm') {
			*b++ = '%';
			*b++ = c;
			continue;
		}
#ifdef _THREAD_SAFE
		strerror_r(olderrno, b, sizeof(buf) - (b - buf));
#else
		strcpy(b, strerror(olderrno));
#endif	/* _THREAD_SAFE */
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	va_start(arg_list, fmt);
	vsprintf(o, buf, arg_list);
	va_end(arg_list)
	c = strlen(outline);
	if (c > MAXLINE)
		c = MAXLINE;

	/* output the message to the local logger */
	if (sendto(LogFile, outline, c, 0, &SyslogAddr, sizeof SyslogAddr) >= 0)
		return (0);
	if (!(LogStat & LOG_CONS))
		return (0);

	/* output the message to the console */
	pid = fork();
	if (pid == -1)
		return (-1);
	if (pid == 0) {
		int fd;

		signal(SIGALRM, SIG_DFL);
		sigsetmask(sigblock(0) & ~sigmask(SIGALRM));
		alarm(5);
		fd = open(ctty, O_WRONLY);
		alarm(0);
		strcat(o, "\r");
		o = strchr(outline, '>') + 1;
		write(fd, o, c + 1 - (o - outline));
		close(fd);
		_exit(0);
	}
	if (!(LogStat & LOG_NOWAIT))
		if (waitpid(pid, NULL, 0) == -1)
			return (-1);
	return (0);
}


/*
 * OPENLOG -- open system log
 */
#ifdef _THREAD_SAFE
int
openlog_r(const char *ident, int logstat, int logfac,
	  struct syslog_data *syslog_data)
#else
int
openlog(const char *ident, int logstat, int logfac)
#endif /* _THREAD_SAFE */
{

	if (ident != NULL)
		LogTag = (char *)ident;
	LogStat = logstat;
	if (logfac != 0)
		LogFacility = logfac & LOG_FACMASK;
	if (LogFile >= 0)
		return (0);
	SyslogAddr.sa_family = AF_UNIX;
	strncpy(SyslogAddr.sa_data, logname, (size_t)sizeof SyslogAddr.sa_data);
	if (LogStat & LOG_NDELAY) {
		LogFile = socket(AF_UNIX, SOCK_DGRAM, 0);
		if (LogFile < 0)
			return (LogFile);
		fcntl(LogFile, F_SETFD, 1);
	}
	return (0);
}


/*
 * CLOSELOG -- close the system log
 */
#ifdef _THREAD_SAFE
void closelog_r(struct syslog_data *syslog_data)
#else
void closelog(void)
#endif	/* _THREAD_SAFE */
{

	(void) close(LogFile);
	LogFile = -1;
}


/*
 * SETLOGMASK -- set the log mask level
 */
#ifdef _THREAD_SAFE
int
setlogmask_r(int pmask, struct syslog_data *syslog_data)
#else
int
setlogmask(int pmask)
#endif	/* _THREAD_SAFE */
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}
