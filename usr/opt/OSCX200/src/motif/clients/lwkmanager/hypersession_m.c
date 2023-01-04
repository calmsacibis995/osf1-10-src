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
** COPYRIGHT (c) 1989, 1990, 1991 BY
** DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.
** ALL RIGHTS RESERVED.
**
** THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED
** ONLY  IN  ACCORDANCE  OF  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE
** INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER
** COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
** OTHER PERSON.  NO TITLE TO AND  OWNERSHIP OF THE  SOFTWARE IS  HEREBY
** TRANSFERRED.
**
** THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE
** AND  SHOULD  NOT  BE  CONSTRUED  AS A COMMITMENT BY DIGITAL EQUIPMENT
** CORPORATION.
**
** DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS
** SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*/


/*
**++
**  Subsystem:
**	LinkWorks Manager
**
**  Version: V1.0
**
**  Abstract:
**	LinkWorks Manager main module
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Andre Pavanello
**	Patricia Avigdor
**
**  Creation Date: 1-Nov-89
**
**  Modification History:
**--
*/

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

#define APPL_NAME "LWKManager"

#ifdef VMS
#define CLASS_NAME "DECW$LWK_MANAGER"
#else
#define CLASS_NAME "lwkmanager"
#endif

/*
**  Type Definitions
*/


/*
**  Static Data Definitions
*/

/*
**  Global Data Definitions
*/

/*
**  External function declarations
*/

_DeclareFunction(_Void EnvRetrieveEnvironment,
    (_EnvContext *env_context, _CurrencyContext currency));

_DeclareFunction(_Void EnvSetCurrency,
    (_EnvWindow env_window, _CurrencyContext currency));

_DeclareFunction(_Void EnvDwCreateCursors, (Widget toplevel));

_DeclareFunction(_Void EnvMessageInitialize, (Widget toplevel));

_DeclareFunction(_EnvWindow EnvRetDisplayEnvironment, (_Boolean create_lb));

        
int  main (argc, argv)
unsigned int argc;
 char *argv [];

/*
**++
**  Functional Description:
**	{@description@}
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	{@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**	{@return-value-list-or-none@}
**
**  Exceptions:
**	{@identifier-list-or-none@}
**--
*/
    {
    _String		    value;
    _Boolean		    forever = _True;
    _Boolean		    DoneWithCopyright;
    _EnvWindow		    env_window = (_EnvWindow) 0;
    Widget		    toplevel;
    lwk_boolean		    hyperinvoked;
    lwk_string		    operation;
    lwk_surrogate	    surrogate;
    XtAppContext	    appcontext;
    Display		    *dpy;
    
    DoneWithCopyright = _False;            

    /*
    **  Initialize the various components. We don't need to Initialize DXm and
    **	Mrm, because it's done in lwk_initialize. 
    */
    
    lwk_initialize(&hyperinvoked, &operation, &surrogate);
    
    XtToolkitInitialize();

    appcontext = XtCreateApplicationContext();

    dpy = XtOpenDisplay(appcontext, NULL, APPL_NAME, CLASS_NAME,
	NULL, NULL, (int *) &argc, argv);

    if (dpy == 0)
	/* exit with error message */
	XtAppErrorMsg(appcontext,
	    "invalidDisplay",
	    "XtOpenDisplay",
	    "XtToolkitError",
	    "Can't Open display",
	    (String *) NULL,
	    (Cardinal *) NULL);
	    
    toplevel = XtAppCreateShell(APPL_NAME, CLASS_NAME,
	applicationShellWidgetClass, dpy, NULL, 0);

    if (toplevel == (Widget) 0) {
	printf("?Application shell creation failed.\n");
	exit(TerminationSuccess);
    }

    /*
    **  Synchronize Xlib if requested
    */

    value = (char *) getenv("HS_SYNCHRONIZE");

    if (value != NULL)
	XSynchronize(XtDisplay(toplevel), TRUE);

    _StartExceptionBlock

    EnvDwOpenMainHierarchy();

    EnvDwCreateCursors(toplevel);

    EnvMessageInitialize(toplevel);
    
    env_window = EnvRetDisplayEnvironment(_False);

    /*
    ** Get events forever. We get out by the exit button.
    */
    
    while (forever) {
	XEvent	event;

	XtAppNextEvent(appcontext, &event);

        /*
	** Leave up copyright in the title bar until the user does anything.
	*/
	
	if ((!DoneWithCopyright) &&
	    ((event.type == ButtonPress) ||
	    (event.type == KeyPress))) {

	    DoneWithCopyright = _True;
	    if (env_window != (_EnvWindow) _NullObject)
		_RemoveCopyright(env_window, (_String) 0);
	}

	XtDispatchEvent(&event);
    }
    
    /*
    **  If any exceptions are raised, report the failure. But how do we get
    **	acces to error code?
    */

    _Exceptions
        _WhenOthers
	    printf("?LinkWorks Manager Catch All Exception Handler:\n");
	    printf("   unexpected exception\n");

    _EndExceptionBlock
    
    exit(TerminationSuccess);
    
    }

