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
static char *rcsid = "@(#)$RCSfile: strerr.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/10/08 16:12:51 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1991  Mentat Inc.
 ** strerr.c 1.2, last change 5/1/91
 **/

#include <stropts.h>
#include <sys/errno.h>
#include <sys/signal.h>
#include <sys/strlog.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mode.h>

#include <locale.h>
#include "strerr_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRERR,n,s)

#ifndef	LOG_DEV_NAME
#define	LOG_DEV_NAME			"/dev/streams/log"
#endif

#ifndef DEFAULT_DIRECTORY
#define	DEFAULT_DIRECTORY		"/var/adm/streams"
#endif

#ifndef DEFAULT_ADMIN_MAIL_NAME
#define	DEFAULT_ADMIN_MAIL_NAME		"root"
#endif

void init_direct();

extern	char	* optarg;
extern	int	optind;

main (argc, argv)
	int	argc;
	char	** argv;
{
	char	* admin_name;
	char	buf[LOGMSGSZ];
	int	c;
	char	ch;
	char	* cp;
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	char	* dir_name;
	int	log_fd;
	FILE	* fp;
	int	flags;
	struct log_ctl	logctl;
	struct strioctl	stri;
	struct tm	* tmp;
	time_t	time1;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_STRERR,NL_CAT_LOCALE);

	admin_name = DEFAULT_ADMIN_MAIL_NAME;
	dir_name = DEFAULT_DIRECTORY;
	while ((c = getopt(argc, argv, "a:A:d:D:")) != -1) {
		switch (c) {
		case 'a': case 'A':
			admin_name = optarg;
			break;
		case 'd': case 'D':	/* Directory name */
			dir_name = optarg;
			break;
		case '?':
		default:
			fprintf(stderr, MSGSTR( STRER_USAGE, 
		"usage: strerr [-a sys_admin_mail_name] [-d logdir]\n"));
			exit(1);
		}
	}

	/* Open a stream to the log device */
	if ((log_fd = open(LOG_DEV_NAME, 2)) == -1) {
		perror(MSGSTR( STRER_NOLOG, "couldn't open log device"));
		exit(1);
	}

	/* Register for error messages */
	stri.ic_cmd = I_ERRLOG;
	stri.ic_timout = -1;
	stri.ic_dp = (char *)0;
	stri.ic_len = 0;
	if (ioctl(log_fd, I_STR, (char *)&stri) == -1) {
		fprintf(stderr, MSGSTR( STRER_EALREADY, 
			"ERROR: strerr already running\n"));
		close(log_fd);
		exit(1);
	}

	/* Open the error file */
	(void) init_direct(dir_name);
	time(&time1);
	tmp = localtime(&time1);
	sprintf(buf, MSGSTR( STRER_FMT1, "%s/error.%02d-%02d"), 
		dir_name, tmp->tm_mon+1, tmp->tm_mday);
	if ((fp = fopen(buf, "a+")) == NULL) {
		perror(buf);
		close(log_fd);
		exit(1);
	}
	/* Become a daemon */
	if (daemon(0, 0)) {
		perror(MSGSTR( STRER_DAEMON, "daemon"));
		close(log_fd);
		exit(1);
	}
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	/* Now loop reading error messages and writing them to the error file */
	ctlbuf.buf = (char *)&logctl;
	ctlbuf.maxlen = sizeof(logctl);
	databuf.buf = buf;
	databuf.maxlen = sizeof(buf);
	flags = 0;
	while (getmsg(log_fd, &ctlbuf, &databuf, &flags) != -1) {
		time_t ttime = (time_t) logctl.ttime;
		cp = ctime(&ttime);
		cp[strlen(cp)-6] = '\0';
		cp += 11;
		if (logctl.flags & SL_FATAL)
			ch = 'F';
		else if (logctl.flags & SL_NOTIFY)
			ch = 'N';
		else if (logctl.flags & SL_TRACE)
			ch = 'T';
		else
			ch = ' ';
		fprintf(fp, "%d %s %lu %d %c", logctl.seq_no, cp,
			logctl.ltime, (int)logctl.level, ch);
		fprintf(fp, ".... %d %d %s\n", (int)logctl.mid, 
		    (int)logctl.sid, buf);
		fflush(fp);

		/* Send a mail message to the system administer */
		if (logctl.flags & SL_NOTIFY) {
			char	buf1[LOGMSGSZ * 2];
			char	cmd_buf[128];
			FILE	* fp;

			sprintf(cmd_buf, MSGSTR( STRER_MAIL, 
				"/usr/bin/mail %s"), admin_name);
			if (fp = popen(cmd_buf, "w")) {
				fprintf(fp, "%d %s %lu %d %c .... %d %d %s\n",
					logctl.seq_no, cp,
					logctl.ltime, (int)logctl.level, ch,
					(int)logctl.mid, (int)logctl.sid, buf);
				pclose(fp);
			}
		}
	}
}


/*
 * Create streams errlog directory if it does not exist
 */
void
init_direct(dir_name)
char *dir_name;
{
  struct stat stat_buf;
  int status;
  mode_t oumask;

  oumask = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

  status = stat(dir_name, &stat_buf);
  
  if (status != 0) {
    if (errno == ENOENT) {
      status = mkdir(dir_name, oumask);
      
      if (status != 0) {
	perror(MSGSTR( STRER_MKDIR, "strerr: couldn't mkdir errlog directory"));
        exit(2);
      }
    } else {
      perror(MSGSTR( STRER_STAT, "strerr: stat on errlog directory"));
      exit(1);
    }
  } 
  else 
    return;
  
}
