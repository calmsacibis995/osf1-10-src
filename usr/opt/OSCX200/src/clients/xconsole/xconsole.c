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
 * $XConsortium: xconsole.c,v 1.9 91/07/25 14:23:46 rws Exp $
 *
 * Copyright 1990 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

#include <X11/Shell.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Dialog.h>
#include <X11/Xaw/Cardinals.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/BulletinB.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/Label.h>

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>

#ifdef __osf__
#include <sys/sysinfo.h>
#include <machine/hal_sysinfo.h>
#endif

#include <dlfcn.h>

/* Fix ISC brain damage.  When using gcc fdopen isn't declared in <stdio.h>. */
#if defined(SYSV) && defined(SYSV386) && defined(__STDC__) && defined(ISC)
extern FILE *fdopen(int, char const *);
#endif

static Boolean	use_motif;

static long	TextLength ();

static Widget	top, text;

static XtInputId	input_id;

static FILE    *input;

static Boolean	notified;
static Boolean	iconified;

static Atom	wm_delete_window;
static Atom	mit_console;
#define MIT_CONSOLE_LEN	12
#define MIT_CONSOLE "MIT_CONSOLE_"
static char	mit_console_name[255 + MIT_CONSOLE_LEN + 1] = MIT_CONSOLE;

#define MAXSIZE_DEF (1<<15)
#define LINE_CUT_INCR 5

typedef struct 
    {
    Widget      top;
    Widget      message;
    Widget      messageTextWidget;
    } ConsCatcherRec, *ConsCatcher;

static ConsCatcherRec cons_req;
static ConsCatcher cons_c = &cons_req;

static struct _app_resources {
    Boolean motif;
    Boolean readStdin;
    int     maxSize;
    char    *file;
    Boolean buttons;
    Boolean stripNonprint;
    Boolean notify;
    Boolean daemon;
    Boolean verbose;
    Boolean exitOnFail;
} app_resources;

#define Offset(field) XtOffsetOf(struct _app_resources, field)

/*
 * If you move the motif and stdin resources from positions 0 and 1 in this
 * array, be sure to modify the code down in main() that refers to them at
 * these positions.  Also, the file resource is referred to at postion 3 in
 * main.
 */
static XtResource  resources[] = {
    {"motif",	"Motif",    XtRBoolean,	sizeof (Boolean),
	Offset (motif),XtRImmediate, (XtPointer)False},
    {"stdin",	"Stdin",    XtRBoolean,	sizeof (Boolean),
	Offset (readStdin),XtRImmediate, (XtPointer)False},
    {"maxSize",	"MaxSize",    XtRInt,	sizeof (Cardinal),
	Offset (maxSize),XtRImmediate, (XtPointer)MAXSIZE_DEF},
    {"file",	"File",	    XtRString,	sizeof (char *),
	Offset (file),	XtRString,  "console" },
    {"buttons",	"Buttons",    XtRBoolean,	sizeof (Boolean),
	Offset (buttons),XtRImmediate, (XtPointer)True},
    {"notify",	"Notify",   XtRBoolean,	sizeof (Boolean),
	Offset (notify), XtRImmediate, (XtPointer)True },
    {"stripNonprint",	"StripNonprint",    XtRBoolean, sizeof (Boolean),
	Offset (stripNonprint), XtRImmediate, (XtPointer)True },
    {"daemon",		"Daemon",	    XtRBoolean,	sizeof (Boolean),
	Offset (daemon), XtRImmediate, (XtPointer)False},
    {"verbose",		"Verbose",	    XtRBoolean,	sizeof (Boolean),
	Offset (verbose),XtRImmediate, (XtPointer)False},
    {"exitOnFail",	"ExitOnFail",    XtRBoolean,	sizeof (Boolean),
	Offset (exitOnFail),XtRImmediate, (XtPointer)False},
};

#undef Offset

static XrmOptionDescRec options[] = {
    {"-file",	    "*file",		XrmoptionSepArg,    NULL},
    {"-notify",	    "*notify",		XrmoptionNoArg,	    "TRUE"},
    {"-nonotify",   "*notify",		XrmoptionNoArg,	    "FALSE"},
    {"-daemon",	    "*daemon",		XrmoptionNoArg,	    "TRUE"},
    {"-verbose",    "*verbose",		XrmoptionNoArg,	    "TRUE"},
    {"-exitOnFail", "*exitOnFail",	XrmoptionNoArg,	    "TRUE"},
    {"-motif", 	    "*motif",		XrmoptionNoArg,	    "TRUE"},
    {"-stdin", 	    "*stdin",		XrmoptionNoArg,	    "TRUE"},
    {"-nostdin",    "*stdin",		XrmoptionNoArg,	    "FALSE"},
    {"-maxSize",    "*maxSize",		XrmoptionSepArg,    NULL},
    {"-buttons",    "*buttons",		XrmoptionNoArg,	    "TRUE"},
    {"-nobuttons",  "*buttons",		XrmoptionNoArg,	    "FALSE"},
};

#define ICON_SIZE 48

static char icon_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x00,
   0x88, 0xff, 0xff, 0xff, 0x47, 0x01, 0x48, 0x00, 0x00, 0x00, 0xc8, 0x02,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x05, 0x28, 0xff, 0x3f, 0x00, 0xd0, 0x0a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0x7f, 0x00, 0x00, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0xff, 0xff, 0x0f, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0xff, 0x00, 0x00, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0xff, 0x7f, 0x00, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0x3f, 0x00, 0x00, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x28, 0x00, 0xe0, 0x81, 0xd0, 0x1a,
   0x28, 0x00, 0xe0, 0x41, 0x50, 0x1d, 0x28, 0x00, 0xc0, 0x23, 0xd0, 0x1a,
   0x28, 0x00, 0x80, 0x27, 0x50, 0x1d, 0x28, 0x00, 0x00, 0x17, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x0b, 0x50, 0x1d, 0x28, 0x00, 0x00, 0x1d, 0xd0, 0x1a,
   0x28, 0x00, 0x80, 0x3c, 0x50, 0x1d, 0x28, 0x00, 0x80, 0x78, 0xd0, 0x1a,
   0x28, 0x00, 0x40, 0x78, 0x50, 0x1d, 0x28, 0x00, 0x20, 0xf0, 0xd0, 0x1a,
   0x28, 0x00, 0x00, 0x00, 0x50, 0x1d, 0x48, 0x00, 0x00, 0x00, 0xc8, 0x1e,
   0x88, 0xff, 0xff, 0xff, 0x47, 0x0f, 0x08, 0x00, 0x00, 0x00, 0xc0, 0x07,
   0xf0, 0xff, 0xff, 0xff, 0xff, 0x03, 0xe0, 0xff, 0xff, 0xff, 0xff, 0x01,
   0xc0, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



XmTextPosition 	(*_xc_XmTextGetLastPosition) (Widget);
void		(*_xc_XmTextReplace) (Widget, XmTextPosition, XmTextPosition,
					char *);
char *		(*_xc_XmTextGetString) (Widget);
Widget		(*_xc_XmCreateForm) (Widget, String, ArgList, Cardinal);
XmString	(*_xc_XmStringLtoRCreate) (char *, XmStringCharSet);
Widget		(*_xc_XmCreatePushButton) (Widget, char *, ArgList, Cardinal);
Widget		(*_xc_XmCreateLabel) (Widget, String, ArgList, Cardinal);
Widget		(*_xc_XmCreateScrolledText) (Widget, String, ArgList,
						Cardinal);
XmTextPosition	(*_xc_XmTextGetInsertionPosition) (Widget);
void		(*_xc_XmTextSetInsertionPosition) (Widget, XmTextPosition);

#ifdef ultrix
#define USE_FILE
#define FILE_NAME   "/dev/xcons"
#endif

#ifndef USE_FILE
#include    <sys/ioctl.h>
#ifdef SVR4
#include    <termios.h>
#include    <sys/stropts.h>		/* for I_PUSH */
#endif

#ifdef TIOCCONS
#define USE_PTY
static int  tty_fd, pty_fd;
static char ttydev[64], ptydev[64];
#endif
#endif

#if defined(SYSV) && defined(SYSV386)
#define USE_OSM
#endif

static void inputReady ();

static
OpenConsole ()
{
    input = 0;
    if (app_resources.file)
    {
	if (!strcmp (app_resources.file, "console"))
	{
	    struct stat sbuf;
	    /* must be owner and have read/write permission */
	    if (!stat("/dev/console", &sbuf) &&
		(sbuf.st_uid == getuid()) &&
		!access("/dev/console", R_OK|W_OK))
	    {
#ifdef USE_FILE
	    	input = fopen (FILE_NAME, "r");
#endif
#ifdef USE_PTY
		int	on = 1;

		if (get_pty (&pty_fd, &tty_fd, ttydev, ptydev) == 0 &&
		    ioctl (tty_fd, TIOCCONS, (char *) &on) != -1)
		{
		    input = fdopen (pty_fd, "r");
		}
#endif
	    }
#ifdef USE_OSM
	    /* Don't have to be owner of /dev/console when using /dev/osm. */
	    input = fdopen(osm_pipe(), "r");
#endif
	    if (input && app_resources.verbose)
	    {
		char	*hostname;
		TextAppend (text, "Console log for ", 16);
		hostname = mit_console_name + MIT_CONSOLE_LEN;
		TextAppend (text, hostname, strlen (hostname));
		TextAppend (text, "\n", 1);
	    }
	}
	else
	{
	    input = fopen (app_resources.file, "r");
	}
	if (!input)
	{
	    if (app_resources.exitOnFail)
		exit(0);
	    TextAppend (text, "Couldn't open ", 14);
	    TextAppend (text, app_resources.file, strlen (app_resources.file));
	    TextAppend (text, "\n", 1);
	}
    }
    else
	input = stdin;

    if (input)
    {
	input_id = XtAddInput (fileno (input), (XtPointer) XtInputReadMask,
			       inputReady, (XtPointer) text);
    }
    /* if acting like dxconsole, add stdin as input, too */
    if (app_resources.readStdin && (input != stdin))
	{
	/*
	 * This is fcntl() is removed because it causes problems with
	 * ksh and sh if dxconsole is killed by ctrl-C
	 * Removing it causes no problems that I can discover.  PBD
	 *
	 * fcntl(0, F_SETFL, O_NDELAY);
	 */
	XtAddInput(0,  (XtPointer) XtInputReadMask,
		   inputReady, (XtPointer) text);
	}
}


static
CloseConsole ()
{
    if (input) {
	XtRemoveInput (input_id);
	fclose (input);
    }
#ifdef USE_PTY
    close (tty_fd);
#endif
}

/*ARGSUSED*/
static void
Quit (widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    exit (0);
}

extern char *malloc ();

static void
Notify ()
{
    Arg	    arglist[1];
    char    *oldName;
    char    *newName;

    if (!iconified || !app_resources.notify || notified)
	return;
    XtSetArg (arglist[0], XtNiconName, &oldName);
    XtGetValues (top, arglist, 1);
    newName = malloc (strlen (oldName) + 3);
    if (!newName)
	return;
    sprintf (newName, "%s *", oldName);
    XtSetArg (arglist[0], XtNiconName, newName);
    XtSetValues (top, arglist, 1);
    free (newName);
    notified = True;
}

/*ARGSUSED*/
static void
Deiconified (widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg	    arglist[1];
    char    *oldName;
    char    *newName;
    int	    oldlen;

    iconified = False;
    if (!app_resources.notify || !notified)
	return;
    XtSetArg (arglist[0], XtNiconName, &oldName);
    XtGetValues (top, arglist, 1);
    oldlen = strlen (oldName);
    if (oldlen >= 2) {
    	newName = malloc (oldlen - 1);
    	if (!newName)
	    return;
    	strncpy (newName, oldName, oldlen - 2);
	newName[oldlen - 2] = '\0';
    	XtSetArg (arglist[0], XtNiconName, newName);
    	XtSetValues (top, arglist, 1);
    	free (newName);
    }
    XtSetArg(arglist[0], XmNiconic, 0);
    XtSetValues(top, arglist, 1);
    notified = False;
}

/*ARGSUSED*/
static void
Iconified (widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{
    Arg	    arglist[1];

    XtSetArg(arglist[0], XmNiconic, 1);
    XtSetValues(top, arglist, 1);
    iconified = True;
}

/*ARGSUSED*/
static void
Clear (widget, event, params, num_params)
    Widget widget;
    XEvent *event;
    String *params;
    Cardinal *num_params;
{

    if (use_motif)
	{
	XmTextPosition pos;

	pos = (*_xc_XmTextGetLastPosition)(cons_c->messageTextWidget);
	(*_xc_XmTextReplace)(cons_c->messageTextWidget, 0, pos, "");
	}
    else
	{
	long	    last;
	XawTextBlock    block;

	last = TextLength (text);
	block.ptr = "";
	block.firstPos = 0;
	block.length = 0;
	block.format = FMT8BIT;
	TextReplace (text, 0, last, &block);
	}
}

static XtActionsRec actions[] = {
    "Quit",	    Quit,
    "Iconified",    Iconified,
    "Deiconified",  Deiconified,
    "Clear",	    Clear,
};

static void
stripNonprint (b)
    char    *b;
{
    char    *c;

    c = b;
    while (*b)
    {
	if (isprint (*b) || isspace (*b) && *b != '\r')
	{
	    if (c != b)
		*c = *b;
	    ++c;
	}
	++b;
    }
    *c = '\0';
}

static void
inputReady (w, source, id)
    XtPointer	w;
    int		*source;
    XtInputId	*id;
{
    char    buffer[1025];
    int	    n;

    n = read (*source, buffer, sizeof (buffer) - 1);
    if (n <= 0)
    {
	if (*source != 0)
	    fclose (input);
	XtRemoveInput (*id);
    }
    Notify ();
    buffer[n] = '\0';
    if (app_resources.stripNonprint)
    {
	stripNonprint (buffer);
	n = strlen (buffer);
    }
    TextAppend ((Widget) text, buffer, n);
}

static Boolean
ConvertSelection (w, selection, target, type, value, length, format)
    Widget w;
    Atom *selection, *target, *type;
    XtPointer *value;
    unsigned long *length;
    int *format;
{
    Display* d = XtDisplay(w);
    XSelectionRequestEvent* req =
	XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);

    if (*target == XA_TARGETS(d)) {
	Atom* targetP;
	Atom* std_targets;
	unsigned long std_length;
	XmuConvertStandardSelection(w, req->time, selection, target, type,
				  (caddr_t*)&std_targets, &std_length, format);
	*value = (XtPointer)XtMalloc(sizeof(Atom)*(std_length + 5));
	targetP = *(Atom**)value;
	*targetP++ = XA_STRING;
	*targetP++ = XA_TEXT(d);
	*targetP++ = XA_LENGTH(d);
	*targetP++ = XA_LIST_LENGTH(d);
	*targetP++ = XA_CHARACTER_POSITION(d);
	*length = std_length + (targetP - (*(Atom **) value));
	bcopy((char*)std_targets, (char*)targetP, sizeof(Atom)*std_length);
	XtFree((char*)std_targets);
	*type = XA_ATOM;
	*format = 32;
	return True;
    }

    if (*target == XA_LIST_LENGTH(d) ||
	*target == XA_LENGTH(d))
    {
    	long * temp;
    	
    	temp = (long *) XtMalloc(sizeof(long));
    	if (*target == XA_LIST_LENGTH(d))
      	  *temp = 1L;
    	else			/* *target == XA_LENGTH(d) */
      	  *temp = (long) TextLength (text);
    	
    	*value = (XtPointer) temp;
    	*type = XA_INTEGER;
    	*length = 1L;
    	*format = 32;
    	return True;
    }
    
    if (*target == XA_CHARACTER_POSITION(d))
    {
    	long * temp;
    	
    	temp = (long *) XtMalloc(2 * sizeof(long));
    	temp[0] = (long) 0;
    	temp[1] = TextLength (text);
    	*value = (XtPointer) temp;
    	*type = XA_SPAN(d);
    	*length = 2L;
    	*format = 32;
    	return True;
    }
    
    if (*target == XA_STRING ||
      *target == XA_TEXT(d) ||
      *target == XA_COMPOUND_TEXT(d))
    {
	extern char *_XawTextGetSTRING();

    	if (*target == XA_COMPOUND_TEXT(d))
	    *type = *target;
    	else
	    *type = XA_STRING;
	*length = TextLength (text);
	if (use_motif)
	    *value = (XtPointer)(*_xc_XmTextGetString)(text);
	else
	    *value = (XtPointer)_XawTextGetSTRING((TextWidget) text, 0, *length);
    	*format = 8;
	/*
	 * Drop our connection to the file; the new console program
	 * will open as soon as it receives the selection contents; there
	 * is a small window where console output will not be redirected,
	 * but I see no way of avoiding that without having two programs
	 * attempt to redirect console output at the same time, which seems
	 * worse
	 */
	CloseConsole ();
    	return True;
    }
    
    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
				    (caddr_t *)value, length, format))
	return True;

    return False;
}

static void
LoseSelection (w, selection)
    Widget  w;
    Atom    *selection;
{
    Quit ((Widget)NULL, (XEvent *)NULL, (String *)NULL, (Cardinal *)NULL);
}

/*ARGSUSED*/
static void
InsertSelection (w, client_data, selection, type, value, length, format)
    Widget	    w;
    XtPointer	    client_data;
    Atom	    *selection, *type;
    XtPointer	    value;
    unsigned long   *length;
    int		    *format;
{
    if (*type != XT_CONVERT_FAIL)
	TextInsert (text, (char *) value, *length);
    XtOwnSelection(top, mit_console, CurrentTime,
		   ConvertSelection, LoseSelection, NULL);
    OpenConsole ();
}


/* DismissMessageBox - "dismiss" button callback.
 *			remove console message area from screen
 */
void DismissMessageBox(w, cons_c, reason)
Widget	w;
ConsCatcher	cons_c;
XmAnyCallbackStruct *reason;
{
    Arg al[1];

   /*
    * set value of XmNiconic to false, then true to ensure a change in
    * state so that we really get iconified.
    */
    XtSetArg(al[0], XmNiconic, 0);
    XtSetValues(cons_c->top, al, 1);

    XtSetArg(al[0], XmNiconic, 1);
    XtSetValues(cons_c->top, al, 1);
}

/* ClearAndDismiss - "clear" button callback.
 *		     delete existing text and remove message area from screen
 */
void ClearAndDismiss(w, cons_c, reason)
Widget	w;
ConsCatcher	cons_c;
XmAnyCallbackStruct *reason;
{
    XmTextPosition pos;
    Arg al[1];

    pos = (*_xc_XmTextGetLastPosition)(cons_c->messageTextWidget);
    (*_xc_XmTextReplace)(cons_c->messageTextWidget, 0, pos, "");

   /*
    * set value of XmNiconic to false, then true to ensure a change in
    * state so that we really get iconified.
    */
    XtSetArg(al[0], XmNiconic, 0);
    XtSetValues(cons_c->top, al, 1);

    XtSetArg(al[0], XmNiconic, 1);
    XtSetValues(cons_c->top, al, 1);
}

/* MakeMessageBox - create the message area widgets */
int MakeMessageBox(cons_c)
ConsCatcher cons_c;
{
    Arg al[1];
    int argc;
    static XtCallbackRec cb[2];
    Widget mesgLabel, dismissButton, clearButton;
    int widthmesg, wclearbutton, wdismissbutton;

    static  Arg args_t[] = {
	{ XmNtopWidget, NULL },
	{ XmNtopAttachment, (XtArgVal)XmATTACH_WIDGET },
        { XmNbottomAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNleftAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNrightAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNscrollVertical, True },
	{ XmNeditable, False },
	{ XmNeditMode, XmMULTI_LINE_EDIT },
     };
    static  Arg args_l[] = {
	{ XmNlabelString, NULL },
	{ XmNtopAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNtopOffset, 10 },
	{ XmNleftAttachment, (XtArgVal)XmATTACH_WIDGET },
	{ XmNrightAttachment, (XtArgVal)XmATTACH_WIDGET },
	{ XmNalignment, XmALIGNMENT_CENTER },
	{ XmNleftWidget, NULL},
	{ XmNrightWidget, NULL},
    };
    static  Arg args_b1[] = {
	{ XmNlabelString, NULL },
	{ XmNleftAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNtopAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNactivateCallback, (XtArgVal)cb },
	{ XmNalignment, XmALIGNMENT_CENTER },
    };
    static  Arg args_b2[] = {
	{ XmNlabelString, NULL },
	{ XmNrightAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNtopAttachment, (XtArgVal)XmATTACH_FORM },
	{ XmNactivateCallback, (XtArgVal)cb },
	{ XmNalignment, XmALIGNMENT_CENTER },
    };

    if ((cons_c->message = (*_xc_XmCreateForm)(cons_c->top, "", NULL, 0)) == NULL)
    	return (False);
    XtManageChild(cons_c->message);

    cb[0].closure = (XtPointer)cons_c;

    if (app_resources.buttons)
	{
	cb[0].callback = (XtCallbackProc) ClearAndDismiss;
	args_b1[0].value = (XtArgVal) (*_xc_XmStringLtoRCreate)("Clear", 
								"ISO8859-1");
	if ((clearButton
	     = (*_xc_XmCreatePushButton)(cons_c->message,
				  "",
				  args_b1,
				  XtNumber(args_b1))) == NULL)
	    return(False);
	    XtManageChild(clearButton);

	cb[0].callback = (XtCallbackProc) DismissMessageBox;
	args_b2[0].value = (XtArgVal) (*_xc_XmStringLtoRCreate)("Dismiss", 
							"ISO8859-1");
	if ((dismissButton
	      = (*_xc_XmCreatePushButton)(cons_c->message, "", args_b2, XtNumber(args_b2))) == NULL)
	    return(False);
	XtManageChild(dismissButton);

	args_l[6].value = (XtArgVal)clearButton;
	args_l[7].value = (XtArgVal)dismissButton;
	argc = XtNumber(args_l);
	}
    else
	{
	args_l[3].value = (XtArgVal)XmATTACH_FORM;
	args_l[4].value = (XtArgVal)XmATTACH_FORM;
	args_l[2].value = 0; 	/* topOffset */
	argc = XtNumber(args_l) - 2;	/* ignore leftWidget and rightWidget */
	}
    args_l[0].value = (XtArgVal) (*_xc_XmStringLtoRCreate)("Console Log", "ISO8859-1");
    if ((mesgLabel
	  = (*_xc_XmCreateLabel)(cons_c->message, "", args_l, argc)) == NULL)
	return(False);
    XtManageChild(mesgLabel);

    /*
      The number of rows and columns of this Text Widget are to be specified using
      the .Xdefaults file.  Set them by adding the foll. lines to your .Xdefaults:

       dxconsole*rows		: 10
       dxconsole*columns	: 80

    */

    if (app_resources.buttons)
	args_t[0].value = (XtArgVal)dismissButton;	/* topWidget */
    else
	args_t[0].value = (XtArgVal)mesgLabel;

    if ((cons_c->messageTextWidget
	  = (*_xc_XmCreateScrolledText)(cons_c->message,
				 "",
				 args_t,
				 XtNumber(args_t))) == NULL)
	return(False);
    XtManageChild(cons_c->messageTextWidget);

    if (app_resources.buttons)
	{
	XtSetArg(al[0], XmNdefaultButton, (XtArgVal)dismissButton);
	XtSetValues(cons_c->message, al, 1);
	}

    XtManageChild(cons_c->message);
    return(True);
}


main (argc, argv)
    char    **argv;
{
    Arg arglist[10];
    Cardinal num_args;
    char     *hostname;
    char     *appname;
    Display  *dpy;
    Pixmap   icon_pixmap;
    int	     alt_cons = 0;
    int	     i;
    void     *libXm_handle;

    if (appname = strrchr(argv[0], '/'))
	appname++;
    else
	appname = argv[0];

#ifdef __osf__
    /*
     * If the user is root and the alternate console (1) or generic
     * console (2) is in use, change the default value of the file
     * resource so that no attempt will be made to open the console
     * unless specifically requested.
     */
    if (0 == getuid())	/* root */
	{
	getsysinfo(GSI_WSD_CONS, &alt_cons, sizeof(alt_cons), 0, 0);
	if (alt_cons != 0)
	    resources[3].default_addr = NULL;
	}
#endif

    use_motif = False;

    if (0 == strcmp("dxconsole", appname))
	use_motif = True;
    for (i=1; (i<argc && !use_motif); i++)
	{
	if  (0 == strcmp("-motif", argv[i]))
	    use_motif = True;
	}

    if (use_motif)
	{
	libXm_handle = dlopen("libXm.so", RTLD_NOW);
	if (!libXm_handle)
	    {
	    fprintf(stderr, "%s: Can't load libXm.so\n", argv[0]);
	    exit(1);
	    }
	
	_xc_XmTextGetLastPosition = dlsym(libXm_handle,"XmTextGetLastPosition");
	_xc_XmTextReplace = dlsym(libXm_handle, "XmTextReplace");
	_xc_XmTextGetString = dlsym(libXm_handle, "XmTextGetString");
	_xc_XmCreateForm = dlsym(libXm_handle, "XmCreateForm");
	_xc_XmCreateLabel = dlsym(libXm_handle, "XmCreateLabel");
	_xc_XmCreateScrolledText = dlsym(libXm_handle, "XmCreateScrolledText");
	_xc_XmTextGetInsertionPosition = dlsym(libXm_handle,
						"XmTextGetInsertionPosition");
	_xc_XmTextSetInsertionPosition = dlsym(libXm_handle,
						"XmTextSetInsertionPosition");
	_xc_XmStringLtoRCreate = dlsym(libXm_handle, "XmStringLtoRCreate");
	_xc_XmCreatePushButton = dlsym(libXm_handle, "XmCreatePushButton");

	if (!(_xc_XmTextGetLastPosition || _xc_XmTextReplace ||
	      _xc_XmTextGetString || _xc_XmCreateForm ||
	      _xc_XmCreateLabel || _xc_XmCreateScrolledText ||
	      _xc_XmTextGetInsertionPosition || 
	      _xc_XmTextSetInsertionPosition || _xc_XmStringLtoRCreate ||
	      _xc_XmCreatePushButton ))
	    {
	    fprintf(stderr, "%s: Can't find routines in libXm.so\n", argv[0]);
	    exit(1);
	    }
	}
    if (0 == strcmp("dxconsole", appname))
	{
	/* default for dxsession is Motif == True */
	resources[0].default_addr = (XtPointer)True;

	/* default for dxsession is Stdin == True */
	resources[1].default_addr = (XtPointer)True;

	top = XtInitialize ("dxconsole", "DXConsole", options, 
			    XtNumber (options), &argc, argv);
	}
    else
	{
	top = XtInitialize ("xconsole", "XConsole", options, XtNumber (options),
			    &argc, argv);
	}


    XtGetApplicationResources (top, (XtPointer)&app_resources, resources,
			       XtNumber (resources), NULL, 0);

    use_motif = app_resources.motif;

    if (app_resources.daemon)
	if (fork ()) exit (0);
    
    /*
     * Enforce an arbitrary minimum for the maxSize so that the line
     * trimming algorithm doesn't break.
     */
    if (app_resources.maxSize <= (128 * LINE_CUT_INCR))
	{
	app_resources.maxSize = 128 * (LINE_CUT_INCR + 1);
	}

    dpy = XtDisplay(top);
    icon_pixmap = XCreatePixmapFromBitmapData(XtDisplay(top), 
					XDefaultRootWindow(dpy), icon_bits,
					ICON_SIZE, ICON_SIZE,
					BlackPixel(dpy, DefaultScreen(dpy)),
				       	WhitePixel(dpy, DefaultScreen(dpy)), 1);

    XtSetArg(arglist[0], XtNiconPixmap, icon_pixmap);
    XtSetValues(top, arglist, 1);


    if (use_motif)
	{
	cons_c->top = top;
	MakeMessageBox(cons_c);
	text = cons_c->messageTextWidget;
	}
    else
	{
	text = XtCreateManagedWidget ("text", asciiTextWidgetClass,
				      top, NULL, 0);
 	}   

    XtAddActions (actions, XtNumber (actions));
    
    XtRealizeWidget (top);
    num_args = 0;
    XtSetArg(arglist[num_args], XtNiconic, &iconified); num_args++;
    XtGetValues(top, arglist, num_args);
    if (iconified)
       Iconified ((Widget)NULL, (XEvent *)NULL,(String *)NULL,(Cardinal *)NULL);
    else
       Deiconified((Widget)NULL,(XEvent *)NULL,(String *)NULL,(Cardinal *)NULL);
    wm_delete_window = XInternAtom(XtDisplay(top), "WM_DELETE_WINDOW",
				   False);
    (void) XSetWMProtocols (XtDisplay(top), XtWindow(top),
                            &wm_delete_window, 1);

    XmuGetHostname (mit_console_name + MIT_CONSOLE_LEN, 255);

    mit_console = XInternAtom(XtDisplay(top), mit_console_name, False);

    if (XGetSelectionOwner (XtDisplay (top), mit_console))
    {
	XtGetSelectionValue(top, mit_console, XA_STRING, InsertSelection,
			    NULL, CurrentTime);
    }
    else
    {
	XtOwnSelection(top, mit_console, CurrentTime,
		       ConvertSelection, LoseSelection, NULL);
	OpenConsole ();
    }
    XtMainLoop ();
    return 0;
}

static long TextLength (w)
    Widget  w;
{
    if (use_motif)
	return (*_xc_XmTextGetLastPosition)(cons_c->messageTextWidget);
    else
	return XawTextSourceScan (XawTextGetSource (w),
				  (XawTextPosition) 0,
				  XawstAll, XawsdRight, 1, TRUE);
}

TextReplace (w, start, end, block)
    Widget	    w;
    XawTextBlock    *block;
{
    Arg		    arg;
    Widget	    source;

    if (use_motif)
	{
	Boolean	edit_mode;

	XtSetArg (arg, XmNeditable, &edit_mode);
	XtGetValues (w, &arg, ONE);
	XtSetArg (arg, XmNeditable, True);
	XtSetValues (w, &arg, ONE);
	(*_xc_XmTextReplace) (w, start, end, block->ptr);
	XtSetArg (arg, XmNeditable, edit_mode);
	XtSetValues (w, &arg, ONE);
	}
    else
	{
	XawTextEditType edit_mode;

	source = XawTextGetSource (w);
	XtSetArg (arg, XtNeditType, &edit_mode);
	XtGetValues (source, &arg, ONE);
	XtSetArg (arg, XtNeditType, XawtextEdit);
	XtSetValues (source, &arg, ONE);
	XawTextReplace (w, start, end, block);
	XtSetArg (arg, XtNeditType, edit_mode);
	XtSetValues (source, &arg, ONE);
	}
}

TextAppend (w, s, len)
    Widget  w;
    char    *s;
{
    long	    last, current, new_top;
    Position	    lines_cut;
    XawTextBlock    block;
    int		    i;

    if (use_motif)
	current = (long)(*_xc_XmTextGetInsertionPosition)(w);
    else
	current = XawTextGetInsertionPoint (w);
    block.firstPos = 0;
    block.format = FMT8BIT;
    last = TextLength (w);
    if (last > app_resources.maxSize)
	{
	if (use_motif)
	    {
	    /*
	     * There's no elegant way to count lines in the XmText interface
	     * without either getting our own copy of the widget's text to
	     * search or writing a lot of code to remember where the
	     * new-lines are in the text we have previously written.
	     * I'll just assume that the average line has 64 chars.
	     */
	    new_top = last - app_resources.maxSize + (64 * LINE_CUT_INCR);
	    }
	else
	    {
	    block.ptr = "\n";
	    block.length = 1;
	    new_top = last - app_resources.maxSize;
	    for (i=0; i<LINE_CUT_INCR; i++)
		{
		XawTextSetInsertionPoint (w, new_top+1);
		new_top = XawTextSearch(w, XawsdRight, &block);
	        }
	    }
	block.ptr = NULL;
	block.length = 0;
	TextReplace (w, 0, new_top, &block);
	if (current == last)
	    current = last = TextLength (w);
	else
	    last = TextLength (w);
	}
    block.ptr = s;
    block.length = len;
    TextReplace (w, last, last, &block);
    if (use_motif)
	(*_xc_XmTextSetInsertionPosition) (w, last + block.length);
    else if (current == last)
	XawTextSetInsertionPoint (w, last + block.length);
}

TextInsert (w, s, len)
    Widget  w;
    char    *s;
{
    XawTextBlock    block;
    long	    current;

    if (use_motif)
	current = (long)(*_xc_XmTextGetInsertionPosition)(w);
    else
	current = XawTextGetInsertionPoint (w);
    block.ptr = s;
    block.firstPos = 0;
    block.length = len;
    block.format = FMT8BIT;
    TextReplace (w, 0, 0, &block);
    if (current == 0)
	if (use_motif)
	    (*_xc_XmTextSetInsertionPosition) (w, len);
	else
	    XawTextSetInsertionPoint (w, len);
}

#ifdef USE_PTY
/* This function opens up a pty master and stuffs it's value into pty.
 * If it finds one, it returns a value of 0.  If it does not find one,
 * it returns a value of !0.  This routine is designed to be re-entrant,
 * so that if a pty master is found and later, we find that the slave
 * has problems, we can re-enter this function and get another one.
 */

#include    "../xterm/ptyx.h"

get_pty (pty, tty, ttydev, ptydev)
    int	    *pty, *tty;
    char    *ttydev, *ptydev;
{
#ifdef SVR4
	if ((*pty = open ("/dev/ptmx", O_RDWR)) < 0) {
	    return 1;
	}
	grantpt(*pty);
	unlockpt(*pty);
	strcpy(ttydev, ptsname(*pty));
	if ((*tty = open(ttydev, O_RDWR)) >= 0) {
	    (void)ioctl(*tty, I_PUSH, "ttcompat");
	    return 0;
	}
	if (*pty >= 0)
	    close (*pty);
#else /* !SVR4, need lots of code */
#ifdef USE_GET_PSEUDOTTY
	if ((*pty = getpseudotty (&ttydev, &ptydev)) >= 0 &&
	    (*tty = open (ttydev, O_RDWR)) >= 0)
	    return 0;
	if (*pty >= 0)
	    close (*pty);
#else
	static int devindex, letter = 0;

#if defined(umips) && defined (SYSTYPE_SYSV)
	struct stat fstat_buf;

	*pty = open ("/dev/ptc", O_RDWR);
	if (*pty < 0 || (fstat (*pty, &fstat_buf)) < 0) {
	  return(1);
	}
	sprintf (ttydev, "/dev/ttyq%d", minor(fstat_buf.st_rdev));
	sprintf (ptydev, "/dev/ptyq%d", minor(fstat_buf.st_rdev));
	if ((*tty = open (ttydev, O_RDWR)) >= 0) {
	    /* got one! */
	    return(0);
	}
	close (*pty);
#else /* not (umips && SYSTYPE_SYSV) */
#ifdef CRAY
	for (; devindex < 256; devindex++) {
	    sprintf (ttydev, "/dev/ttyp%03d", devindex);
	    sprintf (ptydev, "/dev/pty/%03d", devindex);

	    if ((*pty = open (ptydev, O_RDWR)) >= 0 &&
		(*tty = open (ttydev, O_RDWR)) >= 0)
	    {
		/* We need to set things up for our next entry
		 * into this function!
		 */
		(void) devindex++;
		return(0);
	    }
	    if (*pty >= 0)
		close (*pty);
	}
#else /* !CRAY */
	strcpy (ttydev, "/dev/ttyxx");
	strcpy (ptydev, "/dev/ptyxx");
	while (PTYCHAR1[letter]) {
	    ttydev [strlen(ttydev) - 2]  = ptydev [strlen(ptydev) - 2] =
		    PTYCHAR1 [letter];

	    while (PTYCHAR2[devindex]) {
		ttydev [strlen(ttydev) - 1] = ptydev [strlen(ptydev) - 1] =
			PTYCHAR2 [devindex];
		if ((*pty = open (ptydev, O_RDWR)) >= 0 &&
		    (*tty = open (ttydev, O_RDWR)) >= 0)
		{
			/* We need to set things up for our next entry
			 * into this function!
			 */
			(void) devindex++;
			return(0);
		}
		if (*pty >= 0)
		    close (*pty);
		devindex++;
	    }
	    devindex = 0;
	    (void) letter++;
	}
#endif /* CRAY else not CRAY */
#endif /* umips && SYSTYPE_SYSV */
#endif /* USE_GET_PSEUDOTTY */
#endif /* SVR4 */
	/* We were unable to allocate a pty master!  Return an error
	 * condition and let our caller terminate cleanly.
	 */
	return(1);
}
#endif

#ifdef USE_OSM
/*
 * On SYSV386 there is a special device, /dev/osm, where system messages
 * are sent.  Problem is that we can't perform a select(2) on this device.
 * So this routine creates a streams-pty where one end reads the device and
 * sends the output to xconsole.
 */
osm_pipe()
{
  int tty, pid;
  char ttydev[64];
    
  if ((tty = open("/dev/ptmx", O_RDWR)) < 0)  return -1;

  grantpt(tty);
  unlockpt(tty);
  strcpy(ttydev, (char *)ptsname(tty));

  if ((pid = fork()) == 0) {
    int pty, osm, buf, nbytes;

    pty = open(ttydev, O_RDWR);
    osm = open("/dev/osm", O_RDWR);
    while ((nbytes = read(osm, &buf, sizeof(buf))) >= 0)
      write(pty, &buf, nbytes);
  }
  return tty;
}
#endif  /* USE_OSM */
