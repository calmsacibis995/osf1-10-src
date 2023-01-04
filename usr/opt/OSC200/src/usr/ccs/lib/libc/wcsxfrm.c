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
static char *rcsid = "@(#)$RCSfile: wcsxfrm.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 20:44:56 $";
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
 * FUNCTIONS: wcsxfrm
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.2  com/lib/c/str/wcsxfrm.c, libcstr, bos320, 9130320 7/17/91 15:07:05
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcsxfrm = __wcsxfrm
#endif
#endif
#include <sys/localedef.h>
#include <string.h>


/*
 * FUNCTION: __wcsxfrm_C - Method implementing wcsxfrm() for C locale.
 *
 * PARAMETERS:
 *           _LC_collate_t *coll - unused
 *
 *	     wchar_t *ws1 - output string of collation weights.
 *	     wchar_t *ws2 - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to ws1.
 */

size_t 
__wcsxfrm_C( wchar_t *ws1, const wchar_t *ws2, size_t n, _LC_collate_t *coll)
{
    size_t i;
    int len;

    len = wcslen(ws2);

    if (ws1==NULL || n==0)
	return len;

    n--;
    wcsncpy(ws1, ws2, n);
    ws1[n] = 0x00;
    return len;
}


/*
 * FUNCTION: wcsxfrm - transform wchar string to string of 
 *                     collation weights.
 *
 *           Calls wcsxfrm() method for C locale.
 *
 * PARAMETERS:
 *	     wchar_t *ws1 - output string of collation weights.
 *	     wchar_t *ws2 - input string of characters.
 *           size_t n     - max weights to output to s1.
 *
 * RETURN VALUE DESCRIPTIONS: Returns the number of collation weights
 *                            output to ws1.
 */

size_t 
wcsxfrm(wchar_t *ws1, const wchar_t *ws2, size_t n)
{
	if (METHOD(__lc_collate,wcsxfrm) == NULL)
		return __wcsxfrm_C( ws1, ws2, n, __lc_collate);
	else
		return METHOD(__lc_collate,wcsxfrm)(ws1, ws2, n, __lc_collate);
}
