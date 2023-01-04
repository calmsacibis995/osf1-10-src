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
static char rcsid[] = "@(#)$RCSfile: echo.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/10 15:18:38 $";
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
 *	1.12  com/cmd/sh/sh/echo.c, , bos320, 9134320 8/12/91 14:57:42
 */

#include	"defs.h"
#include	<stdlib.h>

#define	echoexit(a)	flushb();return(a)

extern int exitval;

/*	
 *	Echo() argument "argv" is decoded.
 */

echo(argc, argv)
uchar_t **argv;
{
	register uchar_t *cp;
	register int	i, wd;
	int	j, newline;
	
	if(--argc == 0) {
		prc_buff('\n');
		echoexit(0);
	}

        /*
         * check for -n
         */
        if(strcmp((char *)argv[1], "-n") == 0) {
                newline = 0;
                argc--;
                argv++;
        }
        else
                newline = 1;

	for(i = 1; i <= argc; i++) 
	{
		sigchk();
		for(cp = argv[i]; *cp; cp++) 
		{
#ifndef _SBCS
                        int     mblength = mblen((char *)cp, MB_CUR_MAX);
                        if (mblength > 1) {
                                while(--mblength){
                                        prc_buff (*cp);
                                        cp++;
                                }
			} else
#endif
			if(*cp == '\\')
			switch(*++cp) 
			{
				case 'b':
					prc_buff('\b');
					continue;

				case 'c':
					echoexit(0);

				case 'f':
					prc_buff('\f');
					continue;

				case 'n':
					prc_buff('\n');
					continue;

				case 'r':
					prc_buff('\r');
					continue;

				case 't':
					prc_buff('\t');
					continue;

				case 'v':
					prc_buff('\v');
					continue;

				case '\\':
					prc_buff('\\');
					continue;
				case '0':
					j = wd = 0;
					while ((*++cp >= '0' && *cp <= '7') && j++ < 3) {
						wd <<= 3;
						wd |= (*cp - '0');
					}
					prc_buff(wd);
					--cp;
					continue;

				default:
					cp--;
			}
			prc_buff(*cp);
		}
                /*
                 * space between arguments
                 */
                if (i < argc)
                        prc_buff(' ');
	}
        /*
         * ending newline
         */
        if (newline)
                prc_buff('\n');
	echoexit(0);
}

