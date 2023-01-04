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
** COPYRIGHT (c) 1989, 1991 BY
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
**  Version: X0.1
**
**  Abstract:
**	DECwindows LinkWorks Manager User Interface Linkbase dialog box.
**
**  Keywords:
**	HS, UI
**
**  Environment:
**	{@environment description@}
**
**  Author:
**	Patricia Avigdor
**	[@author information@]...
**
**  Creation Date: 10-Nov-89
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

#ifdef VMS
#include <stdlib.h>	/* needed for getenv on VMS only */
#endif

/*
**  Table of Contents
*/

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Void UilActivateOk,
    (Widget w, _WindowPrivate private, XmFileSelectionBoxCallbackStruct *reason));
_DeclareFunction(static _Void UilActivateCancel,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void LbBoxCreate,
    (_WindowPrivate private));
_DeclareFunction(static _Void PopupLbCreateBox,
    (_WindowPrivate private, _String linkbase_name));
_DeclareFunction(static _Void UilCreateName,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateFile,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateLbCreateOk,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateLbCreateOk,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateLbCreateCancel,
    (Widget w, _WindowPrivate private, _Reason reason));

_DeclareFunction(static _Void UilCreateQuestionYes,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateQuestionNo,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilCreateQuestionLabel,
    (Widget w, _WindowPrivate private, _Reason reason));
_DeclareFunction(static _Void UilActivateQuestionCancel,
    (Widget w, _WindowPrivate private, _Reason reason));

/*
**  Static Data Definitions
*/

static MrmRegisterArg _Constant Register[] = {
    {"lb_select_box_ok",		    (caddr_t) UilActivateOk},
    {"lb_select_box_cancel",	    (caddr_t) UilActivateCancel},
    {"lb_create_create_name",	    (caddr_t) UilCreateName},
    {"lb_create_create_file",	    (caddr_t) UilCreateFile},
    {"lb_create_create_ok",	    (caddr_t) UilCreateLbCreateOk},
    {"lb_create_activate_ok",	    (caddr_t) UilActivateLbCreateOk},
    {"lb_create_activate_cancel",   (caddr_t) UilActivateLbCreateCancel},
    {"env_context_sensitive_help",  (caddr_t) EnvHelpPopupHelp}
};

static MrmCount _Constant RegisterSize = XtNumber(Register);

static MrmRegisterArg _Constant Register1[] = {

    {"question_create_yes",	    (caddr_t) UilCreateQuestionYes},
    {"question_create_no",	    (caddr_t) UilCreateQuestionNo},
    {"question_create_label",	    (caddr_t) UilCreateQuestionLabel},
    {"question_activate_cancel",    (caddr_t) UilActivateQuestionCancel},

};

static MrmCount _Constant Register1Size = XtNumber(Register1);

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/


_Void  EnvDWLbBox(private, new)
_WindowPrivate private;
 _Boolean new;

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
    **  If the Dialog Box isn't created, do it.
    */

    if (private->fileselection == (Widget) 0)
	LbBoxCreate(private);

    /*
    ** Set the new flag.
    */

    private->new_window = new;

    /*
    ** If the Dialog Box isn't realized, do it now.
    */

    if (!XtIsRealized(private->fileselection))
	XtRealizeWidget(private->fileselection);

    /*
    ** Popup the dialog box
    */

    if (XtIsManaged(private->fileselection)) {
	XRaiseWindow(XtDisplay(private->fileselection),
	    XtWindow(XtParent(private->fileselection)));
    }
    else
	XtManageChild(private->fileselection);

    return;
    }


static _Void  LbBoxCreate(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	Create the linkbase selection dialog box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**  Result:
**	None
**
**  Exceptions:
**	DRMHierarchyFetchFailed
**--
*/
    {
    MrmHierarchy    hierarchy;
    MrmType	    *dummy_class;
    XmString 	    filter_cstr;
    Arg		    arglist[1];
    _String	    tmp_str;
    _String	    filter_str;

    /*
    ** Register all DRM function names and LbWindow object name.
    */
             
    MrmRegisterNames(Register, RegisterSize);

    /*
    ** Register private data.
    */

    EnvDWRegisterDRMNames(private);

    /*
    **  Open the DRM hierarchy
    */

    hierarchy = EnvDwGetMainHierarchy();

    /*
    ** Fetch the Linkbase dialog box.
    */

    if (MrmFetchWidget(hierarchy, _DrmLinkbaseBox, private->main_widget,
	&private->fileselection, (MrmType *) &dummy_class) != MrmSUCCESS)
	
	_Raise(drm_fetch_error);

    private->lbcreate_file_name = (_String) 0;
	 
    /*
    **  Set the appropriate default filter
    */

#ifdef VMS
    filter_cstr = _StringToXmString((char *) "SYS$LOGIN:*.LINKBASE");
#else
    tmp_str = (_String) getenv("HOME");
    filter_str = _CopyString(tmp_str);
    filter_str = _ConcatString(filter_str, "/*.linkbase");
    filter_cstr = _StringToXmString((char *) filter_str);
    _DeleteString(&filter_str);
#endif

    XtSetArg(arglist[0], XmNdirMask, filter_cstr);
    XtSetValues(private->fileselection, arglist, 1);

    XmStringFree(filter_cstr);
                   
    return;
    }


static _Void  PopupLbCreateBox(private, linkbase_name)
_WindowPrivate private;

    _String linkbase_name;

/*
**++
**  Functional Description:
**	Pops up a message box to ask if the linkbase should be created.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**  Result:
**	None
**
**  Exceptions:
**--
*/
    {
    MrmHierarchy    hierarchy;
    MrmType	    *dummy_class;
    Arg		    arglist[10];
    int		    ac = 0;
    _CString	    label;
    XmString	    cs_linkbase_name;

    _StartExceptionBlock

    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->lbcreate_box == (Widget) 0) {

	/*
	**  Open the DRM hierarchy
	*/

	hierarchy = EnvDwGetMainHierarchy();

	/*
	** Register private data.
	*/
	
	EnvDWRegisterDRMNames(private);

	/*
	** Fetch the Linkbase Create dialog box.
	*/

	if (MrmFetchWidget(hierarchy, _DrmLinkbaseCreateBox,
	    private->main_widget, &private->lbcreate_box,
	    (MrmType *) &dummy_class) != MrmSUCCESS)
	
		_Raise(drm_fetch_error);
    }

    /*
    ** If the Dialog Box isn't realized, do it now.
    */

    if (!XtIsRealized(private->lbcreate_box))
	XtRealizeWidget(private->lbcreate_box);

    /*
    ** Set the name text to blank.
    */
     
    DXmCSTextSetString(private->lbcreate_name, NULL);

    /*
    ** Set the file specification.
    */
    private->lbcreate_file_name = _CopyString(linkbase_name);

    cs_linkbase_name = _StringToXmString((char *) linkbase_name);
    DXmCSTextSetString(private->lbcreate_file, cs_linkbase_name);
    XmStringFree(cs_linkbase_name);
    
    /*
    ** Popup the dialog box.
    */

    XtManageChild(private->lbcreate_box);

    /*
    ** Put the input on the name field.
    */

    /*
    ** Hack ******* for ProcessTraversal **********
    */

    ac = 0;
    XtSetArg(arglist[ac], XmNsensitive, _False); ac++;
    XtSetValues(private->lbcreate_ok_button, arglist, ac);

    XmProcessTraversal(private->lbcreate_name, XmTRAVERSE_CURRENT);

    ac = 0;
    XtSetArg(arglist[ac], XmNsensitive, _True); ac++;
    XtSetValues(private->lbcreate_ok_button, arglist, ac);
    							           
    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
            _Reraise;
	
    _EndExceptionBlock                

    return;
    }

static _Void  UilCreateName(w, private, reason)
Widget w;
 _WindowPrivate private;
 _Reason reason;

{                   
    private->lbcreate_name = w;
    return;
}

static _Void  UilCreateFile(w, private, reason)
Widget w;
 _WindowPrivate private;
 _Reason reason;

{
    private->lbcreate_file = w;
    return;
}

static _Void  UilCreateLbCreateOk(w, private, reason)
Widget w;
 _WindowPrivate private;
 _Reason reason;

{
    private->lbcreate_ok_button = w;
    return;
}


static _Void  UilActivateOk(w, private, reason)
Widget w;
 _WindowPrivate private;

    XmFileSelectionBoxCallbackStruct *reason;

/*
**++
**  Functional Description:
**	Callback routine.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget.
**	private: window private data.
**	reason: reason code.
**
**  Result:
**	None
**
**  Exceptions:
**--
*/
    {
    lwk_status	    status;
    lwk_linkbase    linkbase;
    _String	    linkbase_name;
    _LbWindow	    window;
    _Status	    stat[2];

    _StartExceptionBlock

    XtUnmanageChild(w);

    _SetCursor(private->window, _WaitCursor);

    /*
    ** Get the linkbase name and convert it to a string
    */

    linkbase_name = _CStringToString(reason->value);

    /*
    ** Open the linkbase.
    */

    status = lwk_open(linkbase_name, lwk_c_false, &linkbase);

    /*
    ** If linkbase doesn't exist, pop up message box to ask if the
    ** linkbase should be created.
    */

    switch(status) {

	case lwk_s_success :
	
	    /*
	    ** Check if this linkbase isn't already displayed.
	    */
	
	    if (EnvOpLbWindowLbInWindow(&window, linkbase)) {

		/*
		**  if so , pop the corresponding lbwindow and display a
		**  message
		*/
		
		_RaiseWindow((_Window)window);
		
		stat[0] = _StatusCode(lb_already_open);
		stat[1] = _StatusCode(new_wnd_not_open);
		
		_DisplayMessage((_Window)window, stat, 2);
	    }
	
	    else
	
		/*
		** Open the linkbase window.
		*/
		
		EnvDWLbWindowOpenLbInWindow(private, linkbase);
		
	    /*
	    **  Close the linkbase 
	    */
	    
	    status = lwk_close(linkbase);
	    
	    if (status != lwk_s_success) {
		_SaveHisStatus(status);
		_Raise(close_lb_err);
	    }
		
    	    break;
	
	case lwk_s_no_such_linkbase:
	    PopupLbCreateBox(private, linkbase_name);
	    break;
	
	default:
	    _SaveHisStatus(status);
	    _Raise(open_lb_err);
	    break;
    }

    _DeleteString(&linkbase_name);

    _Exceptions
	_WhenOthers

	    stat[0] =_StatusCode(open_lb_failed);
	
	    _DisplayMessage(private->window, stat, 1);

    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }


static _Void  UilActivateCancel(w, private, reason)
Widget w;
 _WindowPrivate private;
 _Reason reason;

/*
**++
**  Functional Description:
**	Callback routine.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget.
**	private: window private data.
**	reason: reason code.
**
**  Result:
**	None
**
**  Exceptions:
**--
*/
    {

    XtUnmanageChild(private->fileselection);

    return;
    }


static _Void  UilActivateLbCreateOk(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

/*
**++
**  Functional Description:
**	Callback routine.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget.
**	private: window private data.
**	reason: reason code.
**
**  Result:
**	None
**
**  Exceptions:
**--
*/
    {
    _String  	    lb_name;
    _DDIFString	    ddif_name;
    lwk_status	    status;
    Arg		    arglist[2];
    XmString	    cs_name;                          
    lwk_linkbase    linkbase;
    lwk_ddif_string lwk_ddifstr;

    _SetCursor(private->window, _WaitCursor);

    _StartExceptionBlock

    XtUnmanageChild(private->lbcreate_box);

    /*
    ** Create it.
    */

    status = lwk_open(private->lbcreate_file_name, lwk_c_true, &linkbase);

    if (status == lwk_s_success) {
    
	/*
	** Get linkbase name from the text widget.
	*/
                              
	cs_name = (XmString) DXmCSTextGetString(private->lbcreate_name);

	/*
	** Set the linkbase name value.
	*/
	
	if (!XmStringEmpty(cs_name)) {

	    ddif_name = _CStringToDDIFString((_CString) cs_name);
	    lwk_ddifstr = (lwk_ddif_string) ddif_name;
	    
	    status = lwk_set_value(linkbase, lwk_c_p_name,
		lwk_c_domain_ddif_string, &lwk_ddifstr, lwk_c_set_property);

	    _DeleteDDIFString(&ddif_name);
	    XmStringFree(cs_name);
	}
    }
    else
	_Raise(open_lb_err);
	
    /*
    ** Open the linkbase window.
    */

    if (status == lwk_s_success)
	EnvDWLbWindowOpenLbInWindow(private, linkbase);
    else {
	_SaveHisStatus(status);
	_Raise(open_lb_err);
    }


    _FreeMem(private->lbcreate_file_name);
    private->lbcreate_file_name = (_String) 0;

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    _Status	    stat[2];
	    stat[0] =_StatusCode(open_lb_failed);
	    stat[1] =_Others;
	
	    _DisplayMessage(private->window, stat, 2);
	
    _EndExceptionBlock

    _SetCursor(private->window, _DefaultCursor);

    return;
    }

static _Void  UilActivateLbCreateCancel(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

/*
**++
**  Functional Description:
**	Callback routine.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	w: widget.
**	private: window private data.
**	reason: reason code.
**
**  Result:
**	None
**
**  Exceptions:
**--
*/
    {

    XtUnmanageChild(private->lbcreate_box);

    return;
    }


_Void  EnvDwLbBoxSetCursor(private, cursor_type)
_WindowPrivate private;
 _CursorType cursor_type;

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

    if (private->fileselection != (Widget) 0)
	if (XtIsManaged(private->fileselection))
	    EnvDwSetCursor(private->fileselection, cursor_type);
		
    return;
    }


_Void  EnvDWQuestion(private, yes_callback, no_callback, identifier)
_WindowPrivate private;
 _Callback yes_callback;

    _Callback no_callback;
 _String identifier;

/*
**++
**  Functional Description:
**	Create the linkbase question dialog box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**
**  Result:
**	None
**
**  Exceptions:
**	DRMHierarchyFetchFailed
**--
*/
    {
    MrmHierarchy    hierarchy;
    MrmType	    *dummy_class;
    Arg		    arglist[5];
    int		    ac = 0;
    XtCallbackRec   callback[2];
    _CString 	    message;
    
    /*
    **  If the Dialog Box isn't created, do it.
    */

    if (private->question_box == (Widget) 0) {

	/*
	**  Open the DRM hierarchy
	*/

	hierarchy = EnvDwGetMainHierarchy();

	/*
	** Register all DRM function names
	*/

	MrmRegisterNames(Register1, Register1Size);

	/*
	** Register private data.
	*/
	
	EnvDWRegisterDRMNames(private);

	/*
	** Fetch the Linkbase Question dialog box.
	*/

	if (MrmFetchWidget(hierarchy, _DrmLbCloseQuestBox, private->main_widget,
	    &private->question_box, (MrmType *) &dummy_class) != MrmSUCCESS)
	
	    _Raise(drm_fetch_error);
    }

    /*
    ** If the Dialog Box isn't realized, do it now.
    */

    if (!XtIsRealized(private->question_box))
	XtRealizeWidget(private->question_box);

    /*
    ** Set the callbacks
    */

    if (yes_callback != (_Callback) NULL) {
    
	callback[0].callback = (XtCallbackProc) yes_callback;
	callback[0].closure = (XtPointer) private;
	callback[1].callback = NULL;

	XtSetArg(arglist[ac], XmNactivateCallback, (XtCallbackRec *) callback); ac++;
	XtSetArg(arglist[ac], XmNuserData, (Opaque *) private); ac++;
	XtSetValues(private->question_yes_btn, arglist, ac);
    }

    if (no_callback != (_Callback) NULL) {

	callback[0].callback = (XtCallbackProc) no_callback;
	callback[0].closure = (XtPointer) private;
	callback[1].callback = NULL;

	XtSetArg(arglist[ac], XmNactivateCallback, (XtCallbackRec *) callback); ac++;
	XtSetArg(arglist[ac], XmNuserData, (Opaque *) private); ac++;
	XtSetValues(private->question_no_btn, arglist, ac);
    }

    /*
    ** Set the message string fetched from UIL
    */

    if (identifier != (_String) 0) {

	EnvDWGetCStringLiteral(identifier, &message);
	XtSetArg(arglist[0], XmNlabelString,(XmString) message);
	XtSetValues(private->question_label, arglist, 1);
	_DeleteCString(&message);
    }

    /*
    ** Set the Cursor to inactive shape
    */
    _SetCursor(private->window, _InactiveCursor);
		
    /*
    ** Popup the dialog box.
    */

    XtManageChild(private->question_box);

    return;
    }
    
	
static _Void  UilCreateQuestionYes(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    
    private->question_yes_btn = w;
    return;
    }

static _Void  UilCreateQuestionNo(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    
    private->question_no_btn = w;
    return;
    }

static _Void  UilCreateQuestionLabel(w, private, reason)
Widget w;
 _WindowPrivate private;

    _Reason reason;

    {
    
    private->question_label = w;
    return;
    }
    

static _Void  UilActivateQuestionCancel(w, private, reason)
Widget w;
 _WindowPrivate private;

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
    XtUnmanageChild(private->question_box);

    /*
    ** Restore the default cursor
    */
    _SetCursor(private->window, _DefaultCursor);

    return;
    }
