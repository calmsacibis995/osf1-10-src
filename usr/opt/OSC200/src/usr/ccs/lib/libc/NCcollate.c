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
static char	*sccsid = "@(#)$RCSfile: NCcollate.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/06/08 01:15:37 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support
 *
 * FUNCTIONS: NCcollate
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NCcollate.c	1.2  com/lib/c/nls,3.1,9013 2/27/90 21:27:19
 */

/* This function returns the primary collation value for the wchar_t
 * argument. The returned value can be either 
 *	positive -	the real primary (lc_collate) value 
 *	zero     -	the wchar_t is non-collating (ignore)
 *      negative -	the real collation value is to be found in
 *			lc_coldesc (see NLxcol)
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCcollate = __NCcollate
#endif
#include <sys/types.h>
#include <sys/localedef.h>
#include "collation.h"
#include "patlocal.h"

#ifdef NCcollate
#undef NCcollate
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#define NCcollate __NCcollate
#endif

int
#ifdef _NO_PROTO
NCcollate(nlc)
wchar_t nlc;
#else
NCcollate(wchar_t nlc)
#endif
{
    wchar_t colval;

    /*
     * Pretend we're passed a collation handle, just like the OSF/1
     * 1.2 collation functions.
     */
    _LC_collate_t *phdl = __lc_collate;

    /*
     * Check for a compressed collation table.  When the last n code-
     * points in a locale all have the same collation weight,
     * localedef makes them all share the last entry in the collation
     * table.
     */
    if (nlc > phdl->co_hbound)
	nlc = phdl->co_hbound;

    /*
     * A locale has a collation table only if its source file has an
     * LC_COLLATE category.
     */
    if (phdl->co_coltbl == (_LC_coltbl_t *) NULL)
	return 0;

    /*
     * Check if there are any collation elements for this character.
     * If there are, return a negative result.  Our caller will pass
     * that to _NLxcolu() to get the element's collation value.
     */
     if (phdl->co_coltbl[nlc].ct_collel != (_LC_collel_t *) NULL)
	 return -((int) nlc);

    /*
     * Get the primary collation value.  _getcolval() returns a byte
     * string for use in string compares, so need to convert that to
     * a collation weight.
     */
    (void) _getcolval(phdl, &colval, nlc, "", 0);
    colval = byte_string_to_collation(colval);

    /*
     * An out-of-range collation value means the character should be
     * ignored in collating.
     */
    if ((colval < MIN_UCOLL) || (colval > MAX_UCOLL))
	return 0;

    /*
     * Our OSF/1 1.0 callers expect 16-bit collation values, but the
     * values from the 1.2 locales use all 32-bits.  It's safe to
     * truncate the 1.2 values because their high 16-bits are always
     * 0x0101 for single-byte locales.
     */
    return (unsigned short) colval;
}
