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
static char	*sccsid = "@(#)$RCSfile: strncat.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 14:34:26 $";
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
 * FUNCTIONS: strncat
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
 * strncat.c	1.11  com/lib/c/str,3.1,8943 9/13/89 16:30:36
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>

/*
 * FUNCTION: Appends not more than n characters of the string pointed to
 *	     by s2, stopping before a null character is appended, to the
 *	     end of the string pointed to by s1.  The initial character
 *	     of s2 overwrites the null character at the end of s1.  A
 *	     terminating null character is always appended to the result.
 *	     No check is made for overflow of the array pointed to by s1.
 *	     Overlapping copies toward the left work as expected, but
 *	     overlapping copies to the right may give unexpected results.
 *
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS:
 *	     char *s1 - appended string
 *	     char *s2 - copied string
 *	     size_t n - number of characters to append
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer equal to s1.
 */                                                                   
/*LINTLIBRARY*/


char	*
strncat(char *s1, const char *s2, size_t n)
{
	char *os1;

	os1 = s1;
	while(*s1)
		s1++;
	while(*s1 = *s2) {
	        s1++;
	        s2++;
		if(n == 0) {
			*--s1 = '\0';
			break;
		}
	        n--;
        }
	return(os1);
}
