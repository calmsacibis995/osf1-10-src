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
static char *rcsid = "@(#)$RCSfile: __mbtopc.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:21:33 $";
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
 * FUNCTIONS: __mbtopc
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.2  com/lib/c/cppc/__mbtopc.c, libccppc, 9130320 7/17/91 15:08:41
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <stdlib.h>

#undef __mbtopc

int __mbtopc(wchar_t *ws, char *s, int count, int *err)
{
	if (METHOD(__lc_charmap,__mbtopc) == NULL)
		return (__mbtopc_sb( ws, s, count, err, NULL));
	else
		return METHOD(__lc_charmap,__mbtopc)( ws, s, count, err, __lc_charmap);
}

int __mbtopc_sb( wchar_t *pwc, char *s, size_t len, int *err, _LC_charmap_t *handle)
{
    /**********
      If length == 0 return -1
    **********/
    if (len < 1) {
	*err = 1;
	return((size_t)0);
    }

    /**********
      if s is NULL or points to a NULL return 0
    **********/
    if (s == (char *)NULL || *s == '\0') {
	*err = -1;
	return(0);
    }

    /**********
      If pwc is NULL, just return the number of bytes
      otherwise set pwc to s
    **********/
    if (pwc != (wchar_t *)NULL)
	*pwc = (wchar_t)*s;
    return(1);
}
    
		



