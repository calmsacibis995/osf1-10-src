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
**	HyperSession
**
**  Version: V1.0
**
**  Abstract:
**	DECwindows message box
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Andre Pavanello
**
**  Creation Date: 5-Dec-89
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
**  Macro Definitions
*/

#define _RootMessageWindow "root_message_window"
#define _RootDialogWindow "root_dialog_window"

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

_DeclareFunction(static _Void AcknowledgeMessage,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void EndMessage,
    (Widget w, Widget *message, _Reason reason));

_DeclareFunction(static _Void OKMessage,
    (Widget w, Widget *message, _Reason reason));
    
_DeclareFunction(static _Void UilCreateRootDialogYes,
    (Widget w, _WindowPrivate private,_Reason reason));

_DeclareFunction(static _Void UilCreateRootDialogNo,
    (Widget w, _WindowPrivate private,_Reason reason));

_DeclareFunction(static _Void UilCreateRootDialogLabel,
    (Widget w, _WindowPrivate private,_Reason reason));

/*
** External Routine Declarations
*/

_DeclareFunction(_Void EnvHelpPopupRootHelp,
    (Widget w, _CString topic, _Reason reason));

/*
**  Static Data Definitions
*/

static lwk_status _SavedHisStatus = lwk_s_success;

static MrmRegisterArg _Constant drm_registrations[] = {
    {"env_message_ok",		    (caddr_t) AcknowledgeMessage},
    {"env_context_sensitive_help",  (caddr_t) EnvHelpPopupHelp}
};

static MrmCount _Constant drm_registrations_size = XtNumber(drm_registrations);

static MrmRegisterArg _Constant drm_reg_root_message[] = {
    {"env_message_ok",		    (caddr_t) AcknowledgeMessage},
    {"env_context_sensitive_help",   (caddr_t) EnvHelpPopupRootHelp}
};

static MrmCount _Constant drm_reg_root_message_size =
    XtNumber(drm_reg_root_message);

/*
** DRM register table for the root dialog box
*/

static MrmRegisterArg _Constant drm_reg_root_dialog[] = {
    {"root_dialog_activate_cancel", (caddr_t) EndMessage},
    {"env_context_sensitive_help",  (caddr_t) EnvHelpPopupRootHelp}
};

static MrmCount _Constant drm_reg_root_dialog_size =
    XtNumber(drm_reg_root_dialog);

/*
**  Store the Widget Id for use in displaying a message
**  as a child of root.
*/
    
static Widget RootMessageParent = (Widget) 0;

static Widget RootDialogBox = (Widget) 0;
static Widget RootDialogYesBtn = (Widget) 0;
static Widget RootDialogNoBtn = (Widget) 0;
static Widget RootDialogLabel = (Widget) 0;
    

_Void  EnvMessageInitialize(toplevel)
Widget toplevel;

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

    RootMessageParent = toplevel;

    /*
    ** Register the Message callback routines so that the ressource
    ** manager can reslove them at widget creation time
    */
    
    MrmRegisterNames(drm_registrations, drm_registrations_size);

    return;
    }
    

_Void  EnvDWMessageBoxCreate(private)
_WindowPrivate private;

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
    MrmType	    *dummy_class;
    MrmHierarchy    hierarchy;

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = EnvDwGetMainHierarchy();


    /*
    ** Register private data.
    */

    EnvDWRegisterDRMNames(private);

    /*
    ** Fetch the Message Box.
    */

    if (MrmFetchWidget(hierarchy, "message_window", private->main_widget,
	    &private->message_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    return;
    }


_Void  EnvDWMessageBox(private, status, count)
_WindowPrivate private;
 _Status *status;

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
    _Integer	    i;
    _String	    string;
    _CString	    cs_message;
    _MessageVector  vector;
    lwk_status	    stat;
    lwk_ddif_string his_ddif_message = lwk_c_null_object;
    Arg		    arglist[2];
    _Integer	    ac = 0;

    /*
    **  Initialize the message vector for 'count' status strings (but not more
    **	than the maximum). Need for one more because of the his message
    */

    if ((count + 1)  > _MaxMessageCount)
	count = _MaxMessageCount;

    vector.num_words = 2 * count;

    /*
    ** Convert each code to a compound string.
    */

    for (i = 0; i < count; i++) {
	vector.message[i].message = (XmString) _StatusToCString(status[i]);
	vector.message[i].fao_count = 0;
    }

    /*
    **  Append the his message if any
    */

    if (_SavedHisStatus != lwk_s_success) {

	stat = lwk_status_to_ddif_string(_SavedHisStatus, &his_ddif_message);
	
	if (stat == lwk_s_success) {

	    vector.num_words = 2 * (count + 1);

	    cs_message = _DDIFStringToCString((_DDIFString) his_ddif_message);
	
	    vector.message[i].message = (XmString) cs_message;
	    vector.message[i].fao_count = 0;
	}
    }

    /*
    **  Create the Message Box if necessary
    */

    if (private->message_box == (Widget) 0) 
	EnvDWMessageBoxCreate(private);

    /*
    ** Set just the contents of the message box.
    */

    DXmDisplayCSMessage(private->main_widget, "", TRUE, (Position) 0,
	(Position) 0, (int *) 0, (int *) &vector, &private->message_box,
	(int *) 0, (XtCallbackList) 0, (XtCallbackList) 0);

    /*
    ** Remove the Cancel button
    */
    XtUnmanageChild((Widget) XmMessageBoxGetChild(private->message_box,
	XmDIALOG_CANCEL_BUTTON));
    
    /* 
    **	Set the Cursor to inactive shape
    */

    _SetCursor(private->window, _InactiveCursor);


    /*
    ** Popup the message box.
    */

    XtManageChild(private->message_box);

    /*
    ** Free the message strings.
    */

    for (i = 0; i < count; i++)
	_DeleteCString(&vector.message[i].message);

    _SavedHisStatus = lwk_s_success;

    if (his_ddif_message != lwk_c_null_object) {

	lwk_delete_ddif_string(&his_ddif_message);
	his_ddif_message = lwk_c_null_object;
    }

    return;
    }


static _Void  AcknowledgeMessage(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

/*
**++
**  Functional Description:
**    	{@description@}
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

    XtUnmanageChild (w);

    /*
    ** Restore the default cursor
    */

    _SetCursor(private->window, _DefaultCursor);
    
    return;
    }


_Void  EnvDWRootMessageBox(status, count, severity)
hs_status *status;
 _Integer count;

    _MsgSeverityFlag severity;

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
    _Integer	    i;
    _Integer	    ac = 0;
    _String	    string;
    _MessageVector  vector;
    _CString	    cs_message;
    lwk_status	    stat;
    lwk_ddif_string his_ddif_message = lwk_c_null_object;
    Widget	    message_box;
    XtCallbackRec   ok_callback[2];
    MrmType	    *dummy_class;
    MrmHierarchy    hierarchy;
    Arg		    arglist[6];
    
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
    **  Append the his message if any
    */

    if (_SavedHisStatus != lwk_s_success) {

	stat = lwk_status_to_ddif_string(_SavedHisStatus, &his_ddif_message);
	
	if (stat == lwk_s_success) {

	    vector.num_words = 2 * (count + 1);

	    cs_message = _DDIFStringToCString((_DDIFString) his_ddif_message);
	
	    vector.message[i].message = (XmString) cs_message;
	    vector.message[i].fao_count = 0;
	}
    }

    /*
    ** Register the callbacks for the root message
    */
    
    MrmRegisterNames(drm_reg_root_message, drm_reg_root_message_size);

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = EnvDwGetMainHierarchy();

    /*
    ** Fetch the Message Box.
    */

    if (MrmFetchWidget(hierarchy, _RootMessageWindow, RootMessageParent,
	    &message_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	_Raise(drm_fetch_error);

    if (severity == hs_c_fatal_message) {
	/*
	**  Set up the OK callback that exits the program
	*/
	ok_callback[0].callback = (XtCallbackProc) EndMessage;

    } else {
	/*
	**  Set up the OK callback that simply dismisses the msg box
	*/

	ok_callback[0].callback = (XtCallbackProc) OKMessage;
    }    

    ok_callback[0].closure = (XtPointer) NULL;
    ok_callback[1].callback = NULL;

    XtSetArg(arglist[ac], XmNokCallback, ok_callback); ac++;
    XtSetValues(message_box, arglist, ac);

    /*
    ** Set just the contents of the message box.
    */

    DXmDisplayCSMessage(RootMessageParent, "", 0, 0, 0, 0, (int *) &vector,
	&message_box, (int *) 0, (XtCallbackList) 0, (XtCallbackList) 0);
        
    /*
    ** Remove the Cancel button
    */
    
    XtUnmanageChild((Widget) XmMessageBoxGetChild(message_box,
	XmDIALOG_CANCEL_BUTTON));

    /*
    ** Store the toplevel on the Help Button
    */

    XtSetArg(arglist[0], XmNuserData, RootMessageParent);
    XtSetValues(message_box, arglist, 1);

    /*
    ** Popup the message box.
    */

    XtManageChild(message_box);

    /*
    ** Free the message strings.
    */

    for (i = 0; i < count; i++)
	_DeleteCString(&vector.message[i].message);

    _SavedHisStatus = lwk_s_success;

    if (his_ddif_message != lwk_c_null_object) {

	lwk_delete_ddif_string(&his_ddif_message);
	his_ddif_message = lwk_c_null_object;
    }
    
    return;
    }


static _Void  EndMessage(w, message, reason)
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

    XtUnmanageChild(w);

    exit(TerminationSuccess);

    }

static _Void  OKMessage(w, message, reason)
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

    XtUnmanageChild(w);

    }


_Void  EnvMsgSaveHisStatus(his_stat)
lwk_status his_stat;

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

    _SavedHisStatus = his_stat;

    return;
    }


_Void  EnvDWRootDialogBox(yes_callback, no_callback, identifier)
_Callback yes_callback;
 _Callback no_callback;

    _String identifier;

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
    _Integer	    i;
    _String	    string;
    _CString	    message;
    XtCallbackRec   callback[2];
    MrmType	    *dummy_class;
    MrmHierarchy    hierarchy;
    Arg		    arglist[6];
    
    /*
    **  Open the DRM hierarchy
    */

    hierarchy = EnvDwGetMainHierarchy();

    /*
    ** Register the callbacks for the root message
    */
    
    MrmRegisterNames(drm_reg_root_dialog, drm_reg_root_dialog_size);

    /*
    ** Fetch the Message Box.
    */
    
    if (RootDialogBox == (Widget) 0) {

	if (MrmFetchWidget(hierarchy, _RootDialogWindow, RootMessageParent,
		&RootDialogBox, (MrmType *) &dummy_class) != MrmSUCCESS)
	    _Raise(drm_fetch_error);
    }

    /*
    **	If the Dialog Box isn't realized, do it now
    */

    if (!XtIsRealized(RootDialogBox))
	XtRealizeWidget(RootDialogBox);

    /*
    ** Set the callbacks
    */
	    
    if (yes_callback != (_Callback) NULL) {

        callback[0].callback = (XtCallbackProc) yes_callback;
        callback[0].closure = (XtPointer) NULL;
        callback[1].callback = NULL;

        XtSetArg(arglist[0], XmNokCallback, (XtCallbackRec *) callback);
	XtSetValues(RootDialogBox, arglist, 1);
    }

    if (no_callback != (_Callback) NULL) 
        callback[0].callback = (XtCallbackProc) no_callback;
    else 
        callback[0].callback = (XtCallbackProc) EndMessage;

    callback[0].closure = (XtPointer) NULL;
    callback[1].callback = NULL;
	
    XtSetArg(arglist[0], XmNcancelCallback, (XtCallbackRec *) callback);
    XtSetValues(RootDialogBox, arglist, 1);

    /*
    ** Set the message string fetched from UIL
    */

    if (identifier != (_String) 0) {

	EnvDWGetCStringLiteral(identifier, &message);
	XtSetArg(arglist[0], XmNmessageString,(XmString) message);
	XtSetValues(RootDialogBox, arglist, 1);
	_DeleteCString(&message);
    }

    /*
    ** Store the toplevel on the box (this is for the Help Widget)
    */

    XtSetArg(arglist[0], XmNuserData, RootMessageParent);
    XtSetValues(RootDialogBox, arglist, 1);

    /*
    ** Popup the message box.
    */

    XtManageChild(RootDialogBox);

    return;
    }


_Void  EnvDwMessageDismissRootDb()
    {

    if (RootDialogBox != (Widget) 0) {
	XtUnmanageChild(RootDialogBox);
	XFlush(XtDisplay(RootDialogBox));
    }
    
    return;
    }


Widget  EnvDwMessageGetToplevel()
    {
    
    return RootMessageParent;
    }

