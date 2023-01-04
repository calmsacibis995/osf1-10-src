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
static char *rcsid = "@(#)$RCSfile: dec_greet.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/20 15:17:27 $";
#endif
/*
 * xdm - display manager daemon
 *
 * $XConsortium: greet.c,v 1.30 92/04/15 10:52:33 rws Exp $
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

#if defined (USE_MOTIF)
# include <Xm/Xm.h>
# include "gpi.h"
#else /* USE_MOTIF */
# include "Login.h"
#endif /* USE_MOTIF */
# include "dm.h"

#if USE_SIA
static Display	*dpy;
#else
extern Display *dpy;
#endif

static int	done, code;
static char	name[128], password[128];
static Widget		toplevel;
static Widget		login;
static XtAppContext	context;
static XtIntervalId	pingTimeout;

#if defined(USE_MOTIF)
static Boolean AllowAccessFlag = False;
static XtActionProc AllowAccess( Widget		widget,
				 XEvent*	event,
				 String*	params,
				 Cardinal*	num_params );
static char session[128];
XtActionProc GpiSetSessionArgument( Widget		widget,
					XEvent*		event,
					String*		params,
					Cardinal*	num_params );
#endif /*USE_MOTIF*/

#ifdef USE_SIA
#include <siad.h>
#include "sia_wind.h"
#include "sia_wind_xdm.h"

static	SIAENTITY *se = NULL;
#endif	/* USE_SIA */

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

#ifdef USE_SIA
Display * 
InitGreet (d)
struct display	*d;
{
    Arg			arglist[10];
    int			i;
    Screen		*scrn;
    static char *login_name = NULL;
    static char *tty_name   = NULL;
    static char *host_name  = NULL;
    static int	argc = 1;
#if defined(USE_MOTIF)
    static char *argv[] = { "dxlogin", 0 };
#else
    static char *argv[] = { "xlogin", 0 };
#endif

    Debug ("greet %s\n", d->name);

	sia_wind_init(argc,argv,d);
	sia_wind_getdpy((Display **)&dpy, (XtAppContext *)&context,
		(Widget *)&toplevel);

#if !defined(STANDALONE_TEST)
    /* 
     * SecureDisplay is moved into sia_wind_init to avoid xdm killing
     * itself.
     * SecureDisplay (d, dpy);
     */
#endif /* STANDALONE_TEST */

#if defined(USE_LOGO)
    ShowLogo( toplevel );
#endif /*USE_LOGO*/

    if (d->pingInterval)
    {
	pingTimeout = XtAppAddTimeOut (context, d->pingInterval * 60 * 1000,
				       GreetPingServer, (XtPointer) d);
    }
    tty_name = d->name;
    Debug ("InitGreet: tty_name passed to sia_ses_init: %s \n", tty_name);
    if(sia_ses_init(&se,argc,argv,host_name,login_name,tty_name,1,NULL) != SIASUCCESS)
        return NULL;
    return dpy;
}

#define ACCESS_MSG	"You have allowed unlimited X client access\nto your workstation.  Did you intend this?" 

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
#if 0
    UnsecureDisplay (d, XtDisplay (toplevel));
#else
    UnsecureDisplay (d, dpy);
#endif

	/* we're done with the SIA info now, free it and let SIA zero the space */
	if(se != NULL)
		sia_ses_release(&se);

/* >>> removed questioning of whether unlimited access is okay by the  <<< */
/* >>> user.  It seems that SIA should be taking care of this <<< */
	sia_wind_term();
}

Greet (
	struct display		*d,
	struct greet_info	*greet)
{
	struct passwd *p;
	char *login_name = NULL;
	char *tty_name   = NULL;
	char *pswd_name  = NULL;
	char *host_name  = NULL;
	int status = -1;

#if defined(USE_MOTIF)
	AllowAccessFlag = False;
#endif
	while(status != SIASUCCESS)
	    {
	    while(status != SIASUCCESS)
		{
		Debug("Greet: Before sia_ses_authent\n");
		status = sia_ses_authent(sia_wind_collector_greet,NULL,se);
		Debug("Greet: After sia_ses_authent status = %d \n", status);
		if (status != SIASUCCESS)
		    FailedLogin( d, (struct greet_info *)NULL, "Invalid login");
		}
	    Debug("Greet: Before sia_ses_estab\n");
	    status = sia_ses_estab(sia_wind_collector_greet,se);
	    Debug("Greet: After sia_ses_estab status = %d \n", status);
	    if (status == SIASUCCESS)
		status = sia_ses_launch(sia_wind_collector_greet,se);
	    if (status != SIASUCCESS)
		{
		static int  argc = 1;
		static char *argv[] = { "dxlogin", 0 };

		if(se != NULL)
		    sia_ses_release(&se);
		tty_name = d->name;
		if(sia_ses_init(&se,argc,argv,host_name,login_name,tty_name,
						    1,NULL) != SIASUCCESS)
		    return -1;
		}
	    /* launch will most likely seteuid to user. We can't
	     * have that here. But we leave launch here until we find
	     * out just what it may do.
	     */
	    seteuid(0);
	    }
/* make a copy of the entity pwd struct and copy other entity members */
	greet->pwd= (struct passwd *) malloc(sizeof (struct passwd));
	if(greet->pwd == NULL)
		{
		Debug( "RESERVER_DISPLAY\n" );
                sia_ses_release(&se);
                exit( RESERVER_DISPLAY );
                }
	bzero(greet->pwd, sizeof(struct passwd));
        if(se->pwd->pw_name != NULL)
                {
                greet->pwd->pw_name= (char *) malloc(strlen(se->pwd->pw_name) + 1);
                if(greet->pwd->pw_name == NULL)
			{
			Debug( "RESERVER_DISPLAY\n" );
			sia_ses_release(&se);
            		exit( RESERVER_DISPLAY );
        		}
                strcpy(greet->pwd->pw_name,se->pwd->pw_name);
		greet->name = greet->pwd->pw_name;
                }
	else	{
		Debug( "RESERVER_DISPLAY\n" );
                sia_ses_release(&se);
                exit( RESERVER_DISPLAY );
                }
	if(se->pwd->pw_comment != NULL)
		{
		greet->pwd->pw_comment= (char *) malloc(strlen(se->pwd->pw_comment) + 1);
		if(greet->pwd->pw_comment == NULL)
			{
			Debug( "RESERVER_DISPLAY\n" );
                        sia_ses_release(&se);
                        exit( RESERVER_DISPLAY );
                        }
		strcpy(greet->pwd->pw_comment,se->pwd->pw_comment);
		}
	if(se->pwd->pw_gecos != NULL)
                {
                greet->pwd->pw_gecos= (char *) malloc(strlen(se->pwd->pw_gecos) + 1);
                if(greet->pwd->pw_gecos == NULL)
			{
			Debug( "RESERVER_DISPLAY\n" );
                        sia_ses_release(&se);
                        exit( RESERVER_DISPLAY );
                        }
                strcpy(greet->pwd->pw_gecos,se->pwd->pw_gecos);
                }
        if(se->pwd->pw_dir != NULL)
                {
                greet->pwd->pw_dir= (char *) malloc(strlen(se->pwd->pw_dir) + 1);
                if(greet->pwd->pw_dir == NULL)
			{
			Debug( "RESERVER_DISPLAY\n" );
                        sia_ses_release(&se);
                        exit( RESERVER_DISPLAY );
                        }
                strcpy(greet->pwd->pw_dir,se->pwd->pw_dir);
                }
         if(se->pwd->pw_shell != NULL)
                {
                greet->pwd->pw_shell= (char *) malloc(strlen(se->pwd->pw_shell) + 1);
                if(greet->pwd->pw_shell == NULL)
			{
			Debug( "RESERVER_DISPLAY\n" );
                        sia_ses_release(&se);
                        exit( RESERVER_DISPLAY );
                        }
                strcpy(greet->pwd->pw_shell,se->pwd->pw_shell);
                }
        greet->pwd->pw_uid=se->pwd->pw_uid;
        greet->pwd->pw_gid=se->pwd->pw_gid;
        greet->pwd->pw_quota=se->pwd->pw_quota;
	bzero( name, 128 );
	bzero( password, 128 );
    	greet->password = password;
#if defined(USE_MOTIF)
	/* bzero( session, 128 ); */
    	greet->string = session;
#endif /* USE_MOTIF */
	sia_ses_release(&se);

	return 0;
}



#else	/* USE_SIA */

#if !defined(USE_MOTIF)
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
#endif /* USE_MOTIF */

Display *
InitGreet (d)
struct display	*d;
{
    Arg		arglist[10];
    int		i;
    static int	argc;
    Screen		*scrn;
#if defined(USE_MOTIF)
    static char *argv[] = { "dxlogin", 0 };
#else
    static char *argv[] = { "xlogin", 0 };
#endif
    Display		*dpy;

    Debug ("greet %s\n", d->name);
    argc = 1;
    XtToolkitInitialize ();
    context = XtCreateApplicationContext();
#if defined(USE_MOTIF)
    dpy = XtOpenDisplay (context, d->name, "dxlogin", "DXlogin", 0,0,
			 &argc, argv);
#else
    dpy = XtOpenDisplay (context, d->name, "xlogin", "Xlogin", 0,0,
			 &argc, argv);
#endif

    if (!dpy)
	return 0;

    RegisterCloseOnFork (ConnectionNumber (dpy));

#if !defined(STANDALONE_TEST)
    SecureDisplay (d, dpy);
#endif /* STANDALONE_TEST */

    i = 0;
    scrn = DefaultScreenOfDisplay(dpy);
    XtSetArg(arglist[i], XtNscreen, scrn);	i++;
    XtSetArg(arglist[i], XtNargc, argc);	i++;
    XtSetArg(arglist[i], XtNargv, argv);	i++;
#if defined(USE_MOTIF)
    XtSetArg(arglist[i], XmNx, XWidthOfScreen( scrn ) / 2 );  i++;
    XtSetArg(arglist[i], XmNy, XHeightOfScreen( scrn ) / 2 ); i++;
    XtSetArg(arglist[i], XmNwidth, 1 ); 			i++;
    XtSetArg(arglist[i], XmNheight, 1 );                      i++;
    XtSetArg(arglist[i], XmNmappedWhenManaged, False );	i++;
#endif /*USE_MOTIF*/

#if defined(USE_MOTIF)
    toplevel = XtAppCreateShell ((String) NULL, "DXlogin",
		    applicationShellWidgetClass, dpy, arglist, i);
#else
    toplevel = XtAppCreateShell ((String) NULL, "Xlogin",
		    applicationShellWidgetClass, dpy, arglist, i);
#endif


#if defined(USE_MOTIF)
    if ( GpiAbort == GpiEstablishShell( toplevel ) ) 
    {
		Debug( "Failed to establish a GPI top level shell.\n" );
    }
    i = 0;
    XtSetArg( arglist[i], "allow-all-access",	  AllowAccess	     ); i++;
    XtSetArg( arglist[i], "set-session-argument", SetSessionArgument ); i++;
    XtAppAddActions( context, arglist, i );
#else
    i = 0;
    XtSetArg (arglist[i], XtNnotifyDone, GreetDone); i++;
    if (!d->authorize || d->authorizations || !d->authComplain)
    {
	XtSetArg (arglist[i], XtNsecureSession, True); i++;
    }
    login = XtCreateManagedWidget ("login", loginWidgetClass, toplevel,
				    arglist, i);
#endif /*USE_MOTIF*/
    XtRealizeWidget (toplevel);

#if defined(USE_LOGO)
    ShowLogo( toplevel );
#endif /*USE_LOGO*/

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
#if defined(USE_MOTIF)
    allow = AllowAccessFlag;
    if ( allow &&
	 GpiQueryUserYesNo( toplevel, 20*1000, False,
"You have allowed unlimited X client access\nto your workstation.  Did you intend this?" )
	     == GpiContinue ) 
#else
    XtSetArg (arglist[0], XtNallowAccess, (char *) &allow);
    XtGetValues (login, arglist, 1);
    if (allow)
#endif /*USE_MOTIF*/
    {
	Debug ("Disabling access control\n");
#if !defined(STANDALONE_TEST)
	XSetAccessControl (XtDisplay (toplevel), DisableAccess);
#endif /* STANDALONE_TEST */
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
#if defined(USE_MOTIF)
	AllowAccessFlag = False;
	bzero( name, 128 );
	bzero( password, 128 );
	bzero( session, 128 );
	Debug( "dispatching %s\n", d->name );
	if ( GpiLoginPrompt( NULL, NULL, name, 128, password, 128 ) ==
	     GpiAbort )
	  {
	    Debug( "RESERVER_DISPLAY\n" );
	    exit( RESERVER_DISPLAY );
	  }
	else
	  {
	    greet->name = name;
	    greet->password = password;
	    greet->string = session;
	  }
	Debug( "Done dispatch %s\n", d->name );
	Debug( "sessionArgument: %s\n", *session ? session : "<NULL>" );
#else
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
#endif /*USE_MOTIF*/
}

#endif	/* USE_SIA */

/*ARGSUSED*/
FailedLogin (d, greet, str)
struct display		*d;
struct greet_info	*greet;
char * 			str;
{
#if defined(USE_MOTIF) || defined(USE_SIA)
	if (GpiLoginFail( NULL, 0, str ))
	    /* 
	     * return status indicates abort vs.  invalid login	
	     * Exit with this status to reset the display
	     */
	    exit( OBEYSESS_DISPLAY ); 
#else
    DrawFail (login, str);
#endif /*USE_MOTIF*/
}

#ifndef USE_SIA
#if defined(USE_MOTIF)
static XtActionProc AllowAccess(
	Widget		widget,
	XEvent*		event,
	String*		params,
	Cardinal*	num_params )
{
    AllowAccessFlag = ! AllowAccessFlag;
}
#endif /*USE_MOTIF*/
#endif /* USE_SIA */

XtActionProc GpiSetSessionArgument(
	Widget		widget,
	XEvent*		event,
	String*		params,
	Cardinal*	num_params )
{
    if ( *num_params > 0 )
		strcpy( session, params[0] );
    else
		*session = '\0';
    Debug( "Set session argument to: %s\n", session );
}




int GreetUser(
	struct display		*d,
	Display			** dpy,
	struct verify_info	*verify,
	struct greet_info	*greet)
{
    int			verified = VERIFY_FAILED;

    *dpy = InitGreet (d);
    /*
     * Run the setup script - note this usually will not work when
     * the server is grabbed, so we don't even bother trying.
     * Moved into InitGreet() so the font path will be set right for
     * the login widget.
     *
     * if (!d->grabServer)
     *	SetupDisplay (d);
     */
    if (!*dpy) {
	LogError ("Cannot reopen display %s for greet window\n", d->name);
	exit (RESERVER_DISPLAY);
    }
    while ( verified != VERIFY_SUCCESSFUL ) {
	/*
	 * Greet user, requesting name/password
	 */
	code = Greet (d, greet);
#ifdef USE_SIA
	if ( code != 0 ) {
	    Debug("out of greet\n");
	    verified = VERIFY_FAILED;
	    Debug("failed login\n");
	    Debug("failed login\n");
	    sia_wind_term();
            *dpy = InitGreet (d);
	    continue;
	}
#else 
	if (code != 0)
	{
	    CloseGreet (d);
	    SessionExit (d, code, FALSE);
	}
#endif
	/*
	 * Verify user
	 */
#ifdef ORIG
	if (Verify (d, greet, verify))
	    break;
	else
	    FailedLogin (d, &greet);
#else /* ORIG */
#if USE_SIA
	switch (verified = Verify(d, greet, verify, greet->pwd))
#else
	switch (verified = Verify(d, greet, verify))
#endif
	{
	  case VERIFY_FAILED: 
	      FailedLogin (d, &greet, (char *)NULL);
	      break;
	      
	  case VERIFY_CAPACITY_LIMIT_EXCEEDED:
#if defined(USE_MOTIF)
	      FailedLogin( d, (struct greet_info *)NULL,
		"Too many users logged on already.\nPlease try again later." );
#else
	      FailedLogin( d, (struct greet_info *)NULL,
			   "Too many users logged on already." );
#endif /*USE_MOTIF*/
	      break;

	  case VERIFY_SUCCESSFUL:
	  default:
	      break;
	} /* switch */
	
#endif /* ORIG */
    } /* for */
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
	    XAddHost (dpy, &addr);
	    break;
	}
    }
#endif
    CloseGreet (d);
    Debug ("Greet loop finished\n");
#if defined(STANDALONE_TEST)
    exit(0);
#endif /* STANDALONE_TEST */
    return GREET_SUCCESS;
}
