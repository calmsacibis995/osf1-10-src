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
static char	*sccsid = "@(#)$RCSfile: colrm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:22:38 $";
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
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: colrm
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * colrm.c     1.11  com/cmd/files,3.1,9021 4/3/90 21:45:15";
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
COLRM removes unwanted columns from a file
*/

#include <stdio.h>
#include <sys/limits.h>
#include <locale.h>

#include "colrm_msg.h" 
#include <NLchar.h>
#define MSGSTR(n,s) NLgetamsg(MF_COLRM, MS_COLRM, n, s) 

/*
 * NAME: colrm [startcol [endcol]]
 *
 * FUNCTION: COLRM removes unwanted columns from a file
 * NOTE:  If called with only one number then colrm removes columns startcol 
 *        to the end of the line.
 *        If called with two numbers then colrm removes columns startcol to 
 *        endcol.
 */
main(argc,argv)
char **argv;
{
	register int ct = 0, first = 0, last = 0,i = 0,tmpct;
	NLchar c[2], lc, buf[MAX_INPUT];

	(void) setlocale(LC_ALL,"");
    	if (argc > 3) {
        	fprintf(stderr,MSGSTR(USAGE,"usage: colrm first_column [last_column]\n"));
		exit(1);
	}
	if (argc > 1)
		first = atoi(*++argv);
	if ( first < 1) {
		fprintf(stderr,MSGSTR(ERROR1,"colrm: start column is less than 1.\n"));	
		exit(1);
	}
	if (argc > 2) {
		last = atoi(*++argv);
		if (first > last) {
		  fprintf(stderr,MSGSTR(ERROR2,
			"colrm: start column is greater then end column.\n")); 
		  exit(1);
		}
	}

	while (!feof(stdin)) {

		c[0] = getwc(stdin);

		if (c[0] == '\n') {            /* if newline restart counter */
			buf[i] = c[0];
			buf[i+1] = '\0';
			fprintf(stdout,"%S",buf);
			ct = 0;
			i = 0;
			continue;
		}
		if (c[0] == '\b' && ct > 0) {
			ct--;
			i--;
			continue;
		}	
		ct++;
		if (last != 0 && ct > last) {
			buf[i++] = c[0];
			continue;
		}
		if (c[0] == '\t') {
			tmpct = ct;
			ct = ct + 8 & ~ 7;
			if ((last > 0 && tmpct <= first && ct >= first) || 
                                         (last > 0 && ct > first && ct >= last)) {
				while (tmpct <= ct) {
					tmpct++;
					if ((tmpct-1 >= first) && (tmpct-1 <= last))
						continue;
					buf[i++] = ' ';
				}
				continue;
			}	
		}
		if (ct >= first && (last == 0 || ct <= last)) continue;
		buf[i++] = c[0];
	} /* end of while */
	exit(0);
} /* end of main */



