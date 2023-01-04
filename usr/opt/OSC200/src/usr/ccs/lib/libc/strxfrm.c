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
static char	*sccsid = "@(#)$RCSfile: strxfrm.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 23:09:50 $";
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
 * FUNCTIONS: strxfrm
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.18  com/lib/c/str/strxfrm.c, libcstr, bos320,9130320 7/17/91 15:06:35
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/types.h>
#include <sys/localedef.h>
#include <string.h>

/*
 * FUNCTION: strxfrm - converter character string to string
 *                     of collation weights.
 *
 * PARAMETERS:
 *           char *s1 - output string of collation weights.
 *           char *s2 - input string of characters.
 *           size_t n - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to s1.
 */

size_t
strxfrm(char *s1, const char *s2, size_t n)
{
	if (METHOD(__lc_collate,strxfrm) == NULL)
		return __strxfrm_C( s1, s2, n, __lc_collate);
	else
        	return METHOD(__lc_collate,strxfrm)(s1, s2, n, __lc_collate);
}


/*
 * FUNCTION: __strxfrm_C - Method implementing strxfrm() for C locale.
 *
 * PARAMETERS:
 *           _LC_collate_t *coll - unused.
 *
 *	     char *s1 - output string of collation weights.
 *	     char *s2 - input string of characters.
 *           size_t n - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to s1.
 */

int 
__strxfrm_C( char *s1, const char *s2, size_t n, _LC_collate_t *coll)
{
    size_t i;
    int len;

    len = strlen(s2);

    /**********
      if s1 is null or n is 0, just return the length
    **********/
    if (s1==NULL || n==0)
	return len;

    /**********
      copy n-1 bytes from s2 to s1 and null terminate s1
    **********/
    n--;
    strncpy(s1, s2, n);
    s1[n] = '\0';

    return len;
}
