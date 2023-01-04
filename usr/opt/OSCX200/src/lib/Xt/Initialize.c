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
/* $XConsortium: Initialize.c,v 1.200 91/12/19 19:30:59 rws Exp $ */

/***********************************************************
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* Make sure all wm properties can make it out of the resource manager */

#include "IntrinsicI.h"
#include "StringDefs.h"
#include "CoreP.h"
#include "ShellP.h"
#ifdef VMS
#include <descrip.h>
#include <rms.h>
#else
#include <pwd.h>
#endif
#include <stdio.h>

/* conditional compilation */

#ifdef VMS
#define I18N_BUG_FIX
#define EVENT_TIMER_FLAG 13
#ifndef MAXPATHLEN
#define MAXPATHLEN   NAM$C_MAXRSS
#endif
#endif /* VMS */

#if __STDC__
#define Const const
#else
#define Const /**/
#endif

#ifndef X_NOT_STDC_ENV
#include <stdlib.h>
#else
extern char *getenv();
#endif

#include <X11/Xlocale.h>

extern void _XtConvertInitialize();

#ifdef DEC_EXTENSION
void xnl_setlanguage();
#endif

#if (defined(SUNSHLIB) || defined(AIXSHLIB)) && defined(SHAREDCODE)
/*
 * If used as a shared library, generate code under a different name so that
 * the stub routines in sharedlib.c get loaded into the application binary.
 */
#define XtToolkitInitialize _XtToolkitInitialize
#define XtAppInitialize _XtAppInitialize
#define XtInitialize _XtInitialize
#endif /* (SUNSHLIB || AIXSHLIB) && SHAREDCODE */

/*
 * hpux
 * Hand-patched versions of HP-UX prior to version 7.0 can usefully add
 * -DUSE_UNAME in the appropriate config file to get long hostnames.
 */

#ifdef USG
#define USE_UNAME
#endif

#ifdef USE_UNAME
#include <sys/utsname.h>
#endif

/* some unspecified magic number of expected search levels for Xrm */
#define SEARCH_LIST_SIZE 1000

/*
 This is a set of default records describing the command line arguments that
 Xlib will parse and set into the resource data base.
 
 This list is applied before the users list to enforce these defaults.  This is
 policy, which the toolkit avoids but I hate differing programs at this level.
*/

static XrmOptionDescRec Const opTable[] = {
{"+rv",		"*reverseVideo", XrmoptionNoArg,	(XtPointer) "off"},
{"+synchronous","*synchronous",	XrmoptionNoArg,		(XtPointer) "off"},
{"-background",	"*background",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bd",		"*borderColor",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bg",		"*background",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bordercolor","*borderColor",	XrmoptionSepArg,	(XtPointer) NULL},
{"-borderwidth",".borderWidth",	XrmoptionSepArg,	(XtPointer) NULL},
{"-bw",		".borderWidth",	XrmoptionSepArg,	(XtPointer) NULL},
{"-display",	".display",     XrmoptionSepArg,	(XtPointer) NULL},
{"-fg",		"*foreground",	XrmoptionSepArg,	(XtPointer) NULL},
{"-fn",		"*font",	XrmoptionSepArg,	(XtPointer) NULL},
{"-font",	"*font",	XrmoptionSepArg,	(XtPointer) NULL},
{"-foreground",	"*foreground",	XrmoptionSepArg,	(XtPointer) NULL},
{"-geometry",	".geometry",	XrmoptionSepArg,	(XtPointer) NULL},
{"-iconic",	".iconic",	XrmoptionNoArg,		(XtPointer) "on"},
{"-name",	".name",	XrmoptionSepArg,	(XtPointer) NULL},
{"-reverse",	"*reverseVideo", XrmoptionNoArg,	(XtPointer) "on"},
{"-rv",		"*reverseVideo", XrmoptionNoArg,	(XtPointer) "on"},
{"-selectionTimeout",
		".selectionTimeout", XrmoptionSepArg,	(XtPointer) NULL},
{"-synchronous","*synchronous",	XrmoptionNoArg,		(XtPointer) "on"},
{"-title",	".title",	XrmoptionSepArg,	(XtPointer) NULL},
{"-xnllanguage",".xnlLanguage",	XrmoptionSepArg,	(XtPointer) NULL},
{"-xrm",	NULL,		XrmoptionResArg,	(XtPointer) NULL},
};


#ifndef VMS
/*
 * _XtGetHostname - emulates gethostname() on non-bsd systems.
 */

static int _XtGetHostname (buf, maxlen)
    char *buf;
    int maxlen;
{
    int len;

#ifdef USE_UNAME
    struct utsname name;

    uname (&name);
    len = strlen (name.nodename);
    if (len >= maxlen) len = maxlen - 1;
    (void) strncpy (buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void) gethostname (buf, maxlen);
    buf [maxlen - 1] = '\0';
    len = strlen(buf);
#endif
    return len;
}
#endif /* VMS */

#ifdef VMS
/*
**  Provide capability for applications to name their application default file.  For example, this allows
**  mwm to have a class name of "Mwm" and a resource file named "DECW$MWM.DAT".  This is usefule when
**  the class name should be the same on all platforms, but VMS requires the DECW$ on all filenames.
**  The appDefaults argument will be used to construct the filenames DECW$USER_DEFAULTS:<appDefaults>.DAT
**  and DECW$SYSTEM_DEFAULTS:<appDefaults>.DAT.
*/
static char *appDefaultFile = NULL;

void _XtSetDECApplication(appDefaults)
    char *appDefaults;
{
    if (appDefaults)
    {
	if (appDefaultFile)
	    XtFree(appDefaultFile);
	appDefaultFile = XtNewString(appDefaults);
    }
}
#endif /* VMS */

#ifdef SUNSHLIB
void _XtInherit()
{
    extern void __XtInherit();
    __XtInherit();
}
#define _XtInherit __XtInherit
#endif

#if (defined (VMS) && !defined (__alpha))
void __XtInherit()
#else

void _XtInherit()
#endif /* VMS and not Alpha */

{
    XtErrorMsg("invalidProcedure","inheritanceProc",XtCXtToolkitError,
            "Unresolved inheritance operation",
              (String *)NULL, (Cardinal *)NULL);
}


void XtToolkitInitialize()
{
    extern void _XtResourceListInitialize();

#ifdef DEC_EXTENSION
    /*
     *  The Intrinsics spec leaves the behavior of calling XtToolkitInitialize
     *  more than once undefined.  We allow it.
     */
    static Boolean inited = FALSE;

    if (inited) return;
    inited = TRUE;
#endif

#ifdef VMS
#ifdef DEC_MOTIF_EXTENSION_VXT
/******************************************************************/
/* This call will initialize the method extension code used by    */
/* the widget visualizer if the logical DECW$VXT is defined as    */
/* "ON" -- otherwise, the function returns without doing anything */
/******************************************************************/
    VXtRegisterKnownMethods ();
#endif
#endif

    /* Resource management initialization */
    XrmInitialize();

#ifdef DEC_EXTENSION_NOT_YET
    /*
     *  Here is where we can simulate "built-in" quarks for quarks
     *  that are not yet built into Xlib.
     *  Add a line XrmStringToQuark for each "built-in" quark
     */
    XrmStringToQuark("SWNoop");
#endif

    _XtResourceListInitialize();

    /* Other intrinsic intialization */
    _XtConvertInitialize();
    _XtEventInitialize();
    _XtTranslateInitialize();
#ifdef VMS
     sys$clref(EVENT_TIMER_FLAG);
     sys$clref(1);
#endif /* VMS */
}


#ifndef VMS
static String XtGetRootDirName(buf)
     String buf;
{
#ifndef X_NOT_POSIX
     uid_t uid;
#else
     int uid;
     extern int getuid();
#ifndef SYSV386
     extern struct passwd *getpwuid(), *getpwnam();
#endif
#endif
     struct passwd *pw;
     static char *ptr = NULL;

     if (ptr == NULL) {
	if (!(ptr = getenv("HOME"))) {
	    if (ptr = getenv("USER")) pw = getpwnam(ptr);
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

     if (ptr)
 	(void) strcpy(buf, ptr);

     buf += strlen(buf);
     *buf = '/';
     buf++;
     *buf = '\0';
     return buf;
}
#endif /* VMS */

static void CombineAppUserDefaults(dpy, pdb)
    Display *dpy;
    XrmDatabase *pdb;
{
    char* filename;
    char* path;
    Boolean deallocate = False;

#ifdef VMS
        XtPerDisplay pd = _XtGetPerDisplay(dpy);
	filename = XtMalloc(MAXPATHLEN);
	path = NULL;  /* keep some compilers happy */
	(void) strcpy(filename, "DECW$USER_DEFAULTS:");
	if (appDefaultFile)
	    (void) strcat(filename, appDefaultFile);
	else
	    (void) strcat(filename, XrmClassToString(pd->class));
	(void) strcat(filename, ".DAT");
#else
    if (!(path = getenv("XUSERFILESEARCHPATH"))) {
	char *old_path;
	char homedir[PATH_MAX];
	XtGetRootDirName(homedir);
	if (!(old_path = getenv("XAPPLRESDIR"))) {
#ifdef I18N_BUG_FIX
            char *path_default = "%s/%%L/%%N%%C:%s/%%l_%%t/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l_%%t/%%N:%s/%%l/%%N:%s/%%N";
            if (!(path =
                ALLOCATE_LOCAL(8*strlen(homedir) + strlen(path_default))))
              _XtAllocError(NULL);
            sprintf( path, path_default,
                  homedir, homedir, homedir, homedir,
                  homedir, homedir, homedir, homedir);
#else
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
	    if (!(path =
		  ALLOCATE_LOCAL(6*strlen(homedir) + strlen(path_default))))
		_XtAllocError(NULL);
	    sprintf( path, path_default,
		    homedir, homedir, homedir, homedir, homedir, homedir );
#endif
	} else {
#ifdef I18N_BUG_FIX
          char *path_default = "%s/%%L/%%N%%C:%s/%%l_%%t/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l_%%t/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
          if (!(path =
                ALLOCATE_LOCAL( 8*strlen(old_path) + 2*strlen(homedir)
                               + strlen(path_default))))
              _XtAllocError(NULL);
          sprintf(path, path_default,
                  old_path, old_path, old_path, old_path, homedir,
                  old_path, old_path, old_path, old_path, homedir );
#else
	    char *path_default = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
	    if (!(path =
		  ALLOCATE_LOCAL( 6*strlen(old_path) + 2*strlen(homedir)
				 + strlen(path_default))))
		_XtAllocError(NULL);
	    sprintf(path, path_default, old_path, old_path, old_path, homedir,
		    old_path, old_path, old_path, homedir );
#endif
	}
	deallocate = True;
    }

    filename = XtResolvePathname(dpy, NULL, NULL, NULL, path, NULL, 0, NULL);
#endif /* VMS */
    if (filename) {
	(void)XrmCombineFileDatabase(filename, pdb, False);
	XtFree(filename);
    }

    if (deallocate) DEALLOCATE_LOCAL(path);
}

static void CombineUserDefaults(dpy, pdb)
    Display *dpy;
    XrmDatabase *pdb;
{
    char *dpy_defaults = XResourceManagerString(dpy);

    if (dpy_defaults) {
	XrmCombineDatabase(XrmGetStringDatabase(dpy_defaults), pdb, False);
    } else {
	char filename[PATH_MAX];
#ifdef VMS
	(void) strcpy(filename, "SYS$LOGIN:DECW$XDEFAULTS.DAT");
#else
	(void) XtGetRootDirName(filename);
	(void) strcat(filename, ".Xdefaults");
#endif /* VMS */
	(void)XrmCombineFileDatabase(filename, pdb, False);
    }

#ifdef DEC_EXTENSION
/* This extension merges the DXM_DEFAULTS property from the root window
 * into the database.  This property is normally put there by DEC
 * specific tools like the color customizer.  It allows display specific
 * customizations to be made without changing a user's Xdefaults file.
 */	

    {
	Atom atom, type;
	int format;
	unsigned long nitems, left;
	char *dxm_string = NULL;

	atom = XInternAtom (dpy, "DXM_DEFAULTS", TRUE);

	if (atom != (Atom) None)
	{
	    XGetWindowProperty (dpy, DefaultRootWindow(dpy),
			    atom, 0, 999999, FALSE, AnyPropertyType,
			    &type, &format, &nitems, &left, (unsigned char **) &dxm_string);

	    if (type != (Atom) None)
	    {	
	    	XrmCombineDatabase(XrmGetStringDatabase(dxm_string), pdb, TRUE);

	    	if (dxm_string != NULL)
			XtFree (dxm_string);
	    }
	}
    }
#endif

}

/*ARGSUSED*/
static Bool StoreDBEntry(db, bindings, quarks, type, value, data)
    XrmDatabase		*db;
    XrmBindingList      bindings;
    XrmQuarkList	quarks;
    XrmRepresentation   *type;
    XrmValuePtr		value;
    XPointer		data;
{
    XrmQPutResource((XrmDatabase *)data, bindings, quarks, *type, value);
    return False;
}

static XrmDatabase CopyDB(db)
    XrmDatabase db;
{
    XrmDatabase copy = NULL;
    XrmQuark empty = NULLQUARK;

    XrmEnumerateDatabase(db, &empty, &empty, XrmEnumAllLevels,
			 StoreDBEntry, (XPointer)&copy);
    return copy;
}

/*ARGSUSED*/
static String _XtDefaultLanguageProc(dpy, xnl, closure)
    Display   *dpy;	/* unused */
    String     xnl;
    XtPointer  closure;	/* unused */
{
    if (! setlocale(LC_ALL, xnl))
	XtWarning("locale not supported by C library, locale unchanged");

    if (! XSupportsLocale()) {
	XtWarning("locale not supported by Xlib, locale set to C");
	setlocale(LC_ALL, "C");
    }
    if (! XSetLocaleModifiers(""))
	XtWarning("X locale modifiers not supported, using default");

#ifdef I18N_BUG_FIX
    return setlocale(LC_CTYPE, NULL); /* re-query in case overwritten */
#else
    return setlocale(LC_ALL, NULL); /* re-query in case overwritten */
#endif
}

#if NeedFunctionPrototypes
XtLanguageProc XtSetLanguageProc(
    XtAppContext      app,
    XtLanguageProc    proc,
    XtPointer         closure
    )
#else
XtLanguageProc XtSetLanguageProc(app, proc, closure)
    XtAppContext      app;
    XtLanguageProc    proc;
    XtPointer         closure;
#endif
{
    XtLanguageProc    old;

    if (!proc) {
	proc = _XtDefaultLanguageProc;
	closure = NULL;
    }

    if (app) {
	/* set langProcRec only for this application context */
        old = app->langProcRec.proc;
        app->langProcRec.proc = proc;
        app->langProcRec.closure = closure;
    } else {    
	/* set langProcRec for all application contexts */
        ProcessContext process = _XtGetProcessContext();

        old = process->globalLangProcRec.proc;
	process->globalLangProcRec.proc = proc;
	process->globalLangProcRec.closure = closure;
        app = process->appContextList;
        while (app) {
            app->langProcRec.proc = proc;
            app->langProcRec.closure = closure;
	    app = app->next;
        }
    }
    return (old ? old : _XtDefaultLanguageProc);
}

XrmDatabase XtScreenDatabase(screen)
    Screen *screen;
{
    Display *dpy;
    int scrno;
    Bool doing_def;
    XrmDatabase db, olddb;
    XtPerDisplay pd;
    Status do_fallback;
    char *scr_resources;

    dpy = DisplayOfScreen(screen);
    if (screen == DefaultScreenOfDisplay(dpy)) {
	scrno = DefaultScreen(dpy);
	doing_def = True;
    } else {
	scrno = XScreenNumberOfScreen(screen);
	doing_def = False;
    }
    pd = _XtGetPerDisplay(dpy);
    if (db = pd->per_screen_db[scrno])
	return doing_def ? XrmGetDatabase(dpy) : db;
    scr_resources = XScreenResourceString(screen);

    if (ScreenCount(dpy) == 1) {
	db = pd->cmd_db;
	pd->cmd_db = NULL;
    } else {
	db = CopyDB(pd->cmd_db);
    }
    {   /* Environment defaults */
	char	filenamebuf[PATH_MAX];
	char	*filename;

#ifdef VMS
	filename = (char *) NULL;
	/* A way needs to be found to get the current host name on VMS */
	/* so that a per-host defaults file can be specified.	       */
#endif
	if (!(filename = getenv("XENVIRONMENT"))) {
#ifndef VMS
	    int len;
	    (void) XtGetRootDirName(filename = filenamebuf);
	    (void) strcat(filename, ".Xdefaults-");
	    len = strlen(filename);
	    (void) _XtGetHostname (filename+len, PATH_MAX-len);
#endif /* VMS */
	}
#ifdef VMS
	if (filename)
#endif
	(void)XrmCombineFileDatabase(filename, &db, False);
    }
    if (scr_resources)
    {   /* Screen defaults */
	XrmCombineDatabase(XrmGetStringDatabase(scr_resources), &db, False);
	XFree(scr_resources);
    }
    /* Server or host defaults */
    if (!pd->server_db)
	CombineUserDefaults(dpy, &db);
    else {
	(void) XrmCombineDatabase(pd->server_db, &db, False);
	pd->server_db = NULL;
    }

    if (!db)
	db = XrmGetStringDatabase("");
    pd->per_screen_db[scrno] = db;
    olddb = XrmGetDatabase(dpy);
    /* set database now, for XtResolvePathname to use */
    XrmSetDatabase(dpy, db);
    CombineAppUserDefaults(dpy, &db);
    do_fallback = 1;
    {   /* System app-defaults */
	char	*filename;

#ifdef VMS
        XtPerDisplay pd = _XtGetPerDisplay(dpy);
	filename = XtMalloc(MAXPATHLEN);
	(void) strcpy(filename, "DECW$SYSTEM_DEFAULTS:");
	if (appDefaultFile)
	    (void) strcat(filename, appDefaultFile);
	else
	    (void) strcat(filename, XrmClassToString(pd->class));
	(void) strcat(filename, ".DAT");
	{
#else
	if (filename = XtResolvePathname(dpy, "app-defaults",
					 NULL, NULL, NULL, NULL, 0, NULL)) {
#endif /* VMS */
	    do_fallback = !XrmCombineFileDatabase(filename, &db, False);
	    XtFree(filename);
	}
    }
    /* now restore old database, if need be */
    if (!doing_def)
	XrmSetDatabase(dpy, olddb);
    if (do_fallback && pd->appContext->fallback_resources)
    {   /* Fallback defaults */
        XrmDatabase fdb = NULL;
	String *res;

	for (res = pd->appContext->fallback_resources; *res; res++)
	    XrmPutLineResource(&fdb, *res);
	(void)XrmCombineDatabase(fdb, &db, False);
    }
    return db;
}

/*
 * Merge two option tables, allowing the second to over-ride the first,
 * so that ambiguous abbreviations can be noticed.  The merge attempts
 * to make the resulting table lexicographically sorted, but succeeds
 * only if the first source table is sorted.  Though it _is_ recommended
 * (for optimizations later in XrmParseCommand), it is not required
 * that either source table be sorted.
 *
 * Caller is responsible for freeing the returned option table.
 */

static void _MergeOptionTables(src1, num_src1, src2, num_src2, dst, num_dst)
    XrmOptionDescRec *src1, *src2;
    Cardinal num_src1, num_src2;
    XrmOptionDescRec **dst;
    Cardinal *num_dst;
{
    XrmOptionDescRec *table, *endP;
    register XrmOptionDescRec *opt1, *opt2, *whereP, *dstP; 
    int i1, i2, dst_len, order;
    Boolean found;
    enum {Check, NotSorted, IsSorted} sort_order = Check;

    *dst = table = (XrmOptionDescRec*)
	XtMalloc( sizeof(XrmOptionDescRec) * (num_src1 + num_src2) );

    bcopy( src1, table, sizeof(XrmOptionDescRec) * num_src1 );
    if (num_src2 == 0) {
	*num_dst = num_src1;
	return;
    }
    endP = &table[dst_len = num_src1];
    for (opt2 = src2, i2= 0; i2 < num_src2; opt2++, i2++) {
	found = False;
	whereP = endP-1;	/* assume new option goes at the end */
	for (opt1 = table, i1 = 0; i1 < dst_len; opt1++, i1++) {
	    /* have to walk the entire new table so new list is ordered
	       (if src1 was ordered) */
	    if (sort_order == Check && i1 > 0
		&& strcmp(opt1->option, (opt1-1)->option) < 0)
		sort_order = NotSorted;
	    if ((order = strcmp(opt1->option, opt2->option)) == 0) {
		/* same option names; just overwrite opt1 with opt2 */
		*opt1 = *opt2;
		found = True;
		break;
		}
	    /* else */
	    if (sort_order == IsSorted && order > 0) {
		/* insert before opt1 to preserve order */
		/* shift rest of table forward to make room for new entry */
		for (dstP = endP++; dstP > opt1; dstP--)
		    *dstP = *(dstP-1);
		*opt1 = *opt2;
		dst_len++;
		found = True;
		break;
	    }
	    /* else */
	    if (order < 0)
		/* opt2 sorts after opt1, so remember this position */
		whereP = opt1;
	}
	if (sort_order == Check && i1 == dst_len)
	    sort_order = IsSorted;
	if (!found) {
	   /* when we get here, whereP points to the last entry in the
	      destination that sorts before "opt2".  Shift rest of table
	      forward and insert "opt2" after whereP. */
	    whereP++;
	    for (dstP = endP++; dstP > whereP; dstP--)
		*dstP = *(dstP-1);
	    *whereP = *opt2;
	    dst_len++;
	}
    }
    *num_dst = dst_len;
}


/* NOTE: name, class, and type must be permanent strings */
static Boolean _GetResource(dpy, list, name, class, type, value)
    Display *dpy;
    XrmSearchList list;
    String name, class, type;
    XrmValue* value;
{
    XrmRepresentation db_type;
    XrmValue db_value;
    XrmName Qname = XrmPermStringToQuark(name);
    XrmClass Qclass = XrmPermStringToQuark(class);
    XrmRepresentation Qtype = XrmPermStringToQuark(type);

    if (XrmQGetSearchResource(list, Qname, Qclass, &db_type, &db_value)) {
	if (db_type == Qtype) {
	    if (Qtype == _XtQString)
		*(String*)value->addr = db_value.addr;
	    else
		bcopy( db_value.addr, value->addr, value->size );
	    return True;
	} else {
	    WidgetRec widget; /* hack, hack */
	    bzero( &widget, sizeof(widget) );
	    widget.core.self = &widget;
	    widget.core.widget_class = coreWidgetClass;
	    widget.core.screen = (Screen*)DefaultScreenOfDisplay(dpy);
	    XtInitializeWidgetClass(coreWidgetClass);
	    if (_XtConvert(&widget,db_type,&db_value,Qtype,value,NULL)) {
		return True;
	    }
	}
    }
    return False;
}

XrmDatabase _XtPreparseCommandLine(urlist, num_urs, argc, argv, applName,
				   displayName, language)
    XrmOptionDescRec *urlist;
    Cardinal num_urs;
    int argc;
    String *argv;
    String *applName, *displayName, *language;	/* return */
{
    XrmDatabase db = 0;
    XrmOptionDescRec *options;
    Cardinal num_options;
    XrmName name_list[3];
    XrmName class_list[3];
    XrmRepresentation type;
    XrmValue val;
    String *targv;
    int targc = argc;

    targv = (String *) XtMalloc(sizeof(char *) * argc);
    bcopy(argv, targv, sizeof(char *) * argc);
    _MergeOptionTables(opTable, XtNumber(opTable), urlist, num_urs,
		       &options, &num_options);
    name_list[0] = class_list[0] = XrmPermStringToQuark(".");
    name_list[2] = class_list[2] = NULLQUARK;
    XrmParseCommand(&db, options, num_options, ".", &targc, targv);
    if (applName && ! *applName) {
	name_list[1] = XrmPermStringToQuark("name");
	if (XrmQGetResource(db, name_list, name_list, &type, &val) &&
	    type == _XtQString)
	    *applName = val.addr;
    }
    if (displayName && ! *displayName) {
	name_list[1] = XrmPermStringToQuark("display");
	if (XrmQGetResource(db, name_list, name_list, &type, &val) &&
	    type == _XtQString)
	    *displayName = val.addr;
    }
	name_list[1] = XrmPermStringToQuark("xnlLanguage");
	class_list[1] = XrmPermStringToQuark("XnlLanguage");
	if (XrmQGetResource(db, name_list, class_list, &type, &val) &&
	    type == _XtQString)
	    *language = val.addr;

    XtFree((char *)targv);
    XtFree((char *)options);
    return db;
}

static void GetLanguage(dpy, pd)
    Display *dpy;
    XtPerDisplay pd;
{
    XrmRepresentation type;
    XrmValue value;
    XrmName name_list[3];
    XrmName class_list[3];

    if (! pd->language) {
	name_list[0] = pd->name;
	name_list[1] = XrmPermStringToQuark("xnlLanguage");
	class_list[0] = pd->class;
	class_list[1] = XrmPermStringToQuark("XnlLanguage");
	name_list[2] = class_list[2] = NULLQUARK;
	if (!pd->server_db)
	    CombineUserDefaults(dpy, &pd->server_db);
	if (pd->server_db &&
	    XrmQGetResource(pd->server_db,name_list,class_list, &type, &value)
	    && type == _XtQString)
	    pd->language = (char *) value.addr;
    }

    if (pd->appContext->langProcRec.proc) {
	if (! pd->language) pd->language = "";
	pd->language = (*pd->appContext->langProcRec.proc)
	    (dpy, pd->language, pd->appContext->langProcRec.closure);
    }
    else if (! pd->language || pd->language[0] == '\0') /* R4 compatibility */
	pd->language = getenv("LANG");

    if (pd->language) pd->language = XtNewString(pd->language);
#ifdef DEC_EXTENSION
    /*
     *  Call xnl_setlanguage to set the language specific
     *  logical names
     */
    xnl_setlanguage(pd->language);
#endif

}


/* These are the os-specific environment variables checked for a language
** specification.
*/
#ifdef VMS
#define env_variable "XNL$LANG"
#else
#define env_variable "LANG"
#endif /* VMS */

#ifdef DEC_EXTENSION

#include "I18N.h"

/*  Global declarations
*/

I18nContext  _I18nGlobalContextBlock = NULL;

#ifdef VMS
#include <lnmdef.h>
#include <ssdef.h>
#include <psldef.h>

/* Provide the context for the VMS logical names that will be redefined.
**
** NOTE: do NOT add SYS$SHARE to this list.  As of VMS V5.4 at least, this
** will cause any images which are dynamically activated after this to use
** a different form of the image file specification than the one which is
** in the system's INSTALL'd list.  This causes activation of any protected
** (privileged) images to fail (such as nmail and debug).
**	Jim V. (who did this, then spent 2 days trying to figure out
**		how he'd broken it!)
*/
#define num_VMS_paths 7
static char	*VMS_paths[num_VMS_paths+1] = { 
					"SYS$LIBRARY",
					"SYS$MESSAGE",
					"SYS$HELP",
					"SYS$EXAMPLES",
					"DECW$SYSTEM_DEFAULTS",
					"CDA$LIBRARY",
					"VUE$LIBRARY"
					};
#define VMS_lnmtable "LNM$PROCESS"
#ifdef XNL_DEBUG
#define VMS_acmode PSL$C_SUPER
#else
#define VMS_acmode PSL$C_USER
#endif /* XNL_DEBUG */

int xnl_createpath();
int xnl_trnlnm();
int xnl_crelnm();

#endif  /* VMS */


/* This is a global place used to remember the last language that was set.
** It is the common pointer shared by the set and get routines.
*/
static char *last_language = NULL;


/*
 *  xnl_setlanguage sets user-mode process logical names for the 
 *  i18n search lists.
 */
void xnl_setlanguage(language)
	String language;
{
    int language_len;

    if (language != NULL) {
#ifdef VMS
        int status, i;
	for (i=0; i<num_VMS_paths; i++) {
	    status = xnl_createpath(VMS_paths[i], language);
	}
#endif  /* VMS */
	if (last_language != NULL) free(last_language);
	language_len = strlen(language) + 1;
	last_language = (char*) malloc((unsigned) language_len);
	bcopy(language, last_language, language_len);
    }
    /*
     *  Create an I18N context block on the first call to this routine
     */
    if (_I18nGlobalContextBlock == NULL)
    {
        _I18nGlobalContextBlock = (I18nContext)XtMalloc (sizeof(I18nContextRec));
        _I18nGlobalContextBlock->locale = XtNewString(language);
        _I18nGlobalContextBlock->use_mrm_hierarchy = True;
        _I18nGlobalContextBlock->mrm_hierarchy_id = NULL;
        _I18nGlobalContextBlock->widget_class = NULL;

/* License check is done in _I18nLoadShareable in Chinese and Korean */
        I18nLoadShareable();
    }
}

 
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_getlanguage - returns the last saved language specification.
**
**  FORMAL PARAMETERS:
**
**	none
**
**  IMPLICIT INPUTS:
**
**      The global pointer "last_language"
**
**  FUNCTION VALUE:
**
**      The language that was set is returned.  This may be null if
**	no language has been successfully set.
**
**  SIDE EFFECTS:
**
**	None
**
**--
**/

char *xnl_getlanguage()
{
	return(last_language);
}


#ifdef VMS
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	xnl_createpath - create a language specific variant of a
**	    logical name defined search path.
**
**  FORMAL PARAMETERS:
**
**	path - the logical name to create a language specific
**	    variant of.
**	language - the variant to create.
**
**  IMPLICIT INPUTS:
**
**	Existing search list elements from the logical name are 
**	propagated to the new logical name.
**
**  FUNCTION VALUE:
**
**	1 is returned if success, 0 otherwise.
**
**  SIDE EFFECTS:
**
**	The new logical name is created.
**
**--
**/

int xnl_createpath(path, language)
    char *path;
    char *language;
{
	int status, i, trnsl_num;
	char *new_path;
	char *trnsl_table[129];
	int path_len, language_len;
	char *ptr, *lang;
#ifdef I18N_BUG_FIX
	char *new_path1;
	int num_path;
#endif

	/* Allocate enough memory to build a new logical name specification 
	** and then build the logical name string.
	*/
	path_len = strlen(path);
	language_len = strlen(language);
	new_path = XtMalloc(path_len + language_len + 3);

	if (new_path == 0) {
#ifdef XNL_DEBUG
	    printf("Error: could not allocate memory\n");
#endif /* XNL_DEBUG */
	    return(0);
	}

	bcopy( path, new_path, path_len);
	new_path[path_len] =  '_';
	/*
	 * Replace '.' and '@' with '_' if one exists.
	 * E.g. ja_JP.deckanji@mod -> ja_JP_deckanji_mod
	 */
	lang = XtMalloc ( language_len + 1 );
	strcpy( lang, language);
	if ( ptr = strchr ( lang, '.') )
	    *ptr = '_';
	if ( ptr = strchr ( lang, '@') )
	    *ptr = '_';
	bcopy( lang, &new_path[path_len+1], language_len);
	new_path[path_len+1+language_len] =  ':';
	new_path[path_len+1+language_len+1] =  '\0';

	trnsl_table[0] = new_path;

#ifdef I18N_BUG_FIX
	/* We have to make one more logical for no codeset fallback */
	num_path = 1;
	strcpy( lang, language);
	if ( ptr = strchr ( lang, '.') )
	    *ptr = '\0';
	if (language_len != strlen(lang)) {
	    language_len = strlen(lang);
	    new_path1 = XtMalloc(path_len + language_len + 3);
	    bcopy( path, new_path1, path_len);
	    new_path1[path_len] =  '_';
	    bcopy( lang, &new_path1[path_len+1], language_len);
	    new_path1[path_len+1+language_len] =  ':';
	    new_path1[path_len+1+language_len+1] =  '\0';
	    trnsl_table[1] = new_path1;
	    num_path++;
	}
	XtFree(lang);
#endif

	/* Get the old values of the general purpose search list.  If this 
	** fails, then something is really wrong since these logical names 
	** should always be defined.
	*/
	status = xnl_trnlnm(path,
		          "LNM$FILE_DEV",
		          &trnsl_num,
#ifdef I18N_BUG_FIX
			  &trnsl_table[num_path],
#else
		          &trnsl_table[1],
#endif
		          PSL$C_USER);

	if (status != 1) {
	    XtFree(new_path);
#ifdef I18N_BUG_FIX
	    if (num_path > 1)
		XtFree(new_path1);
#endif
#ifdef XNL_DEBUG
	    printf("Error: logical name %s does not exist\n",path);
#endif /* XNL_DEBUG */
	    return(0);
	}


	/* Everything looks good.  Define the logical.
	*/
	trnsl_num = trnsl_num + 1;

	status = xnl_crelnm(path,
			     VMS_lnmtable,
			     trnsl_table,
			     trnsl_num,
			     VMS_acmode);


	/* We're done.  Free up our memory and get out.
	*/
	for (i=0; i<trnsl_num; i++) XtFree(trnsl_table[i]);
	return(status);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_trnlnm - translates a logical name and returns its values.
**
**  FORMAL PARAMETERS:
**
**	lognam - name to translate 
**	tabnam - logical name table to look in
**	trnsl_num (returned) - num of translations found 
**	trnsl_table (returned) - table of pointers to translations found.
**		These pointers need to be deallocated by the caller.
**	access_mode - access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**	If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**	None.
**
**--
**/

int xnl_trnlnm(lognam, tabnam, trnsl_num, trnsl_table, access_mode)
    char *lognam;
    char *tabnam;
    int	*trnsl_num;
    char *trnsl_table[];
    unsigned char access_mode;
{
	typedef struct {
	    unsigned short buf_len;
	    unsigned short item_code;
	    unsigned char *buf_add;
	    unsigned long ret_add;
	} item_desc;

	item_desc item_list[3];
	unsigned long attr, index;
	unsigned short ret_len;
	struct dsc$descriptor_s	lognam_dsc, tabnam_dsc;
	char temp_buf[255];
	int status;


	/* Set up the inputs for the translation.  We can do this once 
	** because everything that changes is passed by ref.
	*/
	lognam_dsc.dsc$a_pointer = lognam;
	lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	lognam_dsc.dsc$w_length  = strlen(lognam);
	lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	tabnam_dsc.dsc$a_pointer = tabnam;
	tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	tabnam_dsc.dsc$w_length  = strlen(tabnam);
	tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	attr = LNM$M_CASE_BLIND;
	index = 0;

	item_list[0].buf_len   = 4;
	item_list[0].buf_add   = &index;
	item_list[0].item_code = LNM$_INDEX;
	item_list[0].ret_add   = &ret_len;

	item_list[1].buf_len   = 255;
	item_list[1].buf_add   = temp_buf;
	item_list[1].item_code = LNM$_STRING;
	item_list[1].ret_add   = &ret_len;

	item_list[2].buf_len   = 0;
	item_list[2].buf_add   = 0;
	item_list[2].item_code = 0;
	item_list[2].ret_add   = 0;


	/* Do as many translations as are required to get all the existing
	** values for the logical name.  For each value, allocate enough 
	** memory to save a copy of it and then put a pointer to it into 
	** our return table.
	*/
	do {
	    status = SYS$TRNLNM(&attr,
			    &tabnam_dsc,
			    &lognam_dsc,
			    &access_mode,
			    &item_list);

	    if ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) ) {
		trnsl_table[index] = XtMalloc( ret_len + 1);
		bcopy(item_list[1].buf_add, trnsl_table[index], ret_len);
		strcpy(trnsl_table[index] + ret_len, "");
		index++;
	    }
	}
	while ( ((status & SS$_NORMAL) == SS$_NORMAL) && (ret_len != 0) );


	/* Return the number of translations found and a normalized status.
	*/
	*trnsl_num = index;
	if ((status & SS$_NORMAL) == SS$_NORMAL) {
	    return (1);
	}
	else return (0);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      xnl_crelnm - creates a logical name 
**	
**  FORMAL PARAMETERS:
**
**	lognam - name to create
**	tabnam - logical name table to create it in
**	trnsl_num - num of translations to be provided
**	trnsl_table - table of translations to provide
**	access_mode - access mode to look in
**
**  IMPLICIT INPUTS:
**
**      The existing logical name.
**
**  FUNCTION VALUE:
**
**	If successful, 1.  Otherwise, 0.
**
**  SIDE EFFECTS:
**
**	The logical name is created.
**
**--
**/

int xnl_crelnm(lognam, tabnam, trnsl_table, trnsl_num, access_mode)
    char *lognam;
    char *tabnam;
    char *trnsl_table[];
    int trnsl_num;
    unsigned char access_mode;
{
	typedef struct {
	    unsigned short buf_len;
	    unsigned short item_code;
	    unsigned long buf_add;
	    unsigned long ret_add;
	} item_desc;

	struct dsc$descriptor_s	lognam_dsc, tabnam_dsc;
	item_desc item_list[129];
	unsigned long attr, index;
	int status;


	/* Set up the inputs for the creation.  
	*/
	attr = 0;

	lognam_dsc.dsc$a_pointer = lognam;
	lognam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	lognam_dsc.dsc$w_length  = strlen(lognam);
	lognam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	tabnam_dsc.dsc$a_pointer = tabnam;
	tabnam_dsc.dsc$b_dtype   = DSC$K_DTYPE_T;
	tabnam_dsc.dsc$w_length  = strlen(tabnam);
	tabnam_dsc.dsc$b_class   = DSC$K_CLASS_S;

	for (index=0; index < trnsl_num; index++) {
	    item_list[index].buf_add   = trnsl_table[index];
	    item_list[index].buf_len   = strlen(trnsl_table[index]);
	    item_list[index].item_code = LNM$_STRING;
	}

	item_list[trnsl_num].buf_add   = 0;
	item_list[trnsl_num].buf_len   = 0;
	item_list[trnsl_num].item_code = 0;


	/* Create the logical name
	*/
	if (access_mode == PSL$C_SUPER) {
	    status = LIB$SET_LOGICAL(&lognam_dsc,
			   0,
		           &tabnam_dsc,
		           0,
		           &item_list);
	}
	else {
	    status = SYS$CRELNM(&attr,
		           &tabnam_dsc,
		           &lognam_dsc,
		           &access_mode,
		           &item_list);
	}


	/* Return a normalized status.
	*/
	if ((status != SS$_NORMAL) && (status != SS$_SUPERSEDE)) {
#ifdef XNL_DEBUG
	    printf("Error: creation of %s in %s failed\n", lognam, tabnam);
#endif /* XNL_DEBUG */
	    return (0);
	}
	else return (1);
}
#endif /* VMS */
#endif /* DEC_EXTENSION */



#if NeedFunctionPrototypes
void _XtDisplayInitialize(
	Display *dpy,
        XtPerDisplay pd,
	_Xconst char* name,
	XrmOptionDescRec *urlist,
	Cardinal num_urs,
	int *argc,
	char **argv)
#else
void _XtDisplayInitialize(dpy, pd, name, urlist, num_urs, argc, argv)
	Display *dpy;
        XtPerDisplay pd;
	String name;
	XrmOptionDescRec *urlist;
	Cardinal num_urs;
	int *argc;
	char **argv;
#endif
{
	Boolean tmp_bool;
	XrmValue value;
	XrmOptionDescRec *options;
	Cardinal num_options;
	XrmDatabase db;
	XrmName name_list[2];
	XrmClass class_list[2];
	XrmHashTable* search_list;
	int search_list_size = SEARCH_LIST_SIZE;

	GetLanguage(dpy, pd);

	/* Parse the command line and remove Xt arguments from argv */
	_MergeOptionTables( opTable, XtNumber(opTable), urlist, num_urs,
			    &options, &num_options );
	XrmParseCommand(&pd->cmd_db, options, num_options, name, argc, argv);

	db = XtScreenDatabase(DefaultScreenOfDisplay(dpy));

	if (!(search_list = (XrmHashTable*)
		       ALLOCATE_LOCAL( SEARCH_LIST_SIZE*sizeof(XrmHashTable))))
	    _XtAllocError(NULL);
	name_list[0] = pd->name;
	class_list[0] = pd->class;
	name_list[1] = NULLQUARK;
	class_list[1] = NULLQUARK;

	while (!XrmQGetSearchList(db, name_list, class_list,
				  search_list, search_list_size)) {
	    XrmHashTable* old = search_list;
	    Cardinal size = (search_list_size*=2)*sizeof(XrmHashTable);
	    if (!(search_list = (XrmHashTable*)ALLOCATE_LOCAL(size)))
		_XtAllocError(NULL);
	    bcopy( (char*)old, (char*)search_list, (size>>1) );
	    DEALLOCATE_LOCAL(old);
	}

	value.size = sizeof(tmp_bool);
	value.addr = (XtPointer)&tmp_bool;
	if (_GetResource(dpy, search_list, "synchronous", "Synchronous",
			 XtRBoolean, &value)) {
	    int i;
	    Display **dpyP = pd->appContext->list;
	    pd->appContext->sync = tmp_bool;
	    for (i = pd->appContext->count; i; dpyP++, i--) {
		(void) XSynchronize(*dpyP, (Bool)tmp_bool);
	    }
	} else {
	    (void) XSynchronize(dpy, (Bool)pd->appContext->sync);
	}

	if (_GetResource(dpy, search_list, "reverseVideo", "ReverseVideo",
			 XtRBoolean, &value)
	        && tmp_bool) {
	    pd->rv = True;
	}

	value.size = sizeof(pd->multi_click_time);
	value.addr = (XtPointer)&pd->multi_click_time;
	if (!_GetResource(dpy, search_list,
			  "multiClickTime", "MultiClickTime",
			  XtRInt, &value)) {
	    pd->multi_click_time = 200;
	}

	value.size = sizeof(pd->appContext->selectionTimeout);
	value.addr = (XtPointer)&pd->appContext->selectionTimeout;
	(void)_GetResource(dpy, search_list,
			   "selectionTimeout", "SelectionTimeout",
			   XtRInt, &value);

#ifndef NO_IDENTIFY_WINDOWS
	value.size = sizeof(pd->appContext->identify_windows);
	value.addr = (XtPointer)&pd->appContext->identify_windows;
	(void)_GetResource(dpy, search_list,
			   "xtIdentifyWindows", "XtDebug",
			   XtRBoolean, &value);
#endif

	XtFree( (XtPointer)options );
	DEALLOCATE_LOCAL( search_list );
}

/*	Function Name: XtAppSetFallbackResources
 *	Description: Sets the fallback resource list that will be loaded
 *                   at display initialization time.
 *	Arguments: app_context - the app context.
 *                 specification_list - the resource specification list.
 *	Returns: none.
 */

#if NeedFunctionPrototypes
void
XtAppSetFallbackResources(
XtAppContext app_context,
String *specification_list
)
#else
void
XtAppSetFallbackResources(app_context, specification_list)
XtAppContext app_context;
String *specification_list;
#endif
{
    app_context->fallback_resources = specification_list;
}

/*	Function Name: XtAppInitialize
 *	Description: A convience routine for Initializing the toolkit.
 *	Arguments: app_context_return - The application context of the
 *                                      application
 *                 application_class  - The class of the application.
 *                 options            - The option list.
 *                 num_options        - The number of options in the above list
 *                 argc_in_out, argv_in_out - number and list of command line
 *                                            arguments.
 *                 fallback_resource  - The fallback list of resources.
 *                 args, num_args     - Arguements to use when creating the 
 *                                      shell widget.
 *	Returns: The shell widget.
 */
	
#if NeedFunctionPrototypes
Widget
XtAppInitialize(
XtAppContext * app_context_return,
_Xconst char* application_class,
XrmOptionDescRec *options,
Cardinal num_options,
int *argc_in_out,
String *argv_in_out,
String *fallback_resources,
ArgList args_in,
Cardinal num_args_in
)
#else
Widget
XtAppInitialize(app_context_return, application_class, options, num_options,
		argc_in_out, argv_in_out, fallback_resources, 
		args_in, num_args_in)
XtAppContext * app_context_return;
String application_class;
XrmOptionDescRec *options;
Cardinal num_options, num_args_in;
int *argc_in_out;
String *argv_in_out, * fallback_resources;     
ArgList args_in;
#endif
{
    XtAppContext app_con;
    Display * dpy;
    register int saved_argc = *argc_in_out;
    Widget root;
    Arg args[3], *merged_args;
    Cardinal num = 0;
    
    XtToolkitInitialize(); /* cannot be moved into _XtAppInit */
    
    dpy = _XtAppInit(&app_con, (String)application_class, options, num_options,
		     argc_in_out, &argv_in_out, fallback_resources);

    XtSetArg(args[num], XtNscreen, DefaultScreenOfDisplay(dpy)); num++;
    XtSetArg(args[num], XtNargc, saved_argc);	                 num++;
    XtSetArg(args[num], XtNargv, argv_in_out);	                 num++;

    merged_args = XtMergeArgLists(args_in, num_args_in, args, num);
    num += num_args_in;

    root = XtAppCreateShell(NULL, application_class, 
			    applicationShellWidgetClass,dpy, merged_args, num);
    
    if (app_context_return)
	*app_context_return = app_con;

    XtFree((XtPointer)merged_args);
    XtFree((XtPointer)argv_in_out);
    return(root);
}

/*	Function Name: XtInitialize
 *	Description: This function can be used to initialize the toolkit.
 *		     It is obsolete; XtAppInitialize is more useful.
 *	Arguments: name - ** UNUSED **
 *                 classname - name of the application class.
 *                 options, num_options - the command line option info.
 *                 argc, argc - the command line args from main().
 *	Returns: a shell widget.
 */
	
/*ARGSUSED*/
#if NeedFunctionPrototypes
Widget 
XtInitialize(
_Xconst char* name,
_Xconst char* classname,
XrmOptionDescRec *options,
Cardinal num_options,
int *argc,
String *argv
)
#else
Widget 
XtInitialize(name, classname, options, num_options, argc, argv)
String name, classname;
XrmOptionDescRec *options;
Cardinal num_options;
String *argv;
int *argc;
#endif
{
    Widget root;
    XtAppContext app_con;
    register ProcessContext process = _XtGetProcessContext();

    root = XtAppInitialize(&app_con, classname, options, num_options,
			   argc, argv, NULL, NULL, (Cardinal) 0);

    process->defaultAppContext = app_con;
    
    return(root);
}

#if ( defined(__osf__) && defined(__alpha) )
    /* required for binary compatability with Mrm 1.1.3 only */
#ifdef DEC_EXTENSION
/* This routine is provided to give Xm a way to register destroy callbacks
 * that are invoked when a display is closed.  These callbacks are needed
 * so that Xm can clean up various gc and pixmap caches ad other per display
 * items.
 */
void _XtAddPDDestroyCallback (display, widget, cbproc, closure)
    Display *display;
    Widget widget;
    XtCallbackProc cbproc;
    XtPointer closure;
{
    XtPerDisplay pd = _XtGetPerDisplay(display);

    _XtAddCallback (&pd->destroy_callbacks,cbproc, closure);
}
#endif
#endif


