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
static char rcsid[] = "@(#)$RCSfile: init_disp.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/06/25 15:21:46 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * COMPONENT_NAME: TCPIP init_disp.c
 * 
 * FUNCTIONS: MSGSTR, init_display, quit, set_edit_chars, sig_sent 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
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
 */
/* init_disp.c	1.6  com/sockcmd/talk,3.1,9021 12/1/89 11:11:29 */
/*
#ifndef lint
static char sccsid[] = "init_disp.c	5.3 (Berkeley) 6/29/88";
#endif  not lint */

/*
 * Initialization code for the display package,
 * as well as the signal handling routines.
 */

#define _SYS_TERMIOS_H_
#include "talk.h"
#include <signal.h>
#undef _SYS_TERMIOS_H_
#include <termios.h>
#include <unistd.h>
#include "talk_msg.h" 

extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

/* 
 * Set up curses, catch the appropriate signals,
 * and build the various windows.
 */
init_display()
{
	void sig_sent();
	struct sigvec sigv;

	initscr();
	(void) sigvec(SIGTSTP, (struct sigvec *)0, &sigv);
	sigv.sv_mask |= sigmask(SIGALRM);
	(void) sigvec(SIGTSTP, &sigv, (struct sigvec *)0);
	curses_initialized = 1;
	clear();
	refresh();
	noecho();
	crmode();
	signal(SIGINT, sig_sent);
	signal(SIGPIPE, sig_sent);
	/* curses takes care of ^Z */
	my_win.x_nlines = LINES / 2;
	my_win.x_ncols = COLS;
	my_win.x_win = newwin(my_win.x_nlines, my_win.x_ncols, 0, 0);
	scrollok(my_win.x_win, FALSE);
	werase(my_win.x_win);		/* 0001 */

	his_win.x_nlines = LINES / 2 - 1;
	his_win.x_ncols = COLS;
	his_win.x_win = newwin(his_win.x_nlines, his_win.x_ncols,
	    my_win.x_nlines+1, 0);
	scrollok(his_win.x_win, FALSE);
	werase(his_win.x_win);		/* 0001 */

	line_win = newwin(1, COLS, my_win.x_nlines, 0);
	box(line_win, '-', '-');
	wrefresh(line_win);
	/* let them know we are working on it */
	current_state = MSGSTR(NO_CONN_YET, "No connection yet");
}

/*
 * Trade edit characters with the other talk. By agreement
 * the first three characters each talk transmits after
 * connection are the three edit characters.
 */

set_edit_chars()
{
	char buf[3];
	int cc;
	struct termios tty;
	
	tcgetattr(0, &tty);
	my_win.cerase = tty.c_cc[VERASE];
	my_win.kill = tty.c_cc[VKILL];
	my_win.eofchar = tty.c_cc[VEOF];
	if (tty.c_cc[VWERASE] == _POSIX_VDISABLE)
		my_win.werase = '\027';	 /* control W */
	else
		my_win.werase = tty.c_cc[VWERASE];

	buf[0] = my_win.cerase;
	buf[1] = my_win.kill;
	buf[2] = my_win.werase;
	cc = write(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error(MSGSTR(LOST_CONN, "Lost the connection")); /*MSG*/
	cc = read(sockt, buf, sizeof(buf));
	if (cc != sizeof(buf) )
		p_error(MSGSTR(LOST_CONN, "Lost the connection")); /*MSG*/
	his_win.cerase = buf[0];
	his_win.kill = buf[1];
	his_win.werase = buf[2];
}

void
sig_sent()
{

	message(MSGSTR(CONN_CLOSE, "Connection closing. Exiting")); /*MSG*/
	quit();
}

/*
 * All done talking...hang up the phone and reset terminal thingy's
 */
quit()
{

	if (curses_initialized) {
		wmove(his_win.x_win, his_win.x_nlines-1, 0);
		wclrtoeol(his_win.x_win);
		wrefresh(his_win.x_win);
		endwin();
	}
	if (invitation_waiting)
		send_delete();
	exit(0);
}
