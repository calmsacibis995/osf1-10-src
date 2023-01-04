/*
** COPYRIGHT (c) 1990, 1991, 1992 BY
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
**  Version: V1.0
**
**  Abstract:
**	DXmEnvState object methods
**
**  Keywords:
**	{@keyword-list-or-none@}
**
**  Environment:
**	{@environment-description@}
**
**  Author:
**	Doug Rayner
**	Patricia Avigdor
**
**  Creation Date: 7-Jul-88
**
**  Modification History:
**
**  001	lg  5-Mar-90	XUI->Motif conversion
**
**--
*/


/*
**  Include Files
*/

#include "his_include.h"
#include "lwk_dxmenvstate.h"
#include "his_dwui_decwindows_m.h"

#ifdef VMS

#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <decw$include/Xatom.h>

#else
#include <Xm/XmP.h>
#include <Mrm/MrmPublic.h>
#include <X11/Xatom.h>
#endif /* VMS */

/*
**  Macro Definitions
*/

#define _PropertyFormat 8
#define _MessageFormat 32

/*
** See explanation in his_ui_navigation.h about Client messages. This is 
** due to a bug in the Xlib. As long as you are using a message format of 32,
** the message data value are interpreted as long (64 on Alpha/OSF1).
*/
#if defined (__alpha)
#define _MaxMessageLength 40
#else
#define _MaxMessageLength 20
#endif /* __alpha */

#define _MaxPropertyLength 2048

#define _NotificationMessageType "_DEC_LWK_CHANGE_NOTIFICATION"
#define _ValidationProperty "_DEC_LWK_VALID"

#define _PropertyTypeDirect XA_PRIMARY
#define _PropertyTypeIndirect XA_SECONDARY

#define VFakeNwindow "window"

#define _PrimaryScreen 0

#define _RootWindow(Widget) \
	    RootWindow(XtDisplay(Widget), _PrimaryScreen)

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static void GetValueProperty,
    (_DXmEnvState currency, _String property, _Domain domain, _AnyPtr value));
_DeclareFunction(static _List_of(_Domain) GetValueListProperty,
    (_DXmEnvState currency, _String property));
_DeclareFunction(static void SetValueProperty,
    (_DXmEnvState currency, _String property, _Domain domain, _AnyPtr value,
	_SetFlag flag));
_DeclareFunction(static void SetValueListProperty,
    (_DXmEnvState currency, _String property, _List values, _SetFlag flag));
_DeclareFunction(static _Termination FindProperty,
    (_String property, _Set prop_set, _Domain domain, _Property *prop));
_DeclareFunction(static void GenerateNotificationAtoms,
    (_DXmEnvState currency));
_DeclareFunction(static _Termination AddNotificationAtom,
    (_DXmEnvState currency, _Set notifications, _Domain domain,
	_String *property));
_DeclareFunction(static Atom NameToAtom,
    (_DXmEnvState currency, _String property));
_DeclareFunction(static void SaveKnownAtom,
    (_DXmEnvState currency, _String property, Atom atom));
_DeclareFunction(static _Boolean RetrieveProperty,
    (_DXmEnvState currency, Window window, Atom atom, Atom *type,
	_AnyPtr *pointer, _Integer *size));
_DeclareFunction(static _Boolean StoreProperty,
    (_DXmEnvState currency, Window window, Atom atom, Atom type,
	_AnyPtr pointer, _Integer size, _SetFlag flag));
_DeclareFunction(static void RegisterNotificationHandler,
    (_DXmEnvState currency));
_DeclareFunction(static void PropertyChanged,
    (Widget w, _DXmEnvState currency, XPropertyEvent *event,
	Boolean *continue_to_dispatch));
_DeclareFunction(static void ReceiveMessage,
    (Widget w, _DXmEnvState currency, XClientMessageEvent *event,
	Boolean *continue_to_dispatch));
_DeclareFunction(static void NotifyOfCurrencyChange,
    (_DXmEnvState currency, _Property property));
_DeclareFunction(static _Boolean ExportObjectId,
    (_Persistent volatile persistent, _VaryingString volatile *encoding,
    _Integer *size));
_DeclareFunction(static _Termination SearchAtom,
    (Atom atom, _Set prop_set, _Domain domain, _Property *prop));
_DeclareFunction(static int LocalErrorHandler,
    (Display *dpy, XErrorEvent *event));

/*
**  Global Data Definitions
*/

_Global _DXmEnvStateTypeInstance;      /* Instance record for Type */
_Global _DXmEnvStateType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  External Routine Declarations
*/

_DeclareFunction(Widget VFakeCreate,
    (Widget pW, char *nameP, Arg *argsP, int argCnt));

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeDXmEnvStateInstance;
static _DXmEnvStatePropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    DXmEnvStatePropertyNameTable;

/*
**  List header for DXmEnvState objects
*/

static _DXmEnvState DXmEnvStateList = (_DXmEnvState) _NullObject;


void  LwkOpDXmState(currency)
_DXmEnvState currency;

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


void  LwkOpDXmStateInitialize(currency, proto, aggregate)
_DXmEnvState currency;
 _DXmEnvState proto;

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

    _ClearValue(&_MainWidget_of(currency), lwk_c_domain_integer);
    _ClearValue(&_Address_of(currency), lwk_c_domain_integer);
    _ClearValue(&_Notifications_of(currency), lwk_c_domain_set);
    _ClearValue(&_MessageCb_of(currency), lwk_c_domain_routine);
    _ClearValue(&_NotificationCb_of(currency), lwk_c_domain_routine);
    _ClearValue(&_UserData_of(currency), lwk_c_domain_closure);

    _ClearValue(&_Next_of(currency), lwk_c_domain_integer);
    _ClearValue(&_MessageWidget_of(currency), lwk_c_domain_integer);
    _ClearValue(&_CurrencyMessageType_of(currency), lwk_c_domain_integer);
    _ClearValue(&_ValidationProperty_of(currency), lwk_c_domain_integer);
    _ClearValue(&_KnownAtoms_of(currency), lwk_c_domain_set);
    _ClearValue(&_NotificationAtoms_of(currency), lwk_c_domain_set);

    /*
    **	Link the new DXmEnvState into the list of DXmEnvState Objects created by
    **	this application.
    */

    _BeginCriticalSection

    _Next_of(currency) = DXmEnvStateList;
    DXmEnvStateList = currency;

    _EndCriticalSection

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(currency, proto, aggregate, MyType);

    /*
    **  If a prototype object was provided, copy properties from it.
    */

    if (proto != (_DXmEnvState) _NullObject) {
	_CopyValue(&_Notifications_of(proto), &_Notifications_of(currency),
	    lwk_c_domain_set);
	_CopyValue(&_MessageCb_of(proto),
	    &_MessageCb_of(currency), lwk_c_domain_routine);
	_CopyValue(&_NotificationCb_of(proto),
	    &_NotificationCb_of(currency), lwk_c_domain_routine);
	_CopyValue(&_UserData_of(proto), &_UserData_of(currency),
	    lwk_c_domain_closure);

	_CopyValue(&_CurrencyMessageType_of(proto),
	    &_CurrencyMessageType_of(currency), lwk_c_domain_integer);
	_CopyValue(&_ValidationProperty_of(proto),
	    &_ValidationProperty_of(currency), lwk_c_domain_integer);
	_CopyValue(&_KnownAtoms_of(proto), &_KnownAtoms_of(currency),
	    lwk_c_domain_set);
	_CopyValue(&_NotificationAtoms_of(proto),
	    &_NotificationAtoms_of(currency), lwk_c_domain_set);
    }

    return;
    }

void  LwkOpDXmStateFree(currency)
_DXmEnvState currency;

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
    **	Unlink this DXmEnvState from the list of DXmEnvState Objects created by
    **	this application.
    */

    _BeginCriticalSection

    if (DXmEnvStateList != (_DXmEnvState) _NullObject) {
	_DXmEnvState previous;

	if (currency == DXmEnvStateList)
	    DXmEnvStateList = _Next_of(currency);
	else {
	    previous = DXmEnvStateList;

	    while (_Next_of(previous) != (_DXmEnvState) _NullObject) {
		if (_Next_of(previous) == currency) {
		    _Next_of(previous) = _Next_of(currency);
		    break;
		}

		previous = _Next_of(previous);
	    }
	}
    }

    _EndCriticalSection

    /*
    ** Clear the validation property on the Message Widget Window
    */

    if (_MessageWidget_of(currency) != (_Widget) 0)
	StoreProperty(currency, _Address_of(currency),
	    _ValidationProperty_of(currency), _PropertyTypeDirect,
	    (_AnyPtr) 0, 0, lwk_c_delete_property);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_MainWidget_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_Address_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_Notifications_of(currency), lwk_c_domain_set);
    _DeleteValue(&_MessageCb_of(currency), lwk_c_domain_routine);
    _DeleteValue(&_NotificationCb_of(currency), lwk_c_domain_routine);
    _DeleteValue(&_UserData_of(currency), lwk_c_domain_closure);

    _DeleteValue(&_Next_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_MessageWidget_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_CurrencyMessageType_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_ValidationProperty_of(currency), lwk_c_domain_integer);
    _DeleteValue(&_KnownAtoms_of(currency), lwk_c_domain_set);
    _DeleteValue(&_NotificationAtoms_of(currency), lwk_c_domain_set);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(currency, MyType);

    return;
    }


_Set  LwkOpDXmStateEnumProps(currency)
_DXmEnvState currency;

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

    set = (_Set) _EnumerateProperties_S(currency, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpDXmStateIsMultiValued(currency, property)
_DXmEnvState currency;
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
	answer = _IsMultiValued_S(currency, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
	    case _AddressIndex :
	    case _MessageCbIndex :
	    case _NotificationCbIndex :
	    case _UserDataIndex :
		answer = _False;
		break;

	    case _NotificationsIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpDXmStateGetValueDomain(currency, property)
_DXmEnvState currency;
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
	domain = (_Domain) _GetValueDomain_S(currency, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
	    case _AddressIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _UserDataIndex :
		domain = lwk_c_domain_closure;
		break;

	    case _NotificationsIndex :
		domain = lwk_c_domain_set;
		break;

	    case _MessageCbIndex :
	    case _NotificationCbIndex :
		domain = lwk_c_domain_routine;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpDXmStateGetValue(currency, property, domain, value)
_DXmEnvState currency;
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
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the DXmEnvState properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _GetValue_S(currency, property, domain, value, MyType);
	else
	    GetValueProperty(currency, property, domain, value);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_MainWidget_of(currency), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _AddressIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Address_of(currency), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _NotificationsIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_Notifications_of(currency), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _MessageCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_MessageCb_of(currency), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _NotificationCbIndex :
		if (_IsDomain(domain, lwk_c_domain_routine))
		    _CopyValue(&_NotificationCb_of(currency), value,
			lwk_c_domain_routine);
		else
		    _Raise(inv_domain);

		break;

	    case _UserDataIndex :
		if (_IsDomain(domain, lwk_c_domain_closure))
		    _CopyValue(&_UserData_of(currency), value,
			lwk_c_domain_closure);
		else
		    _Raise(inv_domain);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_List  LwkOpDXmStateGetValueList(currency, property)
_DXmEnvState currency;
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
    _List value_list;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the DXmEnvState properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    value_list = (_List) _GetValueList_S(currency, property, MyType);
	else
	    value_list = GetValueListProperty(currency, property);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
		_ListValue(&_MainWidget_of(currency), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _AddressIndex :
		_ListValue(&_Address_of(currency), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _NotificationsIndex :
		_ListValue(&_Notifications_of(currency), &value_list,
		    lwk_c_domain_set);
		break;

	    case _MessageCbIndex :
		_ListValue(&_MessageCb_of(currency), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _NotificationCbIndex :
		_ListValue(&_NotificationCb_of(currency), &value_list,
		    lwk_c_domain_routine);
		break;

	    case _UserDataIndex :
		_ListValue(&_UserData_of(currency), &value_list,
		    lwk_c_domain_closure);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }
                
    return value_list;
    }


void  LwkOpDXmStateSetValue(currency, property, domain, value, flag)
_DXmEnvState currency;
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
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the DXmEnvState properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _SetValue_S(currency, property, domain, value, flag, MyType);
	else
	    SetValueProperty(currency, property, domain, value, flag);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
		_SetSingleValuedProperty(&_MainWidget_of(currency),
		    lwk_c_domain_integer, domain, value, flag, _False);

		RegisterNotificationHandler(currency);

		break;

	    case _AddressIndex :
		_Raise(property_is_readonly);
		break;

	    case _NotificationsIndex :
		_SetMultiValuedProperty(&_Notifications_of(currency),
		    lwk_c_domain_string, domain, value, flag, _False, _True);

		GenerateNotificationAtoms(currency);

		break;

	    case _MessageCbIndex :
		_SetSingleValuedProperty(&_MessageCb_of(currency),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _NotificationCbIndex :
		_SetSingleValuedProperty(&_NotificationCb_of(currency),
		    lwk_c_domain_routine, domain, value, flag, _False);
		break;

	    case _UserDataIndex :
		_SetSingleValuedProperty(&_UserData_of(currency),
		    lwk_c_domain_closure, domain, value, flag, _False);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpDXmStateSetValueList(currency, property, values, flag)
_DXmEnvState currency;
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
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the DXmEnvState properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _SetValueList_S(currency, property, values, flag, MyType);
	else
	    SetValueListProperty(currency, property, values, flag);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _MainWidgetIndex :
		_SetSingleValuedProperty(&_MainWidget_of(currency),
		    lwk_c_domain_integer, lwk_c_domain_list, &values, flag,
		    _True);

		RegisterNotificationHandler(currency);

		break;

	    case _AddressIndex :
		_Raise(property_is_readonly);
		break;

	    case _NotificationsIndex :
		_SetMultiValuedProperty(&_Notifications_of(currency),
                    lwk_c_domain_string, lwk_c_domain_list, &values, flag,
		    _True, _True);

		GenerateNotificationAtoms(currency);

		break;

	    case _MessageCbIndex :
		_SetSingleValuedProperty(&_MessageCb_of(currency),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _NotificationCbIndex :
		_SetSingleValuedProperty(&_NotificationCb_of(currency),
		    lwk_c_domain_routine, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    case _UserDataIndex :
		_SetSingleValuedProperty(&_UserData_of(currency),
		    lwk_c_domain_closure, lwk_c_domain_list, &values, flag,
		    _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpDXmStateSendMessage(currency, address, size, message)
_DXmEnvState currency;
 _Integer address;

    _Integer size;
 _AnyPtr message;

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
    Status status;
    Atom valid_type;
    _Boolean *valid_ptr;
    _Integer valid_size;
    XClientMessageEvent xmessage;
    int *handler;

    _StartExceptionBlock

    /*									   
    **	Register and error handler (to supress messages)
    */    

    handler = (int *) XSetErrorHandler(LocalErrorHandler);

    /*
    **	Verify the address by checking for the ValidationProperty on that
    **	Window
    */

    if (!RetrieveProperty(currency, address, _ValidationProperty_of(currency),
	    &valid_type, &valid_ptr, &valid_size)) 
	_Raise(inv_argument);
    
    if (!(valid_type == _PropertyTypeDirect && valid_size == sizeof(_Boolean)
	    && *valid_ptr))
	_Raise(inv_argument);

    /*
    ** If size is negative, return (this allows a mechanism to do a quick
    ** validity check on an address).
    */

    if (size >= 0) {

	/*
	**  Build an X Client Message with the given message body
	*/

	xmessage.type = ClientMessage;
	xmessage.serial = 0;
	xmessage.send_event = TRUE;
	xmessage.display = XtDisplay(_MessageWidget_of(currency));
	xmessage.window = address;
	xmessage.message_type = _CurrencyMessageType_of(currency);
	xmessage.format = _MessageFormat;

	if (size > _MaxMessageLength)
	    size = _MaxMessageLength;

	_CopyMem(message, xmessage.data.b, size);

	/*
	**  Send it
	*/

	status = XSendEvent(XtDisplay(_MessageWidget_of(currency)),
	    (Window) address, FALSE, NoEventMask, (XEvent *) &xmessage);

	/*
	**  If the send failed, raise and exception.
	*/

	if (!status)
	    _Raise(failure);

	/*
	** Get it out to the Server right away
	*/

	XFlush(XtDisplay(_MessageWidget_of(currency)));
    }

    /*
    **	Restore the default error handler
    */
    
    XSetErrorHandler((_Void *) handler);

    _Exceptions
	_WhenOthers
	    XSetErrorHandler((_Void *) handler);
	    _Reraise;
	    
    _EndExceptionBlock

    return;
    }


_Transaction  LwkOpDXmStateTransact(currency, transaction)
_DXmEnvState currency;
 _Transaction transaction;

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
    _Transaction state;

    /*
    **  Get the current Transaction state
    */

    state = _TransactionState_of(currency);

    switch (transaction) {
	case lwk_c_transact_read :
	case lwk_c_transact_read_write :
	    if (!(state == lwk_c_transact_read
		    || state == lwk_c_transact_read_write)) {

		if (_MessageWidget_of(currency) != (_Widget) 0) {
		    XGrabServer(XtDisplay(_MessageWidget_of(currency)));
		    XFlush(XtDisplay(_MessageWidget_of(currency)));
		}

		_TransactionState_of(currency) = transaction;
	    }

	    break;

	case lwk_c_transact_commit :
	case lwk_c_transact_rollback :
	    if (state == lwk_c_transact_read
		    || state == lwk_c_transact_read_write) {

		if (_MessageWidget_of(currency) != (_Widget) 0) {
		    XUngrabServer(XtDisplay(_MessageWidget_of(currency)));
		    XFlush(XtDisplay(_MessageWidget_of(currency)));
		}

		_TransactionState_of(currency) = lwk_c_transact_commit;
	    }

	    break;

	default :
	    _Raise(inv_argument);
	    break;
    }

    /*
    **  Return the old Transaction state
    */

    return state;
    }


static void  GetValueProperty(currency, property, value_domain, value)
_DXmEnvState currency;
 _String property;

    _Domain value_domain;
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
    Atom atom;
    Atom type;
    Window window;
    _Integer size;
    _ObjectId oid;
    _Domain domain;
    _AnyPtr pointer;
    _Integer integer;
    _Boolean boolean;
    _Persistent persistent;

    /*
    **  Can't do anything without a Message Widget
    */

    if (_MessageWidget_of(currency) == (_Widget) 0)
	_Raise(no_such_property);

    /*
    **  Get the value of the property (a byte vector) from the X-Server.
    */

    atom = NameToAtom(currency, property);
    window = _RootWindow(_MessageWidget_of(currency));

    if (!RetrieveProperty(currency, window, atom, &type, &pointer, &size))
	_Raise(no_such_property);

    /*
    **  If the size is zero, then no property was stored.
    */

    if (size == 0)
	_Raise(no_such_property);

    /*
    **  Convert the byte vector back into a value of the proper domain and
    **	return it to the caller.
    */

    if (_IsDomain(lwk_c_domain_persistent, value_domain))
	domain = lwk_c_domain_persistent;
    else
	domain = value_domain;

    switch (domain) {
	case lwk_c_domain_integer :
	    if (!(type == _PropertyTypeDirect && size == sizeof(_Integer)))
		_Raise(inv_domain);

	    integer = *((_Integer *) pointer);

	    _MoveValue(&integer, value, lwk_c_domain_integer);

	    break;

	case lwk_c_domain_boolean :
	    if (!(type == _PropertyTypeDirect && size == sizeof(_Boolean)))
		_Raise(inv_domain);

	    boolean = *((_Boolean *) pointer);

	    _MoveValue(&boolean, value, lwk_c_domain_boolean);

	    break;

	case lwk_c_domain_string :
	case lwk_c_domain_ddif_string :
	    if (type != _PropertyTypeDirect)
		_Raise(inv_domain);

	    _CopyValue(&pointer, value, domain);

	    break;

	case lwk_c_domain_object_id :
	    if (type != _PropertyTypeIndirect)
		_Raise(inv_domain);

	    oid = (_ObjectId) _Import(_TypeObjectId, pointer);

	    _MoveValue(&oid, value, lwk_c_domain_object_id);

	    break;

	case lwk_c_domain_persistent :
	    if (type == _PropertyTypeDirect)
		persistent = (_Persistent) _Import(_DomainToType(value_domain),
		    pointer);
	    else if (type == _PropertyTypeIndirect) {
		oid = (_ObjectId) _Import(_TypeObjectId, pointer);

		persistent = (_Persistent) _Retrieve(oid);

		_Delete(&oid);
	    }
	    else
		_Raise(inv_domain);

	    _MoveValue(&persistent, value, lwk_c_domain_persistent);

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    **  Free the byte vector.
    */

    XFree(pointer);

    return;
    }


static _List  GetValueListProperty(currency, property)
_DXmEnvState currency;
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
    _Raise(not_yet_impl);
    }


static void  SetValueProperty(currency, property, value_domain, value, flag)
_DXmEnvState currency;
 _String property;

    _Domain value_domain;
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
    Atom atom;
    Atom type;
    Window window;
    _Integer size;
    _Domain domain;
    _AnyPtr volatile pointer;
    _Boolean volatile delete_encoding;

    /*
    **  Can't do anything without a Message Widget
    */

    if (_MessageWidget_of(currency) == (_Widget) 0)
	_Raise(no_such_property);

    /*
    **  Initialize
    */

    delete_encoding = _False;
    pointer = (_AnyPtr) _NullObject;
    size = 0;
    type = _PropertyTypeDirect;

    _StartExceptionBlock

    /*
    **  Encode the value into a byte vector.
    */

    if (_IsDomain(lwk_c_domain_persistent, value_domain))
	domain = lwk_c_domain_persistent;
    else
	domain = value_domain;

    switch (domain) {
	case lwk_c_domain_integer :
	    pointer = value;
	    size = sizeof(_Integer);
	    break;

	case lwk_c_domain_boolean :
	    pointer = value;
	    size = sizeof(_Boolean);
	    break;

	case lwk_c_domain_string :
	    pointer = *((_String *) value);

	    if (pointer != (_AnyPtr) _NullObject)
		size = _LengthString(*((_String *) value)) + 1;

	    break;

	case lwk_c_domain_ddif_string :
	    pointer = *((_DDIFString *) value);

	    if (pointer != (_AnyPtr) _NullObject)
		size = _LengthDDIFString(*((_DDIFString *) value));

	    break;

	case lwk_c_domain_object_id :
	    if (*((_ObjectId *) value) != (_ObjectId) _NullObject) {
		size = (_Integer) _Export(*((_ObjectId *) value), _False,
		    (_VaryingStringPtr) &pointer);

		delete_encoding = _True;
		type = _PropertyTypeIndirect;
	    }

	    break;

	case lwk_c_domain_persistent :
	    if (*((_Persistent *) value) != (_Persistent) _NullObject) {
                /*
                ** Try to store as an encoded ObjectId.  If that fails, store
		** as an encoded Persistent object.
                */

		if (ExportObjectId(*((_Persistent *) value), &pointer, &size))
		    type = _PropertyTypeIndirect;

		delete_encoding = _True;
	    }

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    /*
    **  Store the Property value with the on the Root window of the Display.
    */

    atom = NameToAtom(currency, property);
    window = _RootWindow(_MessageWidget_of(currency));

    if (!StoreProperty(currency, window, atom, type, pointer, size, flag))
	_Raise(no_such_property);

    /*
    **  If we created an encoding, delete it.
    */

    if (delete_encoding)
	_DeleteEncoding(&pointer);

    /*
    **	If any exceptions are raised, clean up, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    if (delete_encoding)
		_DeleteEncoding(&pointer);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetValueListProperty(currency, property, values, flag)
_DXmEnvState currency;
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
    _Raise(not_yet_impl);
    }


static _Termination  FindProperty(property_name, prop_set, domain, prop)
_String property_name;
 _Set prop_set;

    _Domain domain;
 _Property *prop;

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
    _Termination answer;

    /*
    **  If the _PropertyName of the Property matches, return the Property.
    **	Otherwise, return zero so that the iteration will continue.
    */

    if (_IsNamed(*prop, property_name))
	answer = (_Termination) *prop;
    else
	answer = (_Termination) 0;

    return answer;
    }


static void  GenerateNotificationAtoms(currency)
_DXmEnvState currency;

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
    _Integer count;

    /*
    **  Delete the old NotificationAtoms set if there was one.
    */

    _Delete(&_NotificationAtoms_of(currency));

    /*
    **  Create a new set for the NotificationAtoms (same size as Notifications
    **	set).
    */

    _GetValue(_Notifications_of(currency), _P_ElementCount, lwk_c_domain_integer,
	&count);

    _NotificationAtoms_of(currency) = (_Set) _CreateSet(_TypeSet,
	    lwk_c_domain_property, count);

    /*
    **  Add an atom for each property name in the Notification list to the
    **	KnownAtoms and NotificationAtoms list.
    */

    _Iterate(_Notifications_of(currency), lwk_c_domain_string,
	(_Closure) currency, AddNotificationAtom);

    return;
    }


static _Termination  AddNotificationAtom(currency, notifications, domain, property)
_DXmEnvState currency;

    _Set notifications;
 _Domain domain;
 _String *property;

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
    Atom atom;
    _Property volatile prop;

    /*
    **	Create a new property and set its name property, then set its value
    **	to be the atom.
    */

    prop = (_Property) _NullObject;

    _StartExceptionBlock

    prop = (_Property) _Create(_TypeProperty);

    _SetValue(prop, _P_PropertyName, lwk_c_domain_string, property,
	lwk_c_set_property);

    atom = NameToAtom(currency, *property);

    _SetValue(prop, _P_Value, lwk_c_domain_integer, &atom, lwk_c_set_property);

    /*
    **  Add this Property to the Set of NotificationAtoms
    */

    _AddElement(_NotificationAtoms_of(currency), lwk_c_domain_property,
	(_AnyPtr) &prop, _False);

    /*
    **	If any exceptions are raised, delete any new Property, then reraise
    **	the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&prop);
	    _Reraise;
    _EndExceptionBlock

    return (_Termination) 0;
    }


static Atom  NameToAtom(currency, property)
_DXmEnvState currency;
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
    Atom atom;

    if (_MessageWidget_of(currency) != (_Widget) 0) 
	atom = XmInternAtom(XtDisplay(_MessageWidget_of(currency)), property,
	    FALSE);

    return atom;
    }



static _Boolean  RetrieveProperty(currency, window, atom, type, pointer, size)
_DXmEnvState currency;
 Window window;
 Atom atom;

    Atom *type;
 _AnyPtr *pointer;
 _Integer *size;

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
    int status;
    _Boolean answer;
    unsigned long remaining, format;

    /*
    **  Get the value stored for the property from the X-Server
    */

    answer = _False;

    if (_MessageWidget_of(currency) != (_Widget) 0) {
	status = XGetWindowProperty(XtDisplay(_MessageWidget_of(currency)),
	    window, atom, 0, _MaxPropertyLength, FALSE, AnyPropertyType, type,
	    (int *) &format, (unsigned long *) size, &remaining,
	    (unsigned char **) pointer);
	
	if (status == 0)
	    answer = _True;
    }

    return answer;
    }


static _Boolean  StoreProperty(currency, window, atom, type, pointer, size, flag)
_DXmEnvState currency;
 Window window;
 Atom atom;

    Atom type;
 _AnyPtr pointer;
 _Integer size;
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
    _Boolean answer;

    /*
    **  Store the value of the property in the X-Server.
    */

    answer = _False;

    if (_MessageWidget_of(currency) != (_Widget) 0) {
        /*
	** If this is a DeletePropery request, we could use XDeleteProperty.
	** However, it is not idempotent.  If an attempt is made to delete a
	** property twice, a BadAtom X Toolkit error results -- ugh, very
	** messy.  So, instead we clear the property's value.  We detect this
	** in the GetProperty and generate a no_such_property exception.
        */
	
	if (flag == lwk_c_delete_property)
	    size = 0;

	XChangeProperty(XtDisplay(_MessageWidget_of(currency)),
	    window, atom, type, _PropertyFormat, PropModeReplace,
	    pointer, size);

	/*
	** Get it out to the Server right away
	*/

	XFlush(XtDisplay(_MessageWidget_of(currency)));

	answer = _True;
    }

    return answer;
    }


static void  RegisterNotificationHandler(currency)
_DXmEnvState currency;

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
    int ac;
    Widget root;
    _Boolean valid;
    Arg arglist[3];
    XmString xstring;
    Widget lw;

    /*
    **  Can't do anything useful if the Widget value is zero.
    */

    if (_MainWidget_of(currency) == (_Widget) 0)
	return;

    /*
    **	Create, manage and realize a dummy Widget to handle ClientMessages used
    **	to communicate between UI services in each application.
    */


    xstring = _StringToXmString((char *) "");
    ac = 0;
    XtSetArg(arglist[ac], XmNlabelString, xstring); ac++;
    XtSetArg(arglist[ac], XmNlabelType, XmSTRING); ac++;

    lw = (Widget) XmCreateLabel((Widget) _MainWidget_of(currency), "",
	arglist, ac);
	
    _MessageWidget_of(currency) = (_Widget)lw;
	
    XmStringFree(xstring);

    XtRealizeWidget(_MessageWidget_of(currency));

    /*
    **	The Window of the Message Widget is our "address" for sending Messages
    */

    _Address_of(currency) = XtWindow(_MessageWidget_of(currency));

    /*
    **	Set a property on the Window which can be used to verify the address.
    */

    _ValidationProperty_of(currency) = NameToAtom(currency,
	_ValidationProperty);

    valid = _True;

    StoreProperty(currency, _Address_of(currency),
	_ValidationProperty_of(currency), _PropertyTypeDirect, &valid,
	sizeof(_Boolean), lwk_c_set_property);

    /*
    **	Determine the atom for the type of message event which will be sent on
    **	property change.  This lets us avoid a Server round-trip on each
    **	PropertyNotify event.  Note: the atom is Server-dependent, so it must
    **	be kept for each DXmEnvState.
    */

    _CurrencyMessageType_of(currency) = NameToAtom(currency,
	_NotificationMessageType);

    /*
    **	Register the ReceiveMessage routine as the handler for client message
    **	events on the Message Widget.
    */

    XtAddEventHandler(_MessageWidget_of(currency), NoEventMask, TRUE,
	(XtEventHandler) ReceiveMessage, (Opaque) currency);

    /*
    ** We only need to do the following things once per application
    */

    if (_Next_of(DXmEnvStateList) == (_DXmEnvState) _NullObject) {
	/*
	** Create and Realize a dummy Widget to handle PropertyChange events
	** from the Root Window.
	*/

	ac = 0;
	XtSetArg(arglist[ac], VFakeNwindow,
	    _RootWindow(_MainWidget_of(currency)));
	ac++;

	root = (Widget) VFakeCreate(_MainWidget_of(currency), "", arglist, ac);

	XtRealizeWidget(root);

	/*
	** Register the PropertyChanged routine as the handler for property
	** change events on the Root Window.
	*/

	XtAddEventHandler(root, PropertyChangeMask, FALSE,
	    (XtEventHandler) PropertyChanged, (Opaque) currency);
    }

    return;
    }


static void  PropertyChanged(w, currency, event, continue_to_dispatch)
Widget w;
 _DXmEnvState currency;

    XPropertyEvent *event;
 Boolean *continue_to_dispatch;

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
    _DXmEnvState next;
    _Property property;
    XClientMessageEvent message;

    /*
    **  It must be a PropertyNotify event
    */

    if (event->type != PropertyNotify)
	return;

    /*
    **  For the Root Window
    */
    
/********** Temporary fix ********************

    if(event->window !=
	    _RootWindow(_MessageWidget_of(currency)))
	return;
	
**********************************/

    /*
    **  Loop over all of the known Currency objects in this application to see
    **	if this property has a change notification request.
    */

    _BeginCriticalSection

    next = DXmEnvStateList;

    while (next != (_DXmEnvState) _NullObject) {
	/*
	**  Must be a Ui on the same display
	*/

	if (XtDisplay(_MessageWidget_of(next)) ==
		XtDisplay(_MessageWidget_of(currency))) {
	    /*
	    **  Check the list of Notifications
	    */

	    if (_NotificationAtoms_of(next) != (_Set) _NullObject) {
		property = (_Property) _Iterate(_NotificationAtoms_of(next),
		    lwk_c_domain_property, (_Closure) event->atom, SearchAtom);

		if (property != (_Property) _NullObject)
		    NotifyOfCurrencyChange(next, property);
	    }
	}

	next = _Next_of(next);
    }

    _EndCriticalSection

    return;
    }


static void  ReceiveMessage(w, currency, event, continue_to_dispatch)
Widget w;
 _DXmEnvState currency;

    XClientMessageEvent *event;
 Boolean *continue_to_dispatch;

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
    **  If the Currency object is invalid, return now.
    */

    if (!_IsValidObject(currency))
	return;

    /*
    **  If this event is not a Client Message of our manufacture, return now.
    */

    if (event->type != ClientMessage)
	return;

    if (event->message_type != _CurrencyMessageType_of(currency))
	return;

    /*
    **  If there is no callback specified for message notification,
    **	return now.
    */

    if (_MessageCb_of(currency) == (_Callback) _NullObject)
	return;

    /*
    **  Invoke the Message callback.
    */

    _StartExceptionBlock

    (_MessageCb_of(currency))(_UserData_of(currency), currency,
	&event->data.l[0]);

    /*
    **	If an exception is raised, ignore it.
    */

    _Exceptions
	_WhenOthers
    _EndExceptionBlock

    return;
    }


static void  NotifyOfCurrencyChange(currency, property)
_DXmEnvState currency;
 _Property property;

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
    _String volatile name;

    /*
    **  If there is no callback specified for currency change notification,
    **	return now.
    */

    if (_NotificationCb_of(currency) == (_Callback) _NullObject)
	return;

    name = (_String) _NullObject;

    _StartExceptionBlock

    _GetValue(property, _P_PropertyName, lwk_c_domain_string, (_AnyPtr) &name);

    (_NotificationCb_of(currency))(_UserData_of(currency), currency, name);

    _DeleteString(&name);

    /*
    **	If an exception is raised, clean up but otherwise ignore it.
    */

    _Exceptions
	_WhenOthers
	    _DeleteString(&name);
    _EndExceptionBlock

    return;
    }


static _Boolean  ExportObjectId(persistent, encoding, size)
_Persistent volatile persistent;

    _VaryingStringPtr volatile encoding;
 _Integer *size;

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
    _Boolean is_oid;
    _ObjectId volatile oid;

    /*
    ** Initialize
    */

    is_oid = _True;
    oid = (_ObjectId) _NullObject;

    _StartExceptionBlock

    oid = (_ObjectId) _GetObjectId(persistent);

    *size = (_Integer) _Export(oid, _False, encoding);

    _Delete(&oid);

    _Exceptions
	_WhenOthers
	    _Delete(&oid);
	    is_oid = _False;
	    *size = (_Integer) _Export(persistent, _False, encoding);
    _EndExceptionBlock

    return is_oid;
    }


static _Termination  SearchAtom(atom, prop_set, domain, prop)
Atom atom;
 _Set prop_set;
 _Domain domain;

    _Property *prop;

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
    Atom value;
    _Termination answer;

    /*
    **  If the Atom matches the value of the Property, return the Property.
    **	Otherwise, return zero so that the iteration will continue.
    */

    _GetValue(*prop, _P_Value, lwk_c_domain_integer, &value);

    if (atom == value)
	answer = (_Termination) *prop;
    else
	answer = (_Termination) 0;

    return answer;
    }


static int  LocalErrorHandler(dpy, event)
Display *dpy;
 XErrorEvent *event;

/*
**++
**  Functional Description:
**	Ignore X Error messages
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
    
