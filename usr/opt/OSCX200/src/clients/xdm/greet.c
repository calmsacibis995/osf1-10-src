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
 * xdm - display manager daemon
 *
 * $XConsortium: greet.c,v 1.32 93/01/12 15:38:36 gildea Exp $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * widget to get username/password
 *
 */

# include <X11/Intrinsic.h>
# include <X11/StringDefs.h>
# include <X11/Shell.h>

# include <X11/Xaw/Command.h>
# include <X11/Xaw/Logo.h>

# include "Login.h"
# include "dm.h"

extern Display	*dpy;

static int	done, code;
static char	name[128], password[128];
static Widget		toplevel;
static Widget		login;
static XtAppContext	context;
static XtIntervalId	pingTimeout;

/*ARGSUSED*/
static void
GreetPingServer (closure, intervalId)
    XtPointer	    closure;
    XtIntervalId    *intervalId;
{
    struct display *d;

    d = (struct display *) closure;
    if (!PingServer (d, XtDisplay (toplevel)))
	SessionPingFailed (d);
    pingTimeout = XtAppAddTimeOut (context, d->pingInterval * 60 * 1000,
				   GreetPingServer, (closure));
}

/*ARGSUSED*/
GreetDone (w, data, status)
    Widget	w;
    LoginData	*data;
    int		status;
{
    Debug ("GreetDone: %s, (password is %d long)\n",
	    data->name, strlen (data->passwd));
    switch (status) {
    case NOTIFY_OK:
	strcpy (name, data->name);
	strcpy (password, data->passwd);
	bzero (data->passwd, NAME_LEN);
	code = 0;
	done = 1;
	break;
    case NOTIFY_ABORT:
	Debug ("RESERVER_DISPLAY\n");
	code = RESERVER_DISPLAY;
	done = 1;
	break;
    case NOTIFY_RESTART:
	Debug ("REMANAGE_DISPLAY\n");
	code = REMANAGE_DISPLAY;
	done = 1;
	break;
    case NOTIFY_ABORT_DISPLAY:
	Debug ("UNMANAGE_DISPLAY\n");
	code = UNMANAGE_DISPLAY;
	done = 1;
	break;
    }
}

Display *
InitGreet (d)
struct display	*d;
{
    Arg		arglist[10];
    int		i;
    static int	argc;
    Screen		*scrn;
    static char	*argv[] = { "xlogin", 0 };
    Display		*dpy;

    Debug ("greet %s\n", d->name);
    argc = 1;
    XtToolkitInitialize ();
    context = XtCreateApplicationContext();
    dpy = XtOpenDisplay (context, d->name, "xlogin", "Xlogin", 0,0,
			 &argc, argv);

    if (!dpy)
	return 0;

    RegisterCloseOnFork (ConnectionNumber (dpy));

    SecureDisplay (d, dpy);

    i = 0;
    scrn = DefaultScreenOfDisplay(dpy);
    XtSetArg(arglist[i], XtNscreen, scrn);	i++;
    XtSetArg(arglist[i], XtNargc, argc);	i++;
    XtSetArg(arglist[i], XtNargv, argv);	i++;

    toplevel = XtAppCreateShell ((String) NULL, "Xlogin",
		    applicationShellWidgetClass, dpy, arglist, i);

    i = 0;
    XtSetArg (arglist[i], XtNnotifyDone, GreetDone); i++;
    if (!d->authorize || d->authorizations || !d->authComplain)
    {
	XtSetArg (arglist[i], XtNsecureSession, True); i++;
    }
    login = XtCreateManagedWidget ("login", loginWidgetClass, toplevel,
				    arglist, i);
    XtRealizeWidget (toplevel);

    XWarpPointer(dpy, None, RootWindowOfScreen (scrn),
		    0, 0, 0, 0,
		    WidthOfScreen(scrn) / 2,
		    HeightOfScreen(scrn) / 2);

    if (d->pingInterval)
    {
    	pingTimeout = XtAppAddTimeOut (context, d->pingInterval * 60 * 1000,
				       GreetPingServer, (XtPointer) d);
    }
    return dpy;
}

CloseGreet (d)
struct display	*d;
{
    Boolean	    allow;
    Arg	    arglist[1];

    if (pingTimeout)
    {
	XtRemoveTimeOut (pingTimeout);
	pingTimeout = 0;
    }
    UnsecureDisplay (d, XtDisplay (toplevel));
    XtSetArg (arglist[0], XtNallowAccess, (char *) &allow);
    XtGetValues (login, arglist, 1);
    if (allow)
    {
	Debug ("Disabling access control\n");
	XSetAccessControl (XtDisplay (toplevel), DisableAccess);
    }
    XtDestroyWidget (toplevel);
    ClearCloseOnFork (ConnectionNumber (XtDisplay (toplevel)));
    XCloseDisplay (XtDisplay (toplevel));
    Debug ("Greet connection closed\n");
}

Greet (d, greet)
struct display		*d;
struct greet_info	*greet;
{
    XEvent		event;
    Arg		arglist[1];

    XtSetArg (arglist[0], XtNallowAccess, False);
    XtSetValues (login, arglist, 1);

    Debug ("dispatching %s\n", d->name);
    done = 0;
    while (!done) {
	    XtAppNextEvent (context, &event);
	    XtDispatchEvent (&event);
    }
    XFlush (XtDisplay (toplevel));
    Debug ("Done dispatch %s\n", d->name);
    if (code == 0)
    {
	greet->name = name;
	greet->password = password;
	XtSetArg (arglist[0], XtNsessionArgument, (char *) &(greet->string));
	XtGetValues (login, arglist, 1);
	Debug ("sessionArgument: %s\n", greet->string ? greet->string : "<NULL>");
    }
    return code;
}


FailedLogin (d, greet)
struct display	*d;
struct greet_info	*greet;
{
    DrawFail (login);
    bzero (greet->name, strlen(greet->name));
    bzero (greet->password, strlen(greet->password));
}


int GreetUser(
    struct display          *d,
    Display                 ** dpy,
    struct verify_info      *verify,
    struct greet_info       *greet)
{

    *dpy = InitGreet (d);
    /*
     * Run the setup script - note this usually will not work when
     * the server is grabbed, so we don't even bother trying.
     */
    if (!d->grabServer)
	SetupDisplay (d);
    if (!*dpy) {
	LogError ("Cannot reopen display %s for greet window\n", d->name);
	exit (RESERVER_DISPLAY);
    }
    for (;;) {
	/*
	 * Greet user, requesting name/password
	 */
	code = Greet (d, greet);
	if (code != 0)
	{
	    CloseGreet (d);
	    SessionExit (d, code, FALSE);
	}
	/*
	 * Verify user
	 */
	if (Verify (d, greet, verify))
	    break;
	else
	    FailedLogin (d, greet);
    }
    DeleteXloginResources (d, *dpy);
#ifdef SECURE_RPC
    for (i = 0; i < d->authNum; i++)
    {
	if (d->authorizations[i]->name_length == 9 &&
	    bcmp (d->authorizations[i]->name, "SUN-DES-1", 9) == 0)
	{
	    XHostAddress	addr;
	    char		netname[MAXNETNAMELEN+1];
	    char		domainname[MAXNETNAMELEN+1];
    
	    getdomainname(domainname, sizeof domainname);
	    user2netname (netname, verify.uid, domainname);
	    addr.family = FamilyNetname;
	    addr.length = strlen (netname);
	    addr.address = netname;
	    XAddHost (*dpy, &addr);
	    break;
	}
    }
#endif
    CloseGreet (d);
    Debug ("Greet loop finished\n");

    return GREET_SUCCESS;
}

