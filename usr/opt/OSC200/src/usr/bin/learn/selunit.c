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
static char	*sccsid = "@(#)$RCSfile: selunit.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:57:33 $";
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
 * FUNCTIONS: selunit, abs, grand
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
 * selunit.c	1.2  com/cmd/man/learn,3.1,9021 9/14/89 06:43:18
 */

#include "stdio.h"
#include "lrnref.h"

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

int	nsave	= 0;
int	review	= 0;

selunit()
{
	static char dobuff[50];
	static char saved[LEN_L];
	char fnam[LEN_MAX], s[LEN_MAX], zb[MAX_LEN];
	char posslev[LEN_L][LEN_L];
	int diff[LEN_L], i, k, m, n, best, alts;
	char *getlesson();
	FILE *f;

	if (again) {
		again = 0;
		if (todo=getlesson()) {
			if (!review)
				unsetdid(todo);
			return;
		}
		wrapup(1);
	}
	while (ask) {
		printf(MSGSTR(LWHATELSSN, "What lesson? ")); /*MSG*/
		fflush(stdout);
		gets(dobuff);
		if (STRCMP(dobuff, MSGSTR(LBYE, "bye")) == 0) /*MSG*/
			wrapup(1);
		level = dobuff;
		if (todo=getlesson()) {
			return;
		}
	}
	alts = 0;
retry:
	f = scrin;			/* use old lesson to find next */
	if (f==NULL) {
		sprintf(fnam, "%s/%s/L%s", dname, sname, level);
		f = fopen(fnam, "r");
		if (f==NULL) {
			perror(fnam);
			fprintf(stderr, MSGSTR(LNOSCRIPT, "Selunit:  no script for lesson %s.\n"), level); /*MSG*/
			wrapup(1);
		}
		while (fgets(zb, MAX_LEN, f)) {
			trim(zb);
			if (STRCMP(zb, MSGSTR(LPNEXT, "#next"))==0) /*MSG*/
				break;
		}
	}
	if (feof(f)) {
		printf(MSGSTR(LCONGRAT, "Congratulations; you have finished this sequence.\n")); /*MSG*/
		fflush(stdout);
		todo = 0;
		wrapup(-1);
	}
	for(i=0; fgets(s, LEN_MAX, f); i++) {
		sscanf(s, "%s %d", posslev[i], &diff[i]);
	}
	best = -1;
	/* cycle through lessons from random start */
	/* first try the current place, failing that back up to
	     last place there are untried alternatives (but only one backup) */
	n = grand()%i;
	for(k=0; k<i; k++) {
		m = (n+k)%i;
		if (already(posslev[m]))
			continue;
		if (best<0)
			best = m;
		alts++;				/* real alternatives */
		if (abs(diff[m]-speed) < abs(diff[best]-speed))
			best = m;
	}
	if (best < 0 && nsave) {
		nsave--;
		strcpy(level, saved);
		goto retry;
	}
	if (best < 0) {
		/* lessons exhausted or missing */
		printf(MSGSTR(LNOALTLESS, "Sorry, there are no alternative lessons at this stage.\n")); /*MSG*/
		printf(MSGSTR(LGETHELP, "See someone for help.\n")); /*MSG*/
		fflush(stdout);
		todo = 0;
		return;
	}
	strcpy (dobuff, posslev[best]);
	if (alts>1) {
		nsave = 1;
		strcpy(saved, level);
	}
	todo = dobuff;
	fclose(f);
}

abs(x)
{
	return(x>=0 ? x : -x);
}

grand()
{
	static int garbage;
	int a[2], b;

	time(a);
	b = a[1]+10*garbage++;
	return(b&077777);
}
