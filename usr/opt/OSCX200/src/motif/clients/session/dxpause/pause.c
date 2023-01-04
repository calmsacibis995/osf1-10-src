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
**      DECwindows Pause Screen; 
**
**  AUTHOR:
**
**      
**
**  ABSTRACT:
**
**      This module implements the pause windows which cover the
**	a systems n screens. These windows prevent a user from using
**	a workstation until a valid password is entered. 
**     
**  Edit History:
** 	12-oct-93 - pjw - Fixed ootb_bug 96 where pause would fail
**		on multi-headed systems when the heads were different
**		depths. This is an architectural issue. pause
**		Creates one top level shell and n children shells to 
**		obscure the displays. These children are inheriting the depth
**		resource from thier parent. Rather than implementing n toplevel
**		shells, the code now explicty sets the depth of each child to 
**		XDefaultDepth for each display. 
**
**--
*/

/* #def LOCAL_UID_PATH 1 */ 
/* sr - dxpause */


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/MessageBP.h>
#include <Xm/DialogS.h>

#include <Mrm/MrmPublic.h>


/* DRM hierarchy file */
static char *db_filename_vec[] = {"DXpause.uid"};

static int db_filename_num = (sizeof db_filename_vec/sizeof db_filename_vec[0]);


static int new_timeout;
static int interval;
static int prefer_blanking;
static int allow_exposures;

static Display *dpy = NULL;
static XtAppContext app_context = NULL;

static Widget toplevel = NULL;

static Widget *multiCover = NULL;
static Widget *multiCover_container = NULL;

static MrmType drm_dummy_class;
static MrmHierarchy s_DRMHierarchy;

static Widget cover = NULL;
static Widget cover_container = NULL;
static Widget cover_dps = NULL;
static Widget box = NULL;
static Widget pass_ok = NULL;
static Widget pass_text = NULL;
static Widget pass_label = NULL;
static Widget pauselabel = NULL;
static Widget	*multiCover_dps = NULL;
Widget  return_focus = 0;
Widget error_widget = NULL;

typedef struct {
    int pause_saver;
    char *root_passwd;
    char *pause_label;
    char *pause_error_message;
    Boolean grab_server;
} OptionsRec;

static OptionsRec options;

#define XtNpauseSaver   "pauseSaver"
#define XtCPauseSaver   "PauseSaver"
#define XtNrootPasswd   "rootPasswd"
#define XtCRootPasswd  "RootPasswd"
#define XtNpauseLabel   "pauseLabel"
#define XtCPauseLabel   "PauseLabel"
#define XtNpauseErrorMessage   "pauseErrorMessage"
#define XtCPauseErrorMessage   "PauseErrorMessage"
#define XtNgrabServer	"grabServer"
#define XtCGrabServer	"GrabServer"


static XtTranslations text_translations_parsed; 

static char text_translations_table [] = 
  {
    "#augment\n\
        <Key>Delete :		delete-previous-character()\n\
	:Ctrl<Key>r :	 	redraw-display()\n\
	:Ctrl<Key>u :		beginning-of-line() kill-to-end-of-line()\n\
	:Ctrl<Key>w :		delete-next-word()\n\
	Any<Key>:		self-insert()"
  };

XtAccelerators text_accelerators_parsed;
char text_accelerator_table[] =
   "#override<Key>Return : ArmAndActivate()";

#define ERR_TIME_INTERVAL	(5*1000)

static int default_timeout = 1;
static XtResource applicationResources[] = {
    {XtNpauseSaver, XtCPauseSaver, XtRInt, sizeof(int),
       XtOffsetOf(OptionsRec, pause_saver), XtRInt, (caddr_t) &default_timeout},
    {XtNrootPasswd, XtCRootPasswd, XtRBoolean, sizeof(Boolean),
     XtOffsetOf(OptionsRec, root_passwd), XtRString,
     FALSE},
    {XtNpauseLabel, XtCPauseLabel, XtRString, sizeof(char *),
     XtOffsetOf(OptionsRec, pause_label), XtRString,
     "Please enter your password"},
    {XtNpauseErrorMessage, XtCPauseErrorMessage, XtRString, sizeof(char *),
     XtOffsetOf(OptionsRec, pause_error_message), XtRString,
     "Incorrect password.  Try again."},
    {XtNgrabServer, XtCGrabServer, XtRBoolean, sizeof(Boolean),
     XtOffsetOf(OptionsRec, grab_server), XtRString,
     "False"}
};


FILE *PSerrorfile = NULL;
XtIntervalId timeout = NULL;
Cursor blankCursor = NULL;
Boolean raised = FALSE;
Boolean wait_for_pass = FALSE;
Boolean wait_error_ack = FALSE;

#define PSERRORFILE "/tmp/DXsession.PSerr"
#define IDLETIME 10



void widget_create_proc (w, tag, reason)
    Widget w;
    Widget *tag;
    unsigned int *reason;
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
}


/*---
!       This routine figures out the x and y coordinates to use to
!	center the widget "dialog_id" in the middle of "in_widget"
!       If "in_widget is null we use the whole screen
!
! Inputs:
!	display - display of widget
!	in_widget - widget to center in
!	dialog_id - widget to center
!	dix - return x position
!	diy - return y position
!---
*/
get_center_coor(display, in_widget, dialog_id, dix, diy)
Display	*display;
Widget	in_widget;
Widget	dialog_id;
Position *dix, *diy;
{
    Arg     arglist[2];
    Dimension width, height;
    Dimension scrwidth, scrheight;

    /* get the width and height of the dialog box */
    XtSetArg (arglist[0], XmNwidth, &width);
    XtSetArg (arglist[1], XmNheight, &height);
    XtGetValues (dialog_id, arglist, 2);
    
    /* if in_widget is null we want to center in the whole screen
     * otherwise we will center in the middle of "in_widget"
     */
    if (in_widget == (Widget)0)  {
    	scrwidth = XDisplayWidth(display, XDefaultScreen(display));
    	scrheight = XDisplayHeight(display, XDefaultScreen(display));
    }
    else {
    	XtSetArg (arglist[0], XmNwidth, &scrwidth);
    	XtSetArg (arglist[1], XmNheight, &scrheight);
    	XtGetValues (in_widget, arglist, 2);
    }
    
    *dix = ((scrwidth/2) - (width/2));
    *diy = ((scrheight/2) - (height/2));
}


XtEventHandler DoErrorGrab(widget, client_data, event, cont)
Widget	widget;
XtPointer client_data;
XEvent *event;
Boolean *cont;
{
    if ((widget == error_widget) && (event->type == Expose)) {
        if (XtGrabKeyboard(error_widget, False, GrabModeAsync,
			   GrabModeAsync, CurrentTime)
	     == GrabSuccess)
	  XtRemoveEventHandler(error_widget, ExposureMask, False,
			       (XtEventHandler)DoErrorGrab, NULL);
	  wait_error_ack = TRUE;
    }
}

void	restart(widget, tag, reason)
Widget	*widget;
caddr_t tag;
unsigned int reason;
{
     XtUnmanageChild(error_widget);
     XtUngrabKeyboard(error_widget, CurrentTime);
     wait_error_ack = FALSE;
     XtGrabKeyboard(pass_text, False,
		    GrabModeAsync, GrabModeAsync, CurrentTime);
     XSetInputFocus(XtDisplay(pass_text), XtWindow(pass_text), 
		    RevertToNone, CurrentTime);
 }


XtEventHandler ack_rcvd(widget, client_data, event, cont)
Widget  widget;
XtPointer client_data;
XEvent *event;
Boolean *cont;
{
    restart(widget, NULL, 0);
}



/* create the error window.  */
int	create_error(message)
XmString message;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create an error message dialog box.  This box is modal.
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
    Arg	arglist[8];
    int 	argcnt = 0;
    XtCallbackRec	cb_err_callback[2];

    cb_err_callback[0].callback = (XtCallbackProc)restart;
    cb_err_callback[0].closure = NULL;
    cb_err_callback[1].callback = NULL;
    cb_err_callback[1].closure = NULL;

    XtSetArg(arglist[argcnt], XmNokCallback, cb_err_callback); argcnt++;
    XtSetArg(arglist[argcnt], XmNmappedWhenManaged, False); argcnt++;
    XtSetArg(arglist[argcnt], XmNmessageString, message); argcnt++;
    XtSetArg(arglist[argcnt], XmNdialogType, XmDIALOG_WARNING); argcnt++;

    MrmFetchWidget(s_DRMHierarchy, "PauseError", multiCover_container[0], 
		   &error_widget, &drm_dummy_class);
    XtSetValues(error_widget, arglist, argcnt);

}



/*  display the error message in the pause error window */
int	pause_error_display(message)
XmString message;
{
    Arg arglist[6];
    Position dix=0, diy=0;
    int argcnt;
    if (error_widget == 0)
      {
	create_error(message);
        XtSetMappedWhenManaged(XtParent(error_widget), False);
      }
    else {
	argcnt = 0;
	XtSetArg(arglist[argcnt], XmNmessageString, message); argcnt++;
	XtSetArg(arglist[argcnt], XmNmappedWhenManaged, False); argcnt++;
        XtSetValues(error_widget, arglist, argcnt);
    }

    /* Turn off parent DialogShell's window manager decorations */
      {
	Arg wargs[2];
	XtSetArg (wargs[0], XtNoverrideRedirect, TRUE);
	XtSetValues(XtParent(error_widget), wargs, 1);
      }


    /* turn off the buttons we don't need */
    if (XtIsManaged(XmMessageBoxGetChild(error_widget,
		  		         XmDIALOG_HELP_BUTTON)))
	XtUnmanageChild(XmMessageBoxGetChild(error_widget,
		  		             XmDIALOG_HELP_BUTTON));
    if (XtIsManaged(XmMessageBoxGetChild(error_widget,
					 XmDIALOG_CANCEL_BUTTON)))
        XtUnmanageChild(XmMessageBoxGetChild(error_widget,
		    			     XmDIALOG_CANCEL_BUTTON));

    XtAddEventHandler(error_widget, ExposureMask, False,
		      (XtEventHandler)DoErrorGrab, NULL);

    XtAddEventHandler(error_widget, KeyPressMask, False,
			(XtEventHandler)ack_rcvd, NULL);
    
    XtManageChild(error_widget);
    /* force the size calculations */
    get_center_coor(XtDisplay(error_widget), (Widget)0,
		    error_widget, &dix, &diy);
    argcnt = 0;
    XtSetArg (arglist[argcnt], XmNx, dix); argcnt++;
    XtSetArg (arglist[argcnt], XmNy, diy); argcnt++;
    XtSetArg(arglist[argcnt], XmNmappedWhenManaged, True); argcnt++;
    XtSetArg(arglist[argcnt], XmNdefaultButtonType, XmDIALOG_OK_BUTTON);
    argcnt++;
    XtSetValues(error_widget, arglist, argcnt);
    XtSetMappedWhenManaged(XtParent(error_widget), True);

     
    XSetInputFocus(XtDisplay(error_widget), XtWindow(error_widget), 
		    RevertToNone, CurrentTime);

}


int	put_pause_error(status, text_index)
unsigned	int	status;
int	text_index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Put an error in the error dialog box for the pause window.
**
**  FORMAL PARAMETERS:
**
**	status - A VMS status value
**	other_text - An index into the UIL message array. Or -1 if no
**		     text is to be displayed
**
**--
**/
{
XmString message_cs;

#ifdef R5_XLIB
/*
 * pause_error_message is specified in resource file and it can be a
 * multibyte character sequence in an Asian locale. Therefore, XmStringCreate()
 * should use the tag XmFONTLIST_DEFAULT_TAG to create a locale specific
 * compound string.
 */
pause_error_display( XmStringCreate(options.pause_error_message, XmFONTLIST_DEFAULT_TAG));
#else
pause_error_display( XmStringCreate(options.pause_error_message, XmSTRING_DEFAULT_CHARSET));
#endif /* R5_XLIB */

}


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
            XtRemoveEventHandler(box, ExposureMask, False,
				 (XtEventHandler) DoPauseGrab, NULL);
    	    XSetInputFocus(XtDisplay(pass_text), XtWindow(pass_text), 
	                   RevertToNone, CurrentTime);
    	    XtGrabPointer(box, True, Button1Mask, GrabModeAsync,
			  GrabModeAsync, None, None, CurrentTime);
	}
    }
}



/*
 * The MoveToTop function is called periodically (every 1/20th second)
 * to make sure the pause screen is at the top of the stacking order.  If
 * the user has not specified resource grabServer: True it is possible for
 * other client windows to be displayed "over" the pause screen, thus 
 * defeating the purpose of the pause screen.  This function makes sure it
 * will happen for only an instant.  It then sets input focus to the password
 * text field or error widget, as appropriate, and set up to be called again
 * in 1/20th second.
 */


static void MoveToTop(data, id)
XtPointer data;
XtIntervalId *id;

{
    XtWidgetGeometry request;
    int i = XScreenCount(XtDisplay(toplevel));

    request.sibling = NULL;
    request.request_mode = CWStackMode;
    request.stack_mode = Above;
    XtMakeGeometryRequest(multiCover[0], &request, NULL);
    XSync(XtDisplay(multiCover[0]), False); 
    if (wait_error_ack) {
	XSetInputFocus(XtDisplay(error_widget), XtWindow(error_widget),
		       RevertToNone, CurrentTime);
    } else {
	XSetInputFocus(XtDisplay(pass_text), XtWindow(pass_text),
		       RevertToNone, CurrentTime);
    }
    for (i = XScreenCount(XtDisplay(toplevel)) - 1; i; i--) {
	XtMakeGeometryRequest(multiCover[i], &request, NULL);
	XSync(XtDisplay(multiCover[i]), False);
	XSetInputFocus(XtDisplay(pass_text), XtWindow(pass_text),
		       RevertToNone, CurrentTime);
    }
    XtAppAddTimeOut(XtWidgetToApplicationContext(toplevel), 50,
                          &MoveToTop, NULL);
}



static void idleHandler(closure, id)
caddr_t closure;
XtIntervalId id;
{
  if (wait_for_pass) {
    /* don't want the password box to disappear during the password
     * check Or while the password incorrect box is unacknowledged.
     * reset the timeout to try again later
     */

    wait_for_pass = FALSE;
    timeout = XtAddTimeOut(IDLETIME*1000, (XtTimerCallbackProc)idleHandler, 0);
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
  timeout = XtAddTimeOut(IDLETIME*1000, (XtTimerCallbackProc)idleHandler, 0);
}

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
  if (options.root_passwd) 
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
char	*val;

buf[0] = 0;
/* Get the current value of the password field */
val = XmTextGetString(pass_text);
len  = strlen(val);
/* Replace it with NULL */
XmTextReplace(pass_text, 0, len, buf);
XtFree(val);
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
register char *str = XmTextGetString(pass_text);

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

    /*
    XtPopdown(cover);
    */
    MultiheadDown(XtDisplay(toplevel));

   /* Ungrab the server */

    /*
    XUngrabServer(XtDisplay(cover));
    */

    XtFree(str);
}

#ifdef USE_SIA
/* there's not good quick way to get argc and argv into a callback
 * struct so...
 */
int Argc;
char ** Argv;
#endif /* USE_SIA */
void
main(argc,argv)
  int argc;
  char *argv[];
{
	int i;
	int res;
	char *name=NULL;
	static Boolean first = TRUE;
	MrmCount nfiles;
	String uid_files[1] =  {"DXpause.uid"};
	Widget root;
	Widget label;
static MrmRegisterArg reglist[] = {
        { "PauseCancelCallback", (caddr_t) cancel_callback },
        { "PauseOkCallback", (caddr_t) pause_callback },
	{"widget_create_proc", (caddr_t) widget_create_proc},
        { "passtext_id", (caddr_t) &pass_text },
        { "passlabel_id", (caddr_t) &pass_label },
        { "pauseok_id", (caddr_t) &pass_ok },
        { "pauselabel_id", (caddr_t) &pauselabel },
      }; 

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

#ifdef R5_XLIB
	/* XtSetLanguageProc() should be called before XtAppInitialize() */
	XtSetLanguageProc(NULL, NULL, NULL);
#endif /* R5_XLIB */
	DXmInitialize();
	MrmInitialize();
	root = toplevel =  XtInitialize(NULL, "DXpause", NULL, 0, &argc, argv);


#ifdef USE_SIA 
	Argc = argc;
	Argv = argv;
#endif /* USE_SIA */

	MrmRegisterNames (reglist, reglist_num);

	XtGetApplicationResources(toplevel,&options,applicationResources,
              XtNumber(applicationResources), NULL, 0);

	nfiles = 1;

	/*
	 * Define the DRM "hierarchy".  For Motif 1.2, use
	 * MrmOpenHierarchyPerDisplay, since the sans perdisplay version is
	 * now deprecated.
	 */
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
	if(MrmOpenHierarchyPerDisplay(XtDisplay(toplevel),
			    nfiles, uid_files, NULL, &s_DRMHierarchy) != MrmSUCCESS)
#else
	if(MrmOpenHierarchy(nfiles, uid_files, NULL, &s_DRMHierarchy) != MrmSUCCESS)
#endif
	{
		fprintf(stderr, "Can\'t open DRM hierarchy\n");
		exit(1); 
	}
	dpy = XtDisplay(root);

#ifdef I18N_MULTIBYTE
/*
 *	Some of the keyboards, for instance Kana Keyboard, have a
 *	locking-shift-key used for typing a native language.
 *	From X11R4, Mode-switch mechanism was introduced as a part of I18n
 *	to support country specific keyboards. Basically, Mode-switch is
 *	assigned to the locking-shift-key. As an operator presses the
 *	locking-shift-key once, the status of Mode-switch changes to ON,
 *	and XLookupString returns country specific KeySyms.
 *
 *	Currently, OSF/1 only supports ios-latin-1 password. If password is
 *	entered with Mode-switch ON, country specific KeySyms will be send to
 *	the input field instead of the iso-latin-1 KsySyms and therefore
 *	password checking will always fail. Mode-switch mechanism is disabled
 *	by setting keymap to be iso-latin-1. This is done by
 *	DWI18n_ChangeKeymap(). Another function DWI18n_RestoreKeymap() is used
 *	to restore the current keymap.
 *
 *	This is required as LK401 has no LED for Compose/Kana, there is no way
 *	for one to know what the current status of keyboard is. Especially
 *	in case of pause session window.
 */
	DWI18n_ChangeKeymap(dpy);
#endif /* I18N_MULTIBYTE */

	MultiheadUp(dpy,first);
	first = 0;
	XtMainLoop();
	
 }

static int MultiheadUp(display,first)
	Display	*display;
	int first;
{
  int num_screens, i;
  
  Dimension width;
  static Arg args[] = {
        { XmNwidth, NULL },			/* 0 */
	{ XmNheight, NULL },			/* 1 */
	{ XmNx, 0 },				/* 2 */
	{ XmNy, 0 },				/* 3 */
	{ XmNgeometry, (XtArgVal)"=+0+0" },	/* 4 */
	{ XmNallowShellResize, FALSE },		/* 5 */
	{ XmNsaveUnder, FALSE },		/* 6 */
        { XmNscreen, NULL },			/* 7 */
	{ XmNborderWidth, 0 },			/* 8 */
	{ XmNcolormap, NULL },			/* 9 */
	{ XmNoverrideRedirect, TRUE },		/* 10 */
	{ XmNdepth, NULL	},		/* 11 */
	{ NULL, NULL }, /* 12, extra slot for setting background */
    };
  Pixel background;

  Arg arglist[5];
  int num_args = 0;
  char name[32];
  Pixel tbackground;
  Position dix, diy;
  XSetWindowAttributes win_attr;

	/* save the screen saver information for later */

  /*
  XGetScreenSaver(display,&new_timeout,&interval,&prefer_blanking,&allow_exposures);
  XSetScreenSaver(display, options.pause_saver*60 , options.pause_saver*60,
                  PreferBlanking, AllowExposures);
  */

  num_screens = XScreenCount(display);

  if (first == 1)
    {
      multiCover = (Widget *)XtMalloc(sizeof(Widget)*num_screens);
      multiCover_container = (Widget *)XtMalloc(sizeof(Widget)*num_screens);
    }

  for (i = 0; i < num_screens; i++)
    {
      if (first == 1)
	{ 
	  XtSetArg(args[0], XmNwidth, DisplayWidth(display, i));
	  XtSetArg(args[1], XmNheight, DisplayHeight(display, i));
	  XtSetArg(args[7], XmNscreen, ScreenOfDisplay(display, i));
	  XtSetArg(args[9], XmNcolormap, XDefaultColormap(display, i));
/* Since we are a child of toplevel, whose depth is determing by the 
 * display that pause was initially started on, make sure to explicitly
 * set the depth properly for each display. Othewise the depth is inherited
 * from toplevel and this will fail on multi-headed systems of varying 
 * depths 
 */
	  XtSetArg(args[11],XmNdepth, XDefaultDepth(display, i));
	  
	  multiCover[i] = XtCreatePopupShell("pause",
					      xmDialogShellWidgetClass, 
					      toplevel,
					      args, 
					      XtNumber(args)-1);

	  MrmFetchWidgetOverride(s_DRMHierarchy, "PauseContainer",
				 multiCover[i], "PauseContainer", args, XtNumber(args)-1,
				 &multiCover_container[i], &drm_dummy_class);

	  XtSetArg(arglist[0], XmNforeground, 0);
	  XtSetArg(arglist[1], XmNbackground, 1);
	  XtSetValues(multiCover[i], arglist, 2);
	  XtSetValues(multiCover_container[i], arglist, 2);
	  XtManageChild(multiCover_container[i]);


	}
    }
  MrmFetchWidget(s_DRMHierarchy, "PauseDialog", multiCover_container[0],
		 &box, &drm_dummy_class);

  text_translations_parsed = XtParseTranslationTable(text_translations_table);
  XtAugmentTranslations(pass_text, text_translations_parsed);

  text_accelerators_parsed = XtParseAcceleratorTable(text_accelerator_table);
  XtSetArg(arglist[0], XmNaccelerators, text_accelerators_parsed); 
  XtSetValues(pass_ok, arglist, 1);
  XtInstallAccelerators(pass_text, pass_ok);

  _XmTextDisableRedisplay(pass_text,TRUE);
  
  XtSetArg(arglist[0], XmNbackground, &tbackground);
  XtGetValues(pass_text, arglist, 1);
  XtSetArg(arglist[0], XmNforeground, tbackground);
  XtSetArg( arglist[1], XmNautoShowCursorPosition, False);

  /*
   XtSetArg( arglist[2], XmNcursorPositionVisible, False);
  XtSetArg( arglist[3], XmNeditMode, XmSINGLE_LINE_EDIT);
  */
  
  XtSetValues(pass_text, arglist, 2);

  /* shouldn't have to do this, but it seems to force correct
   * geometry calculations... 
   */

  XtSetArg(arglist[0], XmNwidth, &width);
  XtGetValues(pauselabel, arglist, 1);
  XtSetArg(arglist[0], XmNwidth, (width < 320) ? 350 : (width + 30)); 
  XtSetArg(arglist[1], XmNheight, 160); 
  XtSetValues(box, arglist, 2);
  
  /* move the box to the center of the screen */
  get_center_coor(XtDisplay(box), (Widget)0, box, &dix, &diy);
  XtSetArg(arglist[0], XmNx, dix);
  XtSetArg(arglist[1], XmNy, diy);
  XtSetValues(box, arglist, 2);

  XtAddEventHandler(box, ExposureMask, False,
		    (XtEventHandler)DoPauseGrab, NULL);
  
  XtSetMappedWhenManaged(box, True);
  XtManageChild(box);
  
  for (i = 0; i < num_screens; i++) 
  {
     XtPopup(multiCover[i], XtGrabNone);
  }
 
  /* Grab the server or start the MoveToTop timer */

  if (options.grab_server) 
  {
      XGrabServer(XtDisplay(box));
  } else {
      XtAppAddTimeOut(XtWidgetToApplicationContext(toplevel), 50, 
		      &MoveToTop, NULL);
  }
}

static int MultiheadDown(display)
	Display	*display;
{
    int num_screens,i;

    /*
    XSetScreenSaver(display,new_timeout,interval,prefer_blanking,allow_exposures);
    */

    num_screens = XScreenCount(display);

    for (i=0; i<num_screens; i++)
      {
	XtPopdown(multiCover[i]);
      }

    XtUngrabPointer(box, CurrentTime);
    XtUngrabKeyboard(box, CurrentTime);

    /* Ungrab the server */
    XUngrabServer(XtDisplay(box));

#ifdef I18N_MULTIBYTE
/*
 *	Some of the keyboards, for instance Kana Keyboard, have a
 *	locking-shift-key used for typing a native language.
 *	From X11R4, Mode-switch mechanism was introduced as a part of I18n
 *	to support country specific keyboards. Basically, Mode-switch is
 *	assigned to the locking-shift-key. As an operator presses the
 *	locking-shift-key once, the status of Mode-switch changes to ON,
 *	and XLookupString returns country specific KeySyms.
 *
 *	Currently, OSF/1 only supports ios-latin-1 password. If password is
 *	entered with Mode-switch ON, country specific KeySyms will be send to
 *	the input field instead of the iso-latin-1 KsySyms and therefore
 *	password checking will always fail. Mode-switch mechanism is disabled
 *	by setting keymap to be iso-latin-1. This is done by
 *	DWI18n_ChangeKeymap(). Another function DWI18n_RestoreKeymap() is used
 *	to restore the current keymap.
 *
 *	This is required as LK401 has no LED for Compose/Kana, there is no way
 *	for one to know what the current status of keyboard is. Especially
 *	in case of pause session window.
 */
    DWI18n_RestoreKeymap(XtDisplay(box));
#endif /* I18N_MULTIBYTE */

    if(MrmCloseHierarchy(s_DRMHierarchy) != MrmSUCCESS)
        fprintf(stderr,"Can\'t close hierarchy\n");
    exit(0);
	
}










