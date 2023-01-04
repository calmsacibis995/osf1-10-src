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
static char	*sccsid = "@(#)$RCSfile: wcspbrk.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 00:08:35 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: wcspbrk
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
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
 * wcspbrk.c	1.4  com/lib/c/nls,3.1,9013 2/27/90 21:44:33
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcspbrk = __wcspbrk
#endif
#endif
#include <sys/types.h>

/*
 *  NAME: wcspbrk
 *
 *  FUNCTION: Locate the first occurence of characters in a string.
 *
 * PARAMETERS:
 *	wchar_t *string1	-	the wide character string
 *	wchar_t *string2	-	the wide character string
 *
 *  RETURN VALUE DESCRIPTION:
 *  Return ptr to first occurrence of any wchar_t from `string2'
 *  in the wchar_t string `string1'; NULL if none exists.
 */

wchar_t *
wcspbrk(const wchar_t *string1, const wchar_t *string2)
{
	register const wchar_t *p;

	do {
		for(p = string2; *p != 0 && *p != *string1; ++p)
			;
		if(*p != 0) {
			return (wchar_t *)(string1);
		}
	}
	while(*string1++);
	return(NULL);
}
