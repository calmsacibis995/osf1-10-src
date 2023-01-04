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
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clock.c,v 1.1.7.4 1993/10/19 19:56:21 Susan_Ng Exp $";
#endif
#endif
/* Header from VMS source pool
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clock.c,v 1.1.7.4 1993/10/19 19:56:21 Susan_Ng Exp $";
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
**	clock.c
**
**  FACILITY:
**      OOTB Clock
**
**  ABSTRACT:
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

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

#ifdef VMS
#include <decw$cursor.h>
#include <nam.h>
#define MAXPATHLEN NAM$C_MAXRSS
#else
#include <strings.h>
#include <X11/decwcursor.h>
#define MAXPATHLEN 256
#endif

#include <X11/cursorfont.h>
#include <DXm/DECspecific.h>

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifndef NO_HYPERHELP
#ifndef VMS
#include <DXm/bkr_api.h>
#endif
Opaque hyper_help_context;
#endif

#ifdef NO_AUDIO
#define NO_SOUND 1
#else
#define NO_SOUND 0
#endif

/* Make toplevel declaration external */
extern Widget toplevel;
extern XtAppContext app_context;

/* A kluge to get around the Maximize to Normalize problem. */
Boolean have_repainted;

/* Public routines entry points. */
Widget DWT$MY_CLOCK();
Widget DWT$MY_CLOCK_CREATE();
Widget DwtMyClock();
Widget DwtMyClockCreate();

/* Private routines */
/* In this module */
void Layout();
void clock_DisplayHelp();
void clock_serror();
void clock_display_message();
void remove_message_box_child();
static void managed_set_changed();
static XtExposeProc Repaint();
static void add_kid();
static Boolean SetValues();
static void Initialize();
static void InitializeStrings();
static void remove_child();
static void find_file();
static void Realize();
static void Destroy();
static void find_menubar_height();
static void Help();
static void CallMenu();
static void CallSettings();
static void SelectDateFont();
static void SelectDigitalFont();
static void Dummy();
static void on_context_activate_proc();
static void tracking_help();
static XtGeometryResult geometry_manager();
static unsigned int analog_repaint();
static unsigned int bell_repaint();
static unsigned int create_bell();
static unsigned int create_proc();
static unsigned int date_repaint();
static unsigned int digital_repaint();
static unsigned int exit_proc();
static unsigned int error_message_done_proc();
static unsigned int help_done_proc();
static unsigned int main_help_proc();
static unsigned int message_done_proc();
static unsigned int restore_settings_proc();
static unsigned int save_settings_proc();
static unsigned int save_current_settings();
static unsigned int mode_settings_proc();
static unsigned int mode_settings_ok_proc();
static unsigned int mode_settings_cancel_proc();
static unsigned int alarm_settings_proc();
static unsigned int alarm_settings_ok_proc();
static unsigned int alarm_settings_cancel_proc();
XtTimerCallbackProc change_clock_size();
char *ResolveFilename ();
static XrmDatabase GetAppUserDefaults();
static XrmDatabase GetAppSystemDefaults();
void audio_file_selection_box();
void audio_play();
void audio_stop();
void audio_file_select_action();

/* In clockutil.c	*/
extern ClockWidget clock_FindMainWidget();
extern XtTimerCallbackProc clock_tic();
extern void initialize();
extern void set_time_strings();
extern void DrawClockFace();
extern void DrawAnalog();
extern void DrawBell();
extern void DrawDigital();
extern void DrawDate();
extern void DetermineClockType();
extern void mode_settings();
extern void alarm_settings();
extern void update_clock();
extern void update_mode();
extern void reset_mode();
extern void LoadFontFamilies();
extern void WorkCursorDisplay();
extern void WorkCursorRemove();
extern void get_audio_hardware();
extern void audio_button_pressed();
extern void audio_set_volume();

/* default event bindings */

static char translations[] = 
	"<Btn3Down>:		MenuCalled()\n\
	 Help<Btn1Down>:	Help()\n\
	 ~Help<Btn1Up>(2):	SettingsCalled()";

/* transfer vector from translation manager action names to
 * address of routines */

static XtActionsRec actions[] = {
    {"MenuCalled", (XtActionProc) CallMenu}, 
    {"SettingsCalled", (XtActionProc) CallSettings}, 
    {"Dummy", (XtActionProc) Dummy}, 
    {"Help", (XtActionProc) Help}, 
    {NULL, NULL}
};

/* widget resources */

static int res_5 = 5;
static int res_150 = 150;
static int res_105 = 105;
static int res_true = TRUE;
static int res_false = FALSE;
static int res_datePercDn = DATE_PERC_DN;
static int res_datePercDa = DATE_PERC_DA;
static int res_analogPercDa = ANALOG_PERC_DA;
static int res_analogPercNa = ANALOG_PERC_NA;
static int res_analogPercDna = ANALOG_PERC_DNA;

static CompositeClassExtensionRec compositeClassExtRec = {NULL, NULLQUARK,
  XtCompositeExtensionVersion, sizeof(CompositeClassExtensionRec), TRUE,};
static XtResource resources[] = {

    /* main window specific resources */
    {XmNx, 				/* field name */
      XtCX, 				/* resource class name */
      XtRPosition, 			/* code for type of resource */
      sizeof(Position), 		/* size of resource in bytes */
      XtOffset(ClockWidget, core. x), 	/* offset in bytes of this field in
					 * widget */
      XtRPosition, 			/* default resource type */
      (caddr_t) & res_5}, 		/* addresss of the default resource
					 * value */

    {XmNy, XtCY, XtRPosition, sizeof(Position), XtOffset(ClockWidget, core. y),
      XtRPosition, (caddr_t) & res_5}, 

    {XmNwidth, XtCWidth, XtRDimension, sizeof(Dimension),
      XtOffset(ClockWidget, core. width), XtRDimension, (caddr_t) & res_150}, 

    {XmNheight, XtCHeight, XtRDimension, sizeof(Dimension),
      XtOffset(ClockWidget, core. height), XtRDimension, (caddr_t) & res_105}, 

    {DwtNanalogOn, XtCAnalogOn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. analog_present), XtRInt, (caddr_t) & res_true}, 

    {DwtNdigitalOn, XtCDigitalOn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. digital_present), XtRInt, (caddr_t) & res_true}, 

    {DwtNdateOn, XtCDateOn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. date_present), XtRInt, (caddr_t) & res_true}, 

    {DwtNmilitaryOn, XtCMilitaryOn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. military_time), XtRInt, (caddr_t) & res_false}, 

    {DwtNmenubarOn, XtCMenubarOn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. menubar_present), XtRInt, (caddr_t) & res_true}, 

    {DwtNalarmOn, XtCAlarmOn, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. alarm_on), XtRInt, (caddr_t) & res_false}, 

    {DwtNrepeatOn, XtCRepeatOn, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. repeat_on), XtRInt, (caddr_t) & res_false}, 

    {DwtNalarmPM, XtCAlarmPM, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. alarm_pm), XtRInt, (caddr_t) & res_false}, 

    {DwtNalarmHour, XtCAlarmHour, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. alarm_hour), XtRString, ""}, 

    {DwtNalarmMinute, XtCAlarmMinute, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. alarm_minute), XtRString, ""}, 

    {DwtNalarmMessage, XtCAlarmMessage, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. alarm_message), XtRString, ""}, 

    {DwtNbellOn, XtCBellOn, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. bell_on), XtRInt, (caddr_t) & res_false}, 

#ifndef NO_AUDIO
    {DwtNaudioDirMask, XtCAudioDirMask, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. audio_dir_mask), XtRString, ""}, 

    {DwtNaudioFilename, XtCAudioFilename, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. audio_filename), XtRString, ""}, 

    {DwtNaudioOn, XtCAudioOn, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. audio_on), XtRInt, (caddr_t) & res_false}, 

    {DwtNspeakerOn, XtCSpeakerOn, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. speaker_on), XtRInt, (caddr_t) & res_false}, 

    {DwtNaudioVolume, XtCAudioVolume, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. audio_volume), XtRInt, (caddr_t) & res_false}, 
#endif

    {DwtNfontFamily, XtCFontFamily, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. font_family), XtRString,
      "-Adobe-Times-Bold-R-Normal--*-*-*-*-P-*-ISO8859-1"}, 

    {DwtNdatePercDn, XtCDatePercDn, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. date_int_dn), XtRInt, (caddr_t) & res_datePercDn}, 

    {DwtNdatePercDa, XtCDatePercDa, XtRInt, sizeof(int) , XtOffset(ClockWidget,
      clock. date_int_da), XtRInt, (caddr_t) & res_datePercDa}, 

    {DwtNanalogPercDa, XtCAnalogPercDa, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. analog_int_da), XtRInt,
      (caddr_t) & res_analogPercDa}, 

    {DwtNanalogPercNa, XtCAnalogPercNa, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. analog_int_na), XtRInt,
      (caddr_t) & res_analogPercNa}, 

    {DwtNanalogPercDna, XtCAnalogPercDna, XtRInt, sizeof(int) ,
      XtOffset(ClockWidget, clock. analog_int_dna), XtRInt,
      (caddr_t) & res_analogPercDna}, 

    {DwtNlanguage, XtCLanguage, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. language), XtRString, ""}, 

    {DwtNdateFormat, XtCDateFormat, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. date_format), XtRString, ""}, 

    {DwtNtimeFormat, XtCTimeFormat, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. time_format), XtRString, ""}, 

    {DwtNmilitaryFormat, XtCMilitaryFormat, XtRString, sizeof(char *) ,
      XtOffset(ClockWidget, clock. military_format), XtRString, ""}, 

/*jv - inserted to kluge around problem that XmDrawingArea widget doesn't
** have a font list resource that clock tries to access for the digital part
*/
    {XmNfontList, XmCFontList, XmRFontList, sizeof(XmFontList),
      XtOffset(ClockWidget, clock.clock_font), XmRString, DXmDefaultFont}, 

/*
    {DwtN, XtC, XtRInt, sizeof (int),
     XtOffset (ClockWidget, clock. ), XtRInt, (caddr_t) &res_true},

    {DwtN, XtC, XtRString, sizeof (char *),
     XtOffset (ClockWidget, clock. ), XtRString, ""},
*/
};

externaldef(clockwidgetclassrec)
     ClockClassRec clockwidgetclassrec = {
	    {(WidgetClass) & xmManagerClassRec, 
					/* superclass         */
	      ClockClassName, 		/* class_name         */
	      sizeof(ClockWidgetRec), 	/* widget_size        */
	      NULL, 			/* class_initialize   */
	      NULL, 			/* chained class part init */
	      FALSE, 			/* class_inited       */
	      Initialize, 		/* instance init proc */
	      NULL, 			/* init hook proc     */
	      Realize, 			/* realize widget proc */
	      actions, 			/* class action table */
	      XtNumber(actions), 	/* num_actions        */
	      resources, 		/* class resource list */
	      XtNumber(resources), 	/* num_resources      */
	      NULLQUARK, 		/* xrm_class          */
	      TRUE, 			/* compress_motion    */
	      TRUE, 			/* compress_exposure  */
	      TRUE, 			/* compress enter-leave */
	      TRUE, 			/* visible_interest   */
	      Destroy, 			/* destroy            */
	      Layout, 			/* resize             */
	      NULL, 			/* expose             */
	      SetValues, 		/* set_values         */
	      NULL, 			/* set values hook proc */
	      XtInheritSetValuesAlmost, 
					/* set values almost proc */
	      NULL, 			/* get values hook proc */
	      NULL, 			/* accept_focus       */
	      XtVersion, 		/* current version    */
	      NULL, 			/* callback offset    */
	      translations, 		/* default trans      */
	      NULL, 			/* Query geom proc      */
	      NULL, 			/* Display Accelerator  */
	      NULL, 			/* extension            */
	      }, 
	    {				/* composite class record */
	      geometry_manager, 	/* childrens geo mgr proc */
	      managed_set_changed, 	/* set changed proc */
	      add_kid, 			/* add a child */
	      remove_child, 		/* remove a child */
	      (XtPointer) & compositeClassExtRec, 
					/* extension */
	      }, 
	    {
              /* Constraint class Init */
              NULL,                     /* constraint resource list */
              0,                        /* number of constraints in list */
              0,                        /* size of constraint record */
              NULL,                     /* initialization */
              NULL,                     /* destroy proc */
              NULL,                     /* set_values proc */
              NULL                      /* pointer to extension record */
	      }, 

	    /* Manager Class */
	    {XtInheritTranslations, NULL, 
					/* get resources */
	      0, 			/* num get_resources */
	      NULL, 			/* get_cont_resources */
	      0, 			/* num_get_cont_resources */
	      XmInheritParentProcess, 	/* parent_process */
	      NULL 			/* extension */
	      }, 
	    {				/* XmPrimitive class */
	      NULL, 			/* XtWidgetProc border_highlight */
	      NULL, 			/* XtWidgetProc border_unhighlight */
	      NULL, 			/* XtTranslations translations */
	      NULL, 			/* XtActionProc arm_and_activate */
	      NULL, 			/* XmGetValueResource get_resources*/
	      NULL, 			/* int num_get_resources */
	      NULL 			/* caddr_t extension */
	      }, 
	    {				/* main window class record */
	      NULL 			/* just a dummy field */
	      }
	};

	externaldef(clockwidgetclass)
	    ClockWidgetClass clockwidgetclass =
	      (ClockWidgetClass) & clockwidgetclassrec;

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
#if !defined(VMS)
static String XtGetRootDirName(buf)
String buf;
{
     int uid;
     extern char *getenv();
/*
#ifdef R5_XLIB
     extern uid_t getuid();
#else
     extern int getuid();
#endif 
*/
     extern struct passwd *getpwuid();
     extern struct passwd *getpwnam();
     struct passwd *pw;
     static char *ptr = NULL;

     if (ptr == NULL)
     {
	if((ptr = getenv("HOME")) == NULL)
	{
	    if((ptr = getenv("USER")) != NULL) pw = getpwnam(ptr);
	    else {
		uid = getuid();
 		pw = getpwuid(uid);
	    }
	    if (pw) ptr = pw->pw_dir;
	    else {
		ptr = NULL;
		*buf = '\0';
	    }
	}
     }

     if (ptr != NULL) 
 	(void) strcpy(buf, ptr);

     buf += strlen(buf);
     *buf = '/';
     buf++;
     *buf = '\0';
     return buf;
}
#endif /* VMS */

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
static XrmDatabase GetAppSystemDefaults(dpy)
Display *dpy;
{
    char        *filename;
    XrmDatabase rdb;

    filename = ResolveFilename (dpy, CLASS_NAME, False, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return rdb;
}

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
static XrmDatabase GetAppUserDefaults(dpy)
Display *dpy;
{
    char* filename;
    XrmDatabase rdb;

    filename = ResolveFilename (dpy, CLASS_NAME, True, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return (rdb);
}

/*
**++
**  ROUTINE NAME:
**	Change Clock Size (w)
**
**  FUNCTIONAL DESCRIPTION
**	A timer event handler to do the setting
**	of the Clock size later and not in the
**	mainline code of the Resize callback routine
**	Layout().
**
**  FORMAL PARAMETERS:
**      mw - the clock widget structure.
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
XtTimerCallbackProc change_clock_size(w, otimer)
    Opaque w;
    XtIntervalId otimer;
{
    ClockWidget mw = (ClockWidget) w;
    Arg arg_list[20];
    int ac = 0;
    Dimension wid, hyt;
    Dimension mwid = (Dimension) Minwidth(mw);
    Dimension mhyt = (Dimension) Minheight(mw);

    wid = Width(mw);
    hyt = Height(mw);

    /* Make sure the new height and width is not less than the allowed
     *  minheight and minwidth. */
    ac = 0;
    if (wid < mwid) {
	XtSetArg(arg_list[ac], XmNwidth, mwid);
	ac++;
    }
    if (hyt < mhyt) {
	XtSetArg(arg_list[ac], XmNheight, mhyt);
	ac++;
    }
    if (ac != 0)
	XtSetValues((Widget) mw, arg_list, ac);
}

/*
**++
**  ROUTINE NAME:
**	add_kid
**
**  FUNCTIONAL DESCRIPTION:
**	add a child to this widget
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
static void add_kid(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal *num_args;
{
    (*((CompositeWidgetClass) compositeWidgetClass)->
      composite_class.insert_child) (w);

    /* If the child is a subclass of dialog, set defaultpositioning to  FALSE, 
     * so that the subwidget don't try to position themselves */
    if (XtIsSubclass(w, xmBulletinBoardWidgetClass)) {
	XmBulletinBoardWidget d = (XmBulletinBoardWidget) w;

	d->bulletin_board.default_position = FALSE;
    }
    /*return (TRUE);*/
}

/*
**++
**  ROUTINE NAME:
**	remove_child
**
**  FUNCTIONAL DESCRIPTION:
**	Delete a single widget from a parent widget
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
static void remove_child(child)
    Widget child;
{
    (*((CompositeWidgetClass) compositeWidgetClass)->
      composite_class.delete_child)(child);
}

/*
**++
**  ROUTINE NAME:
**	Initialize
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will initialize the widget instance
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
static void Initialize(request, new)
    Widget request, 			/* as build from arglist */
      new;				/* after superclasses init */
{

    ClockWidget mw = (ClockWidget) new;

    clock_initialize(mw);

/* The following line is used for debugging. */
#ifdef SYNCH
    XSynchronize(XtDisplay(mw), 1);
#endif

    Dpy(mw) = XtDisplay(mw);
    HelpWidget(mw) = 0;
}

/*
**++
**  ROUTINE NAME:
**	SetupFileName
**
**  FUNCTIONAL DESCRIPTION:
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
#ifdef VMS
char *SetupFileName(filename)
char *filename;
{
    sprintf(filename, "DECW$USER_DEFAULTS:DECW$CARDFILER.DAT");
    return filename;
}
#else
char *SetupFileName(filename)
char *filename;
{
    sprintf(filename,"%s/DXcardfiler", getenv("HOME"));
    return filename;
}
#endif

/*
**++
**  ROUTINE NAME:
**	ResolveFilename
**
**  FUNCTIONAL DESCRIPTION:
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
#if defined(VMS)
char *ResolveFilename (dpy, class_name, userfilename, must_exist)
Display *dpy;
String class_name;
Boolean	userfilename;
Boolean must_exist;  /* ignored on VMS */
{
    char* filename;

    filename = XtMalloc(MAXPATHLEN);
    if (userfilename)
    {
	(void) strcpy(filename, "decw$user_defaults:");
    }
    else
    {
	(void) strcpy(filename, "decw$system_defaults:");
    }
    (void) strcat(filename, class_name);
    (void) strcat(filename, ".dat");

    return (filename);
}
#else
char *ResolveFilename (dpy, class_name, userfilename, must_exist)
Display *dpy;
String class_name;
Boolean	userfilename;
Boolean must_exist;
{
    char* filename;
    char* path;
    Boolean deallocate = False;
/*  XrmDatabase rdb;*/
    extern char *getenv();
    char	homedir[MAXPATHLEN];

    if (userfilename)
    {
	if ((path = getenv("XUSERFILESEARCHPATH")) == NULL)
	{
	    char	*old_path;
	    int		size;

	    XtGetRootDirName(homedir);
	    if ((old_path = getenv("XAPPLRESDIR")) == NULL)
	    {
		char *path_default = "%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
		size = 3 * strlen(homedir) + strlen(path_default);
		path = XtMalloc (size);
		if (path == NULL) _XtAllocError (NULL);
		sprintf (path, path_default, homedir, homedir, homedir );
	    }
	    else
	    {
		char *path_default = "%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
		size = 3 * strlen(old_path) +
		    strlen(homedir) + strlen(path_default);
		path = XtMalloc (size);
		if (path == NULL) _XtAllocError(NULL);
		sprintf
		    (path, path_default, old_path, old_path, old_path, homedir);
	    }
	    deallocate = True;
	}

	filename = XtResolvePathname
	    (dpy, NULL, NULL, NULL, path, NULL, 0, NULL);

	if (deallocate) XtFree (path);
    }
    else
    {
	filename = XtResolvePathname
	    (dpy, "app-defaults", NULL, NULL, NULL, NULL, 0, NULL);
    }

    if ((filename == NULL) && !must_exist)
    {
	filename = XtMalloc(MAXPATHLEN);
	sprintf (filename, "%s/%s", homedir, class_name);
    }

    return (filename);
}
#endif	/* VMS */

/*
**++
**  ROUTINE NAME:
**	InitializeStrings
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will initialize the widget strings
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
static void InitializeStrings(widget)
    Widget widget;
{
    ClockWidget mw = (ClockWidget) widget;
    char *tmp, appl_title[256], temp_str[256];
/*  char base_defaults_name[256], system_defaults_name[256]; */
    int i, j, xtnumber, dont_care, array_offset; 
    long byte_count, cvt_status;
    Arg args[8];
    Cardinal status;
    MrmCode *type;
    XmStringCharSet *char_set;
    XmStringContext CScontext;
    XmString *array_buffer, appl_title_cs;

    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
      "CLOCK_APPLICATION_TITLE", appl_title);
/*
#ifdef VMS
    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
      "CLOCK_VMS_DEFAULTS_NAME", base_defaults_name);
    DefaultsName(mw) = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(DefaultsName(mw), base_defaults_name);
    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
      "CLOCK_VMS_SYSTEM_DEF_NAME", base_defaults_name);
#else
    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
      "CLOCK_UNIX_DEFAULTS_NAME", base_defaults_name);
    DefaultsName(mw) = createHomeDirSpec(base_defaults_name);
    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
      "CLOCK_UNIX_SYSTEM_DEF_NAME", base_defaults_name);
#endif
    SystemDefaultsName(mw) = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(SystemDefaultsName(mw), base_defaults_name);
*/

    /* Set the format strings. If the strings exist in defaults files,  then
     * use those, otherwise fetch them from UIL  */

    if (strcmp(DateFormat(mw), "") == 0) {

	/*  If using US locale, then load in "CLOCK_DATE_FORMAT_US. */
	if (IsUSLocale(mw))
	    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
	      "CLOCK_DATE_FORMAT_US", temp_str);
	else				/* use the CLOCK_DATE_FORMAT_NON_US
					 * format */
	    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
	      "CLOCK_DATE_FORMAT_NON_US", temp_str);
	CSDateFormat(mw) = XmStringCreate(temp_str, XmSTRING_DEFAULT_CHARSET);
    } else
	CSDateFormat(mw) =
	  XmStringCreate(DateFormat(mw), XmSTRING_DEFAULT_CHARSET);

    if (strcmp(TimeFormat(mw), "") == 0) {
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_TIME_FORMAT",
	  temp_str);
	CSTimeFormat(mw) = XmStringCreate(temp_str, XmSTRING_DEFAULT_CHARSET);
    } else
	CSTimeFormat(mw) =
	  XmStringCreate(TimeFormat(mw), XmSTRING_DEFAULT_CHARSET);

    if (strcmp(MilitaryFormat(mw), "") == 0) {

	/*  If using US locale, then load in "CLOCK_MILITARY_FORMAT_US. */
	if (Is12HrLocale(mw))
	    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
	      "CLOCK_MILITARY_FORMAT_US", temp_str);
	else				/* us the CLOCK_MILITARY_FORMAT_NON_US 
					 * format */
	    clock_get_uil_string(ClockHierarchy(mw), Dpy(mw),
	      "CLOCK_MILITARY_FORMAT_NON_US", temp_str);
	CSMilitaryFormat(mw) =
	  XmStringCreate(temp_str, XmSTRING_DEFAULT_CHARSET);
    } else
	CSMilitaryFormat(mw) =
	  XmStringCreate(MilitaryFormat(mw), XmSTRING_DEFAULT_CHARSET);


    /* Load Day strings */

#ifndef NO_XNLS
    if (IsAsianLocale(Language(mw))) {
#endif
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_AM",
	  Amstr(mw).label);
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_PM",
	  Pmstr(mw).label);
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_DAY_SUFFIX",
	  DaySuffix(mw).label);
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_HOUR_SUFFIX",
	  HourSuffix(mw).label);
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_MIN_SUFFIX",
	  MinSuffix(mw).label);

	status = MrmFetchLiteral(ClockHierarchy(mw), "CLOCK_ASIAN_DAY",
	  Dpy(mw), &array_buffer, &type);

	if (status != MrmSUCCESS) {
	    exit(1);			/* couldn't find CLOCK_ASIAN_DAY  */
	}

	array_offset = 0;
	for (i = 0; i < DAY_NUMERAL; i++) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * In original code, all segments except the first one is ignored.
	 * This is OK for single segment compound string (IOS8859-1), but
	 * cause data lost for multi segment compound string.
	 */
	    tmp = DXmCvtCStoFC(array_buffer[array_offset + i], 
			&byte_count, &cvt_status);
#else
	    status =
	      XmStringInitContext(&CScontext, array_buffer[array_offset + i]);
	    status = XmStringGetNextSegment(CScontext, &tmp, &char_set,
	      &dont_care, &dont_care);
#endif /* I18N_BUG_FIX */
	    strcpy(DayNumerals(mw)[i].label, tmp);
	    XtFree(tmp);
#ifdef I18N_BUG_FIX
#else
	    XmStringFree(char_set);
	    XmStringFreeContext(CScontext);
#endif /* I18N_BUG_FIX */
	}

	status = MrmFetchLiteral(ClockHierarchy(mw), "CLOCK_ASIAN_HOUR",
	  Dpy(mw), &array_buffer, &type);

	if (status != MrmSUCCESS) {
	    exit(1);			/* couldn't find CLOCK_ASIAN_DAY  */
	}

	array_offset = 0;
	for (i = 0; i < HOUR_NUMERAL; i++) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * In original code, all segments except the first one is ignored.
	 * This is OK for single segment compound string (IOS8859-1), but
	 * cause data lost for multi segment compound string.
	 */
	    tmp = DXmCvtCStoFC(array_buffer[array_offset + i], 
			&byte_count, &cvt_status);
#else
	    status =
	      XmStringInitContext(&CScontext, array_buffer[array_offset + i]);
	    status = XmStringGetNextSegment(CScontext, &tmp, &char_set,
	      &dont_care, &dont_care);
#endif /* I18N_BUG_FIX */
	    strcpy(HourNumerals(mw)[i].label, tmp);
	    XtFree(tmp);
#ifdef I18N_BUG_FIX
#else
	    XmStringFree(char_set);
	    XmStringFreeContext(CScontext);
#endif /* I18N_BUG_FIX */
	}

	status = MrmFetchLiteral(ClockHierarchy(mw), "CLOCK_ASIAN_MIN",
	  Dpy(mw), &array_buffer, &type);

	if (status != MrmSUCCESS) {
	    exit(1);			/* couldn't find CLOCK_ASIAN_DAY  */
	}

	array_offset = 0;
	for (i = 0; i < MIN_NUMERAL; i++) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * In original code, all segments except the first one is ignored.
	 * This is OK for single segment compound string (IOS8859-1), but
	 * cause data lost for multi segment compound string.
	 */
	    tmp = DXmCvtCStoFC(array_buffer[array_offset + i], 
			&byte_count, &cvt_status);
#else
	    status =
	      XmStringInitContext(&CScontext, array_buffer[array_offset + i]);
	    status = XmStringGetNextSegment(CScontext, &tmp, &char_set,
	      &dont_care, &dont_care);
#endif /* I18N_BUG_FIX */
	    strcpy(MinNumerals(mw)[i].label, tmp);
	    XtFree(tmp);
#ifdef I18N_BUG_FIX
#else
	    XmStringFree(char_set);
	    XmStringFreeContext(CScontext);
#endif /* I18N_BUG_FIX */
	}
#ifndef NO_XNLS
    }
#endif

    status = MrmFetchLiteral(ClockHierarchy(mw), "CLOCK_DAYS_MONTHS", Dpy(mw),
      &array_buffer, &type);

    array_offset = 0;
    for (i = 0; i < NUM_DAYS; i++) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * In original code, all segments except the first one is ignored.
	 * This is OK for single segment compound string (IOS8859-1), but
	 * cause data lost for multi segment compound string.
	 */
	    tmp = DXmCvtCStoFC(array_buffer[array_offset + i], 
			&byte_count, &cvt_status);
#else
	status =
	  XmStringInitContext(&CScontext, array_buffer[array_offset + i]);
	status = XmStringGetNextSegment(CScontext, &tmp, &char_set, &dont_care,
	  &dont_care);
#endif /* I18N_BUG_FIX */
	strcpy(Days(mw)[i].label, tmp);
	XtFree(tmp);
#ifdef I18N_BUG_FIX
#else
	XmStringFree(char_set);
	XmStringFreeContext(CScontext);
#endif /* I18N_BUG_FIX */
    }

    array_offset = NUM_DAYS;
    for (i = 0; i < NUM_MONTHS; i++) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * In original code, all segments except the first one is ignored.
	 * This is OK for single segment compound string (IOS8859-1), but
	 * cause data lost for multi segment compound string.
	 */
	    tmp = DXmCvtCStoFC(array_buffer[array_offset + i], 
			&byte_count, &cvt_status);
#else
	status =
	  XmStringInitContext(&CScontext, array_buffer[array_offset + i]);
	status = XmStringGetNextSegment(CScontext, &tmp, &char_set, &dont_care,
	  &dont_care);
#endif /* I18N_BUG_FIX */
	strcpy(Months(mw)[i].label, tmp);
	XtFree(tmp);
#ifdef I18N_BUG_FIX
#else
	XmStringFree(char_set);
	XmStringFreeContext(CScontext);
#endif /* I18N_BUG_FIX */
    }
    XmStringFree(array_buffer);

    appl_title_cs = (XmString) DXmCvtFCtoCS(appl_title, &byte_count, &cvt_status);
    DWI18n_SetTitle(XtParent(mw), appl_title_cs);
    DWI18n_SetIconName(XtParent(mw), appl_title_cs);
    XmStringFree(appl_title_cs);

    UserDatabase(mw) = NULL;
    SystemDatabase(mw) = NULL;
    MergedDatabase(mw) = XtDatabase(Dpy(mw));
}

/*
**++
**  ROUTINE NAME:
**	clock_get_uil_string
**
**  FUNCTIONAL DESCRIPTION:
**	Fetches a UIL string using DRM.  The UI utility routine cannot
**	be used since a context block does not exist at this point.
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
clock_get_uil_string(hierarchy, display, name_str, buffer)
    MrmHierarchy hierarchy;
    Display *display;
    char *name_str;
    char *buffer;

{
    Cardinal status;
    caddr_t *value;
    MrmCode *type;

    status = MrmFetchLiteral(hierarchy, name_str, display, &value, &type);

    strcpy(buffer, value);
/*    XtFree(value); */
}

/*
**++
**  ROUTINE NAME:
**	managed_set_changed
**
**  FUNCTIONAL DESCRIPTION:
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
static void managed_set_changed(w)
    Widget w;
{
    ClockWidget mw = (ClockWidget) w;
    XtWidgetGeometry g, *gp;
    XtGeometryResult res;

    DetermineClockType(mw);
    Layout(mw);

/*
    gp = &g;
    GeoMode (gp) = CWStackMode;
    gp -> stack_mode = Above;
    res = XtMakeGeometryRequest (AlarmBell (mw), gp, NULL);
*/
}

/*
**++
**  ROUTINE NAME:
**	Help
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will be called from the widget's
**	main event handler via the translation manager.
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
static void Help(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;

{
    ClockWidget mw;
    XmString Frame;

    mw = clock_FindMainWidget(w);

    Frame = XmStringCreate("Overview", XmSTRING_DEFAULT_CHARSET);
    clock_DisplayHelp(mw, Frame);
}

/*
**++
**  ROUTINE NAME:
**	CallMenu
**
**  FUNCTIONAL DESCRIPTION:
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
static void CallMenu(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;
{
    ClockWidget mw = (ClockWidget) w;

    XmMenuPosition(Menu(mw), ev);
    XtManageChild((Widget) Menu(mw));

/*
  MenuPopup (Menu (mw));
  XtPopup (Menu (mw), XtGrabNone);
*/
}

/*
**++
**  ROUTINE NAME:
**	CallSettings
**
**  FUNCTIONAL DESCRIPTION:
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
static void CallSettings(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
/*  mode_settings(mw); */
    alarm_settings(mw);
}

/*
**++
**  ROUTINE NAME:
**	Dummy
**
**  FUNCTIONAL DESCRIPTION:
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
static void Dummy(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;
{

}

/*
**++
**  ROUTINE NAME:
** 	SetValues(old,new)
**
**  FUNCTIONAL DESCRIPTION:
** 	This routine detects differences in two versions
** 	of a widget, when a difference is found the
** 	appropriate action is taken.  It is called when the
**	user calls the public routine XtSetvalues on the widget;
**
**  FORMAL PARAMETERS:
**	Widget old, new;
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
static Boolean SetValues(current, request, new_one)
    Widget current, 			/* original widget */
      request, 				/* as modif by arglist */
      new_one;				/* as modif by superclasses */
{
    ClockWidget old = (ClockWidget) current;
    ClockWidget new = (ClockWidget) new_one;

    return (FALSE);
}

/*
**++
**  ROUTINE NAME:
**	Destroy
**
**  FUNCTIONAL DESCRIPTION:
**	Destroy the widget specific data structs.
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
static void Destroy(w)
    Widget w;
{
    ClockWidget mw = (ClockWidget) w;

/* free the callback lists. One remove call per callback */

}

/*
**++
**  ROUTINE NAME:
**	Realize
**
**  FUNCTIONAL DESCRIPTION:
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
static void Realize(w, window_mask, window_atts)
    Widget w;
    Mask *window_mask;
    XSetWindowAttributes *window_atts;

{
    char *mes_str, *status_str, *xy;
    int date_length, dont_care, update, unmanage_count = 0;
    int tempwidth; 
    int tempheight;
    long time_value;
    struct tm *localtime();
    struct tm dst_tm;	/* current time */
    Arg arg_list[5];
    Cardinal ac, status;
    ClockWidget mw = (ClockWidget) w;
    MrmType *dummy_class;
    Pixel fgpixel, bgpixel;
    Widget unmanage_list[10];
    Widget menu;
    XmString date_string;
    XmStringCharSet *char_set;
    XmStringContext context;
    XtIntervalId timer;

    if (XtIsRealized(mw))
	return;

    XtCreateWindow((Widget) mw, InputOutput, CopyFromParent, *window_mask, window_atts);

#ifdef I18N_BUG_FIX
    Language(mw) = (char *)xnl_getlanguage();
#endif

#ifndef NO_XNLS
#ifndef I18N_BUG_FIX
    Language(mw) = (char *)xnl_getlanguage();
#endif

    if (!IsAsianLocale(Language(mw))) {  /* if not Asian locale... */

        /* If the language is not in the defaults file... */
	if (strcmp(Language(mw), "") == 0) {
            /* let setlocale read it from XNLS */
	    status_str = xnl_winsetlocale(0, 0, &LanguageId(mw), Dpy(mw));

            /* if the above fails, attempt to load US locale */
	    if (status_str == 0) {
		status_str = xnl_winsetlocale(LC_ALL, "en_US.88591",
		  &LanguageId(mw), Dpy(mw));
	    }
	}
	else	 /* If the language is defined, attempt to load
                   the specified locale  */
        {
	    status_str =
	      xnl_winsetlocale(LC_ALL, Language(mw), &LanguageId(mw), Dpy(mw));

            /* if the above fails, attempt to load US locale */
            if (status_str == 0)
                status_str = xnl_winsetlocale(LC_ALL, "en_US.88591",
                  &LanguageId(mw), Dpy(mw));
        }
  
        /* if all else failed, then bad locale */
	if (status_str == 0) {
	    clock_serror(mw, BadLocale, FATAL);
	}

	/* Find out if it is a US locale */
	if (strcmp(status_str, "en_US") == 0)
	    IsUSLocale(mw) = 1;
	else
	    IsUSLocale(mw) = 0;

	/* Find out if it is a 12 hr locale */
	if ((strcmp(status_str, "en_US") == 0) ||
	  (strcmp(status_str, "en_AU") == 0) ||
	  (strcmp(status_str, "en_NZ") == 0) || (strcmp(status_str,
	  "en_PG") == 0) || (strcmp(status_str, "en_FJ") == 0))

	    Is12HrLocale(mw) = 1;
	else
	    Is12HrLocale(mw) = 0;

    }
#else
    IsUSLocale(mw) = 1;
    Is12HrLocale(mw) = 1;
#endif

    /* Set NumFonts () to -1, signifying the Font Families has not been
     * loaded. */
    NumFonts(mw) = -1;

    InitializeStrings(mw);

    (void) time(&time_value);		/* get the time and then load the
					 * strings */
    dst_tm = *localtime(&time_value);

    /*  Make sure that the longest time and date string get assigned. */
    set_time_strings(mw, dst_tm, TRUE, TRUE);

    LastTm(mw) = dst_tm;

    update = 60 - dst_tm.tm_sec;
    timer =
      XtAppAddTimeOut(app_context, update * 1000, (XtTimerCallbackProc) clock_tic, (Opaque) mw);

    if (MrmFetchWidget(ClockHierarchy(mw), "opt_popup_menu", mw, &menu,
      &dummy_class) != MrmSUCCESS) {
	clock_serror(mw, NoPopup, WARNING);
    }

    Menu(mw) = (XmRowColumnWidget) menu;

    if (IsAsianLocale(Language(mw))) {
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_LONGEST_DATE",
	  LongestDate(mw));

	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_LONGEST_TIME",
	  LongestTime(mw));
    } else {
#ifndef NO_XNLS
	date_length = xnl_strftime(LanguageId(mw), &date_string,
	  CSDateFormat(mw), &dst_tm);

	if (date_length != 0) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * Original code cannot correctly handle RtoL segment.
	 */
	    xy = DXmCvtCStoFC(date_string, &byte_count, &cvt_status);
	    strcpy(LongestDate(mw), xy);
	    XtFree(xy);
	    XmStringFree(date_string);
#else
	    strcpy(LongestDate(mw), "");
	    status = XmStringInitContext(&context, date_string);
	    while (status) {
		status = XmStringGetNextSegment(context, &xy, &char_set,
		  &dont_care, &dont_care);
		if (status) {
		    strcat(LongestDate(mw), xy);
		    XtFree(xy);
		    XtFree(char_set);
		}
	    }
	    XmStringFreeContext(context);
	    XmStringFree(date_string);
#endif /* I18N_BUG_FIX */
	}

	if (MilitaryTime(mw))
	    date_length = xnl_strftime(LanguageId(mw), &date_string,
	      CSMilitaryFormat(mw), &dst_tm);
	else
	    date_length = xnl_strftime(LanguageId(mw), &date_string,
	      CSTimeFormat(mw), &dst_tm);

	if (date_length != 0) {
#ifdef I18N_BUG_FIX
	/*
	 * Use DXmCvtCStoFC() to get all text segments in compound string.
	 * Original code cannot correctly handle RtoL segment.
	 */
	    xy = DXmCvtCStoFC(date_string, &byte_count, &cvt_status);
	    strcpy(LongestTime(mw), xy);
	    XtFree(xy);
	    XmStringFree(date_string);
#else
	    strcpy(LongestTime(mw), "");
	    status = XmStringInitContext(&context, date_string);
	    while (status) {
		status = XmStringGetNextSegment(context, &xy, &char_set,
		  &dont_care, &dont_care);
		if (status) {
		    strcat(LongestTime(mw), xy);
		    XtFree(xy);
		    XtFree(char_set);
		}
	    }
	    XmStringFreeContext(context);
	    XmStringFree(date_string);
#endif /* I18N_BUG_FIX */
	}
#else
	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_LONGEST_DATE",
	  LongestDate(mw));

	clock_get_uil_string(ClockHierarchy(mw), Dpy(mw), "CLOCK_LONGEST_TIME",
	  LongestTime(mw));
#endif
    }

    /* Don't load the fonts yet. Load it when the clock will be displaying
     * either the Digital or the Date part. ClockLoadFontFamilies (mw); */

    /* pick up default resources */
    strcpy(AlarmHr(mw), AlarmHour(mw));
    strcpy(AlarmMin(mw), AlarmMinute(mw));
    strcpy(AlarmMes(mw), AlarmMessage(mw));
  if (!NO_SOUND)
  {
    strcpy(AudioFname(mw), AudioFilename(mw));
    strcpy(AudioDirMask(mw), AudioDirectoryMask(mw));
  }
    /* Initialize the minwidth and minheight resources. */

    /* Get the ones for menu bar present */

    XtVaGetValues(XtParent(mw), XmNminWidth, &tempwidth, NULL);
    XtVaGetValues(XtParent(mw), XmNminHeight, &tempheight, NULL);

    Minwidth(mw) = (Dimension) tempwidth;
    Minheight(mw) = (Dimension) tempheight;

    /* Hard code the no menubar ones to 18 x 18. */
    NoMbMinwidth(mw) = 18;
    NoMbMinheight(mw) = 18;

    DetermineClockType(mw);

    /* see if there is audio hardware */
    get_audio_hardware(mw);

    if (!AnalogPresent(mw)) {
	unmanage_list[unmanage_count] = (Widget) AnalogPart(mw);
	unmanage_count++;
    }

    if (!DigitalPresent(mw)) {
	unmanage_list[unmanage_count] = (Widget) DigitalPart(mw);
	unmanage_count++;
    }

    if (!DatePresent(mw)) {
	unmanage_list[unmanage_count] = (Widget) DatePart(mw);
	unmanage_count++;
    }

    if (!MenubarPresent(mw)) {

	/* No menu bar, change the minwidth and minheight constraint. */
	ac = 0;
	XtSetArg(arg_list[ac], XmNminWidth, NoMbMinwidth(mw));
	ac++;
	XtSetArg(arg_list[ac], XmNminHeight, NoMbMinheight(mw));
	ac++;
	XtSetValues(XtParent(mw), arg_list, ac);
	unmanage_list[unmanage_count] = (Widget) TopMenuBar(mw);
	unmanage_count++;
    }

    if (!AlarmOn(mw)) {
	unmanage_list[unmanage_count] = (Widget) AlarmBell(mw);
	unmanage_count++;
    }

    XtUnmanageChildren(unmanage_list, unmanage_count);

    Layout(mw);
}

/*
**++
**  ROUTINE NAME:
**	Layout
**
**  FUNCTIONAL DESCRIPTION:
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
void Layout(p)
    Widget p;
{
    ClockWidget mw = (ClockWidget) p;
    XtIntervalId timer;
    Arg arg_list[20];
    int ac = 0;
    Dimension wid, hyt;
    Dimension A_wid, A_hyt;
    Dimension B_wid, B_hyt;
    Dimension D_wid, D_hyt;
    Dimension N_wid, N_hyt;
    Position A_x, A_y;
    Position B_x, B_y;
    Position N_x, N_y;
    Position D_x, D_y;
    Dimension margin, MB_h;
    Dimension lineheight;
    Dimension DateBorder, DigitalBorder, AnalogBorder;
    XtWidgetGeometry g, *gp;
    int type;

    if (!XtIsRealized(mw))
	return;

    wid = Width(mw);
    hyt = Height(mw);
    type = ClockType(mw);
    MB_h = 0;

    gp = &g;
    GeoMode(gp) = CWX | CWY | CWWidth | CWHeight;

    /* Determine the menu bar height if the menu bar is present   */
    if (MenubarPresent(mw)) {
	gp->width = wid;
	gp->height = Height(TopMenuBar(mw));
	gp->x = 0;
	gp->y = 0;
	find_menubar_height(TopMenuBar(mw), gp);

	clock_ChangeWindowGeometry(TopMenuBar(mw), gp);
	(*(XtCoreProc(TopMenuBar(mw), resize)))(TopMenuBar(mw));

	MB_h = gp->height;

	/* Change the minHeight dynamically, and is always 18 more  then MB_h.
	 */
	Minheight(mw) = MB_h + 18;
	XtSetArg(arg_list[0], XmNminHeight, Minheight(mw));
	XtSetValues(XtParent(mw), arg_list, 1);

	/* Make sure the new height and width is not less than the  allowed
	 * minheight and minwidth.  If they are, set a Xtimer  to have the
	 * clock size change later. */

	if ((wid < Minwidth(mw)) || (hyt < Minheight(mw))) {
	    timer = XtAppAddTimeOut(app_context, 100, (XtTimerCallbackProc) change_clock_size,
	      (Opaque) mw);
	    return;			/* don't do any repaint. */
	}
    }

    hyt = hyt - MB_h;

    /* Initialize all of the Dimensions and Positions to 0        */
    A_wid = A_hyt = D_wid = D_hyt = N_wid = N_hyt = 0;
    A_x = N_x = D_x = 0;
    A_y = N_y = D_y = MB_h;

    DateBorder = Double(XtBorderWidth(DatePart(mw)));
    DigitalBorder = Double(XtBorderWidth(DigitalPart(mw)));
    AnalogBorder = Double(XtBorderWidth(AnalogPart(mw)));

    B_wid = bell_width;
    B_hyt = bell_height;
    B_x = wid - B_wid - AnalogBorder;
    B_y = AnalogBorder + 1 + MB_h;

    if (type == Digital) {
	N_wid = wid - DigitalBorder;
	N_hyt = hyt - DigitalBorder;
    }

    if (type == Date) {
	D_wid = wid - DateBorder;
	D_hyt = hyt - DateBorder;
    }

    if (type == Analog) {
	A_wid = wid - AnalogBorder;
	A_hyt = hyt - AnalogBorder;
    }

    if (type == DigitalDate) {
	D_hyt = hyt * DatePercDn(mw) - DateBorder;
	D_wid = wid - DateBorder;
	N_wid = wid - DigitalBorder;
	N_hyt = hyt - D_hyt - DigitalBorder - DateBorder;
	N_y = D_hyt + DateBorder + D_y;
    }

    if (type == DateAnalog) {
	A_hyt = hyt * AnalogPercDa(mw) - AnalogBorder;
	A_wid = wid - AnalogBorder;
	D_wid = wid - AnalogBorder;
	D_hyt = hyt - A_hyt - AnalogBorder - DigitalBorder;
	D_y = A_hyt + AnalogBorder + A_y;
    }

    if (type == DigitalAnalog) {
	A_hyt = hyt * AnalogPercNa(mw) - AnalogBorder;
	A_wid = wid - AnalogBorder;
	N_wid = wid - DigitalBorder;
	N_hyt = hyt - A_hyt - AnalogBorder - DigitalBorder;
	N_y = A_hyt + AnalogBorder + A_y;
    }

    if (type == DigitalDateAnalog) {
	A_wid = wid * AnalogPercDna(mw) - AnalogBorder;
	A_hyt = hyt - AnalogBorder;
	N_hyt = hyt * DatePercDn(mw) - AnalogBorder;
	N_wid = wid - A_wid - DateBorder - AnalogBorder;
	D_hyt = hyt - N_hyt - DigitalBorder - AnalogBorder;
	D_wid = N_wid;
	D_x = A_wid + AnalogBorder;
	N_x = D_x;
	N_y = D_hyt + DateBorder + D_y;
    }

    if (AlarmOn(mw)) {
	gp->x = B_x;
	gp->y = B_y;
	gp->width = B_wid;
	gp->height = B_hyt;
	clock_ChangeWindowGeometry(AlarmBell(mw), gp);
    }

    if (DigitalPresent(mw)) {

	/* Load the fonts if not already loaded. */
	if (NumFonts(mw) == -1)
	    ClockLoadFontFamilies(mw);
	SelectDigitalFont(mw, N_hyt, N_wid);
    }

    if (DigitalPresent(mw)) {
	if (N_wid < 5)
	    N_wid = 5;
	if (N_hyt < 5)
	    N_hyt = 5;
	gp->x = N_x;
	gp->y = N_y;
	gp->width = N_wid;
	gp->height = N_hyt;
	clock_ChangeWindowGeometry(DigitalPart(mw), gp);
	if (have_repainted)
	    digital_repaint(DigitalPart(mw), 0, 0);
    }

    if (AnalogPresent(mw)) {
	if (A_wid < 5)
	    A_wid = 5;
	if (A_hyt < 5)
	    A_hyt = 5;
	gp->x = A_x;
	gp->y = A_y;
	gp->width = A_wid;
	gp->height = A_hyt;
	clock_ChangeWindowGeometry(AnalogPart(mw), gp);
	if (A_wid > A_hyt)
	    HandWidth(mw) = A_hyt / BOX_FRACTION / 2;
	else
	    HandWidth(mw) = A_wid / BOX_FRACTION / 2;
	if (HandWidth(mw) < 2)
	    HandWidth(mw) = 2;
	if (AnalogGC(mw) != 0) {
	    XSetLineAttributes(Dpy(mw), AnalogGC(mw), HandWidth(mw), LineSolid,
	      CapRound, JoinBevel);
	    XSetLineAttributes(Dpy(mw), AnalogClearGC(mw), HandWidth(mw),
	      LineSolid, CapRound, JoinBevel);
	    if (have_repainted)
		analog_repaint(AnalogPart(mw), 0, 0);
	}
    }

    if (DatePresent(mw)) {

	/* Load the fonts if not already loaded. */
	if (NumFonts(mw) == -1)
	    ClockLoadFontFamilies(mw);
	SelectDateFont(mw, D_hyt, D_wid);
    }
    if (DatePresent(mw)) {
	if (D_wid < 5)
	    D_wid = 5;
	if (D_hyt < 5)
	    D_hyt = 5;
	gp->x = D_x;
	gp->y = D_y;
	gp->width = D_wid;
	gp->height = D_hyt;
	clock_ChangeWindowGeometry(DatePart(mw), gp);
	if (have_repainted)
	    date_repaint(DatePart(mw), 0, 0);
    }

}

/*
**++
**  ROUTINE NAME:
**	find_menubar_height
**
**  FUNCTIONAL DESCRIPTION:
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
static void find_menubar_height(m, g)
    Widget m;
    XtWidgetGeometry *g;
{
    if (m == NULL)
	return;

    if (!XtIsSubclass(m, xmRowColumnWidgetClass)) {
	g->height = XtHeight(m);	/* it's not a menu */
    } else {
	XtWidgetGeometry intended, reply;

	intended.request_mode = CWWidth;
					/* ask about this width */
	intended.width = g->width;	/* if it affects the size */
					/* he'll tell us */

	switch (XtQueryGeometry(m, &intended, &reply)) {
	    case XtGeometryAlmost: 	/* he wants to compromise */

		if (reply.request_mode & CWHeight) {
		    g->height = reply.height;
					/* take height he suggests */
		    return;
		}			/* else just use existing */

	    /* no break */
	    case XtGeometryYes: 	/* he agrees, no problem w/ */
					/* current height */

	    /* no break */

	    case XtGeometryNo: 		/* wants to be his current */
		g->height = XtHeight(m);
					/* size */
		break;
	}
    }
}

/*
**++
**  ROUTINE NAME:
**	SelectDigitalFont
**
**  FUNCTIONAL DESCRIPTION:
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
static void SelectDigitalFont(mw, hyt, wid)
    ClockWidget mw;
    int hyt, wid;
{
    XGCValues gcv;
    int i;
    int hyt_select_id, wid_select_id;
    int use_hyt, use_wid;

    if (NumFonts(mw) == 0)
	return;

    use_hyt = hyt;
    use_wid = wid;

    if (use_hyt <= FontHeights(mw)[0])
	hyt_select_id = 0;
    else if (use_hyt >= FontHeights(mw)[NumFonts(mw) - 1])
	hyt_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_hyt >= FontHeights(mw)[i]) &&
	      (use_hyt < FontHeights(mw)[i + 1]))
		hyt_select_id = i;

    if (use_wid <= DigitalFontWidths(mw)[0])
	wid_select_id = 0;
    else if (use_wid >= DigitalFontWidths(mw)[NumFonts(mw) - 1])
	wid_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_wid >= DigitalFontWidths(mw)[i]) &&
	      (use_wid < DigitalFontWidths(mw)[i + 1]))
		wid_select_id = i;

    if (wid_select_id < hyt_select_id)
	DigitalFont(mw) = Fonts(mw)[wid_select_id];
    else
	DigitalFont(mw) = Fonts(mw)[hyt_select_id];

    if (IsAsianLocale(Language(mw))) {
	if (wid_select_id < hyt_select_id)
	    DigitalFontList(mw) = DigitalFontLists(mw)[wid_select_id];
	else
	    DigitalFontList(mw) = DigitalFontLists(mw)[hyt_select_id];
    }

    if (DigitalGC(mw) != 0) {
	gcv. font = DigitalFont(mw)->fid;

	XChangeGC(Dpy(mw), DigitalGC(mw), GCFont, &gcv);
	XChangeGC(Dpy(mw), DigitalClearGC(mw), GCFont, &gcv);
    }

    DigitalFontHeight(mw) = (DigitalFont(mw)->max_bounds.ascent +
      DigitalFont(mw)->max_bounds.descent);
}

/*
**++
**  ROUTINE NAME:
**	SelectDateFont
**
**  FUNCTIONAL DESCRIPTION:
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
static void SelectDateFont(mw, hyt, wid)
    ClockWidget mw;
    int hyt, wid;
{
    XGCValues gcv;
    int i;
    int hyt_select_id, wid_select_id;
    int use_hyt, use_wid;

    if (NumFonts(mw) == 0)
	return;

    use_hyt = hyt;
    use_wid = wid;

    if (use_hyt <= FontHeights(mw)[0])
	hyt_select_id = 0;
    else if (use_hyt >= FontHeights(mw)[NumFonts(mw) - 1])
	hyt_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_hyt >= FontHeights(mw)[i]) &&
	      (use_hyt < FontHeights(mw)[i + 1]))
		hyt_select_id = i;

    if (use_wid <= DateFontWidths(mw)[0])
	wid_select_id = 0;
    else if (use_wid >= DateFontWidths(mw)[NumFonts(mw) - 1])
	wid_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_wid >= DateFontWidths(mw)[i]) &&
	      (use_wid < DateFontWidths(mw)[i + 1]))
		wid_select_id = i;

    if (wid_select_id < hyt_select_id)
	DateFont(mw) = Fonts(mw)[wid_select_id];
    else
	DateFont(mw) = Fonts(mw)[hyt_select_id];

    if (IsAsianLocale(Language(mw))) {
	if (wid_select_id < hyt_select_id)
	    DateFontList(mw) = DateFontLists(mw)[wid_select_id];
	else
	    DateFontList(mw) = DateFontLists(mw)[hyt_select_id];
    }

    if (DateGC(mw) != 0) {
	gcv. font = DateFont(mw)->fid;

	XChangeGC(Dpy(mw), DateGC(mw), GCFont, &gcv);
	XChangeGC(Dpy(mw), DateClearGC(mw), GCFont, &gcv);
    }

    DateFontHeight(mw) =
      (DateFont(mw)->max_bounds.ascent + DateFont(mw)->max_bounds.descent);
}

/*
**++
**  ROUTINE NAME:
**	geometry_manager
**
**  FUNCTIONAL DESCRIPTION:
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
static XtGeometryResult geometry_manager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request, *reply;
{
    ClockWidget mw = (ClockWidget) XtParent(w);

    /*  Just say no (for now) */

    return (XtGeometryNo);
}

/*
**++
**  ROUTINE NAME:
**	DWT$MY_CLOCK
**
**  FUNCTIONAL DESCRIPTION:
**	Public entry points for VMS
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
Widget DWT$MY_CLOCK(parent, name$dsc, x, y, width, height)
    Widget *parent;
    struct dsc$descriptor_s *name$dsc;
    int *x;
    int *y;
    int *width;
    int *height;
{
    char *name;
    Arg arglist[20];
    int argCount = 0;
    Widget w;

#ifdef VMS
    name = DescToNull(name$dsc);
#endif

    XtSetArg(arglist[argCount], XmNx, *x);
    argCount++;
    XtSetArg(arglist[argCount], XmNy, *y);
    argCount++;
    XtSetArg(arglist[argCount], XmNheight, *height);
    argCount++;
    XtSetArg(arglist[argCount], XmNwidth, *width);
    argCount++;

    w = XtCreateWidget(name, (WidgetClass) clockwidgetclass, *parent, arglist, argCount);

    XtFree(name);

    /* Init the kluge variable (have_repainted) */
    have_repainted = FALSE;

    return (w);

}

/*
**++
**  ROUTINE NAME:
**	DWT$MY_CLOCK_CREATE
**
**  FUNCTIONAL DESCRIPTION:
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
Widget DWT$MY_CLOCK_CREATE(parent, name$dsc, arglist, argCount)

    Widget *parent;
    struct dsc$descriptor_s *name$dsc;
    Arg *arglist;
    int *argCount;
{
    Widget w;
    char *name;

#ifdef VMS
    name = DescToNull(name$dsc);
#endif

    w = XtCreateWidget(name, (WidgetClass) clockwidgetclass, *parent, arglist, *argCount);

    XtFree(name);

    return (w);
}


#ifdef VMS
static char uid_filespec[] = {"DECW$CLOCK"};
#else
static char APPL_CLASS[] = {"DXclock"};
#endif
static char uid_default[] = {"DECW$SYSTEM_DEFAULTS:.UID"};

static MrmOsOpenParam os_ext_list;
static MrmOsOpenParamPtr os_ext_list_ptr = &os_ext_list;

static MrmRegisterArg regvec[] = {
    {"analog_repaint", (caddr_t) analog_repaint}, 
    {"bell_repaint", (caddr_t) bell_repaint}, 
    {"create_bell", (caddr_t) create_bell}, 
    {"create_proc", (caddr_t) create_proc}, 
    {"date_repaint", (caddr_t) date_repaint}, 
    {"digital_repaint", (caddr_t) digital_repaint}, 
    {"error_message_done_proc", (caddr_t) error_message_done_proc}, 
    {"exit_proc", (caddr_t) exit_proc}, 
    {"help_done_proc", (caddr_t) help_done_proc}, 
    {"main_help_proc", (caddr_t) main_help_proc}, 
    {"message_done_proc", (caddr_t) message_done_proc}, 
    {"on_context_activate_proc", (caddr_t) on_context_activate_proc}, 
    {"save_settings_proc", (caddr_t) save_settings_proc}, 
    {"restore_settings_proc", (caddr_t) restore_settings_proc}, 
    {"mode_settings_proc", (caddr_t) mode_settings_proc},
    {"mode_settings_ok_proc", (caddr_t) mode_settings_ok_proc}, 
    {"mode_settings_cancel_proc", (caddr_t) mode_settings_cancel_proc}, 
    {"alarm_settings_proc", (caddr_t) alarm_settings_proc},
    {"alarm_settings_ok_proc", (caddr_t) alarm_settings_ok_proc}, 
    {"alarm_settings_cancel_proc", (caddr_t) alarm_settings_cancel_proc}, 
    {"audio_file_selection_box", (caddr_t) audio_file_selection_box},
    {"audio_file_select_action", (caddr_t) audio_file_select_action},
    {"audio_play", (caddr_t) audio_play},
    {"audio_stop", (caddr_t) audio_stop},
    {"audio_button_pressed", (caddr_t) audio_button_pressed},
    {"audio_set_volume", (caddr_t) audio_set_volume}
};

/*
**++
**  ROUTINE NAME:
**	DwtMyClock
**
**  FUNCTIONAL DESCRIPTION:
**	Public entry points for UNIX
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
Widget DwtMyClock(parent, name, x, y, width, height)

    Widget parent;
    char *name;
    Position x;
    Position y;
    Dimension width;
    Dimension height;
{
    ClockWidget mw;
    MrmHierarchy clock_hierarchy;	/* DRM database hierarchy id  */
    MrmType *dummy_class;
    MrmCount regnum;
    Arg arglist[5];
    int argCount = 0;
    char *file_array[1];
    FILE *fp;

#ifdef VMS
    file_array[0] = uid_filespec;
    os_ext_list.nam_flg.related_nam = 0;
#else
    /* get_uid_filename looks up the environment variable UIDDIR, and attempts 
     * to find a uid file with the correct name in it. */
    char *def_name, *uiddir, *uid_name;
    int uiddir_len, uid_fd;

    /* untested on ultrix */
    os_ext_list.nam_flg.clobber_flg = TRUE;

    file_array[0] =
      XtMalloc(strlen("/usr/lib/X11/uid/.uid") + strlen(CLASS_NAME) + 1);
/*
    sprintf (file_array [0], "/usr/lib/X11/uid/%s.uid", CLASS_NAME);
*/
    sprintf(file_array[0], "%s", CLASS_NAME);

    if ((uiddir = (char *) getenv("UIDDIR")) != NULL) {
	uiddir_len = strlen(uiddir);
	if ((def_name = rindex(file_array[0], '/')) == NULL) {
	    def_name = file_array[0];
	} else
	    def_name++;
	uiddir_len += strlen(def_name) + 2;
					/* 1 for '/', 1 for '\0' */
	uid_name = (char *) XtMalloc(uiddir_len * sizeof(char));
	strcpy(uid_name, uiddir);
	strcat(uid_name, "/");
	strcat(uid_name, def_name);
	if (access(uid_name, R_OK) != 0) {
	    XtFree(uid_name);
	} else {
	    XtFree(file_array[0]);
	    file_array[0] = uid_name;
	}
    }
#endif

    os_ext_list.version = MrmOsOpenParamVersion;
    os_ext_list.default_fname = (char *) uid_default;

    /* Define the DRM "hierarchy" */
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    if (MrmOpenHierarchyPerDisplay(
      XtDisplay(parent),		/* display 		*/      
      1, 				/* number of files      */
      file_array, 			/* files                */
      &os_ext_list_ptr, 		/* os_ext_list          */
      &clock_hierarchy)			/* ptr to returned id   */
      !=MrmSUCCESS)
	clock_serror(parent, NoHierarchy, FATAL);
#else
    if (MrmOpenHierarchy(1, 		/* number of files      */
      file_array, 			/* files                */
      &os_ext_list_ptr, 		/* os_ext_list          */
      &clock_hierarchy)			/* ptr to returned id   */
      !=MrmSUCCESS)
	clock_serror(parent, NoHierarchy, FATAL);
#endif
    regnum = sizeof(regvec) / sizeof(MrmRegisterArg);
    MrmRegisterNames(regvec, regnum);

    if (MrmFetchWidget(clock_hierarchy, "clock_widget", parent, &mw,
      &dummy_class) != MrmSUCCESS) {
	clock_serror(parent, NoClockMain, FATAL);
    }

    XtSetArg(arglist[argCount], XmNx, x);
    argCount++;
    XtSetArg(arglist[argCount], XmNy, y);
    argCount++;
    XtSetArg(arglist[argCount], XmNheight, height);
    argCount++;
    XtSetArg(arglist[argCount], XmNwidth, width);
    argCount++;
    XtSetValues((Widget) mw, arglist, argCount);

    ClockHierarchy(mw) = clock_hierarchy;

    return ((Widget) mw);
}

/*
**++
**  ROUTINE NAME:
**	DwtMyClockCreate
**
**  FUNCTIONAL DESCRIPTION:
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
Widget DwtMyClockCreate(parent, name, args, argCount)
    Widget parent;
    char *name;
    ArgList args;
    int argCount;
{
    return (XtCreateWidget(name, (WidgetClass) clockwidgetclass, parent, args, argCount));
}

/*
**++
**  ROUTINE NAME:
**	remove_message_box_child
**
**  FUNCTIONAL DESCRIPTION:
**	This routine removes the corresponding child from a Message Box.
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
void remove_message_box_child(w, child)
    Widget w;
    unsigned char child;

{
    XmMessageBoxWidget mb;

    mb = (XmMessageBoxWidget) w;
    XtUnmanageChild(XmMessageBoxGetChild(mb, child));
}

/*
**++
**  ROUTINE NAME:
**	create_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int create_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    XtTranslations analog_trans, dialog_trans;
    int ac = 0;
    Arg arg_list[20];
    int widget_num = (int) *tag;
    XmFontList FontList;
    XFontStruct *FontInfo;
    Pixel fgpixel, bgpixel;

    ClockWidget mw;

    mw = clock_FindMainWidget(w);

    switch (widget_num) {

	case k_analog_part: 
	    AnalogPart(mw) = (XmDrawingAreaWidget) w;
	    dialog_trans = XtParseTranslationTable(translations);
	    XtSetArg(arg_list[ac], XmNtranslations, dialog_trans);
	    ac++;
	    XtSetValues((Widget) AnalogPart(mw), arg_list, ac);
	    break;

	case k_digital_part: 
	    DigitalPart(mw) = (XmDrawingAreaWidget) w;
	    dialog_trans = XtParseTranslationTable(translations);
	    XtSetArg(arg_list[ac], XmNtranslations, dialog_trans);
	    ac++;
	    XtSetValues((Widget) DigitalPart(mw), arg_list, ac);
	    break;

	case k_date_part: 
	    DatePart(mw) = (XmDrawingAreaWidget) w;
	    dialog_trans = XtParseTranslationTable(translations);
	    XtSetArg(arg_list[ac], XmNtranslations, dialog_trans);
	    ac++;
	    XtSetValues((Widget) DatePart(mw), arg_list, ac);
	    break;

	case k_mode_settings:
	    Settings(mw) = (XmBulletinBoardWidget) w;
	    break;

	case k_alarm_settings: 
	    AlarmSettings(mw) = (XmBulletinBoardWidget) w;
	    break;

	case k_message: 
	    MessageWid(mw) = (XmMessageBoxWidget) w;
	    remove_message_box_child(w, XmDIALOG_CANCEL_BUTTON);
	    remove_message_box_child(w, XmDIALOG_HELP_BUTTON);
	    break;

	case k_menu_bar: 
	    TopMenuBar(mw) = (Widget) w;
	    break;

	case k_settings_menubar: 
	    MenubarWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_military: 
	    MilitaryTimeWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_error_message: 
	    ErrorMessageWid(mw) = (XmMessageBoxWidget) w;
	    remove_message_box_child(w, XmDIALOG_CANCEL_BUTTON);
	    remove_message_box_child(w, XmDIALOG_HELP_BUTTON);
	    break;

	case k_settings_analog: 
	    AnalogWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_digital: 
	    DigitalWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_date: 
	    DateWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_alarm: 
	    AlarmWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_repeat:
	    RepeatWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_am: 
	    AmWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_pm: 
	    PmWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_hr: 
	    HrWid(mw) = (XmTextWidget) w;
	    break;

	case k_settings_min: 
	    MinWid(mw) = (XmTextWidget) w;
	    break;

	case k_settings_alarm_mes: 
	    AlarmMesWid(mw) = (DXmCSTextWidget) w;
	    break;

	case k_settings_bell:
	    BellOnWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_audio:
	    AudioOnWid(mw) = (XmToggleButtonWidget) w;
	    break;

	case k_settings_speaker:
	    SpeakerOnWid(mw) = (XmPushButtonWidget) w;
	    break;

	case k_settings_headphone:
	    HeadphoneOnWid(mw) = (XmPushButtonWidget) w;
	    break;

	case k_settings_volume:
	    AudioVolumeWid(mw) = (XmScaleWidget) w;
	    break;

	case k_settings_audio_fname:
	    AudioFnameTextWid(mw) = (DXmCSTextWidget) w;
	    break;

	case k_settings_fname_button:
	    AudioFnameButtonWid(mw) = (XmPushButtonWidget) w;
	    break;

	case k_settings_play_button:
	    AudioPlayButtonWid(mw) = (XmPushButtonWidget) w;
	    break;

	case k_settings_stop_button:
	    AudioStopButtonWid(mw) = (XmPushButtonWidget) w;
	    break;

	case k_settings_audio_menu:
	    AudioOutputMenuWid(mw) = (XmRowColumnWidget) w;
	    break;

	case k_main_help: 
	    HelpWidget(mw) = (Widget) w;
	    break;
    }
}

/*
**++
**  ROUTINE NAME:
**	create_bell
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int create_bell(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);

    AlarmBell(mw) = (XmDrawingAreaWidget) w;
}

/*
**++
**  ROUTINE NAME:
**	bell_repaint
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int bell_repaint(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    DrawBell(mw);
}

/*
**++
**  ROUTINE NAME:
**	analog_repaint
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int analog_repaint(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    XmDrawingAreaWidget aw = (XmDrawingAreaWidget) w;
    ClockWidget mw;
    long time_value;
    struct tm *localtime();
    struct tm tm;			/* current time */

    /* Set the kluge variable (have_repainted) to TRUE */

    have_repainted = TRUE;

    mw = clock_FindMainWidget(w);
    DrawClockFace(aw);
    (void) time(&time_value);		/* get the time */
    tm = *localtime(&time_value);
    DrawAnalog(aw, tm, AnalogGC(mw));
}

/*
**++
**  ROUTINE NAME:
**	digital_repaint
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int digital_repaint(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    /* Set the kluge variable (have_repainted) to TRUE */

    have_repainted = TRUE;

    mw = clock_FindMainWidget(w);

    DrawDigital(mw, LastTm(mw));
}

/*
**++
**  ROUTINE NAME:
**	date_repaint
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int date_repaint(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    /* Set the kluge variable (have_repainted) to TRUE */
    have_repainted = TRUE;

    mw = clock_FindMainWidget(w);

    DrawDate(mw);
}

/*
**++
**  ROUTINE NAME:
**	mode_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int mode_settings_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    mode_settings(mw);
}

/*
**++
**  ROUTINE NAME:
**	alarm_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int alarm_settings_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    alarm_settings(mw);
}

/*
**++
**  ROUTINE NAME:
**	mode_settings_ok_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int mode_settings_ok_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    int status;

    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) Settings(mw));
    update_mode(mw);
}

/*
**++
**  ROUTINE NAME:
**	mode_settings_cancel_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int mode_settings_cancel_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) Settings(mw));
    reset_mode(mw);
}

/*
**++
**  ROUTINE NAME:
**	alarm_settings_ok_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int alarm_settings_ok_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    int status;

    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) AlarmSettings(mw));
    update_alarm(mw);
}

/*
**++
**  ROUTINE NAME:
**	alarm_settings_cancel_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int alarm_settings_cancel_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) AlarmSettings(mw));
    reset_alarm(mw);
}

/*
**++
**  ROUTINE NAME:
**	save_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int save_settings_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;
    Position x, y;
    Dimension width, height;

    mw = clock_FindMainWidget(w);

    width = XtWidth(toplevel);
    height = XtHeight(toplevel);
    XtTranslateCoords(toplevel, 0, 0, &x, &y);
    x = x - XtBorderWidth(toplevel);
    y = y - XtBorderWidth(toplevel);

    save_current_settings(mw, x, y, width, height);
}

/*
**++
**  ROUTINE NAME:
**	save_current_settings
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int save_current_settings(mw, x, y, width, height)
    ClockWidget mw;
    Position x, y;
    Dimension width, height;
{
    char level_value[10], geometry_value[40], resource_value[256],
      resource_name[256];
    XrmValue put_value;
    char* filename;

    if (UserDatabase(mw) == NULL)
	UserDatabase(mw) = GetAppUserDefaults(Dpy(mw));

    filename = ResolveFilename (Dpy(mw), CLASS_NAME, True, False);

    if (filename == NULL) return;
  /*if (UserDatabase(mw) == 0 ) return;*/

    sprintf(resource_value, "%d", AlarmOn(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", RepeatOn(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNrepeatOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", AlarmPm(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmPM);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    strcpy(resource_value, AlarmHr(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmHour);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    strcpy(resource_value, AlarmMin(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmMinute);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    strcpy(resource_value, AlarmMes(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmMessage);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", MilitaryTime(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNmilitaryOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", AnalogPresent(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNanalogOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", DigitalPresent(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNdigitalOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", DatePresent(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNdateOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", MenubarPresent(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNmenubarOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%dx%d+%d+%d", width, height, x, y);
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.%s", xrm_geometry);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", BellOn(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNbellOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

  if (!NO_SOUND)
  {
    sprintf(resource_value, "%d", AudioOn(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", SpeakerOn(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNspeakerOn);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    sprintf(resource_value, "%d", AudioVolume(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioVolume);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    strcpy(resource_value, AudioFname(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioFilename);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    strcpy(resource_value, AudioDirMask(mw));
    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;
    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioDirMask);
    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);
  }

/*  XrmPutFileDatabase(UserDatabase(mw), DefaultsName(mw));*/
    XrmPutFileDatabase(UserDatabase(mw), filename);
    XtFree(filename);
}

/*
**++
**  ROUTINE NAME:
**	restore_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int restore_settings_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    char *get_type, resource_name[256], resource_class[256];
    int ac, status, geometry_mask, intx, inty, NewAnalog,
      NewDigital, NewDate, NewMilitaryTime, NewMenubar, NewAlarm;
    unsigned int intwidth, intheight;
    Arg args[10];
    ClockWidget mw;
    Dimension width, height;
    Position x, y;
    XrmValue get_value;

    mw = clock_FindMainWidget(w);
    if (SystemDatabase(mw) == NULL)
	SystemDatabase(mw) = GetAppSystemDefaults(Dpy(mw));
    intx = -8;
    inty = 42;
    intwidth = 170;
    intheight = 100;
    sprintf(resource_name, "Clock.%s", xrm_geometry);
    sprintf(resource_class, "Clock.%s", xrc_geometry);

    status = XrmGetResource(SystemDatabase(mw), 
					/* Database. */
      resource_name, 			/* Resource's ASCIZ name. */
      resource_class, 			/* Resource's ASCIZ class. */
      &get_type, 			/* Resource's type (out). */
      &get_value);			/* Address to return value. */
    if (status != NULL) {
	geometry_mask =
	  XParseGeometry(get_value. addr, &intx, &inty, &intwidth, &intheight);
    }

    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCAlarmOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	AlarmOn(mw) = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNrepeatOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCRepeatOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL) {
	RepeatOn(mw) = atoi(get_value. addr);
    }

    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmPM);
    sprintf(resource_class, "Clock.Clock.%s", XtCAlarmPM);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	AlarmPm(mw) = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmHour);
    sprintf(resource_class, "Clock.Clock.%s", XtCAlarmHour);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	strcpy(AlarmHr(mw), get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmMinute);
    sprintf(resource_class, "Clock.Clock.%s", XtCAlarmMinute);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	strcpy(AlarmMin(mw), get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNalarmMessage);
    sprintf(resource_class, "Clock.Clock.%s", XtCAlarmMessage);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	strcpy(AlarmMes(mw), get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNmilitaryOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCMilitaryOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	NewMilitaryTime = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNanalogOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCAnalogOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	NewAnalog = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNdigitalOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCDigitalOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	NewDigital = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNdateOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCDateOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	NewDate = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNmenubarOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCMenubarOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	NewMenubar = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNbellOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCBellOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	BellOn(mw) = atoi(get_value. addr);

  if (!NO_SOUND)
  {
    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCAudioOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	AudioOn(mw) = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNspeakerOn);
    sprintf(resource_class, "Clock.Clock.%s", XtCSpeakerOn);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	SpeakerOn(mw) = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioVolume);
    sprintf(resource_class, "Clock.Clock.%s", XtCAudioVolume);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	AudioVolume(mw) = atoi(get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioFilename);
    sprintf(resource_class, "Clock.Clock.%s", XtCAudioFilename);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL)
	strcpy(AudioFname(mw), get_value. addr);

    sprintf(resource_name, "Clock.Clock.%s", DwtNaudioDirMask);
    sprintf(resource_class, "Clock.Clock.%s", XtCAudioDirMask);
    status = XrmGetResource(SystemDatabase(mw), resource_name, resource_class,
      &get_type, &get_value);
    if (status != NULL) 
	strcpy(AudioDirMask(mw), get_value. addr);
    else 
	strcpy(AudioDirMask(mw), "");
  }

    if (intx < 0)
	intx =
	  XWidthOfScreen(XDefaultScreenOfDisplay(Dpy(mw))) + intx - intwidth;
    if (inty < 0)
	inty =
	  XHeightOfScreen(XDefaultScreenOfDisplay(Dpy(mw))) + inty - intheight;
    if (intwidth < 100)
	intwidth = 100;
    if (intheight < 100)
	intheight = 100;

    x = (Position) intx;
    y = (Position) inty;
    width = (Dimension) intwidth;
    height = (Dimension) intheight;

    ac = 0;
    XtSetArg(args[ac], XmNx, x);
    ac++;
    XtSetArg(args[ac], XmNy, y);
    ac++;
    XtSetValues(XtParent(mw), args, ac);
    ac = 0;
    XtSetArg(args[ac], XmNwidth, width);
    ac++;
    XtSetArg(args[ac], XmNheight, height);
    ac++;
    XtSetValues((Widget) mw, args, ac);

    update_clock(mw, NewAnalog, NewDigital, NewDate, NewMilitaryTime,
      NewMenubar);

    if (Settings(mw) != NULL)
	reset_mode(mw);
    if (AlarmSettings(mw) != NULL)
	reset_alarm(mw);
}

/*
**++
**  ROUTINE NAME: audio_file_selection_box
**
**  FUNCTIONAL DESCRIPTION:
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
void audio_file_selection_box(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int reason;
{
  if (!NO_SOUND)
  {
    ClockWidget mw;
    int ac = 0;
    long cs_status, byte_count;
    Arg args[5];
    MrmType *dummy_class;
    XmFileSelectionBoxWidget fsbox_widget;
    XmString dir, dirmask;

    mw = clock_FindMainWidget(w);

    WorkCursorDisplay();
    if (AudioFSBoxWid(mw) == NULL) {
	if (MrmFetchWidget(ClockHierarchy(mw), "audio_file_select_dialog",
	         AlarmSettings(mw), &fsbox_widget, &dummy_class) != MrmSUCCESS)
	    clock_serror(mw, NoFSBox, WARNING);
	AudioFSBoxWid(mw) = (XmFileSelectionBoxWidget) fsbox_widget;

	ac=0;
	if (strcmp (AudioDirMask(mw), "") != 0) {
	    dirmask = DXmCvtFCtoCS(AudioDirMask(mw), &byte_count, &cs_status);
	    XtSetArg(args[ac], XmNdirMask, dirmask);
	    ac++;
	    XtSetValues((Widget) fsbox_widget, args, ac);
	} else {
#ifdef VMS
	    XtSetArg(args[ac], XmNdirMask, "AUDIO_VMS_DIR_MASK");
#else
	    XtSetArg(args[ac], XmNdirMask, "AUDIO_UNIX_DIR_MASK");
#endif
	    ac++;
	    MrmFetchSetValues(ClockHierarchy(mw), fsbox_widget, args, ac);
	}
    }
    XtManageChild( (Widget) AudioFSBoxWid(mw));
    WorkCursorRemove();
  }
}

/*
**++
**  ROUTINE NAME: audio_file_select_action
**
**  FUNCTIONAL DESCRIPTION:
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
void audio_file_select_action(w, tag, cbstruct)
    Widget w;
    caddr_t *tag;
    XmFileSelectionBoxCallbackStruct *cbstruct;
{
  if (!NO_SOUND)
  {
/*  char *dir; */
    char *dirmask;
    long byte_count, cvt_status;
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    if (cbstruct->reason == XmCR_OK) {
	DXmCSTextSetString(AudioFnameTextWid(mw), cbstruct->value);

/*	dir = DXmCvtCStoFC(cbstruct->dir, &byte_count, &cvt_status);
	strcpy (AudioDir(mw), dir);
*/
	dirmask = DXmCvtCStoFC(cbstruct->mask, &byte_count, &cvt_status);
	strcpy (AudioDirMask(mw), dirmask);

/*	XtFree(dir); */
	XtFree(dirmask);
    }
  }
}

/*
**++
**  ROUTINE NAME:
**	message_done_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int message_done_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) MessageWid(mw));
}

/*
**++
**  ROUTINE NAME:
**	error_message_done_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int error_message_done_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);
    XtUnmanageChild((Widget) ErrorMessageWid(mw));
}

/*
**++
**  ROUTINE NAME:
**	exit_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int exit_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

#ifndef NO_HYPERHELP
    if (hyper_help_context)
	DXmHelpSystemClose(hyper_help_context, clock_serror,
	  "Help Close Error");
#endif

    mw = clock_FindMainWidget(w);
    XCloseDisplay(Dpy(mw));
    exit(OK_STATUS);
}

/*
**++
**  ROUTINE NAME:
**	on_context_activate_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static void on_context_activate_proc(w, tag, reason, name, parent)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
    String name;
    Widget parent;
{

/* Change to use the DXm Help on context routine. */

    DXmHelpOnContext(toplevel, FALSE);

}

/*
**++
**  ROUTINE NAME:
**	tracking_help
**
**  FUNCTIONAL DESCRIPTION:
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
static void tracking_help()

{
    Widget track_widget;
    Cursor cursor;

    track_widget = NULL;

    cursor = XCreateFontCursor(XtDisplay(toplevel), XC_question_arrow);

    track_widget = XmTrackingLocate(toplevel, cursor, FALSE);

    if (track_widget != 0) {
	if (XtHasCallbacks(track_widget, XmNhelpCallback) == XtCallbackHasSome)
	    XtCallCallbacks(track_widget, XmNhelpCallback, NULL);
    } else {
	XtUngrabPointer(toplevel, CurrentTime);
    }
}

/*
**++
**  ROUTINE NAME:
**	main_help_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int main_help_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;
    XmString Frame;

    mw = clock_FindMainWidget(w);

    Frame = (XmString) tag;
    clock_DisplayHelp(mw, Frame);
}

/*
**++
**  ROUTINE NAME:
**	clock_DisplayHelp
**
**  FUNCTIONAL DESCRIPTION:
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
void clock_DisplayHelp(mw, Frame)
    ClockWidget mw;
    XmString Frame;
{
#ifndef NO_HYPERHELP
    /* Hyperhelp */
    if (!hyper_help_context) {

	WorkCursorDisplay();
	DXmHelpSystemOpen(&hyper_help_context, toplevel, CLOCK_HELP,
	  clock_display_message, NoHelpWidget);
	WorkCursorRemove();
    }

    WorkCursorDisplay();
    DXmHelpSystemDisplay(hyper_help_context, CLOCK_HELP, "Topic", Frame,
      clock_display_message, NoHelpWidget);
    WorkCursorRemove();
#else
    Arg args[5];
    int ac = 0;
    long cs_status, byte_count;
    Widget help_widget;
    MrmType *dummy_class;
    XmString help_lib_name_cs;
    XtWidgetGeometry request, reply;
    /* XtGeometryResult	geo_result; */


/*
    help_lib_name_cs = XmStringCreate("/usr/lib/help/clock", XmSTRING_DEFAULT_CHARSET);
*/

    help_lib_name_cs = DXmCvtFCtoCS("clock", &byte_count, &cs_status);
    XtSetArg(args[ac], DXmNlibrarySpec, help_lib_name_cs);
    ac++;

    if (HelpWidget(mw) == 0) {
	WorkCursorDisplay();
	if (MrmFetchWidgetOverride(ClockHierarchy(mw), 
					/* hierarchy id                 */
	  "main_help", 			/* Index of widget to fetch     */
	  mw, 				/* Parent of widget             */
	  NULL, 			/* Override name                */
	  args, 			/* Override args                */
	  ac, 				/* Override args count          */
	  &help_widget, 		/* Widget id                    */
	  &dummy_class) != MrmSUCCESS)	/* Widget Class                 */
	  {
	    clock_display_message(mw, NoHelpWidget, WARNING);
	}

	HelpWidget(mw) = help_widget;
	WorkCursorRemove();
    }

    if (HelpWidget(mw) != 0) {
	ac = 0;
	XtSetArg(args[ac], DXmNfirstTopic, Frame);
	ac++;
	XtSetValues(HelpWidget(mw), args, ac);
	XtManageChild(HelpWidget(mw));
	request. request_mode = CWStackMode;
	request. stack_mode = Above;

    }
#endif
}

/*
**++
**  ROUTINE NAME:
**	help_done_proc
**
**  FUNCTIONAL DESCRIPTION:
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
static unsigned int help_done_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    ClockWidget mw;

    mw = clock_FindMainWidget(w);

    XtUnmanageChild(HelpWidget(mw));
}

/*
**++
**  ROUTINE NAME:
**	clock_serror
**
**  FUNCTIONAL DESCRIPTION:
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
void clock_serror(parent, str, level)
    Widget parent;
    char *str;
    int level;
{
    Widget MW;

    printf("%s \n", str);
    if (level == FATAL)
	exit(ERROR_STATUS);
}

/*
**++
**  ROUTINE NAME:
**	clock_display_message
**
**  FUNCTIONAL DESCRIPTION:
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
void clock_display_message(parent, str, level)
    Widget parent;
    char *str;
    int level;
{
    Widget MW;

    printf("%s \n", str);

    if (level == FATAL)
	exit(ERROR_STATUS);

}
