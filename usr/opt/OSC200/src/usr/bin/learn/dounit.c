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
static char	*sccsid = "@(#)$RCSfile: dounit.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:56:34 $";
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
 * COMPONENT_NAME: (CMDMAN) commands that allow users to read online
 * documentation
 *
 * FUNCTIONS: dounit
 *
 * ORIGINS: 26, 27 
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
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
 * dounit.c	1.2  com/cmd/man/learn,3.1,9021 9/14/89 06:30:37
 */

#include "stdio.h"
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

int	remind = 2;		/* to remind user of "again" and "bye" */
extern	int	noclobber;

dounit()
{
	char tbuff[LEN_MAX];

	if (todo == 0)
		return;
	wrong = 0;
retry:
	if (!noclobber) {
		begin(todo);		/* clean up play directory */
	}
	sprintf(tbuff, "%s/%s/L%s", dname, sname, todo); /* script = lesson */
	scrin = fopen(tbuff, "r");
	if (scrin == NULL) {
		perror(tbuff);
		fprintf(stderr, MSGSTR(LDOUNIT, "Dounit:  no lesson %s.\n"), tbuff); /*MSG*/
		wrapup(1);
	}

	copy(0, scrin);			/* print lesson, usually */
	if (more == 0)
		return;
	copy(1, stdin);			/* user takes over */
	if (skip)
		setdid(todo, sequence++);
	if (again || skip)		/* if "again" or "skip" */
		return;
	if (more == 0)
		return;
	copy(0, scrin);			/* evaluate user's response */

	if (comfile >= 0)
		close(comfile);
	wait(&didok);
	didok = (status == 0);
	if (!didok) {
		wrong++;
		if (wrong > 1)
			printf(MSGSTR(LSORRYSTILL, "\nSorry, that's still not right.  Do you want to try again?  ")); /*MSG*/
		else
			printf(MSGSTR(LSORRY, "\nSorry, that's not right.  Do you want to try again?  ")); /*MSG*/
		fflush(stdout);
		for(;;) {
			gets(tbuff);
			if (NLyesno(tbuff) == 1) {
				printf(MSGSTR(LTRYPROBAGN, "Try the problem again.\n")); /*MSG*/
				if (remind--) {
					printf(MSGSTR(LWHENEVER, "[ Whenever you want to re-read the lesson, type \"again\".\n")); /*MSG*/
					printf(MSGSTR(LYOUCANLEAVE, "  You can always leave learn by typing \"bye\". ]\n")); /*MSG*/
				}
				goto retry;
			} else if (STRCMP(tbuff, MSGSTR(LBYE, "bye")) == 0) { /*MSG*/
				wrapup(0);
			} else if (NLyesno(tbuff) == 0) {
				wrong = 0;
				printf(MSGSTR(LOKTHATWAS, "\nOK.  That was lesson %s.\n"), todo); /*MSG*/
				printf(MSGSTR(LSKIPPING, "Skipping to next lesson.\n\n")); /*MSG*/
				fflush(stdout);
				break;
			} else {
				printf(MSGSTR(LPLEASETYPE, "Please type yes, no or bye:  ")); /*MSG*/
				fflush(stdout);
			}
		}
	}
	setdid(todo, sequence++);
}

