/*
** COPYRIGHT (c) 1990, 1992 BY
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
**	LinkWorks Services User Interface
**
**  Version: V1.0
**
**  Abstract:
**	Decwindows User Interface for the LWK link menu.
**
**  Keywords:
**	LWK, UI
**
**  Environment:
**	User mode, executable image
**
**  Author:
**	L. Gloyd
**
**  Creation Date: 4-Dec-90
**  
**  Modification History:
**--
*/


/*
** Currently, the DXmHelpSystem (or hyperhelp) is only
** available on VMS.
*/

#if defined(VMS) || defined(__osf__)
#define LWK_DXM_HELP_SYSTEM 1
#else
#undef  LWK_DXM_HELP_SYSTEM 
#endif

/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"
#include "his_registry.h"
#include <X11/Shell.h>

#ifdef LWK_DXM_HELP_SYSTEM /* Use the DXmHelpSystem (hyperhelp) */
#ifdef VMS /* temporary hack because bkr_api.h not available on VMS yet */
#define Bkr_Success                 0
#else	/* non VMS platform */
#include <DXm/bkr_api.h>
#endif /* ifdef VMS */
#else
#include <DXm/DXmHelpB.h>
#endif /* ifdef LWK_DXM_HELP_SYSTEM */

#ifdef LWK_DXM_HELP_SYSTEM /* Use the DXmHelpSystem (hyperhelp) */
#define LwkHelpKey "topic"
#ifdef VMS
#define LwkHelpFile "lwk$dxmhelp" /* LinkWorks help "book" */
#else
#define LwkHelpFile "lwkdxm"
#endif
#endif

_DeclareFunction(void LwkDXmHelpSvnHelpRequested,
    (Widget w, _CString topic, DXmSvnCallbackStruct *cb));
_DeclareFunction(void LwkDXmHelpLinkMenuCascade,
    (Widget w, _DXmUiPrivate private, _Reason reason));
_DeclareFunction(static _DXmUiPrivate WidgetToPrivateData,
    (Widget widget));
_DeclareFunction(void LwkDXmSetShellIconState,
    (Widget shell, long state));
_DeclareFunction(static void DisplayHelpTopic,
    (_DXmUiPrivate private, _CString topic));
_DeclareFunction(static void LwkDXmHelpSystemError,
    (_DXmUiPrivate private, int status));


void  LwkDXmHelpBoxCreate(private)
_DXmUiPrivate private;

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
    MrmType *dummy_class;
    MrmHierarchy hierarchy;
    Widget helpshell;
    
    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);

    /*
    ** Fetch the Help Box.
    */

    helpshell = XtAppCreateShell("HelpWindowShell", "HelpWindow",
	    topLevelShellWidgetClass, XtDisplay(private->main_widget),
	    (ArgList) 0, (Cardinal) 0);

    if (MrmFetchWidget(hierarchy, "HelpWindow", helpshell,
	    &private->help_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }
           


void  LwkDXmHelpContextSensitiveHelp(w, topic, reason)
Widget w;
 _CString topic;
 _Reason reason;

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
    _DXmUiPrivate private;

    /*
    **  Find the PrivateData structure associated with the given Widget
    */

    private = WidgetToPrivateData(w);
               
    /*
    **  If we didn't find one, there's not much we can do about it -- just
    **	return.
    */

    if (private == (_DXmUiPrivate) 0)
	return;

    DisplayHelpTopic(private, topic);

    return;
    }


void  LwkDXmHelpLinkMenuCascade(w, private, reason)
Widget w;
 _DXmUiPrivate private;
 _Reason reason;

/*
**++
**  Functional Description:
**	Display help for Application provided cascade button (on the
**	menu bar).
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
    MrmHierarchy hierarchy;
    MrmCode type;
    XmString helpkey;

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);


    MrmFetchLiteral(hierarchy, _MrmLinkMenuHelpKey,
	XtDisplay(private->main_widget), (caddr_t *) &helpkey, &type);

    /*
    **	Display the help text
    */
    
    DisplayHelpTopic(private, helpkey);


    /*
    ** Cleanup and Close the DRM hierarchy.
    */

    XmStringFree(helpkey);

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
}


void  LwkDXmHelpSvnHelpRequested(w, topic, cb)
Widget w;
 _CString topic;

    DXmSvnCallbackStruct *cb;

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
        
    LwkDXmHelpContextSensitiveHelp(w, topic, &any);

    return;
}


static _DXmUiPrivate  WidgetToPrivateData(widget)
Widget widget;

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
    int ac = 0;
    Arg arglist[10];
    _DXmUiPrivate private;

    /*
    **  Initialize
    */

    private = (_DXmUiPrivate) 0;

    _BeginCriticalSection

    XtSetArg(arglist[ac], XmNuserData, &private); ac++;
    XtGetValues(widget, arglist, ac);

    _EndCriticalSection

    return private;
    }
    


static void  DisplayHelpTopic(private, topic)
_DXmUiPrivate private;
 _CString topic;

/*
**++
**  Functional Description:
**	Display the given help topic.
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
    Arg arglist[1];
    int help_status;
    
    /*
    **  Create the Help Window/Context if necessary
    */

#ifdef LWK_DXM_HELP_SYSTEM /* Use the DXmHelpSystem (hyperhelp) */

    if (private->help_context == (Opaque) 0) {

#if defined(__osf__)

	DXmHelpSystemOpen((Opaque *) &private->help_context,
	    private->main_widget,LwkHelpFile, LwkDXmHelpSystemError,
	    (Opaque) private);

#else
	help_status = DXmHelpSystemOpen((Opaque *) &private->help_context,
	    private->main_widget, LwkHelpFile, LwkDXmHelpSystemError,
	    (Opaque) private);

	if (help_status != Bkr_Success)
	    return;
#endif

    }

#else	    /* Use the DXm help widget for other platforms */

    if (private->help_box == (Widget) 0)
	LwkDXmHelpBoxCreate(private);

#endif /* ifdef LWK_DXM_HELP_SYSTEM (hyperhelp) */ 


#ifdef LWK_DXM_HELP_SYSTEM /* Use the DXmHelpSystem (hyperhelp) */

#if defined(__osf__)

    DXmHelpSystemDisplay((Opaque) private->help_context, NULL, LwkHelpKey,
	(char *) topic, LwkDXmHelpSystemError, (Opaque) private);

#else

    help_status = DXmHelpSystemDisplay((Opaque) private->help_context, NULL,
	LwkHelpKey, (char *) topic, LwkDXmHelpSystemError, (Opaque) private);

    if (help_status != Bkr_Success)
	return;
	
#endif

#else	    /* Use the DXm help widget for other platforms */

    /*
    **  Set the approriate Topic
    */
    XtSetArg(arglist[0], DXmNfirstTopic, topic);
    XtSetValues(private->help_box, arglist, 1);

    /*
    **  Popup the Help Window
    */
    if (XtIsManaged(private->help_box))
	XRaiseWindow(XtDisplay(private->help_box),
	    XtWindow(XtParent(private->help_box)));
    else
	XtManageChild(private->help_box);

    /* 
    **	De-iconize window if necessary
    */
    LwkDXmSetShellIconState(XtParent(private->help_box), NormalState);

#endif /* ifdef LWK_DXM_HELP_SYSTEM(hyperhelp) */


    return;   
}

static void  LwkDXmHelpSystemError(private, status)
_DXmUiPrivate private;
 int status;

{
    printf ("LwkDXmHelpSystemError - Status is %d.\n", status);
    return;
}
