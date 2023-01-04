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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/pause.c,v 1.1.4.2 1993/06/25 18:36:30 Paul_Henderson Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include "smdata.h"
#include "smconstants.h"
#include <stdio.h>

static Widget cover = NULL;
static Widget cover_container = NULL;
static Widget cover_dps = NULL;
static Widget box = NULL;
static Widget pass_ok = NULL;
static Widget pass_text = NULL;
static Widget pass_label = NULL;
static Widget	*multiCover = NULL;
static Widget	*multiCover_container = NULL;
static Widget	*multiCover_dps = NULL;
extern OptionsRec options;

XtAccelerators text_accelerators_parsed;
char text_accelerator_table[] =
   "#override<Key>Return : ArmAndActivate()";

extern  void    VisEvent();


#ifdef DODPS
#  ifdef osf1
#    include "XDPSlib.h"
#    include "dpsXclient.h"
#  else
#    include <DPS/XDPSlib.h>
#    include <DPS/dpsXclient.h>
#  endif

DPSContext *ctx = NULL;
#endif /* DODPS */

FILE *PSerrorfile = NULL;
XtIntervalId timeout = NULL;
Cursor blankCursor = NULL;
Boolean raised = FALSE;
Boolean wait_for_pass = FALSE;

#define PSERRORFILE "/tmp/DXsession.PSerr"
#define IDLETIME 10

/* DoPauseGrab - do the keyboard and pointer grabs for the password
 *		 dialog box when an expose event occurs.  Doing it
 *		 right after the popup can result in the grab failing.
 */
XtEventHandler DoPauseGrab(widget, client_data, event, cont)
Widget	widget;
XtPointer client_data;
XEvent *event;
Boolean *cont;
{
    if ((widget == box) && (event->type == Expose)) {
        if (XtGrabKeyboard(pass_text, False, GrabModeAsync,
			   GrabModeAsync, CurrentTime)
	     == GrabSuccess)
	{
            XtRemoveEventHandler(box, ExposureMask, False, DoPauseGrab, NULL);
    	    XSetInputFocus(XtDisplay(pass_text), XtWindow(pass_text), 
	                   RevertToNone, CurrentTime);
    	    XtGrabPointer(box, True, Button1Mask, GrabModeAsync,
			  GrabModeAsync, None, None, CurrentTime);
	}
    }
}

static void idleHandler(closure, id)
XtPointer closure;
XtIntervalId* id;
{
  if (wait_for_pass) {
    /* don't want the password box to disappear during the password
     * check Or while the password incorrect box is unacknowledged.
     * reset the timeout to try again later
     */
    if (smdata.err_ack) wait_for_pass = FALSE;
    timeout = XtAddTimeOut(IDLETIME*1000, idleHandler, 0);
    return;
  }

  if (blankCursor == NULL) {
    XGCValues gcv;
    Pixmap p;
    GC pgc;
    XColor xc;

    p = XCreatePixmap(XtDisplay(cover), XtWindow(cover), 32, 32, 1);
    gcv.function = GXcopy;
    gcv.fill_style = FillSolid;
    gcv.foreground = 0;
    pgc = XCreateGC(XtDisplay(cover), p,
		    GCForeground | GCFunction | GCFillStyle, &gcv);
    XFillRectangle(XtDisplay(cover), p, pgc, 0, 0, 32, 32);
    xc.red = xc.green = xc.blue = 0;
    blankCursor = XCreatePixmapCursor(XtDisplay(cover), p, p, &xc,
                                       &xc, 0, 0);
    XFreeGC(XtDisplay(cover), pgc);
    XFreePixmap(XtDisplay(cover), p);
  }

  XDefineCursor(XtDisplay(cover), XtWindow(cover), blankCursor);
  XLowerWindow(XtDisplay(box), XtWindow(box));
  timeout = NULL;
  raised = FALSE;
}

static void pauseHandler(w, client_data, event)
Widget w;
caddr_t client_data;
XEvent *event;
{
  if (!raised) {
    XUndefineCursor(XtDisplay(cover), XtWindow(cover));
    XRaiseWindow(XtDisplay(box), XtWindow(box));
    raised = TRUE;
  }
  if (timeout) XtRemoveTimeOut(timeout);
  timeout = XtAddTimeOut(IDLETIME*1000, idleHandler, 0);
}
  
#ifdef DODPS
static void TextOut(ctx, buffer, count)
    DPSContext ctx;
    char *buffer;
    unsigned count;
{
  if (PSerrorfile == NULL) {
    PSerrorfile = fopen(PSERRORFILE, "w");
  }
  fwrite(buffer, 1, count, PSerrorfile);
  fflush(PSerrorfile);
}

static void _doDPS (dpy, win, widget, idx)
  Display *dpy;
  Window win;
  Widget widget;
  int idx;
{
  FILE *source;
  GC    gc;
  int   gcid;
  int   colorArray[12];
  XStandardColormap grayRamp;
  long mask;
  XSetWindowAttributes xswa;
  int i, num_screens;
  
  xswa.bit_gravity = SouthWestGravity;
  xswa.backing_store = WhenMapped;
  mask = CWBitGravity | CWBackingStore;
  XChangeWindowAttributes(dpy, win, mask, &xswa);

  if (ctx == NULL) {
	num_screens = XScreenCount(XtDisplay(cover_dps));
    	ctx = XtMalloc(sizeof(DPSContext) * num_screens);
	for (i=0; i<num_screens; i++) ctx[i] = NULL;
  }

  if (ctx[idx] == NULL) 
  	ctx[idx] = XDPSCreateSimpleContext(dpy, win, DefaultGC(dpy, 0), 0, 0,
		TextOut, DPSDefaultErrorProc, NULL);
  
  if (options.session_pausefile && strlen(options.session_pausefile) 
      && ctx[idx] && (source = fopen(options.session_pausefile, "r")))
  {
    char buf[1000], *ptr;
    XEvent ev;
    long mask;
    FILE *fid;
    Screen *screen;

    unlink(PSERRORFILE);
    DPSSetTextBackstop(TextOut);
    DPSSetContext(ctx[idx]);
    
    screen = XtScreen(widget);
    gc = DefaultGCOfScreen(screen);
    gcid = gc->gid;
    
   /*
    * Set up the color information for PostScript.  We don't want dithering of
    * the colors from the standard color cube -- we want the server to allocate
    * 'actual' colors for the specified colors.  In order for *any* drawing to
    * take place, though, there has to be at least a gray ramp.  So we create a
    * small one that just has black and white in it. We specify that the server
    * should allocate up to 5 actual colors.  The server is free to ignore this
    * and allocate more (it shouldn't need to) or not allocate any and just
    * dither black and white.  There's no way of telling what it will do.
    *
    * The information that passed in the colorArray looks funny (especially
    * the instances of '-1') because of the way XStandardColormap is defined.
    */
    
    grayRamp.colormap = DefaultColormapOfScreen(screen);
    grayRamp.base_pixel = BlackPixelOfScreen(screen);
    grayRamp.red_max = 1;
    grayRamp.red_mult
	= WhitePixelOfScreen(screen) - BlackPixelOfScreen(screen);
    grayRamp.green_max = grayRamp.green_mult = 0;
    grayRamp.blue_max = grayRamp.blue_mult = 0;
    
    colorArray[0] = grayRamp.red_max;	/* nGrays */
    colorArray[1] = grayRamp.red_mult;	/* grayMult */
    colorArray[2] = grayRamp.base_pixel;	/* firstGray */
    colorArray[3] = -1;			/* nReds */
    colorArray[4] = 0;			/* redmult */
    colorArray[5] = -1;			/* nGreens */
    colorArray[6] = 0;			/* greenmult */
    colorArray[7] = -1;			/* nBlues */
    colorArray[8] = 0;			/* bluemult */
    colorArray[9] = 0;			/* firstColor */
    colorArray[10] = grayRamp.colormap;
    colorArray[11] = 5;			/* actual */
    
    PSsetXgcdrawablecolor(gcid, win, 0, XHeightOfScreen(screen), colorArray);
    
    while (1) {
      mask = (1<<0) | (1<<dpy->fd);
      XFlush(dpy);
      if (mask & (1<<0)) {
	if (fgets(buf, 1000, source) == NULL) break;
	if (buf[0] == '#') { /* For including files. */
	  ptr = index(buf, '\n');
	  if (ptr) *ptr = '\0';
	  fid = fopen(buf + 1, "r");
	  if (fid == NULL) {
	    fprintf(stderr, "[%s not found.]\n", buf + 1);
	  } else {
	    while (fgets(buf, 1000, fid))
	      DPSWritePostScript(ctx[idx], buf, strlen(buf));
	    fclose(fid);
	  }
	} else if (buf[0] == '!') { /* Testing XDPSLCreateContextFromID. */
	  XDPSLCreateContextFromID(dpy, atoi(buf + 1), NULL);
	  XSync(dpy, 0);
	} else {
	  DPSWritePostScript(ctx[idx], buf, strlen(buf));
	}
      }
    }

    DPSFlushContext(ctx[idx]);
    fclose(source);
    
    XFlush(dpy);
  }    
}
  
static void doDPS(ignore)
     caddr_t ignore;
{
  int i, num_screens, dfl_screen;

   num_screens = XScreenCount(XtDisplay(cover_dps));
   dfl_screen = XDefaultScreen(XtDisplay(cover_dps));

   for (i=0; i<num_screens; i++) {
	if (i == dfl_screen) 
	    _doDPS(XtDisplay(cover_dps), XtWindow(cover_dps), cover_dps, i);
	else 
   	    _doDPS(XtDisplay(multiCover_dps[i]), XtWindow(multiCover_dps[i]),
		   multiCover_dps[i], i);
   }
}

static void undoDPS()
{
    int i, num_screens;

    num_screens = XScreenCount(XtDisplay(cover_dps));
    for (i=0; i<num_screens; i++) {
	if (ctx[i]) {
	    DPSDestroySpace(DPSSpaceFromContext(ctx[i]));
    	    if (PSerrorfile) fclose(PSerrorfile);
    	    ctx[i] = NULL;
	}
    }
}
#endif /* DODPS */
  
static Boolean check_pass(str)
char *str;
{
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	The callback for when the OK button is hit from the Pause
**	screen.  We need to verify that the password is correct.
**	This is done differently on VMS and ULTRIX.
**
**  FORMAL PARAMETERS:
**
**	str - The null terminated string for the password we are
**	      verifyint
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	1 = Password verified
**	0 = Password rejected
**
**  SIDE EFFECTS:
**
**--
**/
int status;


/*
 * Modified majorly.
 * If checkpass for uid fails, check for root_passwd if that option
 * is set in the defaults file.  If it succeeds, we are fine; if it fails,
 * we lose.  If that option is not set, then only the uid check is valid
 * and that has failed so, we return false...
 */

if (checkpass(getuid(), str, &status)==FALSE) {
  if (options.session_rootpasswd) 
    {
      if (root_checkpass(str, &status)==FALSE) 
	{
	  return_focus = pass_text;
	  put_pause_error(0, status);
	  return(FALSE);
	}
      else 
	{
          return(TRUE);
	}
      
    }
  else
    {
      return_focus = pass_text;
      put_pause_error(0, status);
      return(FALSE);
    }
  
}
else 
  {
    return(TRUE);
  }

}

static void
cancel_callback(wid, reason, arg)
Widget wid;
int reason;
caddr_t arg;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the Clear button is hit from the Pause session
**	dialog box.  We need to reset all of the password fields
**	to NULL.  For VMS this includes a second password.
**
**  FORMAL PARAMETERS:
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
int len;
char buf[1];
char	*value;

buf[0] = 0;
/* Get the current value of the password field */
value = (char *)XmTextGetString(pass_text);
len  = strlen(value);
/* Replace it with NULL */
XmTextReplace(pass_text, 0, len, buf);
XtFree(value);
}
 
static void
pause_callback(wid, reason, arg)
Widget wid;
int reason;
caddr_t arg;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Callback for the OK button.  For ultrix, just verify the
**	password.  For VMS, check if we have a second password which
**	needs verifying.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	We should display an error box here to let the user know
**	when the password is incorrect.
**--
**/
{ 
register char *str = (char *)XmTextGetString(pass_text);

    /* don't want the password window disappearing now */
    wait_for_pass = TRUE;

    if(!check_pass(str)) {
	cancel_callback(wid, reason, arg);
	return;
    }

    /* If the password was correct, then clear the fields, take down the
       pause screen window and ungrab the keyboard and pointer */
    wait_for_pass = FALSE;
    cancel_callback(wid, reason, arg);
    XtRemoveEventHandler(cover, XtAllEvents, True, VisEvent, 0);

    XtPopdown(cover);
    MultiheadDown(XtDisplay(cover));
    XtUngrabPointer(cover, CurrentTime);
    XtUngrabKeyboard(cover, CurrentTime);

   /* Ungrab the server */

    XUngrabServer(XtDisplay(cover));

#ifdef DODPS
    if (options.session_pausefile && strlen(options.session_pausefile)) {
      XtRemoveEventHandler(cover_dps, XtAllEvents, True, pauseHandler, 0);
      XtRemoveEventHandler(box, XtAllEvents, True, pauseHandler, 0);
      XtRemoveEventHandler(pass_text, XtAllEvents, True, pauseHandler, 0);
      if (timeout) XtRemoveTimeOut(timeout);
      undoDPS();
    }
#endif /* DODPS */
    XtFree(str);
}
 
void
pause_session(w, arg, reason) 
Widget *w;
caddr_t arg;
Widget *reason;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the pause screen if this is the first time, otherwise
**	just pop it up.  Grab the keyboard and pointer for security.
**
**  FORMAL PARAMETERS:
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
static Boolean first = TRUE;
Widget root = smdata.toplevel;
Arg	arglist[6];
int width, height;
static Arg args[] = {
    { XmNwidth, NULL },
    { XmNheight, NULL },
    { XmNx, 0 },
    { XmNy, 0 },
    { XmNgeometry, (XtArgVal)"=+0+0" },
    { XmNallowShellResize, TRUE },
    { XmNsaveUnder, FALSE },
    { XmNborderWidth, 1 },
    { NULL, NULL }, /* extra slot for setting background */
};
Widget label, pass2_text,pass2_label;
Position dix, diy;
Pixel tbackground;
Pixel tforeground;

static MrmRegisterArg reglist[] = {
        { "PauseCancelCallback", (caddr_t) cancel_callback },
        { "PauseOkCallback", (caddr_t) pause_callback },
        { "passtext_id", (caddr_t) &pass_text },
        { "passlabel_id", (caddr_t) &pass_label },
        { "scdpasstext_id", (caddr_t) &smdata.pass2_text },
        { "scdpasslabel_id", (caddr_t) &smdata.pass2_label },
        { "pauselabel", (caddr_t) &smdata.pause_label },
        { "pauseok_id", (caddr_t) &pass_ok },
        };

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    if (cover) XtDestroyWidget(cover);
  
    /* Whole screen size */
    width = DisplayWidth(XtDisplay(root),
			 DefaultScreen(XtDisplay(root)));
    height = DisplayHeight(XtDisplay(root),
		           DefaultScreen(XtDisplay(root)));
    args[0].value = (XtArgVal)width;
    args[1].value = (XtArgVal)height;

    cover = XtCreatePopupShell("pause", overrideShellWidgetClass, 
   			       root, args, XtNumber(args)-1);

    MrmFetchWidgetOverride(s_DRMHierarchy, "PauseContainer", cover,
		           "PauseContainer", args, XtNumber(args)-1,
		           &cover_container, &drm_dummy_class);
    XtManageChild(cover_container);

#ifdef DODPS
    if (options.session_pausefile && strlen(options.session_pausefile)) {
        XtSetArg(args[XtNumber(args)-1], XmNbackground,
 		 BlackPixelOfScreen(XtScreen(root)));
	MrmFetchWidgetOverride(s_DRMHierarchy, "PauseContainer",
				cover_container, "PauseDPS",
				args, XtNumber(args),
				&cover_dps, &drm_dummy_class);
        XtManageChild(cover_dps);
    }
#endif /* DODPS */

    MrmFetchWidget(s_DRMHierarchy, "PauseDialog", cover_container,
		   &box, &drm_dummy_class);
    XtSetMappedWhenManaged(box, False);
    XtManageChild(box);

    XtSetArg(arglist[0], XmNlabelString, 
    	     XmStringCreate(smsetup.pause_text, def_char_set));
    XtSetValues(smdata.pause_label, arglist, 1);
	
    HackText(pass_text);

    XtSetArg(arglist[0], XmNbackground, &tbackground);
    XtGetValues(pass_text, arglist, 1);
    XtSetArg(arglist[0], XmNforeground, tbackground);
    XtSetArg(arglist[1], XmNautoShowCursorPosition, False);
    XtSetArg(arglist[2], XmNcolumns, 30); 
    XtSetArg(arglist[3], XmNrows, 1); 
    XtSetValues(pass_text, arglist, 4);

    text_accelerators_parsed = XtParseAcceleratorTable(text_accelerator_table);
    XtSetArg(arglist[0], XmNaccelerators, text_accelerators_parsed); 
    XtSetValues(pass_ok, arglist, 1);
    XtInstallAccelerators(pass_text, pass_ok);

    /* shouldn't have to do this, but it seems to force correct
     * geometry calculations... 
     */
    XtSetArg(arglist[0], XmNwidth, 350); 
    XtSetArg(arglist[1], XmNheight, 150); 
    XtSetValues(box, arglist, 2);

    /* move the box to the center of the screen */
    get_center_coor(XtDisplay(box), (Widget)0, box, &dix, &diy);
    XtSetArg(arglist[0], XmNx, dix);
    XtSetArg(arglist[1], XmNy, diy);
    XtSetValues(box, arglist, 2);

    XtAddEventHandler(cover, VisibilityChangeMask, False, VisEvent, 0);
    XtAddEventHandler(box, ExposureMask, False, DoPauseGrab, NULL);
    XtSetMappedWhenManaged(box, True);

    XtPopup(cover, XtGrabNone);
    MultiheadUp(XtDisplay(pass_text), first);
    first = 0;

#ifdef DODPS
    if (options.session_pausefile && strlen(options.session_pausefile)) {
        XtAppAddWorkProc(XtWidgetToApplicationContext(cover), doDPS, NULL);
        XtAddEventHandler(cover_dps,
			  PointerMotionMask|KeyPressMask|ButtonPressMask,
		          False, pauseHandler, 0);
        XtAddEventHandler(box, PointerMotionMask|KeyPressMask|ButtonPressMask,
		          False, pauseHandler, 0);
        XtAddEventHandler(pass_text,
		          PointerMotionMask|KeyPressMask|ButtonPressMask,
		          False, pauseHandler, 0);
        idleHandler(NULL, NULL);
    }
#endif /* DODPS */

   /* grab the server */

  XGrabServer(XtDisplay(cover));


}

int MultiheadUp(display,first)
Display	*display;
int first;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Pop up a full screen cover for each screen on the system
**
**  FORMAL PARAMETERS:
**
**	display - Connection number
**	first = 1 if this is the first time we call the routine - we
**		need to create the covers.
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
int num_screens, i, num_args;
static Arg args[] = {
    { XmNwidth, 0 },
    { XmNheight, 0 },
    { XmNoverrideRedirect, True },
    { XmNborderWidth, 1 },
    { XmNx, 0 },
    { XmNy, 0 },
    { XmNgeometry, (XtArgVal)"=+0+0" },
    { XmNscreen, NULL },
};

    if (windowsetup.saver) {
      XSetScreenSaver(display, options.session_pausesaver,
		      options.session_pausesaver,
		      PreferBlanking, AllowExposures);
    }

    if ((num_screens = XScreenCount(display)) == 1) return;

    if (first == 1) {
    	multiCover = (Widget *)XtMalloc(sizeof(Widget)*num_screens);
    	multiCover_container = (Widget *)XtMalloc(sizeof(Widget)*num_screens);
#ifdef DODPS
    	multiCover_dps = (Widget *)XtMalloc(sizeof(Widget)*num_screens);
#endif /* DODPS */
    }

    for (i = 0; i < num_screens; i++) {
	if (i != XDefaultScreen(display)) {
	    if (first == 1) {
	        XtSetArg(args[0], XmNwidth, DisplayWidth(display, i));
	        XtSetArg(args[1], XmNheight, DisplayHeight(display, i));
		XtSetArg(args[7], XmNscreen, ScreenOfDisplay(display, i));

		multiCover[i] = XtCreatePopupShell("pause",
			overrideShellWidgetClass, 
			smdata.toplevel, args, XtNumber(args));
	
        	MrmFetchWidgetOverride(s_DRMHierarchy, "PauseContainer",
			multiCover[i], "PauseContainer", args, XtNumber(args),
			&multiCover_container[i], &drm_dummy_class);

#ifdef DODPS
        	if (options.session_pausefile
		    && strlen(options.session_pausefile))
		{
	  		MrmFetchWidgetOverride(s_DRMHierarchy, "PauseContainer",
				multiCover_container[i], "PauseDPS", args,
				XtNumber(args), &multiCover_dps[i],
				&drm_dummy_class);
	                XtManageChild(multiCover_dps[i]);
		}
#endif /* DODPS */

	        XtManageChild(multiCover_container[i]);
	    }
	    XtPopup(multiCover[i], XtGrabNone);
#ifdef DODPS
            if (options.session_pausefile && strlen(options.session_pausefile))
            	XtAddEventHandler(multiCover_dps[i],
			      PointerMotionMask|KeyPressMask|ButtonPressMask,
			      False, pauseHandler, 0);
#endif /* DODPS */
	}
    }
}

int MultiheadDown(display)
Display	*display;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Remove the cover on all screens.
**
**  FORMAL PARAMETERS:
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
int num_screens,i;
Window	win;

    if (windowsetup.saver) {
      XSetScreenSaver(display, windowsetup.saver_seconds*60,
		      windowsetup.saver_seconds*60,
		      PreferBlanking, AllowExposures);
    }
    if ((num_screens = XScreenCount(display)) == 1) return;

    for (i=0; i<num_screens; i++)
    {
        if (i != XDefaultScreen(display)) {
#ifdef DODPS
            if (options.session_pausefile && strlen(options.session_pausefile))
            	XtRemoveEventHandler(multiCover_dps[i],
			      	 XtAllEvents, True, pauseHandler, 0);
#endif /* DODPS */
	    XtPopdown(multiCover[i]);
	}
    }
}

void    VisEvent(w, p_data, event)
Widget  w;
caddr_t	p_data;
XVisibilityEvent        *event;
/*---
!
!   Called whenever the Visibility of the login box changes.
!   If the user puts up a full screen logo, we need to make sure
!   this box stays on top.
!
! Inputs:
!           w               Widget with current focus
!           event           Event

! Outputs:
!
!---
*/
{
    if (!XtIsManaged(box)) return;
    
    if (smdata.err_window_id && XtIsManaged(smdata.err_window_id))
        return;

    XRaiseWindow(XtDisplay(w), event->window);
}
