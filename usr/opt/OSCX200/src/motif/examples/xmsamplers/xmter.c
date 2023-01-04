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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: xmter.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 22:02:42 $";
#endif
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: xmter.c,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 22:02:42 $"
#endif
#endif
/***************************************************************************
*  Xmter.
*   This demo illustrates the use of the R4 shape extension and Motif.
*   Author: Daniel Dardailler. 
*
*   Effects by: Andrew deBlois
***************************************************************************/
#define CLASS_NAME "XMdemos"   

#include <stdio.h>
#include <math.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/BulletinB.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <X11/extensions/shape.h>  /* R4 header required */
#include <Xm/MwmUtil.h>      /* mwm decoration */

#include "terre.xbm"               /* can cause compilation problem with
				      some gcc version, due of the size of 
				      the bitmap array */
/*** Application default stuff */
typedef struct {
  int          speed;
  XtIntervalId timeoutID;
  Pixel        foreground ;
  Pixel        background ;
  Boolean      persue;
} ApplicationData, *ApplicationDataPtr;
ApplicationData AppData;

#define PERSUE_ON_VALUE   92
#define PERSUE_OFF_VALUE -92
#define XmNspeed  "speed"
#define XmCSpeed  "Speed"
#define XmNpersue "persue"
#define XmCPersue "Persue"
static XtResource resources[] = {
    { 
	XmNspeed, XmCSpeed, XmRInt, sizeof(int),
	XtOffsetOf (ApplicationData, speed), 
	XmRImmediate, (caddr_t) 50 },

    /* overwrite the default colors (doing that will lead to weird
       behavior if you specify -fg or -bg on the command line, but
       I really don't care, it's easier to use the standard names) */
    {
	XmNforeground, XmCForeground, XmRPixel, sizeof (Pixel),
	XtOffsetOf (ApplicationData, foreground),
	XmRString, "brown"},
    {
	XmNbackground, XmCBackground, XmRPixel, sizeof (Pixel),
	XtOffsetOf (ApplicationData, background),
	XmRString, "turquoise"},
    {
        XmNpersue, XmCPersue, XmRBoolean, sizeof (Boolean),
	XtOffsetOf (ApplicationData, persue),
	XmRImmediate, (caddr_t) 0}
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
static void    NextBitmap ();

int delayInterval (speed)
/**************************/
int speed;
{
  double maxDelay = 1000.0;
  double val      = (double)(abs(speed));


  return (int)( maxDelay / val );
} 


main(argc, argv) int argc; char **argv ;
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
    AppData.timeoutID = XtAppAddTimeOut(app_con, delayInterval(AppData.speed), NextBitmap, NULL);
    
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

static void NextBitmap (client_data, id)
/****************************************************/
XtPointer client_data;
XtIntervalId *id;
{


  AppData.timeoutID = 0;

  if (AppData.speed != 0)
    {
      if (XtIsRealized(draw))
	{
	  Position x, y;

	  if (AppData.speed > 0) n = (n>28)?0:n+1;
	  else                   n = (n>0)?n-1:29;

	  XCopyPlane (XtDisplay(draw), pterre, XtWindow(draw), gc,
		      0, 64*n, 64, 64, 0, 0, 1);

	  if (AppData.persue)
	    {
	      Position     x, y;
	      int          mx, my, ji;
	      Window       jw;
	      unsigned int jk;

	      XtVaGetValues(toplevel, XmNx, &x, XmNy, &y, NULL);
	      XQueryPointer(XtDisplay(toplevel), XtWindow(toplevel),
			    &jw, &jw, &mx, &my, &ji, &ji, &jk);
	      XtVaSetValues(toplevel,XmNx, x+((mx-x)/10),XmNy, y+((my-y)/10),NULL);
	    }
	}

      AppData.timeoutID = XtAppAddTimeOut(app_con, delayInterval(AppData.speed), NextBitmap, NULL);
    }
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
  XmDrawingAreaCallbackStruct * dacb = (XmDrawingAreaCallbackStruct *) callback_data ;
  static Widget speed_dialog = NULL ;
  Widget speed_scale ;
  Arg args[15] ;
  Cardinal n;
  XmString title_string;
  

  if ((dacb->event->type == ButtonPress) &&
       (dacb->event->xbutton.button == Button3))
    {
      if (speed_dialog == NULL) {
	speed_dialog = XmCreateFormDialog(toplevel, 
					  "Speed control",
					  NULL, 0) ;
	n = 0 ;
	title_string = XmStringCreateLtoR("rotation speed", 
					  (XmStringCharSet) XmSTRING_DEFAULT_CHARSET);
	XtSetArg(args[n], XmNtitleString,     title_string);  n++ ;
	XtSetArg(args[n], XmNshowValue,       True); n++ ;
	XtSetArg(args[n], XmNvalue,           AppData.speed); n++ ;
	XtSetArg(args[n], XmNorientation,     XmHORIZONTAL); n++ ;
	XtSetArg(args[n], XmNleftAttachment,  XmATTACH_POSITION); n++;
	XtSetArg(args[n], XmNleftPosition,    10); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
	XtSetArg(args[n], XmNrightPosition,   90); n++;
	XtSetArg(args[n], XmNminimum,         -100); n++;
	XtSetArg(args[n], XmNmaximum,         100); n++;
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
      if (AppData.speed == PERSUE_ON_VALUE)  AppData.persue = TRUE;
        else
      if (AppData.speed == PERSUE_OFF_VALUE) AppData.persue = FALSE;
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
    {
      AppData.speed = scb->value;
      if (AppData.timeoutID != 0) XtRemoveTimeOut(AppData.timeoutID);
      XtAppAddTimeOut(app_con, delayInterval(AppData.speed), NextBitmap, NULL);
    }
  else
    AppData.speed = scb->value ;
}


static void Syntax(com) char * com ;
/**********************************/
{
    fprintf(stderr, "%s understands all standard Xt options, plus:\n",com);
    fprintf(stderr, "       -speed:        0-100  (earth rotation speed)\n");
}


