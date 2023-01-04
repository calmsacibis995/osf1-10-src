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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: date.c,v $ $Revision: 4.2.8.3 $ (DEC) $Date: 1993/10/11 16:11:11 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989,1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * date.c      1.27  com/cmd/stat,3.1,9021 3/13/90 14:29:50
 */

/**********************************************************************/
/* Include Files						      */
/**********************************************************************/

#include  <sys/secdefines.h>
#if SEC_BASE
#include  <sys/security.h>
#endif

#include	<sys/types.h>
#include	<fcntl.h>
#include	<utmp.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<time.h>
#include	<locale.h>
#include	<errno.h>
#include	<sys/syslog.h>

#include	<wchar.h>
#include	<nl_types.h>
#include	<langinfo.h>
#include	"date_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/

#define  NO_ERROR  0
#define  ERROR_1   1
#define  ERROR_2   2
#define  BLEN     64
#define  BUFSIZE  100
#define YEAR_AND_CENTURY 1

#define MSGSTR(num,str) catgets(catd,MS_DATE,num,str)  

/**********************************************************************/
/* Function Prototype Declaration				      */
/**********************************************************************/

static void  usage(void);
static time_t gtime(char *);
static int switchdate(void);
static int settime(time_t);

/**********************************************************************/
/* Variables					      		      */
/**********************************************************************/

static char buffer[BUFSIZE];

static int uflag = FALSE;
static int nflag = FALSE;

static char 	*pnldate;

static char	*cbp;
static time_t	timbuf;

static struct	utmp	wtmp[2] = {
	{"","",OTIME_MSG,OLD_TIME,0,0,0,0,""},
    	{"","",NTIME_MSG,NEW_TIME,0,0,0,0,""}
};

static nl_catd catd;


/***********************************************************************
 * NAME:  main
 *
 * FUNCTION:  date - with format capabilities
 *
 * RETURN VALUE:
 ***********************************************************************/
void
main(int argc, char *argv[])
{
	int     tfailed,     /* Flag indicating time-setting operation failed */
		wf;		/* file-descriptor for wtmp file */

	int	 opt;		/* Command option parse flag */
	char 	*loc;		/* the current locale */
	char	*dt_format;	/* strftime format from nl_langinfo() */

#if SEC_BASE
	int	     privs_raised;
	privvec_t       saveprivs;
	extern priv_t   *privvec();
#endif

	(void) setlocale (LC_ALL, "");
	catd = catopen(MF_DATE,NL_CAT_LOCALE);

	pnldate = nl_langinfo(D_FMT);


#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif

	tfailed = 0;

	while ( (opt = getopt(argc, argv, "nu")) != EOF ) {
		switch ( opt ) {
		  case 'n':
			nflag = TRUE;
			break;
		  case 'u':
			uflag = TRUE;
			putenv("TZ=UTC0");
			break;
		  case '?':
			usage();
			exit(ERROR_1);
		}
	}

	argc -= optind;
	argv += optind;

	cbp = (char *)argv[0];

	if(argc > 0) {

	    	/*
		 * Check for a user-supplied format string
		 */
		if(*cbp == '+') {
		    time_t	tbuf;

		    (void) time(&tbuf);

		    strftime(buffer, sizeof buffer, ++cbp, localtime(&tbuf));
		    printf("%s\n",buffer);
		    exit(NO_ERROR);
		}

		/*
		 * Check for attempt to set the time
		 */
		timbuf = gtime(cbp);
		if( !timbuf ) {
			(void) fprintf(stderr,
				       MSGSTR(ECONV, "date: bad conversion\n"));
			usage();
			exit(ERROR_2);
		}

		(void) time(&wtmp[0].ut_time);

#if SEC_BASE
		/*
		 * If user is authorized to set the date, raise the
		 * necessary privileges.  SEC_SYSATTR is required to
		 * set it locally, and SEC_REMOTE is required to create
		 * a privileged socket to talk to the time daemon.
		 * The access control overrides allow us to make entries
		 * in the utmp and wtmp files.
		 */
		if (authorized_user("sysadmin")) {
			if (forceprivs(privvec(SEC_SYSATTR, SEC_REMOTE,
						SEC_ALLOWDACACCESS,
#if SEC_MAC
						SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
						SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
						SEC_ALLOWNCAVACCESS,
#endif
						-1), saveprivs)) {

				fprintf(stderr, MSGSTR(EPRIV,
					"date: insufficient privileges\n"));
				exit(ERROR_1);
			}
			privs_raised = 1;
		} else
			privs_raised = 0;
#endif

		if (nflag || !settime(timbuf)) {
#if SEC_BASE
			disablepriv(SEC_SUSPEND_AUDIT);
#endif
			if(stime(&timbuf) < 0) {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				tfailed++;
				(void) fprintf(stderr, MSGSTR(BADPERM,
						"date: no permission\n"));
			}
			else {
#if SEC_BASE
				forcepriv(SEC_SUSPEND_AUDIT);
#endif
				(void) time(&wtmp[1].ut_time);

				/* Attempt to write entries to the
				 * utmp file and to the wtmp file. */

				pututline(&wtmp[0]);
				pututline(&wtmp[1]);

				if ((wf = open(WTMP_FILE, O_WRONLY|O_APPEND)) >= 0) {
					(void) write(wf, (char *)wtmp, sizeof(wtmp));
				}
			}
		}
#if SEC_BASE
		if (privs_raised) {
			seteffprivs(saveprivs, (priv_t *) 0);
		}
#endif

		if (!tfailed) {
			char *username;
			username = getlogin();
			/* single user or no tty */
			if (username == NULL || *username == '\0') {
				username = "root";
			}
			syslog(LOG_NOTICE, "set by %s", username);
		}

	}

	/*** Display default date and time representation ***/
	(void) time(&timbuf);

	loc = setlocale(LC_TIME, NULL);
	if (!strcmp(loc, "POSIX") || !strcmp(loc, "C"))
		dt_format = "%a %b %e %H:%M:%S %Z %Y";
	else
		dt_format = nl_langinfo(D_T_FMT);

	(void)strftime(buffer, BLEN, dt_format, localtime(&timbuf)); 

	printf("%s\n", buffer);
	exit(tfailed ? ERROR_2 : NO_ERROR );
	/* NOTREACHED */
}


char* add_delimiters(char* tmp)
{
    char* new_buf;
    char* delim;
    int i, len, str_len;

    delim = " ";
    str_len = strlen(tmp);

    if ( (new_buf = (char*) malloc(str_len + 8)) == NULL) {
       fprintf(stderr, MSGSTR(EMALLOC,"date: malloc failed\n"));
       return(0);
    }

    new_buf[0] = '\0';

    for(i = 0; i < str_len; i += len) {
	if (*tmp == '.')
	    len = 3;
	else
	    len = 2;

	strncat(new_buf, tmp, len);

	if ((i+len) >= str_len)
		delim = "";

	strcat(new_buf, delim);
	tmp += len;
    }

    return (new_buf);
}



/***********************************************************************
 *  NAME:  gtime
 *
 *  FUNCTION:  convert the time string given on the command line into
 * 		seconds since the Epoch.
 *					For locales w/ reversed mm/dd
 *  FORMATS:	mmddHHMM[.SS[yy]] 	ddmmHHMM[.SS[yy]]
 *		yymmddHHMM[.SS] 	yyddmmHHMM[.SS]
 *    [XPG4]	mmddHHMM[yy]		ddmmHHMM[yy]
 *
 *	Where mm=Month, dd=day, HH=hour, MM=minute, SS=second, yy=year.
 *  ALGORITHM:
 * 	Construct the strptime format string based on the lengths of
 * 	arguments specified.
 *	The month-day ordering is determined by examining the LC_TIME
 *	format to see if the %d comes before the %m in the default date format.
 * 
 * 	The only way to disambiguate between mmddHHMMyy and yymmddHHMM
 *	is to check the leading two digits for values 1-12 (months).
 * 	This is ugly, but we really need to support 'historical'
 * 	(and XPG!) syntax.
 *	      
 *  RETURN VALUE:  	 Non-zero (Conversion succeeded)
 *			 0	  (Conversion failed)
 *
 *	N.B. this means you can't set the time to 1/1/70
 **********************************************************************/
static time_t
gtime(char *argbuf)
{
	char	*s;
	char	*p;
	char    *new_argbuf;
	int	before_dot_length;
	int	after_dot_length;
	time_t	now;
	char	format[sizeof("%y"" %m"" %d"" %H"" %M"" .%S")];	/* max format */
	struct tm newtm;
	int	year_at_end = 0;

	/*
	 * Find out how many characters we have before and after the '.'
	 */
	p = strchr(argbuf, '.');
	if (p == NULL) {
		before_dot_length = strlen(argbuf);
		after_dot_length = 0;
	} else {
		before_dot_length = p - argbuf;
		after_dot_length = strlen(p+1);
		if (after_dot_length == 0)
			return(0);
	}

	/*
	 * Start constructing the strptime format string.
	 * 
	 * If it isn't 8 or 10 characters long, it isn't valid.
	 */
	format[0] = '\0';
	if (before_dot_length == sizeof("yymmddHHMM")-1) {
		int x = ((argbuf[0] - '0') * 10) + (argbuf[1] - '0');

		if (x > 0 && x < 13)		/* a month, not year */
			year_at_end = 1;
		else
			strcat(format, " %y");
	}
	else if (before_dot_length != sizeof("mmddHHMM")-1)
		return(0);

	/*
	 * Month or day first?  check this locale.
	 */
	if (switchdate())
		strcat(format, "%d"" %m"" %H"" %M");
	else
		strcat(format, "%m"" %d"" %H"" %M");

	if (year_at_end)
		strcat(format, " %y");
	/*
	 * If we have characters after the dot, its either seconds
	 * or seconds and year.
	 */
	if (after_dot_length) {
		if (after_dot_length == 2)
			strcat(format, " .%S");
		else if ((after_dot_length == 4) && 
			  (strchr(format, 'y') == NULL))
			strcat(format, " .%S"" %y");
		else
			return(0);
	}

	/*
	 * Set up the call to strptime.
	 */
	tzset();
	now = time(NULL);
	newtm = *localtime(&now);

        new_argbuf = (char*) add_delimiters(argbuf);

	argbuf = new_argbuf;

	s = strptime(argbuf, format, &newtm);

	if (s == NULL || *s != '\0')	/* failure or too many chars */
		return (0);	

	newtm.tm_isdst = -1;		/* Find out if DST in effect */
	errno = 0;
	now = mktime(&newtm);
	if (now == -1 || errno != 0)
		return(0);
	return (now);
}

/***********************************************************************
 *  NAME:  switchdate
 *
 *  FUNCTION:  Check the locale's default date format to see
 *	     if the month and day must change positions.
 *	      
 *  RETURN VALUE:  	 0  no swap is necessary
 *		       1  swap the month and day 
 **********************************************************************/

static int
switchdate(void)
{
	char *c;

	if (pnldate == NULL)
	    return (0);

	if( (c = strstr(pnldate, "%d")) ||
	    (c = strstr(pnldate, "%e")) )
	  /*
	   * Found "day" in format string.  Look for month afterwards
	   */
	    if ( strstr(c,"%m") || strstr(c,"%b") || strstr(c,"%B") )	
		return (1);	

	return (0);
}


#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

extern	int errno;
/***********************************************************************
 *  NAME:  settime
 *
 *  FUNCTION:  Set the date in the machines controlled by timedaemons
 *	     by communicating the new date to the local timedaemon.
 *	     If the timedaemon is in the master state, it performs the
 *	     correction on all slaves.  If it is in the slave state,
 *	     it notifies the master that a correction is needed.
 *	      
 *  RETURN VALUE:  0 - failure
 *		 1 - success
 */

static int
settime(time_t timbuf)
{
	int s, length, port, timed_ack, found, err;
	long waittime;
	fd_set ready;
	char hostname[MAXHOSTNAMELEN];
	struct timeval tout;
	struct timeval tv;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	tv.tv_sec = timbuf;
	tv.tv_usec = 0;

	sp = getservbyname("timed", "udp");
	if (sp == 0) {
		fprintf(stderr,
			MSGSTR(UNK_SERVICE,"date: udp/timed: unknown service\n"));
		return (0);
	}	
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		fprintf(stderr, MSGSTR(NOPORTS,"date: all ports in use\n"));
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	(void) gethostname(hostname, sizeof (hostname));
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)tv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)tv.tv_usec);
	length = sizeof (struct sockaddr_in);
	if (connect(s, (struct sockaddr *)&dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, &msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, &err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
			perror("date: send (delayed error)");
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, &msg, sizeof (struct tsp), 0, (struct sockaddr *)&from,
		    &length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);

		switch (msg.tsp_type) {
		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    MSGSTR(WRONG_ACK,"date: wrong ack received from timed: %s\n"), 
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1) {
		fprintf(stderr,
		    MSGSTR(CANTREACH,"date: can't reach time daemon - time set locally\n"));
	}
bad:
	(void)close(s);
	return (0);
}


/***********************************************************************
 *  NAME:  usage
 *
 *  FUNCTION:  Print out the command options.
 *	     They are different for the super user.
 *	      
 *  RETURN VALUE:  	 void
 **********************************************************************/

static void usage(void)
{
   char prompt[32];
   int ts_flg = 0;
   int ds_flg = 0;


	fprintf(stderr, MSGSTR(USAGE,
		"Usage:  date [-u] [+field descriptors]\n"));

#ifdef SEC_BASE
	if (authorized_user("sysadmin")) {
#else
	if (geteuid() == 0) {
#endif
	   ds_flg = switchdate();

	   sprintf(prompt,"%s""%s""%s""%s",
		   ds_flg ? MSGSTR(USAGEDAY,"dd") : MSGSTR(USAGEMONTH,"mm"),
		   ds_flg ? MSGSTR(USAGEMONTH,"mm") : MSGSTR(USAGEDAY,"dd"),
		   MSGSTR(USAGEHOUR,"HH"),MSGSTR(USAGEMINUTE,"MM"));

	   fprintf(stderr, MSGSTR(USAGEROOT3,
		"\tdate [-nu] [%s[yy]]\n"),prompt);
	   fprintf(stderr, MSGSTR(USAGEROOT,
		"\tdate [-nu] [%s[.SS[yy]]]\n"),prompt);
	   fprintf(stderr, MSGSTR(USAGEROOT2,
		"\tdate [-nu] [yy%s[.SS]]\n"),prompt);
	}
}
