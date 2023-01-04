#if !defined(_iconboxwidgetp_h_)
#define _iconboxwidgetp_h_
/* $Id$ */
/* #module dwc_ui_iconboxwidgetp.h */
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
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, December-1988
**
**  ABSTRACT:
**
**	This include file contains the private structures etc, for the iconbox
**	widget. 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3-002  Paul Ferwerda					09-Feb-1990
**		Added include of CoreP.h since we need it for CorePart. Changed
**		DwtOffsetPtr to XmOffsetPtr.
**	V1-001  Marios Cleovoulou				Dec-1988
**		Initial version.
**--
**/

#include    "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>
#include <Xm/XmP.h>
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/ManagerP.h>
#endif
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_iconboxwidget.h"

#define	IconboxIndex    (XmManagerIndex + 1)

/*
**  Instance Part Record
*/

typedef struct
{
    Dimension		margin_width;
    Dimension		margin_height;
    Dimension		spacing;
    Cardinal		number_of_icons;
    Pixmap		*off_icons;
    unsigned char	*selected_icons;
    unsigned char	selected_icon;
    Cardinal		number_of_selected;
    Cardinal		columns;
    Cardinal		rows;
    DwcIbwIconboxStyle	iconbox_style;
    Boolean		editable;
    XtCallbackList	value_changed_callback;
    XtCallbackList	help_callback;
    Dimension		pixmap_width;
    Dimension		pixmap_height;

    GC			icon_gc;
    GC			icon_on_gc;
    GC			iconinvert_gc;
    Boolean		invert_up;
    Position		invert_x;
    Position		invert_y;
    Dimension		invert_width;
    Dimension		invert_height;
} IconboxPart;

/*
**  Instance record
*/

typedef struct _IconboxRec
{
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    IconboxPart		iconbox;
} IconboxWidgetRec, *IconboxWidget;

/*
**  Class part record
*/

typedef struct _IconboxClassPart
{
    XmOffsetPtr	iconboxoffsets;
    int		reserved;
} IconboxClassPart;

/*
**  Class record
*/

typedef struct _IconboxClassRec
{
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    IconboxClassPart	iconbox_class;
} IconboxClassRec, *IconboxWidgetClass;

extern IconboxClassRec iconboxClassRec;

#define	IbwOffsetPtr(w) \
(((IconboxWidgetClass) ((IconboxWidget) w)->core.widget_class)->iconbox_class.iconboxoffsets)

#define	IbwBackgroundPixel(w, o)    XmField (w,o,Core,background_pixel,Pixel)
#define	IbwForegroundPixel(w, o)    XmField (w,o,XmManager,foreground,Pixel)
#define	IbwMarginWidth(w, o)        XmField (w,o,Iconbox,margin_width,Dimension)
#define	IbwMarginHeight(w, o)       XmField (w,o,Iconbox,margin_height,Dimension)
#define	IbwSpacing(w, o)	    XmField (w,o,Iconbox,spacing,Dimension)
#define	IbwNumberOfIcons(w, o)      XmField (w,o,Iconbox,number_of_icons,Cardinal)
#define	IbwOffIcons(w, o)	    XmField (w,o,Iconbox,off_icons,Pixmap *)
#define	IbwSelectedIcons(w, o)	    XmField (w,o,Iconbox,selected_icons,unsigned char *)
#define	IbwSelectedIcon(w, o)	    XmField (w,o,Iconbox,selected_icon,unsigned char)
#define	IbwNumberOfSelected(w, o)   XmField (w,o,Iconbox,number_of_selected,Cardinal)
#define	IbwColumns(w, o)	    XmField (w,o,Iconbox,columns,Cardinal)
#define	IbwRows(w, o)		    XmField (w,o,Iconbox,rows,Cardinal)
#define	IbwIconboxStyle(w, o)	    XmField (w,o,Iconbox,iconbox_style,DwcIbwIconboxStyle)
#define	IbwEditable(w, o)	    XmField (w,o,Iconbox,editable,Boolean)
#define	IbwHelpCallback(w, o)	    XmField (w,o,Iconbox,help_callback,XtCallbackList)
#define	IbwValueChangedCallback(w, o) XmField (w,o,Iconbox,value_changed_callback,XtCallbackList)

#define	IbwPixmapWidth(w, o)        XmField (w,o,Iconbox,pixmap_width,Dimension)
#define	IbwPixmapHeight(w, o)       XmField (w,o,Iconbox,pixmap_height,Dimension)
#define	IbwIconOffGC(w, o)	    XmField (w,o,Iconbox,icon_gc,GC)
#define IbwIconOnGC(w, o)	    XmField (w,o,Iconbox,icon_on_gc,GC)
#define	IbwIconInvertGC(w, o)	    XmField (w,o,Iconbox,iconinvert_gc,GC)
#define	IbwInvertUp(w, o)	    XmField (w,o,Iconbox,invert_up,Boolean)
#define	IbwInvertX(w, o)	    XmField (w,o,Iconbox,invert_x,Position)
#define	IbwInvertY(w, o)	    XmField (w,o,Iconbox,invert_y,Position)
#define	IbwInvertWidth(w, o)	    XmField (w,o,Iconbox,invert_width,Dimension)
#define	IbwInvertHeight(w, o)	    XmField (w,o,Iconbox,invert_height,Dimension)

#endif /* _iconboxwidgetp_h_ */
