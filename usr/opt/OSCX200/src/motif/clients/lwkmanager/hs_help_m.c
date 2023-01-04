/*
** COPYRIGHT (c) 1989, 1992 BY
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
**	LinkWorks Manager User Interface
**
**  Version: V1.0
**
**  Abstract:
**	DXm LinkWorks Manager Help module
**
**  Keywords:
**	HS, UI
**
**  Environment:
**	{@environment description@}
**
**  Author:
**	Andr Pavanello
**
**  Creation Date: 15-Jan-90
**
**  Modification History:
**--
*/

/*
** Currently, the DXmHelpSystem (or hyperhelp) is only
** available on VMS.
*/
#if defined(VMS) || defined(__osf__)
#define LWKMGR_DXM_HELP_SYSTEM 1
#else
#undef  LWKMGR_DXM_HELP_SYSTEM 
#endif

/*
**  Include Files
*/

#include "hs_include.h"
#include "hs_abstract_objects.h"
#include "hs_decwindows.h"

#ifdef LWKMGR_DXM_HELP_SYSTEM /* Use the DXmHelpSystem (hyperhelp) */
#ifdef VMS /* hack because bkr_api.h not available on VMS yet */
#define Bkr_Success                 0
#else	/* non VMS platform */
#include <DXm/bkr_api.h>
#endif /* ifdef VMS */
#else
#include <DXm/DXmHelpB.h>
#endif /* ifdef LWKMGR_DXM_HELP_SYSTEM */


/*
**  Table of Contents
*/
_DeclareFunction(static void EnvDXmHelpSystemError,
    (_WindowPrivate private, int status));

/*
**  Macro Definitions
*/

#ifdef VMS
#define _HelpLibrary    "DECW$LWK_MANAGER"
#else
#define _HelpLibrary	"lwkmanager"
#endif

#ifdef LWKMGR_DXM_HELP_SYSTEM  /* Use the DXmHelpSystem (hyperhelp) for VMS */
#define LwkMgrHelpKey "topic"
#if defined(VMS)
#define LwkMgrHelpFile "decw$lwk_manager" /* LinkWorks Manager help "book" */
#else
#define LwkMgrHelpFile "lwkmanager"
#endif
#endif

/*
**  Type Definitions
*/

/*
**  Static Data Definitions
*/

#ifdef LWKMGR_DXM_HELP_SYSTEM  
static Opaque RootHelpContext = (Opaque) 0; /* DXmHelpSystem context	*/
#else       
static Widget RootHelpBox = (Widget) 0;	    /* DXmHelp widget id	*/
#endif

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvHelpPopupHelp(w, topic, reason)
Widget w;
 _CString topic;
 _Reason reason;

/*
**++
**  Functional Description:
**
**  Keywords:
**
**  Arguments:
**
**	reason - Don't use this structure because it could be passed from
**		 EnvHelpSvnHelp in which case it is bogus.
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
    {
    _Integer		count = 0;
    _WindowPrivate	private;
    Arg			arglist[5];
    XmString		cs_str;
    MrmType		*dummy_class;
    MrmHierarchy	hierarchy;
    int			help_status;
    
    private = (_WindowPrivate) _GetEnvWindowPrivate;
     
    _SetCursor(private->window, _WaitCursor);

#ifdef LWKMGR_DXM_HELP_SYSTEM  /* Use DXmHelpSystem (hyperhelp) */

    if (private->help_context == (Opaque) 0) {
    
#if defined(__osf__)

	DXmHelpSystemOpen((Opaque *) &private->help_context,
	    private->main_widget, LwkMgrHelpFile, EnvDXmHelpSystemError,
	    private);

#else

	help_status = DXmHelpSystemOpen(&private->help_context,
	    private->main_widget, LwkMgrHelpFile, EnvDXmHelpSystemError,
		private);

	if (help_status != Bkr_Success) {
	    _SetCursor(private->window, _DefaultCursor);
	    return;
	}
#endif
    
    }
    
#else	    /* Use DXmHelp widget */

    if (private->help_box == (Widget) 0) {

	EnvDWRegisterDRMNames(private);

	/*
	**  Open the DRM hierarchy
	*/

	hierarchy = EnvDwGetMainHierarchy();

	/*
	** Fetch the Help Box.
	*/

	cs_str = _StringToXmString((char *)_HelpLibrary);

	XtSetArg(arglist[0], DXmNlibrarySpec, cs_str);

	if (MrmFetchWidgetOverride(hierarchy, "help_window",
	    private->main_widget, (char *) 0, arglist, (int) 1,
	    &private->help_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	
	    _Raise(drm_fetch_error);

	XmStringFree(cs_str);
    }

#endif /* #ifdef LWKMGR_DXM_HELP_SYSTEM (hyperhelp) */

#ifdef LWKMGR_DXM_HELP_SYSTEM  /* Use DXmHelpSystem (hyperhelp) */

#if defined(__osf__)

    DXmHelpSystemDisplay((Opaque) private->help_context, NULL, LwkMgrHelpKey,
	topic, EnvDXmHelpSystemError, private);

#else

    help_status = DXmHelpSystemDisplay(private->help_context, NULL,
	LwkMgrHelpKey, topic, EnvDXmHelpSystemError, private);

    if (help_status != Bkr_Success) {
	_SetCursor(private->window, _DefaultCursor);
	return;
    }

#endif
    
#else	    /* Use DXmHelp widget for other platforms */

    XtSetArg(arglist[count], DXmNfirstTopic, (XmString) topic); count++;

    XtSetValues(private->help_box, arglist, count);

    XtManageChild(private->help_box);

#endif /* #ifdef LWKMGR_DXM_HELP_SYSTEM (hyperhelp) */

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


_Void  EnvHelpPopupRootHelp(w, topic, reason)
Widget w;
 _CString topic;
 _Reason reason;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
    {
    Widget		toplevel;
    Arg			arglist[2];
    XmString		cs_str;
    MrmType		*dummy_class;
    MrmHierarchy	hierarchy;
    int			help_status;
    
#ifdef LWKMGR_DXM_HELP_SYSTEM  /* Use DXmHelpSystem (hyperhelp) */

    if (RootHelpContext == (Opaque) 0) {
        /*
	** Get the toplevel widget from the UserData field of the button
	*/

	XtSetArg(arglist[0], XmNuserData, &toplevel);
	XtGetValues(w, arglist, 1);

	XtRealizeWidget(toplevel);	

#if defined(__osf__)

    DXmHelpSystemOpen(&RootHelpContext, toplevel, LwkMgrHelpFile,
	EnvDXmHelpSystemError, NULL);

#else

	help_status = DXmHelpSystemOpen(&RootHelpContext, toplevel,
	    LwkMgrHelpFile, EnvDXmHelpSystemError, NULL);

	if (help_status != Bkr_Success)
	    return;

#endif

    }

#else	    /* Use DXmHelp widget for other platforms */

    if (RootHelpBox == (Widget) 0) {

	/*
	**  Open the DRM hierarchy
	*/

	hierarchy = EnvDwGetMainHierarchy();

        /*
	** Get the toplevel widget from the UserData field of the button
	*/

	XtSetArg(arglist[0], XmNuserData, &toplevel);
	XtGetValues(w, arglist, 1);

	/*
	** Fetch the Help Box.
	*/

	cs_str = _StringToXmString((char *)_HelpLibrary);

	XtSetArg(arglist[0], DXmNlibrarySpec, cs_str);

	if (MrmFetchWidgetOverride(hierarchy, "help_window",
	    toplevel, (char *) 0, arglist, (int) 1,
	    &RootHelpBox, (MrmType *) &dummy_class) != MrmSUCCESS)
	
	    _Raise(drm_fetch_error);

	XmStringFree(cs_str);
    }

#endif /* #ifdef LWKMGR_DXM_HELP_SYSTEM (hyperhelp) */

#ifdef LWKMGR_DXM_HELP_SYSTEM  /* Use DXmHelpSystem (hyperhelp) */  

#if defined(__osf__)

    DXmHelpSystemDisplay(RootHelpContext, NULL, LwkMgrHelpKey, topic,
	EnvDXmHelpSystemError, NULL);

#else

    help_status = DXmHelpSystemDisplay(RootHelpContext, NULL,
	LwkMgrHelpKey, topic, EnvDXmHelpSystemError, NULL);

    if (help_status != Bkr_Success)
	return;

#endif

#else	    /* Use DXmHelp widget for other platforms */

    XtSetArg(arglist[0], DXmNfirstTopic, (XmString) topic);

    XtSetValues(RootHelpBox, arglist, 1);

    XtManageChild(RootHelpBox);

#endif /* #ifdef LWKMGR_DXM_HELP_SYSTEM (hyperhelp) */

    return;
    }


_Void  EnvHelpSvnHelp(w, topic, cb)
Widget w;
 _CString topic;
 DXmSvnCallbackStruct *cb;

/*
**++
**  Functional Description:
**      {@description@}
**
**  Keywords:
**      {@keyword-list-or-none@}
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
{
    XmAnyCallbackStruct    any;

    /*
    ** Pass a dummy callback structure to the help
    ** routine.  The help routine doesn't look into this
    ** structure.
    **
    ** If you need to look into the DXmSvnCallbackStruct check for
    ** zeros, because it is possible that this routine is being
    ** called from DXmHelpOnContext - in which case the Svn structure
    ** can't be filled in correctly.  (at least for Motif V1.1)
    */
    
    EnvHelpPopupHelp(w, topic, &any);

    return;
}


_Void  EnvHelpOnContextTracking(w, private, reason)
Widget w;
 _WindowPrivate private;
 _Reason reason;

/*
**++
**  Functional Description:
**  
**      For use with the Motif On Context help.  Creates a modal
**	context sensitive help cursor.  When the user selects something
**	context sensitive help on that item (or parent) will appear (if it
**	exists), and the cursor returns to its normal state.
**	
**
**  Keywords:
**      {@keyword-list-or-none@}
**
**  Arguments:
**      {@identifier-list-or-none@}
**  [@non-local-references@]
**  [@pre-and-post-conditions@]
**
**  Result:
**      {@return-value-list-or-none@}
**
**  Exceptions:
**      {@identifier-list-or-none@}
**--
*/
    {
    DXmHelpOnContext(private->main_widget, False);  
					    
    return;
    }

static void  EnvDXmHelpSystemError(private, status)
_WindowPrivate private;
 int status;

{
    printf ("LinkWorks EnvDXmHelpSystemError - Status is %d.\n", status);
    return;
}
