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
**	LinkWorks Manager User Interface
**
**  Version: V1.0
**
**  Abstract:
**	Window Object DECwindows User Interface routines.
**
**  Keywords:
**	{keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Patricia Avigdor
**
**  Creation Date: 15-Nov-89
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


_WindowPrivate  EnvDWWindowCreatePrivateData(window, parent)
_Window window;
 _Widget parent;

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
    _WindowPrivate windowprivate;

    _StartExceptionBlock

    /*
    ** Allocate memory for private data.
    */

    windowprivate = (_WindowPrivate)
	_AllocateMem(sizeof(_WindowPrivateInstance));

    /*
    **	Initialize private data
    */

    _ClearMem(windowprivate, sizeof(_WindowPrivateInstance));

    windowprivate->parent	    = (Widget) parent;
    windowprivate->window	    = (_Window) window;
    windowprivate->new_window	    = _False;

    /*
    ** Set the Display data property on the window.
    */

    _SetValue(window, _P_DisplayData, hs_c_domain_any_ptr, &windowprivate,
	hs_c_set_property);

    /*
    ** If any exception is raised, free the allocated storage and reraise the
    ** exception.
    */

    _Exceptions
        _WhenOthers
	    if (windowprivate != (_WindowPrivate) 0)
		_FreeMem(windowprivate);
            _Reraise;
	
    _EndExceptionBlock

    return windowprivate;
    }


_Void  EnvDWWindowDeletePrivateData(windowprivate)
_WindowPrivate windowprivate;

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

    if (windowprivate == (_WindowPrivate) _NullObject)
	return;
	
    /*
    ** Destroy all realized dialog boxes.
    */

    if (windowprivate->message_box != (Widget) 0)
        XtDestroyWidget(windowprivate->message_box);

    if (windowprivate->help_box != (Widget) 0)
        XtDestroyWidget(windowprivate->help_box);

#ifdef VMS /* Use DXmHelpSystem (hyperhelp) */
    if (windowprivate->help_context != (Opaque) 0)
	DXmHelpSystemClose(windowprivate->help_context,
	    (void *) NULL, (Opaque) NULL);
#endif /* #ifdef VMS (hyperhelp) */

    if (windowprivate->fileselection != (Widget) 0)
        XtDestroyWidget(windowprivate->fileselection);
	
    /*
    ** Free the windowprivate structure.
    */

    _FreeMem(windowprivate);

    return;
    }


_Void  EnvDWSetEditButtons(private, sensitive)
_WindowPrivate private;
 _Boolean sensitive;

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
    Arg arglist[2];

    XtSetArg(arglist[0], XmNsensitive, sensitive);

    XtSetValues(private->cut_button, arglist, 1);
    XtSetValues(private->copy_button, arglist, 1);
    XtSetValues(private->delete_button, arglist, 1);

    }


_Void  EnvDWSetExpandButtonLabel(private, select_data)
_WindowPrivate private;

    _SelectData select_data;

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
    Arg		    arglist[3];
    _Integer	    count = 0;
    _IsExpandable   entry_state;

    /*
    **  Check if something is selected
    */

    if (select_data == (_SelectData) 0) {

	if (private->expand_label == (_CString) 0)
	    EnvDWGetCStringLiteral(_DwtWindowExpand, &(private->expand_label));
	
	if (private->expand_keysym == (KeySym) 0)
	    EnvDWGetKeySym(_DwtWindowExpandKeySym, &(private->expand_keysym));

	XtSetArg(arglist[count], XmNlabelString, private->expand_label);
	count++;

	XtSetArg(arglist[count], XmNmnemonic, private->expand_keysym);
	count++;

	XtSetArg(arglist[count], XmNsensitive, _False);
	count++;
    }

    else {

	/*
	**  Determine what the state of the selected entry is
	*/
	
	entry_state = EnvSvnWindowIsExpandable(private,
	    select_data->svn_data[0], select_data->entry[0],
	    select_data->component[0]);

	/*
	**  Update the menu label accordingly
	*/
	
	switch (entry_state) {

	    case _CollapseEntry :
		if (private->collapse_label == (_CString) 0)
		    EnvDWGetCStringLiteral(_DwtWindowCollapse,
			&(private->collapse_label));
		if (private->collapse_keysym == (KeySym) 0)
		    EnvDWGetKeySym(_DwtWindowCollapseKeySym,
			&(private->collapse_keysym));
		XtSetArg(arglist[count], XmNlabelString,
		    private->collapse_label); count++;
		XtSetArg(arglist[count], XmNmnemonic,
		    private->collapse_keysym); count++;
		XtSetArg(arglist[count], XmNsensitive, _True); count++;
		break;

	    case _ExpandEntry :
		if (private->expand_label == (_CString) 0)
		    EnvDWGetCStringLiteral(_DwtWindowExpand,
			&(private->expand_label));
		if (private->expand_keysym == (KeySym) 0)
		    EnvDWGetKeySym(_DwtWindowExpandKeySym,
			&(private->expand_keysym));
		XtSetArg(arglist[count], XmNlabelString,
		    private->expand_label); count++;
		XtSetArg(arglist[count], XmNmnemonic,
		    private->expand_keysym); count++;
		XtSetArg(arglist[count], XmNsensitive, _True); count++;
		break;

	    case _DimItem :
	    case _EmptyEntry :
		if (private->expand_label == (_CString) 0)
		    EnvDWGetCStringLiteral(_DwtWindowExpand,
			&(private->expand_label));
		if (private->expand_keysym == (KeySym) 0)
		    EnvDWGetKeySym(_DwtWindowExpandKeySym,
			&(private->expand_keysym));
		XtSetArg(arglist[count], XmNlabelString,
		    private->expand_label); count++;
		XtSetArg(arglist[count], XmNmnemonic,
		    private->expand_keysym); count++;
		XtSetArg(arglist[count], XmNsensitive, _False); count++;
		break;
	}
    }

    XtSetValues(private->expand_button, arglist, count);

    return;
    }


_Void  EnvDWWindowRaiseWindow(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**	Set the iconic state of the shell and raise the window.
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
    XEvent event;
    Screen *scrn = XtScreen(private->shell);
    Display *dpy = XtDisplay(private->shell);

    /*
    **  Make sure the window is not iconized
    */
         
    if (!EnvDWIsXUIWMRunning(private->shell)) {
	/*
        **  Assume that we have an ICCCM compliant WM running
	*/
	
        event.xclient.type = ClientMessage;
        event.xclient.display = dpy;
        event.xclient.window = XtWindow(private->shell);
        event.xclient.message_type = XmInternAtom (dpy, "WM_CHANGE_STATE", 
						   False);
        event.xclient.format = 32;
        event.xclient.data.l[0] = NormalState;
        if (event.xclient.message_type != None)
	    XSendEvent(dpy, RootWindowOfScreen(scrn), False,
		       SubstructureRedirectMask|SubstructureNotifyMask, &event);
    }

    else {

	/*
        **  Special case XUI WM
	*/
	
        XWMHints hints;
        hints.flags = StateHint;
        hints.initial_state = NormalState;

        XSetWMHints(dpy, XtWindow(private->shell), &hints);

	XtPopup(private->shell, XtGrabNone);
    }

    /*
    ** Raise the window.
    */
    
    XRaiseWindow(XtDisplay(private->shell), XtWindow(private->shell));

    return;
}
