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
static char *rcsid = "@(#)$RCSfile: __iswctype_std.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/30 20:20:49 $";
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
 * FUNCTIONS: __iswctype_std
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/chr/__iswctype_std.c, libcchr, bos320, 9132320m 8/11/91 17:33:40
 */
/*
 *
 * FUNCTION:  Determines if the process code, wc, has the property in mask
 *
 *
 * PARAMETERS: hdl -- The ctype info for the current locale
 *             wc  -- Process code of character to classify
 *            mask -- Mask of property to check for
 *
 *
 * RETURN VALUES: 0 -- wc does not contain the property of mask
 *              >0 -- wc has the property in mask
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <ctype.h>

int 
__iswctype_std(wint_t wc, wctype_t mask, _LC_ctype_t *hdl)
{
    /**********
      if the process code is outside the bounds of the locale or
      if the mask is -1, then return 0
    **********/
    if ((wc > hdl->max_wc) || (wc == (wint_t)-1) || (mask == -1))
	return(0);

    /**********
      if the process code is less than 256, find the mask in the direct
      table, otherwise get the index
    **********/
    if (wc < 256)
	return(hdl->_mask[wc] & mask);
    else if (wc > hdl->qidx_hbound)
        wc = hdl->qidx_hbound;

    return ((hdl->qmask[hdl->qidx[wc-256]]) & mask);
}
