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
static char	*sccsid = "@(#)$RCSfile: begin.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/10 15:56:17 $";
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
 * FUNCTIONS: begin
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
 * begin.c	1.2  com/cmd/man/learn,3.1,9021 9/14/89 06:29:08
 */

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include "lrnref.h"

#include <NLctype.h>

#include "learn_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_LEARN,n,s) 

begin(lesson)
char *lesson;
{

  int c, n;
  char where [LEN_MAX];
  DIR *dp;
  struct dirent *ep;
  NLchar NLtemp;

	if (((dp = opendir(".")) == NULL)) {	/* clean up play directory */
		perror(MSGSTR(LSTPLYDIR, "Start:  play directory")); /*MSG*/
		wrapup(1);
	}	
	for (ep = readdir(dp); ep != NULL; ep = readdir(dp)) {
		if (ep->d_fileno == 0)
			continue;
		n = ep->d_namlen;
		if (ep->d_name[n-2] == '.' && ep->d_name[n-1] == 'c')
			continue;
		NCdec(ep->d_name, &NLtemp);
		if (NCisalpha(NLtemp))
			unlink(ep->d_name);
	}
	closedir(dp);
	if (ask)
		return;
	sprintf(where, "%s/%s/L%s", dname, sname, lesson);
	if (access(where, 04)==0)	/* there is a file */
		return;
	perror(where);
	fprintf(stderr, MSGSTR(LSTRTNOLESS, "Start:  no lesson %s\n"),lesson); /*MSG*/
	wrapup(1);
}

