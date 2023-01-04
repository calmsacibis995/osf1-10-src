#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: [notepad.c,v 1.5 91/08/17 05:59:29 rmurphy Exp ]$";  /* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1987 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**      Notepad -- DECwindows simple out-of-the-box editor. 
**
**  AUTHOR:
**      Joel Gringorten  - November, 1987
**
**  ABSTRACT:
**      This module contains the user interface and mainline.  
**
**  ENVIRONMENT:
**      User mode, executable image.
**
**  MODIFICATION HISTORY:
**  1987/11/07   J. M. Gringorten  V1-001  JMG0001       
**      Initial version.
**
**  1993/05/25   Peter_Wolfe 	   Fix OSF_QAR 11339.
** 	Crash when cancel save dialog
**
**  1993/07/16   Chas Hunt 	   Fix internal ootb_bug note# 83.
** 	Crashes at menubar item "Options >> Restore Options".
**      -  Deleted version control history above.
**      -  In get_xrmdb(): blocked NULL-xrmdb from "further destruction"!
**      -  In DoSaveSettings(): called get_xrmdb() to provide a "source"
**         database for XrmPutStringResource() and XrmPutFileDatabase().
**      -  In ConditionalReadSettings(): upgraded XrmMergeDatabases() to
**         X11R5's XrmCombineDatabase() at one point.
**
**--
**/

#if defined(VMS)
#include <nam.h>
#else
#include <sys/param.h>
#include <pwd.h>
#endif

#if defined (VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#if defined (VMS) && !defined(__DECC)
#pragma standard
#endif

#define NOTEPAD_MAIN
#include "notepad.h"

MrmType *dummy_class; 
static int zero = 0;
static int def_x = 100;
static int def_y = 100;
notepadStuff Stuff;
static XrmDatabase   xrmdb;  /* CAREFULL: XrmCombineDatabase() &
                              *           XrmMergeDatabases()
                              *           destroy the "source" database
                              *           ("target" database preserved).
                              */
#ifdef VMS
#define CLASS_NAME "DECW$NOTEPAD"
#else
#define CLASS_NAME "DXnotepad"
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

static char	uid_filespec[] = {CLASS_NAME};

/* HyperHelp definitions by default it's on */
#if !defined(HYPERHELP)
#define HYPERHELP 1
#endif

#if HYPERHELP
    static  Opaque	help_context=0;
    void		HyperHelpError();
#endif

static MrmOsOpenParamPtr os_ext_list_ptr = NULL; 

static XtResource resources[] =
{
    {"editInPlace", "EditInPlace", XtRBoolean, sizeof(Boolean),
	XtOffset(notepadStuff *,_editInPlace), XtRString,
	(XtPointer)"False"},
    {"enableBackups", "EnableBackups", XtRBoolean, sizeof(Boolean),
	XtOffset(notepadStuff *,_enableBackups), XtRString,
	(XtPointer)"FALSE"},
    {"backupNamePrefix", "BackupNamePrefix", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_backupNamePrefix), XtRString,
	(XtPointer)""},
    {"backupNameSuffix", "BackupNameSuffix", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_backupNameSuffix), XtRString,
	(XtPointer)".BAK"},
    {"journalNamePrefix", "JournalNamePrefix", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_journalNamePrefix), XtRString,
	(XtPointer)""},
    {"journalNameSuffix", "JournalNameSuffix", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_journalNameSuffix), XtRString,
	(XtPointer)"JNL"},
#ifndef VMS
    {"fileFilter", "FileFilter", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_fileFilter), XtRString,
	(XtPointer)"*"},
#else
    {"fileFilter", "FileFilter", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_fileFilter), XtRString,
	(XtPointer)"*.*"},
#endif
    {"fontFilter", "FontFilter", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_fontFilter), XtRString,
	(XtPointer)"-*"},
    {"overrideLocking", "OverrideLocking", XtRBoolean, sizeof(Boolean),
	XtOffset(notepadStuff *,_overrideLocking), XtRString,
	(XtPointer)"FALSE"},
    {"expert", "Expert", XtRBoolean, sizeof(Boolean),
	XtOffset(notepadStuff *,_expert), XtRString,
	(XtPointer)"FALSE"},
    {"readOnly", "ReadOnly", XtRBoolean, sizeof(Boolean),
	XtOffset(notepadStuff *,_read_only), XtRString,
	(XtPointer)"FALSE"},
    {XtNwidth, XtCWidth, XtRInt, sizeof(int),
	XtOffset(notepadStuff *,_Width), XtRInt,
	(XtPointer)&zero},
    {XtNheight, XtCHeight, XtRInt, sizeof(int),
	XtOffset(notepadStuff *,_Height), XtRInt,
	(XtPointer)&zero},
    {"changesUntilCompress", "ChangesUntilCompress", XtRInt, sizeof(int),
	XtOffset(notepadStuff *,_changesUntilCompress), XtRInt,
	(XtPointer)&zero},
    {"filter", "Filter", XtRString, sizeof(char *),
	XtOffset(notepadStuff *,_filter), XtRString,
	(XtPointer)""},
    {"geometry", "Geometry", XtRString, sizeof(char *),
	 XtOffset(notepadStuff *,_geometry), XtRString,
	 (XtPointer)"=500x700+100+100"}
};


#if !defined(VMS)
static String  XtGetRootDirName(buf)
    String  buf;
/*
 *  Function:
 *      A slightly modified version of the routine of the same
 *      name from Xt Initialize.c
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
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


#if defined(VMS)
char *ResolveFilename (dpy, class_name, userfilename, must_exist)
    Display *dpy;
    String   class_name;
    Boolean  userfilename;
    Boolean  must_exist;    /* ignored on VMS */
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
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
    String   class_name;
    Boolean  userfilename;
    Boolean  must_exist;
/*
 * Function: Same as above
 *
 */
{
    char         *filename;
    char         *path;
    Boolean       deallocate = False;
    XrmDatabase   rdb;
    extern char  *getenv();
    char          homedir[MAXPATHLEN];

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


static XrmDatabase GetAppSystemDefaults(dpy)
    Display *dpy;
/*
 *  Function:
 *      A slightly modified version of the routine of the same
 *      name from Xt Initialize.c
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    char        *filename;
    XrmDatabase  rdb;

    filename = ResolveFilename (dpy, CLASS_NAME, False, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return rdb;
}


static XrmDatabase GetAppUserDefaults(dpy)
    Display *dpy;
/*
 *  Function:
 *      A slightly modified version of the routine of the same
 *      name from Xt Initialize.c
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    char        *filename;
    XrmDatabase  rdb;

    filename = ResolveFilename (dpy, CLASS_NAME, True, True);
    if (filename == NULL) return ((XrmDatabase) 0);

    rdb = XrmGetFileDatabase(filename);
    XtFree(filename);

    return (rdb);
}


static int get_xrmdb (display, do_user, clear_db)
    Display  *display;
    int       do_user;
    int       clear_db;
/*
 *  Function:
 *	Create and load a SYSTEM-defaults resource database (from on-disk).
 *      Merge this into our static current resource database, which is NULL
 *      if "clear_db" is requested. Finally, create and load a USER-
 *      specific resource database (from on-disk) and merge this into our
 *      static current resource database, if "do_user" is requested,
 *
 *  Inputs:
 * 	Display  - ptr to display struct
 * 	do_user  - flag, if true, then merge user specific resource file.
 * 	clear_db - flag, if true, destroy the existing database prior to
 *                 the system defaults being merged into it.
 *
 *  Outputs:
 * 	Returns True if xrmdb is valid, False IF the database is cleared,
 *      AND a USER-specific merge is not requested, AND the SYSTEM-defaults
 *      create/load is unsuccessfull. 
 * 	Note that xrmdb is a static-global pointing to the current database. 
 *
 *  Notes:
 *   1) The static XrmDatabase "xrmdb" that exists after this call, is
 *      usually immediately destroyed via a XrmMerge/XrmCombine call.  Thus,
 *      the sole benifit of it being static is its "static name".
 *   2) 
 *
 */
{
    XrmDatabase  rdb = NULL;

    /*
    ** zero out the xrm data IF IT EXISTS
    */
    if (clear_db && (xrmdb != NULL))
    {
        XrmDestroyDatabase (xrmdb);
        xrmdb = NULL;
    }

    if ((rdb = GetAppSystemDefaults(display)) != NULL)
    {
        XrmMergeDatabases(rdb, &xrmdb);  /* rdb destroyed */
        rdb = NULL;
    }

    if (do_user)
    {
        if ((rdb = GetAppUserDefaults(display)) != NULL)
        {
            XrmMergeDatabases(rdb, &xrmdb);  /* rdb destroyed */
            rdb = NULL;
        }
    }

    return (xrmdb != NULL);
}


Widget searchToggleWidget;
Widget wordWrapToggleWidget;

QueueElement *remqueue(head, tail, this)
    QueueElement **head, **tail, *this;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    QueueElement *prev = this->blink;
    QueueElement *next = this->flink;
	if (this == *head){
	  if(this == *tail) {
	    *head = 0;
	    *tail = 0;
	  } else {
	    *head = next;
	    next->blink = 0;
	  }
	} else {
	  if(this == *tail){
	    *tail = prev;
	    prev->flink = 0;   
	  } else {
	    prev->flink = next;
	    next->blink = prev;
	  }
	}
	this->blink = 0;
	this->flink = 0;
	return (this);
}


QueueElement *insert(head, tail, new, where)
    QueueElement **head, **tail, *new, *where;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
	if(!where){
	    new->flink = *head;
	    *head = new;
	} else {
	    new->flink = where->flink;
	    new->blink = where;
	    where->flink = new;
	}
	if(new->flink == 0)
	    *tail = new;
	else
	    new->flink->blink = new;
	return(new);
}


void setaValue (w, which, what)
    Widget   w;
    char    *which, *what;
/*
 *  Function: Set one Widget parameter
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Arg al[2];

    XtSetArg (al[0], which, what);
    XtSetValues (w, al, 1);
}


assertToggleState (w, tag, any)
    Widget                w;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    state[*tag] = TRUE;
    switch (*tag)
    {
    case 3:
	searchToggleWidget = w;
	break;
    case 4:
	wordWrapToggleWidget = w;
	break;
    }
}


void WordWrapToggle (w, tag, any)
    Widget                w;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 * 	One a XmScrolledText widget is created, the horizontal 
 * 	scroll bar cannot be turned off. If wordwrap is on, the horizontal
 * 	scroll bar must be disabled and vice versa. Changing the
 * 	word wrap settind requires that the user to a Save Options, 
 * 	exit, and restart Notepad. We need a message dialog box here
 * 	to tell the user that. 
 */
{
    View  *vp;    
    state[st_WordWrap] = 1 ^ state[st_WordWrap];

    for(vp = Stuff.viewHead; vp; vp = vp->flink) /* For each window */
    {
	XtVaSetValues				/* Set wordwrap */
	(
	    vp->widget,
	    XmNwordWrap, state[st_WordWrap],
	    XmNscrollHorizontal, !state[st_WordWrap],
	    NULL
	);
	XtVaSetValues 				/* And the horiz. scroll bar */
	(
	    XtParent(vp->widget),
	    XmNscrollHorizontal, !state[st_WordWrap],
	    NULL
	);
    }
}


#ifdef VMS
char *SetupFileName(filename)
    char *filename;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    sprintf(filename, "DECW$USER_DEFAULTS:DECW$NOTEPAD.DAT");
    return filename;
}
#else
char *SetupFileName(filename)
    char *filename;
/*
 *  Function: Same as above.
 *
 */
{
    sprintf(filename,"%s/.notepad.dat", getenv("HOME"));
    return filename;
}
#endif


DoSaveSettings()
/*
 *  Function:
 *      Read in the file-based resource database (ie, gather desired
 *      sittings from SYSTEM- and USER-specific disk-files). Augment
 *      this database with the current value of all user-menu-accessable
 *      items and restore to disk-file.
 *      
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Boolean   wrap;
    Boolean   scase;
    char     *filename;
    int       status;

    filename = ResolveFilename
	(XtDisplay(Stuff.viewHead->widget), CLASS_NAME, True, False);
    if (filename == NULL)
        return;
    else if (xrmdb == NULL)
        /* This static resource db may have been destroyed elsewhere
         * in a call to XrmCombineDatabase() or XrmMergeDatabases().
         */
        status = get_xrmdb            /* Get file-based resource db */
            (XtDisplay(Stuff.viewHead->widget), True, True);

    wrap  = state[st_WordWrap];
    scase = state[st_CaseSensitive];

    XrmPutStringResource
	(&xrmdb, "notepad*textwindow.wordWrap", wrap ? "on" : "off");
    XrmPutStringResource
	(&xrmdb, "notepad*textwindow.scrollHorizontal", !wrap ? "on" : "off");
    XrmPutStringResource
	(&xrmdb, "notepad*textwindow.caseSearch", scase ? "on" : "off");
    if (fontName != NULL)
    {
	XrmPutStringResource (&xrmdb, "notepad*textwindow.fontList", fontName);
    }

    XrmPutFileDatabase(xrmdb, filename);
    XtFree(filename);

    return;
}


ConditionalReadSettings(merge)
    Boolean merge;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int                 status;
    XrmRepresentation   rep;
    XrmValue            value;
    int                 type;
    Display            *display;
    View               *vp;
    XrmDatabase         db;
    typedef struct _settings_struct
    {
        Boolean     wrap;
        Boolean     scase;
        XmFontList  text_font;
        String      text_font_name;
    } SettingsRec, *Settings;

    SettingsRec	settings;

    static XtResource   resourceTable[] =
    {
	{XmNfontList, XmCFontList, XmRFontList, sizeof (XmFontList),
	    XtOffset (Settings, text_font), XtRString, DXmDefaultFont},
	{XmNfontList, XmCFontList, XtRString, sizeof (String),
	    XtOffset (Settings, text_font_name), XtRString, DXmDefaultFont},
	{"caseSearch", "CaseSearch", XtRBoolean, sizeof (Boolean),
	    XtOffset (Settings, scase), XtRImmediate, (caddr_t)True},
	{XmNwordWrap, XmCWordWrap, XtRBoolean, sizeof (Boolean),
	    XtOffset (Settings, wrap), XtRImmediate, (caddr_t)False}
    };

    vp      = Stuff.viewHead;
    display = XtDisplay(vp->widget);

    if (merge)
    {
        /* Gather contents of both SYSTEM- and USER-specific
         * resource-files into a resource database (used for merging into
         * the display's current resource database).
         * NOTE:
         *   It would be nice to staticlly buffer this resource db, just
         *   modifying occasionally, BUT(!) this resource database is
         *   consumed (destroyed) with each call to XrmCombineDatabase()
         *   or XrmMergeDatabases() below. Thus, it is necessary to "reget"
         *   this resource db EITHER HERE OR LATER (in the
         *   save-settings-to-file routine). ("LATER" is chosen -- perhaps
         *   it's more readable, anyway, it does permit the call here rather
         *   than above this block where it could be called needlessly.)
         *   
         */
        status = get_xrmdb (display, True, True);

        /* Now combine this "source" into the "target" database */
#ifdef R5_XLIB
        db = XrmGetDatabase(display);       /* the "target" */
        XrmCombineDatabase(xrmdb,&db,True); /* xrmdb destroyed; "True" overwrite */
        xrmdb = NULL;
        XrmSetDatabase(display, db);
#else
        XrmMergeDatabases(xrmdb, &(display->db));  /* xrmdb destroyed */
        xrmdb = NULL;
#endif
    }

    XtGetSubresources
    (
	vp->widget,
	&settings,
	NULL, NULL,
	resourceTable,
	XtNumber(resourceTable),
	NULL,
	0
    );

    /*
    ** Use old mechanism for setting the fontList. Allows us to get
    */
    for (vp = Stuff.viewHead; vp != NULL; vp = vp->flink)
    {
	XtVaSetValues
	(
	    vp->widget,
	    XmNfontList, settings.text_font,
	    XmNwordWrap, settings.wrap,
	    XmNscrollHorizontal, !settings.wrap,
	    NULL
	);
    }

    /*
    ** Apply these to the UI that modifies them.
    */
    state[st_WordWrap] = settings.wrap;
    state[st_CaseSensitive] = settings.scase;
    XmToggleButtonSetState(wordWrapToggleWidget, settings.wrap, False);
    XmToggleButtonSetState(searchToggleWidget, settings.scase, False);

    fontName = setFontName = settings.text_font_name;

    DoApplyFont();

    return;
}


DoReadSettings()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    ConditionalReadSettings(True);
}


simpleDialog *tagToDialog (tag)
     int  *tag;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd;
    switch (*tag)
    {
    case 1:
	sd = &saveDialog;
	break;
    case 2:
	sd = &undoDialog;
	break;
    case 3:
	sd = &seaDialog;
	break;
    case 4:
	sd = &seaNextIncrDialog;
	break;
    case 5:
	sd = &LineDialog;
	break;
    case 6:
	sd = &replaceDialog;
	break;
    case 7:
	sd = &filterDialog;
	break;
    case 8:
	sd = &searchOptionsDialog;
	break;
    case 9:
	sd = &messageDialog;
	break;
    case 10:
	sd = &openDialog;
	break;
    case 11:
	sd = &fontDialog;
	break;
    };
    return sd;
}


void rememberFocus (w, tag, any)
    Widget                w;
    View                 *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
   Stuff.focusView = tag;
}


Widget NotepadTextCreate(parent, name, args, num_args)
    Widget   parent;
    char    *name;
    Arg     *args;
Cardinal  num_args;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    return XmCreateScrolledText (parent, name, args, num_args);
}


NotepadInitializeForDRM ()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int  stat;

    stat = MrmRegisterClass
    (
	MrmwcUnknown,
	"textwindow",
	"NotepadTextCreate",
	NotepadTextCreate,
	xmTextWidgetClass
    );
    if (stat != MrmSUCCESS)
    {
	fprintf (stderr, "notepad widget registration failed\n");
	return stat;
    };
}


CreatePaneCallback(widget)
    Widget widget;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    workAreaPane = widget;
}


static View *makeView (w, where, wi, h, pos)
    Widget  w;
    View   *where;
    int     wi,h, pos;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    View            *view         = (View *)XtCalloc(sizeof(View), 1);
    XtTranslations   textoverride = NULL;
    static int       Tnum;
    char             name[16];
    Arg              a[20];
    int              n = 0;
    XtCallbackRec    cb[2];

    cb[0].callback = (XtCallbackProc) rememberFocus;
    cb[0].closure  = (XtPointer) view;
    cb[1].callback = (XtCallbackProc) 0;
    XtSetArg(a[n], XmNeditable, TRUE);        n++;
    XtSetArg(a[n], XmNresizable, TRUE);       n++;
    XtSetArg(a[n], XmNwidth, wi);             n++;
    XtSetArg(a[n], XmNheight, h );            n++;
    XtSetArg(a[n], XmNresizeWidth, 0 );       n++;
    XtSetArg(a[n], XmNresizeHeight, 0);       n++;
    XtSetArg(a[n], XmNfocusCallback, cb);     n++;
    XtSetArg(a[n], XmNscrollVertical, TRUE);  n++;
    XtSetArg(a[n], XmNcursorPosition, 0);     n++;
    if(globalFontStruct){
	 XtSetArg(a[n], XmNfontList, 
	    XmFontListCreate(globalFontStruct, "ISO8859-1")); n++;
    }
    MrmFetchWidgetOverride( DRM_hierarchy, "textwindow", workAreaPane,
	NULL, a, n, &view->widget, &dummy_class);	
    insert(&Stuff.viewHead, &Stuff.viewTail, view, where);
    Stuff.viewCount++;

/*
    popup =  XmCreatePopupMenu(view->widget, sprintf(name, "popup_%d", Tnum),
		0, 0, , XmVERTICAL, NULL, NULL, NULL)
    {
	Arg	al[13];
	int	ac = 0;

	XtSetArg(al[ac], XmNx, sprintf); ac++;
	XtSetArg(al[ac], XmNy, name); ac++;
	XtSetArg(al[ac], XmNrowColumnType, "popup_); ac++;
	XtSetArg(al[ac], XmNorientation, d"); ac++;
	XtSetArg(al[ac], XmNentryCallback, Tnum); ac++;
	XtSetArg(al[ac], XmNmapCallback, 0); ac++;
	XtSetArg(al[ac], XmNhelpCallback, 0); ac++;
	XmCreateRowColumn(view,
			widget,
			al, ac);
    };
    textoverride = XtParseTranslationTable("<Btn3Down>: do-popup(popup)");
    XtOverrideTranslations(view->widget, textoverride);
*/
    return(view);
} 


static Boolean UnsetLimits()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    View *vp;
    Arg   minMax[2];

    XtSetArg(minMax[0], XmNpaneMinimum, 1);
    XtSetArg(minMax[1], XmNpaneMaximum, 5000);
    for(vp = Stuff.viewHead; vp; vp = vp->flink)
    {
	XtSetValues(XtParent(vp->widget), minMax, 2);
    }
    setaValue(toplevel, XmNallowShellResize, True);
    return True;
}


DoSplit()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Arg             arg[2];
    int             i;
    Dimension       width, height, viewHeight;
    Arg             minArg[3];
    int             n;
    View           *view, *vp;
    XmTextPosition  pos = XmTextGetInsertionPosition(Stuff.focusView->widget);
    XmTextPosition  savedPos;
    Boolean         foundFocus;
    Boolean         firstAfterFocus;

    setaValue(toplevel, XmNallowShellResize, False);
    setaValue(workAreaPane, XmNrefigureMode, False);
    width = GetWidth(Stuff.focusView->widget);
    height = GetHeight(Stuff.focusView->widget);
    height = height/2;
    i = 0;
    /*
     * Scan the list of widgets in the pane, looking for the
     * focus widget. Save the height and insertion position.
     */
    foundFocus = False;
    firstAfterFocus = False;
    for(vp = Stuff.viewHead; vp; vp = vp->flink)
    {
	i++;
	vp->height = GetHeight(vp->widget);
	if(vp == Stuff.focusView) {
	    n = i;
	    vp->height = height;
	    vp->cursorPos = XmTextGetInsertionPosition(vp->widget);
	    savedPos = vp->cursorPos;
	    foundFocus = True;
	    firstAfterFocus = True;
	    XtSetArg(minArg[0], XmNpaneMinimum, height);
	    XtSetArg(minArg[1], XmNpaneMaximum, height);
	    XtSetArg(minArg[2], XmNheight, height);
	    XtSetValues(XtParent(vp->widget), minArg, 3);
        }
        else
        {
	    if (foundFocus)
            {
		vp->height = height;
		vp->cursorPos = savedPos;
		savedPos = XmTextGetInsertionPosition(vp->widget);
		foundFocus = False;
		if (firstAfterFocus)
                {
		    XtSetArg(minArg[0], XmNpaneMinimum, height);
		    XtSetArg(minArg[1], XmNpaneMaximum, height);
		    XtSetArg(minArg[2], XmNheight, height);
		    XtSetValues(XtParent(vp->widget), minArg, 3);
		    firstAfterFocus = False;
		}
	    }
	}
    }
    view = makeView (workAreaPane, Stuff.focusView, width, height, n+1);
    view->cursorPos = savedPos;
    setaValue(textwindow, XmNwordWrap, state[st_WordWrap]); 
    PSTextSetSource(view->widget, Psource, savedPos, savedPos);
    XtManageChild(XtParent(view->widget));
    XtManageChild(view->widget);
    XmTextSetInsertionPosition(view->widget, savedPos);
    XmTextSetCursorPosition(view->widget, savedPos);
    /*
     * Force the sizes of the text areas
     */
    for(vp = Stuff.viewHead; vp; vp = vp->flink){
	if (vp->widget != view->widget) {
	    XtSetArg(minArg[0], XmNpaneMinimum, vp->height);
	    XtSetArg(minArg[1], XmNpaneMaximum, vp->height);
	    XtSetValues(XtParent(vp->widget), minArg, 2);
	    XmTextSetInsertionPosition(vp->widget, vp->cursorPos);
	    XmTextSetCursorPosition(vp->widget, vp->cursorPos);
	}
    }
    /*
     * Register a work procedure to remove the limits
     */
    XtAddWorkProc((XtWorkProc)UnsetLimits, 0);
    setaValue(workAreaPane, XmNrefigureMode, True);
}


DoUnSplit() 
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
  Arg        arg;
  Arg        minArg;
  View      *next, *old, *vp;
  Dimension  height;
  int        thisH;

    if(Stuff.viewCount < 2){
	Feep();
	return;
    }
    if(Stuff.focusView->blink)
	next = Stuff.focusView->blink;
    else
	next = Stuff.focusView->flink;

    setaValue(workAreaPane, XmNrefigureMode, True);
    setaValue(toplevel, XmNallowShellResize, False);
    for(vp = Stuff.viewHead; vp; vp = vp->flink){
	XtSetArg(arg, XtNheight, &height);
        XtGetValues(XtParent(vp->widget), &arg, 1);
	XtSetArg(minArg, XmNpaneMinimum, height);
	XtSetValues(XtParent(vp->widget), &minArg, 1);
    }
    XtSetArg(minArg, XmNpaneMinimum, 1);
    XtSetValues(XtParent(next->widget), &minArg, 1);
    old = (View *)remqueue(&Stuff.viewHead, &Stuff.viewTail, Stuff.focusView);
    XmProcessTraversal(next->widget, XmTRAVERSE_CURRENT);
    XtUnmanageChild(old->widget);
    XtDestroyWidget(old->widget);
    Stuff.viewCount --;
    setaValue(workAreaPane, XmNrefigureMode, True);
    XtAddWorkProc((XtWorkProc)UnsetLimits, 0);
}


void doSaveWithFile (w, tag, any)
    Widget                w;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *      Handle the OK button on the SaveAs... dialog
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    char          *filename;
    simpleDialog  *sd = &saveDialog;

    filename = XmTextGetString(sd->stringWidget);
    if(strlen(filename))
    {
        DoSave(filename);
        XSetInputFocus
	(
	    XtDisplay(toplevel), XtWindow(textwindow),
	    RevertToParent, XtLastTimestampProcessed(XtDisplay(toplevel))
	);
	XtUnmanageChild(sd->popupWidget);
    }
    else
    {
	Feep();
    }
}


static void do_Exit (w, tag, any)
    Widget                w;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    void do_Save();
   
#if HYPERHELP
    /*  Release the Hyperhelp environment if it's been
    **  allocated.
    */
    if(help_context)
    {
	DXmHelpSystemClose(help_context, HyperHelpError, "Help System Error");
	help_context = 0;		    /* sanity check */
    }
#endif /* HYPERHELP */

    exitWithSave = TRUE;
    do_Save();
}


static void quitCallback (w, tag, any)
    Widget                 w;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    void do_Save();

#if HYPERHELP
    /*  Release the Hyperhelp environment if it's been
    **  allocated.
    */
    if(help_context)
    {
	DXmHelpSystemClose (help_context, HyperHelpError, "Help System Error");
	help_context = 0;				/* sanity check */
     }
#endif /* HYPERHELP */

    switch(any->reason)
    {
    case XmCR_OK:
	exitWithSave = TRUE;
	do_Save();
	break;
    case XmCR_CANCEL:
	XtUnmapWidget(toplevel);
	XFlush(XtDisplay(toplevel));
	closeInputFile();
	exit(NormalStatus);
    }
}


static void do_Quit (w, tag, any)
    Widget                 w;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Arg a[8];
    int n = 0;
    if(modified)
    {
	if(!Stuff._QuitWarnBox)
	{
	    MrmFetchWidgetOverride (DRM_hierarchy, "quitWarn",
		 mainWin, NULL, a,  n, &Stuff._QuitWarnBox, &dummy_class);	
        }
        XtManageChild(Stuff._QuitWarnBox);
    }
    else
    {
	/*  Release the Hyperhelp environment if it's been
	**  allocated.
	*/
#if HYPERHELP
        if(help_context)
        {
             DXmHelpSystemClose
		(help_context, HyperHelpError, "Help System Error");
             help_context = 0;				/* sanity check */
	}
#endif /* HYPERHELP */
	XtUnmapWidget(toplevel);
	XFlush(XtDisplay(toplevel));
	closeInputFile();
        exit (NormalStatus);
    }
}


void cancelDialog(w, tag, any)
    Widget                w;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *      Generic Cancel button callback to bail out of a dialog
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    XtUnmanageChild ((Widget)tag);
    XSetInputFocus
    (
        XtDisplay(toplevel), XtWindow(textwindow),
        RevertToParent, XtLastTimestampProcessed(XtDisplay(toplevel))
    );
}


void callClosureWithSelectedString
        (w, closure, selection, type, value, length, format)
    Widget           w;
    XtPointer        closure;
    Atom            *selection;
    Atom            *type;
    XtPointer       *value;
    unsigned long   *length;
    int             *format;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    char *buf = XtMalloc((*length)+1);
    memcpy (buf, value, *length);
    buf[*length] = 0;
    (*(void(*)())closure) (buf);
    XtFree (buf);
    XtFree ((char *)value);
}


void _doUndo (w, tag, any)
    Widget  w;
    int     tag;
    int     any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
   if(tag == 1)
	DoUndo();
    else 
	DoRedo();
}


static void fileSelectCallback (widget, tag, fsData)
    Widget                             widget;
    caddr_t                            tag;
    XmFileSelectionBoxCallbackStruct  *fsData;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    switch (fsData->reason)
    {
    case XmCR_OK:
	checkLoad(decodeCS(fsData->value));
	/* break;  fall thru */
    case XmCR_CANCEL:
	XtUnmanageChild(widget);
	break;
    case XmCR_HELP:
	;
    }
}


static void checkLoadCallback (w, tag, any)
    Widget                w;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    void do_Save();
    switch(any->reason)
    {
    case XmCR_OK:
	do_Save();
	/* fall thru */
    case XmCR_CANCEL:
	DoLoad((char *)tag);
    }
}


checkLoad(filename)
    char *filename;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    XtCallbackRec   cb[2];
    int             len = strlen(filename);
    int             n   = 0;
    Arg             a[8];
    char           *fn;
    XmTextPosition  left, right;

    if (len > 255)
    {
	Feep();
	return;
    }
    fn = malloc(len+1);
    strcpy(fn, filename);
    if(fn[len-1] == '\n') fn[len-1] = 0;
    cb[0].callback = (XtCallbackProc) checkLoadCallback;
    cb[0].closure  = (XtPointer)fn;
    cb[1].callback = (XtCallbackProc) NULL;

    if (modified)
    {
	left = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdLeft, 1, FALSE);
	right = (*Psource->Scan)(Psource, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);

	if ((right-left == len) || (right-left == len +1)) DoLoad(fn);
	else
	{
	    if(Stuff._LoadWarnBox) XtDestroyWidget(Stuff._LoadWarnBox);
	    XtSetArg(a[n], XmNcancelCallback, cb);  n++;
	    XtSetArg(a[n], XmNokCallback, cb);      n++;
	    MrmFetchWidgetOverride
	    (
		DRM_hierarchy, "loadWarn",
		mainWin, NULL,
		a, n,
		&Stuff._LoadWarnBox, &dummy_class
	    );
	    XtManageChild(Stuff._LoadWarnBox);
	}
    }
    else
    {
	DoLoad(fn);
    }
}


static void doOpenSelected (widget, tag, any)
    Widget                 widget;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{ 
    XtGetSelectionValue
    (
	toplevel,
	XA_PRIMARY,
	XA_STRING, 
	(XtSelectionCallbackProc)callClosureWithSelectedString,
	checkLoad,
	XtLastTimestampProcessed(XtDisplay(toplevel))
    );
}


static void do_Save (widget, tag, any)
    Widget                 widget;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    DoSave("");
}


static void locateDialogInWorkArea (w)
    Widget w;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Position  waX, waY, bX, bY;
    int       waWidth, boxWidth;
    Arg       arg, arg2[2];

    arg.name  = XmNwidth;
    arg.value = (XtArgVal) &boxWidth;

    arg2[0].name  = XtNx;
    arg2[0].value = (XtArgVal) &waX;
    arg2[1].name  = XtNy;
    arg2[1].value = (XtArgVal) &waY;
    XtRealizeWidget(w);

    XtTranslateCoords(textwindow, 0,0, &waX, &waY);
    XtGetValues(w, &arg, 1);
    arg.value = (XtArgVal) &waWidth;
    XtGetValues(workAreaPane, &arg, 1);
    waX += (waWidth - boxWidth)/2;
/* here comes a disgusting hack to get around a window manager bug that
    should be fixed in next realease (don't move window 0 pixels) */

    XtGetValues (XtParent(w), arg2, 2);
    if(abs(arg2[0].value - waX) > 3 && abs(arg2[1].value - 
			waY) > 3){
	arg2[0].value = waX;
	arg2[1].value = waY;
        XtSetValues(w, arg2, 2);
    }
}


setFocusAndSelect (w)
    Widget  w;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    XmTextPosition  left, right;
    XmTextSource    Source    = XmTextGetSource(w);
    Time            timestamp = XtLastTimestampProcessed(XtDisplay(w));

    XtCallAcceptFocus (w, &timestamp);

#if 0
    left  = (*Source->Scan)(Source, 0, XmSELECT_ALL, XmsdLeft,  1, FALSE);
    right = (*Source->Scan)(Source, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    (*Source->SetSelection) (Source, left, right, timestamp);
#else
    XmTextSetSelection
    (
        w,
        0,
        XmTextGetLastPosition(w),
        timestamp
    );
#endif
}


void showSimpleDialog (widget, tag, any)
    Widget                widget;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = (simpleDialog *)tag;

    if(!sd->popupWidget)
	(*sd->createProc)();

    locateDialogInWorkArea(sd->popupWidget);

    if(XtIsManaged(sd->popupWidget))
	XtUnmanageChild(sd->popupWidget);

    XtManageChild(sd->popupWidget);
}


static void includeCallback (widget, tag, fsData)
    Widget                             widget;
    caddr_t                            tag;
    XmFileSelectionBoxCallbackStruct  *fsData;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    switch( fsData->reason)
    {
    case XmCR_OK:
	DoInclude(decodeCS(fsData->value));
	/* break;  fall thru */
    case XmCR_CANCEL:
	XtUnmanageChild(widget);
	break;
    case XmCR_HELP:
	break;
    }
}


doFileOp (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    XtCallbackRec    fsCB[2];
    XtCallbackRec    helpCB[2];
    static XmString  openHelp    = (XmString) NULL;
    static XmString  includeHelp = (XmString) NULL;
    static void      MenuHelpHelp();
    char             trnName[80];

    if (openHelp == (XmString) NULL)
    {
	openHelp    = XmStringCreateSimple("Overview Menus File Open_und");
	includeHelp = XmStringCreateSimple("Overview Menus File include");
    }
    fsCB[0].callback = (XtCallbackProc) fileSelectCallback;
    fsCB[0].closure  = (XtPointer) NULL;
    fsCB[1].callback = (XtCallbackProc) NULL;
    if(!openDialog.dialogWidget) (*openDialog.createProc)();
    switch (*tag)
    {
    case 1:
	fsCB[0].callback = (XtCallbackProc) fileSelectCallback;
	setaValue(openDialog.popupWidget, XmNokCallback,fsCB);
	setaValue(openDialog.popupWidget, XmNcancelCallback,fsCB);
	helpCB[0].callback = (XtCallbackProc) MenuHelpHelp;
	helpCB[0].closure = (XtPointer) openHelp;
	helpCB[1].callback = (XtCallbackProc) NULL;
	helpCB[1].closure = (XtPointer) NULL;
	setaValue(openDialog.popupWidget, XmNhelpCallback,helpCB);
	break;
    case 2:
	fsCB[0].callback = (XtCallbackProc) includeCallback;
	setaValue(openDialog.popupWidget, XmNokCallback,fsCB);
	setaValue(openDialog.popupWidget, XmNcancelCallback,fsCB);
	helpCB[0].callback = (XtCallbackProc) MenuHelpHelp;
	helpCB[0].closure = (XtPointer) includeHelp;
	helpCB[1].callback = (XtCallbackProc) NULL;
	helpCB[1].closure = (XtPointer) NULL;
	setaValue(openDialog.popupWidget, XmNhelpCallback,helpCB);
	break;
    }

    if(XtIsManaged(openDialog.popupWidget))
	XtUnmanageChild(openDialog.popupWidget);
    XtManageChild(openDialog.popupWidget);
}


void addSearchActions()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    XtActionsRec actRec[2];
    actRec[0].string = "DoSearchNext";
    actRec[1].string = "DoSearchPrevious";
    actRec[0].proc = DoSearchNextAndFinish;
    actRec[1].proc = DoSearchPrevAndFinish; 
    XtAddActions(actRec, 2);
}


showSearchDialog (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    char trnName[80];

    if(!seaDialog.popupWidget)
	(*seaDialog.createProc)();

    XtUnmanageChild(seaDialog.popupWidget);

    switch (*tag)
    {
    case 1:
	setaValue
	(
	    XtParent(seaDialog.popupWidget),
	    XmNselectionLabelString,
	    NextString
	);
	setaValue
	(
	    seaDialog.popupWidget,
	    XmNdefaultButton,
	    seaDialog.buttons[0]
	);
	break;
    case 2:
	setaValue
	(
	    XtParent(seaDialog.popupWidget),
	    XmNselectionLabelString,
	    PrevString
	);
	setaValue
	(
	    seaDialog.popupWidget,
	    XmNdefaultButton,
	    seaDialog.buttons[1]
	);
	break;
    }

    locateDialogInWorkArea(seaDialog.popupWidget);
    XtManageChild(seaDialog.popupWidget);
}


static void showReplaceDialog(widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(!replaceDialog.popupWidget)
	(*replaceDialog.createProc)();

    XtUnmanageChild(replaceDialog.popupWidget);

    switch (*tag)
    {
    case 1:
	setaValue
	    (XtParent(replaceDialog.popupWidget), XtNtitle, OnceString);
	XtUnmanageChild(replaceDialog.buttons[3]);/* SelectedButton */
	XtUnmanageChild(replaceDialog.buttons[4]);/* AllButton */
	XtManageChild(replaceDialog.buttons[1]);/* SkipButton */
	XtManageChild(replaceDialog.buttons[0]);/* OnceButton */
	setaValue
	(
	    replaceDialog.popupWidget,
	    XmNdefaultButton,
	    replaceDialog.buttons[0]
	);
	break;
    case 2:
	setaValue
	(
	    XtParent(replaceDialog.popupWidget),
	    XtNtitle,
	    SelectedString
	);
	XtUnmanageChild(replaceDialog.buttons[0]);/* OnceButton */
	XtUnmanageChild(replaceDialog.buttons[4]);/* AllButton */
	XtUnmanageChild(replaceDialog.buttons[1]);/* SkipButton */
	XtManageChild(replaceDialog.buttons[3]);   /*  SelectedButton */	
	setaValue
	(
	    replaceDialog.popupWidget,
	    XmNdefaultButton,
	    replaceDialog.buttons[3]
	);
	break;
    case 3:
	setaValue
	    (XtParent(replaceDialog.popupWidget), XtNtitle, AllString);
	XtUnmanageChild(replaceDialog.buttons[3]);/* SelectedButton */
	XtUnmanageChild(replaceDialog.buttons[0]);/* OnceButton */
	XtUnmanageChild(replaceDialog.buttons[1]);/* SkipButton */
	setaValue
	(
	    replaceDialog.popupWidget,
	    XmNdefaultButton,
	    replaceDialog.buttons[4]
	);
	XtManageChild(replaceDialog.buttons[4]);/* AllButton */
	break;
    }

    locateDialogInWorkArea(replaceDialog.popupWidget);
    XtManageChild(replaceDialog.popupWidget);
}


void hideSimpleDialog(widget, tag, any)
    Widget                 widget;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = (simpleDialog *)tag;
    XSetInputFocus
    (
        XtDisplay(textwindow), XtWindow(textwindow),
        RevertToParent, XtLastTimestampProcessed(XtDisplay(textwindow))
    );
    XtUnmanageChild(sd->popupWidget); 
}


void hideSimpleDialog1 (widget, tag, any)
    Widget                 widget;
    caddr_t                tag;
    XmAnyCallbackStruct   *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = (simpleDialog *)tag;
    XtUnmanageChild(sd->popupWidget);
}


void DoHideSimpleDialog (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);
    hideSimpleDialog (widget, sd, any);
}


void registerButtons(widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);

    sd->buttons[sd->pb_count] = widget;
    sd->pb_count++;
    adjustButtons(sd->buttons, sd->pb_count);
}


void registerPopups(widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);
    sd->popupWidget = widget;
}


void SearchStringModified();
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */


void registerQuestions(widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Widget text;
    simpleDialog *sd = tagToDialog(tag);

    sd->popupWidget = widget;

    sd->buttons[sd->pb_count] = XmSelectionBoxGetChild
	(widget, XmDIALOG_OK_BUTTON);
    sd->pb_count++;
    sd->buttons[sd->pb_count] = XmSelectionBoxGetChild
	(widget, XmDIALOG_CANCEL_BUTTON);
    sd->pb_count++;
    sd->buttons[sd->pb_count] = XmSelectionBoxGetChild
	(widget, XmDIALOG_HELP_BUTTON);
    sd->pb_count++;
    text = XmSelectionBoxGetChild (widget, XmDIALOG_TEXT);
    if(sd->stringWidget == NULL)
	sd->stringWidget = text;
    else
	sd->stringWidget_1 = text;

    if (*tag == 4)
    {
	/* incremental search */
	XtAddCallback
	(
	    text,
	    XmNvalueChangedCallback,
	    (XtCallbackProc)SearchStringModified,
	    (XtPointer)NULL
	);
    }
}


void registerComposites (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);
    sd->popupWidget = widget;

    sd->buttons[sd->pb_count] = XmMessageBoxGetChild
	(widget, XmDIALOG_OK_BUTTON);
    sd->pb_count++;
    sd->buttons[sd->pb_count] = XmMessageBoxGetChild
	(widget, XmDIALOG_CANCEL_BUTTON);
    sd->pb_count++;
    sd->buttons[sd->pb_count] = XmMessageBoxGetChild
	(widget, XmDIALOG_HELP_BUTTON);
    sd->pb_count++;
}


void registerSTexts (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);

    if(sd->stringWidget == NULL)
	sd->stringWidget = widget;
    else
	sd->stringWidget_1 = widget;
}


void registerMenuBar(widget, tag, any)
    Widget                widget;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    menuBar = widget;
}


void CacheFontFamilyList(widget, tag, any)
    Widget                widget;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Stuff.FontFamilyList = widget;
}


void CacheFontMiscList (widget, tag, any)
    Widget                widget;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Stuff.FontMiscList = widget;
}


void CacheFontSizeList (widget, tag, any)
    Widget                widget;
    caddr_t               tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Stuff.FontSizeList = widget;
}


void do_Revert()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    checkLoad(loadedfile);
}


#ifdef unix
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>


static void handler(sig, code, scp)
    int                 sig, code;
    struct sigcontext  *scp;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int status, pid;
    pid = wait3(&status, WNOHANG, 0);
}


do_New()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    extern char  **environ;
    int            pid, argc;
    Arg            arg[6];
    int            i;
    Dimension      x, y, width, height;

    i = 0;
    XtSetArg(arg[i], XmNx, &x); i++;
    XtSetArg(arg[i], XmNy, &y); i++;
    XtSetArg(arg[i], XmNwidth, &width); i++;
    XtSetArg(arg[i], XmNheight, &height); i++;

   XtGetValues(toplevel, arg, i);
    sprintf(geometryString, "dxnotepad =%dx%d+%d+%d",width,height,
                              x+50, y+50);	
    if(!(pid = vfork())){
	if(!vfork()){
	    execlp("sh", "sh", "-c",  geometryString, NULL);
	   _exit(errno);
	} else {
	    _exit(0);
	}
    }
/*    wait */
}
#endif /* unix */


#ifdef VMS
do_New()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int                      flag = 1;
    struct dsc$descriptor_s  comDesc;
    Arg                      arg[6];
    int                      i;
    Dimension                x, y, width, height;

    i = 0;
    XtSetArg(arg[i], XmNx, &x); i++;
    XtSetArg(arg[i], XmNy, &y); i++;
    XtSetArg(arg[i], XmNwidth, &width); i++;
    XtSetArg(arg[i], XmNheight, &height); i++;

    XtGetValues(toplevel, arg, i);
    sprintf(geometryString, "mc decw$notepad -g %dx%d+%d+%d", 
                              width, height, x+50, y+50);
    comDesc.dsc$w_length = strlen(geometryString );
    comDesc.dsc$a_pointer = geometryString;
    comDesc.dsc$b_class = DSC$K_CLASS_S;
    comDesc.dsc$b_dtype = DSC$K_DTYPE_T;
    lib$spawn(&comDesc,0,0,&flag);
}
#endif /* VMS */


void makeFileSelectionDialog(w, sd)
    Widget        w;
    simpleDialog *sd;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
  int n = 0;
  Arg a[10];
    XtSetArg(a[n], XmNdirMask, XmStringLtoRCreate(fileFilter , "ISO8859-1")); n++;
    MrmFetchWidgetOverride( DRM_hierarchy, "chooseFile", mainWin,
	NULL, a,  n, &sd->popupWidget, &dummy_class);	
}


DoSimpleDialog(widget, tag, any)
    Widget               widget;
    int                 *tag;
    XmAnyCallbackStruct *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    simpleDialog *sd = tagToDialog(tag);
    showSimpleDialog (widget, sd, any);
}


setupSearchOptionsDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(searchOptionsDialog.popupWidget)
	return TRUE;
    makeSearchOptionsDialog(workAreaPane, &searchOptionsDialog); 
    return TRUE;
}


setupLineDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(LineDialog.popupWidget)
	return TRUE;
    mkLineDialog(workAreaPane, &LineDialog); 
    return TRUE;
}


setupSearchDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(seaDialog.popupWidget)
	return TRUE;
    mkSimpleFindDialog(workAreaPane,  &seaDialog); 
    return TRUE;
}


setupIncrDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(seaNextIncrDialog.popupWidget)
	return TRUE;
	mkIncrDialog(workAreaPane, &seaNextIncrDialog);
    return TRUE;
}


setupReplaceDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(replaceDialog.popupWidget)
	return TRUE;
    mkReplaceDialog(workAreaPane, &replaceDialog);
    return TRUE;
}


setupUndoDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(undoDialog.popupWidget)
	return TRUE;
    makeUndoDialog(workAreaPane, &undoDialog); 
    return TRUE;
}


setupOpenDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(openDialog.popupWidget)
	return TRUE;
    makeFileSelectionDialog(workAreaPane, &openDialog); 
    return TRUE;
}


setupSaveDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(saveDialog.popupWidget)
	return TRUE;
    mkSaveDialog(workAreaPane, &saveDialog);
    return TRUE;
}


setupFontDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(fontDialog.popupWidget)
	return TRUE;
    makeFontDialog(workAreaPane, &fontDialog);
    return TRUE;
}


#ifdef unix 
setupFilterDialog()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    if(filterDialog.popupWidget)
	return TRUE;
    mkFilterDialog(workAreaPane, &filterDialog);
    return TRUE;
}
#endif /* unix */


makeDialogs()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    LineDialog.createProc = setupLineDialog;
    XtAddWorkProc((XtWorkProc)setupLineDialog, 0);
    seaDialog.createProc = setupSearchDialog;
    XtAddWorkProc((XtWorkProc)setupSearchDialog, 0);
    seaNextIncrDialog.createProc = setupIncrDialog;
    XtAddWorkProc((XtWorkProc)setupIncrDialog, 0);
    replaceDialog.createProc = setupReplaceDialog;
    XtAddWorkProc((XtWorkProc)setupReplaceDialog, 0);
    undoDialog.createProc = setupUndoDialog;
    XtAddWorkProc((XtWorkProc)setupUndoDialog, 0);
    openDialog.createProc = setupOpenDialog;
    XtAddWorkProc((XtWorkProc)setupOpenDialog, 0);
    saveDialog.createProc = setupSaveDialog;
    XtAddWorkProc((XtWorkProc)setupSaveDialog, 0);
    searchOptionsDialog.createProc = setupSearchOptionsDialog;
    XtAddWorkProc((XtWorkProc)setupSearchOptionsDialog, 0);
    fontDialog.createProc = setupFontDialog;
    XtAddWorkProc((XtWorkProc)setupFontDialog, 0);
#ifdef unix 
    filterDialog.createProc = setupFilterDialog;
    XtAddWorkProc((XtWorkProc)setupFilterDialog, 0);
#endif /* unix */
}


DoSelectAll()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
#if 0
    XmTextPosition  left, right;
    XmTextSource    Source = XmTextGetSource(textwindow);
    left  = (*Source->Scan)(Source, 0, XmSELECT_ALL, XmsdLeft,  1, FALSE);
    right = (*Source->Scan)(Source, 0, XmSELECT_ALL, XmsdRight, 1, FALSE);
    (*Source->SetSelection)
        (Source, left, right, XtLastTimestampProcessed(XtDisplay(textwindow)));
#else
    XmTextSetSelection
    (
        textwindow,
        0,
        XmTextGetLastPosition(textwindow),
        XtLastTimestampProcessed(XtDisplay(textwindow))
    );
#endif
}    


DoClear ()
/*
 *  Function:
 * 	Clear the currently selected text
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
  XmTextBlockRec block;
  XmTextPosition left, right;
    block.length = 0;
    block.ptr = NULL;
    block.format = FMT8BIT;
    if((*Psource->GetSelection)(Psource, &left, &right))
    {
	if ((*Psource->Replace)(textwindow, NULL, &left, &right, &block, True)
			!= EditDone)
	{
	    Feep();
	}
    }

} /* end routine DoClear */ 


DoNextIncr()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    setaValue
	(XtParent(seaNextIncrDialog.popupWidget), XtNtitle, NextIncrString);
    incr_direction = 0;
    showSimpleDialog(0, &seaNextIncrDialog, 0);
}


DoPrevIncr()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    setaValue
	(XtParent(seaNextIncrDialog.popupWidget), XtNtitle, PrevIncrString);
    incr_direction = 1;
    showSimpleDialog(0, &seaNextIncrDialog, 0);
}


char *Copy(string)
    char *string;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int    size = strlen(string);
    char  *new  = malloc(size+1);

    memcpy(new, string, size);
    new[size] = 0;
    return new;
}


char *getLabel(w)
    Widget w;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Arg       arg[1];
    XmString  cs;

    arg[0].name  = XmNlabelString;
    arg[0].value = (XtArgVal) &cs;
    XtGetValues( w, arg, 1);
    return decodeCS(cs);
}


InitSearchLabels (widget, tag, any)
    Widget                widget;
    int                  *tag;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    switch (*tag)
    {
    case 1:
	NextString = getLabel(widget);
	break;
    case 2:
	PrevString = getLabel(widget);
	break;
    case 3:
	NextIncrString = getLabel(widget);
	break;
    case 4:
	PrevIncrString = getLabel(widget);
	break;
    case 5:
	OnceString = getLabel(widget);
	break;
    case 6:
	SelectedString = getLabel(widget);
	break;
    case 7:
	AllString = getLabel(widget);
	break;
    }
}


static void HelpOnContext (w, topic, any)
    Widget                w;
    char                 *topic;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    DXmHelpOnContext (toplevel, False);
}


static void MenuHelpHelp(w, topic, any)
    Widget                w;
    char                 *topic;
    XmAnyCallbackStruct  *any;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    int		n = 0;
    Arg		a[10];
#if HYPERHELP
    static char		*libname = NULL;

    /*
    ** Library name is OS dependent.
    */
    if (libname == NULL)
    {
#ifdef VMS
	libname = XtNewString("DECW$NOTEPAD");
#else
	libname = XtNewString("dxnotepad");
#endif
    }

    /*
    ** Initialize help system if necessary.
    */
    if (! help_context)
    {
	DXmHelpSystemOpen
	(
	    &help_context,
	    toplevel,
	    libname,
	    HyperHelpError,
	    "Help System Error"
	);
    }

    /*
    ** Display topic.
    */
    DXmHelpSystemDisplay
    (
	help_context,
	libname,
	"topic",
	topic,
	HyperHelpError,
	"Help System Error"
    );
#else
    XmString xm_string;
    if(!Stuff._helpWidget)
    {
        xm_string = XmStringLtoRCreate("notepad" , "ISO8859-1");
	XtSetArg (a[n], DXmNlibrarySpec, xm_string);	    n++;
	MrmFetchWidgetOverride
	(
	    DRM_hierarchy,
	    "HelpWidget",
	    mainWin,
	    NULL,
	    a,
	    n,
	    &Stuff._helpWidget,
	    &dummy_class
	);
	XmStringFree(xm_string);
    }
    xm_string = XmStringLtoRCreate(topic, "ISO8859-1");
    setaValue (Stuff._helpWidget, DXmNfirstTopic, xm_string);
    XtManageChild(Stuff._helpWidget);
#endif /* HYPERHELP */
}


static Widget makeMessageArea(w)
    Widget w;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Widget         form;
    Arg            a[20];
    int            n = 0;
    XtCallbackRec  CB[2];

 /* XtSetArg(a[n], DwtNallowResize, FALSE); n++; */
    form = XmCreateForm(w, "messageArea", a, n);
    XtManageChild(form);
 /* DwtPaneAllowResizing(form, FALSE); */
    n=0;
    XtSetArg(a[n], XmNresizeWidth, 0 );			n++;
    XtSetArg(a[n], XmNresizeHeight, 0);			n++;
    XtSetArg(a[n], XmNrows, 2); 				n++;
    XtSetArg(a[n], XmNcolumns, 60); 				n++;
    XtSetArg(a[n], XmNtopAttachment,   XmATTACH_FORM);	n++;
    XtSetArg(a[n], XmNbottomAttachment,  XmATTACH_FORM);	n++;
    XtSetArg(a[n], XmNleftAttachment,  XmATTACH_FORM);	n++;
    messageDialog.stringWidget = XmCreateText(form, "messageString", a,n);
    XtManageChild (messageDialog.stringWidget);
    n = 0;
    CB[0].closure = (XtPointer)form;
    CB[0].callback = (XtCallbackProc) cancelDialog;
    CB[1].callback = (XtCallbackProc) NULL;
    XtSetArg(a[n], XmNleftAttachment,   XmATTACH_WIDGET); 	n++;
    XtSetArg(a[n], XmNleftWidget,messageDialog.stringWidget);n++;
    XtSetArg(a[n], XmNtopAttachment,   XmATTACH_FORM);	n++;
    XtSetArg(a[n], XtNlabel, XmStringLtoRCreate("Cancel" , "ISO8859-1"));n++;
    XtSetArg(a[n], XmNactivateCallback, CB  ); n++; 
    XtManageChild(XmCreatePushButton(form, "", a,n));
    messageDialog.popupWidget = form;
    return(form);
}


static makeInitialView()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    Arg         a[8] ;
    Arg         minArg;
    int         n = 0, panew, paneh;
    static Arg  val[3] = {{XmNwidth, 0}, {XmNheight, 0}};
    View       *view;

    n = 0;
    XtSetArg(a[n], XmNresize, XmRESIZE_GROW); n++;
    val[0].value = val[1].value = 0;
    XtGetValues(workAreaPane, val, 2);  
    view = makeView(workAreaPane, 0, val[0].value, val[1].value, 0);
    XtManageChild(view->widget); 
    XtSetArg(minArg, XmNpaneMinimum, 1);
    XtSetValues(view->widget, &minArg, 1);
    Stuff.focusView = view;
    
   XmMainWindowSetAreas(mainWin,menuBar, NULL, NULL, NULL, workAreaPane);
}


notepad_serror() 
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    perror("DRM notepad error"); 
}


static	MrmRegisterArg	regvec [] = { 
    {"DoSetFontFamily",		    (caddr_t) DoSetFontFamily},
    {"DoSetFontSize",         	    (caddr_t) DoSetFontSize},	
    {"DoSetFontMisc",         	    (caddr_t) DoSetFontMisc},
    {"CacheFontFamilyList",         (caddr_t) CacheFontFamilyList},
    {"CacheFontSizeList",           (caddr_t) CacheFontSizeList},
    {"CacheFontMiscList",           (caddr_t) CacheFontMiscList},
    {"BottomProc",		    (caddr_t) BottomProc},
    {"CreatePaneCallback",	    (caddr_t) CreatePaneCallback},
    {"DoApplyFont",		    (caddr_t) DoApplyFont},
    {"DoClear",			    (caddr_t) DoClear},
    {"DoCopy",			    (caddr_t) DoCopy},
    {"DoCustomizeFont",		    (caddr_t) DoCustomizeFont},
    {"DoCut",			    (caddr_t) DoCut},
    {"DoGotoLine",		    (caddr_t) DoGotoLine},
    {"DoHideSimpleDialog",	    (caddr_t) DoHideSimpleDialog},
    {"DoNextIncr",		    (caddr_t) DoNextIncr},
    {"DoPaste",			    (caddr_t) DoPaste},
    {"DoPrevIncr",		    (caddr_t) DoPrevIncr},
    {"DoReadSettings",	            (caddr_t) DoReadSettings},
    {"DoRedo",			    (caddr_t) DoRedo},
    {"DoRevertFont",		    (caddr_t) DoRevertFont},
    {"DoSaveSettings",	            (caddr_t) DoSaveSettings},
    {"DoSearchNext",		    (caddr_t) DoSearchNext},
    {"DoSearchNextForSelection",    (caddr_t) DoSearchNextForSelection},
    {"DoSearchPrevious",	    (caddr_t) DoSearchPrevious},
    {"DoSearchPreviousForSelection",(caddr_t) DoSearchPreviousForSelection},
    {"DoSelectAll",		    (caddr_t) DoSelectAll},
    {"DoSimpleDialog",		    (caddr_t) DoSimpleDialog},
    {"DoSplit",			    (caddr_t) DoSplit},
    {"DoUnSplit",		    (caddr_t) DoUnSplit},
    {"DoUndo",			    (caddr_t) DoUndo},
    {"GotoLineProc",		    (caddr_t) GotoLineProc},
    {"HelpOnContext",		    (caddr_t) HelpOnContext},
    {"InitSearchLabels",	    (caddr_t) InitSearchLabels},
    {"MenuHelpHelp",		    (caddr_t) MenuHelpHelp},
    {"NotepadTextCreate",	    (caddr_t) NotepadTextCreate},
    {"ReplaceAllProc",		    (caddr_t) ReplaceAllProc},
    {"ReplaceOnceProc",		    (caddr_t) ReplaceOnceProc},
    {"ReplaceSelectedProc",	    (caddr_t) ReplaceSelectedProc},
    {"SearchStringModified",	    (caddr_t) SearchStringModified},
    {"SetSearchToggle",	    	    (caddr_t) SetSearchToggle},
    {"TopProc",			    (caddr_t) TopProc},
    {"WordWrapToggle",	   	    (caddr_t) WordWrapToggle },
    {"addSearchActions",	    (caddr_t) addSearchActions},
    {"assertToggleState",	    (caddr_t) assertToggleState},
    {"doFileOp",		    (caddr_t) doFileOp },
    {"doOpenSelected",		    (caddr_t) doOpenSelected },
    {"doSaveWithFile",		    (caddr_t) doSaveWithFile},
    {"do_Exit",			    (caddr_t) do_Exit},
    {"do_New",			    (caddr_t) do_New }, 
    {"do_Quit",			    (caddr_t) do_Quit},
    {"do_Revert",		    (caddr_t) do_Revert},
    {"do_Save",			    (caddr_t) do_Save},
    {"fileSelectCallback",	    (caddr_t) fileSelectCallback},
    {"grabSelection",		    (caddr_t) grabSelection},
    {"journalCallback",	            (caddr_t) journalCallback},
    {"quitCallback",	            (caddr_t) quitCallback},
    {"registerButtons",		    (caddr_t) registerButtons},
    {"registerPopups",		    (caddr_t) registerPopups},
    {"registerSTexts",		    (caddr_t) registerSTexts},
    {"registerComposites",	    (caddr_t) registerComposites},
    {"registerQuestions",	    (caddr_t) registerQuestions},
    {"showReplaceDialog",	    (caddr_t) showReplaceDialog},
    {"showSearchDialog",	    (caddr_t) showSearchDialog},
    {"registerMenuBar",		    (caddr_t) registerMenuBar},
};


void FetchHeierchy (display)
    Display  *display;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    MrmType  *dummy_class;
    char     *file_array[1];
    FILE     *fp;
    int       status;
    int       count;

    file_array[0] = uid_filespec;
    count         = XtNumber(file_array);

    /*
    ** Define the DRM "hierarchy". For Motif 1.2, use
    ** MrmOpenHierarchyPerDisplay since the sans perdisplay version is 
    ** now deprecated. 
    */
    status = 
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
	MrmOpenHierarchyPerDisplay
        (
            display,                   /* display		*/
#else
        MrmOpenHierarchy
        (
#endif
            count,                     /* number of files	*/
            file_array,                /* files     	    	*/
            os_ext_list_ptr,           /* os_ext_list 		*/
            &DRM_hierarchy             /* ptr to returned id	*/
        );
    if (status != MrmSUCCESS)
	notepad_serror (0, 0 /* what goes here? */);
}


fetchStrings()
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    MrmCode   rtype;
    Display  *dpy = XtDisplay(toplevel);

    MrmFetchLiteral(DRM_hierarchy, "untitledString", dpy, &untitledString,
			&rtype);
    MrmFetchLiteral(DRM_hierarchy, "modifiedString", dpy, &modifiedString,
			&rtype);
    MrmFetchLiteral(DRM_hierarchy, "notepadString", dpy, &notepadString,
			&rtype);
    MrmFetchLiteral(DRM_hierarchy, "readonlyString", dpy, &readonlyString,
			&rtype);
}


#if 0
static XrmOptionDescRec options[] = {
{"-g",          "geometry",        XrmoptionSepArg,        (caddr_t) NULL},
};

#ifdef VMS
#define optionCount 1
#else
#define optionCount 0
#endif
#endif


#ifdef COMBINE                  /* ultrix client merge magic */
notepad_main (argc, argv)
#else
main (argc, argv)
#endif
    int argc;
    char **argv;
/*
 *  Function:
 *
 *  Inputs:
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    MrmCount       regnum;
    int            n, i;
    int            status;
    XtAppContext   app_context;
    Display       *top_display;

#ifdef unix
    for(i=3; i < getdtablesize(); i++)
	close(i);
#endif /* unix */
    FileMode = 0640;
    MrmInitialize ();
    DXmInitialize ();
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    XmRepTypeInstallTearOffModelConverter();
#endif

/*  status = PaneInitializeForDRM(); */
    status = NotepadInitializeForDRM();
    
#if 0
    toplevel = XtInitialize
	("notepad", CLASS_NAME, options, (Cardinal)optionCount, &argc, argv);
#else
    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

/* XtSetLanguageProc set's a locale for the toolkit when using R5 or later */
#ifdef R5_XLIB
    XtSetLanguageProc(app_context, NULL, app_context);
#endif

    top_display = XtOpenDisplay
    (
	app_context,
	NULL,
	"notepad",
	CLASS_NAME,
#if 0
	options,
	(Cardinal)optionCount,
#else
	NULL,
	0,
#endif
	&argc,
	argv
    );

    if (top_display == 0)

	/* exit with error */
	XtAppErrorMsg
	(
	    app_context,
	    "invalidDisplay",
	    "XtOpenDisplay",
	    "XtToolkitError",
	    "Can't Open display",
	    (String *) NULL,
	    (Cardinal *) NULL
	);

    toplevel = XtAppCreateShell
    (
	"notepad",
	CLASS_NAME,
	applicationShellWidgetClass,
	top_display,
	NULL,
	0
    );

#endif

    XtGetApplicationResources
	(toplevel, &Stuff, resources, XtNumber(resources), NULL, 0);

    FetchHeierchy (top_display);
    regnum = XtNumber(regvec);
    MrmRegisterNames (regvec, regnum);
    if (MrmFetchWidget (
		DRM_hierarchy,
		"s_main_window",
		toplevel,
		& mainWin,
		& dummy_class) != MrmSUCCESS) {
	notepad_serror  (0 /* pick a number */, 0);
    }

#ifndef XtNiconifyPixmap
#define XtNiconifyPixmap "iconifyPixmap"
#endif

    XtManageChild (mainWin);

    CurDpy =  XtDisplay(toplevel);
    IconInit();
    setaValue(toplevel, XtNiconPixmap, NotepadPixmap);
    setaValue(toplevel, XtNiconifyPixmap, iconifyPixmap);
    setaValue(toplevel, XtNallowShellResize, "TRUE");
    setaValue(toplevel, XmNdeleteResponse, XmDO_NOTHING);
    AddProtocols(toplevel, do_Quit, NULL);

    makeInitialView(); 

    XtRealizeWidget(toplevel);

    ConditionalReadSettings(False);     /* Load system and user resources */
    fetchStrings();			/* Init some strings for the UI */
    if (argc > 1 && argv[1]) 		/* If a filename was specified */
    {          
        DoLoad(argv[1]);                /* Read it in */
	XmProcessTraversal(Stuff.focusView->widget, XmTRAVERSE_CURRENT);
	XmTextSetInsertionPosition(Stuff.focusView->widget, (XmTextPosition) 0);
	XmTextSetCursorPosition(Stuff.focusView->widget, (XmTextPosition) 0);
    } 
    else                        	/* Else create empty notepad window */
    {				
        setSources("");			
	setLoadedFile("");		/* Remember name of current file */
	clearModified(untitledString);
    }
/* Replace the source of the current widget with our private source */
    if(Psource) PSTextSetSource(Stuff.focusView->widget, Psource, 0, 0);

#ifdef unix
    signal(SIGCHLD, handler);
 /* signal(SIGPIPE, sigPipeAbort); */   /* Can't find routine: sigPipeAbort */
#endif /* unix */

    makeDialogs ();
    XtAppMainLoop (app_context);

    exit (NormalStatus);     /* never gets here */
}


void HyperHelpError (problem_string, status)
    char  *problem_string;
    int    status;
/*
 *  Function:
 *      Fall back error routine for the help environment. If an error occurs
 *      within this help system and it cannot be processes by either
 *      LinkWorks or Bookreader, this routine will be executed.
 *
 *  Inputs:
 *      problem_string - Description of the problem.
 *      status - System supplied integer that signifies specific errer:
 *               1 : Cannot find the LinkWorks shareable image.
 *               2 : Cannot find the specified file specification.
 *
 *  Outputs:
 *
 *  Notes:
 */
{
    printf("%s, %x\n", problem_string, status);
    exit(0);
}
