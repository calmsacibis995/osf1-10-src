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
static char	*sccsid = "@(#)$RCSfile: wctomb.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 20:45:15 $";
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
 * FUNCTIONS: wctomb
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  1.2  com/lib/c/cppc/wctomb.c, libccppc, 9130320 7/17/91 15:15:25
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdlib.h>
#include <ctype.h>
#include <sys/localedef.h>

int
__wctomb_sb( char *s, wchar_t pwc, _LC_charmap_t *handle)
{

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == NULL)
	return(0);

    if (pwc > 255)
	return(-1);

    s[0]= (char) pwc;

    return(1);
}

int 
wctomb(char *s, wchar_t pwc)
{
	IF_METHOD(__lc_charmap, wctomb)
		return METHOD(__lc_charmap, wctomb)( s, pwc, __lc_charmap);
	else
		return __wctomb_sb( s, pwc, __lc_charmap);
}

