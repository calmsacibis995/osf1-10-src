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
/* $XConsortium: XcmsIdOfPr.c,v 1.6 91/11/06 16:45:23 rws Exp $" */

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
 *	NAME
 *		XcmsIdOfPr.c
 *
 *	DESCRIPTION
 *		Source for XcmsFormatOfPrefix()
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"

/*
 *      EXTERNS
 */
extern XcmsColorSpace **_XcmsDIColorSpaces;
extern XcmsColorSpace **_XcmsDDColorSpaces;
void _XcmsCopyISOLatin1Lowered();


/*
 *	NAME
 *		XcmsFormatOfPrefix
 *
 *	SYNOPSIS
 */
XcmsColorFormat
XcmsFormatOfPrefix(prefix)
    char *prefix;
/*
 *	DESCRIPTION
 *		Returns the Color Space ID for the specified prefix
 *		if the color space is found in the Color Conversion
 *		Context.
 *
 *	RETURNS
 *		Color Space ID if found; zero otherwise.
 */
{
    XcmsColorSpace	**papColorSpaces;
    char		string_buf[64];
    char		*string_lowered;
    int			len;

    /*
     * While copying prefix to string_lowered, convert to lowercase
     */
    if ((len = strlen(prefix)) >= sizeof(string_buf)) {
	string_lowered = (char *) Xmalloc(len+1);
    } else {
	string_lowered = string_buf;
    }
    _XcmsCopyISOLatin1Lowered(string_lowered, prefix);

    /*
     * First try Device-Independent color spaces
     */
    papColorSpaces = _XcmsDIColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strcmp((*papColorSpaces)->prefix, string_lowered) == 0) {
		if (len >= sizeof(string_buf)) Xfree(string_lowered);
		return((*papColorSpaces)->id);
	    }
	    papColorSpaces++;
	}
    }

    /*
     * Next try Device-Dependent color spaces
     */
    papColorSpaces = _XcmsDDColorSpaces;
    if (papColorSpaces != NULL) {
	while (*papColorSpaces != NULL) {
	    if (strcmp((*papColorSpaces)->prefix, string_lowered) == 0) {
		if (len >= sizeof(string_buf)) Xfree(string_lowered);
		return((*papColorSpaces)->id);
	    }
	    papColorSpaces++;
	}
    }

    if (len >= sizeof(string_buf)) Xfree(string_lowered);
    return(XcmsUndefinedFormat);
}
