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
static char *rcsid = "@(#)$RCSfile: wcsncmp.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 00:08:19 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: wcsncmp
 *
 *
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
 * com/lib/c/str/wcsncmp.c, bos320 2/26/91 17:49:56
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcsncmp = __wcsncmp
#endif
#endif
#include <wchar.h>

/*
 * FUNCTION: Compares at most n pairs of wchar_t from the strings
 *	     pointed to by ws1 and ws2, returning an integer as follows:
 *
 *		Less than 0	If s1 is less than ws2
 *		Equal to 0	If s1 is equal to ws2
 *		Greater than 0	If s1 is greater than ws2.
 *                                                                    
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS: 
 *	     wchar_t *ws1 - first string
 *	     wchar_t *ws2 - second string
 *	     size_t n - number of wchar_tacters to compare
 *
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/


int wcsncmp(const wchar_t *ws1, const wchar_t *ws2, size_t n)
{
	size_t i;

	if(ws1 == ws2)
		return(0);
	for(i = 0; i < n && *ws1 == *ws2++; i++)
		if(*ws1++ == '\0')
			return(0);
	return((i == n)? 0: (*ws1 - *--ws2));
}

