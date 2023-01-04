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
static char	*sccsid = "@(#)$RCSfile: mbtowc.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:29:01 $";
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
 * FUNCTIONS: mbtowc
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/cppc/mbtowc.c, libccppc, 9130320 7/17/91 15:14:28
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <sys/localedef.h>

int
mbtowc(wchar_t *pwc, const char *s, size_t len)
{
	IF_METHOD(__lc_charmap,mbtowc)
		return METHOD(__lc_charmap,mbtowc)( pwc,s,len, __lc_charmap);
	else
		return __mbtowc_sb( pwc,s,len, __lc_charmap);
}



int 
__mbtowc_sb( wchar_t *pwc, const char *ts, size_t len, _LC_charmap_t *handle)
{
    unsigned char *s = (unsigned char *)ts;

    /**********
      if s is NULL return 0
    **********/
    if (s == NULL)
	return(0);

    /**********
      If length == 0 return -1
    **********/
    if (len < 1) {
        _Seterrno(EILSEQ);
        return((size_t)-1);
    }

    /**********
      if pwc is not NULL pwc to s
      length is 1 unless NULL which has length 0
    **********/
    if (pwc != NULL)
	*pwc = (wchar_t)*s;
    if (s[0] != '\0')
	return(1);
    else
        return(0);
}
