/*
** COPYRIGHT (c) 1988, 1989, 1990, 1991 BY
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
**	LinkWorks Services
**
**  Version: X0.1
**
**  Abstract:
**	LWK User Interface object methods 
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_dxmui.h"

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _DXmUiTypeInstance;      /* Instance record for Type */
_Global _DXmUiType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeDXmUiInstance;
static _DXmUiPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties = DXmUiPropertyNameTable;


void  LwkOpDXmUi(dxmui)
_DXmUi dxmui;

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
    return;
    }


_DXmUi  LwkOpDXmUiCreate(type, appl_name, create_menu, default_accelerators, main_window, menu_entry)
_Type type;
 _CString appl_name;
 _Boolean create_menu;

    _Boolean default_accelerators;
 _Widget main_window;
 _Widget menu_entry;

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
    _DXmUi volatile new_dxmui;
    _DXmEnvState volatile currency;

    /*
    **  Create an object instance
    */

    currency = (_DXmEnvState) _NullObject;
    new_dxmui = (_DXmUi) _NullObject;

    _StartExceptionBlock

    /*
    **  Create the DXmUi Object.
    */

    new_dxmui = (_DXmUi) _Create(MyType);

    /*
    **  Call the DECwindows module create routine to do all the grungy stuff.
    */

    _PrivateData_of(new_dxmui) = (_AnyPtr) LwkDXmCreate(new_dxmui, appl_name,
	create_menu, default_accelerators, main_window, menu_entry);

    /*
    **  Create a DECwindows Currency Object and establish currency.
    */

    currency = (_DXmEnvState) _Create(_TypeDXmEnvState);

    _SetValue(currency, _P_MainWidget, lwk_c_domain_integer, &main_window,
	lwk_c_set_property);

    _SetValue(new_dxmui, _P_EnvironmentState, lwk_c_domain_environment_state,
	(_AnyPtr) &currency, lwk_c_set_property);

    /*
    **	If any exceptions are raised, delete the new objects then reraise the
    **	exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&new_dxmui);
	    _Delete(&currency);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the object to the caller
    */

    return new_dxmui;
    }


void  LwkOpDXmUiInitialize(dxmui, proto, aggregate)
_DXmUi dxmui;
 _DXmUi proto;
 _Boolean aggregate;

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
    **  Initialize the properties defined by our type
    */

    _PrivateData_of(dxmui) = (_AnyPtr) _NullObject;

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(dxmui, proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_DXmUi) _NullObject) {
    }

    return;
    }


void  LwkOpDXmUiFree(dxmui)
_DXmUi dxmui;

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
    **  Free the private storage used by our Type.
    */

    LwkDXmDelete(_PrivateData_of(dxmui));

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(dxmui, MyType);

    return;
    }


_Set  LwkOpDXmUiEnumProps(dxmui)
_DXmUi dxmui;

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
    _Set set;

    /*
    **  Ask our supertype to enumerate its properties.
    */

    set = (_Set) _EnumerateProperties_S(dxmui, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpDXmUiIsMultiValued(dxmui, property)
_DXmUi dxmui;
 _String property;

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
    int index;
    _Boolean answer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	answer = _IsMultiValued_S(dxmui, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpDXmUiGetValueDomain(dxmui, property)
_DXmUi dxmui;
 _String property;

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
    int index;
    _Domain domain;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	domain = (_Domain) _GetValueDomain_S(dxmui, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpDXmUiGetValue(dxmui, property, domain, value)
_DXmUi dxmui;
 _String property;
 _Domain domain;

    _AnyPtr value;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_GetValue_S(dxmui, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_List  LwkOpDXmUiGetValueList(dxmui, property)
_DXmUi dxmui;
 _String property;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	return (_List) _GetValueList_S(dxmui, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpDXmUiSetValue(dxmui, property, domain, value, flag)
_DXmUi dxmui;
 _String property;
 _Domain domain;

    _AnyPtr value;
 _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(dxmui, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpDXmUiSetValueList(dxmui, property, values, flag)
_DXmUi dxmui;
 _String property;
 _List values;

    _SetFlag flag;

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
    int index;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(dxmui, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpDXmUiHighlight(dxmui)
_DXmUi dxmui;

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
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmHighlight(_PrivateData_of(dxmui));

    return;
    }


void  LwkOpDXmUiShowLinks(dxmui, links, opr_ids, opr_names, directions, follow_type, iff_visible, update, closure)
_DXmUi dxmui;
 _List links;
 _List opr_ids;

     _List opr_names;
 _List directions;
 _FollowType follow_type;

     _Boolean iff_visible;
 _Boolean update;
 _Closure closure;

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
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmShowLinks(_PrivateData_of(dxmui), links, opr_ids,
	opr_names, directions, follow_type, iff_visible, update, closure);

    return;
    }


void  LwkOpDXmUiShowHistory(dxmui, steps, origin_opr_ids, origin_opr_names, dest_opr_ids, dest_opr_names, iff_visible, closure)
_DXmUi dxmui;
 _List steps;
 _List origin_opr_ids;

     _List origin_opr_names;
 _List dest_opr_ids;
 _List dest_opr_names;

     _Boolean iff_visible;
 _Closure closure;

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
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmShowHistory(_PrivateData_of(dxmui), steps, origin_opr_ids,
	origin_opr_names, dest_opr_ids, dest_opr_names, iff_visible, closure);

    return;
    }


void  LwkOpDXmUiCompleteLink(dxmui, link, iff_visible, closure)
_DXmUi dxmui;
 _Link link;
 _Boolean iff_visible;

    _Closure closure;

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
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmCompleteLink(_PrivateData_of(dxmui), link, iff_visible, closure);

    return;
    }


void  LwkOpDXmUiDisplayMessage(dxmui, status, count)
_DXmUi dxmui;
 _Status *status;
 _Integer count;

/*
**++
**  Functional Description:
**	Create the message box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	main: parent widget id.
**	message: pointer to the message box widget to return.
**	status: status code to display.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    /*
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmMessageBox(_PrivateData_of(dxmui), status, count);

    return;
    }


void  LwkOpDXmUiDisplayWIP(dxmui, subtype_string)
_DXmUi dxmui;
 _String subtype_string;

/*
**++
**  Functional Description:
**	Create the message box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	main: parent widget id.
**	message: pointer to the message box widget to return.
**	status: status code to display.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    /*
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmDisplayWIP(_PrivateData_of(dxmui), subtype_string);

    return;
    }


void  LwkOpDXmUiRemoveWIP(dxmui, subtype_string)
_DXmUi dxmui;
 _String subtype_string;

/*
**++
**  Functional Description:
**	Create the message box.
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Arguments:
**	main: parent widget id.
**	message: pointer to the message box widget to return.
**	status: status code to display.
**
**  Result:
**	None
**
**  Exceptions:
**	None
**--
*/
    {
    /*
    **  Let the DECwindows-dependent module do the grungy work
    */

    LwkDXmRemoveWIP(_PrivateData_of(dxmui), subtype_string);

    return;
    }
