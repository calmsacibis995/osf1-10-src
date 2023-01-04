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
static char	*sccsid = "@(#)$RCSfile: cmdtab.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:10:39 $";
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
 * cmdtab.c	5.3 (Berkeley) 6/30/88
 * cmdtab.c	5.1 16:07:38 8/16/90 SecureWare 
 */

/*
 * lpc -- command tables
 */

#include "lpc.h"
#include "printer_msg.h"

int	abort(), clean(), enable(), disable(), down(), help();
int	quit(), restart(), start(), status(), stop(), topq(), up();

struct cmd cmdtab[] = {
#if SEC_BASE
    { "abort",	CMDTAB_ABORTHELP,	abort,		"printerstat",	-1 },
    { "clean",	CMDTAB_CLEANHELP,	clean,		"lp",		-1 },
    { "enable",	CMDTAB_ENABLEHELP,	enable,		"lp",		-1 },
    { "exit",	CMDTAB_QUITHELP,	quit,		0,		-1 },
    { "disable",CMDTAB_DISABLEHELP,	disable,	"lp",		-1 },
    { "down",	CMDTAB_DOWNHELP,	down,		"lp",		-1 },
    { "help",	CMDTAB_HELPHELP,	help,		0,		-1 },
    { "quit",	CMDTAB_QUITHELP,	quit,		0,		-1 },
    { "restart",CMDTAB_RESTARTHELP,	restart,	"printerstat",	-1 },
    { "start",	CMDTAB_STARTHELP,	start,		"printerstat",	-1 },
    { "status",	CMDTAB_STATUSHELP,	status,		0,		-1 },
    { "stop",	CMDTAB_STOPHELP,	stop,		"printerstat",	-1 },
    { "topq",	CMDTAB_TOPQHELP,	topq,		"lp",		-1 },
    { "up",	CMDTAB_UPHELP,		up,		"lp",		-1 },
    { "?",	CMDTAB_HELPHELP,	help,		0,		-1 },
    { 0 },
#else
    { "abort",	CMDTAB_ABORTHELP,	abort,		1 },
    { "clean",	CMDTAB_CLEANHELP,	clean,		1 },
    { "enable",	CMDTAB_ENABLEHELP,	enable,		1 },
    { "exit",	CMDTAB_QUITHELP,	quit,		0 },
    { "disable",CMDTAB_DISABLEHELP,	disable,	1 },
    { "down",	CMDTAB_DOWNHELP,	down,		1 },
    { "help",	CMDTAB_HELPHELP,	help,		0 },
    { "quit",	CMDTAB_QUITHELP,	quit,		0 },
    { "restart",CMDTAB_RESTARTHELP,	restart,	0 },
    { "start",	CMDTAB_STARTHELP,	start,		1 },
    { "status",	CMDTAB_STATUSHELP,	status,		0 },
    { "stop",	CMDTAB_STOPHELP,	stop,		1 },
    { "topq",	CMDTAB_TOPQHELP,	topq,		1 },
    { "up",	CMDTAB_UPHELP,		up,		1 },
    { "?",	CMDTAB_HELPHELP,	help,		0 },
    { 0 },
#endif
};

int	NCMDS = sizeof (cmdtab) / sizeof (cmdtab[0]);
