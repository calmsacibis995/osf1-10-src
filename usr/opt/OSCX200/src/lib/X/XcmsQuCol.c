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
/* $XConsortium: XcmsQuCol.c,v 1.4 91/05/13 23:27:56 rws Exp $" */

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
 *		XcmsQuCol.c
 *
 *	DESCRIPTION
 *		Source for XcmsQueryColors
 *
 *
 */

#include "Xlibint.h"
#include "Xcmsint.h"



/************************************************************************
 *									*
 *			PUBLIC ROUTINES					*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsQueryColor - Query Color
 *
 *	SYNOPSIS
 */
Status
XcmsQueryColor(dpy, colormap, pXcmsColor_in_out, result_format)
    Display *dpy;
    Colormap colormap;
    XcmsColor *pXcmsColor_in_out;
    XcmsColorFormat result_format;
/*
 *	DESCRIPTION
 *		This routine uses XQueryColor to obtain the X RGB values
 *		stored in the specified colormap for the specified pixel.
 *		The X RGB values are then converted to the target format as
 *		specified by the format component of the XcmsColor structure.
 *
 *	RETURNS
 *		XcmsFailure if failed;
 *		XcmsSuccess if it succeeded.
 *
 *		Returns a color specification of the color stored in the
 *		specified pixel.
 */
{
    return(_XcmsSetGetColors (XQueryColor, dpy, colormap,
	    pXcmsColor_in_out, 1, result_format, (Bool *) NULL));
}
