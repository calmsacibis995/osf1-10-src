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
static char	*sccsid = "@(#)$RCSfile: NLstrcmp.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:27 $";
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
 * FUNCTIONS: NLstrcmp, NCstrcmp 
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
 * NLstrcmp.c	1.11  com/lib/c/str,3.1,9013 2/11/90 17:34:34
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrcmp = __NCstrcmp
#pragma weak NLstrcmp = __NLstrcmp
#endif
#endif
#include <sys/types.h>
#include <string.h>
#include <NLchar.h>

/*
 *
 *  Compare strings.  Returns:  s1>s2; >0  s1==s2; 0  s1<s2; <0.
 *  Two versions here:  NLstrcmp (operates on ASCII with embedded NLS
 *  code points) and NCstrcmp (operates on NLchars).
 *
 */


int NLstrcmp(const char *s1, const char *s2) 
{
	return (strcoll(s1, s2));
} 
	
int NCstrcmp(wchar_t *s1, wchar_t *s2)
{
	return (wcscoll(s1, s2));
}

#ifdef _KJI
#include <wchar.h>
#undef wstrcmp

int wstrcmp(s1, s2)
wchar_t *s1, *s2;
{
	return (NCstrcmp(s1, s2));
}
#endif
