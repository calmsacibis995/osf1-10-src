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
#define SS_NORMAL 1
#include <stdio.h>
#include "smdata.h"
#include "smresource.h"

#include "Xm/PushB.h"

extern	int	end_session();
extern	int	pause_session();
extern	int	setup_menu_cb();
extern	int	session_menu_cb();
#ifdef DOPRINT
extern	int	print_menu_cb();
#endif
#ifdef DOHELP
extern	int	help_menu_cb();
#ifdef HYPERHELP
extern  void    help_system_proc();
#endif
#endif

extern	int	create_menu_cb();
extern	int	client_message();
extern	int	map_event();
extern	Widget	MyCreateWidget();
void	widget_create_proc();

extern OptionsRec options;

int	create_panel()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the session manager main window, menu bar, and control
**	panel.
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
Widget	main_window = NULL;
Arg	args[30];

static XtTranslations main_translations_parsed; /* Translation table */
static XtTranslations label_translations_parsed; 

static char main_translation_table [] = 
    "<ClientMessage>:	   	client_message()";

static char label_translation_table [] = 
     "<MapNotify>:	   	map_event()";

/*
 * now establish an action binding table which associates the above
 * procedure names (ascii) with actual addresses
 */

static XtActionsRec our_action_table [] = {
    {"client_message",	(XtActionProc) client_message},
    {"map_event",	(XtActionProc) map_event}
};

static Arg pulldown_args[] = {
    {XmNsubMenuId, (XtArgVal)NULL},
};

static MrmRegisterArg reglist[] = {
        {"widget_create_proc", (caddr_t) widget_create_proc},
	{"create_menu_cb", (caddr_t)create_menu_cb},
	{"session_menu_cb", (caddr_t)session_menu_cb},
#ifdef DOPRINT
	{"print_menu_cb", (caddr_t)print_menu_cb},
#endif
#ifdef DOHELP
	{"help_menu_cb", (caddr_t)help_menu_cb},
#ifdef HYPERHELP
	{"help_system_proc", (caddr_t)help_system_proc},
#endif
#endif
	{"customize_menu_cb", (caddr_t)setup_menu_cb},
	{"createmenu_id", (caddr_t)&smdata.create_menu},
	{"pausebutton", (caddr_t)&smdata.pause_button},
	{"quitbutton", (caddr_t)&smdata.quit_button},
	{"appsdefbutton", (caddr_t)&smdata.appdefs_button},
	{"appsmenubutton", (caddr_t)&smdata.appmenu_button},
	{"autostartbutton", (caddr_t)&smdata.autostart_button},
	{"keybutton", (caddr_t)&smdata.key_button},
	{"internationalbutton", (caddr_t)&smdata.international_button},
	{"pointerbutton", (caddr_t)&smdata.pointer_button},
#ifdef DOPRINT
	{"printerbutton", (caddr_t)&smdata.printer_button},
#endif /* DOPRINT */
	{"screenbutton", (caddr_t)&smdata.screen_button},
	{"securitybutton", (caddr_t)&smdata.security_button},
	{"smbutton", (caddr_t)&smdata.sm_button},
	{"windowbutton", (caddr_t)&smdata.window_button},
	{"use_last_button", (caddr_t)&smdata.use_last_button},
	{"use_system_button", (caddr_t)&smdata.use_system_button},
	{"save_current_button", (caddr_t)&smdata.save_current_button},
#ifdef DOHELP
#ifndef HYPERHELP
	{"helpoverviewbutton", (caddr_t)&smdata.help_overview},
	{"helpaboutbutton", (caddr_t)&smdata.help_about},
#endif
#endif
#ifdef DOPRINT
	{"printesbutton", (caddr_t)&smdata.print_es},
	{"printposbutton", (caddr_t)&smdata.print_pos},
	{"captureesbutton", (caddr_t)&smdata.capture_es},
	{"captureposbutton", (caddr_t)&smdata.capture_pos},
#endif /* DOPRINT */
};
static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    main_translations_parsed = XtParseTranslationTable(main_translation_table);
    label_translations_parsed
	 = XtParseTranslationTable(label_translation_table);

    MrmRegisterNames (reglist, reglist_num);

    /* make our action table know to the toolkit */
    XtAddActions (our_action_table, 2);	

    /* create the window */
    MrmFetchWidgetOverride(s_DRMHierarchy, "MainWindow", smdata.toplevel,
		        NULL, NULL, 0, &main_window, &drm_dummy_class);

    /* set the window to icon if the user specified that in xdefaults */
    if (smsetup.startup_state == iconified) {
	XtSetArg(args[0], XtNiconic, 1);
	XtSetValues(smdata.toplevel, args, 1);
    }

    XtMoveWidget(smdata.toplevel, smsetup.x, smsetup.y);
    get_customized_menu(smdata.create_menu);

    if (ScreenCount(display_id) > 1) {
	/* put the Screen setup button on the pulldown */
	XtManageChild(smdata.screen_button);
    }

    XtManageChild(main_window);
    XtRealizeWidget(smdata.toplevel);
/*<<< ifdef ONE_DOT_TWO */
    sm_inited = TRUE;
    return(1);
}

int move_event()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the control panel is moved.  We want to save the
**	current position of the control panel for the next time the user
**	logs in.  This routine will get the x and y of the 
**	moved resized dialog box and save them in the resource database.
**	This database is then written to the session manager resource
**	file if the user saves settings when ending the session.
**
**  FORMAL PARAMETERS:
**
**	standard callback parameters not used.
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
Arg arglist[5];
unsigned	int	ivalue,status;
char	astring[10];	
Position    x = 0;
Position    y = 0;
int tmpx, tmpy;
Window tmpchild;

/* Get the current width and height of the dialog box */
if (strcmp(options.session_wm, "System Default") == 0 ||
    strcmp(options.session_wm, options.session_default_wm) == 0) {
  XtSetArg (arglist[0], XmNx, &x);
  XtSetArg (arglist[1], XmNy, &y);
  XtGetValues (smdata.toplevel, arglist, 2);
} else {
  /* not default WM, don't attempt to determine x and y */
  x = smsetup.x;
  y = smsetup.y;
}

if (x != smsetup.x)
    {
    /* If the width has changed, then save the new resource */
    smsetup.x = x;
    status = int_to_str(smsetup.x, astring, sizeof(astring));
    if (status == SS_NORMAL)
	{
	sm_put_resource(smx, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }

if (y != smsetup.y)
    {
    /* If the height has changed, then save the new resource */
    smsetup.y = y;
    status = int_to_str(smsetup.y, astring, sizeof(astring));
    if (status == SS_NORMAL)
	{
	sm_put_resource(smy, astring);
	}
   /* Setting this variable means that the user will be asked if they
      want to save their settings when they end the session */
    smdata.resource_changed = 1;
    }
}


int map_event(w, data, event)
Widget  w;
int	*data;
XMapEvent        *event;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine ensures that the message area label is centered
**	in the dialog box.   To know that it is centered, we need to
**	know the width of the dialog box which is not known until the
**	dialog box is mapped.  Get the width and then attach the label
**	to the left edge of the dialgo box.  A negative offset
**	will move it to the right.  width/4 will make it centered.
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
Arg	arglist[20];
Dimension   count;

XtSetArg (arglist[0], XmNwidth, &count);
XtGetValues (w, arglist, 1);
XtSetArg (arglist[0], XmNleftOffset, (XtArgVal)(-(count/4)));
XtSetValues(w, arglist, 1);
}

Widget MyCreateWidget(name, widgetClass, parent, args, num_args)
    String      name;
    WidgetClass widgetClass;
    Widget      parent;
    ArgList     args;
    Cardinal    num_args;
{
Widget	retwid = NULL;

MrmFetchWidget(s_DRMHierarchy, name, parent, &retwid, &drm_dummy_class);

if (num_args != 0)
    {
    XtSetValues(retwid, args, num_args);
    }
return(retwid);
}

get_customized_menu(parent)
Widget	parent;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Create the application menu from resource files
**
**  FORMAL PARAMETERS:
**
**	parent - The parent for this menu pulldown
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
char	menulist[1024];
unsigned    int i,j,size;
char	*current_p;
Arg	arglist[3];
int  value[4];

if (parent == 0) return;

/* Get the number of items on menu bar */
sm_get_int_resource(inumappmenu, value);
if (value[0] != 0)
    {
    /* Get the application menu string */
    size = sm_get_string_resource(iappmenu, menulist);
    if (size == 0)
	{
	fprintf(stderr, "couldn't generate the create menu");
	return;
	}
    for (i=0,j=0; i<strlen(menulist); i++)
	if (menulist[i] == ',')
	    j++;
    j++;
    }
else
    j=0;

/* Free the old menu widgets and memory */
if (smdata.create_button != NULL)
    {
    for (i=0; i<smdata.create_count; i++)
	{
	XtUnmanageChild(smdata.create_button[i]);
	XtDestroyWidget(smdata.create_button[i]);
	}
    XtFree((char *)smdata.create_button);
    XtSetArg (arglist[0], XmNheight, 1);
    XtSetArg (arglist[1], XmNwidth, 1);
    XtSetValues (parent, arglist, 2);
    smdata.create_button = NULL;
    smdata.create_count = 0;
    }

if (value[0] == 0) return;

smdata.create_button = (Widget *)XtMalloc(j*sizeof(Widget));
smdata.create_count = j;

for (i=0,current_p=menulist; i<j; i++)
    {
    char    *comma;
    char    *menuname;
    int	length;

    comma = strchr(current_p, ',');
    if (comma == 0) comma = strlen(menulist) + menulist;
    length = comma-current_p;
    menuname = XtMalloc(length + 1);
    strncpy(menuname,current_p,length);
    menuname[length] = '\0';
    
    XtSetArg(arglist[0], XmNlabelString,
	     XmStringCreate(menuname, def_char_set));
    smdata.create_button[i]
	= XtCreateWidget(menuname,xmPushButtonWidgetClass, parent, arglist, 1);

    XtManageChild(smdata.create_button[i]);
    XtFree(menuname);
    current_p = current_p + length + 1;
    }
}
