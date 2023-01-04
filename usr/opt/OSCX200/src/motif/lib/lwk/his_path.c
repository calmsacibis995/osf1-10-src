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
**	Path object methods
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
#include "path.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_path.h"
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

_DeclareFunction(static void CopyAggregate, (_Path path, _Path proto));
_DeclareFunction(static _Termination CopySurrogate,
    (_Aggregate aggregate, _List list, _Domain domain, _Surrogate *surrogate));
_DeclareFunction(static _Termination CopyStep,
    (_Aggregate aggregate, _List list, _Domain domain, _Step *step));
_DeclareFunction(static _Termination DropSubObject,
    (_Closure null, _List list, _Domain domain, _Persistent *object));
_DeclareFunction(static void RetrieveSurrogates, (_Path path));
_DeclareFunction(static void RetrieveSteps, (_Path path));
_DeclareFunction(static void QuerySurrogates,
    (_Path path, _QueryExpression expression));
_DeclareFunction(static void QuerySteps,
    (_Path path, _QueryExpression expression));
_DeclareFunction(static _Termination DeleteSubObject,
    (_Closure null, _List list, _Domain domain, _Persistent *object));
_DeclareFunction(static _Termination QueryPath,
    (_QueryContext *context, _Path path, _Domain domain, _Object *object));
_DeclareFunction(static void EncodeSurrogates,
    (_Path path, _DDIShandle handle));
_DeclareFunction(static _Termination EncodeSurrogate,
    (_DDIShandle handle, _Set surrogates, _Domain domain,
	_Surrogate *surrogate));
_DeclareFunction(static void EncodeSteps,
    (_Path path, _DDIShandle handle));
_DeclareFunction(static _Termination EncodeStep,
    (_DDIShandle handle, _Set steps, _Domain domain, _Step *step));
_DeclareFunction(static _HashTableEntry *DecodeSurrogates,
    (_Path path, _DDIShandle handle, _Boolean keys_only));
_DeclareFunction(static void DecodeSteps,
    (_Path path, _DDIShandle handle, _HashTableEntry *hash_table,
	_Boolean keys_only));
_DeclareFunction(static void SetOrigin, (_Path path));
_DeclareFunction(static void SetDestination, (_Path path));
_DeclareFunction(static void SequenceSteps,
    (_Path path, _SetFlag flag, _Object steps));
_DeclareFunction(static void ClearSteps, (_Path path));
_DeclareFunction(static void SetSteps, (_Path path));
_DeclareFunction(static _Termination LinkStep,
    (_Step *previous, _List list, _Domain domain, _Step *step));
_DeclareFunction(static void AddSteps, (_Path path, _Object steps));
_DeclareFunction(static _Termination AddStep,
    (_Path path, _List list, _Domain domain, _Step *step));
_DeclareFunction(static void RemoveSteps, (_Path path, _Object steps));
_DeclareFunction(static _Termination RemoveStep,
    (_Path path, _List list, _Domain domain, _Step *step));
_DeclareFunction(static _Boolean IsValidPathStep, (_Path path, _Step step));
_DeclareFunction(static void FreeHashTable, (_HashTableEntry *table));
_DeclareFunction(static _Termination PreStoreStep,
    (_Linkbase repository, _List list, _Domain domain, _Step *step));
_DeclareFunction(static _Termination StoreStep,
    (_Linkbase repository, _List list, _Domain domain, _Step *step));
_DeclareFunction(static _Termination StoreSurrogate,
    (_Linkbase repository, _List list, _Domain domain,
	_Surrogate *surrogate));

/*
**  External Routine Declarations
*/

/*
**  Global Data Definitions
*/

_Global _PathTypeInstance;      /* Instance record for Type */
_Global _PathType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypePathInstance;
static _PathPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties = PathPropertyNameTable;


void  LwkOpPath(path)
_Path path;

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


void  LwkOpPathInitialize(path, proto, aggregate)
_Path path;
 _Path proto;
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

    _ClearValue(&_Name_of(path), lwk_c_domain_ddif_string);
    _ClearValue(&_OriginOperation_of(path), lwk_c_domain_string);
    _ClearValue(&_FirstStep_of(path), lwk_c_domain_step);
    _ClearValue(&_LastStep_of(path), lwk_c_domain_step);
    _ClearValue(&_FollowedStep_of(path), lwk_c_domain_step);
    _ClearValue(&_Origin_of(path), lwk_c_domain_surrogate);
    _ClearValue(&_Destination_of(path), lwk_c_domain_surrogate);
    _ClearValue(&_Steps_of(path), lwk_c_domain_list);
    _ClearValue(&_Surrogates_of(path), lwk_c_domain_set);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(path, proto, aggregate, MyType);
    
    /*
    **  If a prototype was provided, copy properties from it.
    */

    if (proto != (_Path) _NullObject) {
	_CopyValue(&_Name_of(proto), &_Name_of(path), lwk_c_domain_ddif_string);
	_CopyValue(&_OriginOperation_of(proto), &_OriginOperation_of(path),
	    lwk_c_domain_string);

	if (aggregate)
	    CopyAggregate(proto, path);
    }

    return;
    }


void  LwkOpPathFree(path)
_Path path;

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
    **  Mark the Path DeletePending.  We notice this state in the RemoveSteps
    **	procedure and avoid lots of unnecessary clean-up overhead.
    */

    _SetState(path, _StateDeletePending, _StateSet);

    /*
    **	Delete any Steps
    */

    if (_Steps_of(path) != (_Set) _NullObject)
	_Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) 0,
	    DeleteSubObject);

    /*
    **  Now, delete any Surrogates.
    */

    if (_Surrogates_of(path) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(path), lwk_c_domain_surrogate, (_Closure) 0,
	    DeleteSubObject);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Name_of(path), lwk_c_domain_ddif_string);
    _DeleteValue(&_OriginOperation_of(path), lwk_c_domain_string);
    _DeleteValue(&_FirstStep_of(path), lwk_c_domain_step);
    _DeleteValue(&_LastStep_of(path), lwk_c_domain_step);
    _DeleteValue(&_FollowedStep_of(path), lwk_c_domain_step);
    _DeleteValue(&_Origin_of(path), lwk_c_domain_surrogate);
    _DeleteValue(&_Destination_of(path), lwk_c_domain_surrogate);
    _DeleteValue(&_Steps_of(path), lwk_c_domain_list);
    _DeleteValue(&_Surrogates_of(path), lwk_c_domain_set);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(path, MyType);

    return;
    }


_Set  LwkOpPathEnumProps(path)
_Path path;

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

    set = (_Set) _EnumerateProperties_S(path, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpPathIsMultiValued(path, property)
_Path path;
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
	answer = _IsMultiValued_S(path, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
	    case _OriginOperationIndex :
	    case _FirstStepIndex :
	    case _LastStepIndex :
	    case _FollowedStepIndex :
	    case _OriginIndex :
	    case _DestinationIndex :
		answer = _False;
		break;

	    case _StepsIndex :
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


_Domain  LwkOpPathGetValueDomain(path, property)
_Path path;
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
	domain = (_Domain) _GetValueDomain_S(path, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		domain = lwk_c_domain_ddif_string;
		break;

	    case _OriginOperationIndex :
		domain = lwk_c_domain_string;
		break;

	    case _FirstStepIndex :
	    case _LastStepIndex :
	    case _FollowedStepIndex :
		domain = lwk_c_domain_step;
		break;

	    case _OriginIndex :
	    case _DestinationIndex :
		domain = lwk_c_domain_surrogate;
		break;

	    case _StepsIndex :
		domain = lwk_c_domain_list;
		break;

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


void  LwkOpPathGetValue(path, property, domain, value)
_Path path;
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
	_GetValue_S(path, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		if (_IsDomain(domain, lwk_c_domain_ddif_string))
		    _CopyValue(&_Name_of(path), value,
			lwk_c_domain_ddif_string);
		else
		    _Raise(inv_domain);
		break;

	    case _OriginOperationIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_OriginOperation_of(path), value,
			lwk_c_domain_string);
		else
		    _Raise(inv_domain);
		break;

	    case _FollowedStepIndex :
		if (_IsDomain(domain, lwk_c_domain_step))
		    _CopyValue(&_FollowedStep_of(path), value,
			lwk_c_domain_step);
		else
		    _Raise(inv_domain);

		break;

	    case _FirstStepIndex :
		if (_IsDomain(domain, lwk_c_domain_step))
		    _CopyValue(&_FirstStep_of(path), value, lwk_c_domain_step);
		else
		    _Raise(inv_domain);

		break;

	    case _LastStepIndex :
		if (_IsDomain(domain, lwk_c_domain_step))
		    _CopyValue(&_LastStep_of(path), value, lwk_c_domain_step);
		else
		    _Raise(inv_domain);

		break;

	    case _OriginIndex :
		if (_IsDomain(domain, lwk_c_domain_surrogate))
		    _CopyValue(&_Origin_of(path), value,
			lwk_c_domain_surrogate);
		else
		    _Raise(inv_domain);

		break;

	    case _DestinationIndex :
		if (_IsDomain(domain, lwk_c_domain_surrogate))
		    _CopyValue(&_Destination_of(path), value,
			lwk_c_domain_surrogate);
		else
		    _Raise(inv_domain);

		break;

	    case _StepsIndex :
		/*
		**  Retrieve the Steps if we don't have them.
		*/

		if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateSet))
		    RetrieveSteps(path);

		if (_IsDomain(domain, lwk_c_domain_list))
		    _CopyValue(&_Steps_of(path), value, lwk_c_domain_list);
		else
		    _Raise(inv_domain);

		break;

	    case _SurrogatesIndex :
		/*
		**  Retrieve the Surrogates if we don't have them.
		*/

		if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates,
			_StateSet))
		    RetrieveSurrogates(path);

		if (_IsDomain(domain, lwk_c_domain_set))
		    _CopyValue(&_Surrogates_of(path), value, lwk_c_domain_set);
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


_List  LwkOpPathGetValueList(path, property)
_Path path;
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
	return (_List) _GetValueList_S(path, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_ListValue(&_Name_of(path), &value_list,
		    lwk_c_domain_ddif_string);
		break;

	    case _OriginOperationIndex :
		_ListValue(&_OriginOperation_of(path), &value_list,
		    lwk_c_domain_string);
		break;

	    case _FollowedStepIndex :
		_ListValue(&_FollowedStep_of(path), &value_list,
		    lwk_c_domain_step);
		break;

	    case _FirstStepIndex :
		_ListValue(&_FirstStep_of(path), &value_list,
		    lwk_c_domain_step);
		break;

	    case _LastStepIndex :
		_ListValue(&_LastStep_of(path), &value_list,
		    lwk_c_domain_step);
		break;

	    case _OriginIndex :
		_ListValue(&_Origin_of(path), &value_list,
		    lwk_c_domain_surrogate);
		break;

	    case _DestinationIndex :
		_ListValue(&_Destination_of(path), &value_list,
		    lwk_c_domain_surrogate);
		break;

	    case _StepsIndex :
		/*
		**  Retrieve the Steps if we don't have them.
		*/

		if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateSet))
		    RetrieveSteps(path);

		_CopyValue(&_Steps_of(path), &value_list,
		    lwk_c_domain_list);
		break;

	    case _SurrogatesIndex :
		/*
		**  Retrieve the Surrogates if we don't have them.
		*/

		if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates,
			_StateSet))
		    RetrieveSurrogates(path);

		_CopyValue(&_Surrogates_of(path), &value_list,
		    lwk_c_domain_set);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpPathSetValue(path, property, domain, value, flag)
_Path path;
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
    _Step old;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(path, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(path),
		    lwk_c_domain_ddif_string, domain, value, flag, _False);
		break;

	    case _OriginOperationIndex :
		_SetSingleValuedProperty(&_OriginOperation_of(path),
		    lwk_c_domain_string, domain, value, flag, _False);
		break;

	    case _OriginIndex :
	    case _DestinationIndex :
		_Raise(property_is_readonly);
		break;

	    case _FirstStepIndex :
		old = _FirstStep_of(path);

		_SetSingleValuedProperty(&_FirstStep_of(path),
		    lwk_c_domain_step, domain, value, flag, _False);

		if (!IsValidPathStep(path, _FirstStep_of(path))) {
		    _FirstStep_of(path) = old;
		    _Raise(inv_argument);
		}

		SetOrigin(path);

		break;

	    case _LastStepIndex :
		old = _LastStep_of(path);

		_SetSingleValuedProperty(&_LastStep_of(path), lwk_c_domain_step,
		    domain, value, flag, _False);

		if (!IsValidPathStep(path, _LastStep_of(path))) {
		    _LastStep_of(path) = old;
		    _Raise(inv_argument);
		}

		SetDestination(path);

		break;

	    case _FollowedStepIndex :
		old = _FollowedStep_of(path);

		_SetSingleValuedProperty(&_FollowedStep_of(path),
		    lwk_c_domain_step, domain, value, flag, _False);

		if (!IsValidPathStep(path, _FollowedStep_of(path))) {
		    _FollowedStep_of(path) = old;
		    _Raise(inv_argument);
		}

		break;

	    case _StepsIndex :
		_SetMultiValuedProperty(&_Steps_of(path), lwk_c_domain_step,
		    domain, value, flag, _False, _False);

		SequenceSteps(path, flag, *((_Object *) value));

		break;

	    case _SurrogatesIndex :
		_SetMultiValuedProperty(&_Surrogates_of(path),
		    lwk_c_domain_surrogate, domain, value, flag, _False, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Path modified
    */

    _SetState(path, _StateModified, _StateSet);

    return;
    }


void  LwkOpPathSetValueList(path, property, values, flag)
_Path path;
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
    _Step old;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(path, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _NameIndex :
		_SetSingleValuedProperty(&_Name_of(path),
		    lwk_c_domain_ddif_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _OriginOperationIndex :
		_SetSingleValuedProperty(&_OriginOperation_of(path),
		    lwk_c_domain_string, lwk_c_domain_list, &values,
		    flag, _True);
		break;

	    case _OriginIndex :
	    case _DestinationIndex :
		_Raise(property_is_readonly);
		break;

	    case _FirstStepIndex :
		old = _FirstStep_of(path);

		_SetSingleValuedProperty(&_FirstStep_of(path),
		    lwk_c_domain_step, lwk_c_domain_list, values, flag, _True);

		if (!IsValidPathStep(path, _FirstStep_of(path))) {
		    _FirstStep_of(path) = old;
		    _Raise(inv_argument);
		}

		SetOrigin(path);

		break;

	    case _LastStepIndex :
		old = _LastStep_of(path);

		_SetSingleValuedProperty(&_LastStep_of(path), lwk_c_domain_step,
		    lwk_c_domain_list, values, flag, _True);

		if (!IsValidPathStep(path, _LastStep_of(path))) {
		    _LastStep_of(path) = old;
		    _Raise(inv_argument);
		}

		SetDestination(path);

		break;

	    case _FollowedStepIndex :
		old = _FollowedStep_of(path);

		_SetSingleValuedProperty(&_FollowedStep_of(path),
		    lwk_c_domain_step, lwk_c_domain_list, values, flag, _True);

		if (!IsValidPathStep(path, _FollowedStep_of(path))) {
		    _FollowedStep_of(path) = old;
		    _Raise(inv_argument);
		}

		break;

	    case _StepsIndex :
		_SetMultiValuedProperty(&_Steps_of(path), lwk_c_domain_step,
		    lwk_c_domain_list, &values, flag, _True, _False);

		SequenceSteps(path, flag, *((_Object *) values));

		break;

	    case _SurrogatesIndex :
		_SetMultiValuedProperty(&_Surrogates_of(path),
                    lwk_c_domain_surrogate, lwk_c_domain_list, &values,
		    flag, _True, _True);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Path modified
    */

    _SetState(path, _StateModified, _StateSet);

    return;
    }


void  LwkOpPathEncode(path, aggregate, handle)
_Path path;
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
    **  Begin the encoding of Path.
    */

    type = LWK_K_T_PERS_PATH;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the properties of Path.
    */

    if (_Name_of(path) != (_DDIFString) _NullObject) {
	type = LWK_K_T_PATH_NAME;
	value_length = _LengthDDIFString(_Name_of(path));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _Name_of(path), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **	If encoding the aggregate Path, encode the Surrogates and
    **	Steps as well.
    */

    if (aggregate) {
	EncodeSurrogates(path, handle);
	EncodeSteps(path, handle);
    }

    /*
    **  Finish the Path encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);


    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(path, aggregate, handle, MyType);

    return;
    }


void  LwkOpPathDecode(path, handle, keys_only, properties)
_Path path;
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
    _DDIStype type;
    _DDISlength value_length,
		get_length;
    _DDISentry entry;
    _DDIStable table;
    _DDISconstant additional_info;

    /*
    **  Begin the Path decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_PATH)
	_Raise(inv_encoding);

    /*
    **  Initialize
    */

    hash_table = (_HashTableEntry *) 0;

    _StartExceptionBlock

    /*
    **	Decode the Path Properties.  If extracting Key Properties, all
    **	properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_PATH:
		/*
		**  End of Path Properties.
		*/
		break;

	    case LWK_K_P_PATH_NAME :
		if (!keys_only) {
		    _Name_of(path) = _AllocateMem(value_length);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) _Name_of(path), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_PATH_SURROGATES :
		/*
		**  Decode the Surrogates
		*/

		hash_table = DecodeSurrogates(path, handle, keys_only);

		break;

	    case LWK_K_P_PATH_STEPS :
		/*
		**  Decode the Steps
		*/

		DecodeSteps(path, handle, hash_table, keys_only);

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_PATH);

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _FreeMem(hash_table);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Free the hash table
    */

    FreeHashTable(hash_table);

    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(path, handle, keys_only, properties, MyType);

    return;
    }


_Termination  LwkOpPathIterate(path, domain, closure, routine)
_Path path;
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

	    if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates,
		    _StateSet))
		RetrieveSurrogates(path);

	    if (_Surrogates_of(path) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Surrogates_of(path),
		    lwk_c_domain_surrogate, closure, routine);
	    break;

	case lwk_c_domain_step :
	    /*
	    **  Retrieve the Steps if we don't have them.
	    */

	    if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateSet))
		RetrieveSteps(path);

	    if (_Steps_of(path) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Steps_of(path), lwk_c_domain_step,
		    closure, routine);
	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


_Termination  LwkOpPathQuery(path, domain, expression, closure, routine)
_Path path;
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
	case lwk_c_domain_step :
	    /*
	    **  Pre-fetch the appropriate Steps if we don't have them all.
	    */

	    if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateGet))
		QuerySteps(path, expression);

	    if (_Steps_of(path) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Steps_of(path),
		    lwk_c_domain_step, (_Closure) &context, QueryPath);

	    break;

	case lwk_c_domain_surrogate :
	    /*
	    **	Pre-fetch the appropriate Surrogates if we don't have them all.
	    */

	    if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates,
		    _StateGet))
		QuerySurrogates(path, expression);

	    if (_Surrogates_of(path) == (_Set) _NullObject)
		termination = (_Termination) 0;
	    else
		termination = _Iterate(_Surrogates_of(path),
		    lwk_c_domain_surrogate, (_Closure) &context, QueryPath);

	    break;

	default :
	    _Raise(inv_domain);
	    break;
    }

    return termination;
    }


void  LwkOpPathStore(path, repository)
_Path path;
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
    **	Before storing anything, Iterate over the Steps and do two things: 1)
    **	drop any that have been deleted (we need to do this because the Next
    **	and Previous of other Steps may be effected by the delete), 2) Make
    **	sure each Step has an assigned Identifier (we need this so that the
    **	Id's are available during the Store of the Path (First/Last/Current
    **	Step Id'd) and Steps (Previous/Next Step Id's)).
    */

    if (_Steps_of(path) != (_Set) _NullObject)
	_Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) repository,
	    PreStoreStep);

    /*
    **  Now, Store the Path in the Repository
    */

    LwkLbStore(repository, lwk_c_domain_path, path);

    /*
    **	Then, Iterate over the Surrogates -- drop any which were deleted, store
    **	the rest.
    */

    if (_Surrogates_of(path) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(path), lwk_c_domain_surrogate,
	    (_Closure) repository, StoreSurrogate);

    /*
    **  Finally, Iterate over the Steps -- store them
    */

    if (_Steps_of(path) != (_Set) _NullObject)
	_Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) repository,
	    StoreStep);

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


void  LwkOpPathDrop(path)
_Path path;

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
    **	Find the Repository in which the Path is stored.  If there isn't one,
    **	return now.
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    if (repository == (_Linkbase) _NullObject)
	return;

    /*
    **  Make sure a read/write transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read_write);

    _StartExceptionBlock

    /*
    **  Retrieve the Steps if we don't have them.
    */

    if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateSet))
	RetrieveSteps(path);

    /*
    **  Iterate over the Steps -- drop each one.
    */

    if (_Steps_of(path) != (_List) _NullObject)
	_Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) 0,
	    DropSubObject);

    /*
    **  Retrieve the Surrogates if we don't have them.
    */

    if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates, _StateSet))
	RetrieveSurrogates(path);

    /*
    **  Iterate over the Surrogates -- drop them as well
    */

    if (_Surrogates_of(path) != (_Set) _NullObject)
	_Iterate(_Surrogates_of(path), lwk_c_domain_surrogate, (_Closure) 0,
	    DropSubObject);

    /*
    **  Drop the Path from the Repository in which it is stored.
    */

    LwkLbDrop(lwk_c_domain_path, path);

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


static void  CopyAggregate(path, new)
_Path path;
 _Path new;

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

    aggregate.hash_table = (_HashTableEntry *)
	_AllocateMem(_ImportHashSize * sizeof(_HashTableEntry));

    _HashInitialize(aggregate.hash_table, _ImportHashSize);

    _StartExceptionBlock

    /*
    ** Copy all of the Surrogates
    */

    _Iterate(path, lwk_c_domain_surrogate, (_Closure) &aggregate,
	CopySurrogate);

    /*
    ** Copy all the Steps and set their Origins/Destinations
    */

    _Iterate(path, lwk_c_domain_step, (_Closure) &aggregate, CopyStep);

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


static _Termination  CopySurrogate(aggregate, list, domain, surrogate)
_Aggregate aggregate;
 _List list;

    _Domain domain;
 _Surrogate *surrogate;

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

    /*
    ** Copy the Surrogate and put it into the new Path
    */

    new = (_Surrogate) _Copy(*surrogate, _False);

    _SetValue(new, _P_Container, lwk_c_domain_path, &aggregate->aggregate,
	lwk_c_set_property);

    /*
    ** Add the Surrogate to the hash table using the old Surrogate as a key.
    */

    _HashInsert(aggregate->hash_table, _ImportHashSize, *surrogate, new);

    return (_Termination) 0;
    }


static _Termination  CopyStep(aggregate, list, domain, step)
_Aggregate aggregate;
 _List list;

    _Domain domain;
 _Step *step;

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
    _Step new;
    _Surrogate surrogate;
    _Surrogate new_surrogate;

    /*
    ** Copy the Step
    */

    new = (_Step) _Copy(*step, _False);

    /*
    ** Find its new Origin and Destination in the hash table and set them
    */
    
    _GetValue(*step, _P_Origin, lwk_c_domain_surrogate, &surrogate);

    if (!_HashSearch(aggregate->hash_table, _ImportHashSize, surrogate,
	    &new_surrogate))
	new_surrogate = (_Surrogate) _NullObject;

    _SetValue(new, _P_Origin, lwk_c_domain_surrogate, &new_surrogate,
	lwk_c_set_property);

    _GetValue(*step, _P_Destination, lwk_c_domain_surrogate, &surrogate);

    if (!_HashSearch(aggregate->hash_table, _ImportHashSize, surrogate,
	    &new_surrogate))
	new_surrogate = (_Surrogate) _NullObject;

    _SetValue(new, _P_Destination, lwk_c_domain_surrogate, &new_surrogate,
	lwk_c_set_property);

    /*
    ** Put the new Step into the new Path
    */
    
    _SetValue(new, _P_Path, lwk_c_domain_path, &aggregate->aggregate,
	lwk_c_set_property);

    return (_Termination) 0;
    }


static _Termination  DropSubObject(null, list, domain, object)
_Closure null;
 _List list;
 _Domain domain;

    _Persistent *object;

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


static void  RetrieveSurrogates(path)
_Path path;

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

    QuerySurrogates(path, (_QueryExpression) _NullObject);

    return;
    }


static void  RetrieveSteps(path)
_Path path;

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
    _Step step;
    _Integer count;
    _List volatile new;

    /*
    **  Retrieve all Steps is equivalent to a Query with a null Selection
    **	Expression.
    */

    QuerySteps(path, (_QueryExpression) _NullObject);

    new = (_List) _NullObject;

    _StartExceptionBlock

    /*
    **	Starting at the First Step, run down the List of Steps and add them, in
    **	order, to a new List.  This new list becomes the Steps list.
    */
    
    if (_Steps_of(path) != (_List) _NullObject) {
	_GetValue(_Steps_of(path), _P_ElementCount, lwk_c_domain_integer,
	    &count);

	if (count > 0) {
	    new = (_List) _CreateList(_TypeList, lwk_c_domain_step, count);

	    step = _FirstStep_of(path);

	    while (step != (_Step) _NullObject) {
		_AddElement(new, lwk_c_domain_step, &step, _True);
		_GetValue(step, _P_NextStep, lwk_c_domain_step, &step);
	    }
	}

	_Delete(&_Steps_of(path));
	_Steps_of(path) = new;
    }

    /*
    **  If any exceptions are raised, clean up then reraise the exception.
    */
    
    _Exceptions
	_WhenOthers
	    _Delete(&new);
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  QuerySurrogates(path, expression)
_Path path;
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
    **  Find the Repository in which the Path is stored
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Preserve the Modified state of the Path
    */

    modified = (_Boolean) _SetState(path, _StateModified, _StateGet);

    /*
    **  Retrieve the requested Surrogates
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);
    _GetValue(path, _P_Identifier, lwk_c_domain_integer, &id);

    LwkLbQuerySurrInContainer(repository, path, id, expression);

    /*
    **  Restore the Modified state of the Path
    */

    if (!modified)
	_SetState(path, _StateModified, _StateClear);

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


static void  QuerySteps(path, expression)
_Path path;
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
    **  Find the Repository in which the Path is stored
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Preserve the Modified state of the Path
    */

    modified = (_Boolean) _SetState(path, _StateModified, _StateGet);

    /*
    **  Retrieve the requested Steps
    */

    _GetValue(path, _P_Linkbase, lwk_c_domain_linkbase, &repository);
    _GetValue(path, _P_Identifier, lwk_c_domain_integer, &id);

    LwkLbQueryStepInPath(repository, path, id, expression);

    /*
    **  Restore the Modified state of the Path
    */

    if (!modified)
	_SetState(path, _StateModified, _StateClear);

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


static _Termination  DeleteSubObject(null, list, domain, object)
_Closure null;
 _List list;
 _Domain domain;

    _Persistent *object;

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
    **	Delete the SubObject (i.e., Step or Surrogate).  Must set PendingDelete
    **	to be sure it actually gets deleted!  Use a local pointer to Object
    **	since Delete will set the Object to the NullObject.
    */

    delete = *object;

    _SetState(delete, _StateDeletePending, _StateSet);

    _Delete(&delete);

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static _Termination  QueryPath(context, path, domain, object)
_QueryContext *context;
 _Path path;

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
    _Termination termination;

    /*
    **  If the object is selected by the selection expression, invoke the
    **	callback routine.
    */

    if (_Selected(*object, context->expression))
	termination = (*context->routine)(context->closure, path,
	    domain, object);
    else
	termination = (_Termination) 0;

    return termination;
    }


static void  EncodeSurrogates(path, handle)
_Path path;
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

    if (!(_Boolean) _SetState(path, _StateHaveAllSurrogates, _StateSet))
	RetrieveSurrogates(path);

    if (_Surrogates_of(path) != (_Set) _NullObject) {
	_GetValue(_Surrogates_of(path), _P_ElementCount, lwk_c_domain_integer,
	    &count);

	if (count > 0) {
	    /*
	    **  Begin the Surrogates Encoding.
	    */

	    type = LWK_K_T_PATH_SURROGATES;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of Surrogates.
	    */

	    type = LWK_K_T_PATH_SURR_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Surrogates Set.
	    */

	    type = LWK_K_T_PATH_SURR_ELEMENTS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Surrogates Set encoding each one.
	    */

	    _Iterate(_Surrogates_of(path), lwk_c_domain_surrogate,
		(_Closure) handle, EncodeSurrogate);

	    /*
	    **  End the Surrogates Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Path Surrogates encoding.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    return;
    }


static _Termination  EncodeSurrogate(handle, surrogates, domain, surrogate)
_DDIShandle handle;
 _Set surrogates;

    _Domain domain;
 _Surrogate *surrogate;

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
    _VaryingString encoding;
    _DDIStype type;
    _DDISlength value_length;

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
    
    type = LWK_K_T_PATH_SURR_PROPS;
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) encoding,
	(_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Surrogate ID (its address).
    */
    
    type = LWK_K_T_PATH_SURR_ID;
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


static void  EncodeSteps(path, handle)
_Path path;
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
    **  Retrieve the Steps if we don't have them.
    */

    if (!(_Boolean) _SetState(path, _StateHaveAllSteps, _StateSet))
	RetrieveSteps(path);

    if (_Steps_of(path) != (_Set) _NullObject) {
	_GetValue(_Steps_of(path), _P_ElementCount, lwk_c_domain_integer,
	    &count);

	if (count > 0) {
	    /*
	    **  Begin the Steps Encoding.
	    */

	    type = LWK_K_T_PATH_STEPS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  Encode the count of Steps.
	    */

	    type = LWK_K_T_PATH_STEP_COUNT;
	    value_length = sizeof(_Integer);
	    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &count,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Encode the Steps Set.
	    */

	    type = LWK_K_T_PATH_STEP_ELEMENTS;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	    	(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
     
	    /*
	    **  Iterate over the Steps Set encoding each one.
	    */

	    _Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) handle,
		EncodeStep);

	    /*
	    **  End the Steps Set.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);

	    /*
	    **  End the Path Steps encoding.
	    */

	    type = DDIS_K_T_EOC;
	    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
		(_DDISconstantPtr) 0);

	    if (!_Success(status))
		_Raise(internal_encoding_error);
	}
    }

    return;
    }


static _Termination  EncodeStep(handle, steps, domain, step)
_DDIShandle handle;
 _Set steps;
 _Domain domain;

    _Step *step;

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
    _Path path;
    _DDISstatus status;
    _Integer integer;
    _Surrogate surrogate;
    _VaryingString encoding;
    _DDIStype type;
    _DDISlength value_length;

    /*
    **  Begin encoding the Step.
    */
    
    type = DDIS_K_T_SEQUENCE;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Export the Step
    */

    value_length = _Export(*step, _False, &encoding);

    /*
    **  Encode the Step.
    */
    
    type = LWK_K_T_PATH_STEP_PROPS;
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) encoding,
	(_DDISconstantPtr) 0);

    _DeleteEncoding(&encoding);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Origin ID (its address).
    */
    
    _GetValue(*step, _P_Origin, lwk_c_domain_surrogate, &surrogate);

    type = LWK_K_T_PATH_STEP_ORIG_ID;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &surrogate,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode the Destination ID (its address).
    */
    
    _GetValue(*step, _P_Destination, lwk_c_domain_surrogate, &surrogate);

    type = LWK_K_T_PATH_STEP_DEST_ID;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &surrogate,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encode IsFollowedStep
    */

    _GetValue(*step, _P_Path, lwk_c_domain_path, &path);

    if (_FollowedStep_of(path) == *step)
	integer = 1;
    else
	integer = 0;

    type = LWK_K_T_PATH_STEP_IS_CURR;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length, (_DDISdata) &integer,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  End the Step encoding.
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


static _HashTableEntry  *DecodeSurrogates(path, handle, keys_only)
_Path path;
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
    _HashTableEntry volatile *hash_table;
    _DDIStype type;
    _DDISlength value_length,
		get_length;
    _DDISentry entry;
    _DDIStable table;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Initialize hash table (not needed if extracting Key Properties).
    */

    if (keys_only)
	hash_table = (_HashTableEntry *) 0;
    else {
	hash_table = (_HashTableEntry *)
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

    if (entry != LWK_K_P_PATH_SURR_COUNT)
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

    if (entry != LWK_K_P_PATH_SURR_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **	Decode count Surrogates, or just skip them if extracting Key
    **	Properties.
    */

    if (!keys_only)
	_Surrogates_of(path) =
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

	if (type != LWK_K_T_PATH_SURR_PROPS)
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
	    **  Add the Surrogate to the Path
	    */

	    _SetValue(surrogate, _P_Container, lwk_c_domain_path, &path,
		lwk_c_set_property);
	}

	/*
	**  Decode the Surrogate's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_PATH_SURR_ID)
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

    if (entry != LWK_K_E_PATH_SURR_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Surrogates.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PATH_SURROGATES)
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
    **  Return the hash table (it is used to reconstruct the Steps).
    */

    return((_HashTableEntry *) hash_table);
    }


static void  DecodeSteps(path, handle, hash_table, keys_only)
_Path path;
 _DDIShandle handle;

    _HashTableEntry *hash_table;
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
    _Step step;
    _Integer is_followed_step;
    _VaryingString encoding;
    _DDIStype type;
    _DDISlength value_length,
		get_length;
    _DDISentry entry;
    _DDIStable table;
    _DDISconstant additional_info;
    _DDISconstant cnt;

    /*
    **  Get Step count
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PATH_STEP_COUNT)
	_Raise(inv_encoding);

    value_length = sizeof(_Integer);

    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
	(_DDISdata) &integer, &get_length, &cnt);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    count = (_Integer) cnt;

    /*
    **  Begin decoding the Steps
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PATH_STEP_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Decode count Steps, or just skip them if extracting Key Properties.
    */

    if (!keys_only)
	_Steps_of(path) = (_Set) _CreateSet(_TypeSet, lwk_c_domain_step, count);

    for (i = 0; i < count; i++) {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_SEQUENCE)
	    _Raise(inv_encoding);

	/*
	**  Get the Step's encoding and Import it into a Step
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != LWK_K_T_PATH_STEP_PROPS)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    encoding = (_VaryingString) _AllocateMem(value_length);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) encoding, &get_length, &additional_info);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    if (value_length != get_length)
		_Raise(internal_decoding_error);

	    step = (_Step) _Import(_TypeStep, encoding);

	    _FreeMem(encoding);
        }

	/*
	**  Decode the Origin's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_PATH_STEP_ORIG_ID)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &id);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **  Using the ID as key, find the Surrogate in the hash table, and set
	    **  it as the Origin of the Step.
	    */

	    if (!_HashSearch(hash_table, _ImportHashSize, id, &surrogate))
		surrogate = (_Surrogate) _NullObject;

	    _SetValue(step, _P_Origin, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);
	}

	/*
	**  Decode the Destination's ID
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_PATH_STEP_DEST_ID)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, (_DDISconstantPtr) &id);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **  Using the ID as key, find the Surrogate in the hash table, and set
	    **  it as the Destination of the Step.
	    */

	    if (!_HashSearch(hash_table, _ImportHashSize, id, &surrogate))
		surrogate = (_Surrogate) _NullObject;

	    _SetValue(step, _P_Destination, lwk_c_domain_surrogate, &surrogate,
		lwk_c_set_property);

	    /*
	    **  Add the Step to the Path
	    */

	    _SetValue(step, _P_Path, lwk_c_domain_path, &path, lwk_c_set_property);
	}

	/*
	**  Decode IsFollowedStep
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (entry != LWK_K_P_PATH_STEP_IS_CURR)
	    _Raise(inv_encoding);

	if (!keys_only) {
	    value_length = sizeof(_Integer);

	    status = ddis_get_value(&handle, (_DDISsizePtr) &value_length,
		(_DDISdata) &integer, &get_length, 
		(_DDISconstantPtr) &is_followed_step);

	    if (!_Success(status))
		_Raise(internal_decoding_error);

	    /*
	    **  If this Step was the FollowedStep, reset it
	    */

	    if (is_followed_step != 0)
		_FollowedStep_of(path) = step;
	}

	/*
	**  Finish decoding this Step
	*/

	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	if (type != DDIS_K_T_EOC)
	    _Raise(inv_encoding);
    }

    /*
    **  Finish decoding the Step Set.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PATH_STEP_ELEMENTS)
	_Raise(inv_encoding);

    /*
    **  Finish decoding the Steps.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_E_PATH_STEPS)
	_Raise(inv_encoding);

    return;
    }


static void  SetOrigin(path)
_Path path;

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
    **  Reset the Origin of the Path
    */

    if (_FirstStep_of(path) == (_Step) _NullObject)
	_Origin_of(path) = (_Surrogate) _NullObject;
    else
	_GetValue(_FirstStep_of(path), _P_Origin, lwk_c_domain_surrogate,
	    &_Origin_of(path));

    return;
    }


static void  SetDestination(path)
_Path path;

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
    **  Reset the Destination of the Path
    */

    if (_LastStep_of(path) == (_Step) _NullObject)
	_Destination_of(path) = (_Surrogate) _NullObject;
    else
	_GetValue(_LastStep_of(path), _P_Destination, lwk_c_domain_surrogate,
	    &_Destination_of(path));

    return;
    }


static void  SequenceSteps(path, flag, steps)
_Path path;
 _SetFlag flag;
 _Object steps;

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
    **  Re-sequence the Steps List as required
    */

    switch (flag) {
	case lwk_c_clear_property :
	    ClearSteps(path);
	    break;

	case lwk_c_remove_property :
	    RemoveSteps(path, steps);
	    break;

	case lwk_c_add_property :
	    AddSteps(path, steps);
	    break;

	case lwk_c_set_property :
	    SetSteps(path);
	    break;
    }

    return;
    }


static void  ClearSteps(path)
_Path path;

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
    **	Clear First/Last/Current Steps and Origin/Destination of
    **	the Path
    */

    _Origin_of(path) = (_Surrogate) _NullObject;
    _Destination_of(path) = (_Surrogate) _NullObject;

    _FirstStep_of(path) = (_Step) _NullObject;
    _LastStep_of(path) = (_Step) _NullObject;
    _FirstStep_of(path) = (_Step) _NullObject;

    return;
    }


static void  SetSteps(path)
_Path path;

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
    _Step step;

    /*
    **  A totally new List of Steps was provided, link them all
    */

    if (_Steps_of(path) == (_List) _NullObject)
	ClearSteps(path);
    else {
	step = (_Step) _NullObject;

	_Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) &step,
	    LinkStep);

	/*
	**  Reset the Origin/Destination, First/Last Steps and validate the
	**  FollowedStep
	*/

	_SelectElement(_Steps_of(path), (_Integer) 0, lwk_c_domain_step,
	    &_FirstStep_of(path));

	_SelectElement(_Steps_of(path), (_Integer) -1, lwk_c_domain_step,
	    &_LastStep_of(path));

	_GetValue(step, _P_Origin, lwk_c_domain_surrogate,
	    &_Destination_of(path));

	_GetValue(step, _P_Destination, lwk_c_domain_surrogate,
	    &_Destination_of(path));

	if (!IsValidPathStep(path, _FollowedStep_of(path)))
	    _FollowedStep_of(path) = (_Step) _NullObject;
    }

    return;
    }


static _Termination  LinkStep(previous, list, domain, step)
_Step *previous;
 _List list;
 _Domain domain;

    _Step *step;

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
    **  If there was a previous Step, set this Step to be its NextStep
    */

    if (*previous != (_Step) _NullObject)
	_SetValue(*previous, _P_NextStep, lwk_c_domain_step, step,
	    lwk_c_set_property);

    /*
    **  Set this Step's PreviousStep to be the previous Step
    */

    _SetValue(*step, _P_PreviousStep, lwk_c_domain_step, previous,
	lwk_c_set_property);

    /*
    **  Clear this Step's NextStep
    */

    _SetValue(*step, _P_NextStep, lwk_c_domain_step, (_Step *) _NullObject,
	lwk_c_clear_property);

    /*
    **  Make this Step the previous Step
    */

    *previous = *step;

    /*
    **  Continue Iteration
    */

    return (_Termination) 0;
    }


static void  AddSteps(path, steps)
_Path path;
 _Object steps;

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
    **  One or more Steps were added to the List, link them to the
    **	previous LastStep
    */

    if (_IsType(steps, _TypeList))
        _Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) path, AddStep);
    else
	AddStep(path, (_List) _NullObject, lwk_c_domain_step, &steps);

    /*
    **  Reset the Path's Origin if necessary
    */

    if (_Origin_of(path) == (_Surrogate) _NullObject
	    && _FirstStep_of(path) != (_Step) _NullObject)
	_GetValue(_FirstStep_of(path), _P_Origin, lwk_c_domain_surrogate,
	    &_Origin_of(path));

    /*
    **  Reset the Path's Destination
    */

    if (_LastStep_of(path) == (_Step) _NullObject)
	_Destination_of(path) = (_Surrogate) _NullObject;
    else
	_GetValue(_LastStep_of(path), _P_Destination, lwk_c_domain_surrogate,
	    &_Destination_of(path));

    return;
    }


static _Termination  AddStep(path, list, domain, step)
_Path path;
 _List list;
 _Domain domain;

    _Step *step;

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
    **	If this is a new Step (i.e., it did not have its NextStep and/or
    **	PreviousStep set when it was Retrieved from a Repository), it needs to
    **	be linked into the list
    */

    if (!_SetState(*step, _StatePreviousStepIsId, _StateGet)
	    && !_SetState(*step, _StateNextStepIsId, _StateGet)) {
	/*
	**  If there is a Last Step in the Path, set its NextStep to be this
	**  Step
	*/
	
	if (_LastStep_of(path) != (_Step) _NullObject)
	    _SetValue(_LastStep_of(path), _P_NextStep, lwk_c_domain_step, step,
		lwk_c_set_property);

	/*
	**  Make the PreviousStep of this Step be the LastStep of the Path
	*/

	_SetValue(*step, _P_PreviousStep, lwk_c_domain_step, &_LastStep_of(path),
	    lwk_c_set_property);

	/*
	**  Make this Step the LastStep of the Path
	*/
	
	_LastStep_of(path) = *step;

	/*
	**  If there isn't one already, make this Step the FirstStep in the
	**  Path
	*/

	if (_FirstStep_of(path) == (_Step) _NullObject)
	    _FirstStep_of(path) = *step;
    }

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static void  RemoveSteps(path, steps)
_Path path;
 _Object steps;

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
    **  If the Steps are being removed as part of the Path Deletion process,
    **	don't bother cleaning up the Previous/NextStep links.
    */

    if ((_Boolean) _SetState(path, _StateDeletePending, _StateGet))
	return;

    /*
    **	If one or more Steps were removed from the List, unlink them from the
    **	List.
    */

    if (_IsType(steps, _TypeList))
        _Iterate(_Steps_of(path), lwk_c_domain_step, (_Closure) path,
	    RemoveStep);
    else
	RemoveStep(path, (_List) _NullObject, lwk_c_domain_step, &steps);

    return;
    }


static _Termination  RemoveStep(path, list, domain, step)
_Path path;
 _List list;
 _Domain domain;

    _Step *step;

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
    _Step previous;
    _Step next;

    /*
    **  Get the PreviousStep and NextStep of this Step
    */

    _GetValue(*step, _P_PreviousStep, lwk_c_domain_step, &previous);
    _GetValue(*step, _P_NextStep, lwk_c_domain_step, &next);

    /*
    **	Make the NextStep of the PreviousStep (if any) be the NextStep of this
    **	Step
    */

    if (previous != (_Step) _NullObject)
	_SetValue(previous, _P_NextStep, lwk_c_domain_step, &next,
	    lwk_c_set_property);

    /*
    **	Make the PreviousStep of the NextStep (if any) be the PreviousStep of
    **	this Step
    */

    if (next != (_Step) _NullObject)
	_SetValue(next, _P_PreviousStep, lwk_c_domain_step, &previous,
	    lwk_c_set_property);

    /*
    **	If we are removing the FirstStep of the Path, reset the FirstStep of
    **	the Path to be the NextStep of this Step, and reset the Origin of the
    **	Path.
    */
    
    if (_FirstStep_of(path) == *step) {
	_FirstStep_of(path) = next;

	if (_FirstStep_of(path) == (_Step) _NullObject)
	    _Origin_of(path) = (_Surrogate) _NullObject;
	else
	    _GetValue(_FirstStep_of(path), _P_Origin, lwk_c_domain_surrogate,
		&_Origin_of(path));
    }

    /*
    **	If we are removing the LastStep of the Path, reset the LastStep of the
    **	Path to be the PreviousStep of this Step, and reset the Destination of
    **	the Path.
    */
    
    if (_LastStep_of(path) == *step) {
	_LastStep_of(path) = previous;

	if (_LastStep_of(path) == (_Step) _NullObject)
	    _Destination_of(path) = (_Surrogate) _NullObject;
	else
	    _GetValue(_LastStep_of(path), _P_Destination, lwk_c_domain_surrogate,
		&_Destination_of(path));
    }

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static _Boolean  IsValidPathStep(path, step)
_Path path;
 _Step step;

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
    _Path step_path;

    /*
    **  The Path of the Step must be the given Path
    */

    if (step != (_Step) _NullObject) {
	_GetValue(step, _P_Path, lwk_c_domain_path, &step_path);

	if (step_path != path)
	    return _False;
    }

    return _True;
    }


static void  FreeHashTable(table)
_HashTableEntry *table;

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

    if (table != (_HashTableEntry *) 0) {
	_HashFree(table, _ImportHashSize);
	_FreeMem(table);
    }

    return;
    }


static _Termination  PreStoreStep(repository, list, domain, step)
_Linkbase repository;
 _List list;

    _Domain domain;
 _Step *step;

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
    **	If the Step has DeletePending, Drop it from the Repository and complete
    **	the Deletion.
    */

    if ((_Boolean) _SetState(*step, _StateDeletePending, _StateGet)) {
	_Persistent delete;

	delete = *step;
	LwkLbDrop(domain, delete);
	_Delete(&delete);
    }

    /*
    **  Make sure that this Step has an Identifier assigned
    */

    LwkLbAssignIdentifier(repository, *step);

    /*
    **  Continue the Iteration
    */

    return (_Termination) 0;
    }


static _Termination  StoreStep(repository, list, domain, step)
_Linkbase repository;
 _List list;

    _Domain domain;
 _Step *step;

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
    **  Ask the Database module to Store the Step
    */

    LwkLbStore(repository, domain, *step);

    return (_Termination) 0;
    }


static _Termination  StoreSurrogate(repository, list, domain, surrogate)
_Linkbase repository;
 _List list;

    _Domain domain;
 _Surrogate *surrogate;

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
    **  If the Surrogate has DeletePending, Drop it from the Repository and
    **	complete the Deletion.  Otherwise, ask the Database module to Store it.
    */

    if ((_Boolean) _SetState(*surrogate, _StateDeletePending, _StateGet)) {
	_Persistent delete;

	delete = *surrogate;
	LwkLbDrop(domain, delete);
	_Delete(&delete);
    }
    else
	LwkLbStore(repository, domain, *surrogate);

    return (_Termination) 0;
    }
