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
/*
* $XConsortium: VendorP.h,v 1.20 89/10/04 12:22:55 swick Exp $
* $oHeader: VendorP.h,v 1.2 88/08/18 15:56:48 asente Exp $
*/

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* 
 * VendorP.h - Private definitions for VendorShell widget
 * 
 * Author:	Paul Asente
 * 		Digital Equipment Corporation
 * 		Western Software Laboratory
 * Date:	Thu Dec 3, 1987
 */

/***********************************************************************
 *
 * VendorShell Widget Private Data
 *
 ***********************************************************************/

#ifndef  _XtVendorPrivate_h
#define _XtVendorPrivate_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <X11/Vendor.h>

/* New fields for the VendorShell widget class record */

typedef struct {
    XtPointer       extension;          /* pointer to extension record      */
} VendorShellClassPart;

typedef struct _VendorShellClassRec {
  	CoreClassPart      core_class;
	CompositeClassPart composite_class;
	ShellClassPart  shell_class;
	WMShellClassPart   wm_shell_class;
	VendorShellClassPart vendor_shell_class;
} VendorShellClassRec;

#ifdef DEC_BUG_FIX
#ifdef __cplusplus
extern "C" {
#endif
#endif

externalref VendorShellClassRec vendorShellClassRec;

#ifdef DEC_BUG_FIX
#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration */
#endif
#endif

/* New fields for the vendor shell widget. */

typedef struct {
	int		vendor_specific;
} VendorShellPart;

typedef  struct {
	CorePart 	core;
	CompositePart 	composite;
	ShellPart 	shell;
	WMShellPart	wm;
	VendorShellPart	vendor;
} VendorShellRec, *VendorShellWidget;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif  /* _XtVendorPrivate_h */
