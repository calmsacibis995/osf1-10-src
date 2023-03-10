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
static char	*sccsid = "@(#)$RCSfile: cmd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:10:23 $";
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * 	cmd.c	3.38 (Berkeley) 6/29/88
 */


#include "defs.h"
#include "char.h"

docmd()
{
	register int c;
	register struct ww *w;
	char out = 0;

	while (!out && !quit) {
		if ((c = wwgetc()) < 0) {
			if (terse)
				wwsetcursor(0, 0);
			else {
				wwputs(MSGSTR(COMMAND, "Command: "), cmdwin);
				wwcurtowin(cmdwin);
			}
			do
				wwiomux();
			while ((c = wwgetc()) < 0);
		}
		if (!terse)
			wwputc('\n', cmdwin);
		switch (c) {
		default:
			if (c != escapec)
				break;
		case 'h': case 'j': case 'k': case 'l':
		case ctrl('y'):
		case ctrl('e'):
		case ctrl('u'):
		case ctrl('d'):
		case ctrl('b'):
		case ctrl('f'):
		case ctrl('s'):
		case ctrl('q'):
		case ctrl('['):
			if (selwin == 0) {
				error(MSGSTR(NOWINDOW, "No window."));
				continue;
			}
		}
		switch (c) {
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
			if ((w = window[c - '1']) == 0) {
				error(MSGSTR(NOSUCHWIN, "%c: No such window."), c);
				break;
			}
			setselwin(w);
			if (checkproc(selwin) >= 0)
				 out = 1;
			break;
		case '%':
			if ((w = getwin()) != 0)
				setselwin(w);
			break;
		case ctrl('^'):
			if (lastselwin != 0) {
				setselwin(lastselwin);
				if (checkproc(selwin) >= 0)
					out = 1;
			} else
				error(MSGSTR(NOPREVWIN, "No previous window."));
			break;
		case 'c':
			if ((w = getwin()) != 0)
				closewin(w);
			break;
		case 'w':
			c_window();
			break;
		case 'm':
			if ((w = getwin()) != 0)
				c_move(w);
			break;
		case 'M':
			if ((w = getwin()) != 0)
				movewin(w, w->ww_alt.t, w->ww_alt.l);
			break;
		case 's':
			if ((w = getwin()) != 0)
				c_size(w);
			break;
		case 'S':
			if ((w = getwin()) != 0)
				sizewin(w, w->ww_alt.nr, w->ww_alt.nc);
			break;
		case ':':
			c_colon();
			break;
		case 'h':
			(void) wwwrite(selwin, "\b", 1);
			break;
		case 'j':
			(void) wwwrite(selwin, "\n", 1);
			break;
		case 'k':
			(void) wwwrite(selwin, "\033A", 2);
			break;
		case 'l':
			(void) wwwrite(selwin, "\033C", 2);
			break;
		case ctrl('e'):
			wwscroll(selwin, 1);
			break;
		case ctrl('y'):
			wwscroll(selwin, -1);
			break;
		case ctrl('d'):
			wwscroll(selwin, selwin->ww_w.nr / 2);
			break;
		case ctrl('u'):
			wwscroll(selwin, - selwin->ww_w.nr / 2);
			break;
		case ctrl('f'):
			wwscroll(selwin, selwin->ww_w.nr);
			break;
		case ctrl('b'):
			wwscroll(selwin, - selwin->ww_w.nr);
			break;
		case ctrl('s'):
			stopwin(selwin);
			break;
		case ctrl('q'):
			startwin(selwin);
			break;
		case ctrl('l'):
			wwredraw();
			break;
		case '?':
			c_help();
			break;
		case ctrl('['):
			if (checkproc(selwin) >= 0)
				out = 1;
			break;
		case ctrl('z'):
			wwsuspend();
			break;
		case 'q':
			c_quit();
			break;
		/* debugging stuff */
		case '&':
			if (debug) {
				c_debug();
				break;
			}
		default:
			if (c == escapec) {
				if (checkproc(selwin) >= 0) {
					(void) write(selwin->ww_pty,
						&escapec, 1);
					out = 1;
				}
			} else {
				if (!terse)
					wwbell();
				error(MSGSTR(TYPE4HELP, "Type ? for help."));
			}
		}
	}
	if (!quit)
		setcmd(0);
}

struct ww *
getwin()
{
	register int c;
	struct ww *w = 0;

	if (!terse)
		wwputs(MSGSTR(WHICHWIN, "Which window? "), cmdwin);
	wwcurtowin(cmdwin);
	while ((c = wwgetc()) < 0)
		wwiomux();
	if (debug && c == 'c')
		w = cmdwin;
	else if (debug && c == 'f')
		w = framewin;
	else if (debug && c == 'b')
		w = boxwin;
	else if (c >= '1' && c < NWINDOW + '1')
		w = window[c - '1'];
	else if (c == '+')
		w = selwin;
	else if (c == '-')
		w = lastselwin;
	if (w == 0)
		wwbell();
	if (!terse)
		wwputc('\n', cmdwin);
	return w;
}

checkproc(w)
struct ww *w;
{
	if (w->ww_state != WWS_HASPROC) {
		error(MSGSTR(NOPROCINWIN, "No process in window."));
		return -1;
	}
	return 0;
}

setcmd(new)
char new;
{
	if (new && !incmd) {
		if (!terse)
			wwadd(cmdwin, &wwhead);
		if (selwin != 0)
			wwcursor(selwin, 1);
		wwcurwin = 0;
	} else if (!new && incmd) {
		if (!terse) {
			wwdelete(cmdwin);
			reframe();
		}
		if (selwin != 0)
			wwcursor(selwin, 0);
		wwcurwin = selwin;
	}
	incmd = new;
}

setterse(new)
char new;
{
	if (incmd)
		if (new && !terse) {
			wwdelete(cmdwin);
			reframe();
		} else if (!new && terse)
			wwadd(cmdwin, &wwhead);
	terse = new;
}

/*
 * Set the current window.
 */
setselwin(w)
struct ww *w;
{
	if (selwin == w)
		return;
	if (selwin != 0)
		lastselwin = selwin;
	if ((selwin = w) != 0)
		front(selwin, 1);
}
