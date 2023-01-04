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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader = "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/session/printscr/getcoor.c,v 1.1 90/01/01 00:00:00 devrcs Exp $";
#endif		/* BuildSystemHeader */
/*

**++
**  COPYRIGHT (c) 1987, 1988, 1989 BY
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
**  ALL RIGHTS RESERVED.
** 
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
**  ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
**  OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
**  TRANSFERRED.
** 
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
**  AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
**  CORPORATION.
** 
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
**--
**/


/*
**++
**  MODULE NAME:
**	getcoor.c
**
**  FACILITY:
**      PrintScreen
**
**  ABSTRACT:
**	Subroutines which get rectangle coordinates or window
**		IDs through use of the pointer.
**
**  ENVIRONMENT:
**
**	VMS V5, Ultrix V2.2  DW FT1
**
**  AUTHORS:
**      Kathy Robinson, Mark Antonelli
**
**  RELEASER:
**	Kathy Robinson
**
**  CREATION DATE:     
**	9-MAR-1987
**
**  MODIFICATION HISTORY:
**	6-JAN-1987
**		Condition include for Ultrix
**		Calls to XSynch added
**		Added missing XFreeGC call
**
**
**	June	2,	1987, MFA
**		Rewrite convoluted code.
**	May	14,	1987, MFA
**		Convert X calls to alpha update format.
**	March	9, 	1987, MFA
**--
**/


/*
 *
 *	getrectwin( display, rectangle, window)
 *		display   - (RO) (*Display) the opened display
 *		rectangle - (WO) (*XRectangle) rectangle coordinates
 *		window	  - (WO) (*Window) window ID
 *
 *	restrictions:
 *		memory for rectangle and window should already be allocated
 *
 *	notes:
 */

#include	<stdio.h>
#include	<iprdw.h>
#ifdef VMS
#include <decw$include/cursorfont.h>
#else
#include <X11/cursorfont.h>
#endif

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

getrectwin( dpy, dfs, xrect, screen_ret)
Display	*dpy;
int dfs;		/* Default screen */
XRectangle *xrect; 
Window *screen_ret;
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
	GContext gc;		/* graphics context 			*/
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
	int i, count;		/* Always useful 			*/

	/*
 	 * initialize
	 */

	*screen_ret = NULLWINDOW;

#ifdef SHOWCOORDS
/*
 * If SHOWCOORDS is defined at compilation then a little window showing the
 * image size appears.  (I think that the font nameing changes broke this. )
 */
    strcpy( text, " printscreen: 000x000 ");
    intpos = strlen(" printscreen: ");

    pfont = XLoadQueryFont(dpy, "fixed");   
    popupw = XTextWidth(pfont, text, strlen(text)) + 2;
    popuph = pfont->ascent + pfont->descent + 2;

    values.function = GXcopy;
    values.font = pfont->fid; 
    values.foreground = WhitePixel(dpy, dfs);
    values.background = BlackPixel(dpy, dfs);

    popup = XCreateSimpleWindow(dpy, RootWindow(dpy, dfs), 0, 0, popupw,
		popuph, 1, WhitePixel(dpy, dfs), BlackPixel(dpy, dfs));

    popupgc = (GContext)XCreateGC( dpy, popup, 
		GCForeground+GCBackground+GCFont, &values);
#endif


/*
 * Create a rubberbanding GC.  (Writing mode is invert)
 */

    values.function = GXinvert;
    values.subwindow_mode = IncludeInferiors;
    values.plane_mask = 0XFF; /* should be BlackPix | WhitePix ??? */

    gc = (GContext)XCreateGC( dpy, RootWindow( dpy, dfs), 
		GCFunction |GCPlaneMask | GCSubwindowMode, &values);

/*
 * intiialize all positions and sizes to zero
 */
    oldrect.x = oldrect.y = currect.x = currect.y = 0;
    oldrect.height = oldrect.width = currect.height = currect.width = 0;
    initX = initY = 0;

/*
 * Will change cursor to cue user, so let's create it here. (Used by GrabPtr )
 */
    cursor = XCreateFontCursor(dpy, XC_crosshair);
    if (DisplayPlanes( dpy, dfs) != 1) {
		red.red = white.red = white.green = white.blue = 65535;
		red.blue = red.green = 0;
		/* Go Big Red! */
		XRecolorCursor(dpy, cursor, &red, &red); 
		}


/*
 * need to grab screen to freeze image; and grab pointer so that we can get 
 * all mouse clicks regardless of what window they're in.  (That is, set 
 * owner events to True, and confine to None).
 */                 

#ifndef GRAB
    XGrabServer(dpy); 
#endif


    eventmask = NoEventMask | ButtonPressMask | ButtonReleaseMask;

    root = RootWindow (dpy,dfs);
    while (status = XGrabPointer( dpy, RootWindow(dpy,dfs), True, 
		    ButtonPressMask | ButtonReleaseMask, 
		    GrabModeAsync, GrabModeAsync, root, cursor, 
		    CurrentTime) == GrabFrozen) 
	;
    			XGrabPointer( dpy, root, False, 
				ButtonPressMask | ButtonReleaseMask, 
				GrabModeAsync, GrabModeAsync, 
				root, cursor, CurrentTime);
    if (status != GrabSuccess) 
	return (XERROR);

    XSync(dpy,0);	/* don't discard input events */


    while( !done  )
    { 

		if( XPending( dpy ) )  
		{
		/*
		 * An event is waiting, so dispatch it.
		 */
			XNextEvent( dpy, &event );
			if(( event.type == ButtonPress ) &&
					( event.xbutton.button == Button1 ))
			{
			/*
			 * MB1 went down, put starting info into initX,Y and
			 * the rectangle structures to start selection
			 *
			 * See what root window they're in now and use that 
			 * as the "confine to" window id in GrabPointer.
			 * And update dfs (our default screen) and
			 * 
			 */

				XSync(dpy,0);	/* don't discard input events */
				root_x = event.xbutton.x_root;
				root_y = event.xbutton.y_root;

				initX = root_x;
				initY = root_y;
				COPYRECT(oldrect, currect);
				ASSIGNRECT(currect, initX, initY, 
					root_x, root_y, MINW, MINH);
#	ifdef SHOWCOORDS
				XMapRaised(dpy, popup);
#	endif
				XSync(dpy,0);
				pressed = True;
			} 




			else if (( event.type == ButtonRelease ) &&
					( event.xbutton.button == Button1 ) &&
					  pressed)
			{
			/*
			 * MB1 went up; put final info into the rectangle 
			 * structures to end selection
			 */
				root_x = event.xbutton.x_root;
				root_y = event.xbutton.y_root;
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
			/*
			 * Some unknown event occured.  If selection has
			 * begun, then  update info in the rectangle
			 * structures anyway, so rubberband can be redrawn
			 */
				XQueryPointer( dpy, root,
					&root, &child, &root_x, &root_y, 
					&win_x, &win_y, &mask );
				COPYRECT(oldrect, currect);
				ASSIGNRECT(currect, initX, initY, root_x, 
					root_y, MINW, MINH);
			}
		}



		else if( pressed ) 
		{
		/*
		 * No events have occured.  If selection has
		 * begun, then  update info in the rectangle
		 * structures, so rubberband can be redrawn
		 */
			XQueryPointer(dpy, root,
				&root, &child, &root_x, &root_y, 
				&win_x, &win_y, &mask );
			COPYRECT(oldrect, currect);
			ASSIGNRECT(currect, initX, initY, root_x, root_y, MINW,
				MINH);
		}



		if( pressed && DIFFRECT(oldrect, currect) )
		{
		/*
		 * Rectangle has changed during selection, so
		 * redraw rubberband.
		 */
			XDrawRectangles(dpy, root, 
				gc, &oldrect, 1);
			XDrawRectangles( dpy, root, 
				gc, &currect, 1 );

#		ifdef SHOWCOORDS
			text[intpos] = (currect.width/100) + '0';
			text[intpos+1] = ((currect.width / 10) % 10) +'0';
			text[intpos+2] = (currect.width % 10) +'0';
			text[intpos+4] = (currect.height/100) + '0';
			text[intpos+5] = ((currect.height / 10) % 10) +'0';
			text[intpos+6] = (currect.height % 10) +'0';
			XDrawImageString(dpy, popup, popupgc, 1, 
				pfont->ascent+1, text, strlen(text));  
#		endif
		}
	}

/*
 * We're done now; make rectangle "blink", then erase rubber band box
 */
    if(pressed)
    {
		XFillRectangles( dpy,root, gc, &currect, 1 );
		XFillRectangles( dpy,root, gc, &currect, 1 );
		XDrawRectangles( dpy,root, gc, &currect, 1 );
    }

/*
 * Free up resources and return (xrect holds final selection info
 * Since we free the server, things may change between here and 
 * PrintScreenRect.  Maybe in V1.n this routine can be called from there.
 */
#ifdef SHOWCOORDS
    XDestroyWindow(dpy, popup); 
    XFreeFont(dpy, pfont);  
#endif
    eventmask = NoEventMask;
    XSelectInput(dpy,root, eventmask);
    XUngrabPointer(dpy, CurrentTime);
#ifndef GRAB
    XUngrabServer(dpy); 
#endif
    XFreeCursor(dpy, cursor);
    XFreeGC(dpy,gc);
    XSync(dpy,0);	/* do discard input events */  
    return( Normal );
}

