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
static char rcsid[] = "@(#)$RCSfile: show.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 22:32:59 $";
#endif
/*
 * HISTORY
 */
/*** "show.c  1.7  com/lib/curses,3.1,9008 12/4/89 21:02:48"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   show
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <signal.h>
#include "curses.h"

#ifdef DEBUGfoo
#undef LINES
#define LINES 5
#endif

/*
 * NAME:        show
 */

main(argc, argv)
char **argv;
{
	FILE *fd;
	char linebuf[512];
	int line;
	int done();

	if (argc < 2) {
		(void) fprintf(stderr, "Usage: show file\n");
		exit(1);
	}
	fd = fopen(argv[1], "r");
	if (fd == NULL) {
		perror(argv[1]);
		exit(2);
	}
	signal(SIGINT, ((void (*)(int))done));

	initscr();			/* initialize curses */
	noecho();			/* turn off tty echo */
	cbreak();			/* enter cbreak mode */
	nonl();				/* allow more optimizations */
	idlok(stdscr, TRUE);		/* allow insert/delete line */

	for (;;) {			/* for each screen full */
		(void) move(0, 0);
		/* werase(stdscr); */
		for (line=0; line<LINES; line++) {
			if (fgets(linebuf, (int)(sizeof linebuf), fd) == NULL) {
				clrtobot();
				done();
			}
			(void) mvprintw(line, 0, "%s", linebuf);
		}
		(void) refresh();	/* sync screen */
		if(getch() == 'q')	/* wait for user to read it */
			done();
	}
}

/*
 * Clean up and exit.
 */
done()
{
	(void) move(LINES-1,0);		/* to lower left corner */
	clrtoeol();			/* clear bottom line */
	(void) refresh();		/* flush out everything */
	endwin();			/* curses cleanup */
	exit(0);
}
