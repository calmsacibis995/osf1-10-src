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
static char	*sccsid = "@(#)$RCSfile: NLstrcpy.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:30 $";
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
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: NLstrcpy, NCstrcpy
 *
 * ORIGINS: 3 27
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
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * NLstrcpy.c	1.9 com/lib/c/str,3.1,8943 9/13/89 15:22:43
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrcpy = __NCstrcpy
#pragma weak NLstrcpy = __NLstrcpy
#endif
#endif
#include <NLchar.h>

/*
 *  Copy string s2 to s1.  s1 must be large enough.
 *  return s1.  Two versions here:  NLstrcpy (operates on ASCII with
 *  embedded NLS code points and is identical to strcpy), and NCstrcpy
 *  (operates on NLchar strings).
 */

/*
 * NAME: NLstrcpy
 *
 * FUNCTION: identical to strcpy
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the target string.
 */
char * NLstrcpy(s1, s2)
register char *s1, *s2;
{
	register char *os1;

	os1 = s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}

/*
 * NAME: NCstrcpy
 *
 * FUNCTION: like NLstrcpy except it copies double byte characters instead of mixed byte chars.
 *
 * RETURN VALUE DESCRIPTION: returns a pointer to the target string.
 */
NLchar * NCstrcpy(s1, s2)
register NLchar *s1, *s2;
{
	register NLchar *os1;

	os1 = s1;
	while(*s1++ = *s2++)
		;
	return(os1);
}

#ifdef KJI
#include <wchar.h>
#undef wstrcpy

wchar_t *
wstrcpy(s1, s2)
register wchar_t *s1, *s2;
{
	return (NCstrcpy(s1, s2));
}
#endif
