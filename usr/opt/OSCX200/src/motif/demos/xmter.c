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
/************************************************************************
# Copyright 1991 OPEN SOFTWARE FOUNDATION, INC.
#
# Permission is hereby granted to use, copy, modify and freely distribute
# the software in this file and its documentation for any purpose without
# fee, provided that the above copyright notice appears in all copies and
# that both the copyright notice and this permission notice appear in
# supporting documentation.  Further, provided that the name of Open
# Software Foundation, Inc. ("OSF") not be used in advertising or
# publicity pertaining to distribution of the software without prior
# written permission from OSF.  OSF makes no representations about the
# suitability of this software for any purpose.  It is provided "as is"
# without express or implied warranty.
#
# Open Software Foundation is a trademark of The Open Software Foundation, Inc.
# OSF is a trademark of Open Software Foundation, Inc.
# OSF/Motif is a trademark of Open Software Foundation, Inc.
# Motif is a trademark of Open Software Foundation, Inc.
************************************************************************/

/***************************************************************************
*  Xmter.
*   This demo illustrates the use of the R4 shape extension and Motif.
*   Author: Daniel Dardailler. 
*
***************************************************************************/
#define CLASS_NAME "XMdemos"   

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/BulletinB.h>
#include <Xm/Scale.h>
#include <X11/extensions/shape.h>  /* R4 header required */
#include <Xm/MwmUtil.h>      /* mwm decoration */

#include "terre.xbm"               /* can cause compilation problem with
				      some gcc version, due of the size of 
				      the bitmap array */
/*** Application default stuff */
typedef struct {
  int   speed;
  Pixel foreground ;
  Pixel background ;
} ApplicationData, *ApplicationDataPtr;
ApplicationData AppData;

#define XmNspeed "speed"
#define XmCSpeed "Speed"
static XtResource resources[] = {
    { 
	XmNspeed, XmCSpeed, XmRInt, sizeof(int),
	XtOffset(ApplicationDataPtr, speed), 
	XmRImmediate, (caddr_t) 75 },

    /* overwrite the default colors (doing that will lead to weird
       behavior if you specify -fg or -bg on the command line, but
       I really don't care, it's easier to use the standard names) */
    {
	XmNforeground, XmCForeground, XmRPixel, sizeof (Pixel),
	XtOffset (ApplicationDataPtr, foreground),
	XmRString, "brown"},
    {
	XmNbackground, XmCBackground, XmRPixel, sizeof (Pixel),
	XtOffset (ApplicationDataPtr, background),
	XmRString, "turquoise"}
};

static XrmOptionDescRec options[] = {
    {"-speed", "*speed", XrmoptionSepArg, NULL},
} ;
/*** end of Application default stuff */

static XtAppContext app_con;
static Widget       toplevel, draw ;
static Pixmap       pterre ;
static GC           gc ;
static int          n ;

static void    Syntax() ;
static void    input_callback();
static void    expose_callback();
static Boolean fstep() ;
static void    speed_callback() ;

main(argc, argv) unsigned int argc; char **argv ;
/**************************************/
{
    Arg args[10] ;
    Cardinal n;

    XGCValues values;
    XtGCMask  valueMask;
    Pixmap shape_mask ;
    XGCValues	xgcv;
    GC shapeGC ;
    int shape_event_base, 
        shape_error_base; /* just used as dummy parameters */

    /* Initialize, using my options */
    toplevel = XtAppInitialize(&app_con, CLASS_NAME, 
			       options, XtNumber(options),
			       &argc, argv, NULL, NULL, 0);

    /* if a bad option was reached, XtAppInitialize should let it */
    if (argc > 1) Syntax(CLASS_NAME) ;

    /* read the programmer app default */
    XtGetApplicationResources(toplevel,
			      (XtPointer)&AppData,
			      resources,
			      XtNumber(resources),
			      NULL,
			      0);

    /* create a fixed size drawing area + callback for dialog popup */
    n = 0 ;
    XtSetArg(args[n], XmNwidth, 64);  n++ ;
    XtSetArg(args[n], XmNheight, 64); n++ ;
    draw = XmCreateDrawingArea (toplevel, "draw", args, n);
    XtManageChild(draw);
    XtAddCallback(draw,XmNinputCallback,(XtCallbackProc)input_callback,NULL);
    XtAddCallback(draw,XmNexposeCallback,(XtCallbackProc)expose_callback,NULL);

    XtRealizeWidget(toplevel);

    /* create the big bitmap on the server */
    pterre = XCreateBitmapFromData (XtDisplay(toplevel),XtWindow(toplevel),
				    terre_bits,terre_width,terre_height) ;

    /* create a GC for the earth colors */
    valueMask = GCForeground | GCBackground ;
    values.foreground = AppData.foreground ;
    values.background = AppData.background ;
    gc = XtGetGC ((Widget) toplevel, valueMask , &values);

    /* if there is a shape extension, use it */
    if (XShapeQueryExtension (XtDisplay (draw),
			      &shape_event_base, 
			      &shape_error_base)) {
	shape_mask = XCreatePixmap (XtDisplay (draw), XtWindow (draw),
				    64, 64, 1);
	shapeGC = XCreateGC (XtDisplay (draw), shape_mask, 0, &xgcv);

	/* erase the pixmap as a mask */
	XSetForeground (XtDisplay (draw), shapeGC, 0);
	XFillRectangle (XtDisplay (draw), shape_mask, shapeGC, 0,0, 64,64);
	XSetForeground (XtDisplay (draw), shapeGC, 1);
	/* draw the bounding/clipping shape : a circle */
	XFillArc(XtDisplay (draw), shape_mask, shapeGC, 0,0, 64,64, 0,23040);
	/* shape the parent for event managing and the widget for drawing */
	XShapeCombineMask (XtDisplay (toplevel), XtWindow (toplevel), 
			   ShapeBounding, 0,0, shape_mask, ShapeSet);
	XShapeCombineMask (XtDisplay (draw), XtWindow (draw), 
			   ShapeClip, 0,0, shape_mask, ShapeSet);
	XFreePixmap (XtDisplay (draw), shape_mask);
	/* don't ask me why I use alternatively draw and toplevel as
	   a parameter of XtDisplay, it doesn't matter at all */
    } else {
	fprintf(stderr, 
		"Bad news: there is no shape extension on your server...\n");
    }

    /* animation done in background */
    XtAppAddWorkProc(app_con, fstep, NULL);
    
    if (XmIsMotifWMRunning(toplevel)) {
	MwmHints     mwm_set_hints ;
	Atom         mwm_hints ;
	XWMHints     wm_set_hints;
	XUnmapEvent  Unmap_ev;

	mwm_hints = XmInternAtom(XtDisplay(toplevel), 
				 "_MOTIF_WM_HINTS", False);
	mwm_set_hints.flags = MWM_HINTS_DECORATIONS ;
	mwm_set_hints.decorations = 
	    MWM_DECOR_ALL | MWM_DECOR_BORDER |
		MWM_DECOR_RESIZEH | MWM_DECOR_TITLE |MWM_DECOR_MENU|
		    MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE ;
	XChangeProperty(XtDisplay(toplevel),XtWindow(toplevel),
			mwm_hints,mwm_hints,
			32, PropModeReplace,
			(unsigned char *)&mwm_set_hints, 
			sizeof(MwmHints)/sizeof(int));
	XUnmapWindow(XtDisplay(toplevel),XtWindow(toplevel));
        Unmap_ev.type = UnmapNotify;
        Unmap_ev.event = DefaultRootWindow(XtDisplay(toplevel));
        Unmap_ev.window = XtWindow(toplevel);
        Unmap_ev.from_configure = False;
        XSendEvent(XtDisplay(toplevel), 
		   DefaultRootWindow(XtDisplay(toplevel)), False,
		   (SubstructureNotifyMask|SubstructureRedirectMask), 
		   (XEvent*)&Unmap_ev);

        wm_set_hints.flags = (StateHint);
        wm_set_hints.initial_state = NormalState;
	XSetWMHints(XtDisplay(toplevel), XtWindow(toplevel), 
		    &wm_set_hints);
	XMapWindow(XtDisplay(toplevel),XtWindow(toplevel));
    }

    XtAppMainLoop(app_con);
}

static Boolean fstep(client_data) char * client_data ;      
/****************************************************/
/* Animation motor, it uses the synchronous X request to slow the process, 
   it also slows the whole network, I know ... */
{
    int i ;

    XCopyPlane (XtDisplay(draw), pterre, XtWindow(draw), gc,
		0, 64*n, 64, 64, 0, 0, 1);
    for (i = AppData.speed ; i < 100 ; i++) XSync(XtDisplay(draw), False);
    n = (n>28)?0:n+1;
    return (AppData.speed == 0) ;
}

static void expose_callback(widget, tag, callback_data)
/****************************************************/
Widget widget ;
XtPointer tag ;
XtPointer callback_data ;
{
    XCopyPlane (XtDisplay(draw), pterre, XtWindow(draw), gc,
		0, 64*n, 64, 64, 0, 0, 1);
}

static void input_callback(widget, tag, callback_data)
/****************************************************/
Widget widget ;
XtPointer tag ;
XtPointer callback_data ;
{
    XmDrawingAreaCallbackStruct * dacb = 
	(XmDrawingAreaCallbackStruct *) callback_data ;
    static Widget speed_dialog = NULL ;
    Widget speed_scale ;
    Arg args[10] ;
    Cardinal n;
    XmString title_string;

    if ((dacb->event->type == ButtonPress) &&
	(dacb->event->xbutton.button == Button3)) {
	if (speed_dialog == NULL) {
	    speed_dialog = XmCreateBulletinBoardDialog(toplevel, 
						       "Speed control",
						       NULL, 0) ;
	    n = 0 ;
	    title_string = XmStringCreateLtoR("rotation speed", 
			      (XmStringCharSet) XmSTRING_DEFAULT_CHARSET);
	    XtSetArg(args[n], XmNtitleString, title_string);  n++ ;
	    XtSetArg(args[n], XmNshowValue, True); n++ ;
	    XtSetArg(args[n], XmNvalue, AppData.speed); n++ ;
	    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++ ;
	    speed_scale = XmCreateScale(speed_dialog,
					"speed_scale",
	 				args, n) ;
	    XtAddCallback(speed_scale,XmNdragCallback,
			  (XtCallbackProc)speed_callback,NULL);
	    XtAddCallback(speed_scale,XmNvalueChangedCallback,
			  (XtCallbackProc)speed_callback,NULL);
	    XtManageChild(speed_scale);
	}
	XtManageChild(speed_dialog);
    }
}

static void speed_callback(widget, tag, callback_data)
/******************************************************/
Widget widget ;
XtPointer tag ;
XtPointer callback_data ;
{
    XmScaleCallbackStruct * scb = 
	(XmScaleCallbackStruct *) callback_data ;

    if ((AppData.speed == 0) && (scb->value != 0)) 
	XtAppAddWorkProc(app_con, fstep, NULL);
    AppData.speed = scb->value ;
}


static void Syntax(com) char * com ;
/**********************************/
{
    fprintf(stderr, "%s understands all standard Xt options, plus:\n",com);
    fprintf(stderr, "       -speed:        0-100  (earth rotation speed)\n");
}


