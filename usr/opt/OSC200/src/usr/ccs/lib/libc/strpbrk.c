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
static char	*sccsid = "@(#)$RCSfile: strpbrk.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/12/21 14:34:29 $";
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
 * FUNCTIONS: strpbrk
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
 * strpbrk.c	1.11  com/lib/c/str,3.1,8943 10/20/89 12:57:52
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>

/*
 * FUNCTION: Returns a pointer to the first occurrence in the string
 *	     pointed to by s1 of any character from the string pointed
 *	     to by s2.  A NULL pointer is returned if no character
 *	     matches are found.
 *
 * PARAMETERS:
 *	     char *s1 - string to be searched
 *	     char *s2 - string containing characters to be found
 *
 * RETURN VALUE DESCRIPTION: Returns a pointer to the first occurrence of
 *	     any character from s2 in s1; NULL if no matches are found.
 */
/*LINTLIBRARY*/


char *
strpbrk(const char *s1, const char *s2)
{
	unsigned char *p;
        register int pp;
        register int qq;

        while (TRUE) {
                p = (unsigned char *)s2;
                qq = *(unsigned char *)s1;
                if(qq == '\0')
                        return(NULL);
                while (TRUE) {
                        pp = *p;
                        if (pp != '\0' && pp != qq)
                                ++p;
                        else
                                break;
		}
                if(pp != '\0')
                        return((char *)s1);
                s1++;
	}

}
