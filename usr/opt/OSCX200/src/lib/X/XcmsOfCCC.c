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
/* $XConsortium: XcmsOfCCC.c,v 1.1 91/05/13 22:37:17 rws Exp $ */

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
 *		XcmsOfCCC.c - Color Conversion Context Querying Routines
 *
 *	DESCRIPTION
 *		Routines to query components of a Color Conversion
 *		Context structure.
 *
 *
 */

#include "Xlib.h"
#include "Xcms.h"



/************************************************************************
 *									*
 *			PUBLIC INTERFACES				*
 *									*
 ************************************************************************/

/*
 *	NAME
 *		XcmsDisplayOfCCC
 *
 *	SYNOPSIS
 */

Display *
XcmsDisplayOfCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Queries the Display of the specified CCC.
 *
 *	RETURNS
 *		Pointer to the Display.
 *
 */
{
    return(ccc->dpy);
}


/*
 *	NAME
 *		XcmsVisualOfCCC
 *
 *	SYNOPSIS
 */

Visual *
XcmsVisualOfCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Queries the Visual of the specified CCC.
 *
 *	RETURNS
 *		Pointer to the Visual.
 *
 */
{
    return(ccc->visual);
}


/*
 *	NAME
 *		XcmsScreenNumberOfCCC
 *
 *	SYNOPSIS
 */

int
XcmsScreenNumberOfCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Queries the screen number of the specified CCC.
 *
 *	RETURNS
 *		screen number.
 *
 */
{
    return(ccc->screenNumber);
}


/*
 *	NAME
 *		XcmsScreenWhitePointOfCCC
 *
 *	SYNOPSIS
 */

XcmsColor *
XcmsScreenWhitePointOfCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Queries the screen white point of the specified CCC.
 *
 *	RETURNS
 *		Pointer to the XcmsColor containing the screen white point.
 *
 */
{
    return(&ccc->pPerScrnInfo->screenWhitePt);
}


/*
 *	NAME
 *		XcmsClientWhitePointOfCCC
 *
 *	SYNOPSIS
 */

XcmsColor *
XcmsClientWhitePointOfCCC(ccc)
    XcmsCCC ccc;
/*
 *	DESCRIPTION
 *		Queries the client white point of the specified CCC.
 *
 *	RETURNS
 *		Pointer to the XcmsColor containing the client white point.
 *
 */
{
    return(&ccc->clientWhitePt);
}
