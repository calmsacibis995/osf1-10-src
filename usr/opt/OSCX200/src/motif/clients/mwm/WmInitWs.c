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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: WmInitWs.c,v $ $Revision: 1.1.4.6 $ $Date: 1993/11/10 21:46:57 $"
#endif
#endif
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

/*
 * Included Files:
 */

#ifdef DEC_MOTIF_BUG_FIX
#include <stdlib.h>
#endif /* DEC_MOTIF_BUG_FIX */

#include "WmGlobal.h"
#include "WmResNames.h"
#include "WmICCC.h"
#include <X11/Xos.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <Xm/XmP.h>     /* just for XmRegisterConverters decl */
#include <X11/Shell.h>
#include <X11/Core.h>
#ifdef DEC_MOTIF_EXTENSION
#include <X11/ShellP.h>                                                      
#include <Xm/XmosP.h>
#endif
#ifndef VMS
#include <fcntl.h>
#endif

/*
 * include extern functions
 */
#include "WmCDInfo.h"
#include "WmColormap.h"
#include "WmError.h"
#include "WmEvent.h"
#include "WmFeedback.h"
#include "WmFunction.h"
#include "WmIDecor.h"
#include "WmIPlace.h"
#include "WmIconBox.h"
#include "WmKeyFocus.h"
#include "WmManage.h"
#include "WmMenu.h"
#include "WmProperty.h"
#include "WmResCvt.h"
#include "WmResource.h"
#include "WmSignal.h"
#include "WmProtocol.h"
#include "WmCDecor.h"
#include "stdio.h"
#include "WmResParse.h"

/*
 * Function Declarations:
 */

#ifdef _NO_PROTO
void CopyArgv ();
#ifdef DEC_MOTIF_EXTENSION
void WmInitXuiDele();
#endif 
void InitWmGlobal ();
#ifdef DEC_MOTIF_EXTENSION
void WmInitResDb();
void WmInitResDbMerge();
void WmInitResUserGet();
void WmInitResSysGet();
#endif
void InitWmScreen ();
void InitScreenNames ();
void InitWmWorkspace ();
void ProcessMotifWmInfo ();
void SetupWmWorkspaceWindows ();
void MakeWmFunctionResources ();
void MakeWorkspaceCursors ();
void MakeXorGC ();
#else /* _NO_PROTO */
#ifdef DEC_MOTIF_EXTENSION
void WmInitXuiDele( int screen );
#endif 
void InitWmGlobal (int argc, char *argv [], char *environ []);
#ifdef DEC_MOTIF_EXTENSION
void WmInitResDb( WmScreenData *pSD, int sNum);
void WmInitResDbMerge( WmScreenData *pSD );
void WmInitResSysGet( String name, String *filename );
void WmInitResUserGet( String name, String *filename );
#endif
void InitWmScreen (WmScreenData *pSD, int sNum);
void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD);
void ProcessMotifWmInfo (Window rootWindowOfScreen);
void SetupWmWorkspaceWindows (void);
void MakeWorkspaceCursors (void);
void MakeWmFunctionResources (WmScreenData *pSD);
void MakeXorGC (WmScreenData *pSD);
void CopyArgv (int argc, char *argv []);
void InitScreenNames (void);
#endif /* _NO_PROTO */

/*
 * Global Variables:
 */
extern int firstTime;
#ifdef DEC_MOTIF_EXTENSION
extern char *getenv();
#define MAXPATH 1023
externaldef( $DATA ) String mwm_user_def_res_file = NULL; 
externaldef( $DATA ) String mwm_user_bw_res_file = NULL; 
externaldef( $DATA ) String mwm_user_gray_res_file = NULL;   
externaldef( $DATA ) String mwm_sys_def_res_file = NULL; 
externaldef( $DATA ) String mwm_sys_bw_res_file = NULL; 
externaldef( $DATA ) String mwm_sys_gray_res_file = NULL;   
#endif


#ifdef DEC_MOTIF_EXTENSION
/******************************<->*************************************
 *
 *  WmInitMonitor()
 *
 *  Description:
 *  -----------
 *  Initialize the monitor information.
 *
 *  Outputs:
 *  -------
 *  wmGD = (initialize the global data structure)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO    
void WmInitMonitor()

#else /* _NO_PROTO */
void WmInitMonitor()
#endif /* _NO_PROTO */

{
/* local variables */
int scr, type;                  

/********************************/

    wmGD.multicolor = FALSE;
    type = wmGD.Screens[ 0 ].monitor; 
    wmGD.main_monitor = type;
    /* Is this a multihead system with many colors */
    if ( wmGD.numScreens > 1 )
      {
        type = wmGD.Screens[ 0 ].monitor; 
        /* Test for multicolor */
        for ( scr = 1; scr < wmGD.numScreens; scr++ )
          {
            /* Are the monitors different types ? */
            if ( wmGD.Screens[ scr ].monitor != type )
              /* Yes, don't bring up the color customization. */
              {
                wmGD.multicolor = TRUE;
                break;
              }
          }
        /* Reset the main monitor type to color or gray-scale */
        for ( scr = 1; scr < wmGD.numScreens; scr++ )
          {
            /* Set the main monitor type.
               This is used to limit color customization
               for dual-heads to only the main display. */
            type = wmGD.Screens[ scr ].monitor; 
            switch ( type )
              {
                case k_mwm_gray_type:
                case k_mwm_bw_type:
                  /* Was it color or gray-scale before ? */
                  if (( wmGD.main_monitor != k_mwm_color_type ) &&
                      ( wmGD.main_monitor != k_mwm_gray_type ))
                      /* No, reset the type. */
                      wmGD.main_monitor = type;
                  break;
                case k_mwm_color_type:
                  /* It's color */
                  wmGD.main_monitor = type;
                  break;
                default: break;
              }
          }
          
      }

}

#endif /* DEC_MOTIF_EXTENSION */
#ifdef DEC_MOTIF_EXTENSION

/******************************<->*************************************
 *
 *  WmInitXuiDele( screen )
 *                                                    
 *  Description:
 *  -----------
 *  Delete the XUI window manager property on the root window
 *  if it exists.  In this way, applications can tell it
 *  if the XUI window manager is running
 *
 *  Inputs:
 *  ------
 *  screen = screen number
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmInitXuiDele( screen )

int screen;

#else /* _NO_PROTO */
void WmInitXuiDele( int screen )
#endif /* _NO_PROTO */

/* local variables */
{
Atom DEC_WM_DECORATION_GEOMETRY = None;
Atom aid;
int format = None;
unsigned long items, items_after;
unsigned char *prop;                                        

/********************************/

    DEC_WM_DECORATION_GEOMETRY = XInternAtom( DISPLAY,
                                     "DEC_WM_DECORATION_GEOMETRY", FALSE );
    /* Get the property */
    XGetWindowProperty( DISPLAY, RootWindow( DISPLAY, screen ),
                        DEC_WM_DECORATION_GEOMETRY,
                        0, 1, FALSE, AnyPropertyType, &aid,
                        &format, &items, &items_after, &prop );
               
    /* Was it found ? */
    if ( aid != None)
        /* Remove the XUI window manager property so that
           applications can determine that the XUI window
           manager is not running. */
       XDeleteProperty( DISPLAY, RootWindow( DISPLAY, screen ), aid );

}

/******************************<->*************************************
 *
 *  WmInitResSysGet( name, filename )
 *                                                    
 *  Description:
 *  -----------
 *  Get the resource file for the system resources.
 *
 *  Inputs:
 *  ------
 *  name = resource name
 *  filename = resource file name
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmInitResSysGet( name, filename )

String name;
String *filename;

#else /* _NO_PROTO */
void WmInitResSysGet( String name, String *filename )
#endif /* _NO_PROTO */

/* local variables */
{

/********************************/

    /* Was the filename already set? */
    if ( filename && (*filename != NULL ))
        /* Yes, done */   
        return;
#ifdef VMS
#define k_mwm_sys_path "DECW$SYSTEM_DEFAULTS:"
#define k_mwm_sys_ext ".DAT"                                                
    *filename = XtMalloc( strlen( k_mwm_sys_path ) + strlen( name ) + 
                          strlen( k_mwm_sys_ext ) + 1 );
    (void) strcpy( *filename, k_mwm_sys_path );
    (void) strcat( *filename, name  ); 
    (void) strcat( *filename, k_mwm_sys_ext );  
#else
    /* Is this the mwm configuration file ? */
    if ( strstr( name, "rc" ) == NULL )
      /* No, it's a resource file, search in app-defaults. */
      *filename = XtResolvePathname( DISPLAY, "app-defaults",
         		             name, NULL, NULL, NULL, 0, NULL);
      /* Yes, search in X11. */
      else *filename = XtResolvePathname( DISPLAY, "",
          				  name, NULL, NULL, NULL, 0, NULL);
#endif /* VMS */

}

/******************************<->*************************************
 *
 *  WmInitResUserGet( name, filename )
 *                                                    
 *  Description:
 *  -----------
 *  Get the resource file for the user's resources.
 *
 *  Inputs:
 *  ------
 *  name = resource name
 *  filename = resource file name
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmInitResUserGet( name, filename )

String name;
String *filename;

#else /* _NO_PROTO */
void WmInitResUserGet( String name, String *filename )
#endif /* _NO_PROTO */

/* local variables */
{
char *path;
Boolean deallocate = False;

/********************************/

    /* Was the filename already set? */
    if ( filename && (*filename != NULL ))
        /* Yes, done */   
        return;
#ifdef VMS
#define k_mwm_user_path "DECW$USER_DEFAULTS:"
#define k_mwm_user_ext ".DAT"                                                
    *filename = XtMalloc( strlen( k_mwm_user_path ) + strlen( name ) + 
                          strlen( k_mwm_user_ext ) + 1 );
    (void) strcpy( *filename, k_mwm_user_path );
    (void) strcat( *filename, name  ); 
    (void) strcat( *filename, k_mwm_user_ext );  
#else
    if ((path = getenv("XUSERFILESEARCHPATH")) == NULL)
      {
        String old_path;
#ifndef MOTIF_ONE_DOT_ONE
	char *homedir = (char *)_XmOSGetHomeDirName();
#else
	char homedir[MAXPATH];
	GetHomeDirName(homedir);
#endif /* MOTIF_ONE_DOT_ONE */
	if ((old_path = getenv("XAPPLRESDIR")) == NULL) {
	    char *path_default = "%s/%%L/%%N:%s/%%l/%%N:%s/%%N";
            path = NULL;
 	    if ( ! mwm_alloc( &path, 3*strlen(homedir) + strlen(path_default) +1,
                     "Error allocating path when getting user resource file" ))
                return;
	    sprintf( path, path_default, homedir, homedir, homedir );
	} else {
	    char *path_default = "%s/%%L/%%N:%s/%%l/%%N:%s/%%N:%s/%%N";
	    if ( ! mwm_alloc( &path, 3*strlen(old_path)
				     + strlen(homedir) + strlen(path_default)+1,
                     "Error allocating path when getting user resource file" ))
                return;
	    sprintf(path, path_default, old_path, old_path, old_path, homedir);
	}
	deallocate = True;
    }

    *filename = XtResolvePathname(DISPLAY, NULL, name, NULL, path, NULL, 0, NULL);
#endif /* VMS */
    if (deallocate) free( path );

}

#endif /* DEC_MOTIF_EXTENSION */
/******************************<->*************************************
 *
 *  InitWmGlobal (argc, argv, environ)
 *
 *
 *  Description:
 *  -----------
 *  This function initializes the workspace manager.
 *
 *
 *  Inputs:
 *  ------
 *  argc = number of command line arguments (+1)
 *
 *  argv = window manager command line arguments
 *
 *  environ = window manager environment
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (initialize the global data structure)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void InitWmGlobal (argc, argv, environ)
    int argc;
    char *argv[];
    char *environ[];

#else /* _NO_PROTO */
void InitWmGlobal (int argc, char *argv [], char *environ [])
#endif /* _NO_PROTO */
{
    XSetWindowAttributes sAttributes;
#ifdef VMS
    Window *windowP;
    int actual_format;
    Atom actual_type;
    unsigned long nitems,numbytes;
    Boolean delbit;      
#endif
#ifdef DEC_MOTIF_EXTENSION
    long type;
    Widget wid;
#ifndef MOTIF_ONE_DOT_ONE
	char *debug_file_name = (char *)_XmOSGetHomeDirName();
#else
	char debug_file_name[MAXPATH] = 
#endif

#ifndef VMS
char lang[ 32 ];
#else
XrmDatabase database;
#endif
#endif
    int scr;
    int managed = 0;
    char pch[80];
    Boolean activeSet = False;
    Boolean processedGlobalResources = False;
    WmScreenData *pSD;
    Arg args[11];
    int argnum;
    char *res_class;
    wmGD.errorFlag = False;

#ifdef DEC_MOTIF_BUG_FIX
    wmGD.transientLost = NULL;
#endif
#ifdef _NO_PROTO
    SetupWmSignalHandlers ();
#else
    SetupWmSignalHandlers (0); /* dummy paramater */
#endif
#ifdef DEC_MOTIF_EXTENSION
    wmGD.waitCursor = (Cursor)NULL;
    wmGD.dialog_display = NULL;
    /* debugging information */
    wmGD.debug = getenv( "MWM_DEBUG" ) != NULL;    
    if ( wmGD.debug )
      {
#ifdef VMS                                 
        strcpy( debug_file_name, mwm_debug_file_name );
#else
#ifdef MOTIF_ONE_DOT_ONE
	GetHomeDirName(debug_file_name);
#endif /* MOTIF_ONE_DOT_ONE */
        strcat( debug_file_name, "/" );
        strcat( debug_file_name, mwm_debug_file_name );
#endif       
        if ( ( wmGD.debug_file = fopen( debug_file_name, "w" )) == NULL )
            wmGD.debug = FALSE;
        if ( wmGD.debug )
            fprintf( wmGD.debug_file, "Mwm init started\n" );
      }
#endif /* DEC_MOTIF_EXTENSION */

    /*
     * Do (pre-toolkit) initialization:
     */                     

    wmGD.windowContextType = XUniqueContext ();
    wmGD.screenContextType = XUniqueContext ();

    /* copy argv (the XtInititalize chan ges the original) for use in restart */
    CopyArgv (argc, argv);

    wmGD.environ = environ;

    /* set our name */
#ifdef SUN
    if (wmGD.mwmName = (char*)rindex (wmGD.argv[0], '/'))
#else
    if (wmGD.mwmName = (char*)strrchr (wmGD.argv[0], '/'))
#endif /* SUN */
    {
        wmGD.mwmName++;
    }
    else
    {
        wmGD.mwmName = wmGD.argv[0];
    }
    res_class = WM_RESOURCE_CLASS;

    wmGD.display = NULL;


    /*
     * Do X Tookit initialization:
     *
     */   

    XtToolkitInitialize();
    XmRegisterConverters ();

#ifdef VMS
    /* Set the resource file to DECW$MWM.DAT without changing
       the resource names. */
     _XtSetDECApplication( "DECW$MWM" );
#endif /* VMS */

    wmGD.mwmAppContext = XtCreateApplicationContext();
    wmGD.display = XtOpenDisplay (wmGD.mwmAppContext,
				  NULL,
				  wmGD.mwmName,
				  res_class,
				  NULL,
				  0,
				  &argc, /* R5 changed from Cardinal to int*/
				  argv);
    
    if (!wmGD.display)
    {
	Warning("Could not open display.");
	exit (WM_ERROR_EXIT_VALUE);
    }
#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug ) 
        fprintf( wmGD.debug_file, "Toolkit inited \n" );
#endif
#ifdef VMS
    /* Initialize the process for VMS */
    vms_proc_init();
#endif /* vms */
    /*      
     * Setup error handling:
     */

    WmInitErrorHandler(wmGD.display);

    /*
     * Initialize cursor size info and 
     * display the startup cursor.
     */
    
    InitCursorInfo ();
    ShowWaitState (TRUE);
        
    /*
     * Set up the _MOTIF_BINDINGS property on the root window
     * of screen 0.  Must do this before we create shells.
     */
    
    ProcessMotifBindings ();
    
    argnum = 0;
    XtSetArg (args[argnum], XtNgeometry, NULL);	argnum++;
    XtSetArg (args[argnum], XtNx, 10000);	argnum++;
    XtSetArg (args[argnum], XtNy, 0);		argnum++;
    XtSetArg (args[argnum], XtNwidth, 10);	argnum++;
    XtSetArg (args[argnum], XtNheight, 10);	argnum++;

    /* create topmost shell (application shell) */
    wmGD.topLevelW = XtAppCreateShell (NULL, 
			      res_class,
			      applicationShellWidgetClass,
			      DISPLAY,
			      args,
			      argnum);
                                                    
    /* allocate namespace for screens */
    InitScreenNames();
    
    /* 
     * Determine the screen management policy (all or none)
     * Process command line arguments that we handle 
     * This could change the number of screens we manage 
     */
    ProcessGlobalScreenResources ();
    ProcessCommandLine (argc, argv);

                
    /*                   
     * Allocate data and initialize for screens we manage:
     */

    if (!(wmGD.Screens = (WmScreenData *) 
	    XtMalloc (wmGD.numScreens * sizeof(WmScreenData))))
    {
	ShowWaitState (FALSE);
	Warning ("Insufficient memory for Screen data");
	exit (WM_ERROR_EXIT_VALUE);
    }
    else 
    {

#ifdef DEC_MOTIF_EXTENSION
        /* Setup the black and white and gray-scale resource names. */
        WmInitResUserGet( mwm_bw_res_name, &mwm_user_bw_res_file );
        WmInitResUserGet( mwm_gray_res_name, &mwm_user_gray_res_file );
        WmInitResSysGet( mwm_bw_res_name, &mwm_sys_bw_res_file );
        WmInitResSysGet( mwm_gray_res_name, &mwm_sys_gray_res_file );
        /* Merge in the b&w and gray-scale databases for non-multicolor systems. */
        if ( ONE_SCREEN )
          {
            /* Yes, update the screen type for this screen. */
            wid = (Widget)wmGD.topLevelW;
            WmInitResDb( &wmGD.Screens[ 0 ], WID_SCREEN_NUM );
            WmInitResDbMerge( &wmGD.Screens[ 0 ] );
          }
#endif /* DEC_MOTIF_EXTENSION */
	sAttributes.event_mask = SubstructureRedirectMask;

	for (scr=0; scr<wmGD.numScreens; scr++) 
	{
	    int sNum;
	    
	    /* 
	     * Gain control of the root windows of each screen:
	     */

	    sNum = (wmGD.numScreens == 1) ? DefaultScreen(DISPLAY) : scr;
	    wmGD.errorFlag = False;

	    XChangeWindowAttributes (DISPLAY, RootWindow (DISPLAY, sNum), 
		CWEventMask, &sAttributes);
	    /*
	     * Do XSync to force server action and catch errors
	     * immediately.
	     */
	    XSync (DISPLAY, False /* do not discard events */);

	    if (wmGD.errorFlag)
	    {
		sprintf(pch, 
		    "Another window manager is running on screen %d", sNum);
		Warning ((char *) &pch[0]);
		wmGD.Screens[scr].managed = False;
	    }
	    else 
	    {
		if (!processedGlobalResources)
		{
#ifndef NO_SHAPE
		    wmGD.hasShape = XShapeQueryExtension (DISPLAY,
							  &wmGD.shapeEventBase,
							  &wmGD.shapeErrorBase);
#endif /*  NO_SHAPE  */

#ifdef DEC_MOTIF_BUG_FIX
                    wmGD.replayEnterEvent = False;
#endif
		    wmGD.menuActive = NULL;
		    wmGD.menuUnpostKeySpec = NULL;
		    wmGD.F_NextKeySpec = NULL;
		    wmGD.F_PrevKeySpec = NULL;
		    wmGD.passKeysActive = False;
		    wmGD.passKeysKeySpec = NULL;
		    wmGD.checkHotspot = False;
		    wmGD.configAction = NO_ACTION;
		    wmGD.configPart = FRAME_NONE;
		    wmGD.configSet = False;
		    wmGD.preMove = False;
		    wmGD.gadgetClient = NULL;
		    wmGD.wmTimers = NULL;
#ifdef DEC_MOTIF_EXTENSION
/* For internationalization */
		    wmGD.clientDefaultTitle = 
                        XmStringCreate( DEFAULT_CLIENT_TITLE,
		                        XmFONTLIST_DEFAULT_TAG);
		    wmGD.iconDefaultTitle = 
                        XmStringCreate( DEFAULT_ICON_TITLE,
					XmFONTLIST_DEFAULT_TAG);
                    wmGD.multicolor = False;
                    wmGD.main_monitor = 0;
                    wmGD.mode_switch = 0;
#else
		    wmGD.clientDefaultTitle = 
			XmStringCreateLtoR(DEFAULT_CLIENT_TITLE,
					   XmFONTLIST_DEFAULT_TAG);
		    wmGD.iconDefaultTitle = 
			XmStringCreateLtoR(DEFAULT_ICON_TITLE,
					   XmFONTLIST_DEFAULT_TAG);
#endif /* DEC_MOTIF_EXTENSION */
		    wmGD.attributesWindow = (Window)NULL;
		    wmGD.clickData.pCD = NULL;
		    wmGD.clickData.clickPending = False;
	  	    wmGD.clickData.doubleClickPending = False;
		    wmGD.systemModalActive = False;
		    wmGD.activeIconTextDisplayed = False;
		    wmGD.movingIcon = False;
		    wmGD.queryScreen = True;
		    wmGD.dataType = GLOBAL_DATA_TYPE;
		    

		    /* 
		     * if this is the first screen we can manage, 
		     * process global.
		     */
		    
		    processedGlobalResources = True;

		    /*
		     * Get the _MOTIF_WM_INFO property and determine 
		     * the startup / restart state.
		     */
		    
		    ProcessMotifWmInfo (RootWindow (DISPLAY, sNum));
		    
		    /*
		     * Process global window manager resources:
		     */
		    
		    AddWmResourceConverters ();
		    ProcessWmResources ();

		}
		
		InitWmScreen (&(wmGD.Screens[scr]), sNum);
		wmGD.Screens[scr].managed = True;
#ifdef DEC_MOTIF_EXTENSION
                /* Remove DEC_WM_DECORATION_GEOMETRY property if it exists. */
                WmInitXuiDele( sNum );
#endif /* DEC_MOTIF_EXTENSION */
		managed++;
           
		if (!activeSet) 
		{
		    activeSet = True;
		    ACTIVE_PSD = &wmGD.Screens[scr];
		}
	    }
	}

	if (managed == 0) 
	{
	    /*
	     * No screens for me to manage, give up.
	     */
	    ShowWaitState (FALSE);
	    Warning ("Unable to manage any screens on display.");
	    exit (WM_ERROR_EXIT_VALUE);
	}
    }

#ifndef VMS
    /*
     * Prepare to have child processes (e.g., exec'ed commands).
     * The X connection should not be passed on to child processes
     * (it should be automatically closed when a fork is done).
     */

    if (fcntl (ConnectionNumber (DISPLAY), F_SETFD, 1) == -1)
    {          
	ShowWaitState (FALSE);
	Warning ("Cannot configure X connection");
	exit (WM_ERROR_EXIT_VALUE);
    }
#endif


#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug )
        fprintf( wmGD.debug_file, "resources inited\n" );
#endif
    /*
     * Make the window manager workspace window.
     * Setup the _MOTIF_WM_INFO property on the root window.
     */

    SetupWmWorkspaceWindows ();


    /* make the cursors that the window manager uses */
    MakeWorkspaceCursors ();

    /* Sync the table used by Mwm's modifier parser to actual modMasks used */
    SyncModifierStrings();

#ifdef DEC_MOTIF_EXTENSION
    /* Initialize the monitor type . */
    WmInitMonitor();
    /* Merge non-multicolor systems with several screens. */
    if ( !wmGD.multicolor && !ONE_SCREEN )
        WmInitResDbMerge( &wmGD.Screens[ 0 ] );
#endif /* DEC_MOTIF_EXTENSION */

    /*
     * Setup screen data and resources (after processing Wm resources.
     */
    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);

	if (pSD->managed)
	{
	    /*
	     * Initialize workspace colormap data.
	     */

	    InitWorkspaceColormap (pSD);

	    /*
	     * Process the window manager resource description file (.mwmrc):
	     */

	    ProcessWmFile (pSD);


	    /*
	     * Setup default resources for the system menu and key bindings:
	     */

	    SetupDefaultResources (pSD);


	    /*
	     * Make global window manager facilities:
	     */

	    if(pSD->iconDecoration & ICON_ACTIVE_LABEL_PART)
	    {
		/* create active icon window */
		CreateActiveIconTextWindow(pSD); 
	    }


	    /*
	     * Make menus and other resources that are used by window manager
	     * functions that are activated by menus, buttons and keys.
	     */

	    MakeWmFunctionResources (pSD);
	}

      
    }

#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug )
        fprintf( wmGD.debug_file, "screen resources inited\n" );
    /* For internationalization, save the current mode switch */
    wmGD.mode_switch = ModeSwitchOfDisplay( wmGD.display );
#endif /* DEC_MOTIF_EXTENSION */
    /*
     * Realize the top level widget, make the window override
    /*
     * Realize the top level widget, make the window override
     * redirect so we don't manage it, and then move it out of the way
     */

    XtRealizeWidget (wmGD.topLevelW);

    sAttributes.override_redirect = True;
    XChangeWindowAttributes (DISPLAY, XtWindow (wmGD.topLevelW),
		CWOverrideRedirect, &sAttributes);


    /* setup window manager inter-client communications conventions handling */
    SetupWmICCC ();


    /*
     * Initialize window manager event handling:
     */

    InitEventHandling ();

#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug )
        fprintf( wmGD.debug_file, "event handling inited\n" );
#endif

    /*
     * Initialize frame component graphics
     */
    {
	for (scr = 0; scr < wmGD.numScreens; scr++)
	{
	    pSD = &(wmGD.Screens[scr]);

	    if (pSD->managed)
	    {
		InitClientDecoration (pSD);

		/*
		 * Make an icon box if specificed:
		 */
		if (pSD->useIconBox)
		{
		    InitIconBox (pSD);
		}

		/*
		 * Adopt client windows that exist before wm startup:
		 */

		AdoptInitialClients (pSD);

		/*
		 * Setup initial keyboard focus and colormap focus:
		 */

		InitColormapFocus (pSD);

	    }
	}

#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug )
        fprintf( wmGD.debug_file, "adopted clients\n" );
#endif
        for (scr = 0; scr < wmGD.numScreens; scr++)
        {
            pSD = &(wmGD.Screens[scr]);
	    
            if (pSD->managed)
            {
                ACTIVE_PSD = &wmGD.Screens[scr];
                MapIconBoxes (pSD->pActiveWS);
            }
        }
        firstTime = 0;
    }
             
    InitKeyboardFocus ();

    ShowWaitState (FALSE);
#ifdef AUTOMATION
	AutoInitByteOrderChar();
#endif
#ifdef DEC_MOTIF_EXTENSION
    if ( wmGD.debug )
      {
        fprintf( wmGD.debug_file, "Mwm init complete\n" );
        fflush( wmGD.debug_file );
      }
#endif

} /* END OF FUNCTION InitWmGlobal */



#ifdef DEC_MOTIF_EXTENSION
/******************************<->*************************************
 *
 *  WmInitResDb
 *
 *  Description:
 *  -----------
 *  This function initializes the resource databases for b&w and gray
 *  scale systems.
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to preallocated screen data block
 *  sNum = screen number for this screen
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmInitResDb( pSD, sNum)

WmScreenData *pSD;
int sNum;

#else /* _NO_PROTO */
void WmInitResDb( WmScreenData *pSD, int sNum)
#endif /* _NO_PROTO */

{
/* local variables */
Visual *visual;
FILE *file;

/********************************/

    pSD->reset_resources = FALSE;

    /* Is there only 1 screen ? */
    if ( ONE_SCREEN )
      /* Yes, the resources will be merged, don't open the databases */
      {
        pSD->user_database = NULL;
        pSD->sys_database = NULL;
        pSD->user_database_set = FALSE;
        pSD->sys_database_set = FALSE;
      }
      
    /* Get the database for screens other than colors */
    /* Is it B&W */
    if ( DefaultDepthOfScreen( XScreenOfDisplay( DISPLAY, sNum )) == 1 )
      {
        pSD->reset_resources = TRUE;
        pSD->monitor = k_mwm_bw_type;
        if ( ONE_SCREEN )
            return;
        pSD->reset_resources = TRUE;
        pSD->user_database = XrmGetFileDatabase( mwm_user_bw_res_file );
        pSD->sys_database = XrmGetFileDatabase( mwm_sys_bw_res_file );
      }
    /* else it it gray-scale ? */
    else
      {
        visual = XDefaultVisualOfScreen( XScreenOfDisplay( DISPLAY, sNum ));
        if (( visual->class == StaticGray ) || ( visual->class == GrayScale ))
          {
            pSD->monitor = k_mwm_gray_type;
            if ( ONE_SCREEN )
              return;
            pSD->reset_resources = TRUE;
            pSD->user_database = XrmGetFileDatabase( mwm_user_gray_res_file );
            pSD->sys_database = XrmGetFileDatabase( mwm_sys_gray_res_file );
          }
        else pSD->monitor = k_mwm_color_type;
      }

    /* Update the flags */
    if ( pSD->reset_resources )   
      {
        pSD->user_database_set = pSD->user_database != NULL;
        pSD->sys_database_set = pSD->sys_database != NULL;
        pSD->reset_resources = pSD->sys_database_set ||
                               pSD->user_database_set;
      }

}

/******************************<->*************************************
 *
 *  WmInitResDbMerge
 *
 *  Description:
 *  -----------
 *  This function Merges databases for b&w or gray-scale systems.
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to preallocated screen data block
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

#ifdef _NO_PROTO
void WmInitResDbMerge( pSD )

WmScreenData *pSD;

#else /* _NO_PROTO */
void WmInitResDbMerge( WmScreenData *pSD )
#endif /* _NO_PROTO */

{
/* local variables */
int visual;
XrmDatabase user_database, sys_database, database;

/********************************/

    /* Is this only 1 screen. */
    /* Is it B&W */
    if ( wmGD.Screens[ 0 ].monitor == k_mwm_bw_type )
      {
        /* Open the user and system database */
        user_database = XrmGetFileDatabase( mwm_user_bw_res_file );
        sys_database = XrmGetFileDatabase( mwm_sys_bw_res_file );
      }
    /* else it it gray-scale ? */
    else if ( wmGD.Screens[ 0 ].monitor == k_mwm_gray_type )
      {
        /* Open the user and system database */
        user_database = XrmGetFileDatabase( mwm_user_gray_res_file );
        sys_database = XrmGetFileDatabase( mwm_sys_gray_res_file );
      }
    else return;

    database = XtDatabase( DISPLAY );
    XrmMergeDatabases( sys_database, &database );
    XrmMergeDatabases( user_database, &database );

}

#endif
/******************************<->*************************************
 *
 *  InitWmScreen
 *
 *
 *  Description:
 *  -----------
 *  This function initializes a screen data block.
 *
 *  Inputs:
 *  -------
 *  pSD = pointer to preallocated screen data block
 *  sNum = screen number for this screen
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

#ifdef _NO_PROTO
void
InitWmScreen (pSD, sNum)
    WmScreenData *pSD;
    int sNum;
#else /* _NO_PROTO */
void
InitWmScreen (WmScreenData *pSD, int sNum)
#endif /* _NO_PROTO */
{
    Arg args[12];
    int argnum;


    char *pDisplayName;
    char buffer[256];
    char displayName[256];
    char *token1, *token2;

   /*
    * Set screen data values
    */

    pSD->rootWindow = RootWindow (DISPLAY, sNum);
    pSD->clientCounter = 0;
    pSD->defaultSystemMenuUseBuiltin = TRUE;
    pSD->displayString = NULL;
    pSD->acceleratorMenuCount = 0;
    pSD->activeIconTextWin = (Window)NULL;
    pSD->focusPriority = 0;
    pSD->inputScreenWindow = (Window)NULL;
    pSD->colormapFocus = NULL;
    pSD->keySpecs = NULL;
    pSD->screen = sNum;
    pSD->confirmboxW[DEFAULT_BEHAVIOR_ACTION] = NULL;
    pSD->confirmboxW[CUSTOM_BEHAVIOR_ACTION] = NULL;
    pSD->confirmboxW[RESTART_ACTION] = NULL;
    pSD->confirmboxW[QUIT_MWM_ACTION] = NULL;
    pSD->feedbackWin = (Window)NULL;
    pSD->fbStyle = FB_OFF;
    pSD->fbWinWidth = 0;
    pSD->fbWinHeight = 0;
    pSD->fbLocation[0] = '\0';
    pSD->fbSize[0] = '\0';
    pSD->fbLocX = 0;
    pSD->fbLocY = 0;
    pSD->fbSizeX = 0;
    pSD->fbSizeY = 0;
    pSD->fbLastX = -1;
    pSD->fbLastY = -1;
    pSD->fbLastWidth = -1;
    pSD->fbLastHeight = -1;
    pSD->fbTop = NULL;
    pSD->fbBottom = NULL;
    pSD->actionNbr = -1;
    pSD->clientList = NULL;
    pSD->lastClient = NULL;
    pSD->lastInstalledColormap = (Colormap)NULL;
    pSD->shrinkWrapGC = NULL;
    pSD->bitmapCache = NULL;
    pSD->bitmapCacheSize = 0;
    pSD->bitmapCacheCount = 0;
    pSD->dataType = SCREEN_DATA_TYPE;
    /*
     * Save screen context
     */
    XSaveContext (DISPLAY, pSD->rootWindow, wmGD.screenContextType,
	(caddr_t)pSD);
    /*
     * Create shell widget for screen resource hierarchy
     */
    
    argnum = 0;
    XtSetArg (args[argnum], XtNgeometry, NULL);	argnum++;
    XtSetArg (args[argnum], XtNx, 10000);	argnum++;
    XtSetArg (args[argnum], XtNy, 10000);	argnum++;
    XtSetArg (args[argnum], XtNwidth, 10);	argnum++;
    XtSetArg (args[argnum], XtNheight, 10);	argnum++;
    XtSetArg (args[argnum], XtNoverrideRedirect, True);	argnum++;

    XtSetArg (args[argnum], XtNdepth, 
	    DefaultDepth(DISPLAY, sNum));	argnum++;
    XtSetArg (args[argnum], XtNscreen, 
	    ScreenOfDisplay(DISPLAY, sNum)); 	argnum++;
    XtSetArg (args[argnum], XtNcolormap, 
	    DefaultColormap(DISPLAY, sNum)); 	argnum++;

    pSD->screenTopLevelW = XtCreatePopupShell ((String) wmGD.screenNames[sNum],
					       vendorShellWidgetClass,
					       wmGD.topLevelW,
					       args,
					       argnum);
#ifdef DEC_MOTIF_EXTENSION
    /* Is there only one screen ? */
    if ( !ONE_SCREEN )
        WmInitResDb( pSD, sNum );
#endif /* DEC_MOTIF_EXTENSION */
    /*
     * Fetch screen based resources
     */
    ProcessScreenResources (pSD, wmGD.screenNames[sNum]);

    /*
     * Initialize other screen resources and parameters
     */
    MakeXorGC (pSD);
    InitIconSize(pSD);

    /*
     *  Allocate and initialize a workspace structure
     */
    
    if (!(pSD->pWS = (WmWorkspaceData *) XtMalloc (sizeof(WmWorkspaceData))))
    {
	ShowWaitState (FALSE);
	Warning ("Insufficient memory for Workspace data");
	exit (WM_ERROR_EXIT_VALUE);
    }

    /*
     * Set up workspace for this screen
     */
    InitWmWorkspace (pSD->pWS, pSD);
    pSD->pActiveWS = pSD->pWS;


    pDisplayName = DisplayString (DISPLAY);

    /*
     * Construct displayString for this string.  
     */

#ifdef VMS
    sprintf(buffer, "set DISPLAY/screen=%d\n", sNum);
    {
#else
    
    strcpy(displayName, pDisplayName);

    token1 = (char*)strtok(displayName, ":");		/* parse of hostname */

    if((token2 = (char*)strtok(NULL, ".")) ||		/* parse off dpy & scr # */
       (token2 = (char*)strtok(NULL, "")) ||
       (displayName[0] == ':'))
    {
	if (displayName[0] == ':')		/* local dpy (special case) */
	{
	    if (token2 = (char*)strtok(token1, "."))	/* parse off display # */
		sprintf(buffer, "DISPLAY=:%s.%d\0",
			token2,	sNum);
	} else {				/* otherwise process normally */
   	    sprintf(buffer, "DISPLAY=%s:%s.%d",
		    token1, token2, sNum);
	}
#endif /* VMS */
	/*		
	 * Allocate space for the display string
	 */
    
	if ((pSD->displayString =
	     (String)XtMalloc ((unsigned int) (strlen(buffer) + 1))) == NULL)
	{
	    Warning ("Insufficient memory for displayString");
	}
	else
	{
	    strcpy(pSD->displayString, buffer);
	}     

    }



} /* END OF FUNCTION  InitWmScreen */


/*************************************<->*************************************
 *
 *  InitWmWorkspace
 *
 *
 *  Description:
 *  -----------
 *  This function initializes a workspace data block.
 *
 *  Inputs:
 *  -------
 *  pWS = pointer to preallocated workspace data block
 *  pSD = ptr to parent screen data block
 *
 *  Outputs:
 *  -------
 *************************************<->***********************************/

#ifdef _NO_PROTO
void InitWmWorkspace (pWS, pSD)
    WmWorkspaceData *pWS;
    WmScreenData *pSD;

#else /* _NO_PROTO */
void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    Arg args[10];
    int argnum;

#define DEFAULT_WS_NAME "workspace"

    pWS->pSD = pSD;
    pWS->pIconBox = NULL;
    pWS->dataType = WORKSPACE_DATA_TYPE;

    if ((pWS->name = (char *) 
	    XtMalloc ((1+strlen(DEFAULT_WS_NAME)) * sizeof (char))) == NULL)
    {
	ShowWaitState (FALSE);
	exit (WM_ERROR_EXIT_VALUE);
    }
    strcpy (pWS->name, DEFAULT_WS_NAME);

    /*
     * Create widget for workspace resource hierarchy
     */
    argnum = 0;
    XtSetArg (args[argnum], XtNdepth, 
	    DefaultDepth(DISPLAY, pSD->screen));	argnum++;
    XtSetArg (args[argnum], XtNscreen, 
	    ScreenOfDisplay(DISPLAY, pSD->screen)); 	argnum++;
    XtSetArg (args[argnum], XtNcolormap, 
	    DefaultColormap(DISPLAY, pSD->screen)); 	argnum++;
    XtSetArg (args[argnum], XtNwidth,  5);		argnum++;
    XtSetArg (args[argnum], XtNheight,  5);		argnum++;

    pWS->workspaceTopLevelW = XtCreateWidget (	pWS->name,
						xmPrimitiveWidgetClass,
    						pSD->screenTopLevelW,
					   	args,
						argnum);


    /*
     * Process workspace based resources
     */
    ProcessWorkspaceResources (pWS);	

    /* setup icon placement */
    if (wmGD.iconAutoPlace)
    {
	InitIconPlacement (pWS); 
    }

} /* END OF FUNCTION  InitWmWorkspace */



/*************************************<->*************************************
 *
 *  ProcessMotifWmInfo (rootWindowOfScreen)
 *
 *
 *  Description:
 *  -----------
 *  This function is used retrieve and save the information in the 
 *  _MOTIF_WM_INFO property.  If the property does not exist then
 *  the start / restart state is set to initial startup with the
 *  user specified (not standard) configuration.
 *
 *
 *  Outputs:
 *  -------
 *  wmGD.useStandardBehavior = True if set indicated in property
 *
 *  wmGD.wmRestarted = True if the window manager was restarted
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void ProcessMotifWmInfo (rootWindowOfScreen)
    Window rootWindowOfScreen;
#else /* _NO_PROTO */
void ProcessMotifWmInfo (Window rootWindowOfScreen)
#endif /* _NO_PROTO */
{
    MwmInfo *pMwmInfo;

    wmGD.xa_MWM_INFO = XInternAtom (DISPLAY, _XA_MWM_INFO, False);
    if (pMwmInfo = (MotifWmInfo *)GetMwmInfo (rootWindowOfScreen))
    {
	wmGD.useStandardBehavior =
		(pMwmInfo->flags & MWM_INFO_STARTUP_STANDARD) ? True : False;
	wmGD.wmRestarted = True;
	XFree ((char *)pMwmInfo);
    }
    else
    {
	wmGD.useStandardBehavior = False;
	wmGD.wmRestarted = False;
    }

} /* END OF FUNCTION ProcessMotifWmInfo */



/*************************************<->*************************************
 *
 *  SetupWmWorkspaceWindows ()
 *            
 *
 *  Description:
 *  -----------
 *  This function is used to setup a window that can be used in doing window
 *  management functions.  This window is not visible on the screen.
 *
 *
 *  Outputs:
 *  -------
 *  pSD->wmWorkspaceWin = window that is used to hold wm properties
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void SetupWmWorkspaceWindows ()

#else /* _NO_PROTO */
void SetupWmWorkspaceWindows (void)
#endif /* _NO_PROTO */
{
    int scr;
    WmScreenData *pSD;
    XSetWindowAttributes sAttributes;

    for (scr = 0; scr < wmGD.numScreens; scr++)
    {
	pSD = &(wmGD.Screens[scr]);
	if (pSD->managed)
	{
	    sAttributes.override_redirect = True;
	    sAttributes.event_mask = FocusChangeMask;
	    pSD->wmWorkspaceWin = XCreateWindow (DISPLAY, pSD->rootWindow, 
				      -100, -100, 10, 10, 0, 0, 
				      InputOnly, CopyFromParent,
				      (CWOverrideRedirect |CWEventMask),
				      &sAttributes);

	    XMapWindow (DISPLAY, pSD->wmWorkspaceWin);

	    SetMwmInfo (pSD->rootWindow, 
			(long) ((wmGD.useStandardBehavior) ?
                        MWM_INFO_STARTUP_STANDARD : MWM_INFO_STARTUP_CUSTOM), 
			pSD->wmWorkspaceWin);
	}
    }

} /* END OF FUNCTION SetupWmWorkspaceWindow */



/*************************************<->*************************************
 *
 *  MakeWorkspaceCursors ()
 *
 *
 *  Description:
 *  -----------
 *  This function makes the cursors that the window manager uses.
 *
 *            
 *  Inputs:
 *  ------
 *  XXinput = ...
 *
 *  XXinput = ...
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD = (stretchCursors ...)
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MakeWorkspaceCursors ()

#else /* _NO_PROTO */
void MakeWorkspaceCursors (void)
#endif /* _NO_PROTO */
{
#ifdef DEC_MOTIF_EXTENSION
    /* Why explicitly change the cursor for windows rather than
     * inheriting the cursor from the parent (root) window? Many
     * programs set the "default" cursor by defining the root window
     * cursor, such as session manager and xsetroot.
    */
    if (wmGD.useDECMode) wmGD.workspaceCursor = None;
    else
#endif
    wmGD.workspaceCursor = XCreateFontCursor (DISPLAY, XC_left_ptr);

    wmGD.stretchCursors[STRETCH_NORTH_WEST] =
	XCreateFontCursor (DISPLAY, XC_top_left_corner);
    wmGD.stretchCursors[STRETCH_NORTH] =
	XCreateFontCursor (DISPLAY, XC_top_side);
    wmGD.stretchCursors[STRETCH_NORTH_EAST] =
	XCreateFontCursor (DISPLAY, XC_top_right_corner);
    wmGD.stretchCursors[STRETCH_EAST] =
	XCreateFontCursor (DISPLAY, XC_right_side);
    wmGD.stretchCursors[STRETCH_SOUTH_EAST] =
	XCreateFontCursor (DISPLAY, XC_bottom_right_corner);
    wmGD.stretchCursors[STRETCH_SOUTH] =
	XCreateFontCursor (DISPLAY, XC_bottom_side);
    wmGD.stretchCursors[STRETCH_SOUTH_WEST] =
	XCreateFontCursor (DISPLAY, XC_bottom_left_corner);
    wmGD.stretchCursors[STRETCH_WEST] =
	XCreateFontCursor (DISPLAY, XC_left_side);

    wmGD.configCursor = XCreateFontCursor (DISPLAY, XC_fleur);

    wmGD.movePlacementCursor = XCreateFontCursor (DISPLAY, XC_ul_angle);
    wmGD.sizePlacementCursor = XCreateFontCursor (DISPLAY, XC_lr_angle);


} /* END OF FUNCTION MakeWorkspaceCursors */


                                      
/*************************************<->*************************************
 *
 *  MakeWmFunctionResources (pSD)
 *
 *
 *  Description:
 *  -----------
 *  This function makes menus and other resources that are used by window
 *  manager functions.
 *
 *
 *  Inputs:
 *  ------
 *  wmGD  = (menuSpecs, keySpecs, buttonSpecs)
 *
 * 
 *  Outputs:
 *  -------
 *  wmGD (menuSpecs) = new menu panes, protocol atoms
 *                                                      
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MakeWmFunctionResources (pSD)

    WmScreenData *pSD;

#else /* _NO_PROTO */  
void MakeWmFunctionResources (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    ButtonSpec *buttonSpec;
    KeySpec    *keySpec;
    MenuSpec   *menuSpec;
    Context     menuContext;


    /*
     * Scan through the menu specifications and make wm protocol atoms.
     */


    /*
     * Scan through the button binding specifications making menus if the
     * f.menu function is invoked.
     */

    buttonSpec = pSD->buttonSpecs;
    while (buttonSpec)
    {
	if (buttonSpec->wmFunction == F_Menu)
	{
	    if (buttonSpec->context & F_CONTEXT_WINDOW)
	    {
		menuContext = F_CONTEXT_WINDOW;
	    }
	    else if (buttonSpec->context & F_CONTEXT_ICON)
	    {
		menuContext = F_CONTEXT_ICON;
	    }
	    else
	    {
		menuContext = F_CONTEXT_ROOT;
	    }

	    menuSpec = MakeMenu (pSD, buttonSpec->wmFuncArgs, menuContext,
	                         buttonSpec->context, 
				 (MenuItem *) NULL, FALSE);

	    if (menuSpec)
	    /* 
	     * If successful, save in pSD->acceleratorMenuSpecs 
	     * Note: these accelerators have nonzero contexts.
	     */
	    {
		SaveMenuAccelerators (pSD, menuSpec);
	    }
	    else
	    {
		buttonSpec->wmFunction = F_Nop;
	    }
	}
	buttonSpec = buttonSpec->nextButtonSpec;
    }


    /*
     * Scan through the key binding specifications making menus if the
     * f.menu function is invoked.
     */

    keySpec = pSD->keySpecs;
    while (keySpec)
    {
	if (keySpec->wmFunction == F_Menu)
	{
	    if (keySpec->context & F_CONTEXT_WINDOW)
	    {
		menuContext = F_CONTEXT_WINDOW;
	    }
	    else if (keySpec->context & F_CONTEXT_ICON)
	    {
		menuContext = F_CONTEXT_ICON;
	    }
	    else
	    {
		menuContext = F_CONTEXT_ROOT;
	    }

	    menuSpec = MakeMenu (pSD, keySpec->wmFuncArgs, menuContext,
	                         keySpec->context, 
				 (MenuItem *) NULL, FALSE);

	    if (menuSpec)                   
	    /* 
	     * If successful, save in pSD->acceleratorMenuSpecs 
	     * Note: these accelerators have nonzero contexts.
	     */
	    {
		SaveMenuAccelerators (pSD, menuSpec);
	    }
	    else
	    {
		keySpec->wmFunction = F_Nop;
	    }
	}
	keySpec = keySpec->nextKeySpec;
    }


} /* END OF FUNCTION MakeWmFunctionResources */



/*************************************<->*************************************
 *
 *  MakeXorGC (pSD)
 *
 *
 *  Description:
 *  -----------
 *  Make an XOR graphic context for resizing and moving
 *
 *
 *  Inputs:
 *  ------    
 *  pSD = pointer to screen data
 * 
 *  Outputs:
 *  -------
 *  Modifies global data
 *
 *  Comments:
 *  --------
 *  
 * 
 *************************************<->***********************************/

#ifdef _NO_PROTO
void MakeXorGC (pSD)

    WmScreenData *pSD;
#else /* _NO_PROTO */
void MakeXorGC (WmScreenData *pSD)
#endif /* _NO_PROTO */
{
    XGCValues gcv;
    XtGCMask  mask;

    mask = GCFunction | GCLineWidth | GCSubwindowMode | GCCapStyle;
    gcv.function = GXinvert;
    gcv.line_width = 0;
    gcv.cap_style = CapNotLast;
    gcv.subwindow_mode = IncludeInferiors;

    /* Fix so that the rubberbanding for resize and move will
     *  have more contrasting colors.
     */

#ifdef DEC_MOTIF_BUG_FIX
    gcv.plane_mask = BlackPixelOfScreen( XScreenOfDisplay( DISPLAY, pSD->screen )) ^ 
                     WhitePixelOfScreen( XScreenOfDisplay( DISPLAY, pSD->screen ));
#else
    gcv.plane_mask = BlackPixelOfScreen( DefaultScreenOfDisplay( DISPLAY )) ^ 
                     WhitePixelOfScreen( DefaultScreenOfDisplay( DISPLAY ));
#endif
    mask = mask | GCPlaneMask;

    pSD->xorGC = XCreateGC (DISPLAY, pSD->rootWindow, mask, &gcv);


} /* END OF FUNCTION MakeXorGC */



/*************************************<->*************************************
 *
 *  CopyArgv (argc, argv)
 *
 *
 *  Description:
 *  -----------
 *  This function makes a copy of the window manager's argv for use by
 *  the f.restart function.  A copy must be kept because XtInitialize
 *  changes argv.
 *
 *
 *  Inputs:  
 *  ------
 *  argc = the number of strings in argv
 *
 *  argv = window manager parameters
 *
 * 
 *  Outputs:
 *  -------
 *  Return = a copy of argv
 *
 *************************************<->***********************************/


#ifdef _NO_PROTO
void CopyArgv (argc, argv)
    int argc;
    char *argv[];

#else /* _NO_PROTO */
void CopyArgv (int argc, char *argv [])
#endif /* _NO_PROTO */
{
    int i;


    if ((wmGD.argv = (char **)XtMalloc ((argc + 1) * sizeof (char *))) == NULL)
    {
	Warning ("Insufficient memory for window manager data");
	wmGD.argv = argv;
    }
    else
    {
	for (i = 0; i < argc; i++)
	{
	    wmGD.argv[i] = argv[i];
	}
	wmGD.argv[i] = NULL;
    }
    
} /* END OF FUNCTION CopyArgv */


/*************************************<->*************************************
 *
 *  InitScreenNames ()
 *
 *
 *  Description:
 *  -----------
 *  Initializes the name space for screen names
 *
 *  Outputs:
 *  -------
 *  Modifies global data
 *    + screenNames
 *
 *  Comments:
 *  --------
 *  Initializes screenNames to contain a numeric name for each screen
 *
 *************************************<->***********************************/

#ifdef _NO_PROTO
void InitScreenNames ()

#else /* _NO_PROTO */
void InitScreenNames (void)
#endif /* _NO_PROTO */
{
    int num, numScreens;
    
    numScreens = ScreenCount (wmGD.display);
    
    if (!(wmGD.screenNames = 
	  (unsigned char **) XtMalloc (numScreens * sizeof(char *))))
    {
	ShowWaitState (FALSE);
	Warning ("Insufficient memory for screen names");
	exit (WM_ERROR_EXIT_VALUE);
    }
    
    for (num=0; num<numScreens; num++)
    {
	if (!(wmGD.screenNames[num] = 
	      (unsigned char *) XtMalloc (4*sizeof(char))))
	{
	    ShowWaitState (FALSE);
	    Warning ("Insufficient memory for screen names");
	    exit (WM_ERROR_EXIT_VALUE);
	}
	/* default name is left justified, 3-chars max, zero terminated */
	sprintf((char *)wmGD.screenNames[num],"%d",num%1000);
    }
}
