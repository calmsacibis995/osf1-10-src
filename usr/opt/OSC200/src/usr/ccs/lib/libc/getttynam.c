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
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: getttynam.c,v $ $Revision: 4.2.5.3 $ (OSF) $Date: 1993/12/14 15:11:53 $";
#endif
/*
 * FUNCTIONS: getttynam 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * getttynam.c	1.6  com/lib/c/io,3.1,8943 9/12/89 18:32:59
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak getttynam_r = __getttynam_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak getttynam = __getttynam
#endif
#endif
#ifdef _THREAD_SAFE
#include <stdio.h>
#endif	/* _THREAD_SAFE */

#include <ttyent.h>

#include "ts_supp.h"


#ifdef _THREAD_SAFE

#define	SETTTYENT()	setttyent_r(&tty_fp)
#define	GETTTYENT(t)	(getttyent_r(t, line, len, &tty_fp) != TS_FAILURE)
#define	ENDTTYENT()	endttyent_r(&tty_fp)

#else

#define	SETTTYENT()	setttyent()
#define	GETTTYENT(t)	((t = getttyent()) != TS_FAILURE)
#define	ENDTTYENT()	endttyent()

#endif	/* _THREAD_SAFE */


/*
 * NAME: getttynam
 *
 * FUNCTION: searches the tty file for the name passed to it.
 *
 * RETURN VALUE DESCRIPTION: 
 *	returns a pointer to an object with ttyent structure that contains
 *	tty in the name field or a null pointer if EOF is reached or error
 */

#ifdef _THREAD_SAFE
int
getttynam_r(const char *tty, struct ttyent *ttyent, char *line, int len)
{
	FILE	*tty_fp = 0;
#else
struct ttyent *      
getttynam(const char *tty)
{
	register struct ttyent	*ttyent;
#endif	/* _THREAD_SAFE */

	TS_EINVAL(!ttyent || !line || len <= 0);
	SETTTYENT();
	while (GETTTYENT(ttyent))
		if (strcmp(tty, ttyent->ty_name) == 0) {
			ENDTTYENT();
			return (TS_FOUND(ttyent));
		}
	ENDTTYENT();
	return (TS_NOTFOUND);
}
