#if !defined(_timeslotwidget_h_)
#define _timeslotwidget_h_
/* $Header$ */
/* #module DWC$UI_TIMESLOTWIDGET.H "V3.0-004"*/
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
**	This include file contains the public structures etc, for the timeslot
**	widget. 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
** V3.0-004 Paul Ferwerda					21-Nov-1990
**		Took out DwcTswNdirectionRtoL since they put it in XmManager.
** V3.0-003 Paul Ferwerda					02-Mar-1990
**		Convert to Motif
** 
**	V1-002  Marios Cleovoulou				12-Jun-1989
**		Provide routine to reset MBA timers etc for widget
**	V1-001  Marios Cleovoulou				Dec-1988
**		Initial version.
**--
**/

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include    <X11/Xlib.h>	    /* for XEvent */
#include    <X11/Intrinsic.h>	    /* for stuff like Boolean */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include    "dwc_compat.h"

#if !defined (_timeslotwidgetp_h_)
typedef WidgetClass TimeslotWidgetClass;
typedef Widget TimeslotWidget;
extern	TimeslotWidgetClass	timeslotWidgetClass;
#else
typedef struct	_TimeslotClassRec   *TimeslotWidgetClass;
typedef struct	_TimeslotRec	    *TimeslotWidget;
#endif

#define DwcTswNforeground	    XmNforeground
#define DwcTswNbackground	    XmNbackground
#define DwcTswNpixmapWidth	    "pixmapWidth"
#define DwcTswNpixmapHeight	    "pixmapHeight"
#define DwcTswNpixmapCount	    "pixmapCount"
#define DwcTswNpixmaps		    "pixmaps"
#define DwcTswNpixmapMargin	    "pixmapMargin"

#define DwcTswNknobSize		    "knobSize"
#define DwcTswNsingleCallback	    "singleCallback"
#define DwcTswNdoubleCallback	    "doubleCallback"
#define DwcTswNdragStartCallback    "dragStartCallback"
#define DwcTswNdragCallback         "dragCallback"
#define DwcTswNdragCancelCallback   "dragCancelCallback"
#define DwcTswNdragEndCallback      "dragEndCallback"
#define DwcTswNresizeStartCallback  "resizeStartCallback"
#define DwcTswNresizeCallback       XmNresizeCallback
#define DwcTswNresizeCancelCallback "resizeCancelCallback"
#define DwcTswNresizeEndCallback    "resizeEndCallback"
#define DwcTswNfocusCallback        XmNfocusCallback
#define DwcTswNlosingFocusCallback    XmNlosingFocusCallback
#define DwcTswNcloseCallback        "closeCallback"
#define DwcTswNextendSelectCallback "extendSelectCallback"
#define DwcTswNincrementSizeCallback "incrementSizeCallback"
#define DwcTswNincrementPositionCallback "incrementPositionCallback"

#define DwcTswNeditable		    XmNeditable
#define DwcTswNdragable		    "dragable"

#define DwcTswNfontList		    XmNfontList

#define DwcTswCPixmapWidth	    "PixmapWidth"
#define DwcTswCPixmapHeight	    "PixmapHeight"
#define DwcTswCPixmapCount	    "PixmapCount"
#define DwcTswCPixmaps		    "Pixmaps"
#define DwcTswCPixmapMargin	    "PixmapMargin"

#define DwcTswCKnobSize		    "KnobSize"
#define DwcTswCSingleCallback	    "SingleCallback"
#define DwcTswCDoubleCallback	    "DoubleCallback"
#define DwcTswCDragStartCallback    "DragStartCallback"
#define DwcTswCDragCallback         "DragCallback"
#define DwcTswCDragCancelCallback   "DragCancelCallback"
#define DwcTswCDragEndCallback      "DragEndCallback"
#define DwcTswCResizeStartCallback  "ResizeStartCallback"
#define DwcTswCResizeCallback       XmCResizeCallback
#define DwcTswCResizeCancelCallback "ResizeCancelCallback"
#define DwcTswCResizeEndCallback    "ResizeEndCallback"
#define DwcTswCFocusCallback        "FocusCallback"
#define DwcTswCCloseCallback        "CloseCallback"
#define DwcTswCExtendSelectCallback "ExtendSelectCallback"
#define DwcTswCIncrementSizeCallback "IncrementSizeCallback"
#define DwcTswCIncrementPositionCallback "IncrementPositionCallback"


#define DwcTswCEditable		    XmCEditable
#define DwcTswCDragable		    "Dragable"

#define DwcTswCFontList		    XmCFontlist

#define DwcTswCRSingle		    -1
#define DwcTswCRDragStart	    -2
#define DwcTswCRDrag		    -3
#define DwcTswCRDragEnd		    -4
#define DwcTswCRResizeStart	    -5
#define DwcTswCRResize		    -6
#define DwcTswCRResizeEnd	    -7
#define DwcTswCRFocus		    -8
#define DwcTswCRLosingFocus	    -9
#define DwcTswCRDragCancel	    -10
#define DwcTswCRResizeCancel	    -11
#define DwcTswCRDouble		    -12
#define DwcTswCRClose		    -13
#define DwcTswCRExtendSelect	    -14


typedef struct {
    int		    reason ;
    XEvent	    *event ;
    Boolean	    selected ;
    Boolean	    opened ;
    Boolean	    editable ;
    Boolean	    top_knob ;
} DwcTswCallbackStruct, *DwcTswCallbackStructPtr ;

void TSWSetSelected PROTOTYPE ((Widget w, Boolean selected));

Boolean TSWGetSelected PROTOTYPE ((Widget w));

void TSWSetOpened PROTOTYPE ((Widget w, Boolean opened, Time time));

void TSWSetEditable PROTOTYPE ((Widget w, Boolean editable, Time time));

Boolean TSWGetEditable PROTOTYPE ((Widget w));

void TSWSetState PROTOTYPE ((
    Widget	w,
    Boolean	selected,
    Boolean	opened,
    Boolean	editable,
    Time	time));

void TSWSetText PROTOTYPE ((Widget w, char *text));

void TSWSetCSText PROTOTYPE ((Widget w, XmString text));

char *TSWGetText PROTOTYPE ((Widget w));

XmString TSWGetCSText PROTOTYPE ((Widget w));

Boolean TSWTextChanged PROTOTYPE ((Widget w, Boolean reset));

Dimension TSWGetTrimmingsWidth PROTOTYPE ((Widget w));

Widget TSWTimeslotCreate PROTOTYPE ((
    Widget	parent,
    char	*name,
    ArgList	args,
    int	argCount));

void TSWReset PROTOTYPE ((Widget w));

Widget TSWGetTextWidget PROTOTYPE ((Widget w));

Widget TSWGetTopSash PROTOTYPE ((Widget w));

Widget TSWGetBottomSash PROTOTYPE ((Widget w));

Widget TSWGetMoveSash PROTOTYPE ((Widget w));

Boolean TSWTraverseToText PROTOTYPE ((Widget w));

Boolean TSWTraverseToSash PROTOTYPE ((Widget w));

#endif /* _timeslotwidget_h_ */
