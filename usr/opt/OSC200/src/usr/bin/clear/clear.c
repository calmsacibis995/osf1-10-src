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
static char	*sccsid = "@(#)$RCSfile: clear.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:22:01 $";
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
 * COMPONENT_NAME: CMDTTY tty control commands
 *
 * FUNCTIONS:  main (clear.c)
 *
 * ORIGINS: 26, 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * clear.c     1.4  com/cmd/tty,3.1,9021 11/17/89 09:36:33";
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#ifndef lint
char copyright[] =
"Copyright (c) 1980 Regents of the University of California.\n\
 All rights reserved.\n";
#endif not lint

/* load me with -ltermlib (or -lcurses to use terminfo) */
/* #include <retrofit.h> on version 6 */
/*
 * clear - clear the screen
 */

#define _BSD
#define BSD

#include <stdio.h>
#include <sgtty.h>

char	*getenv();
char	*tgetstr();
char	PC;
short	ospeed;
#undef	putchar
extern int 	putchar(int c);
/*int	putchar();*/

main()
{
	char *cp = getenv("TERM");
	char clbuf[20];
	char pcbuf[20];
	char *clbp = clbuf;
	char *pcbp = pcbuf;
	char *clear;
	char buf[1024];
	char *pc;
	struct sgttyb tty;

	gtty(1, &tty);
	ospeed = tty.sg_ospeed;
	if (cp == (char *) 0)
		exit(1);
	if (tgetent(buf, cp) != 1)
		exit(1);
	pc = tgetstr("pc", &pcbp);
	if (pc)
		PC = *pc;
	clear = tgetstr("cl", &clbp);
	if (clear)
		tputs(clear, tgetnum("li"), putchar);
	exit (clear == (char *) 0);
}
