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
static char	*sccsid = "@(#)$RCSfile: mbstowcs.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:28:37 $";
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
 * FUNCTIONS: mbstowcs
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/cppc/mbstowcs.c, libccppc,  9130320 7/17/91 15:14:00
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <sys/localedef.h>

size_t 
__mbstowcs_sb( wchar_t *pwcs, const char *fs, size_t n, _LC_charmap_t *handle)
{
    const unsigned char *s = (void *)fs;
    int len = n;
    const unsigned char *s0 = s;

    /*
     * Check for attempt to reset stateful encoding
     */
    if (s==NULL)
        return (0);		/* Not using stateful encodings */

    /**********
      if pwcs is a null pointer, just count the number of characters
      in s
    **********/
    if (pwcs == (wchar_t *)NULL) {
	while (*s != '\0')
	      s++;
	return(s - s0);
    }
    
    /**********
      only do n or less characters
    **********/
    while (len-- > 0) {
	*pwcs = (wchar_t) *s;

	/**********
	  if s is null, return
	**********/
	if (*s == '\0')
	    return(s - s0);

	/**********
	  increment s to the next character
	**********/
	pwcs++;
	s++;
    }

    /**********
      Ran out of room in wcs before null was hit on s, return n
    **********/
    return(n);
}

size_t 
mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
	IF_METHOD(__lc_charmap,mbstowcs)
		return METHOD(__lc_charmap,mbstowcs)( pwcs, s, n, __lc_charmap);
	else
		return (__mbstowcs_sb( pwcs, s, n, __lc_charmap));
}

