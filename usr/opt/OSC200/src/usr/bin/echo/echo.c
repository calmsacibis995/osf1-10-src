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
static char rcsid[] = "@(#)$RCSfile: echo.c,v $ $Revision: 4.2.5.6 $ (DEC) $Date: 1993/12/21 21:08:02 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDSH) Bourne shell and related commands
 *
 * FUNCTIONS:
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 *
 * com/cmd/sh/echo.c, cmdsh, bos320, 9130320p 6/27/91 17:48:07
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#define ECH_XPG4    "xpg4"
#define ECH_SVR4    "svr4"
/*
 * NAME:	echo
 *
 * FUNCTION:	echo - write arguments to standard output
 *
 * SYNOPSIS:	echo [arg] ...
 *
 * NOTES:	Echo writes its arguments to standard output.
 *		Escape characters are recognized and processed.
 *
 *		XPG4 does not support '-n' flag, so '-n' will be
 *		treated as a string.  Not option anymore.
 *
 * RETURN VALUE DESCRIPTION:	0
 */

int
main(argc, argv)
int argc;
char *argv[];
{
	register char	*cp;
	register int	i, wd;
	int	j;
	int	c;
        int     newline=1;
	char *getptr=NULL;
	extern int optind;

	(void) setlocale (LC_ALL, "");

	/*
	 * any arguments at all?  if not, exit
	 */
	if(--argc == 0) {
		putchar('\n');
		return (0);
	}

        /* GZ001
         * check for -n ... if env var DEC_XPG4 not set
         */

	/* DS001 Changed DEC_XPG4 environment variable to CMD_ENV as 
	 * part of final resolution of XPG/4 issues.
         */

	getptr = getenv("CMD_ENV");

	if ((getptr == NULL) || (strcmp(getptr, ECH_XPG4)))
	{
	        if (strcmp(argv[1], "-n") == 0) {
	                newline = 0;
	                argc--;
	                argv++;
	        }
	}

	/*
	 * process arguments ...
	 */
	for(i = 1; i <= argc; i++) {
		for(cp = argv[i]; *cp; cp++) {
			if(*cp == '\\')
				/*
				 * process escape sequences
				 */
				switch(*++cp) {
					case 'a':	/* alert	*/
#ifdef __STDC__
						putchar('\a');
#else
						putchar('\007');
#endif
						continue;
					case 'b':	/* backspace	*/
						putchar('\b');
						continue;

					case 'c':	/* no newline	*/
						return (0);

					case 'f':	/* formfeed	*/
						putchar('\f');
						continue;

					case 'n':	/* newline	*/
						putchar('\n');
						continue;

					case 'r':	/* carriage return */
						putchar('\r');
						continue;

					case 't':	/* tab	*/
						putchar('\t');
						continue;

					case 'v':	/* vert. tab */
						putchar('\v');
						continue;

					case '\\':	/* backslash	*/
						putchar('\\');
						continue;

					case '0':	/* octal char	*/
						j = wd = 0;
						while ((*++cp >= '0' &&
						   *cp <= '7') && j++ < 3) {
							wd <<= 3;
							wd |= (*cp - '0');
							}
						putchar(wd);
						--cp;
						continue;

					default:
						cp--;
				}

				putchar(*cp);
		}

		/*
		 * space between arguments...
		 */
		if (i < argc)
			putchar(' ');
	}

	/*
	 * ending newline
	 */
        if(newline)
               putchar('\n');

        return (0);
}
