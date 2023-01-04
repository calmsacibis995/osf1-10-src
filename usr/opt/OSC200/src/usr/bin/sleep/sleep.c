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
static char rcsid[] = "@(#)$RCSfile: sleep.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/11 19:01:16 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (CMDCNTL) system control commands
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

/*
 *	sleep -- suspend execution for an interval
 */                                                                   

#include	<stdio.h>

#include <locale.h>
#include <nl_types.h>
#include <stdlib.h>
#include "sleep_msg.h"
nl_catd catd;

void	errormsg(int, char *);

main(int argc, char **argv)
{
	int	c, n=0;
	char	*s;

	if(argc != 2) 
		errormsg(USG,"usage: sleep seconds\n");
	s = argv[1];

	n = strtol( s, &s, 0);		/* Allow general numbers */

	if ((n < 0) || !s || *s)
	  errormsg(BADCHAR, "sleep: Specify time as a positive integer.\n");

	(void) sleep(n);	/* Don't worry how long we actually slept. */
	exit (0);
}

void
errormsg(int num, char *msg)
{
	(void) setlocale (LC_ALL,"");
        catd = catopen(MF_SLEEP,NL_CAT_LOCALE);

	fprintf(stderr, catgets(catd, MS_SLEEP, num, msg));
	exit(2);
}
