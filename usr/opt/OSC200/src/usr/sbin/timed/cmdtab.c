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
static char	*sccsid = "@(#)$RCSfile: cmdtab.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/10/08 16:13:12 $";
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
 * COMPONENT_NAME: TCPIP cmdtab.c
 * 
 * FUNCTIONS: MSGSTR, sizeof, tab_load 
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
/* cmdtab.c	1.3  com/sockcmd/timed,3.1,9021 12/4/89 09:42:46 */
/*
#ifndef lint
static char sccsid[] = "cmdtab.c	2.5 (Berkeley) 6/18/88";
#endif  not lint */

#include "timedc.h"

int	clockdiff(), help(), msite(), quit(), testing(), tracing();

#include "timed_msg.h"
#define MSGSTR(n,s) catgets(catd, MS_TIMEDC, n, s)

struct table {
    char   cmd[20];
    char   hlp[90];
} cmd_hlp[7];

struct cmd cmdtab[] = {
	{ cmd_hlp[0].cmd, cmd_hlp[0].hlp, 	clockdiff,	0 },
	{ cmd_hlp[1].cmd, cmd_hlp[1].hlp,	help,		0 },
	{ cmd_hlp[2].cmd, cmd_hlp[2].hlp,	msite,		0 },
	{ cmd_hlp[3].cmd, cmd_hlp[3].hlp,	quit,		0 },
	{ cmd_hlp[4].cmd, cmd_hlp[4].hlp,	tracing,	1 },
	{ cmd_hlp[5].cmd, cmd_hlp[5].hlp,	help,		0 },
#ifdef TESTING
	{ cmd_hlp[6].cmd, cmd_hlp[6].hlp,	testing,	1 },
#endif
};
int	NCMDS = sizeof (cmdtab) / sizeof (cmdtab[0]);
tab_load ()
{
    nl_catd catd;
    int i;

    catd = catopen(MF_TIMED,NL_CAT_LOCALE);
    
    for (i =0 ; i < NCMDS; i++)
    {
	cmdtab[i].c_name = cmd_hlp[i].cmd;
	cmdtab[i].c_help = cmd_hlp[i].hlp;
    }
    strcpy(cmd_hlp[0].cmd, MSGSTR(C_CLOCKDIFF, "clockdiff"));
    strcpy(cmd_hlp[1].cmd, MSGSTR(C_HELP, "help"));
    strcpy(cmd_hlp[2].cmd, MSGSTR(C_MSITE, "msite"));
    strcpy(cmd_hlp[3].cmd, MSGSTR(C_QUIT, "quit"));
    strcpy(cmd_hlp[4].cmd, MSGSTR(C_TRACE, "trace"));
    strcpy(cmd_hlp[5].cmd, MSGSTR(C_QUES, "?"));
    strcpy(cmd_hlp[0].hlp, MSGSTR(H_CLOCKDIFF, "measures clock differences between machines (up to 12 hours)"));
    strcpy(cmd_hlp[1].hlp, MSGSTR(H_HELP, "gets help on commands")); 
    strcpy(cmd_hlp[2].hlp, MSGSTR(H_MSITE, "finds location of master"));
    strcpy(cmd_hlp[3].hlp, MSGSTR(H_QUIT, "exits timedc"));
    strcpy(cmd_hlp[4].hlp, MSGSTR(H_TRACE, "turns tracing on or off"));
    strcpy(cmd_hlp[5].hlp, MSGSTR(H_HELP, "gets help on commands"));
#ifdef TESTING
    strcpy(cmd_hlp[6].cmd, MSGSTR(C_ELECTION, "election"));
    strcpy(cmd_hlp[6].hlp, MSGSTR(H_ELECTION, "causes election timers to expire"));
#endif

    catclose(catd);
}
