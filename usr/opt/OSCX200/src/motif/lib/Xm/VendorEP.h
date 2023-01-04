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
/* $Header: /alphabits/u3/x11/ode/rcs/x11/src/motif/lib/dec/Xm/VendorEP.h,v 1.1.2.2 92/04/01 15:22:02 Russ_Kuhn Exp $ */
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)VendorEP.h	3.4 90/04/24";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990  OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, 1990  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY 
*  (c) Copyright 1988 MICROSOFT CORPORATION
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
*  (c) Copyright 1989, 1990  Open Software Foundation, Inc.  Unpublished - all
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
*  (c) Copyright 1989, 1990  Open Software Foundation, Inc.
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
#ifndef  _VendorEP_h
#define _VendorEP_h

#ifdef VMS
#include <DECW$INCLUDE:MwmUtil.h>
#include <DECW$INCLUDE:ShellP.h>
#include <DECW$INCLUDE:XmP.h>
#include <DECW$INCLUDE:VendorE.h>
#include <DECW$INCLUDE:ExtObjectP.h>
#include <DECW$INCLUDE:Traversal.h>
#else
#include <Xm/MwmUtil.h>
#include <X11/ShellP.h>
#include <Xm/XmP.h>
#include <Xm/VendorE.h>
#include <Xm/ExtObjectP.h>
#include <Xm/Traversal.h>
#endif

/* New fields for the shellExt class record */

#define XmInheritEventHandler		((XtEventHandler)_XtInherit)

/* Class Extension definitions */

/*
 * DESKTOP DEFINITIONS
 */
typedef struct _XmDesktopClassPart{
    WidgetClass		child_class;
    XtWidgetProc	insert_child;	  /* physically add child to parent  */
    XtWidgetProc      	delete_child;	  /* physically remove child	     */
    XtPointer		extension;
}XmDesktopClassPart, *XmDesktopClassPartPtr;

typedef struct _XmDesktopClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
}XmDesktopClassRec;

typedef struct {
    Widget		parent;
    Widget		*children;
    Cardinal		num_children;
    Cardinal		num_slots;
} XmDesktopPart, *XmDesktopPartPtr;

#ifndef VMS
externalref XmDesktopClassRec 	xmDesktopClassRec;
#endif

typedef struct _XmDesktopRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
}XmDesktopRec;

/*
 * SCREEN DEFINITIONS
 */
typedef struct _XmScreenClassPart{
    XtPointer		extension;
}XmScreenClassPart, *XmScreenClassPartPtr;

typedef struct _XmScreenClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmScreenClassPart		screen_class;
}XmScreenClassRec;

typedef struct {
    Boolean		mwmPresent;
    Boolean		shellMapped;
} XmScreenPart, *XmScreenPartPtr;

#ifndef VMS
externalref XmScreenClassRec 	xmScreenClassRec;
#endif

typedef struct _XmScreenRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmScreenPart		screen;
}XmScreenRec;

/*
 * DISPLAY DEFINITIONS
 */
typedef struct _XmDisplayClassPart{
    XtPointer		extension;
}XmDisplayClassPart, *XmDisplayClassPartPtr;

typedef struct _XmDisplayClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmDisplayClassPart		display_class;
}XmDisplayClassRec;

typedef struct _XmModalDataRec{
    XmVendorShellExtObject	ve;
    XmVendorShellExtObject	grabber;
    Boolean			exclusive;
    Boolean			springLoaded;
}XmModalDataRec, *XmModalData;

typedef struct {
    XmModalData			modals;
    Cardinal			numModals;
    Cardinal			maxModals;
} XmDisplayPart, *XmDisplayPartPtr;

#ifndef VMS
externalref XmDisplayClassRec 	xmDisplayClassRec;
#endif

typedef struct _XmDisplayRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmDisplayPart		display;
}XmDisplayRec;

/*
 * WORLD DEFINITIONS
 */
typedef struct _XmWorldClassPart{
    XtPointer		extension;
}XmWorldClassPart, *XmWorldClassPartPtr;

typedef struct _XmWorldClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmWorldClassPart		world_class;
}XmWorldClassRec;

typedef struct {
    int				foo;
} XmWorldPart, *XmWorldPartPtr;

#ifndef VMS
externalref XmWorldClassRec 	xmWorldClassRec;
#endif

typedef struct _XmWorldRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmWorldPart			world;
}XmWorldRec;



/*
 * SHELL EXT DEFINITIONS
 */


typedef struct _XmShellExtClassPart{
    XtEventHandler	structureNotifyHandler;
    XtPointer		extension;
}XmShellExtClassPart, *XmShellExtClassPartPtr;

typedef struct _XmShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart 	shell_class;
}XmShellExtClassRec;

typedef struct {
    unsigned int	lastConfigureRequest;
    Boolean		useAsyncGeometry;
} XmShellExtPart, *XmShellExtPartPtr;

#ifndef VMS
externalref XmShellExtClassRec 	xmShellExtClassRec;
#endif

typedef struct _XmShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
}XmShellExtRec;


/* New fields for the VendorShell widget class record */

#define XmInheritProtocolHandler	((XtCallbackProc)_XtInherit)

/* Class Extension definitions */

typedef struct _XmVendorShellExtClassPart{
    XtCallbackProc	delete_window_handler;
    XtCallbackProc	offset_handler;
    XtPointer		extension;
}XmVendorShellExtClassPart, *XmVendorShellExtClassPartPtr;

typedef struct _XmVendorShellExtClassRec{
    ObjectClassPart		object_class;
    XmExtClassPart		ext_class;
    XmDesktopClassPart 		desktop_class;
    XmShellExtClassPart		shell_class;
    XmVendorShellExtClassPart 	vendor_class;
}XmVendorShellExtClassRec;

typedef struct {
 XmFontList		default_font_list;
 unsigned char		focus_policy;
 XmFocusData		focus_data;
 unsigned char		delete_response;
 unsigned char		unit_type;
 MwmHints		mwm_hints;
 MwmInfo		mwm_info;
 String			mwm_menu;
 XtCallbackList		focus_moved_callback;
 /*
  * internal fields
  */
 Widget			old_managed;
 Position		xAtMap, yAtMap, xOffset, yOffset;
 unsigned int		lastOffsetSerial;
 unsigned int		lastMapRequest;
 Boolean		externalReposition;
#define _XmRAW_MAP 0
#define _XmPOPUP_MAP 1
#define _XmMANAGE_MAP 2
 unsigned char		mapStyle;
 XtCallbackList		realize_callback;
 XtGrabKind		grab_kind;
} XmVendorShellExtPart, *XmVendorShellExtPartPtr;

#ifndef VMS
externalref XmVendorShellExtClassRec 	xmVendorShellExtClassRec;
#endif

typedef struct _XmVendorShellExtRec{
    ObjectPart			object;
    XmExtPart			ext;
    XmDesktopPart		desktop;
    XmShellExtPart		shell;
    XmVendorShellExtPart 	vendor;
}XmVendorShellExtRec;

#ifdef _NO_PROTO
extern void SetMwmStuff ();
extern void _XmVendorExtRealize ();
extern Boolean XmIsMotifWMRunning ();
extern XmWorldObject _XmGetWorldObject();
extern XmDisplayObject _XmGetDisplayObject();
extern XmScreenObject _XmGetScreenObject();
#else /* _NO_PROTO */
extern void SetMwmStuff (XmVendorShellExtObject ove, XmVendorShellExtObject nve);
extern void _XmVendorExtRealize (Widget vw);
extern Boolean XmIsMotifWMRunning (Widget shell);
extern XmWorldObject _XmGetWorldObject(Widget shell, ArgList args,
				       Cardinal * num_args);
extern XmDisplayObject _XmGetDisplayObject(Widget shell, ArgList args,
				       Cardinal * num_args);
extern XmScreenObject _XmGetScreenObject(Widget shell, ArgList args,
				       Cardinal * num_args);
#endif /* _NO_PROTO */

#endif  /* _VendorEP_h */
