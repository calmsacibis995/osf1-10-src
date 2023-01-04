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
static char	*sccsid = "@(#)$RCSfile: lptest.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:14:51 $";
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
 * lptest.c	5.3 (Berkeley) 6/30/88
 */


/*
 * lptest -- line printer test program (and other devices).
 */

#include <stdio.h>
#include <locale.h>
#include "printer_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_PRINTER,n,s)

main(argc, argv)
	int argc;
	char **argv;
{
	int len, count;
	register i, j, fc, nc;
	char outbuf[BUFSIZ];

        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_PRINTER,NL_CAT_LOCALE);
	setbuf(stdout, outbuf);
	if (argc >= 2)
		len = atoi(argv[1]);
	else
		len = 79;
	if (len <= 0)
	{
		fprintf(stderr, MSGSTR(LPTEST_1,"usage: lptest [width [lines]]\n"));
		exit(1);
	}
	if (argc >= 3)
		count = atoi(argv[2]);
	else
		count = 200;
	if (count <= 0)
	{
		fprintf(stderr, MSGSTR(LPTEST_1,"usage: lptest [width [lines]]\n"));
		exit(1);
	}
	if (argc > 3)
	{
		fprintf(stderr, MSGSTR(LPTEST_1,"usage: lptest [width [lines]]\n"));
		exit(1);
	}
	fc = ' ';
	for (i = 0; i < count; i++) {
		if (++fc == 0177)
			fc = ' ';
		nc = fc;
		for (j = 0; j < len; j++) {
			putchar(nc);
			if (++nc == 0177)
				nc = ' ';
		}
		putchar('\n');
	}
	(void) fflush(stdout);
	exit(0);
}

