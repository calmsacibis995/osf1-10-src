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
static char	*sccsid = "@(#)$RCSfile: NCcolval.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/06/08 01:15:42 $";
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
 * FUNCTIONS: NCcolval
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
 * NCcolval.c	1.5  com/lib/c/nls,3.1,9013 2/28/90 10:26:55
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NCcolval = __NCcolval
#endif
#include <sys/types.h>
#include <sys/localedef.h>
#include "collation.h"
#include "patlocal.h"

/*LINTLIBRARY*/
/*
 *
 *  Find the (not necessarily unique) collation value of a character.
 *
 *  Elements in NLcoldesc contain four fields: 
 *	cd_stroff	the offset from the start of the NLcoldesc array 
 *			of a collating string,
 *	cd_repoff	the offset from the start of the NLcoldesc array
 *			of a replacement string
 *	cd_cval		a collate value
 *	cd_cuniq	a coluniq value
 *
 *  The fields are interpreted as follows:
 *      cd_stroff  cd_repoff  cd_cval   cd_cuniq
 *      ---------  ---------  -------   --------
 *       "from"      "to"       0         0        n-to-n mapping
 *       "from"        0       coll      uniq      multi-char coll symbol
 *         0           0       coll      uniq      default for char
 *     
 *  IF the "cd_stroff" field is non-zero and the cd_cval is zero,
 *  then the entry defines a many-to-many replacement mapping.
 *  NCcolval is called recursively to determine the collation value of
 *  the first character in the replacement string.
 *  
 *  If the "repoff" field in zero, and "stroff" is non-zero, then 
 *  the entry defines a multi-character collating element (e.g. Spanish
 *  'ch'); input pointer is set past element, and collation value
 *  is returned.
 *  
 *  If "cd_stroff" and "cd_repoff" are zero, then the entry defines the
 *  collating (and coluniq) value for the character itself; and collation
 *  value is returned.
 */

/*  Find collating value for character chr.  If NCcollate returns a negative
 *  value, go through the collating descriptors until find the one for the
 *  individual character and use that (not necessarily unique) collation 
 *  value.  For 1-n collation, use the collation value of the first 
 *  character in the replacement string.
 */
int
NCcolval (wchar_t nlc)
{
    wchar_t colval;

    /*
     * Pretend we're passed a collation handle, just like the OSF/1
     * 1.2 collation functions.
     */
    _LC_collate_t *phdl = __lc_collate;

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
