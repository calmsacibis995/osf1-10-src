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
static char	*sccsid = "@(#)$RCSfile: main.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/10/11 19:19:34 $";
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
 * Copyright (c) 1988 Regents of the University of California.
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

#ifndef lint
char copyright[] =
" Copyright (c) 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#include <sys/types.h>

#include "ring.h"

#include "externs.h"
#include "defines.h"

/*
 * Initialize variables.
 */

void
tninit()
{
    init_terminal();

    init_network();
    
    init_telnet();

    init_sys();

    init_3270();
}


/*
 * main.  Parse arguments, invoke the protocol or command parser.
 */


int
main(argc, argv)
	int argc;
	char *argv[];
{
    char *user = 0;

	setlocale(LC_ALL, "");
        catd = NLcatopen((char *) MF_TELNET, NL_CAT_LOCALE);

    tninit();		/* Clear out things */

    TerminalSaveState();

    prompt = argv[0];
    while ((argc > 1) && (argv[1][0] == '-')) {
	if (!strcmp(argv[1], "-d")) {
	    debug = 1;
	} else if (!strcmp(argv[1], "-n")) {
	    if ((argc > 2) && (argv[2][0] != '-')) {	/* get file name */
		SetNetTrace(argv[2]);
		argv++;
		argc--;
	    }
	  fprintf(stderr,"usage: telnet -n trace file\n") ;
	  exit(1); 
	 } else if (!strcmp(argv[1], "-l")) {
            if ((argc > 2) && (argv[2][0] != '-')) {    /* get user name */
                user = argv[2];
                argv++;
                argc--;
            }
	    else {
	      fprintf(stderr, "usage: telnet -l user \n") ;
              exit(1) ;
	    }
	} else {
#if	defined(TN3270) && defined(unix)
	    if (!strcmp(argv[1], "-t")) {
		if ((argc > 1) && (argv[2][0] != '-')) { /* get file name */
		    transcom = tline;
		    (void) strcpy(transcom, argv[2]);
		    argv++;
		    argc--;
		}
	    } else if (!strcmp(argv[1], "-noasynch")) {
		noasynchtty = 1;
		noasynchnet = 1;
	    } else if (!strcmp(argv[1], "-noasynchtty")) {
		noasynchtty = 1;
	    } else if (!strcmp(argv[1], "-noasynchnet")) {
		noasynchnet = 1;
	    } else
#endif	/* defined(TN3270) && defined(unix) */
	    if (argv[1][1] != '\0') {
		fprintf(stderr, MSGSTR( UNK_OPTION, "Unknown option *%s*.\n"),
			argv[1]);
	    }
	}
	argc--;
	argv++;
    }
    if (argc != 1) {
	if (setjmp(toplevel) != 0)
	    Exit(0);
	if (user) {
            argc += 2;
            argv -= 2;
            argv[0] = argv[2];
            argv[1] = "-l";
            argv[2] = user;
        }
	if (tn(argc, argv) == 1) {
	    return 0;
	} else {
	    return 1;
	}
    }
    (void) setjmp(toplevel);
    for (;;) {
#if	!defined(TN3270)
	command(1, 0, 0);
#else	/* !defined(TN3270) */
	if (!shell_active) {
	    command(1, 0, 0);
	} else {
#if	defined(TN3270)
	    shell_continue();
#endif	/* defined(TN3270) */
	}
#endif	/* !defined(TN3270) */
    }
}
