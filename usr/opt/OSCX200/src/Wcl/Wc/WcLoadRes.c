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
#include <X11/Wc/COPY>

/*
* SCCS_data: @(#) WcLoadRes.c 1.6 92/04/02 10:57:07
*
* Widget Creation Library - WcLoadRes.c
*
* WcLoadResourceFile remembers which resource files have been loaded (by
* resource file name, not complete path name).  It handles absolute pathnames
* starting at root or tilda, or uses the same search path algorithm used by
* GetAppDefaults in X11R4 Xt.  I.e., it uses XUSERFILESEARCHPATH, the users
* home directory, and XAPPLRESDIR in the same way that XtR4 does when it gets
* users application defaults.  This code basically mimics GetAppUserDefaults()
* in mit/lib/Xt/Initialize.c
*
*******************************************************************************
*/

#include <X11/IntrinsicP.h>
#include <X11/Wc/WcCreateP.h>
#include <X11/Wc/MapAg.h>
#include <stdlib.h>		/* getenv, getuid	*/
#include <pwd.h>		/* getpwuid, getpwnam	*/

/*
*******************************************************************************
* Private_data_declarations.
*******************************************************************************
*/

/*  -- Mapping Agent for remembering resource file names loaded.
*******************************************************************************
*/

static MapAgRec rfAgentRec  = MapAg_STATIC;
static MapAg    rfAgent     = &rfAgentRec;

/*
*******************************************************************************
* Private_function_declarations.
*******************************************************************************
*/

/*  -- Find Home Directory
*******************************************************************************
    Used by WcLoadResourceFile and WcUserDefined.
    Argument can be THIS_USER (a NULL) or can be a user's name.
    The character string returned is in static storage, as returned
    by getpwnam().
*/

#define THIS_USER ((char*)0)

char* WcHomeDirectory( user )
    char* user;
{
    struct passwd *pw;
    char* homeDir = NULL;

    if ( WcNull(user) )
    {
	homeDir = getenv("HOME");

        if (WcNull(homeDir))
        {
            char* thisUser = getenv("USER");

            if (WcNull(thisUser))
                pw = getpwuid( getuid() );
            else
                pw = getpwnam( thisUser );
            if (pw)
                homeDir = pw->pw_dir;
        }
    }
    else
    {
	/* some other user */
	pw = getpwnam( user );
	if (pw)
	    homeDir = pw->pw_dir;
    }
    return homeDir;
}

/*  -- Load Resource File - return TRUE if file was actually loaded
*******************************************************************************
    This function loads specified resource file into application resource 
    database. It allows to load resources on as-needed basis, reducing the 
    intitial resource file load overhead.  The directory search for the file 
    (should be) the same as for application class resource file.
    
    To prevent repeated loads of the same file, the function keeps track of 
    each filename.  Note that I do not allow a file to be re-loaded even if 
    it is changed, or if a new file of the same name appears on the search 
    path.  This was done for two reasons: first, it makes the code more 
    portable, as I don't have to depend upon various system calls.  Second, 
    resources can't be un-written, so a user might get the wrong impression 
    that a resource specification can be deleted, and the resource file 
    re-loaded, and something will happen.  It just isn't so.

    NOTE:  
    If XtR4 is not available, then the file search list rule used here is a 
    gross simplification of the R3 resource file search mechanism, without the
    $LANG provision.  The algorithm looks into two directories only, which may 
    be defined as environmental variables:
         XAPPLOADDIR  - defaults to /usr/lib/X11/app-defaults
	 XAPPLRESDIR  - defaults to HOME directory
*/

int WcLoadResourceFile ( w,  name )
    Widget w;
    char*  name;	/* X resources file name */
{
    XrmQuark	filenameQuark;
    XrmDatabase	rdb;

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
    Screen*	correlation;
    XrmDatabase dbUsedByXt;
#else
    Display*	correlation;
    XrmDatabase	dbUsedByXt;
#endif

    /*
     * Hack to prevent core-dumps if someone tries to pass us a display
     * as the first parameter (i.e., they think they are using Wcl 1.x)
     * When they re-compile, instead of just re-link, they will figure it
     * out (or on a Sun, when they run lint or Saber - whoops - Centerline),
     * so no need to give warning message.  The problem is that per-screen
     * resources are not *exactly* correct in the rare case of someone
     * having 2 kinds of screens on one display.
     */
    if ( w != w->core.self )
    {
	w = WcRootWidget(0);
    }

    /*  -- check for repeated load, remember the first time.
    */
#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
    correlation = XtScreenOfObject( w );
#else
    correlation = XtDisplay( w );
#endif
    filenameQuark = XrmStringToQuark( name );
    if ( MapAg_Find( rfAgent, correlation, filenameQuark, NULL ) )
	return 0;	/* already loaded this file - OK, not an error */

    /*  -- Remember we have loaded (or tried to load) this file.
    */
    MapAg_Define( rfAgent, correlation, filenameQuark, NULL, 1 );

    /* Check for garbled pathname (missing or too long)
    */
    if ( WcNull( name ) )
    {
	WcWARN(	w, "WcLoadResourceFile", "noFileName",
		"WcLoadResourceFile() - No file name provided." );
        return 0;
    }
    if (WcStrLen(name) >= (int)MAX_PATHNAME)
    {
	WcWARN1( w, "WcLoadResourceFile", "tooLong",
		"WcLoadResourceFile( %s ) - File name too long.", name );
	return 0;
    }

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
    dbUsedByXt = XtScreenDatabase( correlation );
#else
    dbUsedByXt = correlation->db;
#endif

    /*  -- See if filename is an absolute pathname from root `/' 
    */
    if ( '/' == name[0] )
    {
	if ((rdb = XrmGetFileDatabase( name )) != NULL )
	{
	    XrmMergeDatabases( rdb, &dbUsedByXt );
	    return 1;
	}
	else
	{
	    WcWARN1( w, "WcLoadResourceFile", "notReadable",
"WcLoadResourceFile( %s ) - File not found, not readable, or wrong type.",
		   name );
	    return 0;
	}
    }

    /*  -- See if filename is a pathname from tilda
    */
    if ( '~' == name[0] )
    {
	char* homeDir;
	char  path[ MAX_PATHNAME ];
	char  user[ MAX_PATHNAME ];
	char* from = &name[1];		/* skip the tilda */
	char* to   = &user[0];

	/* Copy user's name into user[].  I.e., all characters between
	** the `~' and the first `/`.  This may mean no characters (a NUL)
	** which means we will need to look up the user's name.  We will
	** not overflow the user[] array because we already checked the 
	** lenght of name at the beginning.  `from' will point into `name'
	** after the `~user', right at the `/`
	*/
	while (*from && *from != '/')
	    *to++ = *from++;
	*to = '\0';

	homeDir = WcHomeDirectory( user );

	if( WcStrLen(homeDir) + WcStrLen(from) >= MAX_PATHNAME )
	{
	    WcWARN3( (Widget)0, "WcLoadResourceFile", "tooLongExpanded",
		"WcLoadResourceFile( %s ) - Expanded name too long: %s%s", 
		name, homeDir, from );
	    return 0; 
	}

	WcStrCpy( path, homeDir );
	WcStrCat( path, from );

	if ((rdb = XrmGetFileDatabase( path )) != NULL )
	{
	    XrmMergeDatabases (rdb, &dbUsedByXt );
	    return 1;
	}
        else
        {
	    WcWARN2( (Widget)0, "WcLoadResourceFile", "notReadable",
		"WcLoadResourceFile( %s ) - Cannot read %s", name, path );
	    return 0;
        }
    }

    /*  -- Look for file in current working directory.
    **     Note this handles when name begins with `.'
    */

    if ((rdb = XrmGetFileDatabase(name)) != NULL )
    {
        XrmMergeDatabases (rdb, &dbUsedByXt );
        return 1;
    }


#ifdef XtSpecificationRelease
    {
	/*
	 * Use XUSERFILESEARCHPATH, user's home directory, and XAPPLRESDIR 
	 * in the same way that Xt does when it gets user's application 
	 * defaults.  This code basically mimics GetAppUserDefaults() in 
	 * mit/lib/Xt/Initialize.c of the X11R4 distribution, or 
	 * CombineAppUserDefaults() in the same file of X11R5 distribution.
	 * The main difference is that this routine uses the filename
	 * argument `name' instead of the application class name.
	 */
	char* filename;
	char* path;
	char* xUserFileSearchPath = getenv("XUSERFILESEARCHPATH");
	int   deallocate = 0;

	if (!xUserFileSearchPath)
	{
	    char* homeDir = WcHomeDirectory( "~" );
	    char* xApplResDir;
	    if ( !(xApplResDir = getenv("XAPPLRESDIR")) )
	    {
#if XtSpecificationRelease > 4
		char* defaultPath = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
#else
		char* defaultPath = "%s/%%L/%%N%:%s/%%l/%%N%:%s/%%N%:%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
#endif
		if ( !(path = XtMalloc(	6*WcStrLen(homeDir) +
					WcStrLen(defaultPath) )))
		{
		    WcWARN( w, "WcLoadResourceFile", "mallocFailed",
				"WcLoadResourceFile() - malloc failed");
		    return 0;
		}
		sprintf( path, defaultPath, 
			homeDir, homeDir, homeDir, homeDir, homeDir, homeDir );
	    }
	    else
	    {
#if XtSpecificationRelease > 4
		char* defaultPath = "%s/%%L/%%N%%C:%s/%%l/%%N%%C:%s/%%N%%C:%s/%%N%%C:%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
#else
		char* defaultPath = "%s/%%L/%%N%:%s/%%l/%%N%:%s/%%N%:%s/%%N%:%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
#endif
		if ( !(path = XtMalloc( 6*WcStrLen(xApplResDir) +
					2*WcStrLen(homeDir) +
					WcStrLen(defaultPath) ) ) )
		{
		    WcWARN( w, "WcLoadResourceFile", "mallocFailed",
				"WcLoadResourceFile() - malloc failed");
		    return 0;
		}
		sprintf( path, defaultPath,
				xApplResDir, xApplResDir, xApplResDir,
				homeDir,
				xApplResDir, xApplResDir, xApplResDir,
				homeDir );
	    }
	    deallocate = 1;
	}

	filename = XtResolvePathname(XtDisplay(w), 0, 0, 0, path, 0, 0, 0);

	if ( filename )
	{
	    if ((rdb = XrmGetFileDatabase(filename)) != NULL )
		XrmMergeDatabases( rdb, &dbUsedByXt );
	    XtFree( filename );
	    if (deallocate) XtFree( path );
	    return 1;
	}
    }

#else
    {
	/* For pre XtR4 (Motif 1.0), do something really simple and stupid.  
	** Look for the file in XAPPLOADDIR, the site defined directory for
	** application defaults (commonly /usr/lib/X11/app-defaults), and
	** look for the file in XAPPLRESDIR which is defined in the user's 
	** environment.
	*/
#ifndef XAPPDIR
#define XAPPDIR "/usr/lib/X11/app-defaults"
#endif

	char  filename[ MAX_PATHNAME ];
	char* sysResDir = XAPPDIR;
	char* usrResDir = getenv("XAPPLRESDIR");
	int found = 0;

        if( ( WcStrLen(sysResDir) + WcStrLen(name) >= MAX_PATHNAME )
	 || ( WcStrLen(usrResDir) + WcStrLen(name) >= MAX_PATHNAME ) )
        {
	    WcWARN1( (Widget)0, "WcLoadResourceFile", "tooLongExpanded",
"WcLoadResourceFile( %s ) Failed - Name too long when expanded.", name );
            return 0;
        }

	if ( WcNonNull(sysResDir) )
	{
	    WcStrCpy( filename, sysResDir );
	    WcStrCat( filename, "/" );
	    WcStrCat( filename, name );

            if ((rdb = XrmGetFileDatabase(filename)) != NULL )
	    {
	        XrmMergeDatabases (rdb, &dbUsedByXt );
		found++;
	    }
	}
	if ( WcNonNull(usrResDir) )
	{
	    WcStrCpy( filename, usrResDir );
	    WcStrCat( filename, name );

            if ((rdb = XrmGetFileDatabase(filename)) != NULL )
	    {
	        XrmMergeDatabases (rdb, &dbUsedByXt );
		found++;
	    }
	}
	if (found)
	    return 1;
    }
#endif

    WcWARN1( (Widget)0, "WcLoadResourceFile", "fileNotFound", 
	   "WcLoadResourceFile(%s) - Failed - file not found.", name );
    return 0;
}

/*  -- Load resource files specified in Wcl record
*******************************************************************************
    Called by WcInitialize(), this loads all the resource files specified
    by the wclResFiles resource returning True if any file was loaded.
*/
int WcMoreResourceFilesToLoad( root, wcl )
    Widget	root;
    WclRecPtr	wcl;
{
    char* wcl_resFiles_copy;
    char* next;
    char  cleanName[MAX_XRMSTRING];
    int   loadedSomething = 0;

#if defined(XtSpecificationRelease) && XtSpecificationRelease > 4
    Screen*	correlation = XtScreenOfObject( root );
#else
    Display*	correlation = XtDisplay( root );
#endif
    
    /* Load all files specified by wcl->resFiles
    */
    if ( !MapAg_Find( rfAgent, NULL, correlation, wcl->resFiles ) )
    {
	/* We have not already loaded this set of resource files
	*/
	MapAg_Define( rfAgent, NULL, correlation, wcl->resFiles, 1 );

	/* Make copy of wcl->resFiles, as WcLoadResourceFile() may well 
	** overwrite (and XtFree()) wcl->resFiles.
	*/
	wcl_resFiles_copy = XtNewString( wcl->resFiles );
	next = WcCleanName( wcl_resFiles_copy, cleanName );

	while ( cleanName[0] != '\0' )
	{
	    /* We still may not need to load any given file
	    */
	    loadedSomething += WcLoadResourceFile( root, cleanName );

	    next = WcSkipWhitespace_Comma( next );
	    next = WcCleanName( next, cleanName );
	}
	XtFree( wcl_resFiles_copy );
    }
    return loadedSomething;
}
