/*
*****************************************************************************
**                                                                          *
**  COPYRIGHT (c) 1988, 1989, 1991, 1992 BY                                 *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.                  *
**  ALL RIGHTS RESERVED.                                                    *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
**                                                                          *
*****************************************************************************
**
** FACILITY:  Session
**
** ABSTRACT:
**
**	Various routines to initialize a session, and handle resource
**	databases and the resource property.
**
** ENVIRONMENT:
**
**      VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette October 1987
**
** Modified by:
**
**	 4-Feb-1992	Edward P Luwish
**		Performance and I18N modifications
**
**	05-Apr-1991	Edward P Luwish
**		Port to Motif UI
**
**	08-Jan-1991	Edward P Luwish
**		Get rid of "key" session manager icon
**
**	25 OCT 1990		KMR
**		Return error staus when we can't open defaults file
**		Initilize names of defaults files
**
*/

/*
** Include files
*/
#include "iprdw.h"

#if defined (VMS)
#include <stdio.h>
#include <iodef.h>
#include <lnmdef.h>
#include <file.h>
#include <psldef.h>
#include <jpidef.h>
#include <ssdef.h>
#include <nam.h>
#else
#include <sys/param.h>
#include <pwd.h>
#endif

#include "smdata.h"
#include "smconstants.h"
#include "smresource.h"
#include "smshare.h"

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Vendor.h>
#include <X11/Xproto.h>
#ifdef vms
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#define	username_size	40

#include "prdw.h"
#include "prdw_entry.h"

#define PrintScreenAppName	"PrintScreen"
#if defined(VMS)
#define PrintScreenClassName	"DECW$PRINTSCREEN"
#else
#define PrintScreenClassName	"DXprint"
#endif

#if defined(VMS)
#ifndef MAXPATHLEN
#define MAXPATHLEN NAM$C_MAXRSS
#endif /* MAXPATHLEN */
#else
#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif /* MAXPATHLEN */
#endif /* VMS */

extern XtAppContext	applicationContext;

#if !defined(VMS)
static String XtGetRootDirName PROTOTYPE((String buf));
#endif

static XrmDatabase GetAppSystemDefaults PROTOTYPE((Display *dpy));

static XrmDatabase GetAppUserDefaults PROTOTYPE((Display *dpy));

static int sm_get_property PROTOTYPE((
    Display	*display,
    int		do_user,
    int		clear_db
));

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
#if !defined(VMS)
static String XtGetRootDirName
#if _PRDW_PROTO_
(
    String	buf
)
#else
(buf)
    String	buf;
#endif
{
     int uid;
     extern char *getenv();
#ifdef R5_XLIB
     extern uid_t getuid();
#else
     extern int getuid();
#endif /* R5_XLIB */
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

char *ResolveFilename
#if _PRDW_PROTO_
(
    Display	*dpy,
    String	class_name,
    Boolean	userfilename,
    Boolean	must_exist
)
#else
(dpy, class_name, userfilename, must_exist)
    Display	*dpy;
    String	class_name;
    Boolean	userfilename;
    Boolean	must_exist;
#endif
{
#if defined(VMS)
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
#else
    char* filename;
    char* path;
    Boolean deallocate = False;
    XrmDatabase rdb;
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
#endif	/* VMS */
}

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
static XrmDatabase GetAppSystemDefaults
#if _PRDW_PROTO_
(
    Display	*dpy
)
#else
(dpy)
    Display	*dpy;
#endif
{
    char	*filename;
    XrmDatabase rdb;

    filename = ResolveFilename (dpy, PrintScreenClassName, False, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return rdb;
}

/* this routine is a slightly modified version of the routine of the same   */
/* name from Xt Initialize.c */
static XrmDatabase GetAppUserDefaults
#if _PRDW_PROTO_
(
    Display	*dpy
)
#else
(dpy)
    Display	*dpy;
#endif
{
    char* filename;
    XrmDatabase rdb;

    filename = ResolveFilename (dpy, PrintScreenClassName, True, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return (rdb);
}

#define EH_BUFSIZ 256

static int ErrorHandler(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
    char bufA[EH_BUFSIZ];
    char msgA[EH_BUFSIZ];
    char numberA[32];
    char *mtype = "XlibMessage";

    /* Ignore soft errors. */

    if (
	/* Attempted to assign input focus to a window, but that window was 
	 * iconified at the time the server processed the request. */

        ((event->error_code == BadMatch) &&
         (event->request_code == X_SetInputFocus)) ||

	/* Attempted to select for ButtonPress on the root window, but
	 * another application already has selected for it. */

        ((event->error_code == BadAccess) &&
         (event->request_code == X_ChangeWindowAttributes))) return (1);

/*  PGLF 8/28/90 I think that this is probably overkill and we should	    */
/*  probably just let the server report things it wants to report */
    XGetErrorText (dpy, event->error_code, bufA, EH_BUFSIZ);
    XGetErrorDatabaseText (dpy, mtype, "XError", "X Error", msgA, EH_BUFSIZ);
    fprintf (stderr, "%s:  %s\n  ", msgA, bufA);
    XGetErrorDatabaseText (dpy, mtype, "MajorCode", "Request Major code %d", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->request_code);
    sprintf (numberA, "%d", event->request_code);
    XGetErrorDatabaseText (dpy, "XRequest", numberA, "", bufA, EH_BUFSIZ);
    fprintf (stderr, " (%s)", bufA);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "MinorCode", "Request Minor code", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->minor_code);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "ResourceID", "ResourceID 0x%x",
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->resourceid);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "ErrorSerial", "Error Serial #%d", 
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, event->serial);
    fputs ("\n  ", stderr);
    XGetErrorDatabaseText (dpy, mtype, "CurrentSerial", "Current Serial #%d",
	msgA, EH_BUFSIZ);
    fprintf (stderr, msgA, XNextRequest(dpy)-1);
    fputs ("\n", stderr);

    return (1);
}

int ws_init
#if _PRDW_PROTO_
(
    int		*argc,
    char	**argv
)
#else
(argc,argv)
    int		*argc;
    char	**argv;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called to initialize a session.  We need to open the display,
**	set up the icons, get the property off of the root window,
**	and set up the session according to the user's customization.
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
*/
{
    Display		*pDisplay;
    unsigned int	status, i;
    XWMHints		wmhints;
    unsigned int	mask;
    XmString		title_cs;
    XmString		icon_cs;

    /* DRM heirarchy file */
#if defined(VMS)
    static char *db_filename_vec[] = {"DECW$PRINTSCREEN" }; 
#else
    static char *db_filename_vec[] = {"DXprint.uid"};
#endif

    static int db_filename_num = XtNumber(db_filename_vec);

    smdata.resource_changed = 0;

#ifdef R5_XLIB
   /*
    * XtSetLanguageProc() should be called before XtAppInitialize()
    */
    XtSetLanguageProc(NULL, NULL, NULL);
#endif /* R5_XLIB */

    /*
    ** Prepare the toolkit.
    */
    MrmInitialize();
    DXmInitialize();
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    XmRepTypeInstallTearOffModelConverter();
#endif

    /*
    ** open the display with the toolkit.  Creates a toplevel shell
    ** which is the base of the applications
    */
    XtToolkitInitialize ();

    applicationContext = XtCreateApplicationContext ();

    display_id = XtOpenDisplay
    (
	applicationContext,
	NULL,
	PrintScreenAppName,
	PrintScreenClassName,
	NULL, 0,
	argc, argv
    );

    if (display_id == 0)
    {
        XtAppErrorMsg
	(
	    applicationContext, "invalidDisplay", "XtOpenDisplay",
	    "XtToolkitError", "Can't Open display", (String *) NULL,
	    (Cardinal *) NULL
	);
    }

    smdata.toplevel = XtAppCreateShell
    (
	PrintScreenAppName,
	PrintScreenClassName,
	applicationShellWidgetClass,
	display_id,
	NULL,
	0
    );

    XSetErrorHandler (ErrorHandler);

/* Open the UID file. Use PerDisplay for Motif 1.2 or later. The other
 * call is now deprecated.
 */
    if (
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
	MrmOpenHierarchyPerDisplay(
		display_id,		/* display                      */
#else
	MrmOpenHierarchy(
#endif
		db_filename_num, 	/* number of UID files          */
		db_filename_vec, 	/* names of the UID files       */
		NULL, 			/* MrmOpenParamPtr =  null      */
		&s_DRMHierarchy) 	/* hierachy return ID           */
	    != MrmSUCCESS)
    {
	printf("Can\'t open Mrm Hierarchy");
        finish();
    }

    /*
    ** Set up the icons
    */
    IconInit (smdata.toplevel);

    title_cs = get_drm_message (k_sm_copytitle_msg);
    DWI18n_SetTitle (smdata.toplevel, title_cs);
    icon_cs = get_drm_message (k_sm_iconname_msg);
    DWI18n_SetIconName (smdata.toplevel, icon_cs);

    /*
    ** Key icon code commented out
    */
    XtVaSetValues
    (
	smdata.toplevel,
#if 0
	XtNiconPixmap, smdata.icon,
	XtNiconifyPixmap, smdata.iconify,
#endif
	XtNallowShellResize, True,
	NULL
    );

    sm_get_property (display_id, True, False);

    return (1);
}

static int sm_get_property
#if _PRDW_PROTO_
(
    Display	*display,
    int		do_user,
    int		clear_db
)
#else
(display, do_user, clear_db)
    Display	*display;
    int		do_user;
    int		clear_db;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Read the printscreen resource file.
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
*/
{
    int			status;
    XrmDatabase		rdb;

    /*
    ** zero out the xrm data
    */
    if (clear_db)
    {
	XrmDestroyDatabase (xrmdb);
	xrmdb = 0;
    }

    /*
    ** Later this should be done as a fallback resource, but that's another
    ** issue.
    */
#ifdef VMS
    rdb = XrmGetStringDatabase
	("PrintScreen.filename: sys$login:decw$capture.tmp");
#else
    rdb = XrmGetStringDatabase
	("PrintScreen.filename: printscreen.ps");
#endif

    if (rdb != NULL)
    {
	XrmMergeDatabases (rdb, &xrmdb);
    }

    if ((rdb = GetAppSystemDefaults(display)) != NULL)
    {
	XrmMergeDatabases (rdb, &xrmdb);
    }

    if (do_user)
    {
	if ((rdb = GetAppUserDefaults(display)) != NULL)
	{
	    XrmMergeDatabases (rdb, &xrmdb);
	}
    }

    return (xrmdb != NULL);
}

int	sm_put_database
#if _PRDW_PROTO_
(
    Display	*dpy
)
#else
(dpy)
    Display	*dpy;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Write out all of the databases to the correct file.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	1 - everything went smoothly
**	0 - there was an error somewhere
**
**  SIDE EFFECTS:
**
**--
*/
{
    char* filename;

    filename = ResolveFilename (dpy, PrintScreenClassName, True, False);
    if (filename == NULL) return (Normal);

    XrmPutFileDatabase (xrmdb, filename);
    XtFree (filename);

    return (Normal);
}

/**********************************************/
int	sm_switch_database
#if _PRDW_PROTO_
(
    Display	*display
)
#else
(display)
    Display	*display;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Read in the current files in sys$login and set up the
**	database correctly.   This is used when we "Use Last Saved"
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
    int	status;

    status = sm_get_property (display, True, True);

    updatesetup();

    return (status);
}

int	sm_save_database
#if _PRDW_PROTO_
(
    Display	*dpy
)
#else
(dpy)
    Display	*dpy;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Calls routines to store the current database in the correct
**	files.
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
*/

{
    int	file_id;
    unsigned    int	ret_status;

    if (smdata.resource_changed)
    {
	ret_status = sm_put_database (dpy);
	if (ret_status != Normal)
	    return(0);
    }

    smdata.resource_changed = 0;
    return(Normal);
}

int	sm_use_managers
#if _PRDW_PROTO_
(
    Display	*display
)
#else
(display)
    Display	*display;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Reads in the system session manager customize files.  Used
**	when use says "Use System defaults".  Also used if we can't
**	find users files.
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
*/
{
    int	status;

    status = sm_get_property (display, False, True);

    updatesetup();

    return (status);
}

XmString get_drm_message
#if _PRDW_PROTO_
(
    int		message
)
#else
(message)
    int		message;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a string from the UIL file string table.
**
**  FORMAL PARAMETERS:
**
**	message - Index into the table of the message we want to retrieve
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**	Returns either NULL or a pointer to the compound string message
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    static XmString	*message_table = NULL;
    static int		nmessages = 0;
    MrmCode		return_type;
    int			status;

    if (message_table == NULL)
    {
	status = MrmFetchLiteral
	(
	    s_DRMHierarchy,			/* hierarchy */
	    "k_sm_message_table",		/* index string */
	    XtDisplay(smdata.toplevel),		/* display */
	    (XtPointer *)&message_table,	/* value */
	    &return_type			/* type */
	);
	if (status != MrmSUCCESS) return (NULL);
    }
    return (message_table[message]);
}
