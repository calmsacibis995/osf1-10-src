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
static char	*sccsid = "@(#)$RCSfile: getscr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:48 $";
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
 * Copyright (c) 1989, 1990 SecureWare, Inc.  All rights reserved.
 */



/* getscr.c - getscreen () handles the keyboard input from screens
 *	and dispatches the appropriate routine for the key pressed.
 */

#include	<ctype.h>
#include	<stdio.h>

#include	"userif.h"
#include	"curs_supp.h"
#include	"key_map.h"
#include	"kitch_sink.h"
#include	"logging.h"


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
	register Menu_scrn	*mp;
	Scrn_desc	*sdp;
	int		thischar;	/* current character */
	int		mi;		/* menu_desc index */
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
		thischar = get_key_conv (stp->window);

		/* disable update of time header */
		scrn_exit_input_mode(stp);

#ifdef DEBUG_KEYBD_INPUT
		mvprintw (1, (COLS/2)-3, "%-4.4x", thischar);
		refresh();
#endif /* DEBUG_KEYBD_INPUT */
		if (thischar == HELP) {
			mi = sdtocn (screenp, &sdp, (sdp - screenp->sd));
			for (mp = screenp->ms; mp->choice != mi; mp++)
				;
			mi = mp - screenp->ms;
			if (screenp->ms[mi].help) {	/* "action" help */
				trav_ret = traverse (screenp->ms[mi].help,
					TRAV_RW);
				stp->ret.flags |=
					trav_ret==QUIT?R_QUIT:
					(trav_ret==ABORT?R_ABORTED:R_ACTHELP);
				stp->ret.item = stp->curfield;
			} else {		/* regular help text */
				help (stp);
			}
			continue;
		} else if (thischar == KEYS_HELP) {
			helpscrn ("help.keys");
			touchwin (stp->window);
			touchwin (stdscr);
			wnoutrefresh (stdscr);
			wnoutrefresh (stp->window);
			correctcursor (stp);
			doupdate();
			continue;
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
				    if ((sttosd(stp))->valid_on_leave) {
					if ((sttosd(stp))->scrnstruct &&
					  !(*(sttosd(stp))->scrnstruct->validate)
					    (screenp->fillin)) {
					      stp->ret.flags = 0;
					      break;
					}
				    }
				    trav_ret = trav_choice (screenp,
				    sdtocn (screenp, &sdp, stp->curfield),
					&dummy, (sttosd(stp)),
					(unsigned short)
					    (sttosd(stp))->valid_on_leave);
				    stp->ret.flags |=
					trav_ret==QUIT ? R_QUIT:
					(trav_ret==ABORT ? R_ABORTED : R_POPUP);
				    stp->ret.item = stp->curfield;
				    break; /* boogie */
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
		} else if (screenp->scrntype == SCR_MENUPROMPT) {
			uchar	handled = 1;

			switch (thischar)  {
			case	DOWN:
				downmprompt (stp);
				break;
			case	UP:
				upmprompt (stp);
				break;
			case	LEFT:
				leftmprompt (stp);
				break;
			case	RIGHT:
				rightmprompt (stp);
				break;
			case	EXECUTE:
				/* if no error, prepare to exit */
				if (!leavefield (stp))  {
					findchoice (stp);
					if (reqmprompt (stp) == 0) {
						stp->ret.flags |= R_EXECUTE;
						stp->ret.item = stp->curfield;
					}
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
			default:
				handled = 0;
				break;
			}
			/*  all other cases other than enter require
			 *  user to be in non-choice
			 */
			if (!handled)  {
				sdp = sttosd (stp);
				if (sdp->type == FLD_CHOICE)
					if (thischar == ENTER)
						/* should no longer happen! */
						downmprompt (stp);
					else
						beep();
				else switch (thischar) {
				case	DELWORD:
					delword (stp);
					break;
				case	BACKSPACE:
					backspace (stp);
					break;
				case	TAB:
					tabmprompt (stp);
					break;
				case	BACKTAB:
					backtabmprompt (stp);
					break;
				case	DELFIELD:
					delfield (stp);
					break;
				case	INSTOGGLE:
					instoggle (stp);
					refresh ();
					break;
				default:
					if (thischar == ENTER)			/* should no longer happen! */
						downmprompt(stp) ;
					else
						if (isprint(thischar))
							fillinkey(stp, thischar);
						else
							beep() ;
				}
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
			case	EXECUTE:
				stp->ret.flags |= R_EXECUTE;
				stp->ret.item = stp->curfield;
				break;
			default:
				beep();
				break;
			}
		}
		correctcursor (stp);
		wrefresh (stp->window);
	} while (0 == (stp->ret.flags &
		(R_CONFIRM | R_EXECUTE | R_POPUP | R_QUIT |
		R_ABORTED | R_HELP | R_ACTHELP)));

	off_insert_ind ();
	EXITFUNC("getscreen");
	return (stp->ret);
}
