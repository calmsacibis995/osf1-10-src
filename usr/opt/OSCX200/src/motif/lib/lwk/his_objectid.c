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
**	ObjectId object methods
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
#include "objid.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_objectid.h"
#include "his_database.h"
#include "his_ddis_encoding.h"
#endif

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
**  External Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _ObjectIdTypeInstance;      /* Instance record for Type */
_Global _ObjectIdType;              /* Type */

/*
**  External Data Declarations
*/

_External long int lwk_lwkobjectid;

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeObjectIdInstance;
static _ObjectIdPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    ObjectIdPropertyNameTable;

/*
**  List of Parse Tables for DDIS Toolkit.
*/

static unsigned long int ParseTableList[2] =
    {1, (unsigned long int) &lwk_lwkobjectid};


void  LwkOpOid()
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


_ObjectId  LwkOpOidImport(our_type, encoding)
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
    _ObjectId volatile oid;
    _DDIShandle handle;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;

    oid = (_ObjectId) _NullObject;
    handle = 0;

    _StartExceptionBlock

    /*
    **  Create an object of our type
    */

    oid = (_ObjectId) _Create(our_type);

    /*
    **  Set up the stream for the DDIS toolkit.
    */

    handle = _OpenDDISStream(encoding, ParseTableList);

    /*
    **  Begin the Object Identifier decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_OBJECTID)
	_Raise(inv_encoding);

    /*
    **  Invoke the _Decode operation on the object
    */

    _Decode((_Object) oid, handle, _False, (_Set *) _NullObject);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If any exceptions are raised, delete the ObjectId, close the
    **	DDIS stream if it is still open, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&oid);

	    if (handle != 0)
		_CloseDDISStream(handle);

	    _Reraise;
    _EndExceptionBlock

    return oid;
    }


void  LwkOpOidInitialize(oid, proto, aggregate)
_ObjectId oid;
 _ObjectId proto;
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

    _ObjectDomain_of(oid) = lwk_c_domain_unknown;

    _ClearValue(&_LinkbaseIdentifier_of(oid), lwk_c_domain_string);
    _ClearValue(&_ObjectIdentifier_of(oid), lwk_c_domain_integer);
    _ClearValue(&_ContainerIdentifier_of(oid), lwk_c_domain_integer);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(oid, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_ObjectId) _NullObject) {
	_ObjectDomain_of(oid) = _ObjectDomain_of(proto);

	_CopyValue(&_LinkbaseIdentifier_of(proto),
	    &_LinkbaseIdentifier_of(oid), lwk_c_domain_string);
	_CopyValue(&_ObjectIdentifier_of(proto), &_ObjectIdentifier_of(oid),
	    lwk_c_domain_integer);
	_CopyValue(&_ContainerIdentifier_of(proto),
	    &_ContainerIdentifier_of(oid), lwk_c_domain_integer);
    }

    return;
    }


void  LwkOpOidFree(oid)
_ObjectId oid;

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

    _DeleteValue(&_LinkbaseIdentifier_of(oid), lwk_c_domain_string);
    _DeleteValue(&_ObjectIdentifier_of(oid), lwk_c_domain_integer);
    _DeleteValue(&_ContainerIdentifier_of(oid), lwk_c_domain_integer);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(oid, MyType);

    return;
    }


_Set  LwkOpOidEnumProps(oid)
_ObjectId oid;

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

    set = (_Set) _EnumerateProperties_S(oid, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpOidIsMultiValued(oid, property)
_ObjectId oid;
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
	answer = _IsMultiValued_S(oid, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
	    case _LinkbaseIdentifierIndex :
	    case _ObjectIdentifierIndex :
	    case _ContainerIdentifierIndex :
		answer = _False;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpOidGetValueDomain(oid, property)
_ObjectId oid;
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
	domain = (_Domain) _GetValueDomain_S(oid, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
	    case _ObjectIdentifierIndex :
	    case _ContainerIdentifierIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _LinkbaseIdentifierIndex :
		domain = lwk_c_domain_string;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpOidGetValue(oid, property, domain, value)
_ObjectId oid;
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
	_GetValue_S(oid, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    *((_Integer *) value) = (_Integer) _ObjectDomain_of(oid);
		else
		    _Raise(inv_domain);

		break;

	    case _ObjectIdentifierIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_ObjectIdentifier_of(oid), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _ContainerIdentifierIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_ContainerIdentifier_of(oid), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _LinkbaseIdentifierIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_LinkbaseIdentifier_of(oid), value,
			lwk_c_domain_string);
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


_List  LwkOpOidGetValueList(oid, property)
_ObjectId oid;
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
	return (_List) _GetValueList_S(oid, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
		integer = (_Integer) _ObjectDomain_of(oid);

		_ListValue(&integer, (_Object *) &value_list,
		    lwk_c_domain_integer);
		break;

	    case _ObjectIdentifierIndex :
		_ListValue(&_ObjectIdentifier_of(oid), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _ContainerIdentifierIndex :
		_ListValue(&_ContainerIdentifier_of(oid), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _LinkbaseIdentifierIndex :
		_ListValue(&_LinkbaseIdentifier_of(oid), &value_list,
		    lwk_c_domain_string);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpOidSetValue(oid, property, domain, value, flag)
_ObjectId oid;
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
    _Integer integer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(oid, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
		_SetSingleValuedProperty(&integer, lwk_c_domain_integer,
		    domain, value, flag, _False);

		_ObjectDomain_of(oid) = (_Domain) integer;

		break;

	    case _ObjectIdentifierIndex :
		_SetSingleValuedProperty(&_ObjectIdentifier_of(oid),
		    lwk_c_domain_integer, domain, value, flag, _False);
		break;

	    case _ContainerIdentifierIndex :
		_SetSingleValuedProperty(&_ContainerIdentifier_of(oid),
		    lwk_c_domain_integer, domain, value, flag, _False);
		break;

	    case _LinkbaseIdentifierIndex :
		_SetSingleValuedProperty(&_LinkbaseIdentifier_of(oid),
		    lwk_c_domain_string, domain, value, flag, _False);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpOidSetValueList(oid, property, values, flag)
_ObjectId oid;
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
    _Integer integer;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(oid, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _ObjectDomainIndex :
		_SetSingleValuedProperty(&integer, lwk_c_domain_integer,
		    lwk_c_domain_list, &values, flag, _True);

		_ObjectDomain_of(oid) = (_Domain) integer;

		break;

	    case _ObjectIdentifierIndex :
		_SetSingleValuedProperty(&_ObjectIdentifier_of(oid),
		    lwk_c_domain_integer, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _ContainerIdentifierIndex :
		_SetSingleValuedProperty(&_ContainerIdentifier_of(oid),
		    lwk_c_domain_integer, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _LinkbaseIdentifierIndex :
		_SetSingleValuedProperty(&_LinkbaseIdentifier_of(oid),
		    lwk_c_domain_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_Integer  LwkOpOidExport(oid, aggregate, encoding)
_ObjectId oid;
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

    *encoding = (_VaryingString) _NullObject;
    handle = 0;

    _StartExceptionBlock

    /*
    **  Initialize the DDIS toolkit stream.
    */

    handle = _CreateDDISStream(encoding, ParseTableList);

    /*
    **  Begin the encoding of Object Identifier.
    */

    type = LWK_K_T_OBJECTID;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Invoke the _Encode operation on the object
    */

    _Encode((_Object) oid, aggregate, handle);

    /*
    **  Finish the Object Identifier encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, 	(_DDISconstantPtr) 0);

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


void  LwkOpOidEncode(oid, aggregate, handle)
_ObjectId oid;
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
    _DDISstatus status;
    _DDIStype type;
    _DDISlength value_length;
    _AnyPtr value;

    /*
    **  Encoding the properties of Object Identifier.
    */

    if (_LinkbaseIdentifier_of(oid) != (_CString) _NullObject) {
	type = LWK_K_T_LINKBASE_ID;
	value_length = _LengthString(_LinkbaseIdentifier_of(oid));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _LinkbaseIdentifier_of(oid), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    type = LWK_K_T_OBJECT_ID;
    value_length = sizeof(_Integer);
    value = (_AnyPtr) _ObjectIdentifier_of(oid);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &value,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    type = LWK_K_T_OBJECT_DOMAIN;
    value_length = sizeof(_Integer);
    value = (_AnyPtr) _ObjectDomain_of(oid);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &value,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    type = LWK_K_T_CONTAINER_ID;
    value_length = sizeof(_Integer);
    value = (_AnyPtr) _ContainerIdentifier_of(oid);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &value,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    return;
    }


void  LwkOpOidDecode(oid, handle, keys_only, properties)
_ObjectId oid;
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
    **	Decode the Object Identifier Properties.  If extracting Key Properties,
    **	all properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_OBJECTID :
		/*
		**  End of Object Indentifier Properties.
		*/
		break;

	    case LWK_K_P_LINKBASE_ID :
		if (!keys_only) {
		    _LinkbaseIdentifier_of(oid) =
			_AllocateMem(value_length + 1);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) _LinkbaseIdentifier_of(oid), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);

		    _LinkbaseIdentifier_of(oid)[get_length] = _EndOfString;
		}

		break;

	    case LWK_K_P_OBJECT_ID :
		if (!keys_only) {
		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) &integer, &get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    _ObjectIdentifier_of(oid) = additional_info;
		}

		break;

	    case LWK_K_P_OBJECT_DOMAIN :
		if (!keys_only) {
		    value_length = sizeof(_Integer);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) &integer, &get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    _ObjectDomain_of(oid) = (_Domain) additional_info;
		}

		break;

	    case LWK_K_P_CONTAINER_ID :
		if (!keys_only) {
		    value_length = sizeof(_Integer);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) &integer, &get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    _ContainerIdentifier_of(oid) = additional_info;
		}

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_OBJECTID);

    return;
    }


_Persistent  LwkOpOidRetrieve(oid)
_ObjectId oid;

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
    _Linkbase repository;
    _Transaction volatile old_state;
    _Persistent volatile persistent;

    /*
    **  Open the given Repository.
    */
    
    repository = (_Linkbase) _Open(_TypeLinkbase,
	_LinkbaseIdentifier_of(oid), _False);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    /*
    **  Retrieve the given object from the Repository.
    */

    persistent = (_Persistent) _NullObject;

    _StartExceptionBlock

    persistent = LwkLbRetrieve(repository, _ObjectDomain_of(oid),
	_ObjectIdentifier_of(oid), _ContainerIdentifier_of(oid));

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    /*
    **  Close the Repository.
    */

    _Close(repository);

    /*
    **	If any exceptions are raised, clean up, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&persistent);

	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);

	    _Close(repository);

	    _Reraise;
    _EndExceptionBlock

    return persistent;
    }
