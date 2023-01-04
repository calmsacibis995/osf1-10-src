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
static char *rcsid = "@(#)$RCSfile: ftpd_syslog.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/01/18 14:27:12 $";
#endif

#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * Special version of syslog which builds on an INET socket so the
 * logging capability can be provided across a chroot for the FTP
 * anonymous account.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <netinet/in.h>
#ifdef	__STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#if defined(NLS) || defined(KJI)
#include <time.h>
#endif

#define	MAXLINE	1024			/* max message size */

#define PRIMASK(p)	(1 << ((p) & LOG_PRIMASK))
#define PRIFAC(p)	(((p) & LOG_FACMASK) >> 3)
#define IMPORTANT 	LOG_ERR

static char	ctty[] = "/dev/console";

static int	LogFile = -1;		/* fd for log */
static int	LogStat	= 0;		/* status bits, set by openlog() */
static char	*LogTag = "syslog";	/* string to tag the entry with */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
static int	LogFacility = LOG_USER;	/* default facility code */
static char	*SyslogHost = "localhost";	/* log host */

static struct sockaddr_in SyslogAddr;	/* AF_INET address of local logger */

extern	int errno, sys_nerr;
extern	char *sys_errlist[];
/*
#ifdef MSG
#include "libc_msg.h"
#endif
*/

#if defined(NLS) || defined(KJI)
#define SPRINTF	NLvsprintf
#else
#define SPRINTF vsprintf
#endif

#ifndef	_NO_PROTO
old_syslog(int pri, const char *fmt, ...)
#else
old_syslog(pri, fmt, va_alist)
int pri;
const char *fmt;
va_dcl
#endif
{
	va_list arg_list;
	char buf[MAXLINE + 1], outline[MAXLINE + 1];
	register char *b, *o;
	register const char *f;
	register int c;
	time_t   now;
	pid_t pid;
	int olderrno = errno;

	/* see if we should just throw out this message */
        if (pri < 0 || PRIFAC(pri) >= LOG_NFACILITIES || (PRIMASK(pri) & LogMask) == 0)
		return(-1);

	if (LogFile < 0)
		old_openlog(LogTag, LogStat | LOG_NDELAY, 0);

	/* set default facility if none specified */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* build the message */
	o = outline;
	sprintf(o, "<%d>", pri);
	o += strlen(o);
	time(&now);
	sprintf(o, "%.15s ", ctime(&now) + 4);
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

	b = buf;

	/* f is the first element of the variable list
	   the rest of the variable list is stored in arg_list
	 */

#ifdef  __STDC__
	va_start(arg_list, fmt);
#else
        va_start(arg_list);
#endif
	
	f = fmt;

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
		if ((unsigned)olderrno > sys_nerr)
			sprintf(b, "error %d", olderrno);
		else
/*
#ifdef MSG
			sprintf(b, NLgetamsg( MF_LIBC, MS_LIBC, errno, 
				sys_errlist[errno]));
#else
*/
			strcpy(b, sys_errlist[olderrno]);
/*
#endif
*/
		b += strlen(b);
	}
	*b++ = '\n';
	*b = '\0';
	
	SPRINTF(o, buf, arg_list);
	c = strlen(outline);
	if (c > MAXLINE)
		c = MAXLINE;

	/* output the message to the local logger */
	if (sendto(LogFile, outline, c, 0, &SyslogAddr, sizeof SyslogAddr) >= 0)
		return(0);
	if (!(LogStat & LOG_CONS))
		return(0);

	/* output the message to the console */
	pid = fork();
	if (pid == -1)
		return(-1);
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
                else
                  return (0);
}

/*
 * OLD_OPENLOG -- open system log
 */

old_openlog(ident, logstat, logfac)
	const char *ident;
	int logstat, logfac;
{
	struct servent *sp;
	struct hostent *hp;

	if (ident != NULL)
		LogTag = (char *)ident;
	LogStat = logstat;
	if (logfac != 0)
		LogFacility = logfac & LOG_FACMASK;
	if (LogFile >= 0)
		return(0);

	sp = getservbyname("syslog", "udp");
	hp = gethostbyname(SyslogHost);
	if (sp != NULL && hp != NULL)
	{
		bzero(&SyslogAddr, sizeof SyslogAddr);
		SyslogAddr.sin_family = AF_INET;
		LogFile = socket(AF_INET, SOCK_DGRAM, 0, 0);
		if (LogStat & LOG_NDELAY) {
			if (LogFile >= 0 && bind(LogFile, &SyslogAddr, sizeof SyslogAddr, 0) < 0) {
				close(LogFile);
				LogFile = -1;
			}
		}
		SyslogAddr.sin_port = sp->s_port;
		bcopy(hp->h_addr, (char *) &SyslogAddr.sin_addr, hp->h_length);
	}	
}

/*
 * OLD_CLOSELOG -- close the system log
 */

void old_closelog()
{

	(void) close(LogFile);
	LogFile = -1;
}

