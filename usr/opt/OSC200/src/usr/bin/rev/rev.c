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
static char	*sccsid = "@(#)$RCSfile: rev.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/07/14 10:46:28 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.3
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (CMDFILES) commands that manipulate files
 *
 * FUNCTIONS: rev
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
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * rev.c	1.7  com/cmd/files,3.1,9013 9/12/89 14:51:46
 */
#include <stdio.h>
#include <locale.h>

/* reverse lines of a file */

#define N 256
FILE *input;

#include "rev_msg.h" 
#define MSGSTR(n,s) NLgetamsg(MF_REV, MS_REV, n, s) 

#include <NLchar.h>
NLchar line[N];

/*
 * NAME: rev [file] ...
 *                                                                    
 * FUNCTION:  Reverse lines of a file
 */  
main(argc,argv)
char **argv;
{
	register int i, c, eof = 0;
	
	(void) setlocale(LC_ALL,"");
	input = stdin;
	do {
		eof = 0;
		if(argc>1) {
			if((input=fopen(argv[1],"r"))==NULL) {
				fprintf(stderr,MSGSTR(OPENFAIL, "rev: cannot open %s\n"), /*MSG*/
					argv[1]);
				exit(1);
			}
		}
		for(;;){
			for(i=0;i<N;i++) {
				line[i] = c = getwc(input);
				switch(c) {
				case EOF:
					eof++;
					break;
				default:
					continue;
				case '\n':
					break;
				}
				break;
			}
			if (eof > 0 ) break;
			while(--i>=0)
				putwc(line[i],stdout);
			putwc('\n',stdout);
		}
		fclose(input);
		argc--;
		argv++;
	} while(argc>1);
	exit(0);
}
