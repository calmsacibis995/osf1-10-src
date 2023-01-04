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
/* $XConsortium: XcmsCvColW.c,v 1.4 91/05/13 23:19:33 rws Exp $" */

/*
 * Code and supporting documentation (c) Copyright 1990 1991 Tektronix, Inc.
 * 	All Rights Reserved
 * 
 * This file is a component of an X Window System-specific implementation
 * of Xcms based on the TekColor Color Management System.  Permission is
 * hereby granted to use, copy, modify, sell, and otherwise distribute this
 * software and its documentation for any purpose and without fee, provided
 * that this copyright, permission, and disclaimer notice is reproduced in
 * all copies of this software and in supporting documentation.  TekColor
 * is a trademark of Tektronix, Inc.
 * 
 * Tektronix makes no representation about the suitability of this software
 * for any purpose.  It is provided "as is" and with all faults.
 * 
 * TEKTRONIX DISCLAIMS ALL WARRANTIES APPLICABLE TO THIS SOFTWARE,
 * INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL TEKTRONIX BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA, OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE, OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR THE PERFORMANCE OF THIS SOFTWARE.
 *
 *
 *
 *	NAME
 *		XcmsCvColW.c
 *
 *	DESCRIPTION
 *		<overall description of what the package does>
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"


/************************************************************************
 *									*
 *			 API PRIVATE ROUTINES				*
 *									*
 ************************************************************************/


/*
 *	NAME
 *		_XcmsConvertColorsWithWhitePt - Convert XcmsColor structures
 *
 *	SYNOPSIS
 */
Status
_XcmsConvertColorsWithWhitePt(ccc, pColors_in_out, pWhitePt, nColors,
	newFormat, pCompressed)
    XcmsCCC ccc;
    XcmsColor *pColors_in_out;
    XcmsColor *pWhitePt;
    unsigned int nColors;
    XcmsColorFormat newFormat;
    Bool *pCompressed;
/*
 *	DESCRIPTION
 *		Convert XcmsColor structures between device-independent
 *		and/or device-dependent formats but allowing the calling
 *		routine to specify the white point to be associated
 *		with the color specifications (overriding
 *		ccc->clientWhitePt).
 *
 *		This routine has been provided for use in white point
 *		adjustment routines.
 *
 *	RETURNS
 *		XcmsFailure if failed,
 *		XcmsSuccess if succeeded without gamut compression,
 *		XcmsSuccessWithCompression if succeeded with gamut
 *			compression.
 *
 */
{
    if (ccc == NULL || pColors_in_out == NULL ||
	    pColors_in_out->format == XcmsUndefinedFormat) {
	return(XcmsFailure);
    }

    if (nColors == 0 || pColors_in_out->format == newFormat) {
	/* do nothing */
	return(XcmsSuccess);
    }

    if (XCMS_DI_ID(pColors_in_out->format) && XCMS_DI_ID(newFormat)) {
	/*
	 * Device-Independent to Device-Independent Conversion
	 */
	return(_XcmsDIConvertColors(ccc, pColors_in_out, pWhitePt, nColors,
		newFormat));
    }
    if (XCMS_DD_ID(pColors_in_out->format) && XCMS_DD_ID(newFormat)) {
	/*
	 * Device-Dependent to Device-Dependent Conversion
	 */
	return(_XcmsDDConvertColors(ccc, pColors_in_out, nColors, newFormat,
		pCompressed));
    }

    /*
     * Otherwise we have:
     *    1. Device-Independent to Device-Dependent Conversion
     *		OR
     *    2. Device-Dependent to Device-Independent Conversion
     */

    if (XCMS_DI_ID(pColors_in_out->format)) {
	/*
	 *    1. Device-Independent to Device-Dependent Conversion
	 */
	/* First convert to CIEXYZ */
	if (_XcmsDIConvertColors(ccc, pColors_in_out, pWhitePt, nColors,
		XcmsCIEXYZFormat) == XcmsFailure) {
	    return(XcmsFailure);
	}
	/* Then convert to DD Format */
	return(_XcmsDDConvertColors(ccc, pColors_in_out, nColors, newFormat,
		pCompressed));
    } else {
	/*
	 *    2. Device-Dependent to Device-Independent Conversion
	 */
	/* First convert to CIEXYZ */
	if (_XcmsDDConvertColors(ccc, pColors_in_out, nColors,
		XcmsCIEXYZFormat, pCompressed) == XcmsFailure) {
	    return(XcmsFailure);
	}
	/* Then convert to DI Format */
	return(_XcmsDIConvertColors(ccc, pColors_in_out, pWhitePt, nColors,
		newFormat));
    }
}
