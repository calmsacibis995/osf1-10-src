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
**	Linknet object methods 
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
#include "network.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_linknet.h"
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

_DeclareFunction(static void CopyAggregate, (_Linknet network, _Linknet proto));
_DeclareFunction(static _Termination _EntryPt CopySurrogate,
    (_Closure closure, _List list, _Domain domain,
	_SurrogatePtr surrogate));
_DeclareFunction(static _Termination _EntryPt CopyLink,
    (_Closure closure, _List list, _Domain domain,
	_LinkPtr link));
_DeclareFunction(static _Termination _EntryPt DropSubObject,
    (_Closure null, _List list, _Domain domain, _PersistentPtr object));
_DeclareFunction(static void RetrieveSurrogates, (_Linknet network));
_DeclareFunction(static void RetrieveLinks, (_Linknet network));
_DeclareFunction(static void QuerySurrogates,
    (_Linknet network, _QueryExpression expression));
_DeclareFunction(static void QueryLinks,
    (_Linknet network, _QueryExpression expression));
_DeclareFunction(static _Termination _EntryPt DeleteSubObject,
    (_Closure null, _List list, _Domain domain, _PersistentPtr object));
_DeclareFunction(static _Termination _EntryPt QueryNetwork,
    (_Closure closure, _Linknet network, _Domain domain,
	_ObjectPtr object));
_DeclareFunction(static void EncodeSurrogates,
    (_Linknet network, _DDIShandle handle));
_DeclareFunction(static _Termination _EntryPt EncodeSurrogate,
    (_Closure closure, _Set surrogates, _Domain domain,
	_SurrogatePtr surrogate));
_DeclareFunction(static void EncodeLinks,
    (_Linknet network, _DDIShandle handle));
_DeclareFunction(static _Termination _EntryPt EncodeLink,
    (_Closure closure, _Set links, _Domain domain,
	_LinkPtr link));
_DeclareFunction(static _HashTableEntryPtr DecodeSurrogates,
    (_Linknet network, _DDIShandle handle, _Boolean keys_only));
_DeclareFunction(static void DecodeLinks,
    (_Linknet network, _DDIShandle handle, _HashTableEntryPtr table,
	_Boolean keys_only));
_DeclareFunction(static void FreeHashTable, (_HashTableEntryPtr table));
_DeclareFunction(static _Termination _EntryPt PreStoreLink,
    (_Closure repository, _List list, _Domain domain,
	_LinkPtr link));
_DeclareFunction(static _Termination _EntryPt StoreSubObject,
    (_Closure repository, _List list, _Domain domain,
	_PersistentPtr object));

/*
**  External Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _LinknetTypeInstance;   /* Instance record for Type */
_Global _LinknetType;           /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeLinknetInstance;
static _LinknetPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    LinknetPropertyNameTable;


void  LwkOpLnet(network)
_Linknet network;

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


void  LwkOpLnetInitialize(network, proto, aggregate)
_Linknet network;
 _Linknet proto;
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

    _ClearValue(&_Name_of(network), lwk_c_domain_ddif_string);
    _ClearValue(&_Links_of(network), lwk_c_domain_set);
    _ClearValue(&_Surrogates_of(network), lwk_c_domain_set);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(network, (_Object) proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Linknet) _NullObject) {
	_CopyValue(&_Name_of(proto), &_Name_of(network),
	    lwk_c_domain_ddif_string);

	if (aggregate)
	    CopyAggregate(proto, network);
    }

    return;
    }


void  LwkOpLnetFree(network)
_Linknet network;

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
    **	Delete any Links
    */

    if (_Links_of(network) != (_Set) _NullObject)
	_Iterate(_Links_of(network), lwk_c_domain_link,
	    (_Closure) 0, DeleteSubObject);

    /*
    **  Now, delete any Surrogates.
    */

    if (_Surrogates_of(network) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(network), lwk_c_domain_surrogate,
	    (_Closure) 0, DeleteSubObject);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Name_of(network), lwk_c_domain_ddif_string);
    _DeleteValue(&_Links_of(network), lwk_c_domain_set);
    _DeleteValue(&_Surrogates_of(network), lwk_c_domain_set);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(network, MyType);

    return;
    }


_Set  LwkOpLnetEnumProps(network)
_Linknet network;

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

    set = (_Set) _EnumerateProperties_S(network, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpLnetIsMultiValued(network, property)
_Linknet network;
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
	answer = _IsMultiValued_S(network, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
	    case _LinksIndex :
	    case _SurrogatesIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpLnetGetValueDomain(network, property)
_Linknet network;
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
	domain = (_Domain) _GetValueDomain_S(network, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _LinksIndex :
	    case _SurrogatesIndex :
		domain = lwk_c_domain_set;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpLnetGetValue(network, property, domain, value)
_Linknet network;
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
	_GetValue_S(network, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Name_of(network), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);
		break;

	    case _LinksIndex :
		/*
		**  Retrieve the Links if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) network,
			_StateHaveAllLinks, _StateSet))
		    RetrieveLinks(network);

		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_Links_of(network), value,
			lwk_c_domain_set);
		else
		    _Raise(inv_domain);

		break;

	    case _SurrogatesIndex :
		/*
		**  Retrieve the Surrogates if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) network,
			_StateHaveAllSurrogates, _StateSet))
		    RetrieveSurrogates(network);

		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_Surrogates_of(network), value,
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


_List  LwkOpLnetGetValueList(network, property)
_Linknet network;
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
	return (_List) _GetValueList_S(network, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_ListValue(&_Name_of(network), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _LinksIndex :
		/*
		**  Retrieve the Links if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) network,
			_StateHaveAllLinks, _StateSet))
		    RetrieveLinks(network);

		_CopyValue(&_Links_of(network), &value_list,
		    lwk_c_domain_set);
		break;

	    case _SurrogatesIndex :
		/*
		**  Retrieve the Surrogates if we don't have them.
		*/

		if (!(_Boolean) _SetState((_Persistent) network,
			_StateHaveAllSurrogates, _StateSet))
		    RetrieveSurrogates(network);

		_CopyValue(&_Surrogates_of(network), &value_list,
		    lwk_c_domain_set);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpLnetSetValue(network, property, domain, value, flag)
_Linknet network;
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
	_SetValue_S(network, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(network),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _LinksIndex :
		_SetMultiValuedProperty(&_Links_of(network),
		    lwk_c_domain_link, domain, value, flag,
		    _False, _True);
		break;

	    case _SurrogatesIndex :
		_SetMultiValuedProperty(&_Surrogates_of(network),
		    lwk_c_domain_surrogate, domain, value, flag, _False, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Network modified
    */

    _SetState((_Persistent) network, _StateModified, _StateSet);

    return;
    }


void  LwkOpLnetSetValueList(network, property, values, flag)
_Linknet network;
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
	_SetValueList_S(network, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(network),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _LinksIndex :
		_SetMultiValuedProperty(&_Links_of(network),
                    lwk_c_domain_link, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    case _SurrogatesIndex :
		_SetMultiValuedProperty(&_Surrogates_of(network),
                    lwk_c_domain_surrogate, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Network modified
    */

    _SetState((_Persistent) network, _StateModified, _StateSet);

    return;
    }


void  LwkOpLnetEncode(network, aggregate, handle)
_Linknet network;
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

    /*
    **  Begin the encoding of Network.
    */

    type = LWK_K_T_PERS_LINKNET;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the properties of Network.
    */

    if (_Name_of(network) != (_DDIFString) _NullObject) {
	type = LWK_K_T_LNET_NAME;
	value_length = _LengthDDIFString(_Name_of(network));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _Name_of(network), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **	If encoding the aggregate Network, encode the Surrogates and
    **	Links as well.
    */

    if (aggregate) {
	EncodeSurrogates(network, handle);
	EncodeLinks(network, handle);
    }

    /*
    **  Finish the Network encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(network, aggregate, handle, MyType);

    return;
    }


void  LwkOpLnetDecode(network, handle, keys_only, properties)
_Linknet network;
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
    _HashTableEntryPtr volatile hash_table;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;

    /*
    **  Begin the Network decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_LINKNET)
	_Raise(inv_encoding);

    /*
    **  Initialize
    */

    hash_table = (_HashTableEntryPtr) 0;

    _StartExceptionBlock

    /*
    **	Decode the Network Properties.  If extracting Key Properties, all
    **	properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_LINKNET:
		/*
		**  End of Network Properties.
		*/
		break;

	    case LWK_K_P_LNET_NAME :
		if (!keys_only) {
		    _Name_of(network) = _AllocateMem(value_length);

		    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
			(_DDISdata) _Name_of(network), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_LNET_SURROGATES :
		/*
		**  Decode the Surrogates
		*/

		hash_table = DecodeSurrogates(network, handle, keys_only);

		break;

	    case LWK_K_P_LNET_LINKS :
		/*
		**  Decode the Links
		*/

		DecodeLinks(network, handle, hash_table, keys_only);

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_LINKNET);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    FreeHashTable(hash_table);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Free the hash table
    */

    FreeHashTable(hash_table);

    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(network, handle, keys_only, properties, MyType);

    return;
    }


_Termination  LwkOpLnetIterate(network, domain, closure, routine)
_Linknet network;
 _Domain domain;

    _Closure closure;
 _Callback routine;

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
    _Termination termination;

    /*
    **  Iterate over the domain requested.
    */

    switch (domain) {
	case lwk_c_domain_surrogate :
	    /*
	    **  Retrieve the Surrogates if we don't have them.
	    */

	    if (!(_Boolean) _SetState((_Persistent) network,
		    _StateHaveAllSurrogates, _StateSet))
		RetrieveSurrogates(network);

	    if (_Surrogates_of(network) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Surrogates_of(network),
		    lwk_c_domain_surrogate, closure, routine);
	    break;

	case lwk_c_domain_link :
	    /*
	    **  Retrieve the Links if we don't have them.
	    */

	    if (!(_Boolean) _SetState((_Persistent) network,
		    _StateHaveAllLinks, _StateSet))
		RetrieveLinks(network);

	    if (_Links_of(network) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Links_of(network),
		    lwk_c_domain_link, closure, routine);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


_Termination  LwkOpLnetQuery(network, domain, expression, closure, routine)
_Linknet network;
 _Domain domain;

    _QueryExpression expression;
 _Closure closure;
 _Callback routine;

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
    _QueryContext context;
    _Termination termination;

    /*
    **  Iterate over the given domain, evaluating the selection expression on
    **	each object of the domain.
    */

    context.routine = routine;
    context.closure = closure;
    context.domain  = domain;
    context.expression = expression;

    switch (domain) {
	case lwk_c_domain_link :
	    /*
	    **	Pre-fetch the appropriate Links if we don't have them
	    **	all.
	    */

	    if (!(_Boolean) _SetState((_Persistent) network,
		    _StateHaveAllLinks, _StateGet))
		QueryLinks(network, expression);

	    if (_Links_of(network) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Links_of(network),
		    lwk_c_domain_link, (_Closure) &context, QueryNetwork);

	    break;

	case lwk_c_domain_surrogate :
	    /*
	    **	Pre-fetch the appropriate Surrogates if we don't have them all.
	    */

	    if (!(_Boolean) _SetState((_Persistent) network,
		    _StateHaveAllSurrogates, _StateGet))
		QuerySurrogates(network, expression);

	    if (_Surrogates_of(network) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Surrogates_of(network),
		    lwk_c_domain_surrogate, (_Closure) &context, QueryNetwork);

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


void  LwkOpLnetStore(network, repository)
_Linknet network;
 _Linkbase repository;

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
    _Transaction old_state;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **	Before storing anything, Iterate over the Links and make sure
    **	that each Link has an assigned Identifier (we need this so that
    **	the Id's are available during the Store of the Surrogates
    **	(InterLink Id's)).
    */

    if (_Links_of(network) != (_Set) _NullObject)
	_Iterate(_Links_of(network), lwk_c_domain_link,
	    (_Closure) repository, PreStoreLink);

    /*
    **  Now, Store the Network in the Repository.
    */

    LwkLbStore(repository, lwk_c_domain_linknet, (_Persistent) network);

    /*
    **	Then, Iterate over the Surrogates -- drop any which were deleted, store
    **	the rest.
    */

    if (_Surrogates_of(network) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(network), lwk_c_domain_surrogate,
	    (_Closure) repository, StoreSubObject);

    /*
    **  Iterate over the Links -- drop/store them as well
    */

    if (_Links_of(network) != (_Set) _NullObject)
	_Iterate(_Links_of(network), lwk_c_domain_link,
	    (_Closure) repository, StoreSubObject);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    return;
    }



void  LwkOpLnetDrop(network)
_Linknet network;

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
    _Transaction old_state;
    _Linkbase repository;

    /*
    **	Find the Repository in which the Network is stored.  If there isn't
    **	one, return now.
    */

    _GetValue((_Object) network, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    if (repository == (_Linkbase) _NullObject)
	return;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **  Retrieve the Links if we don't have them.
    */

    if (!(_Boolean) _SetState((_Persistent) network, _StateHaveAllLinks,
	    _StateSet))
	RetrieveLinks(network);

    /*
    **  Iterate over the Links -- drop each one.
    */

    if (_Links_of(network) != (_Set) _NullObject)
	_Iterate(_Links_of(network), lwk_c_domain_link,
	    (_Closure) 0, DropSubObject);

    /*
    **  Retrieve the Surrogates if we don't have them.
    */

    if (!(_Boolean) _SetState((_Persistent) network, _StateHaveAllSurrogates,
	    _StateSet))
	RetrieveSurrogates(network);

    /*
    **  Iterate over the Surrogates -- drop them as well
    */

    if (_Surrogates_of(network) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(network), lwk_c_domain_surrogate, (_Closure) 0,
	    DropSubObject);

    /*
    **  Drop the Network from the Repository where it is stored.
    */

    LwkLbDrop(lwk_c_domain_linknet, (_Persistent) network);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    return;
    }


static void  CopyAggregate(network, new)
_Linknet network;
 _Linknet new;

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
    _AggregateInstance volatile aggregate;

    /*
    ** Initialize
    */

    aggregate.aggregate = (_Object) new;

    aggregate.hash_table = (_HashTableEntryPtr)
	_AllocateMem(_ImportHashSize * sizeof(_HashTableEntry));

    _HashInitialize(aggregate.hash_table, _ImportHashSize);

    _StartExceptionBlock

    /*
    ** Copy all of the Surrogates
    */

    _Iterate((_Object) network, lwk_c_domain_surrogate, (_Closure) &aggregate,
	CopySurrogate);

    /*
    ** Copy all the Links and set their Sources/Targets
    */

    _Iterate((_Object) network, lwk_c_domain_link, (_Closure) &aggregate,
	CopyLink);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    FreeHashTable(aggregate.hash_table);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Free the hash table
    */

    FreeHashTable(aggregate.hash_table);

    return;
    }


static _Termination _EntryPt  CopySurrogate(closure, list, domain, surrogate)
_Closure closure;
 _List list;

    _Domain domain;
 _SurrogatePtr surrogate;

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
    _Surrogate new;
    _Aggregate aggregate;

    aggregate = (_Aggregate) closure;

    /*
    ** Copy the Surrogate and put it into the new Network
    */

    new = (_Surrogate) _Copy(*surrogate, _False);

    _SetValue(new, _P_Container, lwk_c_domain_linknet, &aggregate->aggregate,
	lwk_c_set_property);

    /*
    ** Add the Surrogate to the hash table using the old Surrogate as a key.
    */

    _HashInsert(aggregate->hash_table, _ImportHashSize,
	(_Integer) *surrogate, new);

    return (_Termination) 0;
    }


static _Termination _EntryPt  CopyLink(closure, list, domain, link)
_Closure closure;
 _List list;

    _Domain domain;
 _LinkPtr link;

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
    _Link new;
    _Aggregate aggregate;
    _Surrogate surrogate;
    _Surrogate new_surrogate;

    aggregate = (_Aggregate) closure;


    /*
    ** Copy the Link
    */

    new = (_Link) _Copy(*link, _False);

    /*
    ** Find its new Source and Target in the hash table and set them
    */
    
    _GetValue(*link, _P_Source, lwk_c_domain_surrogate, &surrogate);

    if (!_HashSearch(aggregate->hash_table, _ImportHashSize,
	    (_Integer) surrogate, &new_surrogate))
	new_surrogate = (_Surrogate) _NullObject;

    _SetValue(new, _P_Source, lwk_c_domain_surrogate, &new_surrogate,
	lwk_c_set_property);

    _GetValue(*link, _P_Target, lwk_c_domain_surrogate, &surrogate);

    if (!_HashSearch(aggregate->hash_table, _ImportHashSize,
	    (_Integer) surrogate, &new_surrogate))
	new_surrogate = (_Surrogate) _NullObject;

    _SetValue(new, _P_Target, lwk_c_domain_surrogate, &new_surrogate,
	lwk_c_set_property);

    /*
    ** Put the new Link into the new Network
    */
    
    _SetValue(new, _P_Linknet, lwk_c_domain_linknet, &aggregate->aggregate,
	lwk_c_set_property);

    return (_Termination) 0;
    }


static _Termination _EntryPt  DropSubObject(null, list, domain, object)
_Closure null;
 _List list;

    _Domain domain;
 _PersistentPtr object;

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
    **  Drop the Sub-Object from the Repository.
    */

    LwkLbDrop(domain, *object);

    return (_Termination) 0;
    }


static void  RetrieveSurrogates(network)
_Linknet network;

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
    **  Retrieve all Surrogates is equivalent to a Query with a null Selection
    **	Expression.
    */

    QuerySurrogates(network, (_QueryExpression) _NullObject);

    return;

    }


static void  RetrieveLinks(network)
_Linknet network;

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
    **	Retrieve all Links is equivalent to a Query with a null Selection
    **	Expression.
    */

    QueryLinks(network, (_QueryExpression) _NullObject);

    return;
    }


static void  QuerySurrogates(network, expression)
_Linknet network;
 _QueryExpression expression;

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
    _Integer id;
    _Boolean modified;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Find the Repository in which the Network is stored
    */

    _GetValue((_Object) network, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Preserve the Modified state of the Network
    */

    modified = (_Boolean) _SetState((_Persistent) network, _StateModified,
	_StateGet);

    /*
    **  Retrieve the requested Surrogates
    */

    _GetValue((_Object) network, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    _GetValue((_Object) network, _P_Identifier, lwk_c_domain_integer, &id);

    LwkLbQuerySurrInContainer(repository, (_Persistent) network, id,
	expression);

    /*
    **  Restore the Modified state of the Network
    */

    if (!modified)
	_SetState((_Persistent) network, _StateModified, _StateClear);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    return;
    }


static void  QueryLinks(network, expression)
_Linknet network;
 _QueryExpression expression;

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
    _Integer id;
    _Boolean modified;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Find the Repository in which the Network is stored
    */

    _GetValue((_Object) network, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Preserve the Modified state of the Network
    */

    modified = (_Boolean) _SetState((_Persistent) network, _StateModified,
	_StateGet);

    /*
    **  Retrieve the requested Links
    */

    _GetValue((_Object) network, _P_Linkbase, lwk_c_domain_linkbase,
	&repository);

    _GetValue((_Object) network, _P_Identifier, lwk_c_domain_integer, &id);

    LwkLbQueryConnInNetwork(repository, network, id, expression);

    /*
    **  Restore the Modified state of the Network
    */

    if (!modified)
	_SetState((_Persistent) network, _StateModified, _StateClear);

    /*
    **  If there is an exception, rollback the transaction then reraise it.
    */

    _Exceptions
	_WhenOthers
	    if (old_state == lwk_c_transact_commit)
		_Transact(repository, lwk_c_transact_rollback);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  If we started a new transaction, commit it
    */

    if (old_state == lwk_c_transact_commit)
	_Transact(repository, lwk_c_transact_commit);

    return;
    }


static _Termination _EntryPt  DeleteSubObject(null, list, domain, object)
_Closure null;
 _List list;

    _Domain domain;
 _PersistentPtr object;

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
    _Persistent delete;

    /*
    **	Delete the SubObject (i.e., Link or Surrogate).  Must set
    **	PendingDelete to be sure it actually gets deleted!  Use a local pointer
    **	to Object since Delete will set the Object to the NullObject.
    */

    delete = *object;

    _SetState(delete, _StateDeletePending, _StateSet);

    _Delete(&delete);

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static _Termination _EntryPt  QueryNetwork(closure, network, domain, object)
_Closure closure;
 _Linknet network;

    _Domain domain;
 _ObjectPtr object;

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
    _Termination termination;
    _QueryContextPtr context;

    context = (_QueryContextPtr) closure;

    /*
    **  If the object is selected by the selection expression, invoke the
    **	callback routine.
    */

    if (_Selected(*object, context->expression))
	termination = (*context->routine)(context->closure, network,
	    domain, object);
    else
	termination = (_Termination) 0;

    return termination;
    }


static void  EncodeSurrogates(network, handle)
_Linknet network;
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
    **  Retrieve the Surrogates if we don't have them.
    */

    if (!(_Boolean) _SetState((_Persistent) network, _StateHaveAllSurrogates,
	    _StateSet))
	RetrieveSurrogates(network);

    if (_Surrogates_of(network) != (_Set) _NullObject) {
	_GetValue(_Surrogates_of(network), _P_ElementCount, lwk_c_domain_integer,
	    &count);

	if (count > 0) {
	    /*
	    **  Begin the Surrogates Encoding.
	    */

	    type = LWK_K_T_LNET_SURROGATES;
	    status = ddis_put(&handle, &type,(_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of Surrogates.
	    */

	    type = LWK_K_T_LNET_SURR_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Surrogates Set.
	    */

	    type = LWK_K_T_LNET_SURR_ELEMENTS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Surrogates Set encoding each one.
	    */

	    _Iterate(_Surrogates_of(network), lwk_c_domain_surrogate,
		(_Closure) handle, EncodeSurrogate);

	    /*
	    **  End the Surrogates Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Network Surrogates encoding.
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


static _Termination _EntryPt  EncodeSurrogate(closure, surrogates, domain, surrogate)
_Closure closure;

    _Set surrogates;
 _Domain domain;
 _SurrogatePtr surrogate;

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
    _VaryingString encoding;
    _DDIStype type;
    _DDISlength value_length;

    handle = (_DDIShandle) closure;

    /*
    **  Begin encoding the Surrogate.
    */
    
    type = DDIS_K_T_SEQUENCE;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Export the Surrogate
    */

    value_length = _Export(*surrogate, _False, &encoding);

    /*
    **  Encode the Surrogate.
    */
    
    type = LWK_K_T_LNET_SURR_PROPS;
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) encoding,
	(_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Surrogate ID (its address).
    */
    
    type = LWK_K_T_LNET_SURR_ID;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) surrogate,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  End the Surrogate encoding.
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


static void  EncodeLinks(network, handle)
_Linknet network;
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
    **  Retrieve the Links if we don't have them.
    */

    if (!(_Boolean) _SetState((_Persistent) network, _StateHaveAllLinks,
	    _StateSet))
	RetrieveLinks(network);

    if (_Links_of(network) != (_Set) _NullObject) {
	_GetValue(_Links_of(network), _P_ElementCount,
	    lwk_c_domain_integer, &count);

	if (count > 0) {
	    /*
	    **  Begin the Links Encoding.
	    */

	    type = LWK_K_T_LNET_LINKS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of Links.
	    */

	    type = LWK_K_T_LNET_LINK_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Links Set.
	    */

	    type = LWK_K_T_LNET_LINK_ELEMENTS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Links Set encoding each one.
	    */

	    _Iterate(_Links_of(network), lwk_c_domain_link,
		(_Closure) handle, EncodeLink);

	    /*
	    **  End the Links Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0,
		(_DDISdata) 0, (_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Network Links encoding.
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


static _Termination _EntryPt  EncodeLink(closure, links, domain, link)
_Closure closure;

    _Set links;
 _Domain domain;
 _LinkPtr link;

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
    _Surrogate surrogate;
    _VaryingString encoding;
    _DDIStype type;
    _DDISlength value_length;

    handle = (_DDIShandle) closure;

    /*
    **  Begin encoding the Link.
    */
    
    type = DDIS_K_T_SEQUENCE;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Export the Link
    */

    value_length = _Export(*link, _False, &encoding);

    /*
    **  Encode the Link.
    */
    
    type = LWK_K_T_LNET_LINK_PROPS;
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) encoding,
	(_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Source ID (its address).
    */
    
    _GetValue(*link, _P_Source, lwk_c_domain_surrogate, &surrogate);

    type = LWK_K_T_LNET_LINK_SOURCE_ID;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &surrogate,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Target ID (its address).
    */
    
    _GetValue(*link, _P_Target, lwk_c_domain_surrogate, &surrogate);

    type = LWK_K_T_LNET_LINK_TARGET_ID;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &surrogate,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  End the Link encoding.
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


static _HashTableEntryPtr  DecodeSurrogates(network, handle, keys_only)
_Linknet network;

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
    _Integer id;
    _Integer integer;
    _Integer count;
    _Surrogate surrogate;
    _VaryingString encoding;
    _HashTableEntryPtr volatile hash_table;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Initialize hash table (not needed if extracting Key Properties).
    */

    if (keys_only)
	hash_table = (_HashTableEntryPtr) 0;
    else {
	hash_table = (_HashTableEntryPtr)
	    _AllocateMem(_ImportHashSize * sizeof(_HashTableEntry));

	_HashInitialize(hash_table, _ImportHashSize);
    }

    _StartExceptionBlock

    /*
    **  Get Surrogate count
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LNET_SURR_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Surrogates
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LNET_SURR_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **	Decode count Surrogates, or just skip them if extracting Key
    **	Properties.
    */

    if (!keys_only)
	_Surrogates_of(network) =
	    (_Set) _CreateSet(_TypeSet, lwk_c_domain_surrogate, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_SEQUENCE)
	    _Raise(inv_encoding);

	/*
	**  Get the Surrogate's encoding and Import it into a Surrogate
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != LWK_K_T_LNET_SURR_PROPS)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    encoding = (_VaryingString) _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) encoding, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    surrogate = (_Surrogate) _Import(_TypeSurrogate, encoding);

	    _FreeMem(encoding);

	    /*
	    **  Add the Surrogate to the Network
	    */

	    _SetValue(surrogate, _P_Container, lwk_c_domain_linknet, &network,
		lwk_c_set_property);
	}

	/*
	**  Decode the Surrogate's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_LNET_SURR_ID)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &id);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **  Using the ID as key, put the Surrogate into the hash table
	    */

	    _HashInsert(hash_table, _ImportHashSize, id, surrogate);
	}

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_EOC)
	    _Raise(inv_encoding);
    }

    /*
    **  Finish decoding the Surrogates Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LNET_SURR_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Surrogates.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LNET_SURROGATES)
	_Raise(inv_encoding);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    FreeHashTable(hash_table);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the hash table (it is used to reconstruct the Links).
    */

    return hash_table;
    }


static void  DecodeLinks(network, handle, hash_table, keys_only)
_Linknet network;
 _DDIShandle handle;

    _HashTableEntryPtr hash_table;
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
    _Integer id;
    _Integer integer;
    _Integer count;
    _Surrogate surrogate;
    _Link link;
    _VaryingString encoding;
    _DDIStable table;
    _DDIStype type;
    _DDISlength value_length;
    _DDISentry entry;
    _DDISlength get_length;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Get Link count
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LNET_LINK_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Links
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_LNET_LINK_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Decode count Links, or just skip them if extracting Key
    **	Properties.
    */

    if (!keys_only)
	_Links_of(network) =
	    (_Set) _CreateSet(_TypeSet, lwk_c_domain_link, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_SEQUENCE)
	    _Raise(inv_encoding);

	/*
	**  Get the Link's encoding and Import it into a Link
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != LWK_K_T_LNET_LINK_PROPS)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    encoding = (_VaryingString) _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) encoding, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    link = (_Link) _Import(_TypeLink, encoding);

	    _FreeMem(encoding);
	}

	/*
	**  Decode the Source's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_LNET_LINK_SOURCE_ID)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &id);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **	Using the ID as key, find the Surrogate in the hash table, and
	    **	set it as the Source of the Link.
	    */

	    if (!_HashSearch(hash_table, _ImportHashSize, id, &surrogate))
		surrogate = (_Surrogate) _NullObject;

	    _SetValue(link, _P_Source, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);
	}

	/*
	**  Decode the Target's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_LNET_LINK_TARGET_ID)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &id);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **	Using the ID as key, find the Surrogate in the hash table, and
	    **	set it as the Target of the Link.
	    */

	    if (!_HashSearch(hash_table, _ImportHashSize, id, &surrogate))
		surrogate = (_Surrogate) _NullObject;

	    _SetValue(link, _P_Target, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);

	    /*
	    **  Add the Link to the Network
	    */

	    _SetValue(link, _P_Linknet, lwk_c_domain_linknet, &network,
		lwk_c_set_property);
	}

	/*
	**  Finish decoding this Link
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_EOC)
	    _Raise(inv_encoding);
    }

    /*
    **  Finish decoding the Links Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LNET_LINK_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Links.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_LNET_LINKS)
	_Raise(inv_encoding);

    return;
    }


static void  FreeHashTable(table)
_HashTableEntryPtr table;

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
    **  Clear all hash table entries, then free the hash table
    */

    if (table != (_HashTableEntryPtr) 0) {
	_HashFree(table, _ImportHashSize);
	_FreeMem(table);
    }

    return;
    }


static _Termination _EntryPt  PreStoreLink(repository, list, domain, link)
_Closure repository;

    _List list;
 _Domain domain;
 _LinkPtr link;

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
    **  Make sure that this Link has an Identifier assigned
    */

    LwkLbAssignIdentifier((_Linkbase) repository, *link);

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static _Termination _EntryPt  StoreSubObject(repository, list, domain, object)
_Closure repository;
 _List list;

    _Domain domain;
 _PersistentPtr object;

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
    **  If the Sub-Object has DeletePending, Drop it from the Repository and
    **	complete the Deletion.  Otherwise, Store the Sub-Object in the
    **	Repository.
    */

    if ((_Boolean) _SetState(*object, _StateDeletePending, _StateGet)) {
	_Persistent delete;

	delete = *object;
	LwkLbDrop(domain, delete);
	_Delete(&delete);
    }
    else
	LwkLbStore((_Linkbase) repository, domain, *object);

    return (_Termination) 0;
    }
