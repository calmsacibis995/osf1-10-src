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
** COPYRIGHT (c) 1991 BY
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
**	LWK DXm User Interface message box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 7-Oct-88
**
**  Modification History:
**	BL4  dpr  24-Jan-89 -- some serious clean up after BL4 code review
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_abstract_objects.h"
#include "his_dwui_decwindows_m.h"

/*
**  Type Definitions
*/

typedef struct __Message {
	    XmString message;
	    unsigned long int fao_count;
	} _Message;

typedef struct __MessageVector {
	    unsigned long int num_words;
	    _Message message[_MaxMessageCount];
	} _MessageVector;

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void AcknowledgeMessage,
    (Widget w, Widget *message, _Reason reason));
_DeclareFunction(static void HelpCallback,
    (Widget w, _CString topic, _Reason reason));
_DeclareFunction(void LwkDXmDisplayWIP,
    (_DXmUiPrivate private, _String string));
_DeclareFunction(void LwkDXmRemoveWIP,
    (_DXmUiPrivate private, _String string));
_DeclareFunction(static void CreateWIPDialog,
    (_DXmUiPrivate private, Widget *wipbox));
_DeclareFunction(static void OKWIP,
    (Widget w, _DXmUiPrivate private, _Reason reason));

/*
**  Static Data Definitions
*/


void  LwkDXmMessageBoxCreate(private)
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
    Widget shell;
    
    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);

    /*
    ** Fetch the Message Box.
    */

    shell = XtAppCreateShell("MessageBox", "MessageBox",
	topLevelShellWidgetClass, XtDisplay(private->main_widget),
	(Arg *) 0, (int) 0);

    if (MrmFetchWidget(hierarchy, "message_window", shell,
	    &private->message_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    return;
    }


void  LwkDXmMessageBox(private, status, count)
_DXmUiPrivate private;
 lwk_status *status;
 _Integer count;

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
    int i;
    int ac = 0;
    _String string;
    _MessageVector vector;
    Widget shell;
    XtCallbackRec ok_callback[2];
    XtCallbackRec help_callback[2];
    Arg	arglist[5];

    /*
    **  Initialize the message vector for 'count' status strings (but not more
    **	than the maximum).
    */

    if (count > _MaxMessageCount)
	count = _MaxMessageCount;

    vector.num_words = 2 * count;

    /*
    ** Convert each code to a compount string.
    */

    for (i = 0; i < count; i++) {
	vector.message[i].message = (XmString) _StatusToCString(status[i]);
	vector.message[i].fao_count = 0;
    }

    /*
    **  Create the Message Box if necessary
    */

    if (private->message_box == (Widget) 0) {
    
	LwkDXmMessageBoxCreate(private);
	
	XtRealizeWidget(private->message_box);

        /*
	** Position the message box
	*/

	if (XtParent(private->main_widget) != 0)
	    shell = XtParent(private->main_widget);
	else
	    shell = private->main_widget;

	LwkDXmPositionWidget(XtParent(private->message_box), shell);

        /*
	** Set the Ok callback 
	*/
	
	ok_callback[0].callback = (XtCallbackProc) AcknowledgeMessage;
	ok_callback[0].closure = (XtPointer) &private->message_box;
	ok_callback[1].callback = NULL;
	XtSetArg(arglist[ac], XmNokCallback, ok_callback); ac++;

	/*
	** Remove the Cancel button
	*/
	
	XtUnmanageChild((Widget) XmMessageBoxGetChild(private->message_box,
	    XmDIALOG_CANCEL_BUTTON));
    }
    
    /*
    **  Set up the callbacks
    **
    ** *NOTE*: we could pass the ok_callback and the help_Callback structure
    ** directly in the call but the toolkit ignores them - The work around is
    ** to set values on the message box.
    */

    help_callback[0].callback = (XtCallbackProc) HelpCallback;
    help_callback[0].closure = (XtPointer) _StatusToHelpKey(status[0]);
    help_callback[1].callback = NULL;

    XtSetArg(arglist[ac], XmNhelpCallback, help_callback); ac++;

    XtSetArg(arglist[ac], XmNuserData, private); ac++;

    XtSetValues(private->message_box, arglist, ac);

    /*
    ** Set the contents of the message box
    */

    DXmDisplayCSMessage (private->main_widget, "", TRUE, (Position) 0,
	(Position) 0, (unsigned char) XmDIALOG_FULL_APPLICATION_MODAL,
	&vector, &private->message_box, (int (*)()) 0, (XtCallbackList) 0,
	(XtCallbackList) 0);

    /*
    ** Popup the message box.
    */

    XtManageChild(private->message_box);

    /*
    ** Free the message strings.
    */

    for (i = 0; i < count; i++)
	_DeleteCString(&vector.message[i].message);

    return;
    }


static void  AcknowledgeMessage(w, message, reason)
Widget w;
 Widget *message;
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
    /*
    ** Unmanage the message box.
    */

    XtUnmanageChild (*message);

    return;
    }

static void  HelpCallback(w, topic, reason)
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
    /*
    ** Unmanage the message box.
    */

    XtUnmanageChild(w);

    /*
    ** Call help
    */

    LwkDXmHelpContextSensitiveHelp(w, topic, reason);

    return;
    }
    

static void  CreateWIPDialog(private, wip_box)
_DXmUiPrivate private;
 Widget *wip_box;

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
    Widget shell;
    Arg arglist[2];
    int ac;
    XtCallbackRec ok_callback[2];
    int status;
    XmString cstring;
    MrmCode type;
    Widget ref_widget;
    
    /*
    **  Open the DRM hierarchy
    */

    hierarchy = LwkDXmOpenDRMHierarchy(private);

    /*
    ** Get the message string (the first time around)
    */

    if (private->wip->message_text == (_CString) 0) {
	if (MrmFetchLiteral(hierarchy, "WorkInProgressMessage",
	    XtDisplay(private->main_widget), (caddr_t *) &cstring, &type)
	    != MrmSUCCESS)
		_Raise(drm_fetch_error);
    
	private->wip->message_text = _CopyCString(cstring);

	XmStringFree(cstring);
    }

    /*
    ** Fetch the Box.
    */


    ac = 0; 
    XtSetArg(arglist[ac], XmNwidth, 5); ac++;
    XtSetArg(arglist[ac], XmNheight, 5); ac++;

    shell = XtAppCreateShell("WIPBox", "WIPBox", topLevelShellWidgetClass,
	XtDisplay(private->main_widget), (Arg *) arglist, (int) ac);

    ok_callback[0].callback = (XtCallbackProc) OKWIP;
    ok_callback[0].closure = (XtPointer) private;
    ok_callback[1].callback = NULL;

    ac = 0; 
    XtSetArg(arglist[ac], XmNokCallback, ok_callback); ac++;

    if (MrmFetchWidgetOverride(hierarchy, "wip_window", shell,
	    "wip_window", arglist, (int) ac, wip_box,
	    (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Close the DRM hierarchy.
    */

    LwkDXmCloseDRMHierarchy(hierarchy);

    /*									 
    ** Remove the Cancel and Help buttons
    */
    
    XtUnmanageChild((Widget) XmMessageBoxGetChild(*wip_box, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild((Widget) XmMessageBoxGetChild(*wip_box, XmDIALOG_HELP_BUTTON));

    XtSetMappedWhenManaged(shell, FALSE);
    XtRealizeWidget(shell);

    XtRealizeWidget(*wip_box);

    /*
    ** Position the Work-in-Progress Box wrt a reference widget
    */
    
    if (XtParent(private->main_widget) != 0)
	ref_widget = XtParent(private->main_widget);	/* get the shell */
    else
	ref_widget = private->main_widget;		 /* it is a shell */

    LwkDXmPositionWidget(shell, ref_widget);

    return;
    }


void  LwkDXmDisplayWIP(private, string)
_DXmUiPrivate private;
 _String string;

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
    _WIPBoxStruct wipbox = (_WIPBoxStruct) 0;
    _CString cstring;
    _CString message_string;
    Arg arglist[1];
    
    /*
    **  Create the Working Box if necessary, or use an existing one
    */

    if (private->wip->idle_box == (_WIPBoxStruct) 0) {

	wipbox = (_WIPBoxStruct)_AllocateMem(sizeof(_WIPBoxStructInstance));
	_ClearMem(wipbox, sizeof(_WIPBoxStructInstance));
	
	CreateWIPDialog(private, &wipbox->wipbox);

    } else {
	wipbox = private->wip->idle_box;
	private->wip->idle_box = private->wip->idle_box->next;
    }

    /*
    ** Insert box into list of current WIP boxes
    */

    wipbox->next = private->wip->current_box;
    private->wip->current_box = wipbox;
    
    /*
    ** Uniquely identify this WIP box by saving the destination string
    */

    wipbox->property_name = _CopyString(string);

    /*
    ** Personalize the Work-In-Progress box message
    */

    cstring = _StringToCString(string);
    message_string = LwkDXmConcatCString(private->wip->message_text,
	cstring);

    XtSetArg(arglist[0], XmNmessageString, message_string);
    XtSetValues(wipbox->wipbox, arglist, 1);
    
    /*
    ** Popup the working box.
    */

    XtManageChild(wipbox->wipbox);

    _DeleteCString(&cstring);
    _DeleteCString(&message_string);
    
    return;
    }


void  LwkDXmRemoveWIP(private, string)
_DXmUiPrivate private;
 _String string;

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

    _WIPBoxStruct wipbox;
    _WIPBoxStruct prev = (_WIPBoxStruct) 0;
    
    /* Check all WIP boxes to find the one(s) that should be removed	    */

    wipbox = private->wip->current_box;

    while (wipbox != (_WIPBoxStruct) 0) {

	/* If we have a match, we need to remove this wip box from the	    */
	/* screen, and place the node in the idle list.			    */

	if (strcmp(wipbox->property_name, string) == 0) {
	    XtUnmanageChild (wipbox->wipbox);
	    _DeleteString(&wipbox->property_name);

	    if (prev != (_WIPBoxStruct) 0) 
		prev->next = wipbox->next;
	    else
		private->wip->current_box = wipbox->next;
    
	    /* Make the next node, the first node in the idle list	    */
	    wipbox->next = private->wip->idle_box;

	    /* Place node a beginning of idle list.			    */
	    private->wip->idle_box = wipbox;

	    /* We moved this node to the idle list, so start		    */
	    /* moving forward from the previous node again.		    */
	    wipbox = prev;
	}

	prev = wipbox;
	if (wipbox != 0)
	    wipbox = wipbox->next;
    }

    return;
    }


void  LwkDXmFreeWIPBoxInfo(private)
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
    _WIPBoxStruct wipbox;
    _WIPBoxStruct next_wipbox;

    _DeleteCString(&private->wip->message_text);

    wipbox = private->wip->current_box;

    while (wipbox != (_WIPBoxStruct) 0) {
	next_wipbox = wipbox->next;

	if (wipbox->property_name != (_String) 0) 
	    _DeleteString(&wipbox->property_name);
	_FreeMem(wipbox);

	wipbox = next_wipbox;
    }

    wipbox = private->wip->idle_box;

    while (wipbox != (_WIPBoxStruct) 0) {
	next_wipbox = wipbox->next;

	if (wipbox->property_name != (_String) 0) 
	    _DeleteString(&wipbox->property_name);
	_FreeMem(wipbox);

	wipbox = next_wipbox;
    }

    return;
}


static void  OKWIP(w, private, reason)
Widget w;
 _DXmUiPrivate private;
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

    _WIPBoxStruct wipbox = (_WIPBoxStruct) 0;

    wipbox = private->wip->current_box;

    while (wipbox != (_WIPBoxStruct) 0) {
	if (wipbox->wipbox == XtParent(w))  {
	    LwkDXmRemoveWIP(private, wipbox->property_name);
	    break;
	}
	wipbox = wipbox->next;
    }

    return;
    }
