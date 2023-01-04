#ifndef _dwc_ui_layoutwidget_h_
#define _dwc_ui_layoutwidget_h_
/* $Id$ */
/* #module dwc_ui_layout.h */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows general purpose
**
**  AUTHOR:
**
**	Marios Cleovoulou, January-1988
**
**  ABSTRACT:
**
**	This include file contains the structures, private and public, of layout
**	widget.
**
**--
*/

#ifdef vaxc
#pragma nostandard
#endif
#include <X11/Intrinsic.h>	    /* for Widget */
#include <Xm/XmP.h>
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/ManagerP.h>
#endif
#ifdef vaxc
#pragma standard
#endif

#include "dwc_compat.h"

#define	LayoutIndex (XmManagerIndex + 1)
/*
**  The layout widget is a subclass of manager.  Define the part we add.
*/

typedef struct {
    XtCallbackList	resize_callback;
    XtCallbackList	geometry_callback;
    XtCallbackList	focus_callback;
    dwcaddr_t		tag;
    Boolean		default_positioning;

    Boolean		got_focus;
    Dimension		margin_width, margin_height;
    unsigned char	shadow_type;
} LayoutPart;

typedef struct {
    CorePart		core ;
    CompositePart	composite ;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    LayoutPart		layout ;
} LayoutWidgetRec, *LayoutWidget ;

typedef struct {
    XmOffsetPtr		layoutoffsets;
    int			mumble ;
} LayoutClassPart ;

typedef struct {
    CoreClassPart	core_class ;
    CompositeClassPart	composite_class ;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    LayoutClassPart	layout_class ;
} LayoutClassRec, *LayoutClass ;

extern LayoutClassRec    layoutwidgetclassrec ;
extern LayoutClass	    layoutwidgetclass ;


#define LwNdefaultPositioning	"defaultPositioning"
#define LwNresizeCallback	"resizeCallback"
#define LwNgeometryCallback	"geometryCallback"
#define LwNfocusCallback	"focusCallback"
#define LwNtag			"tag"

#define LwCTag			"Tag"


#define	LwCRResize	    1
#define	LwCRGeometry	    2
#define	LwCRFocus	    3


typedef struct {
    int			reason ;
    XEvent		*event ;
    Time		time ;
    Dimension		width ;
    Dimension		height ;
    Cardinal		border_width ;
    XtGeometryResult	result ;
    XtWidgetGeometry	*request ;
    XtWidgetGeometry	*reply ;
} LwCallbackStruct ;

void
LwGotFocus PROTOTYPE ((
	Widget	w));

LayoutWidget
LwLayoutCreate PROTOTYPE ((
	Widget	parent,
	char	*name,
	Arg	*arglist,
	int	argcount));

LayoutWidget
LwLayout PROTOTYPE ((
	Widget		parent,
	char		*name,
	Boolean		default_pos,
	Position	x,
	Position	y,
	Dimension	width,
	Dimension	height,
	XtCallbackList	resize_callback,
	XtCallbackList	geometry_callback,
	XtCallbackList	focus_callback,
	XtCallbackList	help_callback,
	dwcaddr_t		tag));

#define	LwOffsetPtr(w) \
(((LayoutClass) ((LayoutWidget) w)->core.widget_class)->layout_class.layoutoffsets)

#define LwTopShadowGC(lw,o) XmField(lw,o,XmManager,top_shadow_GC,GC)
#define LwBottomShadowGC(lw,o) XmField(lw,o,XmManager,bottom_shadow_GC,GC)
#define LwBackgroundGC(lw,o) XmField(lw,o,XmManager,background_GC,GC)
#define LwHighlightGC(lw,o) XmField(lw,o,XmManager,highlight_GC,GC)

#define LwTopShadow(lw,o) XmField(lw,o,XmManager,top_shadow_color,Pixel)
#define LwBottomShadow(lw,o) XmField(lw,o,XmManager,bottom_shadow_color,Pixel)
#define LwBackground(lw,o) XmField(lw,o,XmManager,background,Pixel)
#define LwHighlight(lw,o) XmField(lw,o,XmManager,highlight_color,Pixel)

#define LwTopShadowPixmap(lw,o) XmField(lw,o,XmManager,top_shadow_pixmap,Pixmap)
#define LwBottomShadowPixmap(lw,o) XmField(lw,o,XmManager,bottom_shadow_pixmap,Pixmap)
#define LwBackgroundPixmap(lw,o) XmField(lw,o,XmManager,background_pixmap,Pixmap)
#define LwHighlightPixmap(lw,o) XmField(lw,o,XmManager,highlight_color_pixmap,Pixmap)

#define LwShadowThickness(lw,o) XmField(lw,o,XmManager,shadow_thickness,Dimension)
#define LwHighlightThickness(lw,o) XmField(lw,o,XmManager,highlight_thickness,Dimension)

#define LwShadowType(lw,o) XmField(lw,o,Layout,shadow_type,unsigned char)

#define LwResizeCallback(lw,o) XmField(lw,o,Layout,resize_callback,XtCallbackList)
#define LwGeometryCallback(lw,o) XmField(lw,o,Layout,geometry_callback,XtCallbackList)
#define LwFocusCallback(lw,o) XmField(lw,o,Layout,focus_callback,XtCallbackList)
#define LwHelpCallback(lw,o) XmField(lw,o,XmManager,help_callback,XtCallbackList)

#endif /* _dwc_ui_layoutwidget_h_ */
