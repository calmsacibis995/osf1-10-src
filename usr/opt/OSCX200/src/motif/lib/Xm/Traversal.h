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
/* BuildSystemHeader added automatically */
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/Traversal.h,v 1.1.2.2 92/04/01 15:20:36 Russ_Kuhn Exp $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)Traversal.h	3.10 91/01/10";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 199 0OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
#ifndef _XmTraversal_h
#define _XmTraversal_h

#ifdef VMS
#include <DECW$INCLUDE:ShellP.h>
#else
#include <X11/ShellP.h>
#endif

/* New fields for the vendor shell widget. */

typedef struct _XmFocusMovedCallbackStruct{
    int 	reason;
    XEvent  	*event;
    Boolean 	cont;
    Widget	old, new;
    unsigned char focus_policy;
} XmFocusMovedCallbackStruct, *XmFocusMovedCallback;

typedef struct _XmFocusDataRec *XmFocusData;

#ifdef _NO_PROTO
extern XmFocusData _XmCreateFocusData ();
extern void _XmDestroyFocusData ();
extern void _XmSetActiveTabGroup ();
extern Widget _XmGetActiveItem ();
extern void _XmNavigInitialize ();
extern Boolean _XmChangeNavigationType ();
extern Boolean _XmNavigSetValues ();
extern void _XmNavigDestroy ();
extern Boolean _XmCallFocusMoved ();
extern void _XmPrimitiveEnter ();
extern void _XmPrimitiveLeave ();
extern void _XmPrimitiveUnmap ();
extern void _XmPrimitiveFocusInInternal ();
extern void _XmPrimitiveFocusOut ();
extern void _XmPrimitiveFocusIn ();
extern void _XmManagerEnter ();
extern void _XmManagerFocusInInternal ();
extern void _XmManagerFocusIn ();
extern void _XmManagerFocusOut ();
extern void _XmManagerUnmap ();
extern Boolean _XmMgrTraversal ();
extern void _XmClearKbdFocus ();
extern void _XmClearFocusPath ();
extern Boolean _XmFindTraversablePrim ();
extern Boolean _XmTestTraversability ();
extern Boolean _XmFocusIsHere ();
extern void _XmProcessTraversal ();
extern Boolean XmProcessTraversal ();
extern Widget _XmFindNextTabGroup ();
extern Widget _XmFindPrevTabGroup ();
extern unsigned char _XmGetFocusPolicy ();
extern void _XmClearTabGroup ();
extern Widget _XmFindTabGroup ();
extern Widget _XmGetTabGroup ();
extern Widget _XmFindTopMostShell ();
extern void XmAddTabGroup ();
extern void XmRemoveTabGroup ();
extern void _XmFocusModelChanged ();
extern Boolean _XmGrabTheFocus ();
extern XmFocusData _XmGetFocusData ();
extern Boolean _XmGetManagedInfo ();
extern Boolean _XmCreateVisibilityRect ();
extern int _XmSetRect ();
extern int _XmIntersectRect ();
extern int _XmEmptyRect ();
extern int _XmClearRect ();
#else /* _NO_PROTO */
extern XmFocusData _XmCreateFocusData (void);
extern void _XmDestroyFocusData (XmFocusData focusData);
extern void _XmSetActiveTabGroup (XmFocusData focusData, Widget tabGroup);
extern Widget _XmGetActiveItem (Widget w);
extern void _XmNavigInitialize (Widget request, Widget new, ArgList args, Cardinal *num_args);
extern Boolean _XmChangeNavigationType (Widget current, XmNavigationType newNavType);
extern Boolean _XmNavigSetValues (Widget current, Widget request, Widget new, ArgList args, Cardinal *num_args);
extern void _XmNavigDestroy (Widget w);
extern Boolean _XmCallFocusMoved (Widget old, Widget new, XEvent *event);
extern void _XmPrimitiveEnter (XmPrimitiveWidget pw, XEvent *event);
extern void _XmPrimitiveLeave (XmPrimitiveWidget pw, XEvent *event);
extern void _XmPrimitiveUnmap (XmPrimitiveWidget pw, XEvent *event);
extern void _XmPrimitiveFocusInInternal (XmPrimitiveWidget pw, XEvent *event);
extern void _XmPrimitiveFocusOut (XmPrimitiveWidget pw, XEvent *event);
extern void _XmPrimitiveFocusIn (XmPrimitiveWidget pw, XEvent *event);
extern void _XmManagerEnter (XmManagerWidget mw, XCrossingEvent *event);
extern void _XmManagerFocusInInternal (XmManagerWidget mw, XEvent *event);
extern void _XmManagerFocusIn (XmManagerWidget mw, XEvent *event);
extern void _XmManagerFocusOut (XmManagerWidget mw, XEvent *event);
extern void _XmManagerUnmap (XmManagerWidget mw, XEvent *event);
extern Boolean _XmMgrTraversal (Widget w, int direction);
extern void _XmClearKbdFocus (Widget tabGroup);
extern void _XmClearFocusPath (Widget w);
extern Boolean _XmFindTraversablePrim (CompositeWidget tabGroup);
extern Boolean _XmTestTraversability (Widget widget, XRectangle *visRect);
extern Boolean _XmFocusIsHere (Widget w);
extern void _XmProcessTraversal (Widget w, int dir, Boolean check);
extern Boolean XmProcessTraversal (Widget w, int dir);
extern Widget _XmFindNextTabGroup (Widget w);
extern Widget _XmFindPrevTabGroup (Widget w);
extern unsigned char _XmGetFocusPolicy (Widget w);
extern void _XmClearTabGroup (Widget w);
extern Widget _XmFindTabGroup (Widget w);
extern Widget _XmGetTabGroup (Widget w);
extern Widget _XmFindTopMostShell (Widget w);
extern void XmAddTabGroup (Widget tabGroup);
extern void XmRemoveTabGroup (Widget w);
extern void _XmFocusModelChanged (ShellWidget topmost_shell, caddr_t client_data, caddr_t call_data);
extern Boolean _XmGrabTheFocus (Widget w, XEvent *event);
extern XmFocusData _XmGetFocusData (ShellWidget topmost_shell);
extern Boolean _XmGetManagedInfo (Widget w);
extern Boolean _XmCreateVisibilityRect (Widget w, XRectangle *rectPtr);
extern int _XmSetRect (XRectangle *rect, Widget w);
extern int _XmIntersectRect (XRectangle *srcRectA, Widget widget, XRectangle *dstRect);
extern int _XmEmptyRect (XRectangle *r);
extern int _XmClearRect (XRectangle *r);
#endif /* _NO_PROTO */

#endif /* _XmTraversal_h */
