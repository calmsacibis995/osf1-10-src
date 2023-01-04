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
static char *rcsid = "@(#)$RCSfile: getscr.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:38 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	getscr.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:03:56  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:58:51  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:19:33  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:52:06  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1989, 1990 SecureWare, Inc.  All rights reserved.
 *
 * Based on OSF version:
 *	@(#)getscr.c	1.25 16:31:00 5/17/91 SecureWare
 */

/* #ident "@(#)getscr.c	1.1 11:16:29 11/8/91 SecureWare" */

/* getscr.c - getscreen () handles the keyboard input from screens
 *	and dispatches the appropriate routine for the key pressed.
 */

#include "If.h"
#include "AIf.h"
#include "scrn_local.h"

/*  Prompt a user to fill in a screen.
 *  Returns the state of the screen, including whether a
 *  confirm entry was changed, whether the user just wants to
 *  execute the screen, and whether the screen itself was changed,
 *  as well as telling which item was current when the user finished.
 */

struct	scrn_ret
getscreen (screenp, structp, first_desc)
struct	scrn_parms	*screenp;		/* screen description */
struct	scrn_struct	*structp;	/* screen data returned to process */
int	first_desc;			/* which scrn_struct to start */
{
	struct	state	states;	/* state variables */
	register struct	state	*stp;
	Scrn_desc	*sdp;
	int		thischar;	/* current character */
	int		trav_ret;	/* traverse() return code */
	int		dummy;		/* to spoof trav_choice() */

	ENTERFUNC("getscreen");
	stp = &states;

	/* set up state structure, including curfield */

	initstates (screenp, structp, first_desc, stp);
	if (screenp->inuse > 1)
		off_insert_ind ();
	movetofield (stp);
	sdp = sttosd(stp);
	if (sdp != NULL && (sdp->type == FLD_TOGGLE || sdp->type == FLD_SCRTOG))
		highlighttoggle (stp, sdp);
	refresh ();
	wrefresh (stp->window);


	do {
		/* get a character after polling whether to update headers */

		scrn_enter_input_mode(stp);
		thischar = get_key_conv(stp->window);

		/* disable update of time header */
		scrn_exit_input_mode(stp);

#ifdef DEBUG_KEYBD_INPUT
		mvprintw(1, (COLS/2)-3, "%-4.4x", thischar);
		refresh();
#endif /* DEBUG_KEYBD_INPUT */
		if (thischar == HELP) {

			/* item-specific help, and re-establish headers */
			if (screenp->scrntype == SCR_NOCHANGE ||
			    screenp->w == (WINDOW *) 0)
				beep();

			else {
				help(stp);
				headers(screenp);
				wnoutrefresh(stdscr);
			}

		} else if (thischar == KEYS_HELP) {

			if (screenp->scrntype == SCR_NOCHANGE ||
			    screenp->w == (WINDOW *) 0)
				beep();

			else {
				/* keystroke help, and re-establish headers */

				_HelpDisplayOpen(MSGSTR(GETSCR_1, "ascii,help.keys"));
				headers(screenp);
				wnoutrefresh(stdscr);
				touchwin(screenp->w);
			}

		} else if (screenp->scrntype == SCR_FILLIN)  {
			switch (thischar)  {
			case	SCROLLDOWN:
				scrolldownkey (stp);
				break;
			case	SCROLLUP:
				scrollupkey (stp);
				break;
			case	DOWN:
				downfill (stp);
				break;
			case	UP:
				upfill (stp);
				break;
			case	LEFT:
				leftfill (stp);
				break;
			case	RIGHT:
				rightfill (stp);
				break;
			case	TAB:
			case	ENTER:			/* should no longer happen! */
				tabfill (stp);
				break;
			case	BACKTAB:
				backtabfill (stp);
				break;

			case	DELWORD:
				if ((sttosd(stp))->inout != FLD_OUTPUT)
					delword (stp);
				break;
			case	BACKSPACE:
				if ((sttosd(stp))->inout != FLD_OUTPUT)
					backspace (stp);
				break;
			case	DELFIELD:
				if ((sttosd(stp))->inout != FLD_OUTPUT)
					delfield (stp);
				break;
			case	INSFIELD:
				if ((sttosd(stp))->inout != FLD_OUTPUT)
					insfield (stp);
				break;
			case	INSTOGGLE:
				if ((sttosd(stp))->inout != FLD_OUTPUT)
					instoggle (stp);
					refresh ();
				break;

			case	EXECUTE:
				/* have to successfully leave a field */
				if (!leavefield (stp))
					if (reqfillin (stp) == 0) {
						stp->ret.item = stp->curfield;
						stp->ret.flags |= R_EXECUTE;
					}
				break;
			case	QUITMENU:
				stp->ret.flags |= R_ABORTED;
				stp->ret.item = stp->curfield;
				break;
			case	QUITPROG:
				stp->ret.flags |= R_QUIT;
				stp->ret.item = stp->curfield;
				break;
			case	REDRAW:
				clearok (stdscr, TRUE);
				refresh ();
				clearok (stp->window, TRUE);
				break;
			case	SPACE:
				if ((sttosd(stp))->type == FLD_CHOICE) {

					/*
					 * choice selected on fillin screen
					 */

					trav_ret = choicefill(stp);
					switch (trav_ret) {
					case QUIT:
						stp->ret.flags |= R_QUIT;
						break;
					case ABORT:
						stp->ret.flags |= R_ABORTED;
						break;
					default:
						stp->ret.flags = 0;
						break;
					}
					stp->ret.item = stp->curfield;
					break;
				}
				/* ELSE FALL THRU TO DEFAULT */
			default:
				if (isprint (thischar))
					fillinkey (stp, thischar);
				else	beep();
			}
		} else if (screenp->scrntype == SCR_MENU)  {
			switch (thischar)  {
			case	DOWN:
				downmenu (stp);
				break;
			case	UP:
				upmenu (stp);
				break;
			case	SPACE:
			case	EXECUTE:
				stp->ret.item = stp->curfield;
				stp->ret.flags |= R_EXECUTE;
				break;
			case	REDRAW:
				clearok (stdscr, TRUE);
				refresh ();
				clearok (stp->window, TRUE);
				break;
			case	LEFT:
			case	BACKTAB:
				leftmenu (stp);
				break;
			case	RIGHT:
			case	ENTER:	/* should no longer happen! */
			case	TAB:
				rightmenu (stp);
				break;
			case	QUITMENU:
				stp->ret.flags |= R_ABORTED;
				stp->ret.item = stp->curfield;
				break;
			case	QUITPROG:
				stp->ret.flags |= R_QUIT;
				stp->ret.item = stp->curfield;
				break;
			default:
				beep();
			}
		} else if (screenp->scrntype == SCR_TEXT) {
				stp->ret.flags |= R_ABORTED;
				stp->ret.item = stp->curfield;
		} else { /* SCR_NOCHANGE */
			switch (thischar)  {
			case	SCROLLDOWN:
				scrolldownkey (stp);
				break;
			case	SCROLLUP:
				scrollupkey (stp);
				break;
			case	DOWN:
				downfill (stp);
				break;
			case	UP:
				upfill (stp);
				break;
			case	TAB:
			case	ENTER:			/* should no longer happen! */
				tabfill (stp);
				break;
			case	BACKTAB:
				backtabfill (stp);
				break;
			case	REDRAW:
				clearok (stdscr, TRUE);
				refresh ();
				clearok (stp->window, TRUE);
				break;
			case	SPACE:
			case	EXECUTE:
				if ((sttosd(stp))->type == FLD_CHOICE) {
					trav_ret = trav_choice (screenp,
					sdtocn (screenp, &sdp, stp->curfield),
					&dummy, (sttosd(stp)),
					(unsigned short) 0);
					stp->ret.flags |=
						trav_ret==QUIT?R_QUIT:
						(trav_ret==ABORT?R_ABORTED:							R_POPUP);
					stp->ret.item = stp->curfield;
					break; /* boogie */
				}
				/* ELSE FALL THRU TO QUITMENU */
			case	QUITMENU:
				stp->ret.flags |= R_ABORTED;
				stp->ret.item = stp->curfield;
				break;
			case	QUITPROG:
				stp->ret.flags |= R_QUIT;
				stp->ret.item = stp->curfield;
				break;
			default:
				beep();
				break;
			}
		}
		correctcursor(stp);
		wnoutrefresh(stp->window);
		doupdate();
	} while (0 == (stp->ret.flags &
		(R_CONFIRM | R_EXECUTE | R_POPUP | R_QUIT |
		R_ABORTED | R_HELP | R_ACTHELP)));

	off_insert_ind ();
	EXITFUNC("getscreen");
	return (stp->ret);
}
