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
static char	*sccsid = "@(#)$RCSfile: NLxcol.c,v $ $Revision: 4.2.5.5 $ (DEC) $Date: 1993/11/23 21:33:03 $";
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
 * FUNCTIONS: NLxcol, NCxcol, NLxcolu, NCxcolu
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NLxcol.c	1.12  com/lib/c/nls,3.1,9013 2/27/90 21:36:10
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/types.h>
#include <sys/localedef.h>
#include "collation.h"
#include "patlocal.h"

/*
 *
 *  Perform extended collation, according to the lc_coldesc element
 *  indexed by "desc". Passed parameters are: "desc" (index to 
 *  coldesc element), str (pointer to input +1), rstr (pointer to
 *  replacement).  
 *  Elements in lc_coldesc contain four fields: 
 *	cd_stroff	the offset from the start of the lc_strings array 
 *			of a collating string,
 *	cd_repoff	the offset from the start of the lc_strings array
 *			of a replacement string
 *	cd_cval		a collate value
 *	cd_cuniq	a coluniq value
 *
 *  The fields are interpreted as follows:
 *      cd_stroff  cd_repoff  cd_cval   cd_cuniq
 *      ---------  ---------  -------   --------
 *       "from"      "to"       0        uniq      n-to-n mapping
 *       "from"        0       coll      uniq      multi-char coll symbol
 *         0           0       coll      uniq      default for char
 *     
 *  IF the "cd_stroff" field is non-zero and the cd_cval is zero,
 *  then the entry defines a many-to-many replacement mapping.
 *  If the input matches the "cd_stroff" string, then it is  
 *  replaced by the replacement string. The input pointer is modified
 *  to point past string to be replaced, the replacement pointer is
 *  set to the replacement string, and -1 is returned.
 *  
 *  If the "repoff" field in zero, and "stroff" is non-zero, then 
 *  the entry defines a multi-character collating element (e.g. Spanish
 *  'ch'); input pointer is set past element, and collation value
 *  is returned.
 *  
 *  If "cd_stroff" and "cd_repoff" are zero, then the entry defines the
 *  collating (and coluniq) value for the character itself; and collation
 *  value is returned.
 *
 * Change Activity:
 *   P24692 05/08/87 Return -1 if 1-to-n collation is used.
 */

#define ONE_TO_N -1

/*
 * NAME: _NLxcolu
 *
 * FUNCTION: Find  collation value and unique collation value for character 
 * beginning at str.
 *
 * RETURN VALUE DESCRIPTION: -1 if 1-to-n collation is used. Otherwise,
 *                           return colluniq value. 
 */
/*  Find collating value and unique collation value or replacement string 
 *  for character beginning at str.
 *  If no replacement string exists, return collation value and
 *  set rstr to NULL.  Else return -1 and set rstr to point to
 *  replacement string.
 */
#ifdef _NO_PROTO
int _NLxcolu(descptr, str, rstr, uniq)
int descptr;
unsigned char **str;
wchar_t **rstr;
wchar_t *uniq;
#else
int
_NLxcolu(int descptr, unsigned char **str, wchar_t **rstr, wchar_t *uniq)
/* descptr - coldesc rel. ptr */
/* **str - input string */
/* **rstr - replacement string */
/* *uniq - unique collation value */
#endif
{
    int rval;
    wchar_t wc;
    int n;
    wchar_t prim_colval;
    wchar_t uniq_colval;

    /*
     * Pretend we're passed a collation handle, just like the OSF/1
     * 1.2 collation functions.
     */
    _LC_collate_t *phdl = __lc_collate;

    /*
     * This is just a formality because everyone in OSF/1 1.0 passes
     * NULL for 'rstr'.  Because of that, we don't need to worry about
     * replacement strings. 
     */
    if (rstr != NULL)
	*rstr = 0;

    if (uniq != NULL)
	*uniq = 0;

    /*
     * 'descptr' should be negative.  That's because we're only called
     * after NCcollate() returns a negative value.  Our caller passes
     * that value to us as 'descptr'.  Making it positive gives us the
     * code-point originally passed to NCcollate().
     */
    rval = descptr;
    if (descptr < 0) {
	wc = -descptr;

	/*
	 * Get the primary and unique collation values.  _getcolval()
	 * returns a byte string for use in string compares, so need to
	 * convert that to a collation weight.
	 */
	n = _getcolval(phdl, &prim_colval, wc, (char *) *str, 0);
	prim_colval = byte_string_to_collation(prim_colval);
	n = _getcolval(phdl, &uniq_colval, wc, (char *) *str, MAX_NORDS);
	uniq_colval = byte_string_to_collation(uniq_colval);

	/*
	 * Skip over the collation element.
	 */
	*str += n;

	/*
	 * An out-of-range collation value means the character should
	 * be ignored in collating.
	 */
	if ((prim_colval < MIN_UCOLL) || (prim_colval > MAX_UCOLL) ||
	    (uniq_colval < MIN_UCOLL) || (uniq_colval > MAX_UCOLL))
	    return 0;

	/*
	 * Our OSF/1 1.0 callers expect 16-bit collation values, but
	 * the values from the 1.2 locales use all 32-bits.  It's safe
	 * to truncate the 1.2 values because their high 16-bits are
	 * always 0x0101 for single-byte locales.
	 */
	rval = (unsigned short) prim_colval;
	if (uniq != NULL)
	    *uniq = (unsigned short) uniq_colval;
    }

    return rval;
}
