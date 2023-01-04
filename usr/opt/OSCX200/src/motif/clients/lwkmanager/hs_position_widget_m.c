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
** COPYRIGHT (c) 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/
/*									   
**++
**  FACILITY:
**
**	General purpose
**	
**  AUTHOR:
**
**  ABSTRACT:
**
**	Position a widget to the right, left, or below a
**	given reference widget according to Motif Style
**	Guide conventions.
**
**  ENVIRONMENT:
**
**	DECwindows, user mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	X7-1	ap  10-Nov-89	Integrate with HyperSession
**--
**/


/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"


/*
**  Widget Border fudge factors
*/

#define LEFT_MARGIN 10
#define RIGHT_MARGIN 10
#define BOTTOM_MARGIN 10
#define TOP_MARGIN 25


#define XOFFSET 20
#define YOFFSET 30

/*
**  Type Definitions
*/

typedef struct rect {
	Position x1, y1, /* (X,Y) lower left coordinate of rectangle   */
	         x2, y2; /* (X,Y) upper right coordinate of rectangle  */
	Dimension width, /* Width of rectangle			       */
	         height; /* Height of rectangle			       */
	int      area;   /* Area of rectangle			       */
    } rect;

/*
**  Table of Contents
*/

_DeclareFunction(static void screen_to_rect,
    (Widget widget, rect *screen_rect));
_DeclareFunction(static void widget_to_rect,
    (Widget widget, rect *widget_rect));
_DeclareFunction(static void calc_position,
    (rect *widget, rect *ref_rect, rect *screen_rect));
_DeclareFunction(static void set_position,
    (Widget widget, Position x, Position y));


                                                               
_Void  EnvDWPositionWindow(lb_win, env_win, lb_win_count)
_WindowPrivate lb_win;
 _WindowPrivate env_win;

    _Integer lb_win_count;

/*
**++
**  Functional Description:
**	Position a new Widget on the screen according to the position
**	of a reference widget.
**
**  Keywords:
**	Position, Window
**
**  Arguments:
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    rect widget_rect;	/* Rectangle describing new widget		*/
    rect ref_rect;	/* Rectangle describing a widget to reference   */
    rect screen_rect;	/* Rectangle describing the screen		*/
    Widget new_widget;
    Widget ref_widget;
    
    new_widget = lb_win->shell;
    ref_widget = env_win->shell;

    /*
    **  Get the screen coordinates.
    */

    screen_to_rect(new_widget, &screen_rect);

    /*
    **  Get the rectangular coordinates of the new widget.
    */

    widget_to_rect(new_widget, &widget_rect);

    /*
    **  If the new Widget is larger than the screen, just position it at (0,0).
    **	If there is no reference widget, center the new widget on the screen.
    */

    if (widget_rect.area >= screen_rect.area) {
	set_position(new_widget, 0, 0);
	return;
    }

    if (ref_widget == NULL) {
	set_position(new_widget,
		     (screen_rect.width - widget_rect.width) / 2,
		     (screen_rect.height - widget_rect.height) / 2);
	return;
    }

    widget_to_rect(ref_widget, &ref_rect);

    /*
    **  Position widget according to reference widget
    */

    calc_position(&widget_rect, &ref_rect, &screen_rect);
    
    set_position(new_widget, widget_rect.x1 + lb_win_count*XOFFSET,
	widget_rect.y1 + lb_win_count*YOFFSET);

    return;
    }

static void  screen_to_rect(widget, pscr)
Widget widget;
 rect *pscr;

/*
**++
**  Functional Description:
**	Get the coordinates of the screen
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : any Widget on the screen
**	
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    pscr->width = WidthOfScreen(XtScreen(widget));
    pscr->height = HeightOfScreen(XtScreen(widget));

    pscr->area = pscr->height * pscr->width;
    
    pscr->x1 = 0;
    pscr->y1 = 0;
    pscr->x2 = pscr->width;
    pscr->y2 = pscr->height;

    return;
    }

static void  widget_to_rect(widget, widget_rect)
Widget widget;
 rect *widget_rect;

/*
**++
**  Functional Description:
**	Determine the screen coordinates of a Widget.
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : Widget to analyze
**	widget_rect : rect structure to fill
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    int x, y;
    Dimension width, height;
    Arg	arglist[2];
    Window window;

    /*
    **	Get the size and the coordinates of the Widget.  If it is not realized
    **	we can't get the coordintates, so create a dummy rectangle.
    */

    if (XtIsRealized(widget)) {
	XtSetArg(arglist [0], XmNwidth,  &width);
	XtSetArg(arglist [1], XmNheight, &height);
	XtGetValues(widget, arglist, 2);

	XTranslateCoordinates(XtDisplay(widget), XtWindow(widget),
	    RootWindowOfScreen(XtScreen(widget)), 0, 0, &x, &y,
	    &window);

	/*
	**  Fudge in a border for the Widget
	*/

	widget_rect->width = width + LEFT_MARGIN + RIGHT_MARGIN;
	widget_rect->height = height + TOP_MARGIN + BOTTOM_MARGIN;
	widget_rect->x1 = x - LEFT_MARGIN;
	widget_rect->y1 = y - TOP_MARGIN;
    }
    else {
	widget_rect->width = 1;
	widget_rect->height = 1;
	widget_rect->x1 = 0;
	widget_rect->y1 = 0;
    }

    /*
    **  Calculate the other rectangular coordinates
    */

    widget_rect->x2 = widget_rect->x1 + widget_rect->width;
    widget_rect->y2 = widget_rect->y1 + widget_rect->height;
    widget_rect->area = widget_rect->height * widget_rect->width;

    return;
    }
                           

static void  calc_position(widget, pref, pscr)
rect *widget;
 rect *pref;
 rect *pscr;

/*
**++
**  Functional Description:
**
**	Attempt to place a widget to the right, left, or below
**	the reference widget
**
**  Keywords:
**	None
**
**  Arguments:
**	widget : rectangle structure for the widget to be positioned
**	pref :  rectangle structure for parent widget to be referenceed
**	pscr   : rectangle structure for screen 
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Position best_x, best_y;
    Boolean done = False;
    
    /*
    ** Attempt to place the widget to the right of the reference widget
    */
    
    best_x = pref->x2;
    best_y = (pref->y1 + pref->y2 - widget->height)/2;

    /*
    **	Shift the widget if it is off the screen
    */
    
    if (best_y + widget->height > pscr->y2)	
	best_y = pscr->y2 - widget->height;	    /* shift up */
    if (best_y < pscr->y1)
	best_y = pscr->y1;			    /* shift down */
    if (best_x < pscr->x1)
	best_x = pscr->x1;			    /* shift right */


    if ( ((best_x + widget->width) <= pscr->x2) &&
	 ((best_y + widget->height) <= pscr->y2) &&
	 (best_y >= pscr->y1))
	 done = True;


     if (!done) { /* try for the left side */
	
	best_x = pref->x1 - widget->width;
	best_y = (pref->y1 + pref->y2 - widget->height)/2;
	
	/*
	**	Shift the widget if it is off the screen
	*/

	if (best_y + widget->height > pscr->y2)	
	    best_y = pscr->y2 - widget->height;	    /* shift up */
	if (best_y < pscr->y1)
	    best_y = pscr->y1;			    /* shift down */
	if (best_x + widget->width > pscr->x2)
	    best_x = pscr->x2 - widget->width;	    /* shift left */

	if ( (best_x >= pscr->x1) &&
	     ((best_y + widget->height) <= pscr->y2) &&
	     (best_y >= pscr->y1))
	     done = True;
    };

    if (!done) { /* try for the bottom */
    
	best_x = (pref->x1 + pref->x2 - widget->width)/2;
	best_y = pref->y2;

	/*
	**	Shift the widget if it is off the screen
	*/
	
	if (best_y < pscr->y1)
	    best_y = pscr->y1;			    /* shift down */
	if (best_x < pscr->x1)
	    best_x = pscr->x1;			    /* shift right */
	if (best_x + widget->width > pscr->x2)
	    best_x = pscr->x2 - widget->width;	    /* shift left */

	if ( (best_x >= pscr->x1) &&
	     ((best_y + widget->height) <= pscr->y2) &&
	     (best_y >= pscr->y1))
	     done = True;
    }


    if (!done) { /* Center the widget */
	best_x = (pref->x1 + pref->x2 - widget->width)/2;
	best_y = (pref->y1 + pref->y2 - widget->height)/2;

	/*
	**	Shift the widget if it is off the screen
	*/
	
	if (best_y + widget->height > pscr->y2)
	    best_y = pscr->y2 - widget->height;	    /* shift up */
	if (best_y < pscr->y1)
	    best_y = pscr->y1;			    /* shift down */
	if (best_x < pscr->x1)
	    best_x = pscr->x1;			    /* shift right */
	if (best_x + widget->width > pscr->x2)
	    best_x = pscr->x2 - widget->width;	    /* shift left */

    };
        	
    /*
    **	Set the rectangle coordinates of the new widget to what we think is
    **	best.
    */
    
    widget->x1 = best_x;
    widget->y1 = best_y;
    widget->x2 = widget->x1 + widget->width;
    widget->y2 = widget->y1 + widget->height;
    
    return;
    }


static void  set_position(widget, x, y)
Widget widget;
 Position x;
 Position y;

/*
**++
**  Functional Description:
**	Set the postion of a widget to a given (X,Y) coordinate
**
**  Keywords:
**	Nonme
**
**  Arguments:
**	widget : ID of the widget to be positioned
**	x : X coordinate
**	y : Y coordinate
**
**  Result:
**	Void
**
**  Exceptions:
**	None
**--
*/
    {
    Arg	arglist[2];

    XtSetArg(arglist[0], XmNx, x + LEFT_MARGIN);
    XtSetArg(arglist[1], XmNy, y + TOP_MARGIN);
    XtSetValues(widget, arglist, 2);
    
    return;
    }
