
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
/* xkeycaps, Copyright (c) 1991 Jamie Zawinski <jwz@lucid.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#include "version.h"

#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <Xm/Form.h>
#include <X11/Xmu/Error.h>

#include "KbdWidgetP.h"

#include "xkeycaps.h"
#include "defaults.h"

char *progname;
Widget global_menu;
Widget commands_menu;
/*
**	initialize to known (unknown?) state -bg
*/
KeyWidget CurrentKey = (KeyWidget) 0;
Opaque help_context;

extern struct info_labels *make_info_widget ();
extern Widget make_menu_bar();
extern struct key_menus *make_key_menus ();
extern void keyboard_handle_mapping_notify ();
extern void keyboard_handle_keymap_notify ();

/*
      <KeyDown>F4:	PopupMenu()	                \n\
      <KeyDown>F10, <KeyUp>F10:	PopupMenu()	                \n\
*/

static XtTranslations keyboard_translations_parsed; 

static char keyboard_translations_table [] = 
  {
    "#override\n\
            <Motion>:	DescribeKey(mouse, unlessTracking)	\n\
            <KeyDown>F4:	PopupMenu()	                \n\
            <KeyDown>F10, <KeyUp>F10:	PopupMenu()	        \n\
            !Ctrl<KeyDown>F4, !Ctrl<KeyUp>F4:  HighlightKey()	       	\
		                 DescribeKey(unlessMod)			\
		                 DescribeKey(displayed)			\
		                 SimulateKeyPress()			\
                                 UnhighlightKey()			\
                                 SimulateKeyRelease()			\n\
            <KeyDown>Return, <KeyUp>Return:  HighlightKey()		\
		                 DescribeKey(unlessMod)			\
		                 DescribeKey(displayed)			\
		                 SimulateKeyPress()			\
                                 UnhighlightKey()			\
                                 SimulateKeyRelease()			\n\
            <KeyDown>, <KeyUp>:	HighlightKey()				\
		        DescribeKey()			\
		        DescribeKey(displayed)			\
		        SimulateKeyPress()			\
                        UnhighlightKey()			\
                        SimulateKeyRelease()			\n\
            <Btn1Down>:	HighlightKey(unlessMod)		\
		ToggleKey(ifMod)			\
		TrackKey(unlessMod)			\
		SimulateKeyPress(ifHighlighted)		\
		SimulateKeyRelease(unlessHighlighted)	\n\
            <Btn1Up>:	UntrackKey(highlighted)		\
		SimulateKeyRelease(highlighted, unlessMod) \
		UnhighlightKey(highlighted, unlessMod)	\n\
            <Btn3Down>:	PopupMenu()"
  };

xkeycaps_error_handler (dpy, error)
     Display *dpy;
     XErrorEvent *error;
{
  switch (error->request_code)
    {
    case X_GetKeyboardMapping:
      return 0;
    case X_SendEvent:
      XmuPrintDefaultErrorMessage (dpy, error, stderr);
      return 0;
    default:
      XmuPrintDefaultErrorMessage (dpy, error, stderr);
      exit (-1);
    }
}


static KeyboardWidget
make_keyboard (box, box2, info, name)
     Widget box;
     Widget box2;
     struct info_labels *info;
     char *name;
{
  Arg av[20];
  int ac = 0;
  KeyboardWidget keyboard;

  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, box2); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
  
  if (name) XtSetArg (av[ac], "keyboard", name), ac++;
  keyboard = (KeyboardWidget) XtCreateManagedWidget ("keyboard",
						     keyboardWidgetClass,
						     box, av, ac);
  keyboard->keyboard.label_widgets = info;
  keyboard->keyboard.key_menus = make_key_menus (keyboard);
  keyboard_translations_parsed =
    XtParseTranslationTable(keyboard_translations_table);
  XtOverrideTranslations((Widget)keyboard, keyboard_translations_parsed);
  return keyboard;
}


static void
maybe_relabel_window (keyboard)
     KeyboardWidget keyboard;
{
  /* If the user hasn't specified the -title option, set the window title
     to be something more informative.
   */
  Widget toplevel = (Widget) keyboard;
  char buf1 [100], buf2 [100];
  XrmValue value;
  char *type;
  XrmDatabase db = XtDatabase (XtDisplay (keyboard));
  char *name, *class;
  while (XtParent (toplevel)) toplevel = XtParent (toplevel);
  XtGetApplicationNameAndClass (XtDisplay (keyboard), &name, &class);
  sprintf (buf1, "%s.title", name);
  sprintf (buf2, "%s.Title", class);
  if (XrmGetResource (db, buf1, buf2, &type, &value)) return;
  sprintf (buf1, "%s Keyboard", keyboard->keyboard.kbd->name);
  XStoreName (XtDisplay (toplevel), XtWindow (toplevel), buf1);
}


void
replace_keyboard (keyboard, name)
     KeyboardWidget *keyboard;
     char *name;
{
  /* Doing this seems to leak about 8k each time, but I really don't care. */
  Widget box, box2;
  KeyboardWidget new_kbd;
  Widget toplevel;
  Arg av[20];
  int ac = 0;
  toplevel = box = XtParent (*keyboard);
  while (XtParent (toplevel)) toplevel = XtParent (toplevel);

  XtSetArg (av[ac], XmNtopWidget, &box2); 
  XtGetValues ((Widget) *keyboard, av, 1);
  new_kbd = make_keyboard (box, box2, (*keyboard)->keyboard.label_widgets,
			   name);
  XtUnmanageChild ((Widget) *keyboard);
  XtDestroyWidget ((Widget) *keyboard);
  *keyboard = new_kbd;
  XtSetKeyboardFocus (toplevel, (Widget) *keyboard);
  maybe_relabel_window (*keyboard);
}

void help_error(problem_string, status)
    char *problem_string;
    int status;
{
    fprintf(stderr, "%s, %x\n", problem_string, status);
}
/*
**  new flag from command line -- force dxkeycaps to ignore
**  what the kernel told us our specific keyboard is, and
**  give user ALL possible keyboard options (e.g. lk401, lk201,
**  lk421 and lk443 instead of just lk201).
*/
int forceAll = 0;

main (argc, argv)
     int argc;
     char **argv;
{
  char *class = "DXkeycaps";
  XtAppContext app;
  Widget toplevel, box, box2, menubar;
  struct info_labels *info;
  KeyboardWidget keyboard = 0;
  struct keyboard *kbd;
  Arg av [20];
  int ac = 0;
  int i;
  char *tmp;

  MrmInitialize();
  DXmInitialize();
  /*
   * assume we'll choose the correct keyboard for the user by
   * setting the forceAll flag to FALSE. Then scan the command 
   * line parameters to see if the user is specifying something. 
   * If this is the case, set forceAll to TRUE, to be sure we'll 
   * include ALL possible keyboards and allow the widest possible 
   * latitude in available choices. -bg
   */
  forceAll = 0;
  for (i = 1; i < argc; i++)
  {
	  if (!strcmp ("-kbd", argv[i]) 
		  || !strcmp ("-keyboard", argv[i]))
	  {
		  forceAll = 1;
		  break;
	  }
  }
  toplevel = XtAppInitialize (&app, class, options, XtNumber (options),
			      &argc, argv, DXkeycapsDefaults, NULL, 0);

  XtGetApplicationNameAndClass (XtDisplay (toplevel), &progname, &class);
  if (argc > 1)
  {
    int	tempix;

    for (tempix = 1; tempix < argc; tempix++)
    {
	if (! strcmp (argv[tempix], "-all"))
	{
	    forceAll = 1;
	}
    }
    if (! forceAll)
    {
	fprintf (stderr, "%s: unknown option %s\n", progname, argv [1]);
	exit (-1);
    }
  }
  ac = 0;
  /*
  XtSetArg(av[ac], XmNtranslations, XtParseTranslationTable("")); ac++;
  */
  box = XtCreateManagedWidget ("form1", xmFormWidgetClass, toplevel, av, ac);
  /*
  **	need to pass Display * to make_menu_bar so it can pass it
  **	on to create_commands_menu, which uses it to determine what
  **	kind of keyboard is on the display that's running...
  */
  menubar = make_menu_bar (XtDisplay(toplevel),box, &keyboard);
  if (! menubar)
  {
	fprintf (stderr, "Cannot determine keyboard type\n");
	exit (1);
  }

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  XtSetValues(menubar, av, ac);

  ac = 0;
  XtSetArg (av[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
  XtSetArg (av[ac], XmNtopWidget, menubar); ac++;
  XtSetArg (av[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
  XtSetArg (av[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
  box2 = XtCreateManagedWidget ("form2", xmFormWidgetClass, box, av, ac);
  info = make_info_widget (box2);

  keyboard = make_keyboard (box, box2, info, 0);

  XtAddEventHandler ((Widget) keyboard, KeymapStateMask, False,
		     keyboard_handle_keymap_notify, 0);
  XtAddEventHandler ((Widget) keyboard, 0, True,
		     keyboard_handle_mapping_notify, 0);

  XtRealizeWidget (toplevel);
  XtSetKeyboardFocus (toplevel, (Widget) keyboard);
  maybe_relabel_window (keyboard);
  XSetErrorHandler (xkeycaps_error_handler);

  message (keyboard, "");
  /*
  message2 (keyboard, version);
  */

#ifdef HYPERHELP
  /* open help system */
  DXmHelpSystemOpen(&help_context, toplevel, "dxkeycaps", help_error,
			"dxkeycaps: help system error");
#endif

  while (1)
    {
      XEvent event;
      XtAppNextEvent (app, &event);

      /* MappingNotify and KeymapNotify events don't have an associated
	 window, so there's no way to register an event-handler function
	 for one of these with Xt.  Lose, lose.
       */
      if (event.xany.type == KeymapNotify)
	keyboard_handle_keymap_notify (keyboard, 0, &event);
      else if (event.xany.type == MappingNotify)
	keyboard_handle_mapping_notify (keyboard, 0, &event);

      XtDispatchEvent (&event);
    }
}
