/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  PrintScreen
**
** ABSTRACT:
**
**	This module has miscellaneous routines
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1989
**
** Modified by:
**
**	04-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
*/

/*
** Include files
*/
#include "iprdw.h"

#ifdef VMS
#include <ssdef.h>
#include <prvdef.h>
#include <lnmdef.h>
#endif /* VMS */

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#include "prdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "smresource.h"
#include "smshare.h"
#ifdef vms
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#include "prdw_entry.h"

static void do_cursor PROTOTYPE((int on, Window win));

void widget_create_proc
#if _PRDW_PROTO_
(
    Widget		w,
    Widget		*tag,
    unsigned int	*reason
)
#else
(w, tag, reason)
    Widget		w;
    Widget		*tag;
    unsigned int	*reason;
#endif
/*---
!
!
!       This routine is called when a UIL widget is created
!       We need to fill in our global pointers to these widgets
!
! Inputs:
!       w           The widget id of the widget that is being created
!       tag         Pointer to data where widget id should be stored
!       reason      The reason for this callback
!
! Outputs:
!       *tag - The widget IDs of the widgets being created are stored
!                   in this address.
!---
*/
{
    *tag = w;
    return;
}

void wait_cursor
#if _PRDW_PROTO_
(
    int		on
)
#else
(on)
    int		on;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the cursor on the session manager window to a watch, or
**	back to the default cursor. 
**
**  FORMAL PARAMETERS:
**
**	on - If = 1, display the watch
**	     If = 0, remove the watch
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**	
**--
**/
{
    if (smdata.toplevel != 0) do_cursor(on, XtWindow(smdata.toplevel));
    return;
}

static void do_cursor
#if _PRDW_PROTO_
(
    int		on,
    Window	win
)
#else
(on,win)
    int		on;
    Window	win;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Change the cursor on a window to a watch, or
**	back to the default cursor. 
**
**  FORMAL PARAMETERS:
**
**	on - If = 1, display the watch
**	     If = 0, remove the watch
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**	
**--
**/
{
    Font cfont;
    static Cursor wait_cursor = 0;
    static XColor fore = {0, 65535, 65535, 65535};	/* white */
    static XColor back = {0, 0, 0, 0}; /* black */
    int wait_c;

    if (on == 1)
    {
	if (wait_cursor == 0)
	{
	    cfont = XLoadFont (display_id, "decw$cursor");
	    if (!cfont) return;

	    wait_c = decw$c_wait_cursor;
	    wait_cursor = XCreateGlyphCursor
		(display_id, cfont, cfont, wait_c, wait_c + 1, &fore, &back);
	}
	XDefineCursor (display_id, win, wait_cursor);
	XFlush (display_id);
    }
    else
    {
	XUndefineCursor (display_id, win);
    }
    return;
}

int determine_system_color
#if _PRDW_PROTO_
(
    Display		*display_id,
    unsigned int	screen_num
)
#else
(display_id, screen_num)
    Display		*display_id;
    unsigned int	screen_num;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Looks at the depth of the screen and determines if this is a
**	black/white, color, or gray scale machine.
**
**  FORMAL PARAMETERS:
**
**	display - Pointer to the open display connection
**	screen_num - Screen number to look at.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**--
**/
{
    Visual  *vis;
    Screen  *screen;
    int	    depth;

    screen = ScreenOfDisplay (display_id, screen_num);
    vis = DefaultVisualOfScreen (screen);
    depth = DefaultDepthOfScreen (screen);

    switch (vis->class)
    {
    case StaticGray:
	return ((depth == 1) ? black_white_system : gray_system);
    case GrayScale:
	return (gray_system);
    case    StaticColor:
    case    PseudoColor:
    case    DirectColor:
    case    TrueColor:
	return (color_system);
    default:
	return (black_white_system);
    }
}
