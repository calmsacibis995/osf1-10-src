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
static char *rcsid = "@(#)$RCSfile: wcswidth.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/06/08 20:44:18 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCCCPPC) LIBC Code-Point/Process-Code Conversion Functions
 *
 * FUNCTIONS: wcswidth
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/cppc/wcswidth.c, libccppc, 9130320 7/17/91 15:12:32
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wcswidth = __wcswidth
#endif
#endif
#include <stdlib.h>
#include <ctype.h>
#include <sys/localedef.h>

/*
  returns the number of characters for a SINGLE-BYTE codeset
*/
int
__wcswidth_latin( wchar_t *wcs, size_t n, _LC_charmap_t *hdl)
{
    int dispwidth;
    
    /**********
      if wcs is null or points to a null, return 0
    **********/
    if (wcs == (wchar_t *)NULL || *wcs == (wchar_t) '\0')
	return(0);

    /**********
      count the number of process codes in wcs, if
      there is a process code > 255, return -1
    **********/
    for (dispwidth=0; wcs[dispwidth] != (wchar_t)NULL && dispwidth<n; dispwidth++)
	if (wcs[dispwidth] > 255)
	    return(-1);

    return(dispwidth); 
}

int 
wcswidth(wchar_t *wcs, size_t n)
{
    int i;

    for (i=0; i < n ; i++)
	if (!wcs[i])
	    break;		/* Null termination is okay */
    	else if (!iswprint(wcs[i]))
	    return (-1);	/* XPG4 says so */
	    

	IF_METHOD(__lc_charmap,wcswidth)
		return METHOD(__lc_charmap,wcswidth)( wcs, n, __lc_charmap);
	else
		return __wcswidth_latin(wcs,n, __lc_charmap);
}
