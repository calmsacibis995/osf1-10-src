/* dwc_ui_icons.c */
#ifndef lint
static char rcsid[] = "$Id$";
#endif /* lint */
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
**	Marios Cleovoulou, May-1988
**
**  ABSTRACT:
**
**	This is the icon module for the DECWindows Calendar.
**
**--
*/

#include "dwc_compat.h"

#if defined(vaxc) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <X11/Intrinsic.h>		/* basic X things, Widget,etc. */
#include <X11/Shell.h>			/* XtNiconPixmap */
#include <X11/cursorfont.h>		/* XC_watch */
#include <X11/DECwI18n.h>
#include <X11/DECWmHints.h>
#include <DXm/DECspecific.h>		/* cursor types */
#include <Xm/AtomMgr.h>			/* for XmInternAtom */
#include <dwi18n_lib.h>
#if defined(vaxc) && !defined(__DECC)
#pragma standard
#endif

#include "dwc_ui_calendar.h"		/* AllDisplaysRec */
#include "dwc_ui_dateformat.h"		/* for DATEFORMATTimeToText */
#include "dwc_ui_datestructures.h"	/* dtb */
#include "dwc_ui_uil_values_const.h"	/* dwc_k_icon_day_format */
#include "dwc_ui_misc.h"		/* for MISCGetFontFromFontlist */
#include "dwc_ui_icons.h"

extern AllDisplaysRecord ads;		/* defined in dwc_ui_calender.c */

static	XColor  black_background = {0,     0,     0,     0}; /* Black */
static	XColor  white_foreground = {0, 65535, 65535, 65535}; /* White */

/*
**  Width and height of all icons....
*/
extern Pixmap main_icons[4],alarm_icons[4];

static const unsigned int	icon_sizes[] = {17, 32, 50, 75};
static const int		icon_top_line[] = {-1, -1, 15, 30};
static const int		icon_bottom_line[] = {16, 24, 37, 52};
static const int		icon_center_line[] = {9, 16, 24, 37};
static const char		*main_names[] =
{
    "CalendarIcon17",
    "CalendarIcon32",
    "CalendarIcon50",
    "CalendarIcon75"
};
static const char		*alarm_names[] =
{
    "AlarmIcon17",
    "AlarmIcon32",
    "AlarmIcon50",
    "AlarmIcon75"
};
static const Pixmap		*type_array[]={main_icons,alarm_icons};
static const char		**name_array[]={main_names,alarm_names};


/*	  
**  We've need a depth one bitmap so we don't need real colors here
*/	  
#define BITMAP_FOREGROUND	1
#define BITMAP_BACKGROUND	0


Cursor ICONSInactiveCursorCreate PROTOTYPE ((Widget w));
Cursor ICONSWaitCursorCreate PROTOTYPE ((Widget w));

Pixmap ICONSCopyPixmap
#ifdef _DWC_PROTO_
	(
	Widget		toplevel,
	Pixmap		src,
	unsigned int	width,
	unsigned int	height)
#else
	(toplevel, src, width, height)
	Widget		toplevel;
	Pixmap		src;
	unsigned int	width;
	unsigned int	height;
#endif
{
    Display			*display = XtDisplay(toplevel);
    Window			root = RootWindowOfScreen(XtScreen(toplevel));
    static unsigned long	mask = GCGraphicsExposures;
    Pixmap			new;
    XGCValues			gcv;
    static GC			gc = 0;

    new = XCreatePixmap (display, root, width, height, 1);

    if (gc == (GC) 0)
    {
	gcv.graphics_exposures = True;
	gc = XCreateGC (display, new, mask, &gcv);
    }

    XCopyArea (display, src, new, gc, 0, 0, width, height, 0, 0);

    return (new);
}

void ICONSSetIconifyIcon
#ifdef _DWC_PROTO_
	(
	Widget		toplevel,
	int		icon_type)
#else
	(toplevel, icon_type)
	Widget	toplevel;
	int		icon_type;
#endif
{
    Pixmap		spix;
    Atom		dwm_atom;
    DECWmHintsRec	dwm_hint;
    int			status;

    /*
    ** Check for XUI.
    */
    if (!MISCIsXUIWMRunning (toplevel, False)) return;

    /*
    ** Look for dxwm and do iconify pixmap if possible.
    */
    dwm_atom = XmInternAtom (XtDisplay(toplevel), "DEC_WM_HINTS", True);
    if (dwm_atom != None)
    {
	if ((type_array[icon_type])[0] == 0)
	{
	    status = MISCFetchIconLiteral_1
	    (
		ads.hierarchy,
		(String)(name_array[icon_type])[0],
		XtScreen(toplevel),
		XtDisplay(toplevel),
		(Pixmap *)&(type_array[icon_type])[0],
		icon_sizes[0],
		icon_sizes[0]
	    );
	    if (status != MrmSUCCESS) return;
	}

	spix = (type_array[icon_type])[0];

	dwm_hint.value_mask = DECWmIconifyPixmapMask;
	dwm_hint.iconify_pixmap = spix;
	XChangeProperty
	(
	    XtDisplay(toplevel),
	    XtWindow (toplevel),
	    dwm_atom,
	    dwm_atom,
	    32,
	    PropModeReplace,
	    (unsigned char *)&dwm_hint,
	    9
	);
    }
}

void ICONSSetIconBoxIcon
#ifdef _DWC_PROTO_
	(
	Widget		toplevel,
	char		icon_bits[],
	dtb		*date_time,
	XmFontList	day_fontlist,
	XmFontList	month_fontlist,
	Boolean		invert,
	Boolean		reparent,
	Boolean		mapped)
#else	/* no prototypes */
	(toplevel, icon_bits, date_time, day_fontlist, month_fontlist, invert,
	reparent, mapped)
	Widget		toplevel;
	char		icon_bits[];
	dtb		*date_time;
	XmFontList	day_fontlist;
	XmFontList	month_fontlist;
	Boolean		invert;
	Boolean		reparent;
	Boolean		mapped;
#endif	/* prototype */
{
    Pixmap		icon_pixmap, old_icon_pixmap = 0;
    XGCValues		gcv;
    int			direction;
    int			font_descent_return;
    int			font_ascent_return;
    XCharStruct		overall;
    int			x;
    Arg			arglist [10];
    Cardinal		ac;
    XWMHints		*wmhints;
    int			topline, bottomline, centerline;
    int			icon_type;
    int			status;
    int			t_height, t_width;

    /*
    ** A bunch of stuff to cache to improve subsequent performance.
    */
    static Cardinal	month_text_size;
    static Cardinal	day_text_size;
    static Cardinal	month_x;
    static Cardinal	month_y;
    static Cardinal	day_x;
    static char		*day = NULL;
    static char		*month = NULL;
    static int		day_num = 0;
    static int		month_num = 0;
    static GC		day_gc = 0;
    static GC		month_gc = 0;
    static GC		inv_day_gc = 0;
    static XFontStruct	*day_font = NULL;
    static XFontStruct	*month_font = NULL;
    static int		height = 0;
    static int		width = 0;
    static int		size_index = 0;
    XmString		xm_str;
    long		byte_count, cvt_status;
    static Widget	save_toplevel = NULL;

    /*
    ** If this is the first bogus alarm call.  It comes before anything else.
    ** Ignore it.
    */
    if (!reparent && (height == 0)) return;

    /*
    ** Set the iconify pixmap if needed.
    */
    if (date_time != NULL)
	icon_type = 0;
    else
	icon_type = 1;

    /*
    ** Handle a fairly common situation where no change is necessary because
    ** an alarm went off but the date hasn't changed from when the application
    ** first set stuff up.
    */
    if ((date_time != NULL) &&
	(date_time->day == day_num) &&
	(date_time->month == month_num) &&
	(save_toplevel == toplevel) &&
	!reparent) return;

    /*
    ** Only recheck the sizes if forced.  This won't speed up start up but
    ** it may help a little later.
    */
    if (reparent)
    {
	/*
	** Get the maximum supported icon size.
	*/
	MISCGetBestIconSize (toplevel, &t_height, &t_width);

	for (size_index = XtNumber(main_icons) - 1;
		size_index > 0;
		    size_index--)
	{
	    if ((t_height >= icon_sizes[size_index]) &&
		(t_width >= icon_sizes[size_index])) break;
	}

	/*
	** On a reparent notify, we don't need to check whether the dates
	** have changed (they haven't) only whether the size has changed.
	*/
	if (icon_sizes[size_index] == height && toplevel == save_toplevel)
	    return;
	save_toplevel = toplevel;

	height = width = icon_sizes[size_index];
    }

    /*
    ** Do if have the icon of the current size?  If not get it from Mrm.
    */
    if ((type_array[icon_type])[size_index] == 0)
    {
	status = MISCFetchIconLiteral_1
	(
	    ads.hierarchy,
	    (String)(name_array[icon_type])[size_index],
	    XtScreen(toplevel),
	    XtDisplay(toplevel),
	    (Pixmap *)&(type_array[icon_type])[size_index],
	    icon_sizes[size_index],
	    icon_sizes[size_index]
	);
	if (status != MrmSUCCESS)
	    return;
    }

    /*
    ** Have to copy the pixmap because we write into it.
    */
    icon_pixmap = ICONSCopyPixmap
	(toplevel, (type_array[icon_type])[size_index], width, height);

    /*
    ** Do the date display in the icon if necessary.
    */
    if (date_time != NULL)
    {
	/*
	** Where do we draw.
	*/
	topline = icon_top_line[size_index];
	bottomline = icon_bottom_line[size_index];
	centerline = icon_center_line[size_index];

	/*
	** Get the fonts for writing stuff into the icon.  These are cached.
	*/
	if (day_font == NULL)
	{
	    MISCGetFontFromFontlist (day_fontlist, &day_font);
	    MISCGetFontFromFontlist (month_fontlist, &month_font);
	}

	/*
	**  Set up GCs for day and month text.  These are cached.
	*/
	gcv.foreground = BITMAP_FOREGROUND;
	gcv.font = day_font->fid;
	if (day_gc == (GC)0)
	{
	    day_gc = XCreateGC
		(XtDisplay(toplevel), icon_pixmap, GCFont | GCForeground, &gcv);
	}
	gcv.font = month_font->fid;
	if (month_gc == (GC)0)
	{
	    month_gc = XCreateGC
		(XtDisplay(toplevel), icon_pixmap, GCFont | GCForeground, &gcv);
	}

	/*
	**  Get the month and day strings.  Cached as necessary.
	*/
	if ((date_time->month != month_num) || month == NULL)
	{
	    month_num = date_time->month;
	    if (month != NULL) XtFree (month);
	    month = DATEFORMATTimeToText (dwc_k_icon_month_format, date_time);
	}
	if ((date_time->day != day_num) || day == NULL)
	{
	    day_num = date_time->day;
	    if (day != NULL) XtFree (day);		 
	    day = DATEFORMATTimeToText (dwc_k_icon_day_format, date_time);
	}

	/*
	**  Write the month name into the pixmap below ICON_TOP_LINE centering
	**  in the pixmap.
	*/
	if (topline >= 0)
	{
	    xm_str = DXmCvtFCtoCS(month, &byte_count, &cvt_status);
	    XmStringDraw
	    (
		XtDisplay(toplevel),
		icon_pixmap,
		month_fontlist,
		xm_str,
		month_gc,
		0,
/* - baseline ? */
		topline,
		width,
		XmALIGNMENT_CENTER,
		XmSTRING_DIRECTION_DEFAULT,
		NULL
	    );
	    XmStringFree(xm_str);
	}

	/*
	**  Write the day number into the pixmap above ICON_BOTTOM_LINE
	**  centering in the pixmap.
	*/
	xm_str = DXmCvtFCtoCS (day, &byte_count, &cvt_status);
	XmStringDraw
	(
	    XtDisplay(toplevel),
	    icon_pixmap,
	    day_fontlist,
	    xm_str,
	    day_gc,
	    0,
	    bottomline - XmStringBaseline(day_fontlist, xm_str),
	    width,
	    XmALIGNMENT_CENTER,
	    XmSTRING_DIRECTION_DEFAULT,
	    NULL
	);
	XmStringFree(xm_str);
    }
    
    if (invert)
    {
	gcv.function   = GXinvert;
	gcv.foreground = BITMAP_FOREGROUND;
	gcv.background = BITMAP_BACKGROUND;
	gcv.plane_mask = gcv.foreground ^ gcv.background;
	if (inv_day_gc == (GC)0)
	{
	    inv_day_gc = XCreateGC
	    (
		XtDisplay(toplevel),
		icon_pixmap,
		GCFunction | GCPlaneMask,
		&gcv
	    );
	}

	XFillRectangle
	    (XtDisplay (toplevel), icon_pixmap, inv_day_gc, 0, 0, width, height);
    }
#if 0    
    XtVaGetValues (toplevel, XtNiconPixmap, &old_icon_pixmap, NULL);
    if (old_icon_pixmap != 0)
	XFreePixmap (XtDisplay(toplevel), old_icon_pixmap);
#endif
    /*	  
    **  HACK: Under Motif 1.1 changing iconPixmap will cause the window to go to
    **	its intial state. That is, if you have calendar set to start iconic,
    **	it's mapped, and you pass midnight, it will automatically iconify
    **	itself.  This appears to be a side-effect of ICCCM-compliant behavior,
    **	so we need to call Xlib directly instead of setting XtNiconPixmap. Code
    **	from DECW$MAIL.
    */
    if (((XtWindow(toplevel)) != (Window)0) && mapped)
    {
	wmhints = XGetWMHints(XtDisplay(toplevel), XtWindow(toplevel));
	if (wmhints == NULL)
	{
	    wmhints = (XWMHints *)XtCalloc(1, sizeof(XWMHints));
	    wmhints->flags &= ~StateHint;
	    wmhints->flags |= IconPixmapHint;
	    wmhints->icon_pixmap = icon_pixmap;
	    XSetWMHints(XtDisplay(toplevel), XtWindow(toplevel), wmhints);
	    XtFree ((char *)wmhints);
	}
	else
	{
	    wmhints->flags &= ~StateHint;
	    wmhints->flags |= IconPixmapHint;
	    wmhints->icon_pixmap = icon_pixmap;
	    XSetWMHints(XtDisplay(toplevel), XtWindow(toplevel), wmhints);
	    XFree ((char *)wmhints);
	}
    }
    else
    {
	ac=0;
	XtSetArg(arglist[ac], XtNiconPixmap, icon_pixmap); ac++;
	XtSetValues(toplevel, arglist, ac);
    }

}

void ICONSSetIconTime
#ifdef _DWC_PROTO_
	(
	Widget	toplevel,
	dtb	*date_time,
	Boolean	show_text,
	char	*text,
	Boolean	nl_after_text,
	Boolean	show_day,
	Boolean	full_day,
	Boolean	nl_after_day,
	Boolean	show_time,
	Boolean	ampm)
#else	/* no prototypes */
	(toplevel, date_time, show_text, text, nl_after_text, show_day, full_day, nl_after_day, show_time, ampm)
	Widget	toplevel;
	dtb	*date_time;
	Boolean	show_text;
	char	*text;
	Boolean	nl_after_text;
	Boolean	show_day;
	Boolean	full_day;
	Boolean	nl_after_day;
	Boolean	show_time;
	Boolean	ampm;
#endif	/* prototype */
{
    char	icon_text [256];
    char	*time_text;
    Arg		arglist [10];
    long	byte_count, cvt_status;
    XmString	xm_icon_text;

    icon_text [0] = '\0';

    if (show_text)
    {
	strcat (icon_text, text);
	if (show_day || show_time)
	{
	    if (nl_after_text)
	    {
		strcat (icon_text, "\n");
	    }
	    else
	    {
		strcat (icon_text, " ");
	    }
	}
    }

    if (show_day)
    {
	if (full_day)
	{
	    time_text = DATEFORMATTimeToText (dwc_k_icon_dayname_full_format, date_time);
	}
	else
	{
	    time_text = DATEFORMATTimeToText (dwc_k_icon_dayname_short_format, date_time);
	}

	strcat (icon_text, time_text);
	XtFree (time_text);
	if (show_time)
	{
	    if (nl_after_day)
	    {
		strcat (icon_text, "\n");
	    }
	    else
	    {
		strcat (icon_text, " ");
	    }
	}
    }

    if (show_time)
    {
	if (ampm)
	{
	    time_text = DATEFORMATTimeToText (dwc_k_icon_time_ampm_format, date_time);
	}
	else
	{
	    time_text = DATEFORMATTimeToText (dwc_k_icon_time_format, date_time);
	}

	strcat (icon_text, time_text);
	XtFree (time_text);
    }

    xm_icon_text = DXmCvtFCtoCS(icon_text, &byte_count, &cvt_status);
    DWI18n_SetIconName (toplevel, xm_icon_text);
    XmStringFree(xm_icon_text);

}

Cursor ICONSWaitCursorCreate
#ifdef _DWC_PROTO_
	(
	Widget   w)
#else	/* no prototypes */
	(w)
	Widget  w;
#endif	/* prototype */
{
    return (DXmCreateCursor(w, DXm_WAIT_CURSOR));
}

Cursor ICONSInactiveCursorCreate
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    return (DXmCreateCursor(w, DXm_INACTIVE_CURSOR));
}

static void cursor_display
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Cursor	cursor,
	Boolean	this_one_too)
#else	/* no prototypes */
	(w, cursor, this_one_too)
	Widget	w;
	Cursor	cursor;
	Boolean	this_one_too;
#endif	/* prototype */
{
    Cardinal	i, j;
    Widget	wshell;
    Widget	shell;
    Widget	*popups;
    Cardinal	num_popups;

    wshell = (Widget) MISCFindParentShell (w);

    shell  = ads.root;
    popups = shell->core.popup_list;
    num_popups = shell->core.num_popups;

    for (i = 0; i < num_popups; i++)
    {
	if (XtIsRealized (popups [i]) && (popups [i] != wshell))
	{
	    XDefineCursor
		(XtDisplay (popups [i]), XtWindow (popups [i]), cursor);
	}
    }

    for (j = 0;  j < ads.number_of_calendars;  j++)
    {
	shell  = ads.cds [j]->toplevel;
	if (shell == NULL) continue;
	popups = shell->core.popup_list;
	num_popups = shell->core.num_popups;

	if (XtIsRealized (shell) && (shell != wshell))
	{
	    XDefineCursor (XtDisplay (shell), XtWindow (shell), cursor);
	}

	for (i = 0;  i < num_popups;  i++)
	{
	    if (XtIsRealized (popups [i]) && (popups [i] != wshell))
	    {
		XDefineCursor
		    (XtDisplay (popups [i]), XtWindow (popups [i]), cursor);
	    }
	}
    }
    
    if (this_one_too)
    {
	XDefineCursor (XtDisplay (wshell), XtWindow (wshell), cursor);
    }

    XFlush (XtDisplay (wshell));

}

void ICONSWaitCursorDisplay
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Cursor	cursor)
#else	/* no prototypes */
	(w, cursor)
	Widget	w;
	Cursor	cursor;
#endif	/* prototype */
{
    cursor_display (w, cursor, TRUE);
}

void ICONSInactiveCursorDisplay
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Cursor	cursor)
#else	/* no prototypes */
	(w, cursor)
	Widget	w;
	Cursor	cursor;
#endif	/* prototype */
{
    cursor_display (w, cursor, FALSE);
}

static void cursor_remove
#ifdef _DWC_PROTO_
	(
	Widget	w,
	Boolean	this_one_too)
#else	/* no prototypes */
	(w, this_one_too)
	Widget	w;
	Boolean	this_one_too;
#endif	/* prototype */
{
    Cardinal	i, j;
    Widget	wshell;
    Widget	shell;
    Widget	*popups;
    Cardinal	num_popups;

    wshell = (Widget) MISCFindParentShell(w);

    shell  = ads.root;
    popups = shell->core.popup_list;
    num_popups = shell->core.num_popups;

    for (i = 0;  i < num_popups;  i++)
    {
	if (XtIsRealized (popups [i]) && (popups [i] != wshell))
	{
	    XUndefineCursor (XtDisplay (popups [i]), XtWindow (popups [i]));
	}
    }

    for (j = 0;  j < ads.number_of_calendars;  j++)
    {
	shell  = ads.cds [j]->toplevel;
	if (shell == NULL) continue;
	popups = shell->core.popup_list;
	num_popups = shell->core.num_popups;

	if (XtIsRealized (shell) && (shell != wshell))
	{
	    XUndefineCursor (XtDisplay (shell), XtWindow (shell));
	}

	for (i = 0;  i < num_popups;  i++)
	{
	    if (XtIsRealized (popups [i]) && (popups [i] != wshell))
	    {
		XUndefineCursor (XtDisplay (popups [i]), XtWindow (popups [i]));
	    }
	}
    }

    if (this_one_too)
    {
	XUndefineCursor (XtDisplay (wshell), XtWindow (wshell));
    }

    XFlush (XtDisplay (wshell));

}

void ICONSWaitCursorRemove
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    cursor_remove (w, TRUE);
}

void ICONSInactiveCursorRemove
#ifdef _DWC_PROTO_
	(
	Widget	w)
#else	/* no prototypes */
	(w)
	Widget	w;
#endif	/* prototype */
{
    cursor_remove (w, FALSE);
}
