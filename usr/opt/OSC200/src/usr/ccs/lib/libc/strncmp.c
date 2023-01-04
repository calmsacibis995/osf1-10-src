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
static char	*sccsid = "@(#)$RCSfile: strncmp.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/12/21 14:34:28 $";
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
 * FUNCTIONS: strncmp
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
 * strncmp.c	1.10  com/lib/c/str,3.1,8943 9/13/89 16:30:48
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <string.h>
#include <sys/param.h>

/*
 * FUNCTION: Compares at most n pairs of characters from the strings
 *	     pointed to by s1 and s2, returning an integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *                                                                    
 * NOTES:    Handles the pathological case where the value of n equals
 *	     the maximum value of an unsigned long integer.
 *
 * PARAMETERS: 
 *	     char *s1 - first string
 *	     char *s2 - second string
 *	     size_t n - number of characters to compare
 *
 * RETURN VALUE DESCRIPTION: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/


int	
strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;
	int cdiff;
        register int ch1, ch2;

        if(s1 == s2)
                return(0);
        i = 0;
        while (TRUE) {
                ch1 = *(unsigned char *)s1;
                ch2 = *(unsigned char *)s2;
                if (i < n && ch1 == ch2) {
                        if (ch1 == '\0')
                                return(0);
                        s1++;
                        s2++;
                        i++;
		}
                else
                        break;
	}
        if (i == n)
                return(0);
        else {
                cdiff = ch1 - ch2;
                return(MAX(-1,MIN(1,cdiff)));
	}

}
