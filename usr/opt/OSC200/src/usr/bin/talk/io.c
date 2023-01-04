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
static char rcsid[] = "@(#)$RCSfile: io.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/06/25 15:21:49 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/* 
 * COMPONENT_NAME: TCPIP io.c
 * 
 * FUNCTIONS: MSGSTR, message, p_error, talk 
 *
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
/* io.c	1.5  com/sockcmd/talk,3.1,9021 10/24/89 12:32:25 */
/*
 "io.c	5.3 (Berkeley) 6/29/88";
*/
/*
 * This file contains the I/O handling and the exchange of 
 * edit characters. This connection itself is established in
 * ctl.c
 */

#include "talk.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "talk_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TALK,n,s) 

#define A_LONG_TIME 10000000
#define STDIN_MASK (1<<fileno(stdin))	/* the bit mask for standard
					   input */
extern int errno;

/*
 * The routine to do the actual talking
 */
talk()
{
	register int read_template, sockt_mask;
	int read_set, nb;
	char buf[BUFSIZ];
	struct timeval wait;
	int mb_cur_max = MB_CUR_MAX ;

	message(MSGSTR(CONN_EST, "Connection established")); /*MSG*/
	beep(); beep(); beep();
	current_line = 0;
	sockt_mask = (1<<sockt);

	/*
	 * Wait on both the other process (sockt_mask) and 
	 * standard input ( STDIN_MASK )
	 */
	read_template = sockt_mask | STDIN_MASK;
	forever {
		read_set = read_template;
		wait.tv_sec = A_LONG_TIME;
		wait.tv_usec = 0;
		nb = select(32, &read_set, 0, 0, &wait);
		if (nb <= 0) {
			if (errno == EINTR) {
				read_set = read_template;
				continue;
			}
			/* panic, we don't know what happened */
			p_error(MSGSTR(UNEXPECT_ERR, "Unexpected error from select")); /*MSG*/
			quit();
		}
		if (read_set & sockt_mask) { 
			/* There is data on sockt */
			nb      = read(sockt, buf, sizeof buf);
			buf[nb] = '\0' ;
			if (nb <= 0) {
				message(MSGSTR(CONN_WAIT, "Connection closed. Interrupt to exit...")); /*MSG*/
				pause();	/* wait for ctrl-C */
				quit();
			}
			display(&his_win, buf, nb);
		}
		if (read_set & STDIN_MASK) {
			/*
			 * We can't make the tty non_blocking, because
			 * curses's output routines would screw up
			 */
			ioctl(0, FIONREAD, (struct sgttyb *) &nb);
			nb	= read(0, buf, nb);
			buf[nb] = '\0'		  ;
			if (mb_cur_max > 1) {
				/*
				 * Read a complete multibyte character before
				 * displaying
				 */
				while ((nb < mb_cur_max) &&
				       (mblen(buf, nb) < 0)) {
					read(0, &buf[nb], 1) ;
					buf[++nb] = '\0'     ;
				}
			}
			display(&my_win, buf, nb) ;
			/* might lose data here because sockt is non-blocking */
			write(sockt, buf, nb);
		}
	}
}

extern	int errno;
extern	int sys_nerr;
extern	char *sys_errlist[];

/*
 * p_error prints the system error message on the standard location
 * on the screen and then exits. (i.e. a curses version of perror)
 */
p_error(string) 
	char *string;
{
	char *sys;

	sys = MSGSTR(UNKNOWN_ERR, "Unknown error"); /*MSG*/
	if (errno < sys_nerr)
		sys = sys_errlist[errno];
	wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
	wprintw(my_win.x_win, "[%s : %s (%d)]\n", string, sys, errno); 
	wrefresh(my_win.x_win);
	move(LINES-1, 0);
	refresh();
	quit();
}

/*
 * Display string in the standard location
 */
message(string)
	char *string;
{
	static char blank_line[] =  
	"                                                                     ";

	/*
	 * Output a blank line first to make sure that a new
	 * message won't overlap with an old message.
	 */
	wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
	wprintw(my_win.x_win, blank_line) ;
	wrefresh(my_win.x_win) ;
	wmove(my_win.x_win, current_line%my_win.x_nlines, 0);
	wprintw(my_win.x_win, "[%s]\n", string);
	wrefresh(my_win.x_win);
}
