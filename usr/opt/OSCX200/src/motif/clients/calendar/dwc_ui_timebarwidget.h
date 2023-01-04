#if !defined(_timebarwidget_h_)
#define _timebarwidget_h_ 1
/* $Header$ */
/* #module DWC$UI_TIMEBARWIDGET.H "V1-001"				    */
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
**	This include file contains the public structures etc, for the timebar
**	widget. 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V1-001  Marios Cleovoulou				Dec-1988
**		Initial version.
**--
**/

#include "dwc_compat.h"

#define DwcTbwNforeground		"foreground"
#define DwcTbwNbackground		"background"
#define DwcTbwNhorizontalMargin		"horizontalMargin"
#define DwcTbwNverticalMargin		"verticalMargin"
#define DwcTbwNfontList			"fontList"

#define DwcTbwCHorizontalMargin		"HorizontalMargin"
#define DwcTbwCVerticalMargin		"VerticalMargin"
#define DwcTbwCFontList			"FontList"

#define DwcTbwNonWork			0
#define DwcTbwWork			1
#define DwcTbwBusy			2

#if !defined(_timebarwidgetp_h_)
typedef unsigned char DwcTbwTimeKind;
typedef struct _DwcTbgTimebarGidgetRec *DwcTbgTimebarGidget;
typedef struct _TbwTimeRangeRec *TbwTimeRange;
typedef Widget TimebarWidget;
typedef WidgetClass TimebarWidgetClass;
extern TimebarWidgetClass timebarWidgetClass;
#endif

void TBWDestroyGidget PROTOTYPE ((Widget w, DwcTbgTimebarGidget tbg));

void TBWRedisplayGidget PROTOTYPE ((
	Widget			w,
	DwcTbgTimebarGidget	tbg,
	XRectangle		*expose_area));

void TBWResizeGidget PROTOTYPE ((
	Widget			w,
	DwcTbgTimebarGidget	tbg,
	Position		x,
	Position		y,
	Dimension		width,
	Dimension		height));

void TBWUpdateGidget PROTOTYPE ((Widget w, DwcTbgTimebarGidget tbg));

Widget TBWTimebarCreate PROTOTYPE ((
	Widget			parent,
	char			*name,
	Arg			*arglist,
	int			argcount));

DwcTbgTimebarGidget TBWCreateGidget PROTOTYPE ((
	Widget			w,
	Position		x,
	Position		y,
	Dimension		width,
	Dimension		height,
	Pixel			foreground));

void TBWSetupTimebar PROTOTYPE ((
	Widget			w,
	Cardinal		start,
	Cardinal		finish,
	DwcTbwTimeKind		kind));

void TBWUpdateTimebar PROTOTYPE ((Widget w));

void TBWSetupGidget PROTOTYPE ((
	DwcTbgTimebarGidget		tbg,
	Cardinal			start,
	Cardinal			finish,
	DwcTbwTimeKind			kind));

void TBWSetTilesAndStipples PROTOTYPE ((Widget w));

#endif /* _timebarwidget_h_ */
