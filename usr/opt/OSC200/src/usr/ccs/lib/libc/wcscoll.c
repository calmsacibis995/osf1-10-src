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
static char *rcsid = "@(#)$RCSfile: wcscoll.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/11/18 15:40:34 $";
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
 * FUNCTIONS: wcscoll
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/str/wcscoll.c, libcstr, bos320, 9130320 7/17/91 15:07:40
 */
 
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcscoll = __wcscoll
#endif
#endif
#include <stdlib.h>
#include <sys/localedef.h>
#include <string.h>
#include <sys/param.h>

/*
 * FUNCTION: Compares the strings pointed to by ws1 and ws2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If ws1 is less than ws2
 *		Equal to 0	If ws1 is equal to ws2
 *		Greater than 0	If ws1 is greater than ws2.
 *
 *           Calls the collation methods for the current locale.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS:
 *	     wchar_t *s1 - first string
 *	     wchar_t *s2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int 
wcscoll(const wchar_t *wcs1, const wchar_t *wcs2)
{
	int i;

	IF_METHOD(__lc_collate,wcscoll)
		i = METHOD(__lc_collate,wcscoll)( wcs1, wcs2, __lc_collate);
	else
		i = __wcscoll_C( wcs1, wcs2, __lc_collate);
	
	return(MAX(-1,MIN(1,i)));
}    

/*
 * FUNCTION: Compares the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 * PARAMETERS: (Uses file codes )
 *           _LC_collate_t *coll - handle to collation information
 *	     wchar_t *wcs1       - first string
 *	     wchar_t *wcs2       - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
int 
__wcscoll_C( const wchar_t *wcs1, const wchar_t *wcs2, _LC_collate_t *coll)
{
    if(wcs1 == wcs2)
	return(0);
    while(*wcs1 == *wcs2++)
	if(*wcs1++ == '\0')
	    return(0);
    
    return(*wcs1 - *--wcs2);
}
