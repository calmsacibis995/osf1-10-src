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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: DragCI.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/15 16:19:36 $ */
/*
*  (c) Copyright 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* $Header: /usr/sde/osf1/rcs/x11/src/motif/lib/Xm/DragCI.h,v 1.1.4.3 1993/07/15 16:19:36 Richard_June Exp $ */
#ifndef _XmDragCI_h
#define _XmDragCI_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>
#include <Xm/DragCP.h>
#include <Xm/DragIconP.h>
#include <Xm/DropSMgrP.h>
#include <Xm/DisplayP.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _XmDRAG_MASK_BASE \
(ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#ifdef DRAG_USE_MOTION_HINTS
#define _XmDRAG_GRAB_MASK \
(_XmDRAG_MASK_BASE PointerMotionHintMask)
#else
#define _XmDRAG_GRAB_MASK _XmDRAG_MASK_BASE
#endif /* _XmDRAG_USE_MOTION_HINTS */

#define _XmDRAG_EVENT_MASK(dc) \
  ((((XmDragContext)dc)->drag.trackingMode == XmDRAG_TRACK_WM_QUERY) \
   ? (_XmDRAG_GRAB_MASK | EnterWindowMask | LeaveWindowMask) \
   : (_XmDRAG_GRAB_MASK))

enum{	XmCR_DROP_SITE_TREE_ADD = _XmNUMBER_DND_CB_REASONS,
	XmCR_DROP_SITE_TREE_REMOVE
	} ;
/*
 *  values for dragTrackingMode 
 */
enum { XmDRAG_TRACK_WM_QUERY, XmDRAG_TRACK_MOTION, XmDRAG_TRACK_WM_QUERY_PENDING };


/* Strings to use for the intern atoms */
typedef String	XmCanonicalString;

#define XmMakeCanonicalString( s) \
	  (XmCanonicalString) XrmQuarkToString( XrmStringToQuark( s))

#define _XmAllocAndCopy( data, len) \
		   memcpy( (XtPointer) XtMalloc(len), (XtPointer)(data), (len))


typedef struct _XmDragTopLevelClientDataStruct{
    Widget	destShell;
    Position	xOrigin, yOrigin;
	Dimension	width, height;
    XtPointer	iccInfo;
    Boolean	sourceIsExternal;
	Window	window;
	Widget	dragOver;
}XmDragTopLevelClientDataStruct, *XmDragTopLevelClientData;

typedef struct _XmDragMotionClientDataStruct{
    Window	window;
    Widget	dragOver;
}XmDragMotionClientDataStruct, *XmDragMotionClientData;


#if 0
/* These are not currently in use. */
typedef struct _XmTargetsTableEntryRec{
    Cardinal	numTargets;
    Atom	*targets;
}XmTargetsTableEntryRec, XmTargetsTableEntry;

typedef struct _XmTargetsTableRec{
    Cardinal	numEntries;
    XmTargetsTableEntryRec entries[1]; /* variable size array in-place */
}XmTargetsTableRec, *XmTargetsTable;
#endif

/*
 * dsm to dragDisplay comm
 */
/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeAddCallbackStruct{
    int		    	reason;
    XEvent          	*event;
    Widget		rootShell;
    Cardinal		numDropSites;
    Cardinal		numArgsPerDSHint;
}XmDropSiteTreeAddCallbackStruct, *XmDropSiteTreeAddCallback;

/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeRemoveCallbackStruct{
    int			reason;
    XEvent          	*event;
    Widget		rootShell;
}XmDropSiteTreeRemoveCallbackStruct, *XmDropSiteTreeRemoveCallback;

/* Move to DropSMgrI.h */
typedef struct _XmDropSiteTreeUpdateCallbackStruct{
    int			reason;
    XEvent          	*event;
    Widget		rootShell;
    Cardinal		numDropSites;
    Cardinal		numArgsPerDSHint;
}XmDropSiteTreeUpdateCallbackStruct, *XmDropSiteTreeUpdateCallback;

/* Move to DropSMgrI.h */
typedef struct _XmAnimationData {
    Widget		dragOver;
    Window		window;
    Position		windowX, windowY;
    Screen		*screen;
    XmRegion		clipRegion;
    XmRegion		dropSiteRegion;
    XtPointer		saveAddr;
}XmAnimationDataRec, *XmAnimationData;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmDragCI_h */
