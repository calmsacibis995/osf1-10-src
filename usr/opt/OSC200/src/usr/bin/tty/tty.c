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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tty.c,v $ $Revision: 4.2.2.3 $ (OSF) $Date: 1993/10/11 19:26:14 $";
#endif
 /* 001GZ Derived from OSF1 1.2 */
/* static char sccsid[] = "%Z%%M%	%I%  %W% %G% %U%"; */
/*
 * COMPONENT_NAME: (CMDSTAT) status
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
 */

/**********************************************************************/
/* Include File                                                       */
/**********************************************************************/
#include  <stdio.h>
#include  <unistd.h>
#include  <locale.h>
#include  "tty_msg.h"

/**********************************************************************/
/* Constant Definition / Macro Function                               */
/**********************************************************************/
#define  ISTERM   0
#define  NOTTERM  1
#define  ERROR    2

#define  MSGSTR(Num,Str)  catgets(catd,MS_TTY,Num,Str)

/**********************************************************************/
/* Function Prototype Declaration                                     */
/**********************************************************************/

/**********************************************************************/
/* Global / External Variables                                        */
/**********************************************************************/
extern int	optind;				/* Used by getopt     */

int		 sflg = FALSE;			/* s flag             */
nl_catd	 catd;					/* Catalog descriptor */

/**********************************************************************/
/* Name: main                                                         */
/* Function: Returns user's terminal name.                            */
/* Return Value: 0  Standard input is a terminal.                     */
/*               1  Standard input is not a terminal.                 */
/*              >1  An error occured.                                 */
/**********************************************************************/
int  main(argc, argv)
int  argc;
char *argv[];
{
	register char  *p;
	register int    i;

	(void) setlocale(LC_ALL,"");
	catd = catopen(MF_TTY,NL_CAT_LOCALE);

	/* 001GZ delete "?" from opstring */
	while( (i = getopt(argc, argv, "s")) != EOF ) {
		switch( i ) {
		  case 's':
			sflg = TRUE;
			break;
		  case '?':
			fprintf( stderr, MSGSTR(USAGE, "Usage: tty [-s]\n") );
			exit(ERROR);
		}
	}

	p = ttyname(0);

	if ( sflg == FALSE ) {
		if ( p )  fprintf( stdout, "%s\n", p );
		else      fprintf( stderr, "%s\n", MSGSTR(NOTTTY,"not a tty") );
	}
	exit( p ? ISTERM : NOTTERM );
}
