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
static char *rcsid = "@(#)$RCSfile: iswctype.c,v $ $Revision: 1.1.6.3 $ (DEC) $Date: 1993/09/30 20:21:09 $";
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
 * FUNCTIONS: iswctype
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  1.2  com/lib/c/chr/iswctype.c, libcchr, bos320, 9130320 7/17/91 15:16:26
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak iswctype = __iswctype
#endif
#endif
#include <sys/localedef.h>
#include <ctype.h>

#undef iswctype
#ifdef _NAME_SPACE_WEAK_STRONG
#define iswctype __iswctype
#endif

int 
iswctype(wint_t wc, wctype_t mask)
{
	if (METHOD(__lc_ctype,iswctype) == NULL)
		return __iswctype_sb( wc, mask, __lc_ctype);
	else
		return METHOD(__lc_ctype,iswctype)( wc, mask, __lc_ctype);
}

int 
__iswctype_sb( wint_t wc, wctype_t mask, _LC_ctype_t *hdl)
{
    /**********
      if wc is outside of the bounds, or 
      if the mask is -1, then return 0
    **********/
    if ((wc > hdl->max_wc) || (wc == (wint_t)-1) || (mask == -1) )
	return(0);

    return(hdl->_mask[wc] & mask);
}
