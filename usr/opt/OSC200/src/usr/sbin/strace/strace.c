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
static char *rcsid = "@(#)$RCSfile: strace.c,v $ $Revision: 4.2.4.5 $ (DEC) $Date: 1993/10/08 16:01:56 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990  Mentat Inc.
 ** strace.c 2.1, last change 11/14/90
 **/

#include <stropts.h>
#include <sys/errno.h>
#include <sys/strlog.h>
#include <time.h>
#include <stdio.h>

#include <locale.h>
#include "strace_msg.h"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_STRACE,n,s)

#ifndef LOG_DEV_NAME
#define LOG_DEV_NAME	"/dev/streams/log"
#endif

main (argc, argv)
	int	argc;
	char	** argv;
{
	char	buf[LOGMSGSZ];
	char	ch;
	char	* cp;
	char	** cpp;
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	int	fd;
	int	flags;
	int	i1;
	struct log_ctl	logctl;
	struct strioctl	stri;
	struct trace_ids trace1;

	setlocale( LC_ALL, "" );
	catd = catopen(MF_STRACE,NL_CAT_LOCALE);

	if ((fd = open(LOG_DEV_NAME, 2)) == -1) {
		perror(MSGSTR( STRACE_NOLOG, "couldn't open log device"));
		exit(1);
	}
        stri.ic_cmd = I_TRCLOG;
     	stri.ic_timout = -1;
	stri.ic_dp = (char *)&trace1;
	stri.ic_len = sizeof(trace1);
	if (argc > 1) {
		if (((argc - 1) % 3) != 0) {
			fprintf(stderr, MSGSTR( STRACE_USE, 
				"usage: [mod_ID sub_ID pri_level] ...\n"));
			exit(1);
		}
		for (i1 = 1; i1 < argc; i1 += 3) {
		        cpp = &argv[i1];
			if (strcmp(cpp[0], "all") == 0)
				trace1.ti_mid = -1;
			else
				trace1.ti_mid = atoi(cpp[0]);
			if (strcmp(cpp[1], "all") == 0)
				trace1.ti_sid = -1;
			else
				trace1.ti_sid = atoi(cpp[1]);
			if (strcmp(cpp[2], "all") == 0)
				trace1.ti_level = (char)-1;
			else
				trace1.ti_level = atoi(cpp[2]);
			if (ioctl(fd, I_STR, &stri) == -1) {
				fprintf(stderr, MSGSTR( STRACE_RUN, 
					"ERROR: strace already running\n"));
				exit(1);
			}
		}
	} else {
		trace1.ti_mid = -1;
		trace1.ti_sid = -1;
		trace1.ti_level = (char)-1;
		if (ioctl(fd, I_STR, &stri) == -1) {
			fprintf(stderr, MSGSTR( STRACE_RUN, 
				"ERROR: strace already running\n"));
			exit(1);
		}
	}
	ctlbuf.buf = (char *)&logctl;
	ctlbuf.maxlen = sizeof(logctl);
	databuf.buf = buf;
	databuf.maxlen = sizeof(buf);
	flags = 0;
	while (getmsg(fd, &ctlbuf, &databuf, &flags) != -1) {
		time_t ttime = (time_t) logctl.ttime;
		cp = ctime(&ttime);
		cp[strlen(cp)-6] = '\0';
		cp += 11;
		if (logctl.flags & SL_FATAL)
			ch = 'F';
		else if (logctl.flags & SL_NOTIFY)
			ch = 'N';
		else if (logctl.flags & SL_ERROR)
			ch = 'E';
		else
			ch = ' ';
		printf("%d %s %lu %d %c", logctl.seq_no, cp,
			logctl.ltime, (int)logctl.level, ch);
		printf(".... %d %d %s\n", 
			(int)logctl.mid, (int)logctl.sid, buf);
		fflush(stdout);
	}
}
