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
static char	*sccsid = "@(#)$RCSfile: headers.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:52 $";
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
/*
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/* support routines for screen headers */

#include "userif.h"
#include "curs_supp.h"

#define LITEM 1
#define CITEM 2
#define HITEM 3
#define PITEM 4
#define RITEM 5

#define IPLEFTS(S,I)	if (not_vars(I)) LEFTS(S, I)
#define IPCENTERS(S,I)	if (not_vars(I)) CENTERS(S, I)
#define IPCENTERH(S,I)	if (not_vars(I)) CENTERH(S, I)
#define IPRIGHTS(S,I)	if (not_vars(I)) RIGHTS(S, I)

/*
 * old lengths of hdrs & footers
 */

static int olh = 0, och = 0, orh = 0, olf = 0, ocf = 0, orf = 0, oti = 0;
static int oc3 = 0, oc2 = 0, oc1 = 0;

extern uchar *updhfv();
extern char *Malloc();
static void clearout();
extern void cursor();


/*
 * headers (Scrn_parms spp) - put up headers, footers & screen title
 *
 */

headers(spp)
Scrn_parms *spp;
{
	Scrn_parms *sp = spp;
	Scrn_hdrs *sh = sp->sh;
	uchar *ip;		/* item ptr */

	if (sh != NULL) {
		if (olh)
			clearout(olh, HDR_LINE, LITEM);
		if (sh->lh) {
			ip = updhfv(sh->lh);
			IPLEFTS(HDR_LINE, ip);
			olh = strlen(ip);
			if (ip != sh->lh) {
				Free(ip);
			}
		} else
			olh = 0;
		if (och)
			clearout(och, HDR_LINE, CITEM);
		if (sh->ch) {
			ip = updhfv(sh->ch);
			IPCENTERS(HDR_LINE, ip);
			och = strlen(ip);
			if (ip != sh->ch) {
				Free(ip);
			}
		} else
			och = 0;

		if (orh)
			clearout(orh, HDR_LINE, RITEM);
		if (sh->rh) {
			ip = updhfv(sh->rh);
			IPRIGHTS(HDR_LINE, ip);
			orh = strlen(ip);
			if (ip != sh->rh) {
				Free(ip);
			}
		} else
			orh = 0;

		cmd_lines(sh);

		if (oti)
			clearout(oti, TITLE_LINE, HITEM);
		if (sh->title) {
			ip = updhfv(sh->title);
			IPCENTERH(TITLE_LINE, ip);
			oti = strlen(ip);
			if (ip != sh->title)
				Free(ip);
		} else
			oti = 0;


		if (olf)
			clearout(olf, FTR_LINE, LITEM);
		if (sh->lf) {
			ip = updhfv(sh->lf);
			IPLEFTS(FTR_LINE, ip);
			olf = strlen(ip);
			if (ip != sh->lf) {
				Free(ip);
			}
		} else
			olf = 0;

		if (ocf)
			clearout(ocf, FTR_LINE, CITEM);
		if (sh->cf) {
			ip = updhfv(sh->cf);
			IPCENTERS(FTR_LINE, ip);
			ocf = strlen(ip);
			if (ip != sh->cf) {
				Free(ip);
			}
		} else
			ocf = 0;

		if (orf)
			clearout(orf, FTR_LINE, RITEM);
		if (sh->rf) {
			ip = updhfv(sh->rf);
			IPRIGHTS(FTR_LINE, ip);
			orf = strlen(ip);
			if (ip != sh->rf) {
				Free(ip);
			}
		} else
			orf = 0;
	}
}



cmd_lines(sh)
Scrn_hdrs *sh;
{
	if (oc3)
		clearout(oc3, CMDS_LINE3, PITEM);
	if (sh->c3) {
		CENTERP(CMDS_LINE3, sh->c3);
		oc3 = strlen(sh->c3);
	} else
		oc3 = 0;

	if (oc2)
		clearout(oc2, CMDS_LINE2, PITEM);
	if (sh->c2) {
		CENTERP(CMDS_LINE2, sh->c2);
		oc2 = strlen(sh->c2);
	} else
		oc2 = 0;

	if (oc1)
		clearout(oc1, CMDS_LINE1, PITEM);
	if (sh->c1) {
		CENTERP(CMDS_LINE1, sh->c1);
		oc1 = strlen(sh->c1);
	} else
		oc1 = 0;
}

/*
 * Logic for updating headers.
 * Just before entering input mode, the program checks whether the screen
 * needs to be updated for time.
 * If so, it updates the time and puts the cursor back by calling
 * reenterfield().  If it's in input mode, the signal catcher for SIGALRM
 * knows it's ok to redraw the screen.  Otherwise, it just updates a flag
 * and waits until it's time to update the screen.
 */

static int alarm_happened;
static struct state *saved_stp;
static int in_input_mode;

void
scrn_enter_input_mode(stp)
struct state *stp;
{
	saved_stp = stp;

	if (alarm_happened) {
		alarm_happened = 0;
		if (stp) {
			if (stp->screenp->sd)
				leavefield(stp);
			headers(stp->screenp);
			if (stp->screenp->sd)
			{
				struct scrn_desc *sdp;

				sdp = sttosd(stp);
				reenterfield(stp, sdp);
			}
			wnoutrefresh(stdscr);
			wnoutrefresh(stp->window);
			if (stp->screenp->sd)
				correctcursor(stp);
			doupdate();
		}
	}
	in_input_mode = 1;
}

/*
 * called after input has been received.  Turns off updating until
 * back in input mode.
 */

void
scrn_exit_input_mode(stp)
struct state *stp;
{
	in_input_mode = 0;
	saved_stp = NULL;
}

/*
 * called from the signal catching routine for SIGALRM
 */

void
updatetimedate()
{
	if (in_input_mode) {
		if (saved_stp) {
			if (saved_stp->screenp->sd)
				leavefield(saved_stp);
			headers(saved_stp->screenp);
			if (saved_stp->screenp->sd) {
				struct scrn_desc *sdp;

				sdp = sttosd(saved_stp);
				reenterfield(saved_stp, sdp);
			}
			wnoutrefresh(stdscr);
			wnoutrefresh(saved_stp->window);
			if (saved_stp->screenp->sd)
				correctcursor(saved_stp);
			doupdate();
		}
	}
	else
		alarm_happened = 1;
}

/*
 * char_init (s, n, c) - init buffer s with n chars c and a term. NULL
 */

char_init(s, n, c)
uchar *s;
int n;
uchar c;
{
	register uchar *sp = s;
	register uchar cr = c;
	register int i = 0;

	for ( ; i < n; i++, sp++)
		*sp = cr;
	*sp = NULL;
}
 
/*
 * clearout (len, item, line, lcr) - clear old header area
 */

static void
clearout(len, line, lcr)
int len;
int line;		/* line # of screen */
int lcr;		/* left/center/right */
{
	static char *ib = NULL;		/* item as spaces */

	/*
	 * allocate an array of spaces as wide as the screen.
	 * This assumes the screen width doesn't change.
	 */

	if (ib == NULL) {
		ib = Malloc(COLS + 1);
		if (ib == NULL)
			MemoryError();
		memset(ib, ' ', COLS);
		ib[COLS] = '\0';
	}

	if (len != 0) {
		ib[len] = '\0';
		switch(lcr) {
		case LITEM:
			LEFTS(line, ib);
			break;
		case CITEM:
		case HITEM:
		case PITEM:
			CENTERS(line, ib);
			break;
		case RITEM:
			RIGHTS(line, ib);
			break;
		}
		ib[len] = ' ';
	}
}
