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
static char	*sccsid = "@(#)$RCSfile: strrchr.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/10/19 18:38:08 $";
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
 * COMPONENT_NAME: (LIBCSTR) Standard C Library String Handling Functions
 *
 * FUNCTIONS: strrchr
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
 * strrchr.c	1.11  com/lib/c/str,3.1,8943 10/20/89 12:59:47
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>

/*
 * FUNCTION: Returns a pointer to the last occurrence of c, converted
 *	     to a char, in the string pointed to by s.  A NULL pointer
 *	     is returned if the character does not occur in the string. 
 *	     The terminating null character is considered to be part of
 *	     the string. 
 *
 * PARAMETERS: 
 *	     char *s - string to be searched
 *	     int  c  - character to be found
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the last occurrence of
 *	     character c in string s; NULL if c is not found in s.
 */
/*LINTLIBRARY*/


char	*
strrchr(const char *s, int c)
{
	char *r;
	register int ch ;

	r = NULL;
	c = (unsigned char)c;
	while (TRUE) {
		ch = *(unsigned char *)s ;
		if (ch == c)
			r = (char *)s;
		if (ch)
			s++;
		else
			return(r);
	}
}
