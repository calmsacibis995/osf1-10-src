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
/* $XConsortium: init_win.c,v 5.2 91/04/14 11:13:45 rws Exp $ */

/***********************************************************
Copyright 1989,1990, 1991 by Sun Microsystems, Inc. and the X Consortium.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Sun Microsystems,
the X Consortium, and MIT not be used in advertising or publicity 
pertaining to distribution of the software without specific, written 
prior permission.  

SUN MICROSYSTEMS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT 
SHALL SUN MICROSYSTEMS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#define	WIN_X	200
#define	WIN_Y  	200
#define	WIN_W  	400
#define	WIN_H  	400

#define	CONNECT_TO_DISPLAY(name) \
	if (!(appl_display = XOpenDisplay(name))) { \
		perror( "Cannot open display\n"); \
		exit(-1); \
	} 

Display *appl_display;
Window appl_window;

void init_window();

void
init_window()
{
	XSetWindowAttributes xswa;
	Colormap map;
	unsigned int bw = 1;
	char *display_name = NULL;
	int bg_pix, bd_pix;
	Visual visual;
	XEvent exposureEvent;

	CONNECT_TO_DISPLAY(display_name);

	map = XDefaultColormap(appl_display, DefaultScreen(appl_display));
			 
	bg_pix = BlackPixel(appl_display, DefaultScreen(appl_display));
	bd_pix = WhitePixel(appl_display, DefaultScreen(appl_display));
						  
	xswa.backing_store = NotUseful;
	xswa.event_mask = ExposureMask | StructureNotifyMask;
	xswa.background_pixel = bg_pix;
	xswa.border_pixel = bd_pix;
	visual.visualid = CopyFromParent;
	appl_window = XCreateWindow(appl_display,
		RootWindow(appl_display, DefaultScreen(appl_display)),
		WIN_X, WIN_Y, WIN_W, WIN_H, bw,
		DefaultDepth(appl_display, DefaultScreen(appl_display)),
		InputOutput, &visual,
		CWEventMask | CWBackingStore | CWBorderPixel | CWBackPixel,
		&xswa);

	XChangeProperty(appl_display,
		appl_window, XA_WM_NAME, XA_STRING, 8,
		PropModeReplace, (unsigned char *)"BeachBall", 5);
	XMapWindow(appl_display, appl_window);
						 
	/** sync and wait for exposure event **/
	XSync(appl_display, 0);
	XWindowEvent(appl_display,
		appl_window, ExposureMask, &exposureEvent);
}
