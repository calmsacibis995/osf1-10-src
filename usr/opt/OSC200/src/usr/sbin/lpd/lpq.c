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
static char	*sccsid = "@(#)$RCSfile: lpq.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/08 15:14:32 $";
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
 * lpq.c	5.5 (Berkeley) 6/30/88";
 * lpq.c	4.1 15:58:48 7/19/90 SecureWare
 */


/*
 * Spool Queue examination program
 *
 * lpq [-l] [-Pprinter] [user...] [job...]
 *
 * -l long output
 * -P used to identify printer as per lpr/lprm
 */

#include <locale.h>
#include "lp.h"

#if SEC_BASE
#define MSGSTR_SEC(n,s) catgets(catd,MS_PRINTER_SEC,n,s)
#endif SEC_BASE

#if SEC_BASE
#include <sys/security.h>
#endif

char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */

main(argc, argv)
	register int	argc;
	register char	**argv;
{
    char *thisArg;
	int	ch, lflag;		/* long output option */
	int     seconds = 0;
        int displayq(int);

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif

	name = *argv++;
	if (gethostname(host, sizeof(host))) {
		perror(MSGSTR(LPQ_1, "lpq: gethostname"));
		exit(1);
	}
	openlog("lpd", 0, LOG_LPR);

	lflag = 0;
	while ( --argc > 0 )
	{
	    thisArg = *argv;
	    if ( *thisArg == '+' )
	    {
		if (thisArg[1])
		    seconds = atoi(thisArg);
		if ( seconds <= 0 )
		    seconds = 1;
		argv++;
	    }
	    else if ( *thisArg == '-' )
	    {
		switch(thisArg[1]) 
		{
		case 'l':			/* long output */
		    ++lflag;
		    break;
		case 'P':		/* printer name */
		    if ( thisArg[2] )
			printer = thisArg+2;
		    else
			if( --argc ) printer = *++argv;
		    break;
		case '?':
		default:
		    usage();
		    }
		argv++;
	    }
	    else
		break;
	}
	if (printer == NULL && (printer = getenv("PRINTER")) == NULL)
		printer = DEFLP;

	for (; argc > 0 ; --argc)
	{
	    thisArg = *argv++;
	    if (isdigit(*thisArg))
	    {
		if (requests >= MAXREQUESTS)
		    fatal(MSGSTR(LPQ_2, "too many requests"));
		requ[requests++] = atoi(thisArg);
	    }
	    else
	    {
		if (users >= MAXUSERS)
		    fatal(MSGSTR(LPQ_3, "too many users"));
		user[users++] = thisArg;
	    }
	}
#if SEC_BASE
    if (!forcepriv(SEC_REMOTE)
#if SEC_MAC
	|| authorized_user("macquery") && !forcepriv(SEC_ALLOWMACACCESS)
#endif
	)
    {
	fprintf(stderr, MSGSTR_SEC(PRIVS, "%s: insufficient privileges\n"), "lpq");
	exit(1);
    }
#endif
    if (seconds)
    {
	void clearscreen();
	clearscreen();
	while (displayq(lflag))
	{
	    sleep(seconds);
	    clearscreen();
	}
    }
    else
	displayq(lflag);
    exit(0);
}

void clearscreen()
{
#ifdef TERMCAP
    char    *tgetstr();
    char    PC;
    char *cp = getenv("TERM");
    char clbuf[20];
    char pcbuf[20];
    char *clbp = clbuf;
    char *pcbp = pcbuf;
    char *clear;
    char buf[1024];
    char *pc;
    
    if (cp == (char *) 0)
	return;
    if (tgetent(buf, cp) != 1)
	return;
    pc = tgetstr("pc", &pcbp);
    if (pc)
	PC = *pc;
    clear = tgetstr("cl", &clbp);
    if (clear)
	tputs(clear, tgetnum("li"), putchar);
    return;
#endif
}

usage()
{
	fprintf(stderr, 
		MSGSTR(LPQ_4, "usage: lpq [+n] [-l] [-Pprinter] [user ...] [job ...]\n"));
	exit(1);
}

