#if !defined(_dayslotswidget_h_)
#define _dayslotswidget_h_
/* $Header$ */
/* #module dwc_ui_dayslotswidget.h */
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
**	This include file contains the public structures etc, for the dayslots
**	widget. 
**
**--
*/

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/X.h>	    /* for Time */
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_compat.h"

#if !defined(_dayslotswidgetp_h_)
typedef struct _DswEntryRecord *DwcDswEntry;
typedef struct _DswTimerBlockRec *DswTimerBlock;
typedef Widget DayslotsWidget;
typedef WidgetClass DayslotsWidgetClass;
extern DayslotsWidgetClass dayslotsWidgetClass;
#endif

#define	DwcDswNpreferredDayslotsSize	"preferredDayslotsSize"
#define	DwcDswNpreferredFineIncrement	"preferredFineIncrement"
#define	DwcDswNonTheLine		"onTheLine"
#define	DwcDswNtimeVerticalMargin	"timeVerticalMargin"
#define	DwcDswNtimeHorizontalMargin	"timeHorizontalMargin"
#define	DwcDswNforeground		XmNforeground
#define	DwcDswNbackground		XmNbackground
#define	DwcDswNindexForeground		"indexForeground"
#define	DwcDswNindexBackground		"indexBackground"
#define	DwcDswNstackTopDown		"stackTopDown"
#define	DwcDswNpixmaps			"pixmaps"
#define	DwcDswNpixmapHeight		"pixmapHeight"
#define	DwcDswNpixmapWidth		"pixmapWidth"
#define	DwcDswNdefaultIcons		"defaultIcons"
#define	DwcDswNdefaultNumIcons		"defaultNumIcons"
#define	DwcDswNtimeTexts		"timeTexts"
#define	DwcDswNminuteTexts		"minuteTexts"
#define	DwcDswNeditable			XmNeditable
#define	DwcDswNdayNotes			"dayNotes"

#define	DwcDswNworkStart		"workStart"
#define	DwcDswNworkFinish		"workFinish"
#define	DwcDswNlunchStart		"lunchStart"
#define	DwcDswNlunchFinish		"lunchFinish"

#define	DwcDswNtimebarWidth		"timebarWidth"
#define	DwcDswNwrkhrsTimebar		"wrkhrsTimebar"
#define	DwcDswNhrsTimebar		"hrsTimebar"

#define	DwcDswNgetTextCallback		"getTextCallback"
#define	DwcDswNscrollCallback		"scrollCallback"
#define	DwcDswNentryCallback		"entryCallback"
#define	DwcDswNentryHelpCallback	"entryHelpCallback"
#define DwcDswNfocusDisposeCallback	"focusDisposeCallback"
#define DwcDswNfontList			XmNfontList

#define DwcDswCDayslotsSize		"DayslotsSize"
#define	DwcDswCFineIncrement	        "FineIncrement"
#define	DwcDswCOnTheLine		"OnTheLine"
#define	DwcDswCTimeMargin		"TimeMargin"
#define	DwcDswCStackTopDown		"StackTopDown"
#define	DwcDswCPixmaps			"Pixmaps"
#define	DwcDswCPixmapHeight		"PixmapHeight"
#define	DwcDswCPixmapWidth		"PixmapWidth"
#define	DwcDswCDefaultIcons		"DefaultIcons"
#define	DwcDswCDefaultNumIcons		"DefaultNumIcons"
#define	DwcDswCTimeTexts		"TimeTexts"
#define	DwcDswCMinuteTexts		"MinuteTexts"
#define	DwcDswCEditable			"Editable"
#define	DwcDswCDayNotes			"DayNotes"

#define	DwcDswCWorkStart		"WorkStart"
#define	DwcDswCWorkFinish		"WorkFinish"
#define	DwcDswCLunchStart		"LunchStart"
#define	DwcDswCLunchFinish		"LunchFinish"

#define	DwcDswCTimebarWidth		"TimebarWidth"
#define	DwcDswCWrkhrsTimebar		"WrkhrsTimebar"
#define	DwcDswCHrsTimebar		"HrsTimebar"
#define DwcDswCFontList			XmCFontList

#define DwcDswCRScroll		    -1
#define DwcDswCREntryCreate	    -2
#define DwcDswCREntryChange	    -3
#define DwcDswCREntryEdit	    -4
#define DwcDswCREntryEditNew	    -5
#define DwcDswCRCompleteOutstanding -6
#define DwcDswCREntrySelect	    -7
#define DwcDswCREntryExtendSelect   -8
#define DwcDswCREntryDeselect	    -9
#define DwcDswCREntryQuickCopy	    -10
#define DwcDswCREntryQuickMove	    -11
#define DwcDswCRGetText		    -12


typedef struct {
    int		    reason ;
    XEvent	    *event ;
    int		    value ;
    int		    shown ;
    int		    inc ;
    int		    pageinc ;
    int		    min_value ;
    int		    max_value ;
} DSWScrollCallbackStruct, *DSWScrollCallbackStructPtr ;

typedef struct {
    int		    reason ;
    XEvent	    *event ;
    XmString	    *times ;
    int		    index ;
} DSWGetTextCallbackStruct, *DSWGetTextCallbackStructPtr ;


typedef struct {
    int		    reason ;
    XEvent	    *event ;
    Boolean	    text_changed ;
    Boolean	    time_changed ;
    int		    start ;
    int		    duration ;
    Time	    time ;
    caddr_t	    tag ;
    DwcDswEntry	    entry ;
} DSWEntryCallbackStruct, *DSWEntryCallbackStructPtr ;


typedef struct {
    Boolean	    text_changed ;
    int		    start ;
    int		    duration ;
    caddr_t	    tag ;
    Widget	    parent ;
} DSWEntryDataStruct, *DSWEntryDataStructPtr ;

Widget DSWDayslotsCreate PROTOTYPE ((
	Widget	parent,
	char	*name,
	Arg	*arglist,
	int	argcount));

Boolean DSWRequestOperationOk PROTOTYPE ((DayslotsWidget w, Time time));

Widget DwcDayslots PROTOTYPE ((
	Widget			parent,
	char			*name,
	unsigned short int	dayslots_size,
	Position		y,
	Dimension		width,
	XtCallbackList		help_callback));

void DSWUpdateDisplay PROTOTYPE ((DayslotsWidget dsw));

Boolean DSWRequestCloseOpenEntry PROTOTYPE ((DayslotsWidget dsw, Time time));

DwcDswEntry DSWGetOpenEntry PROTOTYPE ((DayslotsWidget dsw));

void DSWClearAllEntries PROTOTYPE ((DayslotsWidget dsw));

DwcDswEntry DSWAddEntry PROTOTYPE ((
	DayslotsWidget		dsw,
	unsigned short int	time,
	unsigned short int	duration,
	char			*text,
	unsigned char		*icons,
	Cardinal		num_icons,
	Boolean			editable,
	caddr_t			tag));

DwcDswEntry DSWAddEntryCS PROTOTYPE ((
	DayslotsWidget		dsw,
	unsigned short int	time,
	unsigned short int	duration,
	XmString		text,
	unsigned char		*icons,
	Cardinal		num_icons,
	Boolean			editable,
	caddr_t			tag));

void DSWGetEntryData PROTOTYPE ((
	DwcDswEntry entry,
	DSWEntryDataStructPtr data));

void DSWDeleteEntry PROTOTYPE ((DayslotsWidget dsw, DwcDswEntry entry));

void DSWCloseEntry PROTOTYPE ((
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	Time		time));

void DSWSetEntryText PROTOTYPE ((DwcDswEntry entry, char *text));

void DSWSetEntryCSText PROTOTYPE ((DwcDswEntry entry, XmString text));

char *DSWGetEntryText PROTOTYPE ((DwcDswEntry entry));

XmString DSWGetEntryCSText PROTOTYPE ((DwcDswEntry entry));

Widget DSWGetEntryTextWidget PROTOTYPE ((DwcDswEntry entry));

Widget DSWGetEntryTimeslotWidget PROTOTYPE ((DwcDswEntry entry));

void DSWSetEntryTag PROTOTYPE ((DwcDswEntry entry, caddr_t tag));

void DSWSetEntryTimes PROTOTYPE ((
	DwcDswEntry		entry,
	unsigned short int	time,
	unsigned short int	duration));

void DSWSetEntryEditable PROTOTYPE ((
	DwcDswEntry	entry,
	Boolean		editable,
	Time		time));

Boolean DSWGetEntryEditable PROTOTYPE ((DwcDswEntry entry));

void DSWSetEntryIcons PROTOTYPE ((
	DayslotsWidget	dsw,
	DwcDswEntry	entry,
	unsigned char	*icons,
	Cardinal	num_icons));

void DSWSetWorkHours PROTOTYPE ((
	DayslotsWidget	dsw,
	Cardinal	start,
	Cardinal	finish));

void DSWMoveDayslots PROTOTYPE ((DayslotsWidget dsw, Position new_y));

void DSWMoveDayslotsToTime PROTOTYPE ((DayslotsWidget dsw, int time));

void DSWDoScrollCallback PROTOTYPE ((DayslotsWidget dsw));

void DSWSetEntrySelected PROTOTYPE ((DwcDswEntry entry, Boolean selected));

void DSWCallSelectExtendOnAll PROTOTYPE ((DayslotsWidget dsw, Time time));

Cardinal DSWGetDayslotsHeight PROTOTYPE ((DayslotsWidget dsw));

void DSWCancelOperation PROTOTYPE ((DayslotsWidget dsw));

Boolean DSWCheckValues PROTOTYPE ((
	DayslotsWidget	dsw,
	Pixmap		*pixmaps,
	XmFontList	fontlist,
	int		time_v_margin,
	int		slot_size,
	int		fine_increment));

Boolean DSWTraverseToOpen PROTOTYPE ((DayslotsWidget dsw));

#else
#endif /* _dayslotswidget_h_ */
