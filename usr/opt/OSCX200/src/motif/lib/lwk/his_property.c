/*
** COPYRIGHT (c) 1988 BY
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
**	HyperInformation Services
**
**  Version: X0.1
**
**  Abstract:
**	Type-dependent Property object methods 
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

#ifdef MSDOS
#include "include.h"
#include "property.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_property.h"
#include "his_ddis_encoding.h"
#endif /* MSDOS */

/*
**  Macro Definitions
*/

/*
**  Type Definitions
*/

/*
**  Forward Routine Declarations
*/

_DeclareFunction(static _Termination _EntryPt EncodeValue,
    (_Closure closure, _List values, _Domain domain, _AnyValuePtr value));
_DeclareFunction(static void EncodeList,
    (_DDIShandle handle, _ListPtr value));
_DeclareFunction(static void DecodeValue,
    (_DDIShandle handle, _Domain domain, _AnyValuePtr value));
_DeclareFunction(static void DecodeList,
    (_DDIShandle handle, _Domain domain, _ListPtr value));

/*
**  Global Data Definitions
*/

_Global _PropertyTypeInstance;      /* Instance record for Type */
_Global _PropertyType;              /* Type */

/*
**  External Data Declarations
*/

_External long int lwk_lwkproperty;

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypePropertyInstance;
static _PropertyPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    PropertyPropertyNameTable;

/*
**  List of Parse Tables for DDIS Toolkit.
*/

static unsigned long int ParseTableList[2] =
    {1, (unsigned long int) &lwk_lwkproperty};


void  LwkOpProp(prop)
_Property prop;

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


_Property  LwkOpPropImport(our_type, encoding)
_Type our_type;
 _VaryingString encoding;

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
    _DDISstatus status;
    _Property volatile prop;
    _DDIShandle handle;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;

    prop = (_Property) _NullObject;
    handle = 0;

    _StartExceptionBlock

    /*
    **  Create an object of our type
    */

    prop = (_Property) _Create(our_type);

    /*
    **  Set up the stream for the DDIS toolkit.
    */

    handle = _OpenDDISStream(encoding, ParseTableList);

    /*
    **  Begin the Property decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PROPERTY)
	_Raise(inv_encoding);

    /*
    **  Invoke the _Decode operation on the object
    */

    _Decode((_Object) prop, handle, _False, (_SetPtr) _NullObject);

    /*
    **  Finish the Property decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PROPERTY)
	_Raise(inv_encoding);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If any exceptions are raised, delete the Property, close the
    **	DDIS stream if it is still open, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    if (prop != (_Property) _NullObject)
		_Delete(&prop);
	    if (handle != 0)
		_CloseDDISStream(handle);
	    _Reraise;
    _EndExceptionBlock

    return prop;
    }


void  LwkOpPropImportKeyProperties(our_type, encoding, properties)
_Type our_type;
 _VaryingString encoding;

    _Set *properties;

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
    _DDISstatus status;
    _Property volatile prop;
    _DDIShandle handle;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;

    prop = (_Property) _NullObject;
    handle = 0;

    _StartExceptionBlock

    /*
    **  Create an object of our type
    */

    prop = (_Property) _Create(our_type);

    /*
    **  Set up the stream for the DDIS toolkit.
    */

    handle = _OpenDDISStream(encoding, ParseTableList);

    /*
    **  Begin the Property decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PROPERTY)
	_Raise(inv_encoding);

    /*
    **  Invoke the _Decode operation on the object
    */

    _Decode((_Object) prop, handle, _True, properties);

    /*
    **  Add this Property to the Key Property Set
    */

    if (*properties == (_Set) _NullObject)
	*properties = (_Set) _CreateSet(_TypeSet, lwk_c_domain_property,
	    (_Integer) 0);

    _AddElement(*properties, lwk_c_domain_property, (_AnyPtr) &prop, _False);

    /*
    **  Finish the Property decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PROPERTY)
	_Raise(inv_encoding);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If the NoSUchProperty exception is raise, just clean up and continue.
    **	If any other exceptions are raised, clean up then reraise the
    **	exception.
    */

    _Exceptions
	_When(no_such_property)
	    _Delete(&prop);

	    if (handle != 0)
		_CloseDDISStream(handle);

	_WhenOthers
	    _Delete(&prop);

	    if (handle != 0)
		_CloseDDISStream(handle);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkOpPropInitialize(prop, proto, aggregate)
_Property prop;
 _Property proto;
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
    _Domain domain;

    /*
    **  Initialize the properties defined by our type
    */

    _Domain_of(prop) = lwk_c_domain_unknown;

    _ClearValue(&_PropertyName_of(prop), lwk_c_domain_string);
    _ClearValue(&_Value_of(prop), lwk_c_domain_integer);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(prop, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype object was provided, copy properties from it.
    */

    if (proto != (_Property) _NullObject) {
	_CopyValue(&_PropertyName_of(proto), &_PropertyName_of(prop),
	    lwk_c_domain_string);

	_Domain_of(prop) = _Domain_of(proto);

	if (_Domain_of(proto) != lwk_c_domain_unknown)
	    _CopyValue(&_Value_of(proto), &_Value_of(prop), _Domain_of(proto));
    }

    return;
    }


void  LwkOpPropFree(prop)
_Property prop;

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
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_PropertyName_of(prop), lwk_c_domain_string);

    if (_Domain_of(prop) != lwk_c_domain_unknown)
	_DeleteValue(&_Value_of(prop), _Domain_of(prop));

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(prop, MyType);

    return;
    }


_Set  LwkOpPropEnumProps(prop)
_Property prop;

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

    set = (_Set) _EnumerateProperties_S(prop, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpPropIsMultiValued(prop, property)
_Property prop;
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
	answer = _IsMultiValued_S(prop, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
	    case _DomainIndex :
		answer = _False;
		break;

	    case _ValueIndex :
		answer = _IsDomain(lwk_c_domain_list, _Domain_of(prop));
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpPropGetValueDomain(prop, property)
_Property prop;
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
	domain = (_Domain) _GetValueDomain_S(prop, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
		domain = lwk_c_domain_string;
		break;

	    case _DomainIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _ValueIndex :
		domain = _Domain_of(prop);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpPropGetValue(prop, property, domain, value)
_Property prop;
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
	_GetValue_S(prop, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_PropertyName_of(prop), value,
			lwk_c_domain_string);
		else
		    _Raise(inv_domain);

		break;

	    case _DomainIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    *((_IntegerPtr) value) = (_Integer) _Domain_of(prop);
		else
		    _Raise(inv_domain);

		break;

	    case _ValueIndex :
		if (_IsDomain(domain, _Domain_of(prop)))
		    _CopyValue(&_Value_of(prop), value, _Domain_of(prop));
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


_List  LwkOpPropGetValueList(prop, property)
_Property prop;
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
    _Integer integer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	return (_List) _GetValueList_S(prop, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
		_ListValue(&_PropertyName_of(prop), &value_list,
		    lwk_c_domain_string);
		break;

	    case _DomainIndex :
		integer = (_Integer) _Domain_of(prop);

		_ListValue(&integer, (_ObjectPtr) &value_list,
		    lwk_c_domain_integer);
		break;

	    case _ValueIndex :
		if (_IsDomain(lwk_c_domain_list, _Domain_of(prop)))
		    _CopyValue(&_Value_of(prop), &value_list,
			_Domain_of(prop));
		else
		    _ListValue(&_Value_of(prop), &value_list,
			_Domain_of(prop));

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpPropSetValue(prop, property, domain, value, flag)
_Property prop;
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
    _Boolean is_multi_valued;
    _Boolean value_is_list;
    _Boolean property_is_set;
    _Integer property_domain;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(prop, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
		_SetSingleValuedProperty(&_PropertyName_of(prop),
		    lwk_c_domain_string, domain, value, flag, _False);
		break;

	    case _DomainIndex :
		_Raise(property_is_readonly);
		break;

	    case _ValueIndex :
		if (_Domain_of(prop) == lwk_c_domain_unknown) {
		    /*
		    **	The Property domain is unknown.  Set it to the
		    **	appropriate domain.
		    */

		    if (flag == lwk_c_add_property) {
			/*
			**  If the flag indicate that a value is to be added
			**  to the value list, the set the property to be
			**  multi-valued.
			*/

			_Domain_of(prop) = lwk_c_domain_list;
			is_multi_valued = _True;

			property_domain = (_Integer) domain;
		    }
		    else if (_IsDomain(lwk_c_domain_list, domain)) {
			/*
			** If the given domain is a List, the property domain
			** is multi_valued and the property domain is the
			** domain of the List.
			*/

			_Domain_of(prop) = domain;
			is_multi_valued = _True;

			_GetValue(*((_ListPtr) value), _P_Domain,
			    lwk_c_domain_integer, &property_domain);
		    }
		    else {
			/*
			**  Normal, single valued property of the given domain.
			*/

			_Domain_of(prop) = domain;
			is_multi_valued = _False;

			property_domain = (_Integer) domain;
		    }
		}
		else {
		    /*
		    **  The property domain is known -- get the value domain.
		    */

		    if (_IsDomain(lwk_c_domain_list, _Domain_of(prop))) {
			is_multi_valued = _True;

			_GetValue((_List) _Value_of(prop).object, _P_Domain,
			    lwk_c_domain_integer, &property_domain);
		    }
		    else {
			is_multi_valued = _False;

			property_domain = (_Integer) _Domain_of(prop);
		    }
		}

		/*
		**  Set the property appropriately.
		*/

		if (_IsDomain(lwk_c_domain_list, domain))
		    value_is_list = _True;
		else
		    value_is_list = _False;

		if (is_multi_valued) {
		    if (_IsDomain(lwk_c_domain_set, _Domain_of(prop)))
			property_is_set = _True;
		    else
			property_is_set = _False;

		    _SetMultiValuedProperty((_ListPtr) &_Value_of(prop),
			(_Domain) property_domain, domain, value, flag,
			value_is_list, property_is_set);
		}
		else {
		    _SetSingleValuedProperty(&_Value_of(prop),
			(_Domain) property_domain, domain, value, flag,
			value_is_list);
		}

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpPropSetValueList(prop, property, values, flag)
_Property prop;
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
    _Boolean is_multi_valued;
    _Boolean property_is_set;
    _Integer property_domain;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(prop, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _PropertyNameIndex :
		_SetSingleValuedProperty(&_PropertyName_of(prop),
		    lwk_c_domain_string, lwk_c_domain_list, values, flag, _True);
		break;

	    case _DomainIndex :
		_Raise(property_is_readonly);
		break;

	    case _ValueIndex :
		property_domain = (_Integer) lwk_c_domain_unknown;

		if (_Domain_of(prop) == lwk_c_domain_unknown) {
		    /*
		    **	The Property domain is unknown.  Set it to the
		    **	appropriate domain.
		    */

		    is_multi_valued = _True;

		    if (values != (_List) _NullObject) {
			if (_IsType(values, _TypeSet))
			    _Domain_of(prop) = lwk_c_domain_set;
			else
			    _Domain_of(prop) = lwk_c_domain_list;

			_GetValue(values, _P_Domain, lwk_c_domain_integer,
			    &property_domain);
		    }
		}
		else {
		    /*
		    **  The property domain is known -- get the value domain.
		    */

		    if (_IsDomain(lwk_c_domain_list, _Domain_of(prop))) {
			is_multi_valued = _True;

			_GetValue((_List) _Value_of(prop).object, _P_Domain,
			    lwk_c_domain_integer, &property_domain);
		    }
		    else {
			is_multi_valued = _False;

			property_domain = (_Integer) _Domain_of(prop);
		    }
		}

		/*
		** Set the values appropriately.
		*/

		if (is_multi_valued) {
		    if (_IsDomain(lwk_c_domain_set, _Domain_of(prop)))
			property_is_set = _True;
		    else
			property_is_set = _False;

		    _SetMultiValuedProperty((_ListPtr) &_Value_of(prop),
			(_Domain) property_domain, _Domain_of(prop), values,
			flag, _True, property_is_set);
		}
		else {
		    _SetSingleValuedProperty(&_Value_of(prop),
			(_Domain) property_domain, lwk_c_domain_list, values,
			flag, _True);
		}

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_Integer  LwkOpPropExport(prop, aggregate, encoding)
_Property prop;
 _Boolean aggregate;

    _VaryingString *encoding;

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
    _DDISstatus status;
    _DDIShandle volatile handle;
    _DDIStype type;
    _DDISlength value_length;

    *encoding = (_VaryingString) _NullObject;
    handle = 0;

    _StartExceptionBlock

    /*
    **  Initialize the DDIS toolkit stream.
    */

    handle = _CreateDDISStream(encoding, ParseTableList);

    /*
    **  Begin the encoding of Property.
    */

    type = LWK_K_T_PROPERTY;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Invoke the _Encode operation on the object
    */

    _Encode((_Object) prop, aggregate, handle);

    /*
    **  Finish the Property encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If any exceptions are raised, delete the partial encoding, then
    **	reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _DeleteEncoding(encoding);
	    if (handle != 0)
		_CloseDDISStream(handle);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the pointer and size to the caller.
    */

    return (_Integer) _VSize_of(*encoding);
    }


void  LwkOpPropEncode(prop, aggregate, handle)
_Property prop;
 _Boolean aggregate;
 _DDIShandle handle;

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
    _Integer integer;
    _DDISstatus status;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Encode the Property name.
    */

    if (_PropertyName_of(prop) == (_String) _NullObject)
	_Raise(internal_encoding_error);
    else {
	type = LWK_K_T_PROPERTY_NAME;
	value_length = _LengthString(_PropertyName_of(prop));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _PropertyName_of(prop), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **  Encode the Property value domain.
    */

    type = LWK_K_T_PROPERTY_DOMAIN;
    value_length = sizeof(_Integer);

    integer = (_Integer) _Domain_of(prop);

    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &integer,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the Property value or values.
    */

    switch (_Domain_of(prop)) {
	case lwk_c_domain_integer :
	case lwk_c_domain_boolean :
	case lwk_c_domain_float :
	case lwk_c_domain_string :
	case lwk_c_domain_ddif_string :
	case lwk_c_domain_date :

	    EncodeValue((_Closure) handle, (_List) _NullObject,
		_Domain_of(prop), &_Value_of(prop));

	    break;

	case lwk_c_domain_list :
	case lwk_c_domain_set :

	    EncodeList(handle, (_ListPtr) &_Value_of(prop));

	    break;

	default :
	    _Raise(internal_encoding_error);
	    break;
    }

    return;
    }


void  LwkOpPropDecode(prop, handle, keys_only, properties)
_Property prop;
 _DDIShandle handle;
 _Boolean keys_only;

    _Set *properties;

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
    _DDISstatus status;
    _Integer integer;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;

    /*
    **  Decode the Property.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PROPERTY_VALUE :
	    case LWK_K_E_PROPERTY_VALUES :
		/*
		**  End of Property value.
		*/
		break;

	    case LWK_K_P_PROPERTY_NAME :
		_PropertyName_of(prop) = _AllocateMem(value_length + 1);

		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) _PropertyName_of(prop), &get_length,
		    &additional_info);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (value_length != get_length)
		    _Raise(internal_decoding_error);

		_PropertyName_of(prop)[get_length] = _EndOfString;

		/*
		**  If extracting Key Properties, check to see if this one has
		**  the required prefix.  If not, raise an exception.
		*/

		if (keys_only && !_HasPrefixString(_PropertyName_of(prop),
			_ApplicationDefinedKeyPrefix))
		    _Raise(no_such_property);

		break;

	    case LWK_K_P_PROPERTY_DOMAIN :
		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) &integer, &get_length, &additional_info);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		_Domain_of(prop) = (_Domain) additional_info;

		break;

	    case LWK_K_P_PROPERTY_VALUE :
		/*
		**  Decode the value.
		*/

		DecodeValue(handle, _Domain_of(prop), &_Value_of(prop));

		break;

	    case LWK_K_P_PROPERTY_VALUES :
		/*
		**  Decode the values.
		*/

		/*
		**  Key Properties must be single-valued.
		*/

		if (keys_only)
		    _Raise(no_such_property);

		DecodeList(handle, _Domain_of(prop),
		    (_ListPtr) &_Value_of(prop));

		break;

	    default :
		_Raise(inv_encoding);
		break;
	}

    } while (entry != LWK_K_E_PROPERTY_VALUE
	     && entry != LWK_K_E_PROPERTY_VALUES);

    return;
    }


static _Termination _EntryPt  EncodeValue(closure, values, domain, value)
_Closure closure;
 _List values;

    _Domain domain;
 _AnyValuePtr value;

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
    _DDISstatus status;
    _DDIShandle handle;
    _DDIStype type;
    _DDISlength value_length;
    _DDISconstant additional_info;

    handle = (_DDIShandle) closure;

    /*
    **  Begin encoding Value.
    */

    type = LWK_K_T_PROPERTY_VALUE;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the value based on its domain.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    type = LWK_K_T_VALUE_INTEGER;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) value,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	case lwk_c_domain_boolean :
	    type = LWK_K_T_VALUE_BOOLEAN;
	    value_length = sizeof(_Boolean);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) value,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	case lwk_c_domain_float :
	    type = LWK_K_T_VALUE_FLOAT;
	    value_length = sizeof(_Float);
	    additional_info = DDIS_K_F_FLOAT;
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) value,
		&additional_info);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	case lwk_c_domain_date :
	    type = LWK_K_T_VALUE_DATE;
	    value_length = _LengthDate(value->date);
	    status = ddis_put(&handle, &type, &value_length,
		(_DDISdata) value->date, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	case lwk_c_domain_string :
	    type = LWK_K_T_VALUE_STRING;
	    value_length = _LengthString(value->string);
	    status = ddis_put(&handle, &type, &value_length,
		(_DDISdata) value->string, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	case lwk_c_domain_ddif_string :
	    type = LWK_K_T_VALUE_CSTRING;
	    value_length = _LengthDDIFString(value->ddifstring);
	    status = ddis_put(&handle, &type, &value_length,
		(_DDISdata) value->ddifstring, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    break;

	default :
	    _Raise(internal_encoding_error);
	    break;
    }

    /*
    **  Finish encoding the value.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Continue the iteration.
    */

    return (_Termination) 0;
    }


static void  EncodeList(handle, list)
_DDIShandle handle;
 _ListPtr list;

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
    _DDISstatus status;
    _Integer count;
    _Integer list_domain;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Begin encoding the value list.
    */

    type = LWK_K_T_PROPERTY_VALUES;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the List count.
    */

    _GetValue(*list, _P_ElementCount, lwk_c_domain_integer, &count);

    type = LWK_K_T_VALUES_COUNT;
    value_length = sizeof(_Integer);

    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the List domain.
    */

    _GetValue(*list, _P_Domain, lwk_c_domain_integer, &list_domain);

    type = LWK_K_T_VALUES_DOMAIN;
    value_length = sizeof(_Integer);

    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &list_domain,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Begin encoding the List elements.
    */

    type = LWK_K_T_VALUES_ELEMENTS;

    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Iterate over the List encoding each value.
    */

    _Iterate(*list, (_Domain) list_domain, (_Closure) handle, EncodeValue);

    /*
    **  Finish encoding the List elements.
    */

    type = DDIS_K_T_EOC;

    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Finish encoding the value list.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    return;
    }


static void  DecodeValue(handle, domain, value)
_DDIShandle handle;
 _Domain domain;

    _AnyValuePtr value;

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
    _DDISstatus status;
    _Integer integer;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;

    /*
    **  Begin decoding the value.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    /*
    **  Decode the value based on its domain.
    */

    switch (domain) {
	case lwk_c_domain_integer :
	    if (type != LWK_K_T_VALUE_INTEGER)
		_Raise(inv_encoding);

	    if (value_length > sizeof(_Integer))
		_Raise(internal_decoding_error);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length,
		(_DDISconstantPtr) &value->integer);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    break;

	case lwk_c_domain_boolean :
	    if (type != LWK_K_T_VALUE_BOOLEAN)
		_Raise(inv_encoding);

	    if (value_length > sizeof(_Integer))
		_Raise(internal_decoding_error);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length,
		(_DDISconstantPtr) &value->boolean);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    break;

	case lwk_c_domain_float :
	    if (type != LWK_K_T_VALUE_FLOAT)
		_Raise(inv_encoding);

	    value_length = sizeof(_Float);
	    additional_info = DDIS_K_F_FLOAT;

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) value, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    break;

	case lwk_c_domain_date :
	    if (type != LWK_K_T_VALUE_DATE)
		_Raise(inv_encoding);

	    value->date = _AllocateMem(value_length + 1);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) value->date, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    value->date[get_length] = _EndOfString;

	    break;

	case lwk_c_domain_string :
	    if (type != LWK_K_T_VALUE_STRING)
		_Raise(inv_encoding);

	    value->string = _AllocateMem(value_length + 1);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) value->string, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    value->string[get_length] = _EndOfString;

	    break;

	case lwk_c_domain_ddif_string :
	    if (type != LWK_K_T_VALUE_CSTRING)
		_Raise(inv_encoding);

	    value->ddifstring = _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) value->ddifstring, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    break;

	default :
	    _Raise(internal_decoding_error);
	    break;

    }

    return;
    }


static void  DecodeList(handle, domain, list)
_DDIShandle handle;
 _Domain domain;
 _ListPtr list;

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
    _DDISstatus status;
    _Integer integer;
    _Integer count;
    _Integer list_domain;
    _AnyValue value;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Decode the List count.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_VALUES_COUNT)
	_Raise(inv_encoding);

    if (value_length > sizeof(_Integer))
	_Raise(inv_encoding);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (value_length != get_length)
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Decode the List domain.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_VALUES_DOMAIN)
	_Raise(inv_encoding);

    if (value_length > sizeof(_Integer))
	_Raise(inv_encoding);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &list_domain);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (value_length != get_length)
	_Raise(internal_decoding_error);

    /*
    **  Create the List/Set of the proper size and domain.
    */

    switch (domain) {
	case lwk_c_domain_list :
	    *list = (_List) _CreateList(_TypeList, (_Domain) list_domain,
		count);
	    break;

	case lwk_c_domain_set :
	    *list = (_Set) _CreateSet(_TypeSet, (_Domain) list_domain, count);
	    break;

	default :
	    _Raise(internal_decoding_error);
	    break;
    }

    /*
    **  Begin decoding the value List.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_VALUES_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Decoding count values.
    */

    for (i = 0; i < count; i++) {
	/*
	**  Begin decoding the value(s).
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry == LWK_K_P_VALUES_VALUE)
	    DecodeValue(handle, (_Domain) list_domain, &value);
	else if (entry == LWK_K_P_VALUES_VALUES)
	    DecodeList(handle, (_Domain) list_domain, (_ListPtr) &value);
	else
	    _Raise(internal_decoding_error);

	/*
	**  Add it to the value list.
	*/

	_AddElement(*list, (_Domain) list_domain, &value, _False);

	/*
	**  Finish decoding the value(s).
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_E_PROPERTY_VALUE
	    && entry != LWK_K_E_PROPERTY_VALUES)
		_Raise(internal_decoding_error);
    }

    /*
    **  Finish decoding the Value list.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_VALUES_ELEMENTS)
	_Raise(inv_encoding);

    return;
    }


_Boolean  LwkOpPropIsNamed(prop, property_name)
_Property prop;
 _String property_name;

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
    **  If the given property name is equal to the _PropertyName of the
    **	Property, return _True, else return _False.
    */

    if (_CompareString(_PropertyName_of(prop), property_name) == 0)
	answer = _True;
    else
	answer = _False;

    return answer;
    }
