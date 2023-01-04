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
**	CompPath object methods 
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
#include "cpath.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_comppath.h"
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

_DeclareFunction(static _Termination EncodePathObjectDesc,
    (_DDIShandle handle, _Set paths, _Domain domain, _ObjectDesc *pathid));
_DeclareFunction(static _Termination QueryCompPath,
    (_QueryContext *context, _Object composite, _Domain domain,
	_Object *object));
_DeclareFunction(static _Termination FindObject,
    (_Persistent object, _Set set, _Domain domain, _Persistent *processed));

/*
**  External Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _CompPathTypeInstance;      /* Instance record for Type */
_Global _CompPathType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeCompPathInstance;
static _CompPathPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties =
    CompPathPropertyNameTable;


void  LwkOpCPath(cpath)
_CompPath cpath;

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


void  LwkOpCPathInitialize(cpath, proto, aggregate)
_CompPath cpath;
 _CompPath proto;

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

    _ClearValue(&_Name_of(cpath), lwk_c_domain_ddif_string);
    _ClearValue(&_Paths_of(cpath), lwk_c_domain_list);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(cpath, proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_CompPath) _NullObject) {
	_CopyValue(&_Name_of(proto), &_Name_of(cpath),
	    lwk_c_domain_ddif_string);

	_CopyValue(&_Paths_of(proto), &_Paths_of(cpath), lwk_c_domain_list);
    }

    return;
    }


void  LwkOpCPathFree(cpath)
_CompPath cpath;

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

    _DeleteValue(&_Name_of(cpath), lwk_c_domain_ddif_string);
    _DeleteValue(&_Paths_of(cpath), lwk_c_domain_list);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(cpath, MyType);

    return;
    }


_Set  LwkOpCPathEnumProps(cpath)
_CompPath cpath;

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

    set = (_Set) _EnumerateProperties_S(cpath, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpCPathIsMultiValued(cpath, property)
_CompPath cpath;
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
	answer = _IsMultiValued_S(cpath, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		answer = _False;
		break;

	    case _PathsIndex :
		answer = _True;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpCPathGetValueDomain(cpath, property)
_CompPath cpath;
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
	domain = (_Domain) _GetValueDomain_S(cpath, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _PathsIndex :
		domain = lwk_c_domain_list;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpCPathGetValue(cpath, property, domain, value)
_CompPath cpath;
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
	_GetValue_S(cpath, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Name_of(cpath), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);
		break;

	    case _PathsIndex :
		if (_IsDomain(domain, lwk_c_domain_list))
		    _CopyValue(&_Paths_of(cpath), value, lwk_c_domain_set);
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


_List  LwkOpCPathGetValueList(cpath, property)
_CompPath cpath;
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
	return (_List) _GetValueList_S(cpath, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_ListValue(&_Name_of(cpath), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _PathsIndex :
		_CopyValue(&_Paths_of(cpath), &value_list,
		    lwk_c_domain_set);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpCPathSetValue(cpath, property, domain, value, flag)
_CompPath cpath;
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
	_SetValue_S(cpath, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(cpath),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _PathsIndex :
		_SetMultiValuedProperty(&_Paths_of(cpath),
		    lwk_c_domain_object_desc, domain, value, flag,
		    _False, _False);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Composite Path modified
    */

    _SetState(cpath, _StateModified, _StateSet);

    return;
    }


void  LwkOpCPathSetValueList(cpath, property, values, flag)
_CompPath cpath;
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
	_SetValueList_S(cpath, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(cpath),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _PathsIndex :
		_SetMultiValuedProperty(&_Paths_of(cpath),
		    lwk_c_domain_object_desc, lwk_c_domain_list, &values,
		    flag, _True, _False);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Composite Path modified
    */

    _SetState(cpath, _StateModified, _StateSet);

    return;
    }


void  LwkOpCPathEncode(cpath, aggregate, handle)
_CompPath cpath;
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
    **  Begin the encoding of Composite Path.
    */

    type = LWK_K_T_PERS_COMP_PATH;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Name property
    */

    if (_Name_of(cpath) != (_DDIFString) _NullObject) {
	type = LWK_K_T_CPATH_NAME;
	value_length = _LengthDDIFString(_Name_of(cpath));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _Name_of(cpath), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **  Encode the Paths property.
    */

    if (_Paths_of(cpath) != (_Set) _NullObject) {
	_GetValue(_Paths_of(cpath), _P_ElementCount, lwk_c_domain_integer,
	    &count);

	if (count > 0) { 
	    /*
	    **  Begin the encoding of Path ObjectDesc's.
	    */

	    type = LWK_K_T_CPATH_OBJECT_DESCS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    /*
	    **  Encode the count of the Path ObjectDesc's Set.
	    */

	    type = LWK_K_T_CPATH_DESC_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Path ObjectDesc's Set.
	    */

	    type = LWK_K_T_CPATH_DESCS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Path ObjectDesc's Set encoding each Path
	    **  ObjectDesc.
	    */

	    _Iterate(_Paths_of(cpath), lwk_c_domain_object_desc,
		(_Closure) handle, EncodePathObjectDesc);

	    /*
	    **  Finish encoding the Path ObjectDesc's Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Finish encoding the Path ObjectDesc's.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    /*
    **  Finish the Composite Path encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(cpath, aggregate, handle, MyType);

    return;
    }


void  LwkOpCPathDecode(cpath, handle, keys_only, properties)
_CompPath cpath;
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
    _DDIStype type;
    _DDISlength value_length,
		get_length;
    _DDISentry entry;
    _DDIStable table;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Begin the Composite Path decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_COMP_PATH)
	_Raise(inv_encoding);

    /*
    **	Decode the Composite Path Properties.  If extracting Key Properties,
    **	all properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_COMP_PATH:
		/*
		**  End of Composite Path Properties.
		*/
		break;

	    case LWK_K_P_CPATH_NAME :
		if (!keys_only) {
		    _Name_of(cpath) = _AllocateMem(value_length);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) _Name_of(cpath),
			&get_length, &additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_CPATH_OBJECT_DESCS:
		/*
		**  Begin decoding of Path ObjectDesc's Set.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_P_CPATH_DESC_COUNT)
		    _Raise(inv_encoding);

		/*
		**  Decode the count for the Path ObjectDesc's Set.
		*/

		value_length = sizeof(_Integer);

		status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		    (_DDISdata) &integer, &get_length, &cnt);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		count = (_Integer) cnt;

		/*
		**  Decode the Set of Path ObjectDesc's.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_P_CPATH_DESCS)
		    _Raise(inv_encoding);

		if (!keys_only)
		    _Paths_of(cpath) = (_Set) _CreateSet(_TypeSet,
			lwk_c_domain_object_desc, count);

                for (i = 0; i < count; i++) {
		    _AnyPtr volatile encoding;
		    _ObjectDesc volatile odesc;

		    status = ddis_get_tag(&handle, &type, &value_length, &entry,
			&table);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (type != DDIS_K_T_OCTET_STRING)
			_Raise(inv_encoding);

		    if (!keys_only) {
			encoding = (_AnyPtr) _NullObject;
			odesc = (_ObjectDesc) _NullObject;

			_StartExceptionBlock

			encoding = _AllocateMem(value_length);

			status = ddis_get_value(&handle,
			    (_DDISsizePtr) &value_length, (_DDISdata) encoding,
			     &get_length, &additional_info);

			if (!_Success(status))
			    _Raise(internal_decoding_error);

			if (value_length != get_length)
			    _Raise(internal_decoding_error);

			/*
			** Import the Path ObjectDesc.
			*/
		    
			odesc = (_ObjectDesc) _Import(_TypeObjectDesc,
			    encoding);

			/*
			**  Add it to the List of Paths.
			*/

			_AddElement(_Paths_of(cpath), lwk_c_domain_object_desc,
			    (_AnyPtr) &odesc, _False);

			_DeleteEncoding(&encoding);

			_Exceptions
			    _WhenOthers
				_DeleteEncoding(&encoding);
				_Delete(&odesc);
				_Reraise;
			_EndExceptionBlock
		    }
		}

		/*
		**  Finish decoding the Set of Path ObjectDesc's.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_E_CPATH_DESCS)
		    _Raise(inv_encoding);

		/*
		**  Finish decoding the Path ObjectDesc's.
		*/

		status = ddis_get_tag(&handle, &type, &value_length, &entry,
		    &table);

		if (!_Success(status))
		    _Raise(internal_decoding_error);

		if (entry != LWK_K_E_CPATH_OBJECT_DESCS)
		    _Raise(inv_encoding);

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_COMP_PATH);

    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(cpath, handle, keys_only, properties, MyType);

    return;
    }


_Termination  LwkOpCPathIterate(cpath, domain, closure, routine)
_CompPath cpath;
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
    _QueryContext context;
    _Termination termination;

    /*
    **  Iterate based on the domain
    */

    switch (domain) {
	case lwk_c_domain_object_desc :
	case lwk_c_domain_path :
	case lwk_c_domain_step :
	case lwk_c_domain_surrogate :
	    if (_Paths_of(cpath) == (_List) _NullObject)
		termination = (_Termination) 0;
	    else {
		if (routine == (_Callback) _NullObject)
		    _Raise(inv_argument);

		context.routine = routine;
		context.closure = closure;
		context.domain = domain;
		context.expression = (_QueryExpression) _NullObject;

		context.composites_processed = (_Object)
		    _CreateSet(_TypeSet, lwk_c_domain_comp_path,
			(_Integer) 0);

		_StartExceptionBlock

		termination = _Iterate(_Paths_of(cpath),
		    lwk_c_domain_object_desc, (_Closure) &context,
		    QueryCompPath);

		_Delete(&context.composites_processed);

		_Exceptions
		    _WhenOthers
			_Delete(&context.composites_processed);
			_Reraise;
		_EndExceptionBlock
	    }

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


_Termination  LwkOpCPathQuery(cpath, domain, expression, closure, routine)
_CompPath cpath;
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

    switch (domain) {
	case lwk_c_domain_step :
	case lwk_c_domain_surrogate :
	    if (_Paths_of(cpath) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else {
		context.routine = routine;
		context.closure = closure;
		context.domain  = domain;
		context.expression = expression;

		termination = _Iterate(_Paths_of(cpath),
		    lwk_c_domain_object_desc, (_Closure) &context,
		    QueryCompPath);
	    }

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


void  LwkOpCPathStore(cpath, repository)
_CompPath cpath;
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
    **  Store the CompPath in the Repository
    */

    LwkLbStore(repository, lwk_c_domain_comp_path, cpath);

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


void  LwkOpCPathDrop(cpath)
_CompPath cpath;

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
    **	Find the Repository in which the Composite Path is stored.  If there
    **	isn't one, return now.
    */

    _GetValue(cpath, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    if (repository == (_Linkbase) _NullObject)
	return;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **  Drop the CompPath from the Repository where it is stored.
    */

    LwkLbDrop(lwk_c_domain_comp_path, cpath);

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


static _Termination  EncodePathObjectDesc(handle, paths, domain, pathid)
_DDIShandle handle;
 _Set paths;

    _Domain domain;
 _ObjectDesc *pathid;

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
    _DDISlength size;
    _VaryingString encoding;
    _DDIStype type;
    
    /*
    **  Export the Path ObjectDesc.
    */

    size = _Export(*pathid, _False, &encoding);

    /*
    **  Encode the Path ObjectDesc.
    */

    type = DDIS_K_T_OCTET_STRING;
    status = ddis_put(&handle, &type, &size, (_DDISdata) encoding,
	(_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Continue the iteration.
    */

    return (_Termination) 0;
    }


static _Termination  QueryCompPath(context, composite, domain, object)
_QueryContext *context;

    _Object composite;
 _Domain domain;
 _Object *object;

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
    _Termination found;
    _Termination termination;
    _Persistent persistent;

    /*
    **	If we have the object of the original iteration request, invoke the
    **	callback routine.  Otherwise, re-iterate/query.
    */

    if (context->domain == domain) {
	/*
	**  If there was a selection expression, make sure that it selects the
	**  object before invoking the callback routine.
	*/

	if (context->expression == (_QueryExpression) _NullObject)
	    termination = (*context->routine)(context->closure, composite,
		domain, object);
	else {
	    if (_Selected(*object, context->expression))
		termination = (*context->routine)(context->closure,
		    composite, domain, object);
	    else
		termination = (_Termination) 0;
	}
    }
    else {
        /*
	** If the domain is Object Descriptor, retrieve the object.
        */

	if (domain != lwk_c_domain_object_desc)
	    persistent = *object;
	else {
	    persistent = (_Persistent) _Retrieve((_ObjectDesc) *object);
	    domain = _TypeToDomain(_Type_of(persistent));
	}

	if (context->domain == domain) {
	    /*
	    **	We now have the object domain request.  If there was a
	    **	selection expression, make sure that it selects the object
	    **	before invoking the callback routine.
	    */

	    if (context->expression == (_QueryExpression) _NullObject)
		termination = (*context->routine)(context->closure, composite,
		    domain, &persistent);
	    else {
		if (_Selected(persistent, context->expression))
		    termination = (*context->routine)(context->closure,
			composite, domain, &persistent);
		else
		    termination = (_Termination) 0;
	    }
	}
	else {
            /*
	    ** Check for redundancies and cycles
            */

	    switch (domain) {
		case lwk_c_domain_path :
		    found = (_Termination) 0;
		    break;

		case lwk_c_domain_comp_path :
		    /*
		    ** Only process this Composite Path if we haven't seen it
		    ** already (otherwise we loop!).
		    */
		    
		    found = _Iterate(context->composites_processed,
			lwk_c_domain_comp_path, (_Closure) persistent,
			FindObject);

		    if (found == (_Termination) 0)
			_AddElement(context->composites_processed,
			    lwk_c_domain_comp_path, &persistent, _True);

		    break;

		default :
		    _Raise(inv_domain);
		    break;
	    }

            /*
            ** If not redundant or a cycle, re-iterate/query.
            */
	    
	    if (found == (_Termination) 0) {
		if (context->expression == (_QueryExpression) _NullObject)
		    termination = _Iterate(persistent, context->domain,
			(_Closure) context, QueryCompPath);
		else
		    termination = _Query(persistent, context->domain,
			context->expression, (_Closure) context,
			QueryCompPath);
	    }
	}
    }

    return termination;
    }


static _Termination  FindObject(object, set, domain, processed)
_Persistent object;
 _Set set;
 _Domain domain;

    _Persistent *processed;

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
    if (object == *processed)
	return (_Termination) 1;
    else
	return (_Termination) 0;
    }
