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
static char	*sccsid = "@(#)$RCSfile: ctermid.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:43:45 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)
#endif
/*
 * FUNCTIONS: ctermid 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/*LINTLIBRARY*/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak ctermid = __ctermid
#endif
#include <stdio.h>

extern char *strcpy();

#ifndef _THREAD_SAFE
static char res[L_ctermid];
#endif

/*
* Thread-safe ctermid 
*/
#ifdef _THREAD_SAFE
char *
ctermid(char *s)
{
	if (s != NULL)
		return(strcpy(s,"/dev/tty"));
	else
		return(NULL);
}
#else /* _THREAD_SAFE */
char *
ctermid(char *s)
{
	return (strcpy(s != NULL ? s : res, "/dev/tty"));
}
#endif /* _THREAD_SAFE */
