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
static char *rcsid = "@(#)$RCSfile: __mbstopcs.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:21:21 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: (LIBCCHR) LIBC Character Classification Funcions
 *
 * FUNCTIONS: __mbstopcs
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/cppc/__mbstopcs.c, libccppc, 9130320 7/17/91 15:08:09
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <stdlib.h>
#include <ctype.h>

#undef __mbstopcs

int __mbstopcs_sb( wchar_t *pwcs, size_t pwcs_len, 
		     const char *s, size_t s_len, int stopchr, 
		     char **endptr, int *errn, _LC_charmap_t *handle)
{
    int cnt;

    cnt = 0;

    while(1) {
	/**********
	  if we have hit the stopchr, set the endpointer and break
	  out of the while
	**********/
	if (s[cnt] == (char) stopchr) {
	    pwcs[cnt] = (wchar_t) s[cnt];
	    cnt++;
	    *endptr = (char *)&(s[cnt]);
	    break;
	}

	/**********
	  otherwise set pwcs and increment cnt
	**********/
	pwcs[cnt] = (wchar_t) s[cnt];
	cnt++;

	/**********
	  if the end of either array has been reached, set the endpointer
	  and break out of the while loop
	**********/
	if (cnt >= pwcs_len || cnt >= s_len) {
	    *endptr = (char *)&(s[cnt]);
	    break;
	}
	
    }

    /**********
      Return the number of characters converted from s to pwcs
    **********/
    return(cnt);
}



int __mbstopcs(wchar_t *ws, size_t ws_sz, 
               char *s, size_t s_sz, int stopchr, char **endptr, int *err)
{
	if (METHOD( __lc_charmap, __mbstopcs) == NULL)
		return (__mbstopcs_sb( ws, ws_sz, s, s_sz, stopchr, endptr, err, NULL));
	else
		return METHOD( __lc_charmap, __mbstopcs)(ws, ws_sz, s, s_sz, 
					stopchr, endptr, err, __lc_charmap);
}
