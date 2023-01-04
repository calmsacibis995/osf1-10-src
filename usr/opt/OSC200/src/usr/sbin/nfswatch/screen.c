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
static char *rcsid = "@(#)$RCSfile: screen.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 19:23:32 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/screen.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * screen.c - routines for updating the screen.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: screen.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.8  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.7  1993/01/18  19:35:16  davy
 * Small patch from Jeff.
 *
 * Revision 3.6  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.5  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.4  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.3  1993/01/13  21:25:31  davy
 * Stupid portability change for IRIX.
 *
 * Revision 3.2  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.1  1993/01/13  14:35:44  davy
 * Added "n" command to toggle displaying host names/numbers.
 *
 * Revision 3.0  1991/01/23  08:23:23  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.4  91/01/17  10:13:30  davy
 * New features from Jeff Mogul.
 * 
 * Revision 1.7  91/01/16  15:49:41  mogul
 * Make sure snapshot captures up-to-date screen.
 * 
 * Revision 1.6  91/01/07  14:24:05  mogul
 * fixed a few little bugs
 * 
 * Revision 1.5  91/01/07  14:10:43  mogul
 * Added scrolling
 * Added help screen
 * 
 * Revision 1.4  91/01/04  14:13:29  mogul
 * Support for client counters
 * disable screen update during database upheaval
 * indicate how many files/file systems/procedures/clients not shown
 * 
 * Revision 1.3  91/01/03  17:42:16  mogul
 * Support for per-procedure counters
 * 
 * Revision 1.2  90/08/17  15:47:48  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:21:03  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <curses.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>

#include "nfswatch.h"
#include "externs.h"
#include "screen.h"

/*
 * Screen text items which are always displayed.
 */
static struct scrtxt scrtxts[] = {
	SCR_HOST_X,	SCR_HOST_Y,	dsthost,
	SCR_ELAPS_X0,	SCR_ELAPS_Y,	"Elapsed time:",
	SCR_ELAPS_X,	SCR_ELAPS_Y,	"00:00:00",
	SCR_PKTINT_X0,	SCR_PKTINT_Y,	"Interval packets:",
	SCR_PKTTOT_X0,	SCR_PKTTOT_Y,	"Total packets:",
	SCR_PKTHDR_X,	SCR_PKTHDR_Y,	"int   pct   total",
	SCR_PKTHDR_X+SCR_MIDDLE,	SCR_PKTHDR_Y,
					"int   pct   total",
	-1,		-1,		NULL
};

/*
 * Screen text items displayed when showing file systems
 * (showwhich == SHOWFILESYSTEM).
 */
static struct scrtxt fstxts[] = {
	SCR_NFSHDR_X,	SCR_NFSHDR_Y,	"File Sys        int   pct   total",
	SCR_NFSHDR_X+SCR_MIDDLE,	SCR_NFSHDR_Y,
					"File Sys        int   pct   total",
	-1,		-1,		NULL
};

/*
 * Screen text items displayed when showing individual files
 * (showwhich == SHOWINDVFILES).
 */
static struct scrtxt fitxts[] = {
	SCR_NFSHDR_X,	SCR_NFSHDR_Y,	"File            int   pct   total",
	SCR_NFSHDR_X+SCR_MIDDLE,	SCR_NFSHDR_Y,
					"File            int   pct   total",
	-1,		-1,		NULL
};

/*
 * Screen text items displayed when showing individual procedures
 * (showwhich == SHOWNFSPROCS).
 */
static struct scrtxt prtxts[] = {
	0,		SCR_NFSHDR_Y,
		" Procedure           int   pct   total  completed  ave.resp  var.resp  max.resp",
	-1,		-1,		NULL
};

/*
 * Screen text items displayed when showing individual clients
 * (showwhich == SHOWCLIENTS).
 */
static struct scrtxt cltxts[] = {
	0,		SCR_NFSHDR_Y,
		"Client host          int   pct   total",
	SCR_MIDDLE,	SCR_NFSHDR_Y,
		"Client host          int   pct   total",
	-1,		-1,		NULL
};

/*
 * Screen text items displayed when showing individual authenticators
 * (showwhich == SHOWAUTH).
 */
static struct scrtxt actxts[] = {
	0,		SCR_NFSHDR_Y,
		"Authenticator        int   pct   total",
	SCR_MIDDLE,	SCR_NFSHDR_Y,
		"Authenticator        int   pct   total",
	-1,		-1,		NULL
};

/*
 * Screen text items for help page (showwhich == SHOWHELP).
 */
#define	SCR_HELPHDR_Y (SCR_NFSHDR_Y - 1)
static struct scrtxt helptxts[] = {
	2,		SCR_HELPHDR_Y+0, "^L\tRedraw screen",
	3,		SCR_HELPHDR_Y+1, "a\tDisplay RPC authentication",
	3,		SCR_HELPHDR_Y+2, "c\tDisplay NFS client hosts",
	3,		SCR_HELPHDR_Y+3, "f\tDisplay file systems",
	3,		SCR_HELPHDR_Y+4, "l\tToggle logging",
	3,		SCR_HELPHDR_Y+5, "n\tToggle host numbers/names",
	3,		SCR_HELPHDR_Y+6, "p\tDisplay NFS procedures",
	3,		SCR_HELPHDR_Y+7, "q\tQuit",
	3,		SCR_HELPHDR_Y+8, "s\tWrite snapshot",

	SCR_MIDDLE+3,	SCR_HELPHDR_Y+0, "u\tToggle sort by % usage",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+1, ">\tIncrease cycle time by one sec",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+2, "<\tDecrease cycle time by one sec",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+3, "+\tIncrease cycle time by 10 secs",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+4, "-\tDecrease cycle time by 10 secs",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+5, "=\tReset cycle time to 10 secs",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+6, "]\tScroll forward",
	SCR_MIDDLE+3,	SCR_HELPHDR_Y+7, "[\tScroll back",
	SCR_MIDDLE,	SCR_HELPHDR_Y+8, " \t*space bar resumes display*",

	SCR_MIDDLE - 6,	SCR_HELPHDR_Y+9, "--COMMANDS--",

	-1,		-1,		NULL
};

static int showbyname = 1;	/* show host names/numbers	*/

static int prev_showwhich = 0;	/* used for resuming after help */

/*
 * scrolling offsets, one for each display type, in terms of displayed
 * items (i.e., lines skipped * 2)
 */
static int scroll_off[SHOW_MAXCODE+1];

/*
 * This array tells the scrolling mechanism how far a display can be scrolled;
 *	NULL entries are not scrollable.
 */
static int maxnfsproc = MAXNFSPROC;
static int *scroll_limit[SHOW_MAXCODE+1] = {
		NULL,
		&nfilecounters, &nnfscounters, &maxnfsproc, &nclientcounters,
		&nauthcounters, NULL
};

/*
 * setup_screen - initialize the display screen.
 */
void
setup_screen(device)
char *device;
{
	int len;
	char *tstr;
	char *ctime();
	register int i;
	char buf[BUFSIZ];
	struct timeval tv;
	register struct scrtxt *st;

#ifdef ultrix
	struct winsize ws;

	/*
	 * Get around the old version of curses Ultrix has.
	 * -lcursesX would probably do this as well.
	 */
	if (ioctl(2, TIOCGWINSZ, &ws) >= 0)
		LINES = ws.ws_row;
#endif

	/*
	 * Initialize the screen.
	 */
	(void) initscr();
	screen_inited = 1;

	/*
	 * Turn echo off, cbreak on.
	 */
	(void) noecho();

#ifdef crmode
	(void) crmode();
#else
	(void) cbreak();
#endif

	/*
	 * Clear the screen.
	 */
	(void) clear();

	/*
	 * Get the time of day.
 	 */
	(void) gettimeofday(&tv, 0);
	tstr = ctime(&tv.tv_sec);

	(void) mvprintw(SCR_DATE_Y, SCR_DATE_X, "%.24s", tstr);

	/*
	 * Now draw the various strings on the screen.
	 */
	for (st = scrtxts; st->s_text != NULL; st++)
		(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

	if (allintf)
		(void) sprintf(buf, "Monitoring packets from all interfaces");
	else
		(void) sprintf(buf, "Monitoring packets from interface %s",
			       device);

	len = strlen(buf);

	(void) mvprintw(SCR_IF_Y, SCR_MIDDLE - (len/2), buf);
	(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X0, prompt);
}

/*
 * label_screen - put all packet counter labels on screen.
 */
void
label_screen()
{
	register int i;
	register struct scrtxt *st;
	register int soff = scroll_off[showwhich];

	/*
	 * Display packet counter labels.
	 */
	for (i = 0; i < PKT_NCOUNTERS; i++) {
		(void) mvprintw(pkt_counters[i].pc_namey,
			pkt_counters[i].pc_namex, "%.*s",
			SCR_PKTLEN, pkt_counters[i].pc_name);
	}

	/*
	 * Clear NFS packet counter lines, since we may be
	 * changing them.
	 */
	for (i = SCR_NFSHDR_Y; i < (LINES - 1); i++) {
		(void) move(i, 0);
		(void) clrtoeol();
	}

	/*
	 * Display the labels for the NFS packet counter lines.
	 */
	if (showwhich == SHOWFILESYSTEM) {
		for (st = fstxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

		for (i = soff;
			(i < nnfscounters) && ((i - soff) < NFSLINES); i++) {
			(void) mvprintw(nfs_counters[i - soff].nc_namey,
				nfs_counters[i - soff].nc_namex, "%.*s",
				SCR_NFSLEN, nfs_counters[i].nc_name);
		}
	}
	else if (showwhich == SHOWNFSPROCS) {
		for (st = prtxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

		for (i = soff;
			(i < MAXNFSPROC) && ((i - soff) < NFSLINES); i++) {
			(void) mvprintw(prc_counters[i - soff].pr_namey,
					prc_counters[i - soff].pr_namex,
					"%.*s", SCR_NFSLEN,
					prc_counters[i].pr_name);
		}
	}
	else if (showwhich == SHOWCLIENTS) {
		for (st = cltxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

		for (i = soff; 
		      (i < nclientcounters) && ((i - soff) < NFSLINES); i++) {
			if (showbyname) {
			    (void) mvprintw(clnt_counters[i - soff].cl_namey,
					clnt_counters[i - soff].cl_namex,
					"%.*s", SCR_NFSLEN,
					clnt_counters[i].cl_name);
			}
			else {
			    struct in_addr tmp;

			    bcopy(&clnt_counters[i].cl_ipaddr, &tmp, sizeof(long));
			    (void) mvprintw(clnt_counters[i - soff].cl_namey,
					clnt_counters[i - soff].cl_namex,
					"%.*s", SCR_NFSLEN,
					inet_ntoa(tmp));
			}
		}
	}
	else if (showwhich == SHOWAUTH) {
		for (st = actxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

		for (i = soff;
		     (i < nauthcounters) && ((i - soff) < NFSLINES); i++) {
			(void) mvprintw(auth_counters[i - soff].ac_namey,
					auth_counters[i - soff].ac_namex,
					"%.*s", SCR_NFSLEN,
					auth_counters[i].ac_name);
		}
	}
	else if (showwhich == SHOWHELP) {
		/* We "stole" one line so we must clear it */
		(void) move(SCR_HELPHDR_Y, 0);
		(void) clrtoeol();

		for (st = helptxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);
	}
	else {
		for (st = fitxts; st->s_text != NULL; st++)
			(void) mvprintw(st->s_y, st->s_x, "%s", st->s_text);

		for (i = soff;
			(i < nfilecounters) && ((i - soff) < NFSLINES); i++) {
			(void) mvprintw(fil_counters[i - soff].fc_namey,
				fil_counters[i - soff].fc_namex, "%.*s",
				SCR_NFSLEN, fil_counters[i].fc_name);
		}
	}
}

/*
 * update_screen - update the screen with new information.
 */
void
update_screen()
{
	char *tstr;
	char *ctime();
	float percent;
	struct timeval tv;
	register int i, nfstotal;
	register int soff = scroll_off[showwhich];

	(void) gettimeofday(&tv, (struct timezone *) 0);

	(void) mvprintw(SCR_DATE_Y, SCR_DATE_X, "%.24s", ctime(&tv.tv_sec));

	tv.tv_sec -= starttime.tv_sec;
	tstr = prtime(tv.tv_sec);

	/*
	 * Print the headers.
	 */
	(void) mvprintw(SCR_ELAPS_Y, SCR_ELAPS_X, "%.8s", tstr);
	(void) mvprintw(SCR_PKTINT_Y, SCR_PKTINT_X,
		"%8d (network)   %8d (to host)   %8d (dropped)",
		int_pkt_total, int_dst_pkt_total, int_pkt_drops);
	(void) mvprintw(SCR_PKTTOT_Y, SCR_PKTTOT_X,
		"%8d (network)   %8d (to host)   %8d (dropped)",
		pkt_total, dst_pkt_total, pkt_drops);

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

		(void) mvprintw(pkt_counters[i].pc_inty,
			pkt_counters[i].pc_intx, "%5d",
			pkt_counters[i].pc_interval);

		(void) mvprintw(pkt_counters[i].pc_pcty,
			pkt_counters[i].pc_pctx, "%3.0f%%",
			percent);

		(void) mvprintw(pkt_counters[i].pc_toty,
			pkt_counters[i].pc_totx, "%8d",
			pkt_counters[i].pc_total);
	}

	/*
	 * Calculate the total number of NFS packets this
	 * interval.
	 */
	nfstotal = pkt_counters[PKT_NFSWRITE].pc_interval +
		   pkt_counters[PKT_NFSREAD].pc_interval;

	if (showwhich == SHOWFILESYSTEM) {
		/*
		 * Print the NFS counters.  Percentage is calculated as
		 * packets this interval over total NFS packets this
		 * interval.
		 */
		HowMany("file systems", nnfscounters);

		for (i = soff;
			(i < nnfscounters) && ((i - soff) < NFSLINES); i++) {
			if (nfstotal) {
			    percent = ((float) nfs_counters[i].nc_interval /
					  (float) nfstotal) * 100.0;
			}
			else {
				percent = 0.0;
			}

			(void) mvprintw(nfs_counters[i - soff].nc_inty,
				nfs_counters[i - soff].nc_intx, "%5d",
				nfs_counters[i].nc_interval);

			(void) mvprintw(nfs_counters[i - soff].nc_pcty,
				nfs_counters[i - soff].nc_pctx, "%3.0f%%",
				percent);

			(void) mvprintw(nfs_counters[i - soff].nc_toty,
				nfs_counters[i - soff].nc_totx, "%8d",
				nfs_counters[i].nc_total);
		}
	}
	else if (showwhich == SHOWNFSPROCS) {
		long count, tot_count;
		double dcount, sumsqr, sum;

		tot_count = 0;

		/* Sum up all over all the procedures  */
		for (i = 0; i < MAXNFSPROC; i++) {
			tot_count += prc_counters[i].pr_interval;
		}

		HowMany("NFS Procedures", MAXNFSPROC);

		for (i = soff;
			(i < MAXNFSPROC) && ((i - soff) < NFSLINES); i++) {
		    if (tot_count)
			percent = (((float) prc_counters[i].pr_interval)
					 / ((float)tot_count)) * 100.0;
		    else
			percent = 0.0;

		    (void) mvprintw(prc_counters[i - soff].pr_inty,
			    prc_counters[i - soff].pr_intx, "%5d",
			    prc_counters[i].pr_interval);

		    (void) mvprintw(prc_counters[i - soff].pr_pcty,
			    prc_counters[i - soff].pr_pctx, "%3.0f%%",
			    percent);

		    (void) mvprintw(prc_counters[i - soff].pr_toty,
			    prc_counters[i - soff].pr_totx, "%8d",
			    prc_counters[i].pr_total);

		    count = prc_counters[i].pr_complete;

		    if (count != 0) {
			dcount = (double) count;
			sum = prc_counters[i].pr_response;
			sumsqr = prc_counters[i].pr_respsqr;

			(void) mvprintw(prc_counters[i - soff].pr_rmaxy,
					prc_counters[i - soff].pr_rmaxx,
					"%8.2f",
					prc_counters[i].pr_maxresp);

			(void) mvprintw(prc_counters[i - soff].pr_compy,
					prc_counters[i - soff].pr_compx,
					"%8d",
					count);

			(void) mvprintw(prc_counters[i - soff].pr_respy,
					prc_counters[i - soff].pr_respx,
					"%8.2f",
					sum / dcount);

			if (count > 1) {
				(void) mvprintw(prc_counters[i - soff].pr_rsqry,
						prc_counters[i - soff].pr_rsqrx,
						"%8.2f",
						sqrt((dcount * sumsqr - sum * sum) /
						     (dcount * (dcount - 1.0))));
			}
		    }
		}
	}
	else if (showwhich == SHOWCLIENTS) {
		long tot_count;

		tot_count = 0;

		/* Sum up all over all the clients  */
		for (i = 0; i < nclientcounters; i++) {
			tot_count += clnt_counters[i].cl_interval;
		}

		HowMany("client hosts", nclientcounters);

		for (i = soff;
		     (i < nclientcounters) && ((i - soff) < NFSLINES); i++) {
		    if (tot_count)
			percent = (((float) clnt_counters[i].cl_interval)
					 / ((float)tot_count)) * 100.0;
		    else
			percent = 0.0;

		    (void) mvprintw(clnt_counters[i - soff].cl_inty,
			    clnt_counters[i - soff].cl_intx, "%5d",
			    clnt_counters[i].cl_interval);

		    (void) mvprintw(clnt_counters[i - soff].cl_pcty,
			    clnt_counters[i - soff].cl_pctx, "%3.0f%%",
			    percent);

		    (void) mvprintw(clnt_counters[i - soff].cl_toty,
			    clnt_counters[i - soff].cl_totx, "%8d",
			    clnt_counters[i].cl_total);
		}
	}
	else if (showwhich == SHOWAUTH) {
		long tot_count;

		tot_count = 0;

		/*
		 * Sum up over all the authenticators.
		 */
		for (i=0; i < nauthcounters; i++) {
			tot_count += auth_counters[i].ac_interval;
		}

		HowMany("authenticators", nauthcounters);

		for (i = soff;
		     (i < nauthcounters) && ((i - soff) < NFSLINES); i++) {
			if (tot_count)
				percent = (((float) auth_counters[i].ac_interval)
					   / ((float) tot_count)) * 100.0;
			else
				percent = 0.0;

			(void) mvprintw(auth_counters[i - soff].ac_inty,
					auth_counters[i - soff].ac_intx,
					"%5d",
					auth_counters[i].ac_interval);

			(void) mvprintw(auth_counters[i - soff].ac_pcty,
					auth_counters[i - soff].ac_pctx,
					"%3.0f%%",
					percent);

			(void) mvprintw(auth_counters[i - soff].ac_toty,
					auth_counters[i - soff].ac_totx,
					"%8d",
					auth_counters[i].ac_total);
		}
	}
	else if (showwhich == SHOWHELP) {
		/* do nothing */
	}
	else {
		/*
		 * Print the file counters.  Percentage is calculated as
		 * packets this interval over total NFS packets this
		 * interval.
		 */
		HowMany("files", nfilecounters);

		for (i = soff;
			(i < nfilecounters) && ((i - soff) < NFSLINES); i++) {
			if (nfstotal) {
			    percent = ((float) fil_counters[i].fc_interval /
					  (float) nfstotal) * 100.0;
			}
			else {
				percent = 0.0;
			}

			(void) mvprintw(fil_counters[i - soff].fc_inty,
				fil_counters[i - soff].fc_intx, "%5d",
				fil_counters[i].fc_interval);

			(void) mvprintw(fil_counters[i - soff].fc_pcty,
				fil_counters[i - soff].fc_pctx, "%3.0f%%",
				percent);

			(void) mvprintw(fil_counters[i - soff].fc_toty,
				fil_counters[i - soff].fc_totx, "%8d",
				fil_counters[i].fc_total);
		}
	}

	(void) move(SCR_PROMPT_Y, SCR_PROMPT_X);
	(void) clrtoeol();
	(void) refresh();
}

/*
 * HowMany - put up a label showing how many XXs known/not displayed
 */
HowMany(legend, number)
char *legend;
int number;
{
	static char message[80];
	int mlen;
	
	if (number > NFSLINES) {
	    sprintf(message, "%d %s [%d not displayed]", number, legend,
			number - NFSLINES);
	}
	else
	    sprintf(message, "%d %s", number, legend);

	mlen = strlen(message);
	(void)move(SCR_NFSHDR_Y - 1, 0);
	(void)clrtoeol();
	(void)mvprintw(SCR_NFSHDR_Y - 1, SCR_MIDDLE - (mlen/2), message);
	
	/*
	 * Display scrolling information
	 */
	if (scroll_off[showwhich] > 0)
	    (void)mvprintw(SCR_NFSHDR_Y - 1, 2, "<-more");
	if (scroll_limit[showwhich]) {
	    int slim = *scroll_limit[showwhich];
	    if ((scroll_off[showwhich] + NFSLINES) < slim)
		(void)mvprintw(SCR_NFSHDR_Y - 1, 
				(SCR_MIDDLE * 2) - 8, "more->");
	}
}


/*
 * command - process a command.
 */
void
command()
{
	register int c;
	struct itimerval itv;
	int oldm;
	int reset_prevshowwhich = 1;

	if ((c = getchar()) == EOF)
		finish(-1);

	c &= 0177;

	switch (c) {
	case '\014':			/* redraw			*/
		(void) clearok(curscr, TRUE);
		(void) refresh();
		reset_prevshowwhich = 0;
		break;
	case 'a':			/* show RPC authentication	*/
	case 'A':
		showwhich = SHOWAUTH;
		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"display RPC authenticators (starting next update)");

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change screen labels.
		 */
		label_screen();
		break;
	case 'c':			/* show NFS clients		*/
	case 'C':
		showwhich = SHOWCLIENTS;
		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"display NFS clients (starting next update)");

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change screen labels.
		 */
		label_screen();
		break;
	case 'f':			/* toggle what we show		*/
	case 'F':
		switch (showwhich) {
		case SHOWFILESYSTEM:
		    /*
		     * Only do this if it makes sense.
		     */
		    if (filelist == NULL)
			break;
		    showwhich = SHOWINDVFILES;
		    break;
		case SHOWINDVFILES:
		    showwhich = SHOWFILESYSTEM;
		    break;
		case SHOWNFSPROCS:
		case SHOWCLIENTS:
		case SHOWHELP:
		    showwhich = SHOWFILESYSTEM;
		    break;
		}

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"display %s (starting next update)",
				(showwhich == SHOWFILESYSTEM ?
				"file systems" : "individual files"));

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change screen labels.
		 */
		label_screen();
		break;
	case 'l':			/* toggle logging		*/
	case 'L':
		reset_prevshowwhich = 0;
		if (logging) {
			(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"logging turned off");

			(void) fprintf(logfp, "#\n# endlog\n#\n");
			(void) fclose(logfp);
			logging = 0;
		}
		else {
			if ((logfp = fopen(logfile, "a")) == NULL) {
				(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
					"could not open \"%s\"", logfile);
			}
			else {
				(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
					"logging to \"%s\"", logfile);

				(void) fprintf(logfp, "#\n# startlog\n#\n");
				(void) fprintf(logfp, "# NFSwatch log file\n");
				(void) fprintf(logfp, "#    Packets from: %s\n",
					(srcflag ? srchost : "all hosts"));
				(void) fprintf(logfp, "#    Packets to:   %s\n#\n",
					dsthost);

				logging = 1;
			}
		}

		(void) clrtoeol();
		(void) refresh();
		break;
	case 'n':			/* show numbers/names		*/
	case 'N':
		showbyname = (!showbyname);

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"show host %s (starting next update)",
				showbyname ? "names" : "numbers");

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change screen labels.
		 */
		label_screen();
		break;
	case 'p':			/* show NFS procedures		*/
	case 'P':
		showwhich = SHOWNFSPROCS;
		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"display NFS procedures (starting next update)");

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change screen labels.
		 */
		label_screen();
		break;
	case 'q':			/* quit				*/
	case 'Q':
		(void) mvaddch(SCR_PROMPT_Y, SCR_PROMPT_X, c);
		(void) refresh();
		finish(-1);
		break;
	case 's':			/* snapshot			*/
	case 'S':
		reset_prevshowwhich = 0;
#ifdef SVR4
		sighold(SIGALRM);
#else
		oldm = sigblock(sigmask(SIGALRM));
#endif
	    				/* no async redisplay while dumping */
		snapshot();
#ifdef SVR4
		sigrelse(SIGALRM);
#else
		(void) sigsetmask(oldm);	/* permit async redisplay */
#endif
		break;
	case 'u':			/* toggle sort method		*/
	case 'U':
		reset_prevshowwhich = 0;
		sortbyusage = (!sortbyusage);

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"sort by %s (starting next update)",
				sortbyusage ? "percent usage" : "name");

		(void) clrtoeol();
		(void) refresh();

		/*
		 * Change the sort order now.
		 */
#ifdef SVR4
		sighold(SIGALRM);
#else
		oldm = sigblock(sigmask(SIGALRM));
#endif
	    				/* no redisplay while unstable */
		sort_nfs_counters();
		sort_prc_counters();
		sort_clnt_counters();
		sort_auth_counters();
#ifdef SVR4
		sigrelse(SIGALRM);
#else
		(void) sigsetmask(oldm);	/* permit redisplay */
#endif

		/*
		 * Update screen labels and values.
		 */
		label_screen();
		update_screen();
		break;
	case '>':			/* add 1 to cycle time		*/
		reset_prevshowwhich = 0;
		cycletime++;

		(void) getitimer(ITIMER_REAL, &itv);

		itv.it_interval.tv_sec = cycletime;
		itv.it_interval.tv_usec = 0;

		(void) setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"cycletime = %d seconds", cycletime);
		(void) clrtoeol();
		(void) refresh();
		break;
	case '<':			/* subtract 1 from cycletime	*/
		reset_prevshowwhich = 0;
		if (cycletime > 1) {
			cycletime--;

			(void) getitimer(ITIMER_REAL, &itv);

			itv.it_interval.tv_sec = cycletime;
			itv.it_interval.tv_usec = 0;

			(void) setitimer(ITIMER_REAL, &itv,
				(struct itimerval *) 0);

			(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"cycletime = %d seconds", cycletime);
			(void) clrtoeol();
			(void) refresh();
		}

		break;
	case '+':			/* increase cycletime		*/
		reset_prevshowwhich = 0;
		cycletime += CYCLETIME;

		(void) getitimer(ITIMER_REAL, &itv);

		itv.it_interval.tv_sec = cycletime;
		itv.it_interval.tv_usec = 0;

		(void) setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"cycletime = %d seconds", cycletime);
		(void) clrtoeol();
		(void) refresh();
		break;
	case '-':			/* decrease cycletime		*/
		reset_prevshowwhich = 0;
		if (cycletime > CYCLETIME) {
			cycletime -= CYCLETIME;

			(void) getitimer(ITIMER_REAL, &itv);

			itv.it_interval.tv_sec = cycletime;
			itv.it_interval.tv_usec = 0;

			(void) setitimer(ITIMER_REAL, &itv,
				(struct itimerval *) 0);

			(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
				"cycletime = %d seconds", cycletime);
			(void) clrtoeol();
			(void) refresh();
		}

		break;
	case '=':			/* reset cycletime		*/
		reset_prevshowwhich = 0;
		cycletime = CYCLETIME;

		(void) getitimer(ITIMER_REAL, &itv);

		itv.it_interval.tv_sec = cycletime;
		itv.it_interval.tv_usec = 0;

		(void) setitimer(ITIMER_REAL, &itv, (struct itimerval *) 0);

		(void) mvprintw(SCR_PROMPT_Y, SCR_PROMPT_X,
			"cycletime reset to %d seconds", cycletime);
		(void) clrtoeol();
		(void) refresh();
		break;
	case ']':			/* scroll forward		*/
		reset_prevshowwhich = 0;
		{
		    int slim;
		    if (scroll_limit[showwhich] == NULL)
			break;
		    slim = *(scroll_limit[showwhich]);
		    if (slim <= NFSLINES)
			break;
		    scroll_off[showwhich] += NFSLINES - 2;
		    if (scroll_off[showwhich] > (slim - NFSLINES)) {
			scroll_off[showwhich] = (slim - NFSLINES);
		    }
		}

		(void) clrtoeol();
		label_screen();
		update_screen();
		(void) refresh();

		break;
	case '[':			/* scroll back			*/
		reset_prevshowwhich = 0;
		scroll_off[showwhich] -= NFSLINES - 2;
		if (scroll_off[showwhich] < 0)
		    scroll_off[showwhich] = 0;

		(void) clrtoeol();
		label_screen();
		update_screen();
		(void) refresh();

		break;
	case ' ':			/* resume after help */
		if (prev_showwhich > 0) {
		    showwhich = prev_showwhich;
		    label_screen();
		    update_screen();
		    (void) refresh();
		    break;
		}
		/* not set?  drops through to "default" */
	default:			/* give them some help		*/
		reset_prevshowwhich = 0;

		if (showwhich != SHOWHELP)	/* save current display mode */
			prev_showwhich = showwhich;

		showwhich = SHOWHELP;
		
		label_screen();
		(void) refresh();

		break;
	}

	if (reset_prevshowwhich)
		prev_showwhich = 0;
}
