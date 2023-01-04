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
static char	*sccsid = "@(#)$RCSfile: logger.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/11 17:19:12 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: logger
 *
 * ORIGINS: 26, 27
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * logger.c	1.4  com/cmd/oper,3.1,9021 4/26/90 17:18:57
 */

#include <stdio.h>
#include <unistd.h>
#include <locale.h>
#include <sys/syslog.h>
#include <NLctype.h>
#include <dec/binlog/binlog.h>      /* binary event logger support */

#include "logger_msg.h" 
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LOGGER,n,s) 

#define BUFSIZE 200
#define TMPBUFSIZE 2*BUFSIZE

/*
**  LOGGER -- read and log utility
**
**	This routine reads from an input and arranges to write the
**	result on the system log, along with a useful tag.
*/


main(argc, argv)
	int argc;
	char **argv;
{
	extern char *optarg;
	extern int optind;
	int pri = LOG_NOTICE;
	int ch, logflags = 0;
	char *tag, buf[BUFSIZE], tmp[TMPBUFSIZE]; 
	int binlogflag = 0;          /* log to binary event logger ? */

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_LOGGER,NL_CAT_LOCALE);
	tag = NULL;

	while ((ch = getopt(argc, argv, "f:ibp:t:")) != EOF)
		switch ((char)ch) {
		  case 'f':		/* file to log */
			if (freopen(optarg, "r", stdin) == NULL) {
				fprintf(stderr, "logger: ");
				perror(optarg);
				exit(1);
			}
			break;
		  case 'i':		/* log process id also */
			logflags |= LOG_PID;
			break;
		  case 'p':		/* priority */
			pri = pencode(optarg);
			break;
		  case 't':		/* tag */
			tag = optarg;
			break;
		  case 'b':            /* send message to binary event logger */
			binlogflag++;
			break;
		  case '?':
		  default:
			usage();
		}
	argc -= optind;
	argv += optind;

	/* setup for logging */
	openlog(tag ? tag : getlogin(), logflags, 0);
	(void) fclose(stdout);

	/* log input line if appropriate */
	if (argc > 0)
	{
		register char *p, *endp;
		int len;

		for (p = buf, endp = buf + sizeof(buf) - 1;;) {
			len = strlen(*argv);
			if (p + len < endp && p > buf) {
				*--p = '\0';
				checkmsg(buf, tmp);
				syslog(pri, tmp);
				p = buf;
			}
			if (len > sizeof(buf) - 1) {
				if (binlogflag)   /* put msg in binary log */
					binlogmsg(ELMSGT_INFO, *argv++);
				else {
					checkmsg(*argv++, tmp);
				        syslog(pri, tmp);
				}
				if (!--argc)
					break;
			} else {
				bcopy(*argv++, p, len);
				p += len;
				if (!--argc)
					break;
				*p++ = ' ';
				*--p = '\0';
			}
		}
		if (p != buf) {
			*p = '\0';
			 if (binlogflag)     /* put msg in binary log */
				  binlogmsg(ELMSGT_INFO, buf);
			 else {
				  checkmsg(buf, tmp);
			          syslog(pri, tmp);
			 }
		}
		exit(0);
	}

	/*
	 * Logging from a file to the binary event log is not supported
	 * Doing so can certainly be done, but each 'buf' read from the file
	 * would be a separate event log record.  You could program around
	 * this by reading the file into one big buffer.  However the -b
	 * option is really only for logging a short simple message.
	 */
	if (binlogflag)
		  usage();


	/* main loop */
	while (fgets(buf, (int)sizeof(buf), stdin) != NULL) {
		checkmsg(buf, tmp);
		syslog(pri, tmp);
        }
	exit(0);
}


struct code {
	char	*c_name;
	int	c_val;
};

struct code	PriNames[] = {
	"panic",	LOG_EMERG,
	"emerg",	LOG_EMERG,
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"err",		LOG_ERR,
	"error",	LOG_ERR,
	"warn",		LOG_WARNING,
	"warning",	LOG_WARNING,
	"notice",	LOG_NOTICE,
	"info",		LOG_INFO,
	"debug",	LOG_DEBUG,
	NULL,		-1
};

struct code	FacNames[] = {
	"kern",		LOG_KERN,
	"user",		LOG_USER,
	"mail",		LOG_MAIL,
	"daemon",	LOG_DAEMON,
	"auth",		LOG_AUTH,
	"security",	LOG_AUTH,
	"syslog",	LOG_SYSLOG,
	"lpr",		LOG_LPR,
	"news",		LOG_NEWS,
	"uucp",		LOG_UUCP,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1
};


/*
 *  Decode a symbolic name to a numeric value
 */

pencode(s)
	register char *s;
{
	register char *p;
	int lev;
	int fac;
	char buf[100];

	for (p = buf; *s && *s != '.'; )
		*p++ = *s++;
	*p = '\0';
	if (*s++) {
		fac = decode(buf, FacNames);
		if (fac < 0)
			bailout(MSGSTR(UNKFACILITY, "unknown facility name: "), buf);
		for (p = buf; *p++ = *s++; )
			continue;
	} else
		fac = 0;
	lev = decode(buf, PriNames);
	if (lev < 0)
		bailout(MSGSTR(UNKPRIORITY, "unknown priority name: "), buf);

	return ((lev & LOG_PRIMASK) | (fac & LOG_FACMASK));
}


/*
 * NAME: decode
 * FUNCTION: search code table for name, return its value
 */
decode(name, codetab)
	char *name;
	struct code *codetab;
{
	register struct code *c;
	register char *p;
	char buf[40];

	if (NCisdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (NCisupper(*p))
			*p = NCtolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);
	return (-1);
}

/*
 * NAME: bailout
 * FUNCTION: display error message and exit
 */
bailout(msg, arg)
	char *msg, *arg;
{
	fprintf(stderr, "logger: %s%s\n", msg, arg);
	exit(1);
}

usage()
{
	fputs(MSGSTR(USAGE,
	    "logger: [-i] [-b] [-f file] [-p pri] [-t tag] [ message ... ]\n"),
	    stderr);
	exit(1);
}

/*
 * The syslog(3) function expects a printf() type format string as
 * an argument.  So, if logger is given a string with a '%' it will 
 * cause syslog(3) to put unexpected stuff in the log file or possibly 
 * cause syslog(3) to core dump because it will treat the % as a format
 * specifier. 
 *
 * This routine checks the message before it is given to syslog(3).  If it 
 * contains any occurances of '%' than it stuffs in another '%' immediatly 
 * after....   i.e. if a % is supposed to be in the output than you need
 * two of them sequentialy just like you would for printf().  
*/
checkmsg(buf, tmp)
char *buf;            /* data to check */
char *tmp;            /* data after checking */
{
     register char *b, *t;
     long end;

     b = buf;
     t = tmp;

     end = strlen(buf) + (long)buf;

     while ( (long)b < end  &&  *b != '\0'  &&  t < &tmp[TMPBUFSIZE]) {
	 if (*b != '%')  {     /* if current character not a % just save it */
	      *t++ = *b++;     
	      continue;
	 }
	 *t++ = *b++;
         *t++ = '%';           /* stuff in another % */
      }
}
