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
#ifdef OSF1
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clockutils.c,v 1.1.7.4 1993/10/19 19:56:43 Susan_Ng Exp $";
#endif
#endif
/* Header from VMS source 
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clockutils.c,v 1.1.7.4 1993/10/19 19:56:43 Susan_Ng Exp $";
*/
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	clockutils.c
**
**  FACILITY:
**      OOTB Clock
**
**  ABSTRACT:
**	Various Display and utilites routines for the clock
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:
**	6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/

#include "clockdefs.h"
#include "clock.h"
#include "dwi18n_lib.h"

#include <math.h>
#include <ctype.h>

#ifndef NO_AUDIO
#include "diva_def.h"
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif

#include <Xm/Protocols.h>
#include <DXm/DECspecific.h>

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifdef NO_AUDIO
#define NO_SOUND 1
#else
#define NO_SOUND 0
#endif

Cursor watch_cursor = 0;
Cursor WorkCursorCreate();
void WorkCursorDisplay();
void WorkCursorRemove();
void DrawBell();
void DrawDate();
void DrawDigital();
void clock_initialize();
void set_off_alarm();
void set_time_strings();
void mode_settings();
void display_error_message();
void update_clock();
void update_mode();
void reset_mode();
void update_alarm();
void reset_alarm();
void military_update(); 
void LoadFontFamilies();
void ClockLoadFontFamilies();
static void DrawHand();
static int round();
void get_audio_hardware();
void play_completion_handler();
void audio_play_async();
void audio_set_insensitive();
void audio_button_pressed();
void audio_set_volume();
void audio_stop();

/* In clock.c	*/
extern void Layout();
extern void clock_display_message();
extern Widget toplevel;
extern XtAppContext app_context;
#ifndef NO_AUDIO
diva_msg_context diva_context = 0;
#endif

/* Clock Xt Warning handler */
void ClockXtWarnHandler();



/*
**++
**  ROUTINE NAME:
**	WorkCursorCreate
**
**  FUNCTIONAL DESCRIPTION
**	This routine creates a "wait" cursor
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
Cursor WorkCursorCreate(wid)
    Widget wid;				/* Widget to be used to create the
					 * cursor.  The fields used are the
					 * display and the colormap.  Any
					 * widget with the same display and
					 * colormap can be used later with this
					 * cursor */
{
    int status;
    Cursor cursor;
    Font font;
    XColor fcolor, bcolor, dummy;

    font = XLoadFont(XtDisplay(wid), "DECw$Cursor");
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "Black",
      &fcolor, &dummy);
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "White",
      &bcolor, &dummy);
    cursor = XCreateGlyphCursor(XtDisplay(wid), font, font, decw$c_wait_cursor,
      decw$c_wait_cursor + 1, &fcolor, &bcolor);
    return cursor;
}

/*
**++
**  ROUTINE NAME:
**	WorkCursorDisplay
**
**  FUNCTIONAL DESCRIPTION
**	This routine displays a watch cursor in the
**	Clock window.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorDisplay()
{

    /* Display the watch cursor in the clock window */
    if (watch_cursor == 0) {
	watch_cursor = DXmCreateCursor(toplevel, decw$c_wait_cursor);
    }

    XDefineCursor(XtDisplay(toplevel), XtWindow(toplevel), watch_cursor);

    /* Use XtAddGrab to enable toolkit filtering of events */
    XtAddGrab(toplevel, TRUE, FALSE);
    XFlush(XtDisplay(toplevel));
}

/*
**++
**  ROUTINE NAME:
**	WorkCursorRemove
**
**  FUNCTIONAL DESCRIPTION
**	Return to the normal cursor in both windows
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorRemove()
{
    XUndefineCursor(XtDisplay(toplevel), XtWindow(toplevel));

    /* Use XtRemoveGrab to disable toolkit filtering of events */
    XtRemoveGrab(toplevel);
}

/*
**++
**  ROUTINE NAME:
**	clock_initialize (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	initialize clock variables
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	AnalogGC, DigitalGC, DateGC, BellGC
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void clock_initialize(w)
    Widget w;
{
    ClockWidget mw = (ClockWidget) w;

    Settings(mw) = 0;
    AlarmSettings(mw) = 0;
    AnalogGC(mw) = 0;
    DigitalGC(mw) = 0;
    DateGC(mw) = 0;
    BellGC(mw) = 0;

    DatePercDn(mw) = mw->clock. date_int_dn / 100.0;
    DatePercDa(mw) = mw->clock. date_int_da / 100.0;
    AnalogPercDa(mw) = mw->clock. analog_int_da / 100.0;
    AnalogPercNa(mw) = mw->clock. analog_int_na / 100.0;
    AnalogPercDna(mw) = mw->clock. analog_int_dna / 100.0;

    HandWidth(mw) = 2;
}

/*
**++
**  ROUTINE NAME:
**	clock_FindMainWidget (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Returns the id of the clock widget
**
**  FORMAL PARAMETERS:
**	w - the the child
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**	ClockWidget - the resulting clock widget
**
**  SIDE EFFECTS:
**--
**/
ClockWidget clock_FindMainWidget(w)
    Widget w;
{
    Widget p;

    p = w;
    while (!XtIsSubclass(p, (WidgetClass) clockwidgetclass))
    {
	Widget temp;
	temp = XtParent(p);
        if (temp == NULL)
        {
            int i;
            WidgetList foo;

            foo = DXmChildren (p);
            for (i = 0; i < DXmNumChildren(p); i++)
            {
                if (XtIsSubclass(foo[i], (WidgetClass) clockwidgetclass))
                {
                    p = (Widget)foo[i];
                    return ((ClockWidget)p);
                }
            }
            return (NULL);
        }
        p = temp;
    }
    return ((ClockWidget) p);
}

/*
**++
**  ROUTINE NAME:
**	CreateDateTimeCS
**
**  FUNCTIONAL DESCRIPTION
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
int CreateDateTimeCS(mw, language, xm_format, tm, xm_str)
    ClockWidget mw;
    char *language;
    XmString xm_format;
    struct tm *tm;
    XmString *xm_str;
{
    char buf[2], date_time[256];
    int n;
    long byte_count, cvt_status;
    char *format;

    format = DXmCvtCStoFC(xm_format, &byte_count, &cvt_status);
    if ((byte_count == 0) || (cvt_status == DXmCvtStatusFail))
	return 0;

    n = 0;
    date_time[0] = '\0';
    while (format[n] != (int) NULL) {
	if (format[n] == '%') {
	    switch ((int) format[++n]) {
		case 'X':
		    XtFree(format);
		    format = XtMalloc(8);
		    strcpy(format, "%I:%M%p");
					/* US Time Format */
		    date_time[0] = '\0';
		    n = 0;
		    break;
		case 'a':
		    strcat(date_time, Days(mw)[tm->tm_wday].label);
		    n++;
		    break;
		case 'b':
		    strcat(date_time, Months(mw)[tm->tm_mon].label);
		    n++;
		    break;
		case 'd':
		    strcat(date_time, DayNumerals(mw)[tm->tm_mday - 1].label);
		    strcat(date_time, DaySuffix(mw).label);
		    n++;
		    break;
		case 'H':
		    strcat(date_time, HourNumerals(mw)[tm->tm_hour].label);
		    strcat(date_time, HourSuffix(mw).label);
		    n++;
		    break;
		case 'I':
		    if (tm->tm_hour == 12) {
			strcat(date_time, HourNumerals(mw)[12].label);
		    } else {
			strcat(date_time,
			  HourNumerals(mw)[tm->tm_hour % 12].label);
		    }
		    strcat(date_time, HourSuffix(mw).label);
		    n++;
		    break;
		case 'M':
		    strcat(date_time, MinNumerals(mw)[tm->tm_min].label);
		    strcat(date_time, MinSuffix(mw).label);
		    n++;
		    break;
		case 'p':
		    if (tm->tm_hour < 12)
			strcat(date_time, Amstr(mw).label);
		    else
			strcat(date_time, Pmstr(mw).label);
		    n++;
		    break;
		defaults:
		    n++;
		    break;
	    }
	} else {
	    buf[0] = format[n];
	    buf[1] = '\0';
	    strcat(date_time, buf);
	    n++;
	}
    }

    *xm_str = DXmCvtFCtoCS(date_time, &byte_count, &cvt_status);
    XtFree(format);
    return (byte_count);
}

/*
**++
**  ROUTINE NAME:
**	DrawAnalog (w, tm, gc)
**
**  FUNCTIONAL DESCRIPTION:
**  	draw the analog clock
**
**  FORMAL PARAMETERS:
**	w - the analog part widget
**	tm - the time to draw the hands at
**	gc - the GC to draw the hand in
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawAnalog(w, tm, gc)
    Widget w;
    struct tm tm;
    GC gc;
{
    Dimension radius, minute_hand_length, hour_hand_length, SmalerSide;
    Position centerX, centerY;
    XmDrawingAreaWidget mw = (XmDrawingAreaWidget) w;

    if (gc == 0)
	return;
    if (Width(mw) > Height(mw))
	SmalerSide = Height(mw);
    else
	SmalerSide = Width(mw);

    radius = Half(SmalerSide) - DEF_PADDING;
    minute_hand_length = ((MINUTE_HAND_FRACT * radius) / 100);
    hour_hand_length = ((HOUR_HAND_FRACT * radius) / 100);

    /* Redraw new hands.*/
    DrawHand(w, minute_hand_length, ((double) tm.tm_min) / 60.0, gc);

    DrawHand(w, hour_hand_length,
      ((((double) tm.tm_hour) + (((double) tm.tm_min) / 60.0)) / 12.0), gc);
}

/*
**++
**  ROUTINE NAME:
**	DrawHand(w, length, fraction_of_a_circle, gc)
**
**  FUNCTIONAL DESCRIPTION:
**      Draws a clock hand
**
**  FORMAL PARAMETERS:
**	w - the analog part widget
** 	length -- the maximum length of the hand.
** 	Fraction_of_a_circle -- a fraction between 0 and 1 (inclusive)
** 		indicating how far around the circle (clockwise) from high noon.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	hour_x1, hour_x2, hour_y1, hour_y2
**	minute_x1, minute_x2, minute_y1, minute_y2
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void DrawHand(w, length, fraction_of_a_circle, gc)
    Widget w;
    Dimension length;
    double fraction_of_a_circle;
    GC gc;
{
    double angle, cosangle, sinangle;
    Dimension radius, SmalerSide;
    Position centerX, centerY, x_offset, y_offset;
    XmDrawingAreaWidget mw = (XmDrawingAreaWidget) w;
    ClockWidget pw = (ClockWidget) Parent(mw);


    if (Width(mw) > Height(mw))
	SmalerSide = Height(mw);
    else
	SmalerSide = Width(mw);

    centerY = Half(Height(mw));
    centerX = Half(Width(mw));

    radius = Half(SmalerSide) - DEF_PADDING;

    /*  A full circle is 2 PI radians.  Angles are measured from 12 o'clock,
     * clockwise increasing.  Since in X, +x is to the right and +y is
     * downward: x = x0 + r * sin(theta)  y = y0 - r * cos(theta) */
    angle = TWOPI * fraction_of_a_circle;
    cosangle = cos(angle);
    sinangle = sin(angle);
#ifdef debug
    printf("len %d, cos %lf angle %lf \n", length, cosangle, angle);
#endif
    x_offset = (Position) round(length * sinangle);
    y_offset = (Position) round(length * cosangle);
    XDrawLine(Dpy(pw), Win(pw), gc, centerX, centerY, centerX + x_offset,
      centerY - y_offset);
}

/*
**++
**  ROUTINE NAME:
**	DrawClockFace (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Draw the clock face.  Draw 12 boxes
**  	around the clock face.  '12', '3', '6',
**  	and '9' are larger than the others.
**
**  FORMAL PARAMETERS:
**	w - the analog part widget
**
**  IMPLICIT INPUTS:
** 	radius and the analog_subwin structure
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawClockFace(w)
    Widget w;
{
    register int i;
    double angle, cosangle, sinangle;
    int tic_radius, tic_length, rec_count = 0, ac = 0;
    Arg al[5];
    Dimension radius, SmalerSide, sm_sq_size, lg_sq_size;
    Display *dpy;
    Position centerX, centerY;
    Pixel fgpixel, bgpixel;
    XGCValues gcv;
    XmDrawingAreaWidget mw = (XmDrawingAreaWidget) w;
    XRectangle rects[15];
    ClockWidget pw = (ClockWidget) Parent(mw);

    dpy = Dpy(pw);
    if (AnalogGC(pw) == 0) {

	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget)AnalogPart(pw), al, ac);

	gcv. foreground = fgpixel;
	gcv. background = bgpixel;

	AnalogGC(pw) =
	  XCreateGC(dpy, XtWindow(pw), GCForeground | GCBackground, &gcv);
	XSetLineAttributes(Dpy(pw), AnalogGC(pw), HandWidth(pw), LineSolid,
	  CapRound, JoinBevel);
	gcv. foreground = bgpixel;
	gcv. background = fgpixel;

	AnalogClearGC(pw) =
	  XCreateGC(dpy, XtWindow(pw), GCForeground | GCBackground, &gcv);
	XSetLineAttributes(Dpy(pw), AnalogClearGC(pw), HandWidth(pw),
	  LineSolid, CapRound, JoinBevel);
    }

    if (Width(mw) > Height(mw))
	SmalerSide = Height(mw);
    else
	SmalerSide = Width(mw);

    /* clear the analog window */
    /* XClearWindow (Dpy (pw), Win (pw)); */

    XFillRectangle(dpy, Win(pw), AnalogClearGC(pw), 0, 0, Width(mw),
      Height(mw));

    centerY = Height(mw) / 2;
    centerX = Width(mw) / 2;
    radius = SmalerSide / 2 - DEF_PADDING;

    tic_radius = radius - SmalerSide / BOX_FRACTION;
    tic_length = SmalerSide / BOX_FRACTION / 2;

    sm_sq_size = SmalerSide / BOX_FRACTION / 2;
    lg_sq_size = SmalerSide / BOX_FRACTION;

    if (sm_sq_size < 2)
	sm_sq_size = 2;

    /* if (lg_sq_size < 4) lg_sq_size = 4;*/

    /* draw the little tics */
    for (i = 0; i < 60; i = i + 5)
	if (i % 15 != 0) {
	    angle = TWOPI * ((double) i / 60.);
	    cosangle = cos(angle);
	    sinangle = sin(angle);

	    rects[rec_count].x = centerX + (int) (tic_radius * sinangle);
	    rects[rec_count].y = centerY + (int) (tic_radius * cosangle);
	    rects[rec_count].width = sm_sq_size;
	    rects[rec_count].height = sm_sq_size;
	    rec_count++;
	}

    /* draw the four big squares */
    rects[rec_count].x = centerX - tic_length;
    rects[rec_count].y = centerY - tic_radius - 2 * tic_length;
    rects[rec_count].width = lg_sq_size;
    rects[rec_count].height = lg_sq_size;
    rec_count++;

    rects[rec_count].x = centerX - tic_radius - 2 * tic_length,
      rects[rec_count].y = centerY - tic_length;
    rects[rec_count].width = lg_sq_size;
    rects[rec_count].height = lg_sq_size;
    rec_count++;

    rects[rec_count].x = centerX + tic_radius;
    rects[rec_count].y = centerY - tic_length;
    rects[rec_count].width = lg_sq_size;
    rects[rec_count].height = lg_sq_size;
    rec_count++;

    rects[rec_count].x = centerX - tic_length;
    rects[rec_count].y = centerY + tic_radius;
    rects[rec_count].width = lg_sq_size;
    rects[rec_count].height = lg_sq_size;
    rec_count++;

    XFillRectangles(dpy, Win(pw), AnalogGC(pw), rects, rec_count);
}

/*
**++
**  ROUTINE NAME:
**	DrawDigital (w, tm)
**
**  FUNCTIONAL DESCRIPTION:
**  	Updates the current Digital time
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**	tm - the previous time for comparaison
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawDigital(w, tm)
    Widget w;
    struct tm tm;
{
    char *str;
    int len, ac = 0;
    long byte_count, cvt_status;
    int direction = 0, ascent = 0, descent = 0;
    Arg al[5];
    ClockWidget mw = (ClockWidget) w;
    Dimension width, height;
    Display *dpy;
    Pixel fgpixel, bgpixel;
    Position x, y;
    Window win;
    XCharStruct overall;
    XGCValues gcv;
    XmFontList FontList;
    XmFontContext context;
    XmString xm_digital;
    XmStringCharSet charset;

    if (!XtIsRealized(DigitalPart(mw)))
	return;

    dpy = Dpy(mw);
    if (DigitalGC(mw) == 0) {

	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) DigitalPart(mw), al, ac);

	gcv. foreground = bgpixel;
	gcv. background = fgpixel;

	DigitalClearGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	gcv. foreground = fgpixel;
	gcv. background = bgpixel;

	/* Get the toolkits default font. */
	FontList = mw->clock.clock_font;

	if (NumFonts(mw) == 0) {
	    XmFontListInitFontContext(&context, FontList);
	    XmFontListGetNextFont(context, &charset, &DigitalFont(mw));
	    XtFree(charset);
	    XmFontListFreeFontContext(context);

	    /* This was replaced by the above lines. */
	    /* DigitalFont (mw) = FontList -> font; */
	    DigitalFontHeight(mw) = (DigitalFont(mw)->max_bounds.ascent +
	      DigitalFont(mw)->max_bounds.descent);
	}

	gcv. font = DigitalFont(mw)->fid;

	DigitalGC(mw) = XCreateGC(dpy, XtWindow(mw),
	  GCForeground | GCBackground | GCFont, &gcv);

    }

    str = DigPtr(mw);
    len = strlen(str);
    win = XtWindow(DigitalPart(mw));
    height = XtHeight(DigitalPart(mw));
    width = Width(DigitalPart(mw));
/* jv - ALWAYS clear */
#ifdef JV
    /* clear the digital window if going from 2 digits to 1 */
    if (((tm.tm_hour == 12) && (tm.tm_min == 59)) ||
      ((tm.tm_hour == 0) && (tm.tm_min == 59)))
#endif
	XFillRectangle(dpy, win, DigitalClearGC(mw), 0, 0, width, height);

    if (!IsAsianLocale(Language(mw))) {
	XTextExtents(DigitalFont(mw), str, len, &direction, &ascent, &descent,
	  &overall);

	x = (width - overall.width) / 2;
	y = height - ((height - ascent + descent) / 2);
	if (x < 0)
	    x = 0;
	if (y < 0)
	    y = 0;
	XDrawString(dpy, win, DigitalGC(mw), x, y, str, len);

    } else {
	xm_digital = DXmCvtFCtoCS(str, &byte_count, &cvt_status);

	x = 0;
	y = (height - XmStringHeight(DigitalFontList(mw), xm_digital)) / 2;
	if (y < 0)
	    y = 0;

	XmStringDraw(dpy, win, DigitalFontList(mw), xm_digital, DigitalGC(mw),
	  x, y, width, XmALIGNMENT_CENTER, XmSTRING_DIRECTION_L_TO_R, NULL);

	XtFree((char *)xm_digital);
    }
}

/*
**++
**  ROUTINE NAME:
**	DrawDate (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Updates the current Date
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawDate(w)
    Widget w;
{
    char str[30];
    int len, ac = 0;
    long byte_count, cvt_status;
    int direction = 0, ascent = 0, descent = 0;
    Arg al[5];
    ClockWidget mw = (ClockWidget) w;
    Dimension width, height, height_factor;
    Display *dpy;
    Pixel fgpixel, bgpixel;
    Position x, y;
    Window win;
    XmFontList FontList;
    XmFontContext context;
    XmString xm_date;
    XmStringCharSet charset;
    XCharStruct overall;
    XGCValues gcv;

    if (!XtIsRealized(DigitalPart(mw)))
	return;

    dpy = Dpy(mw);
    if (DateGC(mw) == 0) {

	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) DatePart(mw), al, ac);

	gcv. foreground = bgpixel;
	gcv. background = fgpixel;

	DateClearGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	gcv. foreground = fgpixel;
	gcv. background = bgpixel;

	/* Get the toolkits default font. */
	FontList = mw->clock.clock_font;

	if (NumFonts(mw) == 0) {
	    XmFontListInitFontContext(&context, FontList);
	    XmFontListGetNextFont(context, &charset, &DateFont(mw));
	    XtFree(charset);
	    XmFontListFreeFontContext(context);

	    /* This was replaced by the above lines. */
	    /* DateFont (mw) = FontList -> font;*/

	    DateFontHeight(mw) = (DateFont(mw)->max_bounds.ascent +
	      DateFont(mw)->max_bounds.descent);
	}

	gcv. font = DateFont(mw)->fid;

	DateGC(mw) = XCreateGC(dpy, XtWindow(mw),
	  GCForeground | GCBackground | GCFont, &gcv);
    }

    /* clear the date window */
    win = XtWindow(DatePart(mw));
    height = XtHeight(DatePart(mw));
    width = Width(DatePart(mw));

    XFillRectangle(dpy, win, DateClearGC(mw), 0, 0, width, height);

    if (!IsAsianLocale(Language(mw))) {
	len = strlen(DatePtr(mw));
	XTextExtents(DateFont(mw), DatePtr(mw), len, &direction, &ascent,
	  &descent, &overall);
	height_factor = height - ((height - ascent + descent) / 2);

	x = (width - overall.width) / 2;
	y = height_factor;
	XDrawString(dpy, win, DateGC(mw), x, y, DatePtr(mw), len);

    } else {
	xm_date = DXmCvtFCtoCS(DatePtr(mw), &byte_count, &cvt_status);

	x = 0;
	y = (height - XmStringHeight(DateFontList(mw), xm_date)) / 2;

	XmStringDraw(dpy, win, DateFontList(mw), xm_date, DateGC(mw), x, y,
	  width, XmALIGNMENT_CENTER, XmSTRING_DIRECTION_L_TO_R, NULL);

	XtFree((char *)xm_date);
    }

#ifdef NO_USE
    str = NumPtr(mw);
    len = strlen(str);
    XTextExtents(DateFont(mw), str, len, &direction, &ascent, &descent,
      &overall);
    x = (width - overall.width) / 2;
    y = height_factor + height / 3;
    XDrawString(dpy, win, DateGC(mw), x, y, str, len);

    str = DayPtr(mw);
    len = strlen(str);
    XTextExtents(DateFont(mw), str, len, &direction, &ascent, &descent,
      &overall);
    x = (width - overall.width) / 2;
    y = height_factor + 2 * (height / 3);
    XDrawString(dpy, win, DateGC(mw), x, y, str, len);
#endif
}

/*
**++
**  ROUTINE NAME:
**	DrawBell (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Redraws the Bell icon
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawBell(w)
    Widget w;
{
    int type, ac = 0;
    Arg al[5];
    ClockWidget mw = (ClockWidget) w;
    Display *dpy;
    Pixel fgpixel, bgpixel;
    XGCValues gcv;

    dpy = Dpy(mw);
    type = ClockType(mw);

    if (BellGC(mw) == 0) {

	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) mw, al, ac);

	gcv. foreground = fgpixel;
	gcv. background = bgpixel;

	BellGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	BellPixmap(mw) = XCreateBitmapFromData(dpy, XtWindow(mw), bell_bits,
	  bell_width, bell_height);
	if (BellPixmap(mw) == 0)
	    clock_display_message(mw, NoBellPixmap, WARNING);
    }

    if (type == Digital) {
	ac = 0;
	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) DigitalPart(mw), al, ac);
    } else if ((type == Date) || (type == DigitalDate) ||
      (type == DigitalDateAnalog)) {
	ac = 0;
	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) DatePart(mw), al, ac);
    } else {
	ac = 0;
	XtSetArg(al[ac], XmNforeground, &fgpixel);
	ac++;
	XtSetArg(al[ac], XmNbackground, &bgpixel);
	ac++;
	XtGetValues((Widget) AnalogPart(mw), al, ac);
    }

    XSetForeground(dpy, BellGC(mw), fgpixel);
    XSetBackground(dpy, BellGC(mw), bgpixel);

    XCopyPlane(Dpy(mw), BellPixmap(mw), XtWindow(AlarmBell(mw)), BellGC(mw), 0,
      0, bell_width, bell_height, 0, 0, 1);
}

/*
**++
**  ROUTINE NAME:
**	round(x)
**
**  FUNCTIONAL DESCRIPTION:
**  	return the rounded off value of x
**
**  FORMAL PARAMETERS:
**	double x- number to round off
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static int round(x)
    double x;

{
    return (x >= 0 ? (int) (x + .5) : (int) (x - .5));
}

/*
**++
**  ROUTINE NAME:
**	clock_tic(w)
**
**  FUNCTIONAL DESCRIPTION:
**  	another second has gone by.
** 	update global vars and redraw windows
**
**  FORMAL PARAMETERS:
**	w - the analog part widget
**
**  IMPLICIT INPUTS:
** 	hour_x1, hour_x2, hour_y1, hour_y2
**	minute_x1, minute_x2, minute_y1, minute_y2
**	alarm_on, cur_tod, alarm_min, alarm_hr
**	on_the_minute
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
XtTimerCallbackProc clock_tic(w, otimer)
    Opaque w;
    XtIntervalId otimer;
{
    int i, j;
    int cur_tod;
    int alarm_hr, alarm_min, alarm_tod;
    int update;
    long time_value;
    struct tm tm;			/* current time */
    struct tm *localtime();
    Boolean date_changed;
    ClockWidget mw = (ClockWidget) w;
    XtIntervalId timer;


    (void) time(&time_value);		/* get the time */
    tm = *localtime(&time_value);	/* converts time to a tm struct
					   to contain sec, min, hr etc */

    /* schedule the next timeout to occur at "update" seconds */
    update = 60 - tm.tm_sec;
    timer =
      XtAppAddTimeOut(app_context, update * 1000, 
		(XtTimerCallbackProc)clock_tic, (Opaque) mw);

    /* set up time-of-day vars */
    if (tm.tm_hour > 11)
	cur_tod = PM;
    else
	cur_tod = AM;

    if (tm.tm_mday == CurrentDay(mw))
	date_changed = FALSE;
    else
	date_changed = TRUE;

    /* Don't need to call this if Date and Digital time are not selected. */
    if ((DigitalPresent(mw) || DatePresent(mw)) != 0)
	set_time_strings(mw, tm, date_changed, FALSE);

    if (AnalogPresent(mw)) {
	DrawAnalog(AnalogPart(mw), LastTm(mw), AnalogClearGC(mw));
	DrawAnalog(AnalogPart(mw), tm, AnalogGC(mw));
    }

    if (DigitalPresent(mw))
	DrawDigital(mw, LastTm(mw));

    if (DatePresent(mw) && date_changed)
	DrawDate(mw);

    /* check for alarm */ 
    if (AlarmOn(mw)) 
    {			   
        /* if alarm enabled, map the system time to our cannonical 
	 * form - a number between 1 and 12 - in order to compare 
	 * against the user specified alarm time 
	 */
	if (tm.tm_hour > 11)	   
	    tm.tm_hour -= 12;

        /* if time is really midnight 12 AM or noon 12 PM,
         * then represent it that way */
        if (tm.tm_hour == 0)
            tm.tm_hour = 12;

	alarm_hr = atoi(AlarmHr(mw));
	alarm_min = atoi(AlarmMin(mw));
	
	/* if in 24 hour mode, derive AM/PM flag from alarm time;
	 * else, obtain PM flag from toggle button
	 */
	if (MilitaryTime(mw))	/* 24 hour mode */
	{
	    if (alarm_hr > 11)	/* starting at 12 noon, turn PM flag on */
	    	alarm_tod = PM;  
	    else
		alarm_tod = AM;
	}
	else			/* 12 hour mode */
	    alarm_tod = AlarmPm(mw);	/* get PM toggle button state */

        /* Map The user specified alarm hour to cannonical form -
	 * number between 1 and 12 - to compare against the converted 
	 * system time
	 */
	if (alarm_hr > 12)	
	    alarm_hr -= 12;

        /* if it's midnight 12 AM, use the same representation
           as the system time
         */
        if (alarm_hr == 0)
                alarm_hr = 12;

	/* if system time == alarm time, and system vs. alarm AM/PM flag 
	 * are the same , set off the alarm 
	 */
	if ((tm.tm_hour == alarm_hr) && (tm.tm_min == alarm_min) &&
	  (cur_tod == alarm_tod))
	    set_off_alarm(mw);
    }	/* end check for alarm */

    LastTm(mw) = tm;
}

/*
**++
**  ROUTINE NAME:
**	set_time_strings ()
**
**  FUNCTIONAL DESCRIPTION:
**  	change the date month string and day
**  	string to the upper-case equivilents
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void set_time_strings(w, tm, date_changed, military_time_changed)
    Widget w;
    struct tm tm;
    Boolean date_changed, military_time_changed;

{
    char *time_ptr, tod_string[3], *status_str, *xy;
    int i, j, cur_tod, date_length;
    long byte_count, cvt_status;
    int direction = 0, ascent = 0, descent = 0;
    Boolean dont_care_sep, change_width = FALSE;
    Cardinal status;
    ClockWidget mw = (ClockWidget) w;
    XmString date_string, xm_longest_time, xm_longest_date;
    XmStringCharSet char_set;
    XmStringContext context;
    XmStringDirection dont_care_direction;
    XCharStruct overall;

    if (IsAsianLocale(Language(mw))) {
	if (MilitaryTime(mw))
	    date_length = CreateDateTimeCS(mw, Language(mw),
	      CSMilitaryFormat(mw), &tm, &date_string);
	else
	    date_length = CreateDateTimeCS(mw, Language(mw), CSTimeFormat(mw),
	      &tm, &date_string);
    } else {
#ifndef NO_XNLS
	if (MilitaryTime(mw))
	    date_length = xnl_strftime(LanguageId(mw), &date_string,
	      CSMilitaryFormat(mw), &tm);
	else
	    date_length = xnl_strftime(LanguageId(mw), &date_string,
	      CSTimeFormat(mw), &tm);
#else 
	if (MilitaryTime(mw))
	    date_length = CreateDateTimeCS(mw, Language(mw),
	      CSMilitaryFormat(mw), &tm, &date_string);
	else
	    date_length = CreateDateTimeCS(mw, Language(mw), 
	      CSTimeFormat(mw), &tm, &date_string);
#endif
    }

    if (date_length != 0) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * Original code cannot correctly handle RtoL segment.
	 */
        xy = DXmCvtCStoFC(date_string, &byte_count, &cvt_status);
	strcpy(DigPtr(mw), xy);
	XtFree(xy);
#else
	strcpy(DigPtr(mw), "");
	status = XmStringInitContext(&context, date_string);
	while (status) {
	    status = XmStringGetNextSegment(context, &xy, &char_set,
	      &dont_care_direction, &dont_care_sep);
	    if (status) {
		strcat(DigPtr(mw), xy);
		XtFree(xy);
		XtFree(char_set);
	    }
	}
#endif /* I18N_BUG_FIX */

	/* If military time button has been toggled, update LongestTime() and
	 * change text with array. */
	if (military_time_changed) {
	    strcpy(LongestTime(mw), DigPtr(mw));
	    change_width = TRUE;
	}
#ifdef I18N_BUG_FIX
#else
	XmStringFreeContext(context);
#endif /* I18N_BUG_FIX */
	XmStringFree(date_string);	/* Free up memory allocated by XNLS */
    }


    if (date_changed) {
	if (IsAsianLocale(Language(mw)))
	    date_length = CreateDateTimeCS(mw, Language(mw), CSDateFormat(mw),
	      &tm, &date_string);
	else
#ifndef NO_XNLS
	    date_length = xnl_strftime(LanguageId(mw), &date_string,
	      CSDateFormat(mw), &tm);
#else
	    date_length = CreateDateTimeCS(mw, Language(mw), CSDateFormat(mw),
	      &tm, &date_string);
#endif

	if (date_length != 0) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * Original code cannot correctly handle RtoL segment.
	 */
            xy = DXmCvtCStoFC(date_string, &byte_count, &cvt_status);
	    strcpy(DatePtr(mw), xy);
	    XtFree(xy);
#else
	    strcpy(DatePtr(mw), "");
	    status = XmStringInitContext(&context, date_string);
	    while (status) {
		status = XmStringGetNextSegment(context, &xy, &char_set,
		  &dont_care_direction, &dont_care_sep);
		if (status) {
		    strcat(DatePtr(mw), xy);
		    XtFree(xy);
		    XtFree(char_set);
		}
	    }
	    XmStringFreeContext(context);
#endif /* I18N_BUG_FIX */
	    XmStringFree(date_string);	/* Free up memory allocated by XNLS */
	}

	/* If date has changed, update the LongestDate(mw). */
	strcpy(LongestDate(mw), DatePtr(mw));
	change_width = TRUE;
	CurrentDay(mw) = tm.tm_mday;
    }

    /* There is a change in the width of the Date or Digital Text, readjust the
     * FontWidths[] array and  then call Layout() to select the a new size
     * based on the new width */
    if (change_width) {
	if (!IsAsianLocale(Language(mw))) {
	    for (i = 0; i < NumFonts(mw); i++) {
		XTextExtents(Fonts(mw)[i], LongestTime(mw), strlen(
		  LongestTime(mw)), &direction, &ascent, &descent, &overall);
		DigitalFontWidths(mw)[i] = overall.width;
		XTextExtents(Fonts(mw)[i], LongestDate(mw), strlen(
		  LongestDate(mw)), &direction, &ascent, &descent, &overall);
		DateFontWidths(mw)[i] = overall.width;
	    }
	} else {
	    xm_longest_time =
	      DXmCvtFCtoCS(LongestTime(mw), &byte_count, &cvt_status);
	    xm_longest_date =
	      DXmCvtFCtoCS(LongestDate(mw), &byte_count, &cvt_status);

	    for (i = 0; i < NumFonts(mw); i++) {
		DigitalFontWidths(mw)[i] =
		  XmStringWidth(DigitalFontLists(mw)[i], xm_longest_time);
		DateFontWidths(mw)[i] =
		  XmStringWidth(DateFontLists(mw)[i], xm_longest_date);
	    }
	    XmStringFree(xm_longest_time);
	    XmStringFree(xm_longest_date);
	}
	Layout(mw);
    }
}

/*
**++
**  ROUTINE NAME:
**	strip_leading_zero(str_in)
**
**  FUNCTIONAL DESCRIPTION:
**  	if the passed in string begins with a
**  	zero, remove it
**
**  FORMAL PARAMETERS:
**	char *str_in - the string to strip of a leading zero
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
strip_leading_zero(str_in)
    char *str_in;
{
    int i, old_len;
    char cur_char;

    if (str_in[0] == '0') {
	old_len = strlen(str_in);
	for (i = 0; i < old_len; i++)
	    str_in[i] = str_in[i + 1];
    }
}

/*
**++
**  ROUTINE NAME:
**	DetermineClockType (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Sets the clock type based on which parts are on
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**	ClockType
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DetermineClockType(w)
    Widget w;
{
    int AP, DP, NP, manage_count = 0;
    ClockWidget mw = (ClockWidget) w;
    Widget manage_set[10];

    AP = AnalogPresent(mw);
    DP = DatePresent(mw);
    NP = DigitalPresent(mw);

    if ((AP) && (DP) && (NP))
	ClockType(mw) = DigitalDateAnalog;

    if ((AP) && (DP) && (!NP))
	ClockType(mw) = DateAnalog;

    if ((AP) && (!DP) && (NP))
	ClockType(mw) = DigitalAnalog;

    if ((!AP) && (DP) && (NP))
	ClockType(mw) = DigitalDate;

    if ((AP) && (!DP) && (!NP))
	ClockType(mw) = Analog;

    if ((!AP) && (DP) && (!NP))
	ClockType(mw) = Date;

    if ((!AP) && (!DP) && (NP))
	ClockType(mw) = Digital;

    /* Special CASE: if all are off, turn them all on! */
    if ((!AP) && (!DP) && (!NP)) {

	AnalogPresent(mw) = TRUE;
	DigitalPresent(mw) = TRUE;
	DatePresent(mw) = TRUE;

	ClockType(mw) = DigitalDateAnalog;

	manage_set[manage_count] = (Widget) AnalogPart(mw);
	manage_count++;
	manage_set[manage_count] = (Widget) DigitalPart(mw);
	manage_count++;
	manage_set[manage_count] = (Widget) DatePart(mw);
	manage_count++;

	if (manage_count)
	    XtManageChildren(manage_set, manage_count);
    }
}

/*
**++
**  ROUTINE NAME:
**	set_off_alarm ()
**
**  FUNCTIONAL DESCRIPTION:
**  	alarm time has been reached, beep
**  	3 times and map notify window
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void set_off_alarm(w)
    Widget w;
{
    int i, ac = 0;
    long byte_count, cvt_status;
    Arg al[5];
    Atom wm_protocols_atom, wm_take_focus_atom;
    ClockWidget mw = (ClockWidget) w;
    Display *display;
    XmString CS;
    XtGeometryResult geo_result;
    XtWidgetGeometry request, reply;

    if (!RepeatOn(mw)) {
	AlarmOn(mw) = FALSE;
	if (AlarmWid(mw) != NULL)
	    XmToggleButtonSetState(AlarmWid(mw), AlarmOn(mw), TRUE);
    }

    CS = DXmCvtFCtoCS(AlarmMes(mw), &byte_count, &cvt_status);

    XtSetArg(al[ac], XmNmessageString, CS);
    ac++;
    XtSetValues((Widget) MessageWid(mw), al, ac);
    XtFree((char *)CS);

    /* Don't allow alarm box to take focus */
    ac = 0;
    XtSetArg(al[ac], XmNinput, False);
    ac++;
    XtSetValues(XtParent(MessageWid(mw)), al, ac);
    display = XtDisplay(XtParent(mw));
    wm_take_focus_atom = XmInternAtom(display, "WM_TAKE_FOCUS", False);
    XmDeactivateWMProtocol(XtParent(MessageWid(mw)), wm_take_focus_atom);

    XtManageChild((Widget) MessageWid(mw));
    if (!RepeatOn(mw))
	XtUnmanageChild((Widget) AlarmBell(mw));

    /* bring message window to top of stack */
/*  this has been commented out because if session is paused
**  then the alarm message can be seen, and it shouldn't

    request. request_mode = CWStackMode;
    request. stack_mode = Above;
    geo_result = XtMakeGeometryRequest(MessageWid(mw), &request, &reply);
*/
    if (BellOn(mw)) {
	for (i = 1; i < 4; i++)
	    XBell(Dpy(mw), 0);
    }
  if (!NO_SOUND)
  {
    if (AudioOn(mw) && AudioCapable(mw) && !AudioAsyncActive(mw))
	audio_play_async(mw, SpeakerOn(mw), AudioVolume(mw), AudioFname(mw));
  }
}

/*
**++
**  ROUTINE NAME:
**	get_audio_hardware ()
**
**  FUNCTIONAL DESCRIPTION:
**  	see if display is audio capable
**	set AudioCapable if so.   	
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void get_audio_hardware(mw)
    ClockWidget mw;
{
#ifndef NO_AUDIO
    diva_status status;
#endif

    AudioCapable(mw) = FALSE;
#ifndef NO_AUDIO
    AudioHardware(mw) = diva_c_any_hw;

    status = diva_assign(&AudioChannel(mw), AudioHardware(mw), (char *)NULL, (char *)NULL);
    if (status == diva_s_success) {
	AudioCapable(mw) = TRUE;
#ifdef NOT_WORKING /* acc vio's, DIVA bug */
        status = diva_deassign(&AudioChannel(mw));
        if (status != diva_s_success)
#ifdef AUDIO_DEBUG
	    printf("diva_deassign failed, Status: %d\n",status);
#endif
#endif /* NOT_WORKING */
    } 

#ifdef AUDIO_DEBUG
    else {
	printf("diva_assign failed, Status: %d\n",status);
    }
#endif /* AUDIO_DEBUG */
#endif /* NO_AUDIO */
}

/*
**++
**  ROUTINE NAME:
**	audio_set_insensitive ()
**
**  FUNCTIONAL DESCRIPTION:
**  	Sensitize the audio button
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_set_insensitive(mw)
    ClockWidget mw;
{
    Boolean sensitivity;

    sensitivity = FALSE;
    if (!AudioCapable(mw))
	XtSetSensitive((Widget) AudioOnWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioOutputMenuWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioVolumeWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioFnameButtonWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioFnameTextWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioPlayButtonWid(mw), sensitivity);
    XtSetSensitive((Widget) AudioStopButtonWid(mw), sensitivity);
}

/*
**++
**  ROUTINE NAME:
**	audio_button_pressed ()
**
**  FUNCTIONAL DESCRIPTION:
**  	Sensitize the rest of audio widget parts
**	based on whether the audio button is active
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_button_pressed(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int reason;
{
  if (!NO_SOUND)
  {
    ClockWidget mw;
    Boolean sensitivity;

    mw = clock_FindMainWidget(w);

    sensitivity = XmToggleButtonGetState(AudioOnWid(mw));
    if (!AudioAsyncActive(mw))
	XtSetSensitive((Widget)AudioPlayButtonWid(mw), sensitivity);
    XtSetSensitive((Widget)AudioOutputMenuWid(mw), sensitivity);
    XtSetSensitive((Widget)AudioVolumeWid(mw), sensitivity);
    XtSetSensitive((Widget)AudioFnameButtonWid(mw), sensitivity);
    XtSetSensitive((Widget)AudioFnameTextWid(mw), sensitivity);
  }
}

/*
**++
**  ROUTINE NAME:
**	audio_play_async ()
**
**  FUNCTIONAL DESCRIPTION:
**  	Play file async
**	   	
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_play_async(mw, speaker, volume, filename)
    ClockWidget mw;
    int speaker;
    int volume;
    char *filename;
{
#ifndef NO_AUDIO
    diva_msg_handle msg_hdl = NULL;
    diva_status status;
    unsigned long audio_output;

#ifdef AUDIO_DEBUG
	printf("start audio_play_async\n");
#endif
	if (!AudioAsyncActive(mw)) {
	    AudioAsyncActive(mw) = TRUE;
	    if (AlarmSettings(mw) != NULL) {
		XtSetSensitive(AudioStopButtonWid(mw), TRUE);
		XtSetSensitive(AudioPlayButtonWid(mw), FALSE);
	    }
	}

	status = diva_assign(&AudioChannel(mw), AudioHardware(mw), (char *)NULL, (char *)NULL);
	if (status == diva_s_success) {
	    status = diva_load_from_file(
			AudioChannel(mw),
			&msg_hdl,
			filename,
			0, 0, 0, 0, 0);
	    if (status == diva_s_success) {
		status = diva_register_async(
				AudioChannel(mw), 
				diva_c_play, 
				play_completion_handler, 
				(diva_opaque) mw);
#ifdef AUDIO_DEBUG
		if (status != diva_s_success)
		    printf("Could not register diva_c_play as asynchronous, Status: %d\n", status);
#endif
		if (speaker)
		    audio_output = diva_c_output_speaker;
		else 
		    audio_output = diva_c_output_headphones;

		diva_set_dev_attribute(AudioChannel(mw),
			   diva_c_output_channel, audio_output);
		diva_set_dev_attribute(AudioChannel(mw),
			   diva_c_play_volume, volume);
		status = diva_play(AudioChannel(mw), 
				&diva_context, msg_hdl);
		if (status != diva_s_success) {
			/* start can't play file */
		    display_error_message(mw, "Audio_Play_Failure");
#ifdef AUDIO_DEBUG
		    printf("diva_play failed, Status: %d\n", status);
#endif
		    status = diva_deassign(&AudioChannel(mw));
#ifdef AUDIO_DEBUG
		    if (status != diva_s_success) 
			printf("diva_deassign failed, Status: %d\n", AudioFname(mw), status);
#endif
		    if (AlarmSettings(mw) != NULL) {
			XtSetSensitive(AudioStopButtonWid(mw), FALSE);
			if (XmToggleButtonGetState(AudioOnWid(mw))) {
			    XtSetSensitive(AudioPlayButtonWid(mw), TRUE);
			    XmProcessTraversal(AudioPlayButtonWid(mw), XmTRAVERSE_CURRENT); 
			}
		    }
		    AudioAsyncActive(mw) = FALSE;
		} 	/* end can't play file */
	    } else {	/* start can't load file */
		display_error_message(mw, "Audio_Filename_Bad");
#ifdef AUDIO_DEBUG
		printf("Could not load audio file: %s, Status: %d\n", AudioFname(mw), status);
#endif
		status = diva_deassign(&AudioChannel(mw));
#ifdef AUDIO_DEBUG
		if (status != diva_s_success)
		    printf("diva_deassign failed, Status: %d\n", AudioFname(mw), status);
#endif
		if (AlarmSettings(mw) != NULL) {
		    XtSetSensitive(AudioStopButtonWid(mw), FALSE);
		    if (XmToggleButtonGetState(AudioOnWid(mw))) {
			XtSetSensitive(AudioPlayButtonWid(mw), TRUE);
			XmProcessTraversal(AudioPlayButtonWid(mw), XmTRAVERSE_CURRENT); 
		    }
		}
		AudioAsyncActive(mw) = FALSE;
	    }		/* end can't load file */
	} else { 	/* start bad channel */
	    display_error_message(mw, "Audio_Channel_Failure");
#ifdef AUDIO_DEBUG
	    printf("Could not assign audio channel, Status: %d\n",status);
#endif
	    AudioAsyncActive(mw) = FALSE;
	} 		/* end bad channel */
#ifdef AUDIO_DEBUG
	printf("end audio_play_async\n");
#endif
#endif /* NO_AUDIO */
} /* end routine */

/*
**++
**  ROUTINE NAME:
**	audio_play ()
**
**  FUNCTIONAL DESCRIPTION:
**  	Play the current file 
**	   	
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_play(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
  if (!NO_SOUND)
  {
    char *fname_str;
    int audio, bell, speaker, volume;
    long byte_count, cvt_status;
    XmString xm_fname_str;
    ClockWidget mw;
    Widget audio_menu_widget;

    mw = clock_FindMainWidget(w);

    if (AudioAsyncActive(mw))
	return;

    /* Lock out any more play requests */
    AudioAsyncActive(mw) = TRUE;
    if (AlarmSettings(mw) != NULL) {
	XtSetSensitive((Widget)AudioStopButtonWid(mw), TRUE);
	XtSetSensitive((Widget)AudioPlayButtonWid(mw), FALSE);
    }

    xm_fname_str = DXmCSTextGetString(AudioFnameTextWid(mw));
    fname_str = DXmCvtCStoFC(xm_fname_str, &byte_count, &cvt_status);
    XtFree((char *)xm_fname_str);

    bell = XmToggleButtonGetState(BellOnWid(mw));
    audio = XmToggleButtonGetState(AudioOnWid(mw));
    XmScaleGetValue(AudioVolumeWid(mw), &volume);

    XtVaGetValues (
      (Widget)AudioOutputMenuWid(mw), XmNmenuHistory, &audio_menu_widget, NULL);
    if (audio_menu_widget == (Widget) SpeakerOnWid(mw))
	speaker = TRUE;
    else
	speaker = FALSE;

    audio_play_async(mw, speaker, volume, fname_str);
  }
}

/*
**++
**  ROUTINE NAME:
**	audio_stop ()
**
**  FUNCTIONAL DESCRIPTION:
**  	Stop the current file 
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_stop(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
  if (!NO_SOUND)
  {
    ClockWidget mw;
#ifndef NO_AUDIO
    diva_status status;
#endif

    mw = clock_FindMainWidget(w);

    if (!AudioAsyncActive(mw))
	return;

#ifndef NO_AUDIO
    status = diva_stop_play(AudioChannel(mw), &diva_context);
#endif
#ifdef AUDIO_DEBUG
    if (status != diva_s_success)
	printf("diva_stop_play failed, Status: %d\n",status);
#endif
    XtSetSensitive((Widget)AudioStopButtonWid(mw), FALSE);
    if (XmToggleButtonGetState(AudioOnWid(mw))) {
	XtSetSensitive((Widget)AudioPlayButtonWid(mw), TRUE);
        XmProcessTraversal(AudioPlayButtonWid(mw), XmTRAVERSE_CURRENT);
    }
    /* Release lock */
    AudioAsyncActive(mw) = FALSE;
  }
}

/*
**++
**  ROUTINE NAME:
**	audio_set_volume ()
**
**  FUNCTIONAL DESCRIPTION:
**  	set the volume 
**	   	
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void audio_set_volume(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
  if (!NO_SOUND)
  {
    int volume;
#ifndef NO_AUDIO
    diva_status status;
#endif
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XmScaleGetValue(AudioVolumeWid(mw), &volume);
    if (AudioAsyncActive(mw)) {
#ifndef  NO_AUDIO
        status = diva_set_dev_attribute(AudioChannel(mw),
		diva_c_play_volume, volume);
#endif
#ifdef AUDIO_DEBUG
	if (status != diva_s_success)
	    printf("diva_set_volume failed, Status: %d\n",status);
#endif
    }
  }
}    

/*
**++
**  ROUTINE NAME: play_completion_handler
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure gets called with the status when an 
**	asynchronous play completes from DIVA.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
#ifndef NO_AUDIO
void play_completion_handler(completion_param)
    diva_opaque *completion_param;
{
    diva_status status;
    ClockWidget mw = (ClockWidget) completion_param;

    status = diva_deassign(&AudioChannel(mw));
#ifdef AUDIO_DEBUG
    if (status != diva_s_success)
	printf("diva_deassign failed, Status: %d\n", status);
#endif
    if (AlarmSettings(mw) != NULL) {
        XtSetSensitive(AudioStopButtonWid(mw), FALSE);
	if (XmToggleButtonGetState(AudioOnWid(mw))) {
	    XtSetSensitive(AudioPlayButtonWid(mw), TRUE);
            XmProcessTraversal(AudioPlayButtonWid(mw), XmTRAVERSE_CURRENT);
	}
    }
    AudioAsyncActive(mw) = FALSE;
}
#endif /* NO_AUDIO */

/*
**++
**  ROUTINE NAME: display_error_message
**
**  FUNCTIONAL DESCRIPTION:
**      This procedure creates a message dialog box. It takes the message as
**      a parameter, creates and maps the dialog box and then waits until
**      the user hits the continue button before allowing the program to
**      continue.
**
**  FORMAL PARAMETERS:
**              message - pointer to the message to be displayed
**              parentwindow - window in which this dialog box
**                      should be centered
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void display_error_message(mw, message)
    ClockWidget mw;
    char *message;
{
    Arg message_args[1];

  if (!NO_SOUND)
  {
    if (AlarmSettings(mw) != NULL) {
	if (XmToggleButtonGetState(AudioOnWid(mw)))
	    if (!XtIsSensitive(AudioPlayButtonWid(mw)))
		XtSetSensitive((Widget)AudioPlayButtonWid(mw), TRUE);
	if (XtIsSensitive(AudioStopButtonWid(mw)))
	    XtSetSensitive((Widget)AudioStopButtonWid(mw), FALSE);
    }
    AudioAsyncActive(mw) = FALSE;
  }
    XtSetArg(message_args[0], XmNmessageString, message);
    MrmFetchSetValues(ClockHierarchy(mw), ErrorMessageWid(mw), message_args,
      1);
    XtManageChild((Widget) ErrorMessageWid(mw));
}

/*
**++
**  ROUTINE NAME:
**	update_mode (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Stores the current parameter values based on settings.
**	Makes sure the proper parts of the clock are visible
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
** 	Settings widgets values
**
**  IMPLICIT OUTPUTS:
**	All local settings
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Visible parts of the clock may change
**--
**/
void update_mode(w)
    Widget w;
{
    int NewAnalog, NewDigital, NewDate, NewMilitaryTime, NewMenubar;
    Boolean no_display_selected = FALSE;
    ClockWidget mw = (ClockWidget) w;

    NewAnalog = XmToggleButtonGetState(AnalogWid(mw));
    NewDigital = XmToggleButtonGetState(DigitalWid(mw));
    NewDate = XmToggleButtonGetState(DateWid(mw));
    NewMilitaryTime = XmToggleButtonGetState(MilitaryTimeWid(mw));
    NewMenubar = XmToggleButtonGetState(MenubarWid(mw));

    if (NewAnalog || NewDigital || NewDate) {
	update_clock(mw, NewAnalog, NewDigital, NewDate, 
	  NewMilitaryTime, NewMenubar);
    } else {
        update_clock(mw, AnalogPresent(mw), DigitalPresent(mw), DatePresent(mw),
	  NewMilitaryTime, MenubarPresent(mw));
        XmToggleButtonSetState(AnalogWid(mw), AnalogPresent(mw), TRUE);
	XmToggleButtonSetState(DigitalWid(mw), DigitalPresent(mw), TRUE);
	XmToggleButtonSetState(DateWid(mw), DatePresent(mw), TRUE);
	XmToggleButtonSetState(MenubarWid(mw), MenubarPresent(mw), TRUE);
	no_display_selected = TRUE;
    }

    if (no_display_selected)
	display_error_message(mw, "No_Display_Selected");
}

/*
**++
**  ROUTINE NAME:
**	update_alarm (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Stores the current parameter values based on settings.
**	Makes sure the proper parts of the clock are visible
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
** 	Settings widgets values
**
**  IMPLICIT OUTPUTS:
**	All local settings
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Visible parts of the clock may change
**--
**/
void update_alarm(w)
    Widget w;
{
    char *hr_str, *min_str, *mes_str, *null_str = "";
    int i, hr_val, min_val, hr_str_len, min_str_len, NewAlarm;
    long byte_count, cvt_status;
    Boolean bad_hour, bad_minute, bad_alarm_time;
    ClockWidget mw = (ClockWidget) w;
    XmString xm_mes_str;
    char *fname_str;
    XmString xm_fname_str;

    hr_str = XmTextGetString(HrWid(mw));
    min_str = XmTextGetString(MinWid(mw));
    xm_mes_str = DXmCSTextGetString(AlarmMesWid(mw));
    mes_str = DXmCvtCStoFC(xm_mes_str, &byte_count, &cvt_status);
    XtFree((char *) xm_mes_str);
  if (!NO_SOUND)
  {
    xm_fname_str = DXmCSTextGetString(AudioFnameTextWid(mw));
    fname_str = DXmCvtCStoFC(xm_fname_str, &byte_count, &cvt_status);
    XtFree((char *)xm_fname_str);
  }
    if (hr_str == 0)
	hr_str = null_str;
    if (min_str == 0)
	min_str = null_str;
    if (mes_str == 0)
	mes_str = null_str;

    hr_str_len = strlen(hr_str);
    min_str_len = strlen(min_str);

    /* use sscanf only if there are characters in the string, otherwise set the
     * value to zero. */
    if (hr_str_len == 0)
	hr_val = 0;
    else
	sscanf(hr_str, "%d", &hr_val);

    if (min_str_len == 0)
	min_val = 0;
    else
	sscanf(min_str, "%d", &min_val);

    bad_hour = FALSE;
    bad_minute = FALSE;
    bad_alarm_time = FALSE;

    /* Test hour string for non digits or hour values out of range */
    for (i = 0; i < hr_str_len; i++)
	if (isdigit(hr_str[i]) == 0)
	    bad_hour = TRUE;
    if (hr_val > 23 || hr_val < 0)
	bad_hour = TRUE;

    /* Test minutes string for non digits or minute values out of range */
    for (i = 0; i < min_str_len; i++)
	if (isdigit(min_str[i]) == 0)
	    bad_minute = TRUE;
    if (min_val > 59 || min_val < 0)
	bad_minute = TRUE;

    if (bad_minute)
	XmTextSetString(MinWid(mw), AlarmMin(mw));
    if (bad_hour)
	XmTextSetString(HrWid(mw), AlarmHr(mw));

    if (bad_minute || bad_hour) {
	bad_alarm_time = TRUE;
    } else {
	strcpy(AlarmHr(mw), hr_str);
	strcpy(AlarmMin(mw), min_str);
    }
    strcpy(AlarmMes(mw), mes_str);
  if (!NO_SOUND)
  {
    strcpy(AudioFname(mw), fname_str);
    XtFree(fname_str);
  }
    XtFree(hr_str);
    XtFree(min_str);
    XtFree(mes_str);

    if (bad_alarm_time) {
	NewAlarm = AlarmOn(mw);
	if (AlarmPm(mw)) {
	    XmToggleButtonSetState(AmWid(mw), FALSE, TRUE);
	    XmToggleButtonSetState(PmWid(mw), TRUE, TRUE);
	} else {
	    XmToggleButtonSetState(AmWid(mw), TRUE, TRUE);
	    XmToggleButtonSetState(PmWid(mw), FALSE, TRUE);
	}
/*	XmToggleButtonSetState(PmWid(mw), AlarmPm(mw), TRUE); */
	XmToggleButtonSetState(AlarmWid(mw), AlarmOn(mw), TRUE);
	XmToggleButtonSetState(RepeatWid(mw), RepeatOn(mw), TRUE);
    } else {
	NewAlarm = XmToggleButtonGetState(AlarmWid(mw));
	AlarmPm(mw) = XmToggleButtonGetState(PmWid(mw));
	RepeatOn(mw) = XmToggleButtonGetState(RepeatWid(mw));
    }

    BellOn(mw) = XmToggleButtonGetState(BellOnWid(mw));

  if (!NO_SOUND)
  {
    if (AudioCapable(mw)) {
	Widget audio_menu_widget;

	AudioOn(mw) = XmToggleButtonGetState(AudioOnWid(mw));

	XtVaGetValues (
	  (Widget)AudioOutputMenuWid(mw), XmNmenuHistory, &audio_menu_widget, NULL);
	if (audio_menu_widget == (Widget) SpeakerOnWid(mw))
	    SpeakerOn(mw) = TRUE;
	else
	    SpeakerOn(mw) = FALSE;
	XmScaleGetValue(AudioVolumeWid(mw), &AudioVolume(mw));
    }
  }

    if (bad_alarm_time)
	display_error_message(mw, "Illegal_Alarm_Time");
    else {
	if (NewAlarm != AlarmOn(mw)) {
	    AlarmOn(mw) = NewAlarm;
	    if (AlarmOn(mw))
		XtManageChild((Widget) AlarmBell(mw));
	    else
		XtUnmanageChild((Widget) AlarmBell(mw));
	}
    }
    if (AlarmOn(mw))
	DrawBell(mw);
    
}

/*
**++
**  ROUTINE NAME:
**	update_clock (w)
**
**  FUNCTIONAL DESCRIPTION:
**	Makes sure the proper parts of the clock are visible
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
** 	Settings widgets values
**
**  IMPLICIT OUTPUTS:
**	All local settings
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Visible parts of the clock may change
**--
**/
void update_clock(mw, NewAnalog, NewDigital, NewDate, NewMilitaryTime,
  NewMenubar)
    ClockWidget mw;
    int NewAnalog;
    int NewDigital;
    int NewDate;
    int NewMilitaryTime;
    int NewMenubar;
{
    int manage_count = 0, unmanage_count = 0, ac = 0;
    long time_value;
    struct tm *localtime();
    struct tm tm;			/* current time */
    Arg al[5];
    Widget manage_set[10], unmanage_set[10];

    if (NewAnalog != AnalogPresent(mw)) {
	AnalogPresent(mw) = NewAnalog;
	if (AnalogPresent(mw)) {
	    manage_set[manage_count] = (Widget) AnalogPart(mw);
	    manage_count++;
	} else {
	    unmanage_set[unmanage_count] = (Widget) AnalogPart(mw);
	    unmanage_count++;
	}
    }

    if (NewDigital != DigitalPresent(mw)) {
	DigitalPresent(mw) = NewDigital;
	if (DigitalPresent(mw)) {
	    manage_set[manage_count] = (Widget) DigitalPart(mw);
	    manage_count++;
	} else {
	    unmanage_set[unmanage_count] = (Widget) DigitalPart(mw);
	    unmanage_count++;
	}
    }

    if (NewDate != DatePresent(mw)) {
	DatePresent(mw) = NewDate;
	if (DatePresent(mw)) {
	    manage_set[manage_count] = (Widget) DatePart(mw);
	    manage_count++;
	} else {
	    unmanage_set[unmanage_count] = (Widget) DatePart(mw);
	    unmanage_count++;
	}
    }

    if (NewMenubar != MenubarPresent(mw)) {
	MenubarPresent(mw) = NewMenubar;

	/* Menubar present has changed, need to adjust minwidth and minheight
	 * accordingly. */
	if (MenubarPresent(mw)) {
	    XtSetArg(al[ac], XmNminWidth, Minwidth(mw));
	    ac++;
	    XtSetArg(al[ac], XmNminHeight, Minheight(mw));
	    ac++;
	    manage_set[manage_count] = (Widget) TopMenuBar(mw);
	    manage_count++;
	} else {
	    XtSetArg(al[ac], XmNminWidth, NoMbMinwidth(mw));
	    ac++;
	    XtSetArg(al[ac], XmNminHeight, NoMbMinheight(mw));
	    ac++;
	    unmanage_set[unmanage_count] = (Widget) TopMenuBar(mw);
	    unmanage_count++;
	}

	/* Go and set the new minWidth and minHeight */
	XtSetValues(XtParent(mw), al, ac);
    }
    if (manage_count)
	XtManageChildren(manage_set, manage_count);
    if (unmanage_count)
	XtUnmanageChildren(unmanage_set, unmanage_count);

    /* now we have to update the date and time strings 
     * in case they weren't previously displayed */
    
    /* get the time in case we need to change strings */
    (void) time(&time_value);
    tm = *localtime(&time_value);

    if (NewMilitaryTime != MilitaryTime(mw)) {
	MilitaryTime(mw) = NewMilitaryTime;
	set_time_strings(mw, tm, FALSE, TRUE);
	if (AlarmSettings(mw) != NULL)
	    military_update(mw); 
    }

    if (DigitalPresent(mw)) {
	set_time_strings(mw, tm, FALSE, FALSE);
	if (XtIsManaged(DigitalPart(mw)))
	    DrawDigital(mw, tm);
    }

    if (DatePresent(mw)) {
	set_time_strings(mw, tm, TRUE, FALSE);
	if (XtIsManaged(DatePart(mw)))
	    DrawDate(mw);
    }

}

/*
**++
**  ROUTINE NAME:
**	reset_mode (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Resets the settings widgets vars when a cancel is performed
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
** 	Settings widgets values
**
**  IMPLICIT OUTPUTS:
**	All local settings
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Visible parts of the clock may change
**--
**/
void reset_mode(w)
    Widget(w);
{
    ClockWidget mw = (ClockWidget) w;
    XmToggleButtonSetState(AnalogWid(mw), AnalogPresent(mw), TRUE);
    XmToggleButtonSetState(DigitalWid(mw), DigitalPresent(mw), TRUE);
    XmToggleButtonSetState(DateWid(mw), DatePresent(mw), TRUE);
    XmToggleButtonSetState(MilitaryTimeWid(mw), MilitaryTime(mw), TRUE);
    XmToggleButtonSetState(MenubarWid(mw), MenubarPresent(mw), TRUE);
}

/*
**++
**  ROUTINE NAME:
**	reset_alarm (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Resets the settings widgets vars when a cancel is performed
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
** 	Settings widgets values
**
**  IMPLICIT OUTPUTS:
**	All local settings
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**	Visible parts of the clock may change
**--
**/
void reset_alarm(w)
    Widget(w);
{
    char *text;
    int count = 0;
    long byte_count, cvt_status;
    Boolean separator;
    ClockWidget mw = (ClockWidget) w;
    XmString xm_mes_str, xm_fname_str;
    XmStringCharSet charset;
    XmStringContext context;
    XmStringDirection direction;

    XmToggleButtonSetState(AlarmWid(mw), AlarmOn(mw), TRUE);
    XmToggleButtonSetState(RepeatWid(mw), RepeatOn(mw), TRUE);
    XmToggleButtonSetState(BellOnWid(mw), BellOn(mw), TRUE);

  if (!NO_SOUND)
  {
    XmToggleButtonSetState(AudioOnWid(mw), AudioOn(mw), TRUE);

    if (!AudioCapable(mw) || !AudioOn(mw)) {
	audio_set_insensitive(mw);
    } else {
	if (!AudioAsyncActive(mw)) { 
	    XtSetSensitive((Widget)AudioStopButtonWid(mw), FALSE);
	    XtSetSensitive((Widget)AudioPlayButtonWid(mw), TRUE);
	}
	XtSetSensitive((Widget)AudioOutputMenuWid(mw), TRUE);
	XtSetSensitive((Widget)AudioVolumeWid(mw), TRUE);
	XtSetSensitive((Widget)AudioFnameButtonWid(mw), TRUE);
	XtSetSensitive((Widget)AudioFnameTextWid(mw), TRUE);
    }

    if (AudioVolume(mw) > 100)
	AudioVolume(mw) = 100;
    if (AudioVolume(mw) < 0)
	AudioVolume(mw) = 0;
    XmScaleSetValue(AudioVolumeWid(mw), AudioVolume(mw));

    if (SpeakerOn(mw)) {
	XtVaSetValues (
	  (Widget)AudioOutputMenuWid(mw), XmNmenuHistory, SpeakerOnWid(mw), NULL);
    } else {
	XtVaSetValues (
	  (Widget)AudioOutputMenuWid(mw), XmNmenuHistory, HeadphoneOnWid(mw), NULL);
    }
  } /* end NO_SOUND */
    if (AlarmPm(mw)) {
	XmToggleButtonSetState(AmWid(mw), FALSE, TRUE);
	XmToggleButtonSetState(PmWid(mw), TRUE, TRUE);
    } else {
	XmToggleButtonSetState(AmWid(mw), TRUE, TRUE);
	XmToggleButtonSetState(PmWid(mw), FALSE, TRUE);
    }

    XmTextSetString(HrWid(mw), AlarmHr(mw));
    XmTextSetString(MinWid(mw), AlarmMin(mw));
    xm_mes_str = DXmCvtFCtoCS(AlarmMes(mw), &byte_count, &cvt_status);
    DXmCSTextSetString(AlarmMesWid(mw), xm_mes_str);
  if (!NO_SOUND)
  {
    xm_fname_str = DXmCvtFCtoCS(AudioFname(mw), &byte_count, &cvt_status);
    DXmCSTextSetString(AudioFnameTextWid(mw), xm_fname_str);
  }
    XmTextSetCursorPosition(HrWid(mw), strlen(AlarmHr(mw)));
    XmTextSetCursorPosition(MinWid(mw), strlen(AlarmMin(mw)));

    DXmCSTextSetCursorPosition(AlarmMesWid(mw),
      DWI18n_CharCount(AlarmMes(mw)));
    XmStringFree(xm_mes_str);
}

/*
**++
**  ROUTINE NAME:
**	military_update
**
**  FUNCTIONAL DESCRIPTION:
**	Manage/Unmanage AM and PM alarm buttons based on
**	whether military time is selected. 
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void military_update(mw)
    ClockWidget mw;
{
    char *hr_str, *null_str = "";
    int hr_val, hr_str_len, pm_state;

    hr_str = XmTextGetString(HrWid(mw));

    if (hr_str == 0)
	hr_str = null_str;
    hr_str_len = strlen(hr_str);
    if (hr_str_len == 0)
	hr_val = 0;
    else {
        sscanf(hr_str, "%d", &hr_val);
    }

    if (MilitaryTime(mw)) {
	if (XtIsManaged(PmWid(mw))) {
	    pm_state = XmToggleButtonGetState(PmWid(mw));
	    if (hr_val <= 12) { 
		if (pm_state) {
		    if (hr_val < 12)
			hr_val += 12;
		} else { 
		    if (hr_val == 12)
			hr_val = 0;
		}
	    }
	    XtUnmanageChild((Widget) AmWid(mw));
	    XtUnmanageChild((Widget) PmWid(mw));
	    sprintf(hr_str, "%d", hr_val);
	    XmTextSetString(HrWid(mw), hr_str);
	}
    } else {
        if (!XtIsManaged(PmWid(mw))) {
	    if (hr_val >= 12 && hr_val < 24) {
		hr_val -= 12;
		XmToggleButtonSetState(PmWid(mw), TRUE ,TRUE);
		XmToggleButtonSetState(AmWid(mw), FALSE ,TRUE);
	    } else {
		XmToggleButtonSetState(PmWid(mw), FALSE ,TRUE);
		XmToggleButtonSetState(AmWid(mw), TRUE ,TRUE);
	    }
            if (hr_val == 0)
		hr_val = 12;
	    XtManageChild((Widget) AmWid(mw));
	    XtManageChild((Widget) PmWid(mw));
	    sprintf(hr_str, "%d", hr_val);
	    XmTextSetString(HrWid(mw), hr_str);
	}
    }
    XtFree(hr_str);
}

/*
**++
**  ROUTINE NAME:
**	mode_settings (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Load the widgets with the stored values and display the settings
**	dialog box.
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void mode_settings(w)
    Widget(w);
{
    ClockWidget mw = (ClockWidget) w;
    Widget settings_wid;
    Boolean settings_found = TRUE;
    Boolean first_time_in = FALSE;
    MrmType *dummy_class;

    if (Settings(mw) == NULL) {
	WorkCursorDisplay();
	if (MrmFetchWidget(ClockHierarchy(mw), "mode_settings_dialog_box", mw,
	  &settings_wid, &dummy_class) != MrmSUCCESS) {
	    clock_display_message(mw, NoSettings, WARNING);
	    settings_found = FALSE;
	}
	if (settings_found)
	    Settings(mw) = (XmBulletinBoardWidget) settings_wid;
	first_time_in = TRUE;
	WorkCursorRemove();
    }

    if (Settings(mw) != NULL) {
	if (first_time_in) {
	    reset_mode(mw);
	}
	XtManageChild((Widget) Settings(mw));
    }
}

/*
**++
**  ROUTINE NAME:
**	alarm_settings (w)
**
**  FUNCTIONAL DESCRIPTION:
**  	Load the widgets with the stored values and display the alarm
**	settings dialog box.
**
**  FORMAL PARAMETERS:
**	w - the main clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void alarm_settings(w)
    ClockWidget w;
{
    ClockWidget mw = (ClockWidget) w;
    Widget settings_wid;
    Boolean settings_found = TRUE;
    Boolean first_time_in = FALSE;
    MrmType *dummy_class;
    Widget widget_list[3];
/*  WidgetList widget_list; */

    if (AlarmSettings(mw) == NULL) {
	WorkCursorDisplay();
  if (NO_SOUND)
  {
	if (MrmFetchWidget(ClockHierarchy(mw), "no_audio_alarm_settings_dialog_box", mw,
	  &settings_wid, &dummy_class) != MrmSUCCESS) {
	    clock_display_message(mw, NoSettings, WARNING);
	    settings_found = FALSE;
	}
  }
  else {
	if (MrmFetchWidget(ClockHierarchy(mw), "alarm_settings_dialog_box", mw,
	  &settings_wid, &dummy_class) != MrmSUCCESS) {
	    clock_display_message(mw, NoSettings, WARNING);
	    settings_found = FALSE;
	}
  }
	if (settings_found)
	    AlarmSettings(mw) = (XmBulletinBoardWidget) settings_wid;
	first_time_in = TRUE;
	WorkCursorRemove();
    }

    if (AlarmSettings(mw) != NULL) {
	if (first_time_in) {
	    reset_alarm(mw);
/*	    widget_list = DXmChildren (XtParent (AudioFnameButtonWid(mw)) ); 
	    DXmFormSpaceButtonsEqually ( (Widget) AlarmSettings(mw) , widget_list , DXmNumChildren (widget_list) );
*/
  if (!NO_SOUND)
  {
	    widget_list[0] = (Widget) AudioFnameButtonWid(mw);
	    widget_list[1] = (Widget) AudioPlayButtonWid(mw);
	    widget_list[2] = (Widget) AudioStopButtonWid(mw);
	    DXmFormSpaceButtonsEqually ( (Widget) AlarmSettings(mw) , (Widget) widget_list , (Cardinal) 3 );
  }
	}
	XmTextSetSelection(HrWid(mw), 0, strlen(AlarmHr(mw)), CurrentTime);
        military_update(mw);
	XtManageChild((Widget) AlarmSettings(mw));
    }
}

/*
**++
**  ROUTINE NAME:
**	clock_ChangeWindowGeometry (w, size)
**
**  FUNCTIONAL DESCRIPTION:
**  	Calls XtMoveWidet and XtResizeWidget to change the size and location of
**	widget
**
**  FORMAL PARAMETERS:
**  	w - the widget
**	size - geometry specification
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
#ifdef DWTVMS
#define ReadDesc(source_desc,addr,len)	\
    LIB$ANALYZE_SDESC(source_desc,&len,&addr)
#endif

void clock_ChangeWindowGeometry(w, size)
    Widget w;
    XtWidgetGeometry *size;
{


    /* Split the changes into the two modes the intrinsic's understand, */

    if (size->request_mode & (CWX | CWY)) {
	int newx, newy;

	if (size->request_mode & CWX)
	    newx = size->x;
	else
	    newx = w->core.x;
	if (size->request_mode & CWY)
	    newy = size->y;
	else
	    newy = w->core.y;
	XtMoveWidget(w, newx, newy);
    }

    if (size->request_mode & (CWWidth | CWHeight | CWBorderWidth)) {
	if (size->request_mode & CWWidth)
	    w->core.width = size->width;
	if (size->request_mode & CWHeight)
	    w->core.height = size->height;
	if (size->request_mode & CWBorderWidth)
	    w->core.border_width = size->border_width;
	XtResizeWindow(w);
    }
}

/*
**++
**  ROUTINE NAME:
**	ClockLoadFontFamilies (mw)
**
**  FUNCTIONAL DESCRIPTION:
** 	Loads all of the fonts based on user request
**
**  FORMAL PARAMETERS:
**	mw - the clock widget
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void ClockLoadFontFamilies(mw)
    ClockWidget mw;
{
    char sqrt_char[2], pattern[100], *CharSet, **fontnames;
    int i, j, temp, len, font_height, num_found, KFontHeights[10],
      OrderedArray[10], maxnames = 10;
    long byte_count, cvt_status;
    int direction = 0, ascent = 0, descent = 0;
    XmString xm_str;
    XCharStruct overall;
    XFontStruct *KFonts[10];

    strcpy(pattern, mw->clock. font_family);
    fontnames = XListFonts(Dpy(mw), pattern, maxnames, &num_found);

    if (IsJaLocale(Language(mw))) {
	if (num_found == 0) {
	    fontnames =
	      XListFonts(Dpy(mw),
	      "-JDECW-Screen-Medium-R-Normal--*-*-*-*-M-*-JISX0208-Kanji11",
	      maxnames, &num_found);
	}
    }

    for (i = 0; i < num_found; i++) {
	KFonts[i] = DXmLoadQueryFont(Dpy(mw), fontnames[i]);
	KFontHeights[i] =
	  (KFonts[i]->max_bounds.ascent + KFonts[i]->max_bounds.descent);
	OrderedArray[i] = i;
    }

    for (i = 0; i < num_found; i++)
	for (j = 0; j < num_found - 1; j++)
	    if (KFontHeights[OrderedArray[j]] >
	      KFontHeights[OrderedArray[j + 1]]) {
		temp = OrderedArray[j];
		OrderedArray[j] = OrderedArray[j + 1];
		OrderedArray[j + 1] = temp;
	    }

    NumFonts(mw) = num_found;
    for (i = 0; i < num_found; i++) {
	Fonts(mw)[i] = KFonts[OrderedArray[i]];
	FontHeights(mw)[i] = KFontHeights[OrderedArray[i]];
	if (IsAsianLocale(Language(mw))) {
	    CharSet = DWI18n_GetXLFDCharSet(pattern);
	    DigitalFontLists(mw)[i] = XmFontListCreate(Fonts(mw)[i], CharSet);
	    XtFree(CharSet);
	    xm_str = DXmCvtFCtoCS(LongestTime(mw), &byte_count, &cvt_status);
	    DigitalFontWidths(mw)[i] =
	      XmStringWidth(DigitalFontLists(mw)[i], xm_str);
	    XmStringFree(xm_str);
	} else {
	    XTextExtents(Fonts(mw)[i], LongestTime(mw), strlen(
	      LongestTime(mw)), &direction, &ascent, &descent, &overall);
	    DigitalFontWidths(mw)[i] = overall.width;
	}

	if (IsAsianLocale(Language(mw))) {
	    CharSet = DWI18n_GetXLFDCharSet(pattern);
	    DateFontLists(mw)[i] = XmFontListCreate(Fonts(mw)[i], CharSet);
	    XtFree(CharSet);
	    xm_str = DXmCvtFCtoCS(LongestDate(mw), &byte_count, &cvt_status);
	    DateFontWidths(mw)[i] =
	      XmStringWidth(DateFontLists(mw)[i], xm_str);
	    XmStringFree(xm_str);
	} else {
	    XTextExtents(Fonts(mw)[i], LongestDate(mw), strlen(
	      LongestDate(mw)), &direction, &ascent, &descent, &overall);
	    DateFontWidths(mw)[i] = overall.width;
	}
    }
    XFreeFontNames(fontnames);
}

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME:
**	DescToNull
**
**  FUNCTIONAL DESCRIPTION:
**  	Converts a string descriptor to a null terminated string
**
**  FORMAL PARAMETERS:
**	desc - the descriptor
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
char *DescToNull(desc)
    struct dsc$descriptor_s *desc;
{
    char *nullterm_string, *temp_string;
    unsigned short temp_length;

    if (desc->dsc$b_class <= DSC$K_CLASS_D) {
	temp_length = desc->dsc$w_length;
	temp_string = desc->dsc$a_pointer;
    } else
	ReadDesc(desc, temp_string, temp_length);

    nullterm_string = (char *) XtMalloc(temp_length + 1);

    if (temp_length != 0)
	mybcopy(temp_string, nullterm_string, temp_length);

    *(nullterm_string + temp_length) = '\0';

    return (nullterm_string);
}
#endif

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME: mybcopy
**
**  FUNCTIONAL DESCRIPTION:
**      Replaces the C bcopy routine. Copies an area of memory to another.
**
**  FORMAL PARAMETERS:
**      b1 - buffer to copy from.
**      b2 - buffer to copy into.
**      n  - number of bytes to copy.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mybcopy(b1, b2, n)
    char *b1;
    char *b2;
    int n;
{
    int i;

    for (i = 0; i < n; i++) {
        *b2 = *b1;
        b1++;
        b2++;
    }
}
#endif
/*
**++
**  ROUTINE NAME:
**	ClockXtWarnHandler
**
**  FUNCTIONAL DESCRIPTION:
**  	Clock local Xt Warning handler routine.  
** 	The intent of this routine is to filter out the warning message
**	"X Toolkit Warning: XtRemoveGrab asked to remove a widget not 
**	 on the list". This message occurs because clock frequently
**	 does a XtAddGrab/RemoveGrab when it displays/undisplays a 
**	 watch cursor. The routines that do this are WorkCursorDisplay/
**	 WorkCursorRemove. The problem is that the grab list is a 
**	 stack and when you do a remove your grab and all those
**	 above yours (more recent) are also removed. Clock was seeing
**	 these messages when invoking HyperHelp routines and managing
**	 File Selction Widgets. Both of these do an XtAddGrab that
**	 gets mismatched when we do our remove. We really do want
**	 out watch cursor since these two things are expensive operations. 
**	 The toolkit team suggested that the hyperhelp warning is really 
**	 a Bookreader bug and they should be anticipating this problem 
**	 and trapping the message themselves. I assume the file 
**	 selection widget should do the same. 
**	 Note that there are times when this message might indicate a real
**	 bug. In the development environment, define the CLOCK_TOOLKIT_MESSAGES
**	 environment variable/logical to enable these messages. 
**
**  FORMAL PARAMETERS:
**	mesg - pointer to the warning message
**
**  IMPLICIT INPUTS:
**	X Toolkit warning message 
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  NOTES:
**	What happens when Motif is translated??? Is this message still
**	in English. If not this routine is hosed. 
**--
**/
void ClockXtWarnHandler(mesg)
char *mesg;
{
	char *value;
	char *filter_mesg = "XtRemoveGrab";

	/* display message only if CLOCK_TOOLKIT_MESSAGES is defined */
	value = getenv("CLOCK_TOOLKIT_MESSAGES");

	if (value)
		printf("%s\n",mesg);
	else
	{
		/* if this is the XtRemoveGrab warning message, do not print
	 	* else, print the warning message
	 	*/
		if (strstr(mesg,filter_mesg))
			return;
		else
			printf("%s\n",mesg);
	}
}
