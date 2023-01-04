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
static char	*sccsid = "@(#)$RCSfile: mktemp.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/11/13 10:43:38 $";
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
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: (LIBBSD)  Berkeley Compatibility Library
 *
 * FUNCTIONS:  mktemp
 *
 * ORIGINS: 26 27
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#include <stdio.h>		/* for NULL		*/
#include <sys/access.h>		/* for access(,F_OK)...	*/

#define	TMPSIZE	6

#ifdef KJI
#include <sys/NLchar.h>
#endif

extern int strlen(), access(), getpid();

/*
 * NAME:	mktemp
 *
 * FUNCTION:	mktemp - construct a unique filename
 *
 * NOTES:	Mktemp expects a string of length at least 6, with
 *		six trailing 'X's, and overwrites the X's with a
 *		(hopefully) unique encoding of the process' pid and
 *		a pseudo-random number.
 *
 * RETURN VALUE DESCRIPTION:
 *		If a unique filename was not generated, a "/" is returned.
 *		Else the filename is returned.
 */

char *
mktemp(template)
char *template;
{
	char *pt, *s = template;
	pid_t pid;
	char t[TMPSIZE + 1];	/* hold a unique number */
	int i;

  pid = getpid();

  for(i = 0; i <= TMPSIZE; i++)  {
		t[TMPSIZE - i] = (pid % 10) + '0';
		pid /=10;
  }

  t[TMPSIZE + 1] = '\0';

  pt = t;
#ifdef KJI
	char *sp;		/* current data point in s */
	char *xptr = NULL;	/* pointer to first 'X' in string */

	s += strlen(template);	/* point at the terminal null */
	pt += sptrlen(pt);

	/* search forward for the first of the trailing 'X's in s */
	for(sp = template; *sp != '\0'; sp+=NLchrlen(sp))
		if(NCdechr(sp) == 'X') {
			if(xptr == NULL)
				xptr = sp;
		} else
			xptr = NULL;
	if(xptr == NULL)
		xptr = s;

	while(--s >= xptr)
#else
	s += strlen(template);	/* point at the terminal null */
	pt += strlen(pt);	/* get a unique number	*/
	while((*--s == 'X') && (pt > t))
#endif
		*s = *--pt;

	s++;
	i = 'a';

	while(access(template, F_OK) != -1) {
		if (i=='z')
			return("/");
		*s = i++;
	}

	return(template);
}
