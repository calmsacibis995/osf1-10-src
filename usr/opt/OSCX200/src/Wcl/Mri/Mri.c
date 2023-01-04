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
* SCCS_data: @(#) Mri.c 1.3 92/03/18 10:44:50
*
* Motif Resource Interpreter - Mri.c
*
* Mri.c implements a Motif Resource Interpreter which allows prototype Motif
* interfaces to be built from resource files.  The Widget Creation library is
* used.
*
******************************************************************************
*/

#include <X11/Xm/Xm.h>		/* Motif and Xt Intrinsics	*/
#include <X11/Wc/WcCreate.h>	/* Widget Creation Library	*/
#include <X11/Xmp/Xmp.h>	/* Motif Public widgets etc.	*/


/******************************************************************************
**  Private Data
******************************************************************************/

/* All Wcl applications should provide at least these Wcl options:
*/
static XrmOptionDescRec options[] = {
    WCL_XRM_OPTIONS
};


/******************************************************************************
**  Private Functions
******************************************************************************/

/*ARGSUSED*/
static void DeleteWindowCB( w, clientData, callData )
    Widget	w;
    XtPointer	clientData;
    XtPointer	callData;
{
    /* This callback is invoked when the user selects `Close' from
    ** the mwm frame menu on the upper left of the window border.
    ** Do whatever is appropriate.
    */
    printf("Closed by window manager.\n");
}

/*ARGSUSED*/
static void RegisterApplication ( app )
    XtAppContext app;
{
    /* -- Useful shorthand for registering things with the Wcl library */
#define RCP( name, class  ) WcRegisterClassPtr   ( app, name, class );
#define RCO( name, constr ) WcRegisterConstructor( app, name, constr );
#define RAC( name, func   ) WcRegisterAction     ( app, name, func );
#define RCB( name, func   ) WcRegisterCallback   ( app, name, func, NULL );

    /* -- register widget classes and constructors */
    /* -- Register application specific actions */
    /* -- Register application specific callbacks */
}


/******************************************************************************
*   MAIN function
******************************************************************************/

main ( argc, argv )
    int     argc;
    String  argv[];
{   
    /*  -- Intialize Toolkit creating the application shell
    */
    Widget appShell = XtInitialize ( 
	argv[0], WcAppClass( argc, argv ),	/* application name & class */
	options, XtNumber(options),		/* descr of cmd line options */
	&argc, argv 
    );
    XtAppContext app = XtWidgetToApplicationContext(appShell);

    /*  -- Register all application specific callbacks and widget classes
    */
    RegisterApplication ( app );

    /*  -- Register all Motif classes, constructors, and Xmp CBs & ACTs
    */
    XmpRegisterAll ( app );

    /*  -- Create widget tree below toplevel shell using Xrm database
    */
    if ( WcWidgetCreation ( appShell ) )
	exit(1);

    /*  -- Realize the widget tree
    */
    XtRealizeWidget ( appShell );

    /*  -- Optional, but frequently desired:  
    ** Provide a callback which gets invoked when the user selects `Close' from
    ** the mwm frame menu on the top level shell.  A real application will need
    ** to provide its own callback instead of DeleteWindowCB, and probably 
    ** client data too.  MUST be done after shell widget is REALIZED!  Hence,
    ** this CANNOT be done using wcCallback (in a creation time callback).
    */
    XmpAddMwmCloseCallback( appShell, DeleteWindowCB, NULL );

    /*  -- and finally, enter the main application loop
    */
    XtMainLoop ( );
}
