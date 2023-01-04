#ifndef dwc_ui_misc_h
#define dwc_ui_misc_h 1
/* $Id$ */
/* #module dwc_ui_misc.h */
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
**	Denis G. Lacroix, February 1989
**
**  ABSTRACT:
**
**	Public include file for Calendar's User Interface error handling 
**
**--
*/

#include <time.h>

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>			/* for XmString */
#include <DXm/DXmCSText.h>		/* for DXmCSText */
#include <Xm/Text.h>			/* for XmTextWidget */
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_compat.h"
#include "dwc_ui_calendar.h"		/* for CalendarDisplay */
#include "dwc_ui_uil_values_const.h"
#ifdef __osf__
#include <sys/time.h>
#endif
/*									    
**  Function Prototype Definitions
*/

XtGeometryResult	MISCAgreeableGeometryManager PROTOTYPE ((
	Widget			w,
	XtWidgetGeometry	*request,
	XtWidgetGeometry	*reply
    ));

void			MISCChangeView PROTOTYPE ((
	CalendarDisplay		cd,
	show_kind		view,
	Cardinal		day,
	Cardinal		month,
	Cardinal		year
    ));

Dimension		MISCCvtUnitsToPixels PROTOTYPE ((
	Dimension		internal_unit_dimension,
	Dimension		converting_unit
    ));

void			MISCDayClock PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XmAnyCallbackStruct	*cbs
    ));

void			MISCFetchModal PROTOTYPE ((
	Widget			widget,
	caddr_t			tag,
	XmAnyCallbackStruct	*callback_data
    ));

Widget			MISCFetchWidget PROTOTYPE ((
	Widget			widget,
	caddr_t			tag,
	XmAnyCallbackStruct	*callback_data,
	Boolean			manage_it
    ));

XmString		MISCFetchDRMValue PROTOTYPE ((
	int			format_index
    ));

int			MISCFindCalendarDisplay PROTOTYPE ((
	CalendarDisplay		*cd,
	Widget			widget
    ));


Widget			MISCFindParentShell PROTOTYPE ((
	Widget			w
    ));

void			MISCFitFormDialogOnScreen PROTOTYPE ((
	CalendarDisplay		cd,
	Widget			formdialog
    ));

Boolean			MISCFirstDOWWithCondition PROTOTYPE ((
	CalendarDisplay		cd,
	Cardinal		week,
	Cardinal		fiscal_year,
	Boolean			workday,
	Boolean			entries,
	Cardinal		*day,
	Cardinal		*month,
	Cardinal		*year
    ));

void			MISCGenericCreateProc PROTOTYPE ((
	Widget			widget,
	int			*tag,
	XmAnyCallbackStruct	*callback_data
    ));

void			MISCGetFontFromFontlist PROTOTYPE ((
	XmFontList		fontlist,
	XFontStruct		**font
    ));

void			MISCGetScreenPixelPosition PROTOTYPE ((
	CalendarDisplay		cd,
	Position		percentage_of_screen_x,
	Position		percentage_of_screen_y,
	Dimension		profile_format_width,
	Dimension		profile_format_height,
	Dimension		*return_width,
	Dimension		*return_height,
	Position		*return_x,
	Position		*return_y
    ));

void			MISCGetScreenFractionalPosition PROTOTYPE ((
	CalendarDisplay		cd,
	Dimension		object_width,
	Dimension		object_height,
	Position		x,
	Position		y,
	Position		*percentage_of_screen_x,
	Position		*percentage_of_screen_y
    ));

char			*MISCGetTextFromCS PROTOTYPE ((
	XmString		cs
    ));

void			MISCGetTime PROTOTYPE ((
	struct tm		**time_block
    ));

Time			MISCGetTimeFromEvent PROTOTYPE ((
	XEvent			*event
    ));

Time			MISCGetTimeFromAnyCBS PROTOTYPE ((
	XmAnyCallbackStruct	*cbs
    ));

void			MISCIconStateChange PROTOTYPE ((
	Widget			w,
	caddr_t			tag,
	XPropertyEvent		*p
    ));

Boolean			MISCIncludeFileIntoText PROTOTYPE ((
	CalendarDisplay		cd,
	char			*file_spec,
	DXmCSTextWidget		tw
    ));

void			MISCLoadPixmapArrays PROTOTYPE ((
	Widget			parent,
	unsigned char		size
    ));

void			MISCPutMessagesOnTop PROTOTYPE ((
	Widget			w
    ));

void			MISCRaiseToTheTop PROTOTYPE ((
	Widget			widget
    ));

void			MISCSetAutoScroll PROTOTYPE ((
	CalendarDisplay		cd,
	Boolean			onoff
    ));

void			MISCSetGCClipMask PROTOTYPE ((
	Display			*d,
	GC			gc,
	XRectangle		*expose_area,
	Region			region
    ));

Boolean			MISCTestDayConditions PROTOTYPE ((
	CalendarDisplay		cd,
	Boolean			workday,
	Boolean			has_entries,
	Cardinal		day,
	Cardinal		month,
	Cardinal		year
    ));

Boolean MISCTestRepeatOpen PROTOTYPE ((CalendarDisplay cd));

void MISCUpdateCallback PROTOTYPE ((
	Widget			r,
	XtCallbackList		*rstruct,
	Widget			s,
	XtCallbackList		*sstruct,
	char			*argname
    ));

void MISCUpdateTime PROTOTYPE ((void));

Pixmap MISCXtGrayPixmap PROTOTYPE ((Screen *screen));

void MISCSizeButtonsEqually PROTOTYPE ((
	Widget			*widgets,
	Cardinal		num_widgets
));

void MISCHeightButtonsEqually PROTOTYPE ((
	Widget			*widgets,
	Cardinal		num_widgets
));

void MISCWidthButtonsEqually PROTOTYPE ((
	Widget *widgets,
	Cardinal num_widgets
));

void MISCSpaceButtonsEqually PROTOTYPE ((
	Widget *widgets,
	Cardinal num_widgets
));

#define k_include_success 1
#define k_include_empty 2
#define k_include_fail 0

int MISCFontListWidth PROTOTYPE ((XmFontList	fontlist));

int MISCFontListAscent PROTOTYPE ((XmFontList	fontlist));

int MISCFontListHeight PROTOTYPE ((XmFontList	fontlist));

int MISCFetchIconLiteral_1 PROTOTYPE ((
    MrmHierarchy	hierarchy_id,
    String		index,
    Screen		*screen,
    Display		*display,
    Pixmap		*pixmap,
    unsigned int	width,
    unsigned int	height
));

Boolean MISCIsXUIWMRunning PROTOTYPE ((Widget widget, Boolean check));

Boolean MISCIsXUIWMReallyRunning PROTOTYPE ((Widget widget));

void MISCAddProtocols PROTOTYPE ((
    Widget	widget,
    XtCallbackProc	delete,
    XtCallbackProc	save
    ));

Pixmap MISCCreatePixmapFromBitmap PROTOTYPE ((
    Widget	w,
    Pixmap	src,
    Dimension	width,
    Dimension	height
    ));

void MISCGetBestIconSize PROTOTYPE ((Widget w, int *rheight, int *rwidth));

void MISCFocusOnMap PROTOTYPE ((Widget to_be_mapped, Widget to_be_traversed));

int MISCXmStringCharCount PROTOTYPE ((XmString cs));
#else
#endif	/* dwc_ui_misc_h */
