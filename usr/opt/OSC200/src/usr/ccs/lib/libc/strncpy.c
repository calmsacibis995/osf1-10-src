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
static char	*sccsid = "@(#)$RCSfile: strncpy.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/11/18 15:40:11 $";
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
 * FUNCTIONS: strncpy
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
 * strncpy.c	1.11  com/lib/c/str,3.1,8943 9/13/89 16:31:01
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>

/*
 *
 * FUNCTION: Copies not more than n characters of the string pointed to
 *	     by s2, stopping after a null character is copied, into the
 *	     string pointed to by s1.  If s2 is less than n characters
 *	     long, null characters are appended to s1 until exactly n
 *	     characters have been written.  If s2 is n or more characters
 *	     long, then only the first n characters are copied and the
 *	     result is not terminated with a null character.  No check
 *	     is made for overflow of the array pointed to by s1. 
 *	     Overlapping copies toward the left work as expected, but
 *	     overlapping copies to the right may give unexpected results.
 *
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS: 
 *	     char *s1 - overlaid string
 *	     char *s2 - copied string
 *	     size_t n - number of characters to copy
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer equal to s1.
 */
/*LINTLIBRARY*/

#pragma weak NLstrncpy = strncpy

char 	*
strncpy(char *s1, const char *s2, size_t n)
{
	char *os1 = s1;
	size_t i;

	for (i = 0; i < n;)
		if ((*s1++ = *s2++) == '\0')
			for (i++; i < n; i++)
				*s1++ = '\0';
		else
			i++;
	return (os1);
}
