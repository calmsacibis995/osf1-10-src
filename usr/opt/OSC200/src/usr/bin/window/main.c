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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/11 19:54:41 $";
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
 * 	main.c	3.33 (Berkeley) 6/29/88
 */

#include "defs.h"
#include <sys/signal.h>
#include <stdio.h>
#include "string.h"
#include "char.h"
#include "local.h"

/*
 * from locale.h
 */
#define LC_ALL		0xFFFF  /* name of locale's category name       */

#define next(a) (*++*(a) ? *(a) : (*++(a) ? *(a) : (char *)usage()))

/*ARGSUSED*/
main(argc, argv)
char **argv;
{
	register char *p;
	char fflag = 0;
	char dflag = 0;
	char xflag = 0;
	char *cmd = 0;
	char tflag = 0;

	setlocale(LC_ALL,"");
	catd = catopen(MF_WINDOW,NL_CAT_LOCALE);

	nbufline = NLINE;
	escapec = ESCAPEC;	
	if (p = rindex(*argv, '/'))
		p++;
	else
		p = *argv;
	debug = strcmp(p, "a.out") == 0;
	while (*++argv) {
		if (**argv == '-') {
			switch (*++*argv) {
			case 'f':
				fflag++;
				break;
			case 'c':
				if (cmd != 0) {
					(void) fprintf(stderr,
						MSGSTR(ONLYONEC, "Only one -c allowed.\n"));
					(void) usage();
				}
				cmd = next(argv);
				break;
			case 'e':
				setescape(next(argv));
				break;
			case 't':
				tflag++;
				break;
			case 'd':
				dflag++;
				break;
			case 'D':
				debug = !debug;
				break;
			case 'x':
				xflag++;
				break;
			default:
				(void) usage();
			}
		} else
			(void) usage();
	}
	if ((p = getenv("SHELL")) == 0)
		p = SHELL;
	if ((shellfile = str_cpy(p)) == 0)
		nomem();
	if (p = rindex(shellfile, '/'))
		p++;
	else
		p = shellfile;
	shell[0] = p;
	shell[1] = 0;
	(void) gettimeofday(&starttime, (struct timezone *)0);
	if (wwinit() < 0) {
		(void) fprintf(stderr, "%s.\n", wwerror());
		exit(1);
	}
#ifndef POSIX_TTY
        if (debug)
                wwnewtty.ww_tchars.t_quitc = wwoldtty.ww_tchars.t_quitc;
        if (xflag) {
                wwnewtty.ww_tchars.t_stopc = wwoldtty.ww_tchars.t_stopc;
                wwnewtty.ww_tchars.t_startc = wwoldtty.ww_tchars.t_startc;
        }
#else
	if (debug) {
		wwnewtty.ww_termios.c_cc[VQUIT] =
			wwoldtty.ww_termios.c_cc[VQUIT];
		wwnewtty.ww_termios.c_lflag |= ISIG;
	}
	if (xflag) {
		wwnewtty.ww_termios.c_cc[VSTOP] =
			wwoldtty.ww_termios.c_cc[VSTOP];
		wwnewtty.ww_termios.c_cc[VSTART] =
			wwoldtty.ww_termios.c_cc[VSTART];
		wwnewtty.ww_termios.c_iflag |= IXON;
	}
#endif
	if (debug || xflag)
		(void) wwsettty(0, &wwnewtty, &wwoldtty);

	if ((cmdwin = wwopen(wwbaud > 2400 ? WWO_REVERSE : 0, 1, wwncol,
			     0, 0, 0)) == 0) {
		(void) wwflush();
		(void) fprintf(stderr, "%s.\r\n", wwerror());
		goto bad;
	}
	cmdwin->ww_mapnl = 1;
	cmdwin->ww_nointr = 1;
	cmdwin->ww_noupdate = 1;
	cmdwin->ww_unctrl = 1;
	if ((framewin = wwopen(WWO_GLASS|WWO_FRAME, wwnrow, wwncol, 0, 0, 0))
	    == 0) {
		(void) wwflush();
		(void) fprintf(stderr, "%s.\r\n", wwerror());
		goto bad;
	}
	wwadd(framewin, &wwhead);
	if ((boxwin = wwopen(WWO_GLASS, wwnrow, wwncol, 0, 0, 0)) == 0) {
		(void) wwflush();
		(void) fprintf(stderr, "%s.\r\n", wwerror());
		goto bad;
	}
	fgwin = framewin;

	wwupdate();
	wwflush();
	(void) signal(SIGCHLD, (void (*)(int))wwchild);
	setvars();

	setterse(tflag);
	setcmd(1);
	if (cmd != 0)
		(void) dolongcmd(cmd, (struct value *)0, 0);
	if (!fflag) {
		if (dflag || doconfig() < 0)
			dodefault();
		if (selwin != 0)
			setcmd(0);
	}

	mloop();

bad:
	wwend();
	return 0;
}

usage()
{
	(void) fprintf(stderr, MSGSTR(USAGE, "Usage: window [-e escape-char] [-c command] [-t] [-f] [-d]\n"));
	exit(1);
	return 0;			/* for lint */
}

nomem()
{
	(void) fprintf(stderr, MSGSTR(OUTOFMEM, "Out of memory.\n"));
	exit(1);
}
