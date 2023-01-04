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
** COPYRIGHT (c) 1990, 1991 BY
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
**  Facility:
**
**	LinkWorks Manager
**
**  Version:  V1.0
**
**  Abstract:
**
**	Implements the Show Properties user interface.
**
**  Keywords:
**      SHOW PROPERTIES
**
**  Author:
**      W. Ward Clark, MEMEX Project
**
**  Creation Date:  23-Feb-90
**
**  Modification History:
**--
*/

/*
**  Include Files
*/
#include "hs_include.h"
#include "hs_object.h"
#include "hs_decwindows.h"
#include <time.h>

#ifdef VMS
#include <XNL$def.h>
#else
#include <locale.h>
#include <langinfo.h>
#endif

/*
**  Macro Definitions
*/

#define MaxWidgetListCount 15

/*
**  Type Definitions
*/

/*
**  Table of Contents
*/

/*
**  External Routine Declarations
*/

_DeclareFunction(_Void CreateNewObject,
    (lwk_domain domain, _HsObject *hs_object));

/*
**  Forward Routine Declarations	
*/

_DeclareFunction(_Void EnvShowPropInitialize,
    ());
_DeclareFunction(_Void EnvShowPropDisplayProperties,
    (_HsObject hs_object, _AnyPtr display_data, _Boolean new_object));
_DeclareFunction(static _PropPrivate CreatePropBox,
    (lwk_domain domain, Widget parent_widget, _Boolean new_object));
_DeclareFunction(static _Void AddPropBoxToCycle,
    (_WindowPrivate private, _PropPrivate prop));
_DeclareFunction(static _PropPrivate FindPropBox,
    (_WindowPrivate private, lwk_domain domain));
_DeclareFunction(static _Void DeactivatePropBox,
    (_PropPrivate prop));
_DeclareFunction(static _Void FreePropBox,
    (_PropPrivate prop));

_DeclareFunction(static _Void ResetActionButtonsState,
    (_PropPrivate prop, _Boolean new_object));
_DeclareFunction(static _Void SetTitleBox,
    (Widget shell, lwk_domain domain, _Boolean new_object));
_DeclareFunction(static _Void SetPropBox,
    (_Window window, _HsObject hs_object, _Boolean new_object,
	_PropPrivate prop));
_DeclareFunction(static _Void FillinPropBox,
    (_PropPrivate prop));
_DeclareFunction(static _Void SetPropName,
    (_HsObject hs_object, _PropPrivate prop));
_DeclareFunction(static _Void SetPropIdentifier,
    (_HsObject hs_object, _PropPrivate prop));
_DeclareFunction(static _Void SetPropDesc,
    (_HsObject hs_object, _PropPrivate prop));
_DeclareFunction(static _Void SetPropAuthor,
    (_HsObject hs_object, _PropPrivate prop));
_DeclareFunction(static _Void SetPropDate,
    (_HsObject hs_object, _PropPrivate prop));
_DeclareFunction(static _Void SetPropElements,
    (_HsObject hs_object, lwk_integer element_domain,
	lwk_string property, _PropPrivate prop));
_DeclareFunction(static _Void ShowPropDismiss,
    (_PropPrivate prop_list));
_DeclareFunction(static _Void ShowPropSetCursor,
    (_PropPrivate prop_list, _CursorType cursor_type));

_DeclareFunction(static _Void UilCreateName,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateDescription,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateAuthor,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateDate,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateExtra,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateOk,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateApply,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateReset,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilCreateCancel,
    (Widget w, _PropPrivate prop, _Reason reason));
    
_DeclareFunction(static _Void UilChangeName,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilChangeDescription,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UpdateActionButtonsState,
    (_PropPrivate prop));

_DeclareFunction(static _Void UilActivateOk,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilActivateApply,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void SavePropChanges,
    (_PropPrivate prop));
_DeclareFunction(static _Void UilActivateReset,
    (Widget w, _PropPrivate prop, _Reason reason));
_DeclareFunction(static _Void UilActivateCancel,
    (Widget w, _PropPrivate prop, _Reason reason));

_DeclareFunction(static _Void DateToTm,
    (char *date, tm_t *time));

/*
**  Macro Definitions
*/
#define DRM_lb_index		"linkbase_prop_box"
#define DRM_net_index		"linknet_prop_box"
#define DRM_net_list_index	"net_list_prop_box"
#define DRM_path_index		"path_prop_box"
#define DRM_path_list_index	"path_list_prop_box"

#define _NullXtCString (XmString) 0
#define _NullHisString (lwk_string) lwk_c_null_object

/*
**  Static Data Definitions
*/
static MrmRegisterArg _Constant drm_registrations[] = {
    {"prop_create_name",	(caddr_t) UilCreateName},
    {"prop_create_description",	(caddr_t) UilCreateDescription},
    {"prop_create_author",	(caddr_t) UilCreateAuthor},
    {"prop_create_date",	(caddr_t) UilCreateDate},
    {"prop_create_extra",	(caddr_t) UilCreateExtra},
    {"prop_create_ok",		(caddr_t) UilCreateOk},
    {"prop_create_apply",	(caddr_t) UilCreateApply},
    {"prop_create_reset",	(caddr_t) UilCreateReset},
    {"prop_create_cancel",	(caddr_t) UilCreateCancel},

    {"prop_change_name",	(caddr_t) UilChangeName},
    {"prop_change_description",	(caddr_t) UilChangeDescription},

    {"prop_activate_ok",	(caddr_t) UilActivateOk},
    {"prop_activate_apply",	(caddr_t) UilActivateApply},
    {"prop_activate_reset",	(caddr_t) UilActivateReset},
    {"prop_activate_cancel",	(caddr_t) UilActivateCancel},

    {"env_context_sensitive_help", (caddr_t) EnvHelpPopupHelp}
};

static MrmCount _Constant drm_registrations_size = XtNumber(drm_registrations);

/*
**  Global Data Definitions
*/

/*
**  External Data Declarations
*/

_Void  EnvShowPropInitialize()
/*
**++
**  Functional Description:
**
**	Initializes the Show Properties subsystem.
**
**  Keywords:
**	SHOW PROPERTIES, INITIALIZE
**
**  Formal Parameters:
**
**      None
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      None
**--
*/
{
    /*
    ** Register the Show Properties callback routines so that the
    ** resource manager can resolve them at widget creation time.
    */
    MrmRegisterNames(drm_registrations, drm_registrations_size);

    /*
    ** Return to the caller.
    */
    return;
}



_Void  EnvShowPropSetCursor (private, cursor_type)
_WindowPrivate private;
 _CursorType cursor_type;

/*
**++
**  Functional Description:
**
**	Sets given cursor on properties boxes.
**
**  Keywords:
**	SHOW PROPERTIES, CURSOR, POINTER
**
**  Formal Parameters:
**
**      None
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      None
**--
*/
{
    ShowPropSetCursor(private->linkbase_prop, cursor_type);
    ShowPropSetCursor(private->comp_net_prop, cursor_type);
    ShowPropSetCursor(private->network_prop, cursor_type);
    ShowPropSetCursor(private->comp_path_prop, cursor_type);
    ShowPropSetCursor(private->path_prop, cursor_type);

    return;
}


static _Void  ShowPropSetCursor (prop_list, cursor_type)
_PropPrivate prop_list;
 _CursorType cursor_type;

/*
**++
**  Functional Description:
**
**	Sets given cursor on properties boxes.
**
**  Keywords:
**	SHOW PROPERTIES, CURSOR, POINTER
**
**  Formal Parameters:
**
**      None
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      None
**--
*/
{
    _PropPrivate prop;

    prop = prop_list;
    while (prop != (_PropPrivate) 0) {
	    EnvDwSetCursor(prop->box_widget, cursor_type);
	    prop = prop->next;
    }

    return;
}



_Void  EnvShowPropDisplayProperties(hs_object, display_data, new_object)

    _HsObject	hs_object;

    _AnyPtr	display_data;

    _Boolean	new_object;

/*
**++
**  Functional Description:
**
**	Displays a Properties dialog box for a specific object.
**	The box is preloaded with the current object properties.
**
**  Keywords:
**      SHOW PROPERTIES
**
**  Formal Parameters:
**
**      hs_object :
**          The object (e.g., Linknet) to be displayed.
**
**      display_data :
**          DECwindows properties of the parent window.
**
**      new_object :
**          Indicates whether the object is being created (true) or
**	    already exists (false).
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      {@identifiers or None@}
**--
*/
{
    lwk_domain	    domain;
    _Integer	    type;
    _PropPrivate    prop;
    _WindowPrivate  private;
    _Boolean	    recycled = _False;

    /*
    ** Determine the type of object being displayed.
    */
    _GetValue(hs_object, _P_HisType, hs_c_domain_integer, &type);

    domain = (lwk_domain) type;

    /*
    ** Locate an available properties box of the proper type.
    */
    private = (_WindowPrivate) display_data;

    /*
    ** Try to recycle a modeless box.
    */
    if ((prop = FindPropBox(private, domain)) == (_PropPrivate) 0) {
    
	/*
	** Create a new box if none is available.
	*/
	prop = CreatePropBox(domain, private->main_widget, new_object);

	/*
	** Realize it.
	*/
                
	XtRealizeWidget(prop->box_widget);

	/*
	** Add the box to the cycle list.
	*/
	
	AddPropBoxToCycle(private, prop);
    }
    else {
    
	prop->active = _True;
	recycled = _True;

    }

    /*
    ** Setup the properties box for the requested object.
    */
    SetPropBox(private->window, hs_object, new_object, prop);

    /*
    ** Setup the appropriate title for the box except for the linkbase
    */
    if (domain != lwk_c_domain_linkbase)
	SetTitleBox(prop->box_widget, domain, new_object);
    
    /*
    ** Popup the properties box and wait for the user to do something.
    */
    XtManageChild(prop->box_widget);

    if ((recycled) || (new_object))
        /*
	** Reset the default button
	*/
	ResetActionButtonsState(prop, new_object);
	
    /*
    ** Return to the caller.
    */
    return;
}


_Void  EnvDWShowPropDismiss(private)
_WindowPrivate private;

/*
**++
**  Functional Description:
**
**  Keywords:
**
**  Formal Parameters:
**
**      None
**
**  Result:
**
**      None
**
**  Exceptions:
**
**      None
**--
*/
{
    ShowPropDismiss(private->linkbase_prop);
    ShowPropDismiss(private->comp_net_prop);
    ShowPropDismiss(private->network_prop);
    ShowPropDismiss(private->comp_path_prop);
    ShowPropDismiss(private->path_prop);

    return;
}

static _Void  ShowPropDismiss(prop_list)
_PropPrivate prop_list;

    {
    _PropPrivate prop;

    prop = prop_list;
    while (prop != (_PropPrivate) 0) {
	if (prop->active) {
	    XtUnmanageChild(prop->box_widget);
	    prop->active = _False;
	}
	prop = prop->next;
    }

    return;
    }

static _PropPrivate  CreatePropBox(domain, parent_widget, new_object)

    lwk_domain	    domain;

    Widget	    parent_widget;

    _Boolean	    new_object;

{
    _PropPrivate    prop;
    MrmHierarchy    hierarchy_id;
    char	    *box_index;
    MrmType	    *class_return;
    _Integer	    status;
    Arg		    arglist[2];

    /*
    ** Allocate memory for the Properties Box.
    */
    prop = (_PropPrivate) _AllocateMem(sizeof(_PropPrivateInstance));

    /*
    ** Initialize the box.
    */
    _ClearMem(prop, sizeof(_PropPrivateInstance));

    prop->domain = domain;
    prop->active = _True;  /* It's not yet active, but it will be soon. */

    /*
    ** Open the DRM hierarchy.
    */
    hierarchy_id = EnvDwGetMainHierarchy();

    /*
    ** Register private data.
    */
    EnvDWRegisterDRMNames(prop);

    /*
    ** Use the domain type to determine the specific box to fetch.
    */
    switch (domain) {
        case lwk_c_domain_linkbase:
            box_index = DRM_lb_index;
            break;
        case lwk_c_domain_linknet:
            box_index = DRM_net_index;
            break;
        case lwk_c_domain_comp_linknet:
            box_index = DRM_net_list_index;
            break;
        case lwk_c_domain_path:
            box_index = DRM_path_index;
            break;
        case lwk_c_domain_comp_path:
            box_index = DRM_path_list_index;
            break;
        default:
            _Raise(failure);  /* invalid object */
            break;
    }

    status = MrmFetchWidget(hierarchy_id, box_index, parent_widget,
	&prop->box_widget, (MrmType *) &class_return);

    if (status != MrmSUCCESS)
	_Raise(drm_fetch_error);

    /*
    ** Return the allocated box to the caller.
    */
    return prop;
}

static _Void  AddPropBoxToCycle(private, prop)
_WindowPrivate private;
 _PropPrivate prop;

    {
    _PropPrivate    *prop_box_list;

    switch (prop->domain) {
	case lwk_c_domain_linkbase:
	    prop_box_list = &private->linkbase_prop;
	    break;
	case lwk_c_domain_comp_linknet:
	    prop_box_list = &private->comp_net_prop;
	    break;
	case lwk_c_domain_linknet:
	    prop_box_list = &private->network_prop;
	    break;
	case lwk_c_domain_comp_path:
	    prop_box_list = &private->comp_path_prop;
	    break;
	case lwk_c_domain_path:
	    prop_box_list = &private->path_prop;
	    break;
    }
    /*
    ** List is empty.
    */
    if (*prop_box_list == (_PropPrivate) 0)
	*prop_box_list = prop;
    /*
    ** Add at the beginning of the list.
    */
    else {
	prop->next = *prop_box_list;
	*prop_box_list = prop;
    }
    return;
    }

static _PropPrivate  FindPropBox(private, domain)
_WindowPrivate private;
 lwk_domain domain;

{
    _PropPrivate    prop_box_list,
		    prop;	

    switch (domain) {
	case lwk_c_domain_linkbase:
	    prop_box_list = private->linkbase_prop;
	    break;
	case lwk_c_domain_comp_linknet:
	    prop_box_list = private->comp_net_prop;
	    break;
	case lwk_c_domain_linknet:
	    prop_box_list = private->network_prop;
	    break;
	case lwk_c_domain_comp_path:
	    prop_box_list = private->comp_path_prop;
	    break;
	case lwk_c_domain_path:
	    prop_box_list = private->path_prop;
	    break;
    }
    prop = prop_box_list;
    while ((prop!= (_PropPrivate) 0) && (prop->active))
	prop = prop->next;
	    	
    /*
    ** Return to the caller.
    */
    return (_PropPrivate) prop;
}

static _Void  DeactivatePropBox(prop)
_PropPrivate prop;

{
    /*
    ** Unmanage the Properties Box.
    */
    XtUnmanageChild(prop->box_widget);

    /*
    ** Indicate that the box is available for reuse.
    */
    prop->active = _False;

    /*
    ** Force to True the name_changed and desc_changed values for reuse.
    ** Only the name_changed is forced to True for a linkbase show properties
    ** box, because there is no description field.
    */
    prop->name_changed = _True;

    if (prop->domain != lwk_c_domain_linkbase)
	prop->desc_changed = _True;
    
    /*
    ** If it's a modal box, discard it.
    */
/*    if (prop->new_object)
	FreePropBox(prop);
*/
    /*
    ** Return to the caller.
    */
    return;
}

static _Void  FreePropBox(prop)
_PropPrivate prop;

{
    /*
    ** Destroy the entire dialog box (including child widgets).
    */
    if (prop->box_widget != (Widget) 0)
        XtDestroyWidget(prop->box_widget);

    /*
    ** Free the Properties Box memory.
    */
    _FreeMem(prop);

    /*
    ** Return to the caller.
    */
    return;
}


static _Void  ResetActionButtonsState(prop, new_object)
_PropPrivate prop;
 _Boolean new_object;

/*
**++
**  Functional Description:
**
**	Reset the state of the action buttons in a Properties dialog box.
**	The Cancel button is the default one. The OK, Apply and Reset are
**	insensitive.
**
**  Keywords:
**      SHOW PROPERTIES
**
**--
*/
{
    Arg		arglist[2];
    _Integer	ac = 0;

    if (prop->ok_button != NULL) {
	XtSetArg(arglist[ac], XmNsensitive, _False); ac++;
	XtSetValues(prop->ok_button, arglist, ac);
    }
    
    if ((prop->apply_button != NULL) && (prop->reset_button != NULL)) {
	ac = 0;
	XtSetArg(arglist[ac], XmNsensitive, _False); ac++;
	XtSetValues(prop->apply_button, arglist, ac);
	XtSetValues(prop->reset_button, arglist, ac);
    }
    
    if (prop->cancel_button != NULL) {

	/*
	** Change the default push button
	*/
	ac = 0;
	XtSetArg(arglist[ac], XmNdefaultButton, prop->cancel_button); ac++;
	XtSetValues(prop->box_widget, arglist, ac);

	if (new_object) {

            /*
	    ** Hack ******* for ProcessTraversal **********
	    */
	    
	    ac = 0;
	    XtSetArg(arglist[ac], XmNsensitive, _False); ac++;
	    XtSetValues(prop->cancel_button, arglist, ac);

            /*
	    ** The first text field will receive keyboard events
	    */
	    XmProcessTraversal(prop->name_widget, XmTRAVERSE_CURRENT);

	    ac = 0;
	    XtSetArg(arglist[ac], XmNsensitive, _True); ac++;
	    XtSetValues(prop->cancel_button, arglist, ac);

	}

	else
	    
	    /*
	    ** The Cancel button will receive keyboard events
	    */
	    XmProcessTraversal(prop->cancel_button, XmTRAVERSE_CURRENT);

    }
    return;
}


static _Void  SetTitleBox(widget, domain, new_object)
Widget widget;
 lwk_domain domain;
 _Boolean new_object;

{
    Arg		arglist[1];
    _CString	cs_name;

    switch (domain) {

	case lwk_c_domain_linknet:
	    if (new_object)
		EnvDWGetCStringLiteral(_DwtPropNetCreateBoxTitle, &cs_name);
	    else
		EnvDWGetCStringLiteral(_DwtPropNetShowBoxTitle, &cs_name);
	    break;

	case lwk_c_domain_comp_linknet:
	    if (new_object)
		EnvDWGetCStringLiteral(_DwtPropNetListCreateBoxTitle, &cs_name);
	    else
		EnvDWGetCStringLiteral(_DwtPropNetListShowBoxTitle, &cs_name);
	    break;

	case lwk_c_domain_path:
	    if (new_object)
		EnvDWGetCStringLiteral(_DwtPropPathCreateBoxTitle, &cs_name);
	    else
		EnvDWGetCStringLiteral(_DwtPropPathShowBoxTitle, &cs_name);
	    break;

	case lwk_c_domain_comp_path:
	    if (new_object)
		EnvDWGetCStringLiteral(_DwtPropPathListCreateBoxTitle, &cs_name);
	    else
		EnvDWGetCStringLiteral(_DwtPropPathListShowBoxTitle, &cs_name);
	    break;

    }

    XtSetArg(arglist[0], XmNdialogTitle, cs_name);

    XtSetValues(widget, arglist, 1);

    return;
}
    

static _Void  SetPropBox(window, hs_object, new_object, prop)

    _Window	    window;

    _HsObject	    hs_object;

    _Boolean	    new_object;

    _PropPrivate    prop;

{
    Arg	arglist[2];

    /*
    ** Save LinkWorks Manager's window and object.
    */
    prop->window = window;
    prop->hs_object = hs_object;
    prop->new_object = new_object;

    /*
    ** Fillin the object-specific properties.
    */
    FillinPropBox(prop);

    return;
}


static _Void  FillinPropBox(prop)
_PropPrivate prop;

/*
**++
**  Functional Description:
**
**	Fillin the object-specific properties
**
**  Keywords:
**      SHOW PROPERTIES
**
**--
*/
{
    switch (prop->domain) {

        /*
        ** Linkbase
        */
        case lwk_c_domain_linkbase:
	    SetPropName(prop->hs_object, prop);
	    SetPropIdentifier(prop->hs_object, prop);
            break;

        /*
        ** Linknet
        */
        case lwk_c_domain_linknet:
	    SetPropName(prop->hs_object, prop);
	    SetPropDesc(prop->hs_object, prop);
	    SetPropElements(prop->hs_object, lwk_c_domain_set,
		lwk_c_p_links, prop);
	    SetPropAuthor(prop->hs_object, prop);
	    SetPropDate(prop->hs_object, prop);
            break;

        /*
        ** Linknet List
        */
        case lwk_c_domain_comp_linknet:
	    SetPropName(prop->hs_object, prop);
	    SetPropDesc(prop->hs_object, prop);
	    SetPropAuthor(prop->hs_object, prop);
	    SetPropDate(prop->hs_object, prop);
            break;

        /*
        ** Path
        */
        case lwk_c_domain_path:
	    SetPropName(prop->hs_object, prop);
	    SetPropDesc(prop->hs_object, prop);
	    SetPropElements(prop->hs_object, lwk_c_domain_list,
		lwk_c_p_steps, prop);
	    SetPropAuthor(prop->hs_object, prop);
	    SetPropDate(prop->hs_object, prop);
            break;

        /*
        ** Path List
        */
        case lwk_c_domain_comp_path:
	    SetPropName(prop->hs_object, prop);
	    SetPropDesc(prop->hs_object, prop);
	    SetPropAuthor(prop->hs_object, prop);
	    SetPropDate(prop->hs_object, prop);
            break;
    }

    /*
    ** Return to the caller.
    */
    return;
}

static _Void  SetPropName(hs_object, prop)

    _HsObject	    hs_object;

    _PropPrivate    prop;

{
    _CString 	h_cstring;
    XmString	x_cstring;

    _GetCSProperty(hs_object, lwk_c_p_name, &h_cstring);

    if (h_cstring == _NullObject) {
    
	x_cstring = _StringToXmString((char *) "");
	DXmCSTextSetString (prop->name_widget, x_cstring);
	XmStringFree(x_cstring);
    }
    else {
	
	DXmCSTextSetString (prop->name_widget, (XmString) h_cstring);
	_DeleteCString(&h_cstring);
    }

    prop->name_changed = _False;

    return;
}


static _Void  SetPropIdentifier(hs_object, prop)

    _HsObject	    hs_object;

    _PropPrivate    prop;

{
    lwk_string	h_string;
    XmString	x_cstring;
    
    _GetProperty(hs_object, lwk_c_domain_string, lwk_c_p_identifier,
	&h_string);

    if (h_string == _NullHisString) {
	x_cstring = _StringToXmString((char *) "");
        DXmCSTextSetString(prop->extra_widget, x_cstring);
	XmStringFree(x_cstring);
    }
    else {
    
	x_cstring = _StringToXmString((char *) h_string);
	DXmCSTextSetString(prop->extra_widget, (XmString) x_cstring);
	lwk_delete_string(&h_string);
    }

    return;
}


static _Void  SetPropDesc(hs_object, prop)

    _HsObject	    hs_object;

    _PropPrivate    prop;

{
    _CString	h_cstring;
    XmString    x_cstring;

    _GetCSProperty(hs_object, lwk_c_p_description, &h_cstring);

    if (h_cstring == _NullObject) {
    
	x_cstring = _StringToXmString((char *) "");
        DXmCSTextSetString(prop->desc_widget, x_cstring);
	XmStringFree(x_cstring);
    }
    else {
	
	DXmCSTextSetString (prop->desc_widget, (XmString) h_cstring);
	_DeleteCString(&h_cstring);
    }

    prop->desc_changed = _False;

    return;
}


static _Void  SetPropAuthor(hs_object, prop)
_HsObject hs_object;
 _PropPrivate prop;

    {
    _CString	h_cstring;
    XmString    x_cstring;

    _GetCSProperty(hs_object, lwk_c_p_author, &h_cstring);

    if (h_cstring == _NullObject) {
    
	x_cstring = _StringToXmString((char *) "");
	DXmCSTextSetString (prop->author_widget, x_cstring);
	XmStringFree(x_cstring);
    }
    else {
	
	DXmCSTextSetString(prop->author_widget, (XmString) h_cstring);
	_DeleteCString(&h_cstring);
    }

    return;
    }


static _Void  SetPropDate(hs_object, prop)
_HsObject hs_object;
 _PropPrivate prop;

    {
#ifdef VMS
    static Locale locale_id = (Locale) 0;
    char	*lang_str;
#else
    static int	locale_id = 0;
    char	timestring[100];
    char	*date_time_format;
#endif
    static	XmString format = (XmString) 0;
    XmString	s = (XmString) 0;
    lwk_date	h_date;
    XmString    x_cstring;
    tm_t	time;
    int		length;
    
    _GetProperty(hs_object, lwk_c_domain_date, lwk_c_p_creation_date,
	&h_date);

    if (h_date == (lwk_date) lwk_c_null_object) {
    
	x_cstring = _StringToXmString((char *) "");
        DXmCSTextSetString(prop->date_widget, x_cstring);
	XmStringFree(x_cstring);
    }
    else {

	/*
	** Initialize the XNL locale and the XNL date/time format string
	*/
	
	if (locale_id == 0) {

	    /*
	    ** load the default locale
	    */

#ifdef VMS
	    lang_str = xnl_winsetlocale(LC_TIME, 0, &locale_id,
		XtDisplay(prop->date_widget));

	    if (lang_str == 0) /* fall back on en_US if no default */
		lang_str = xnl_winsetlocale(LC_TIME, "en_US.88591", &locale_id,
		    XtDisplay(prop->date_widget));

	    if (lang_str != 0)
		xnl_free(lang_str);
		
	    EnvDWGetCStringLiteral(_DwtPropDateTimeFormat, &format); 
#else
	    /* 
	    **  Attempt to set the language according to the
	    **	environment variables used by NLS.  Otherwise, we'll
	    **	get the default C locale.
	    */

	    setlocale(LC_TIME, "");
		
            /*
	    ** Set the flag to 1 so next time we won't need to do this
	    */
	    
	    locale_id = 1;
#endif
	}

	/* Fill in time structure from h_date */

	DateToTm(h_date, &time);
						
	/*
	** Get the formatted date time string
	*/
	
#ifdef VMS
	length = xnl_strftime(locale_id, &s, format, &time);

	if (length == 0) {
	    x_cstring = _StringToXmString((char *) "");
	    DXmCSTextSetString(prop->date_widget, x_cstring);
	    XmStringFree(x_cstring);
	}
	else {
	    DXmCSTextSetString(prop->date_widget, (XmString) s);
	    xnl_free(s);
	}
#else
	date_time_format = (char *) nl_langinfo(D_T_FMT);
	if (date_time_format == (char *) 0) {
	    x_cstring = _StringToXmString((char *) h_date);
	} else {
	    strftime(timestring, sizeof(timestring),
		date_time_format, &time);
	    x_cstring = _StringToXmString((char *) timestring);
	}

	DXmCSTextSetString(prop->date_widget, x_cstring);
	XmStringFree(x_cstring);
#endif	

	lwk_delete_date(&h_date);
    }

    return;
    }


static _Void  DateToTm(date, tm)
char *date;
 tm_t *tm;

    {
    char delim;
    int args;
    int tdf_hour;
    int tdf_minute;
    time_t anytime;
    
    /*
    **	Date is formatted in DDIS General Time format:
    **	
    **	    YYYYMMDDhhmmssZ
    **	    YYYYMMDDhhmmss+hhmm
    **	    YYYYMMDDhhmmss-hhmm
    */

    /*
    **  Initialize unused fields
    */

    tm->tm_wday = 0;
    tm->tm_yday = 0;
    tm->tm_isdst = 0;
	     
    /*
    **  Parse the Date into its component fields
    */

    args = sscanf((char *) date, "%4d%2d%2d%2d%2d%2d%c%2d%2d", &tm->tm_year,
	&tm->tm_mon, &tm->tm_mday, &tm->tm_hour, &tm->tm_min, &tm->tm_sec,
	&delim, &tdf_hour, &tdf_minute);

    /*
    **  Make some minor adjustments
    */

    tm->tm_mon--;
    tm->tm_year -= 1900;


    /*
    **  Adjust for any Time Differential Factor (TDF)
    */

    if (args > 7) {
	if (delim == '+') {
	    tm->tm_hour += tdf_hour;
	    tm->tm_min += tdf_minute;
	}
	else if (delim == '-') {
	    tm->tm_hour -= tdf_hour;
	    tm->tm_min -= tdf_minute;
	}
    }

    /*
    **	Call mktime to adjust the values of wday and yday
    */

    anytime = mktime(tm);
    
    return;
}



static _Void  SetPropElements(hs_object, element_domain, property, prop)

    _HsObject	    hs_object;

    lwk_integer	    element_domain;

    lwk_string	    property;

    _PropPrivate    prop;

{
    lwk_set	    contents;
    lwk_integer	    binary_count;
    char            ascii_count[11];
    XmString	    x_cstring;

    _GetProperty(hs_object, element_domain, property, &contents);

    if (contents == (lwk_set) lwk_c_null_object)
	binary_count = 0;
    else
	lwk_get_value(contents, lwk_c_p_element_count, lwk_c_domain_integer,
	    &binary_count);

    sprintf(ascii_count, "%u", binary_count);

    x_cstring = _StringToXmString((char *) ascii_count);
    DXmCSTextSetString(prop->extra_widget, (XmString) x_cstring);
    XmStringFree(x_cstring);

    lwk_delete(&contents);

    return;
}


static _Void  UilCreateName(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

    {
    prop->name_widget = w;
    
    return;
    }

static _Void  UilCreateDescription(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->desc_widget = w;
    return;
}

static _Void  UilCreateAuthor(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->author_widget = w;
    return;
}

static _Void  UilCreateDate(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->date_widget = w;
    return;
}

static _Void  UilCreateExtra(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->extra_widget = w;
    return;
}

static _Void  UilCreateOk(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->ok_button = w;
    return;
}

static _Void  UilCreateApply(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->apply_button = w;
    return;
}

static _Void  UilCreateReset(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->reset_button = w;
    return;
}

static _Void  UilCreateCancel(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    prop->cancel_button = w;
    return;
}


static _Void  UilChangeName(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    if (!(prop->name_changed)) {
	UpdateActionButtonsState(prop);
	prop->name_changed = _True;
    }

    return;
}       


static _Void  UilChangeDescription(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    if (!(prop->desc_changed)) {
	if (prop->domain != lwk_c_domain_linkbase)
	    UpdateActionButtonsState(prop);
	    
        prop->desc_changed = _True;
    }
    return;
}


static _Void  UpdateActionButtonsState(prop)
_PropPrivate prop;

{
    Arg		arglist[2];
    _Integer	ac = 0;

    if (prop->ok_button != NULL) {
        /*
	** The ok button is set to sensitive
	*/
	XtSetArg(arglist[ac], XmNsensitive, _True); ac++;
	XtSetValues(prop->ok_button, arglist, ac);

        /*
	** Change the default push button
	*/
	ac = 0;
	XtSetArg(arglist[ac], XmNdefaultButton, prop->ok_button); ac++;
	XtSetValues(prop->box_widget, arglist, ac);
    }
    
    if ((prop->apply_button != NULL) && (prop->reset_button != NULL)) {
	ac = 0;
	XtSetArg(arglist[ac], XmNsensitive, _True); ac++;
	XtSetValues(prop->apply_button, arglist, ac);
	XtSetValues(prop->reset_button, arglist, ac);
    }
    
    return;
}


static _Void  UilActivateOk(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    /*
    ** Save the changes.
    */
    SavePropChanges(prop);
    
    /*
    ** Deactivate the Properties Box.
    */
    DeactivatePropBox(prop);

    /*
    ** Return to the caller.
    */
    return;
}


static _Void  UilActivateApply(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    /*
    ** Save the changes.
    */
    SavePropChanges(prop);

    /*
    ** If we are in a Create dialog box, create a new object and reset the box
    ** with empty fields.
    */
    if (prop->new_object) {
	CreateNewObject(prop->domain, &prop->hs_object);
	FillinPropBox(prop);
    }
    
    /*
    ** Reset the state of the active buttons.
    */
    ResetActionButtonsState(prop, prop->new_object);

    return;
}


static _Void  SavePropChanges(prop)
_PropPrivate prop;

{
    XmString	x_cstring;

    /*
    ** Update the object name if it has been changed.
    */               
    if (prop->name_changed) {
    	x_cstring = (XmString) DXmCSTextGetString (prop->name_widget);

	_SetCSProperty(prop->hs_object, lwk_c_p_name, (_CString) x_cstring,
	    lwk_c_set_property);

	XmStringFree(x_cstring);
        x_cstring = _NullXtCString;
    }

    /*
    ** Update the object description if it has been changed.
    */
    if (prop->desc_changed) {
	x_cstring = (XmString) DXmCSTextGetString (prop->desc_widget);

	_SetCSProperty(prop->hs_object, lwk_c_p_description,
	    (_CString) x_cstring, lwk_c_set_property);

	XmStringFree(x_cstring);
        x_cstring = _NullXtCString;
    }

    /*
    ** Let the object manager decide when to actually store any changes.
    */
    if (prop->name_changed || prop->desc_changed)
	if (prop->new_object)
	    _Update(prop->window, prop->hs_object, _NewObject);
	else
	    _Update(prop->window, prop->hs_object, _ModifiedProp);

    prop->name_changed = _False;
    prop->desc_changed = _False;

    return;
}


static _Void  UilActivateReset(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    /*
    ** Reset the last saved changes.
    */
    FillinPropBox(prop);

    /*
    ** Reset the sate of the action buttons.
    */
    ResetActionButtonsState(prop, prop->new_object);
    
    return;
}


static _Void  UilActivateCancel(w, prop, reason)
Widget w;
 _PropPrivate prop;
 _Reason reason;

{
    lwk_object	hisobject;

    /*
    ** Abort the creation of a new object.
    */
    if (prop->new_object) {
	/*
	** Get the his object.
	*/
	_GetValue(prop->hs_object, _P_HisObject, hs_c_domain_lwk_object,
		&hisobject);
	/*
	** Delete the his object.
	*/
	lwk_delete(&hisobject);

	/*
	** Delete the hs object.
	*/
	_Delete(&(prop->hs_object));
    }
	
    /*
    ** Deactivate the Properties Box.
    */
    DeactivatePropBox(prop);

    /*
    ** Return to the caller.
    */
    return;
}

