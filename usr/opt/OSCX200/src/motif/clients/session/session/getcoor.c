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
 *
 *	getrectwin( display, screennum, rectangle, window)
 *		display   - (RO) (*Display) the opened display
 *		dfs	  - (RO) (int) screen number to capture
 *		rectangle - (WO) (*XRectangle) rectangle coordinates
 *		window	  - (WO) (*Window) window ID
 *
 *	restrictions:
 *		memory for rectangle and window should already be allocated
 *
 *	notes:
 *		getrectwin  - A right or left button press determines the x
 *			and y coordinates of the rectangle(equal to
 *			pointer coordinates). A button release determines the
 *			width and height. The rectangle is rubberbanded until
 *			the button release. A middle button press returns
 *			the window id at the position
 *			where the pointer is located.
  */

#include	<stdio.h>
#include	"iprdw.h"
#include <X11/Xlib.h>
#include <X11/cursorfont.h>

#define COPYRECT(to,from) \
	to.x = from.x; \
	to.y = from.y; \
	to.height = from.height; \
	to.width = from.width

#define DIFFRECT(rec1,rec2) \
	((rec1.x!=rec2.x)||(rec1.y!=rec2.y)||(rec1.width!=rec2.width)|| \
	(rec1.height!=rec2.height))

#define ASSIGNRECT(rect,xorg,yorg,curx,cury,minx,miny) \
	rect.width = MAX(EUCDIS(xorg,curx),minx); \
	rect.height = MAX(EUCDIS(yorg,cury),miny); \
	if(curx<xorg) \
		rect.x = MIN(xorg-minx,curx); \
	else \
		rect.x = xorg; \
	if(cury<yorg) \
		rect.y = MIN(yorg-miny,cury); \
	else \
		rect.y = yorg

#define MINW	16
#define MINH	16

Cursor	XCursor();

getrectwin( dpy, dfs, xrect, window )
Display	*dpy;
int dfs;
XRectangle *xrect; 
Window	*window;
{
	char	text[128];	/* text for popup info window		*/
	int	intpos;		/* index into text (integer position)	*/
	int	root_x, root_y;	/* pointer coordinates rel to root win	*/
	int	win_x, win_y;	/* pointer coordinates rel to sub win	*/
	int	initX, initY;	/* initial pointer coordinates	 	*/
	int	popuph, popupw;	/* dimensions of popup info window	*/
	unsigned int mask;	/* current mask value			*/
	long	status;		/* return status value			*/
	long	eventmask;	/* events we are interested in		*/
	bool done = False;	/* have pointer coordinates or window ID*/
	bool pressed = False;	/* a button has been pressed  		*/
	GC gc;			/* graphics context 			*/
	GContext popupgc;	/* popup window graphics context 	*/
	XGCValues values;	/* data structure for graphics context 	*/
	Cursor	cursor;		/* the new cursor value			*/
	XColor  red;		/* a color structure containing red	*/
	XColor  white;		/* a color structure containing white	*/
	XEvent	event;		/* an X event				*/
	XFontStruct *pfont;	/* popup info window font		*/
	XRectangle oldrect;	/* the last rectangle to be dpyed	*/
	XRectangle currect;	/* the currect rectangle		*/
	Window	root, child;	/* window ids 				*/
	Window	popup;		/* ID of informational window		*/

	/*
 	 * initialize
	 */

	values.function = GXinvert;
	values.subwindow_mode = IncludeInferiors;
	values.plane_mask = WhitePixel(dpy, dfs) ^ BlackPixel(dpy, dfs);
	
	gc = XCreateGC( dpy, RootWindow( dpy, dfs), 
		GCFunction |GCPlaneMask | GCSubwindowMode, &values);

	eventmask = NoEventMask | ButtonPressMask | ButtonReleaseMask;
	XSelectInput(dpy, RootWindow(dpy, dfs), eventmask);

	oldrect.x = oldrect.y = currect.x = currect.y = 0;
	oldrect.height = oldrect.width = currect.height = currect.width = 0;
	initX = initY = 0;

	/*
	 * need to grab pointer
	 */

	XGrabServer(dpy); 

	XGrabPointer( dpy, RootWindow( dpy, dfs), False, ButtonPressMask |
		ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None,
		CurrentTime);

	cursor = XCreateFontCursor(dpy, XC_crosshair);
	if (DisplayPlanes( dpy, dfs) != 1) {
		red.red = white.red = white.green = white.blue = 65535;
		red.blue = red.green = 0;
		/* Go Big Red! */
		XRecolorCursor(dpy, cursor, &red, &red);
		}


	XSync(dpy, False);	/* don't discard input events */
	XChangeActivePointerGrab(dpy, eventmask, cursor, CurrentTime );


	while( !done  )
	{ 
		if( XPending( dpy ) )  
		{
			XNextEvent( dpy, &event );
			if(( event.type == ButtonPress ) &&
					( event.xbutton.button == Button1 ))
			{
			XQueryPointer( dpy, RootWindow( dpy, dfs),
				&root, &child, &root_x, &root_y, 
				&win_x, &win_y, &mask );
				initX = root_x;
				initY = root_y;
			COPYRECT(oldrect, currect);
			ASSIGNRECT(currect, initX, initY, 
				root_x, root_y, MINW, MINH);
				*window = NULLWINDOW;
#	ifdef SHOWCOORDS
			XMapRaised(dpy, popup);
#	endif
			XSync(dpy, False);
			pressed = True;
			} 
			else if (( event.type == ButtonRelease ) &&
					( event.xbutton.button == Button1 ))
			{
				XQueryPointer( dpy, RootWindow( dpy, dfs),
					&root, &child, &root_x, &root_y, 
					&win_x, &win_y, &mask );
				COPYRECT(oldrect, currect);
				ASSIGNRECT(currect, initX, initY, root_x, 
					root_y, MINW, MINH);
				xrect->x = currect.x;
				xrect->y = currect.y;
				xrect->height = currect.height -1; /*???*/
				xrect->width = currect.width -1;
				done = True;
			}
			else if (pressed)
			{
				XQueryPointer( dpy, RootWindow( dpy, dfs),
					&root, &child, &root_x, &root_y, 
					&win_x, &win_y, &mask );
				COPYRECT(oldrect, currect);
				ASSIGNRECT(currect, initX, initY, root_x, 
					root_y, MINW, MINH);
			}
		}
		else if( pressed ) 
		{
			XQueryPointer(dpy, RootWindow(dpy, dfs),
				&root, &child, &root_x, &root_y, 
				&win_x, &win_y, &mask );
			COPYRECT(oldrect, currect);
			ASSIGNRECT(currect, initX, initY, root_x, root_y, MINW,
				MINH);
		}
		if( pressed && DIFFRECT(oldrect, currect) )
		{
			XDrawRectangles(dpy, RootWindow(dpy,dfs), 
				gc, &oldrect, 1);
			XDrawRectangles( dpy, RootWindow(dpy, dfs), 
				gc, &currect, 1 );

		}
	}

	/*
	 * make rectangle "blink", then erase rubber band box
	 */
	if(pressed)
	{
		XFillRectangles( dpy, RootWindow(dpy, dfs), gc, &currect, 1 );
		XFillRectangles( dpy, RootWindow(dpy, dfs), gc, &currect, 1 );
		XDrawRectangles( dpy, RootWindow(dpy, dfs), gc, &currect, 1 );
	}
	XSync(dpy, False);

	eventmask = NoEventMask;
	XSelectInput(dpy, RootWindow(dpy, dfs), eventmask);
	XUngrabPointer(dpy, CurrentTime);
	XUngrabServer(dpy); 
	XFreeCursor(dpy, cursor);
	XFreeGC(dpy,gc);
	XSync(dpy, True);	/* do discard input events */  
	return( Normal );
}

