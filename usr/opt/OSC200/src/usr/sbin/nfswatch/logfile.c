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
static char *rcsid = "@(#)$RCSfile: logfile.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/12/21 23:35:37 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/logfile.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * logfile.c - routines for updating log and snapshot files
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: logfile.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.3  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.2  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.1  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.0  1991/01/23  08:23:05  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.2  90/08/17  15:47:22  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:34  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/time.h>
#include <curses.h>
#include <stdio.h>

#include "nfswatch.h"
#include "externs.h"
#include "screen.h"

/*
 * snapshot - take a snapshot of the screen.
 */
void
snapshot()
{
	FILE *fp;
	register int x, y;
	char buffer[BUFSIZ];

	/*
	 * We want to append to the snapshot file.
	 */
	if ((fp = fopen(snapshotfile, "a")) == NULL) {
		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"could not open \"%s\"", snapshotfile);
		(void) refresh();
		return;
	}

	/*
	 * For all lines but the last one ...
	 */
	for (y = 0; y < LINES - 1; y++) {
		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"dumping line %d", y + 1);
		(void) clrtoeol();
		(void) refresh();

		/*
		 * Search backwards on each line for a non-space.
		 * x is the count of significant characters.
		 */
		for (x = COLS - 1; x > 0; x--) {
			/*
			 *  Non-space found, increment Z and exit loop
			 */
			if (curscr->_y[y][x] != ' ') {
				x++;
				break;
			}
		}

		/*
		 * Copy x characters and append a \n and \0 to the
		 * string, then output it.
		 */
		(void) strncpy(buffer, curscr->_y[y], x);
		buffer[x++] = '\n';
		buffer[x] = '\0';

		(void) fputs(buffer, fp);
	}

	(void) fputc('\014', fp);
	(void) fclose(fp);

	/*
	 * Tell them we're done.
	 */
	(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
		"screen dumped to \"%s\"", snapshotfile );
	(void) refresh();
}

/*
 * update_logfile - put out the information to the log file.
 */
void
update_logfile()
{
	char *tstr;
	char *ctime();
	float percent;
	char buf[BUFSIZ];
	struct timeval tv;
	register int i, j, nfstotal;

	(void) gettimeofday(&tv, (struct timezone *) 0);

	/*
	 * Start a log entry.
	 */
	(void) fprintf(logfp, "#\n# begin\n#\n");
	(void) fprintf(logfp, "Date: %.24s\n", ctime(&tv.tv_sec));

	tv.tv_sec -= starttime.tv_sec;
	tstr = prtime(tv.tv_sec);

	(void) fprintf(logfp, "Cycle Time: %d\n", cycletime);
	(void) fprintf(logfp, "Elapsed Time: %.8s\n", tstr+11);

	/*
	 * Print total packet counters.
	 */
	(void) fprintf(logfp, "#\n# total packets     %8s %8s %8s\n#\n",
		"network", "to host", "dropped");
	(void) fprintf(logfp, "Interval Packets:   %8d %8d %8d\n",
		int_pkt_total, int_dst_pkt_total, int_pkt_drops);
	(void) fprintf(logfp, "Total Packets:      %8d %8d %8d\n",
		pkt_total, dst_pkt_total, pkt_drops);

	/*
	 * Put out a header for the packet counters.
	 */
	(void) fprintf(logfp, "#\n# packet counters         %8s %8s %8s\n#\n",
		"int", "pct", "total");

	/*
	 * Print the packet counters.  Percentage is calculated as
	 * this interval counter over total packets this interval.
	 */
	for (i = 0; i < PKT_NCOUNTERS; i++) {
		if (int_dst_pkt_total) {
			percent = ((float) pkt_counters[i].pc_interval /
				  (float) int_dst_pkt_total) * 100.0;
		}
		else {
			percent = 0.0;
		}

		(void) sprintf(buf, "%s:", pkt_counters[i].pc_name);
		(void) fprintf(logfp, "%-25s %8d %7.0f%% %8d\n",
			buf, pkt_counters[i].pc_interval, percent,
			pkt_counters[i].pc_total);
	}

	/*
	 * Calculate the total number of NFS packets this
	 * interval.
	 */
	nfstotal = pkt_counters[PKT_NFSWRITE].pc_interval +
		   pkt_counters[PKT_NFSREAD].pc_interval;

	/*
	 * Put out a header for the NFS counters.
	 */
	(void) fprintf(logfp, "#\n# nfs counters            %8s %8s %8s\n#\n",
		"int", "pct", "total");

	/*
	 * Print the NFS counters.  Percentage is calculated as
	 * packets this interval over total NFS packets this
	 * interval.
	 */
	for (i = 0; i < nnfscounters; i++) {
		if (nfstotal) {
			percent = ((float) nfs_counters[i].nc_interval /
				  (float) nfstotal) * 100.0;
		}
		else {
			percent = 0.0;
		}

		(void) sprintf(buf, "%s:", nfs_counters[i].nc_name);
		(void) fprintf(logfp, "%-25s %8d %7.0f%% %8d\t",
			buf, nfs_counters[i].nc_interval, percent,
			nfs_counters[i].nc_total);

		/*
		 * Print individual proc counters.
		 */
		(void) fputc('(', logfp);

		for (j = 0; j < MAXNFSPROC; j++) {
			if (j)
				(void) fputc('/', logfp);

			(void) fprintf(logfp, "%d", nfs_counters[i].nc_proc[j]);
		}

		(void) fprintf(logfp, ")\n");
	}

	/*
	 * Put out a header for the individual file counters.
	 */
	(void) fprintf(logfp, "#\n# file counters           %8s %8s %8s\n#\n",
		"int", "pct", "total");

	/*
	 * Print the individual file counters.  Percentage is
	 * calculated as packets this interval over total NFS
	 * packets this interval.
	 */
	for (i = 0; i < nfilecounters; i++) {
		if (nfstotal) {
			percent = ((float) fil_counters[i].fc_interval /
				  (float) nfstotal) * 100.0;
		}
		else {
			percent = 0.0;
		}

		(void) sprintf(buf, "%s:", fil_counters[i].fc_name);
		(void) fprintf(logfp, "%-25s %8d %7.0f%% %8d\t",
			buf, fil_counters[i].fc_interval, percent,
			fil_counters[i].fc_total);

		/*
		 * Print individual proc counters.
		 */
		(void) fputc('(', logfp);

		for (j = 0; j < MAXNFSPROC; j++) {
			if (j)
				(void) fputc('/', logfp);

			(void) fprintf(logfp, "%d", fil_counters[i].fc_proc[j]);
		}

		(void) fprintf(logfp, ")\n");
	}

	/*
	 * End the log entry.
	 */
	(void) fprintf(logfp, "#\n# end\n#\n");
	(void) fflush(logfp);
}
