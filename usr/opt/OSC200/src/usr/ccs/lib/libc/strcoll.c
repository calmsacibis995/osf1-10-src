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
static char	*sccsid = "@(#)$RCSfile: strcoll.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/10/05 21:03:14 $";
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
 * FUNCTIONS: strcoll, wcscoll
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.21  com/lib/c/str/strcoll.c, libcstr, bos320,9130320 7/17/91 15:06:05
 */
 
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/types.h>
#include <sys/localedef.h>
#include <string.h>
#include <sys/param.h>

/*
 * FUNCTION: Compares the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 *           Calls the collation methods for the current locale.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *s1 - first string
 *	     char *s2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

#pragma weak NLstrcmp = strcoll

int 
strcoll(const char *s1, const char *s2)
{
	int i;

	if (METHOD( __lc_collate, strcoll) == NULL)
		i = __strcoll_C( s1, s2, __lc_collate);
	else
		i = METHOD( __lc_collate, strcoll)( s1, s2, __lc_collate);

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
 *	     char *s1 - first string
 *	     char *s2 - second string
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */

int 
__strcoll_C( const char *s1, const char *s2, _LC_collate_t *coll)
{
    if(s1 == s2)
	return(0);
    while(*s1 == *s2++)
	if(*s1++ == '\0')
	    return(0);
    return(*s1 - *--s2);
}
