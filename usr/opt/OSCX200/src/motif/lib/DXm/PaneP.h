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
 * @(#)$RCSfile: PaneP.h,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1993/08/03 00:03:38 $
 */ 
/*
 * Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/* 
 * PanePrivate.h - Private definitions for Pane widget
 */
 
#ifndef _PanePrivate_h
#define _PanePrivate_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif
#include <Xm/PrimitiveP.h> 
#include <Xm/ManagerP.h> 

/*
 *  Redefine XtIsRealized since windows are integer and cannot be standard
 *  compared to NULL.
 */
#undef  XtIsRealized
#define XtIsRealized(widget)	((widget)->core.window != 0)

/*
 * New fields for the Pane widget class record 
 */
typedef struct {
    XmOffsetPtr	paneoffsets;
    int mumble;   /* No new procedures */
} PaneClassPart;
 
/* 
 * Full class record declaration
 */
typedef struct _PaneClassRec {
    CoreClassPart	core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    PaneClassPart	pane_class;
} PaneClassRec, *PaneClass;
 
 
/*
 * New fields for the Pane widget record
 */
typedef struct _PanePart {
    XtCallbackList    helpcallback,	/* Help callback */
                      map_callback,     /* about to be mapped */
		      unmap_callback,   /* just been unmapped */                            
		      focus_callback;	/* grabbed input focus */
    unsigned char orientation;		/* Horizontal or Vertical pane. */
    Pixel foreground;		/* Foreground color for mullions, borders, etc. */
    Dimension mullionsize;	/* Size of mullions. same as borderwidth */
    Dimension mullionlength;	/* Actual Size of mullions. */
    Boolean leftmullion;	/* Have we actually left the mullion area */
    Dimension intersub;		/* Number of pixels between subwidgets. */
    WidgetList children;	/* List of subwidgets. */
    Cardinal num_children;	/* Number of subwidgets. */
    int resize_mode;		/* If TRUE, children are allowed to change */
				/* their length to whatever they want, and */
				/* we don't attempt to make things add up to */
				/* the correct total length. */
    GC invgc;			/* GC to use to draw magic borders */
    Widget whichadjust;		/* Which window we're currently dragging. */
    Dimension origloc;		/* Where the button was originally pressed. */
    Boolean overridetext;	/* Whether to override text bindings. */
    Widget lasthadfocus;	/* The last widget to which we gave focus. */
} PanePart;
 
/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/
 
typedef struct _PaneRec {
    CorePart	    core;
    CompositePart   composite;
    ConstraintPart  constraint;
    XmManagerPart   manager;
    PanePart	    pane;
} PaneRec;

#include "Pane.h"

/* 
 * Data to be kept for each child.
 */
typedef struct _ConstraintsRec {
    Dimension min, max;		/* Limits on length of this widget. */
    Dimension dlength;		/* Desired length for this widget. */
    Widget mullion;		/* Widget used for the mullion below this */
				/* widget (if any) */
    Dimension magicborder;	/* Where the last magic border was drawn. */
    Position position;		/* Where to place this beast */
    Cardinal sharedflag;	/* The position is shared with another widget */
				/* Also is the subwidget viewable */
    Boolean resizable;		/* allow widget to be resized by pane? */

} ConstraintsRec, *Constraints;
 
#ifndef PANE
/*#pragma nostandard*/
externalref PaneClassRec panewidgetclassrec;
/*#pragma standard*/
#endif

/***********************************************************************
 *
 * Pane Widget Private Data
 *
 ***********************************************************************/
 
typedef struct _MullionPart {
    XtCallbackList helpcallback;
} MullionPart;
 
typedef struct _MullionRec {
    CorePart	core;
    XmPrimitivePart primitive;
    MullionPart	mullion;
} MullionRec, *MullionWidget;

typedef struct _MullionClassPart
{
        XtPointer		extension; /* Pointer to extension record */
} MullionClassPart;

typedef struct _MullionClassRec
{
    CoreClassPart        core_class;
    XmPrimitiveClassPart primitive_class;
    MullionClassPart	 mullion_class;
} MullionClassRec;

externalref MullionClassRec mullionclassrec;

/* Macros to access pane fields */

#define PaneIndex (XmManagerIndex + 1)

#define PaneField(w,class,field,type)				\
	XmField((w),						\
		 ((PaneClass)(w)->core.widget_class)->		\
		 	pane_class.paneoffsets,class,		\
		 field,						\
		 type)
 
#define X(w)		PaneField(w,Core,x,Position)
#define Y(w)		PaneField(w,Core,y,Position)
 
#define Width(w)		PaneField(w,Core,width,Dimension)
#define Height(w)		PaneField(w,Core,height,Dimension)
#define BackgroundPixel(w)	PaneField(w,Core,background_pixel,Pixel)
 
#define Children(w)		PaneField(w,Composite,children,WidgetList)
#define NumChildren(w)		PaneField(w,Composite,num_children,Cardinal)
 
#define MullionSize(w)		PaneField(w,Pane,mullionsize,Dimension)
#define MullionLength(w)	PaneField(w,Pane,mullionlength,Dimension)
#define LeftMullion(w)		PaneField(w,Pane,leftmullion,Boolean)
#define Orientation(w)		PaneField(w,Pane,orientation,unsigned char)
#define PaneNumChildren(w)	PaneField(w,Pane,num_children,Cardinal)
#define PaneChildren(w)		PaneField(w,Pane,children,WidgetList)
#define InterSub(w)		PaneField(w,Pane,intersub,Dimension)
#define ResizeMode(w)		PaneField(w,Pane,resize_mode,unsigned char)
#define Foreground(w)		PaneField(w,Pane,foreground,Pixel)
#define InvGC(w)		PaneField(w,Pane,invgc,GC)
#define WhichAdjust(w)		PaneField(w,Pane,whichadjust,Widget)
#define OrigLoc(w)		PaneField(w,Pane,origloc,Dimension)
#define OverrideText(w)		PaneField(w,Pane,overridetext,Boolean)
#define LastHadFocus(w)		PaneField(w,Pane,lasthadfocus,Widget)
#define HelpCallback(w)		PaneField(w,Pane,helpcallback,XtCallbackList)


 *  */
 
 
#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _PanePrivate_h */
/* DON'T ADD STUFF AFTER THIS #endif */
