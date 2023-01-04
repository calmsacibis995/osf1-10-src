/*
** COPYRIGHT (c) 1988, 1991 BY
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
**	Persistent object methods 
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
#include "persist.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_persistent.h"
#include "his_database.h"
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

_DeclareFunction(static _Termination _EntryPt EncodeProperty,
    (_Closure closure, _Set properties, _Domain domain, _Property *property));
_DeclareFunction(static void EnumerateProperties,
    (_Set set, _Persistent persistent));
_DeclareFunction(static _Boolean IsMultiValuedProperty,
    (_Persistent persistent, _String property));
_DeclareFunction(static _Domain GetValueDomainProperty,
    (_Persistent persistent, _String property));
_DeclareFunction(static void GetValueProperty,
    (_Persistent persistent, _String property, _Domain domain, _AnyPtr value));
_DeclareFunction(static _List_of(_Domain) GetValueListProperty,
    (_Persistent persistent, _String property));
_DeclareFunction(static void SetValueProperty,
    (_Persistent persistent, _String property, _Domain domain, _AnyPtr value,
	_SetFlag flag));
_DeclareFunction(static void SetValueListProperty,
    (_Persistent persistent, _String property, _List values, _SetFlag flag));
_DeclareFunction(static void GetObjectName,
    (_Persistent persistent, _DDIFString volatile *object_name));
_DeclareFunction(static _Termination _EntryPt AddPropertyToList,
    (_Closure prop_names, _Set props, _Domain domain, _Property *prop));
_DeclareFunction(static _Termination _EntryPt FindProperty,
    (_Closure property_name, _Set prop_set, _Domain domain, _Property *prop));
_DeclareFunction(static void EncodeKeywords,
    (_Persistent persistent, _DDIShandle handle));
_DeclareFunction(static _Termination _EntryPt EncodeKeyword,
    (_Closure closure, _Set keywords, _Domain domain, _DDIFString *keyword));
_DeclareFunction(static void DecodeKeywords,
    (_Persistent persistent, _DDIShandle handle, _Boolean keys_only));

/*
**  Global Data Definitions
*/

_Global _PersistentTypeInstance;      /* Instance record for Type */
_Global _PersistentType;              /* Type */

/*
**  External Data Declarations
*/

_External long int lwk_lwkpersistent;

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypePersistentInstance;
static _PersistentPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    PersistentPropertyNameTable;

/*
**  List of Parse Tables for DDIS Toolkit.
*/

static unsigned long int ParseTableList[2] =
    {1, (unsigned long int) &lwk_lwkpersistent};


void  LwkOpPersist(persistent)
_Persistent persistent;

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


_Persistent  LwkOpPersistImport(our_type, encoding)
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
    _DDIShandle handle;
    _Persistent volatile persistent;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;

    handle = 0;
    persistent = (_Persistent) _NullObject;

    _StartExceptionBlock

    /*
    **  Create an object of the given type
    */

    persistent = (_Persistent) _Create(our_type);

    /*
    **  Set up the stream for the DDIS toolkit.
    */

    handle = _OpenDDISStream(encoding, ParseTableList);

    /*
    **  Begin the Persistent Object decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length,
	&entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERSISTENT)
	_Raise(inv_encoding);

    /*
    **  Begin the Persistent Object subtype decoding.
    */

      status = ddis_get_tag(&handle, &type, &value_length,
	&entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_TYPE)
	_Raise(inv_encoding);

    /*
    **  Invoke the _Decode operation on the object
    */

    _Decode((_Object) persistent, handle, _False, (_SetPtr) _NullObject);

    /*
    **  End the Persistent Object decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    if (entry != LWK_K_E_PERSISTENT)
	_Raise(inv_encoding);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If any exceptions are raised, delete the Persistent object, close the
    **	DDIS stream if it is still open, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&persistent);

	    if (handle != 0)
		_CloseDDISStream(handle);

	    _Reraise;
    _EndExceptionBlock

    return persistent;
    }


void  LwkOpPersistImportKeyProperties(our_type, encoding, properties)
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
    _DDIShandle handle;
    _Persistent volatile persistent;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;

    /*
    **  Initialize
    */

    handle = 0;
    *properties = (_Set) _NullObject;
    persistent = (_Persistent) _NullObject;

    _StartExceptionBlock

    /*
    **  Create an object of the given type
    */

    persistent = (_Persistent) _Create(our_type);

    /*
    **  Set up the stream for the DDIS toolkit.
    */

    handle = _OpenDDISStream(encoding, ParseTableList);

    /*
    **  Begin the Persistent Object decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERSISTENT)
	_Raise(inv_encoding);

    /*
    **  Begin the Persistent Object subtype decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_TYPE)
	_Raise(inv_encoding);

    /*
    **  Invoke the _Decode operation on the object
    */

    _Decode((_Object) persistent, handle, _True, properties);

    /*
    **  End the Persistent Object decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    if (entry != LWK_K_E_PERSISTENT)
	_Raise(inv_encoding);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **  We don't need the Persistent Object any more
    */

    _Delete(&persistent);

    /*
    **	If any exceptions are raised, delete the Persistent object, close the
    **	DDIS stream if it is still open, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&persistent);

	    if (handle != 0)
		_CloseDDISStream(handle);

	    _Reraise;
    _EndExceptionBlock

    return;
    }


void  LwkOpPersistInitialize(persistent, proto, aggregate)
_Persistent persistent;
 _Persistent proto;

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

    _ClearValue(&_Linkbase_of(persistent), lwk_c_domain_linkbase);
    _ClearValue(&_Identifier_of(persistent), lwk_c_domain_integer);
    _ClearValue(&_Description_of(persistent), lwk_c_domain_ddif_string);
    _ClearValue(&_Author_of(persistent), lwk_c_domain_ddif_string);
    _ClearValue(&_CreationDate_of(persistent), lwk_c_domain_date);
    _ClearValue(&_Keywords_of(persistent), lwk_c_domain_set);

    _ClearValue(&_Properties_of(persistent), lwk_c_domain_set);
    _StateFlags_of(persistent) = _InitialStateFlags;

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(persistent, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Persistent) _NullObject) {
	_CopyValue(&_Description_of(proto), &_Description_of(persistent),
	    lwk_c_domain_ddif_string);
	_CopyValue(&_Author_of(proto), &_Author_of(persistent),
	    lwk_c_domain_ddif_string);
	_CopyValue(&_CreationDate_of(proto), &_CreationDate_of(persistent),
	    lwk_c_domain_date);
	_CopyValue(&_Keywords_of(proto), &_Keywords_of(persistent),
	    lwk_c_domain_set);
	_CopyValue(&_Properties_of(proto), &_Properties_of(persistent),
	    lwk_c_domain_set);
    }

    return;
    }


void  LwkOpPersistFree(persistent)
_Persistent persistent;

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
    **  Remove the object from the Linkbase's Object cache
    */

    LwkLbUnCache(_Linkbase_of(persistent), _Identifier_of(persistent),
	persistent);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Linkbase_of(persistent), lwk_c_domain_linkbase);
    _DeleteValue(&_Identifier_of(persistent), lwk_c_domain_integer);
    _DeleteValue(&_Description_of(persistent), lwk_c_domain_ddif_string);
    _DeleteValue(&_Author_of(persistent), lwk_c_domain_ddif_string);
    _DeleteValue(&_CreationDate_of(persistent), lwk_c_domain_date);
    _DeleteValue(&_Keywords_of(persistent), lwk_c_domain_set);

    _DeleteValue(&_Properties_of(persistent), lwk_c_domain_set);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(persistent, MyType);

    return;
    }


_Set  LwkOpPersistEnumProps(persistent)
_Persistent persistent;

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

    set = (_Set) _EnumerateProperties_S(persistent, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Add the names of the non-base properties
    */
    
    EnumerateProperties(set, persistent);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpPersistIsMultiValued(persistent, property)
_Persistent persistent;
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
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    answer = _IsMultiValued_S(persistent, property, MyType);
	else
	    answer = IsMultiValuedProperty(persistent, property);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
	    case _IdentifierIndex :
	    case _DescriptionIndex :
	    case _AuthorIndex :
	    case _CreationDateIndex :
		answer = _False;
		break;

	    case _KeywordsIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpPersistGetValueDomain(persistent, property)
_Persistent persistent;
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
	/*
	**  If property is a base property, pass the request to our supertype.
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    domain = (_Domain) _GetValueDomain_S(persistent, property, MyType);
	else
	    domain = GetValueDomainProperty(persistent, property);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
		domain = lwk_c_domain_linkbase;
		break;

	    case _IdentifierIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _DescriptionIndex :
	    case _AuthorIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _CreationDateIndex :
		domain = lwk_c_domain_date;
		break;

	    case _KeywordsIndex :
		domain = lwk_c_domain_set;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpPersistGetValue(persistent, property, domain, value)
_Persistent persistent;
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
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _GetValue_S(persistent, property, domain, value, MyType);
	else
	    GetValueProperty(persistent, property, domain, value);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
		if (_IsDomain(domain, lwk_c_domain_linkbase))
		    _CopyValue(&_Linkbase_of(persistent), value,
			lwk_c_domain_linkbase);
		else
		    _Raise(inv_domain);

		break;

	    case _IdentifierIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Identifier_of(persistent), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _DescriptionIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Description_of(persistent), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _AuthorIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Author_of(persistent), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);

		break;

	    case _CreationDateIndex :
		if (_IsDomain(domain, lwk_c_domain_date))
		    _CopyValue(&_CreationDate_of(persistent), value,
			lwk_c_domain_date);
		else
		    _Raise(inv_domain);

		break;

	    case _KeywordsIndex :
		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_Keywords_of(persistent), value,
			lwk_c_domain_set);
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


_List  LwkOpPersistGetValueList(persistent, property)
_Persistent persistent;

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
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    value_list = (_List) _GetValueList_S(persistent, property, MyType);
	else
	    value_list = GetValueListProperty(persistent, property);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
		_ListValue(&_Linkbase_of(persistent), &value_list,
		    lwk_c_domain_linkbase);
		break;

	    case _IdentifierIndex :
		_ListValue(&_Identifier_of(persistent), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _DescriptionIndex :
		_ListValue(&_Description_of(persistent), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _AuthorIndex :
		_ListValue(&_Author_of(persistent), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _CreationDateIndex :
		_ListValue(&_CreationDate_of(persistent), &value_list,
		    lwk_c_domain_date);
		break;

	    case _KeywordsIndex :
		_CopyValue(&_Keywords_of(persistent), &value_list,
		    lwk_c_domain_set);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpPersistSetValue(persistent, property, domain, value, flag)
_Persistent persistent;
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
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _SetValue_S(persistent, property, domain, value, flag, MyType);
	else
	    SetValueProperty(persistent, property, domain, value, flag);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
		_SetSingleValuedProperty(&_Linkbase_of(persistent),
		    lwk_c_domain_linkbase, domain, value, flag, _False);
		break;

	    case _IdentifierIndex :
		_SetSingleValuedProperty(&_Identifier_of(persistent),
		    lwk_c_domain_integer, domain, value, flag, _False);
		break;

	    case _DescriptionIndex :
		_SetSingleValuedProperty(&_Description_of(persistent),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _AuthorIndex :
		_SetSingleValuedProperty(&_Author_of(persistent),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _CreationDateIndex :
		_SetSingleValuedProperty(&_CreationDate_of(persistent),
		    lwk_c_domain_date, domain, value, flag, _False);
		break;

	    case _KeywordsIndex :
		_SetMultiValuedProperty(&_Keywords_of(persistent),
		    lwk_c_domain_ddif_string, domain, value, flag, _False,
		    _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


void  LwkOpPersistSetValueList(persistent, property, values, flag)
_Persistent persistent;
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
	**  Otherwise check the non-base properties.
	*/

	if (_HasPrefixString(property, _BasePropertyPrefix))
	    _SetValueList_S(persistent, property, values, flag, MyType);
	else
	    SetValueListProperty(persistent, property, values, flag);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _LinkbaseIndex :
		_SetSingleValuedProperty(&_Linkbase_of(persistent),
		    lwk_c_domain_linkbase, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _IdentifierIndex :
		_SetSingleValuedProperty(&_Identifier_of(persistent),
		    lwk_c_domain_integer, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _DescriptionIndex :
		_SetSingleValuedProperty(&_Description_of(persistent),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _AuthorIndex :
		_SetSingleValuedProperty(&_Author_of(persistent),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _CreationDateIndex :
		_SetSingleValuedProperty(&_CreationDate_of(persistent),
		    lwk_c_domain_date, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _KeywordsIndex :
		_SetMultiValuedProperty(&_Keywords_of(persistent),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return;
    }


_ObjectId  LwkOpPersistGetObjectId(persistent)
_Persistent persistent;

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
    _Integer domain;
    _String volatile linkbase_id;
    _ObjectId volatile oid;

    /*
    **  Make sure this Object was stored/retrieved
    */

    if (_Linkbase_of(persistent) == (_Linkbase) _NullObject)
	_Raise(no_such_linkbase);

    oid = (_ObjectId) _NullObject;
    linkbase_id = (_String) _NullObject;

    _StartExceptionBlock

    /*
    **  Create the ObjectId object.
    */

    oid = (_ObjectId) _Create(_TypeObjectId);

    /*
    **  Set the Object Domain property of the ObjectId.
    */

    domain = (_Integer) _TypeToDomain(_Type_of(persistent));

    _SetValue(oid, _P_ObjectDomain, lwk_c_domain_integer, &domain,
	lwk_c_set_property);

    /*
    **  Set the Object Identifier property of the ObjectId.
    */

    _SetValue(oid, _P_ObjectIdentifier, lwk_c_domain_integer,
	&_Identifier_of(persistent), lwk_c_set_property);

    /*
    **  Set the Linkbase Identifier property of the ObjectId.
    */

    _GetValue(_Linkbase_of(persistent), _P_Identifier, lwk_c_domain_string,
	(_AnyPtr) &linkbase_id);

    _SetValue(oid, _P_LinkbaseIdentifier, lwk_c_domain_string,
	(_AnyPtr) &linkbase_id, lwk_c_set_property);

    _DeleteString(&linkbase_id);

    /*
    **	If any exceptions are raised, delete any new ObjectId and/or Linkbase
    **	id, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&oid);
	    _DeleteString(&linkbase_id);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the ObjectId.
    */

    return oid;
    }


#ifndef MSDOS
_ObjectDesc  LwkOpPersistGetObjectDesc(persistent)
_Persistent persistent;

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
    _Integer domain;
    _String volatile linkbase_id;
    _DDIFString volatile object_name;
    _DDIFString volatile linkbase_name;
    _ObjectDesc volatile odesc;

    /*
    **  Make sure this Object was stored/retrieved
    */

    if (_Linkbase_of(persistent) == (_Linkbase) _NullObject)
	_Raise(no_such_linkbase);

    odesc = (_ObjectDesc) _NullObject;
    linkbase_id = (_String) _NullObject;
    linkbase_name = (_DDIFString) _NullObject;
    object_name = (_DDIFString) _NullObject;

    _StartExceptionBlock

    /*
    **  Create the ObjectDesc object.
    */

    odesc = (_ObjectDesc) _Create(_TypeObjectDesc);

    /*
    **  Set the Object Domain property of the ObjectDesc.
    */

    domain = (_Integer) _TypeToDomain(_Type_of(persistent));

    _SetValue(odesc, _P_ObjectDomain, lwk_c_domain_integer, &domain,
	lwk_c_set_property);

    /*
    **  Set the Object Identifier property of the ObjectDesc.
    */

    _SetValue(odesc, _P_ObjectIdentifier, lwk_c_domain_integer,
	&_Identifier_of(persistent), lwk_c_set_property);

    /*
    **  Set the Linkbase Identifier property of the ObjectDesc.
    */

    _GetValue(_Linkbase_of(persistent), _P_Identifier, lwk_c_domain_string,
	(_AnyPtr) &linkbase_id);

    _SetValue(odesc, _P_LinkbaseIdentifier, lwk_c_domain_string,
	(_AnyPtr) &linkbase_id, lwk_c_set_property);

    /*
    **  Set the Linkbase Name property of the ObjectDesc.
    */

    _GetValue(_Linkbase_of(persistent), _P_Name, lwk_c_domain_ddif_string,
	(_AnyPtr) &linkbase_name);

    _SetValue(odesc, _P_LinkbaseName, lwk_c_domain_ddif_string,
	(_AnyPtr) &linkbase_name, lwk_c_set_property);

    /*
    **  Set the Object Name property of the ObjectDesc.
    */

    GetObjectName(persistent, &object_name);

    _SetValue(odesc, _P_ObjectName, lwk_c_domain_ddif_string,
	(_AnyPtr) &object_name, lwk_c_set_property);

    _DeleteString(&linkbase_id);
    _DeleteDDIFString(&linkbase_name);
    _DeleteDDIFString(&object_name);

    /*
    **	If any exceptions are raised, delete any new ObjectDesc and/or
    **	Linkbase id, then reraise the exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&odesc);
	    _DeleteString(&linkbase_id);
	    _DeleteDDIFString(&linkbase_name);
	    _DeleteDDIFString(&object_name);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the ObjectDesc.
    */

    return odesc;
    }
#endif /* !MSDOS */


_Integer  LwkOpPersistExport(persistent, aggregate, encoding)
_Persistent persistent;
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
    **  Begin the encoding of Persistent Object.
    */

    type = LWK_K_T_PERSISTENT;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Begin the encoding of the Persistent Object subtype.
    */

    type = LWK_K_T_PERS_TYPE;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0,
	(_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Invoke the _Encode operation on the object
    */

    _Encode((_Object) persistent, aggregate, handle);

    /*
    **  Finish the Persistent Object encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put((_DDIShandlePtr) &handle, &type, (_DDISlengthPtr) 0, (
	_DDISdata) 0, (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Close the stream.
    */

    _CloseDDISStream(handle);

    /*
    **	If any exceptions are raised, delete the partial encoding, close the
    **	DDIS stream, then reraise the exception.
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


void  LwkOpPersistEncode(persistent, aggregate, handle)
_Persistent persistent;
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
    _Integer count;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Finish the Persistent Object subtype encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Begin Persistent Object properties encoding.
    */

    type = LWK_K_T_PERS_PROPERTIES;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the properties of Persistent Object.
    */

    if (_Description_of(persistent) != (_DDIFString) _NullObject) {
	type = LWK_K_T_PERS_DESCRIPTION;
	value_length = _LengthDDIFString(_Description_of(persistent));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _Description_of(persistent), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    if (_Author_of(persistent) != (_DDIFString) _NullObject) {
	type = LWK_K_T_PERS_AUTHOR;
	value_length = _LengthDDIFString(_Author_of(persistent));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _Author_of(persistent), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    if (_CreationDate_of(persistent) != (_Date) _NullObject) {
	type = LWK_K_T_PERS_CREATION_DATE;
	value_length = _LengthDate(_CreationDate_of(persistent));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _CreationDate_of(persistent), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    EncodeKeywords(persistent, handle);

    /*
    **  Encode any non-base properties.
    */

    if (_Properties_of(persistent) != (_Set) _NullObject) {
	_GetValue(_Properties_of(persistent), _P_ElementCount,
	    lwk_c_domain_integer, &count);

	if (count > 0) {
	    /*
	    **  Begin the encoding of the non-base properties.
	    */

	    type = LWK_K_T_PERS_APPL_PROPS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of non-base properties.
	    */

	    type = LWK_K_T_PERS_PROP_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Begin the encoding the properties Set.
	    */

	    type = LWK_K_T_PERS_PROP_ELEMENTS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the properties, encoding each.
	    */

	    _Iterate(_Properties_of(persistent), lwk_c_domain_property,
		(_Closure) handle, EncodeProperty);

	    /*
	    **  Finish encoding the properties Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Finish encoding the non-base properties.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    /*
    **  Finish the Persistent Object properties encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    return;
    }


void  LwkOpPersistDecode(persistent, handle, keys_only, properties)
_Persistent persistent;
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
    int i;
    _DDISstatus status;
    _Integer integer;
    _Integer count;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Finish the Persistent Object subtype decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PERS_TYPE)
	_Raise(inv_encoding);

    /*
    **  Begin the Persistent Object properties decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_PROPERTIES)
	_Raise(inv_encoding);

    /*
    **	Decode the Persistent Object Properties.  If extracting Key Properties,
    **	all properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_PROPERTIES :
		/*
		**  End of Persistent Object Properties.
		*/
		break;

	    case LWK_K_P_PERS_DESCRIPTION :
		if (!keys_only) {
		    _Description_of(persistent) = _AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _Description_of(persistent), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_PERS_AUTHOR :
		if (!keys_only) {
		    _Author_of(persistent) = _AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _Author_of(persistent), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_PERS_CREATION_DATE :
		if (!keys_only) {
		    _CreationDate_of(persistent) =
			_AllocateMem(value_length + 1);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _CreationDate_of(persistent), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);

		    _CreationDate_of(persistent)[get_length] = _EndOfString;
		}

		break;

	    case LWK_K_P_PERS_KEYWORDS :
		DecodeKeywords(persistent, handle, keys_only);

		break;

	    case LWK_K_P_PERS_APPL_PROPS:
		/*
		**  Decode the Property count.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_P_PERS_PROP_COUNT)
		    _Raise(inv_encoding);

		value_length = sizeof(_Integer);

		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) &integer, &get_length, &cnt);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		count = (_Integer) cnt;

		/*
		**  Begin decoding the Property Set.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_P_PERS_PROP_ELEMENTS)
		    _Raise(inv_encoding);

		/*
		**  Decode count Properties.
		*/

		if (!keys_only)
		    _Properties_of(persistent) = (_Set)
			_CreateSet(_TypeSet, lwk_c_domain_property, count);

		for (i = 0; i < count; i++) {
		    _VaryingString volatile encoding;
		    _Property volatile prop;

		    status = ddis_get_tag(&handle, &type, &value_length, &entry,
			&table);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (type != DDIS_K_T_OCTET_STRING)
			_Raise(inv_encoding);

		    encoding = (_VaryingString) _NullObject;
		    prop = (_Property) _NullObject;

		    _StartExceptionBlock

		    encoding = (_VaryingString) _AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) encoding, &get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);

		    /*
		    **  Import the Property from the encoding.
		    */

		    if (keys_only)
			_ImportKeyProperties(_TypeProperty, encoding,
			    properties);
		    else {
			prop = (_Property) _Import(_TypeProperty, encoding);

			_AddElement(_Properties_of(persistent),
			    lwk_c_domain_property, (_AnyPtr) &prop, _False);
		    }

		    _Exceptions
			_WhenOthers
			    _DeleteEncoding(&encoding);
			    _Delete(&prop);
			    _Reraise;
		    _EndExceptionBlock

		    _DeleteEncoding(&encoding);
		}

		/*
		**  Finish decoding the Property Set.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_E_PERS_PROP_ELEMENTS)
		    _Raise(inv_encoding);

		/*
		**  Finish decoding the Properties.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_E_PERS_APPL_PROPS)
		    _Raise(inv_encoding);

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_PROPERTIES);

    return;
    }


_Boolean  LwkOpPersistSetState(persistent, state, operation)
_Persistent persistent;
 _PersistentState state;

    _StateOperation operation;

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
    _StateFlags flag;
    _Boolean old_state;

    /*
    **  Turn the given State into a Flag
    */

    flag = _StateToFlag(state);

    /*
    **  Get the old State
    */

    old_state = ((_StateFlags_of(persistent) & flag) != 0);

    /*
    **  Set the State as requested
    */

    switch (operation) {
	case _StateSet :
	    _StateFlags_of(persistent) |= flag;
	    break;

	case _StateClear :
	    _StateFlags_of(persistent) &= ~flag;
	    break;

	case _StateComplement :
	    _StateFlags_of(persistent) ^= flag;
	    break;

	case _StateGet :
	default :
	    break;
    }

    /*
    **  Return the old state
    */

    return old_state;
    }


static void  EncodeKeywords(persistent, handle)
_Persistent persistent;
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
    _Integer count;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Encode any Keywords
    */

    if (_Keywords_of(persistent) != (_Set) _NullObject) {
	_GetValue(_Keywords_of(persistent), _P_ElementCount,
	    lwk_c_domain_integer, &count);

	if (count > 0) {
	    /*
	    **  Begin the Keywords Set.
	    */

	    type = LWK_K_T_PERS_KEYWORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of the Keyword Set.
	    */

	    type = LWK_K_T_PERS_KEY_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Keywords Set.
	    */

	    type = LWK_K_T_PERS_WORDS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Keywords Set encoding each Keyword.
	    */

	    _Iterate(_Keywords_of(persistent), lwk_c_domain_ddif_string,
		(_Closure) handle, EncodeKeyword);

	    /*
	    **  End the Keywords Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Keywords Property.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    return;
    }


static _Termination _EntryPt  EncodeKeyword(closure, keywords, domain, keyword)
_Closure closure;
 _Set keywords;

    _Domain domain;
 _DDIFString *keyword;

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

    handle = (_DDIShandle) closure;

    /*
    **  Encode the Keyword.
    */
    
    type = DDIS_K_T_OCTET_STRING;
    value_length = _LengthDDIFString(*keyword);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) *keyword,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Continue the iteration.
    */

    return (_Termination) 0;
    }


static void  DecodeKeywords(persistent, handle, keys_only)
_Persistent persistent;
 _DDIShandle handle;

    _Boolean keys_only;

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
    _DDIFString ddifstring;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Decode the Keyword count.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry,
	&table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_KEY_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Keywords.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_WORDS)
	_Raise(inv_encoding);

    /*
    **	Decode count Keywords, or just skip them if extracting Key Properties.
    */

    if (!keys_only)
	_Keywords_of(persistent) =
	    (_Set) _CreateSet(_TypeSet, lwk_c_domain_ddif_string, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_OCTET_STRING)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    ddifstring = _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) ddifstring, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    _AddElement(_Keywords_of(persistent), lwk_c_domain_ddif_string,
		&ddifstring, _False);
	}
    }

    /*
    **  Finish decoding the Keywords Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry,
	&table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PERS_WORDS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Keywords.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry,
	&table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PERS_KEYWORDS)
	_Raise(inv_encoding);

    return;
    }


static _Termination _EntryPt  EncodeProperty(closure, properties, domain, property)
_Closure closure;
 _Set properties;

    _Domain domain;
 _Property *property;

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
    _DDIShandle handle;
    _DDISstatus status;
    _Integer size;
    _VaryingString encoding;
    _DDIStype type;

    handle = (_DDIShandle) closure;

    /*
    **  Export the Property.
    */

    size = _Export(*property, _False, &encoding);

    /*
    **  Encode the Property.
    */

    type = DDIS_K_T_OCTET_STRING;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) &size,
	(_DDISdata) encoding, (_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Continue the iteration.
    */

    return (_Termination) 0;
    }


static void  EnumerateProperties(set, persistent)
_Set set;
 _Persistent persistent;

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
    **  If there is no Properties set, then there is nothing to do.
    */

    if (_Properties_of(persistent) == (_Set) _NullObject)
	return;

    /*
    **  Iterate over the set of Properties, adding each one's name to the
    **	property name set.
    */

    _Iterate(_Properties_of(persistent), lwk_c_domain_property, (_Closure) set,
	AddPropertyToList);

    return;
    }


static _Boolean  IsMultiValuedProperty(persistent, property)
_Persistent persistent;
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
    _Boolean answer;
    _Property prop;

    /*
    **  If there is no Properties set, then there no such property.
    */

    if (_Properties_of(persistent) == (_Set) _NullObject)
	_Raise(no_such_property);

    prop = (_Property) _Iterate(_Properties_of(persistent),
	lwk_c_domain_property, (_Closure) property, FindProperty);

    if (prop == (_Property) _NullObject)
	_Raise(no_such_property);

    answer = _IsMultiValued(prop, _P_Value);

    return answer;
    }


static _Domain  GetValueDomainProperty(persistent, property)
_Persistent persistent;
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
    _Integer domain;
    _Property prop;

    /*
    **  If there is no Properties set, then there no such property.
    */

    if (_Properties_of(persistent) == (_Set) _NullObject)
	_Raise(no_such_property);

    prop = (_Property) _Iterate(_Properties_of(persistent),
	lwk_c_domain_property, (_Closure) property, FindProperty);

    if (prop == (_Property) _NullObject)
	_Raise(no_such_property);

    _GetValue(prop, _P_Domain, lwk_c_domain_integer, &domain);

    return (_Domain) domain;
    }


static void  GetValueProperty(persistent, property, domain, value)
_Persistent persistent;
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
    _Property prop;

    /*
    **  If there is no Properties set, then there no such property.
    */

    if (_Properties_of(persistent) == (_Set) _NullObject)
	_Raise(no_such_property);

    prop = (_Property) _Iterate(_Properties_of(persistent),
	lwk_c_domain_property, (_Closure) property, FindProperty);

    if (prop == (_Property) _NullObject)
	_Raise(no_such_property);

    _GetValue(prop, _P_Value, domain, value);

    return;
    }


static _List  GetValueListProperty(persistent, property)
_Persistent persistent;
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
    _List value_list;
    _Property prop;

    /*
    **  If there is no Properties set, then there no such property.
    */

    if (_Properties_of(persistent) == (_Set) _NullObject)
	_Raise(no_such_property);

    prop = (_Property) _Iterate(_Properties_of(persistent),
	lwk_c_domain_property, (_Closure) property, FindProperty);

    if (prop == (_Property) _NullObject)
	_Raise(no_such_property);

    value_list = (_List) _GetValueList(prop, _P_Value);

    return value_list;
    }


static void  SetValueProperty(persistent, property, domain, value, flag)
_Persistent persistent;
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
    _Boolean volatile new_property;
    _Property volatile prop;

    prop = (_Property) _NullObject;

    _StartExceptionBlock

    /*
    **	If there are any Properties, search the list for one with the
    **	specified name.
    */

    if (_Properties_of(persistent) != (_Set) _NullObject)
	prop = (_Property) _Iterate(_Properties_of(persistent),
	    lwk_c_domain_property, (_Closure) property, FindProperty);

    /*
    **	If there were no Properties, or there are none with the specified name,
    **	remember for later.
    */

    if (prop != (_Property) _NullObject)
	new_property = _False;
    else
	new_property = _True;

    /*
    **	Set/Delete the Property value.
    */

    if (flag == lwk_c_delete_property) {
	if (!new_property)
	    _DeleteElement(_Properties_of(persistent), lwk_c_domain_property,
		(_AnyPtr) &prop);
    }
    else {
	/*
	**  If necessary, create a new Property with the given name after
	**  checking its validity.
	*/
	
	if (new_property) {
	    if (!_IsValidPropertyName(property))
		_Raise(inv_argument);

	    prop = (_Property) _Create(_TypeProperty);

	    _SetValue(prop, _P_PropertyName, lwk_c_domain_string, &property,
		lwk_c_set_property);
	}

	/*
	**  Set the Property value.
	*/

	_SetValue(prop, _P_Value, domain, value, flag);

	/*
	**  If a new Property was created, add it to the Property list.
	*/

	if (new_property) {
	    if (_Properties_of(persistent) == (_Set) _NullObject)
		_Properties_of(persistent) = (_Set) _CreateSet(_TypeSet,
		    lwk_c_domain_property, (_Integer) 0);

	    _AddElement(_Properties_of(persistent), lwk_c_domain_property,
		(_AnyPtr) &prop, _False);
	}
    }

    /*
    **  Mark the Persistent Object modified
    */

    _SetState((_Object) persistent, _StateModified, _StateSet);

    /*
    **	If any exceptions are raised, delete any new Property, then reraise
    **	the exception.
    */

    _Exceptions
	_WhenOthers
	    if (new_property)
		_Delete(&prop);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetValueListProperty(persistent, property, values, flag)
_Persistent persistent;
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
    _Boolean volatile new_property;
    _Property volatile prop;

    prop = (_Property) _NullObject;

    _StartExceptionBlock

    /*
    **	If there are any Properties, search the list for one with the
    **	specified name.
    */

    if (_Properties_of(persistent) != (_Property) _NullObject)
	prop = (_Property) _Iterate(_Properties_of(persistent),
	    lwk_c_domain_property, (_Closure) property, FindProperty);

    /*
    **	If there were no Properties, or there are none with the specified name,
    **	remember for later.
    */

    if (prop != (_Property) _NullObject)
	new_property = _False;
    else
	new_property = _True;

    /*
    **	Set/Delete the Property value.
    */

    if (flag == lwk_c_delete_property) {
	if (!new_property)
	    _DeleteElement(_Properties_of(persistent), lwk_c_domain_property,
		(_AnyPtr) &prop);
    }
    else {
	/*
	**  If necessary, create a new Property with the given name after
	**  checking its validity.
	*/
	
	if (new_property) {
	    if (!_IsValidPropertyName(property))
		_Raise(inv_argument);

	    prop = (_Property) _Create(_TypeProperty);

	    _SetValue(prop, _P_PropertyName, lwk_c_domain_string, &property,
		lwk_c_set_property);
	}

	/*
	**  Set the Property value.
	*/

	_SetValueList(prop, _P_Value, values, flag);

	/*
	**  If a new Property was created, add it to the Property list.
	*/

	if (new_property) {
	    if (_Properties_of(persistent) == (_Set) _NullObject)
		_Properties_of(persistent) = (_Set) _CreateSet(_TypeSet,
		    lwk_c_domain_property, (_Integer) 0);

	    _AddElement(_Properties_of(persistent), lwk_c_domain_property,
		(_AnyPtr) &prop, _False);
	}
    }

    /*
    **  Mark the Persistent Object modified
    */

    _SetState((_Object) persistent, _StateModified, _StateSet);

    /*
    **	If any exceptions are raised, delete any new Property, then reraise
    **	the exception.
    */

    _Exceptions
	_WhenOthers
	    if (new_property)
		_Delete(&prop);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  GetObjectName(persistent, object_name)
_Persistent persistent;

    _DDIFString volatile *object_name;

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
    _StartExceptionBlock

    _GetValue((_Object) persistent, _P_Name, lwk_c_domain_ddif_string,
	(_AnyPtr) object_name);

    _Exceptions
	_When(no_such_property)
	    *object_name = (_DDIFString) _NullObject;
	_WhenOthers
	_Reraise;
    _EndExceptionBlock

    return;
    }


static _Termination _EntryPt  AddPropertyToList(prop_names, props, domain, prop)
_Closure prop_names;
 _Set props;

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
    _String name;

    /*
    **  Get the _PropertyName from the Property.
    */

    _GetValue(*prop, _P_PropertyName, lwk_c_domain_string, &name);

    /*
    **  Add the name to the property name set.
    */

    _AddElement((_Set) prop_names, lwk_c_domain_string, &name, _False);

    /*
    **  Return zero so that the iteration will continue.
    */

    return (_Termination) 0;
    }


static _Termination _EntryPt  FindProperty(property_name, prop_set, domain, prop)
_Closure property_name;
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

    if (_IsNamed(*prop, (_String) property_name))
	answer = (_Termination) *prop;
    else
	answer = (_Termination) 0;

    return answer;
    }
