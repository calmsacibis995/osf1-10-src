#ifndef _timebarwidgetp_h_
#define _timebarwidgetp_h_
/* $Header$ */
/* #module DWC$UI_TIMEBARWIDGETP.H "V3-001"				    */
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
**	This include file contains the private structures etc, for the timebar
**	widget. 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**  V3-001  Paul Ferwerda					05-Mar-1990
**		Port to Motif. Took out TbwDirectionRtoL since nobody was using
**		it. Took out TbwHelpCallback since nobody was using it and the
**		tbw has nothing to "grab". Took away TbwBackgroundGC since it
**		wasn't used at all. Added foregroundand gray_pixmap to
**		the instance info. Added include of <Xm/XmP.h>. Added kludge
**		fix to XmField.
**
**	V2-003	Per Hamnqvist					17-Apr-1989
**		Change reference to times so that it uses element [0] to
**		avoid compilation problems on PMAX
**	V2-002	Per Hamnqvist					05-Apr-1989
**		Remove spaces in call to XmField to make it compile on
**		PMAX ..
**	V1-001  Marios Cleovoulou				Dec-1988
**		Initial version.
**--
**/

#ifdef vaxc
#pragma nostandard
#endif
#include <Xm/XmP.h>		/* for CorePart, XmPrimitive... */
#if (((XmVERSION == 1) && (XmREVISION == 2)) || XmVERSION == 2)
#include <Xm/PrimitiveP.h>
#endif
#ifdef vaxc
#pragma standard
#endif

#include "dwc_compat.h"

typedef unsigned char DwcTbwTimeKind;

typedef struct _TbwTimeRangeRec
{
    Position	    y1;
    Position	    y2;
    DwcTbwTimeKind  kind;
} TbwTimeRangeRec, *TbwTimeRange;


typedef struct _DwcTbgTimebarGidgetRec
{
    XRectangle	    timebar;
    DwcTbwTimeKind  times[24 * 60];
    TbwTimeRange    ranges;
    GC		    foreground_gc;
    Pixmap	    gray_pixmap;
} DwcTbgTimebarGidgetRec, *DwcTbgTimebarGidget;


#define	TimebarIndex    (XmPrimitiveIndex + 1)

/*
**  Instance Part Record
*/
typedef struct
{
    DwcTbwTimeKind	times[24 * 60];
    TbwTimeRange	ranges;
    Dimension		h_margin;
    Dimension		v_margin;
    GC			foreground_gc;
    Pixmap		gray_pixmap;
    XmFontList		fontlist;
} TimebarPart;

/*
**  Instance record
*/

typedef struct _TimebarRec
{
    CorePart		core;
    XmPrimitivePart	primitive;    
    TimebarPart		timebar;
} TimebarWidgetRec, *TimebarWidget;

/*
**  Class part record
*/
typedef struct _TimebarClass
{
    XmOffsetPtr		timebaroffsets;
    int			reserved;
} TimebarClassPart ;

/*
**  Class record
*/

typedef struct _TimebarClassRec
{
    CoreClassPart	    core_class;
    XmPrimitiveClassPart    primitive_class;
    TimebarClassPart	    timebar_class;
} TimebarClassRec, *TimebarWidgetClass;

#define	TbwOffsetPtr(w) \
(((TimebarWidgetClass) ((TimebarWidget) w)->core.widget_class)->timebar_class.timebaroffsets)

#define	TbwBackgroundPixel(w,o)	    XmField(w,o,Core,background_pixel,Pixel)
#define TbwForegroundPixel(w,o)	    XmField(w,o,XmPrimitive,foreground,Pixel)
#define	TbwTimes(w, o)		    XmField (w,o,Timebar,times[0],DwcTbwTimeKind *)
#define	TbwRanges(w, o)		    XmField (w,o,Timebar,ranges,TbwTimeRange)
#define	TbwHMargin(w, o)	    XmField (w,o,Timebar,h_margin,Dimension)
#define	TbwVMargin(w, o)	    XmField (w,o,Timebar,v_margin,Dimension)

#define TbwFontList(w, o)	    XmField(w,o,Timebar,fontlist,XmFontList)
#define	TbwForegroundGC(w, o)	    XmField(w,o,Timebar,foreground_gc,GC)
#define	TbwGrayPixmap(w, o)	    XmField(w,o,Timebar,gray_pixmap,Pixmap)

/*
** Now we do the public stuff.
*/
#include "dwc_ui_timebarwidget.h"

#endif /* _timebarwidgetp_h_ */
