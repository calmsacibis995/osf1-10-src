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
**	Path Step object methods 
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
#include "step.h"
#include "database.h"
#include "ddisencd.h"
#else /* !MSDOS */
#include "his_include.h"
#include "lwk_step.h"
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

_DeclareFunction(static void MoveStep, (_Step step, _Path path));
_DeclareFunction(static void SetOrigin, (_Step step, _Surrogate old_origin));
_DeclareFunction(static void SetDestination,
    (_Step step, _Surrogate old_destination));
_DeclareFunction(static void RetrieveOrigin, (_Step step));
_DeclareFunction(static void RetrieveDestination, (_Step step));
_DeclareFunction(static void RetrievePreviousStep, (_Step step));
_DeclareFunction(static void RetrieveNextStep, (_Step step));
_DeclareFunction(static void MoveSurrogate, (_Surrogate surrogate, _Path path));
_DeclareFunction(static void EncodeSurrogate,
    (_Surrogate surrogate, unsigned int type, _DDIShandle handle));
_DeclareFunction(static _Surrogate DecodeSurrogate,
    (_DDIShandle handle, _DDISsize value_length));

/*
**  Global Data Definitions
*/

_Global _StepTypeInstance;      /* Instance record for Type */
_Global _StepType;              /* Type */

/*
**  External Data Declarations
*/

/*
**  Static Data Definitions
*/

static _Type _Constant MyType = &_TypeStepInstance;
static _StepPropertyNameTable;
static _PropertyNameTableEntry _Constant *MyProperties = StepPropertyNameTable;


void  LwkOpStep(step)
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
    return;
    }


void  LwkOpStepInitialize(step, proto, aggregate)
_Step step;
 _Step proto;
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

    _ClearValue(&_Origin_of(step).surrogate, lwk_c_domain_surrogate);
    _ClearValue(&_Destination_of(step).surrogate, lwk_c_domain_surrogate);
    _ClearValue(&_PreviousStep_of(step).step, lwk_c_domain_step);
    _ClearValue(&_NextStep_of(step).step, lwk_c_domain_step);
    _ClearValue(&_FollowType_of(step), lwk_c_domain_integer);
    _ClearValue(&_DestinationOperation_of(step), lwk_c_domain_string);
    _ClearValue(&_Interval_of(step), lwk_c_domain_integer);
    _ClearValue(&_Path_of(step), lwk_c_domain_path);

    /*
    **  Invoke the Initialize operation in our supertype
    */

    _Initialize_S(step, proto, aggregate, MyType);
    
    /*
    **  If a prototype object was provided, copy properties from it.
    */

    if (proto != (_Step) _NullObject) {
	_CopyValue(&_FollowType_of(proto), &_FollowType_of(step),
	    lwk_c_domain_integer);
	_CopyValue(&_DestinationOperation_of(proto),
	    &_DestinationOperation_of(step), lwk_c_domain_string);
	_CopyValue(&_Interval_of(proto), &_Interval_of(step),
	    lwk_c_domain_integer);

	if (aggregate) {
	    if (_Origin_of(proto).surrogate != (_Surrogate) _NullObject)
		_Origin_of(step).surrogate = (_Surrogate)
		    _Copy(_Origin_of(proto).surrogate, _False);

	    if (_Destination_of(proto).surrogate != (_Surrogate) _NullObject)
		_Destination_of(step).surrogate = (_Surrogate)
		    _Copy(_Destination_of(proto).surrogate, _False);
	}
    }

    return;
    }


void  LwkOpStepExpunge(step)
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
    _Path old_path;
    _Boolean phase_2;
    _Linkbase repository;
    _Surrogate old_surrogate;

    /*
    ** Complement the Pending Delete state.  If it was already set, we need to
    ** complete phase 2 of the deletion process.  If the Step is not Stored in
    ** a Repository, we cna complete phase 2 now too.
    */

    phase_2 = _False;

    if ((_Boolean) _SetState(step, _StateDeletePending, _StateComplement))
	phase_2 = _True;
    else {
	_Linkbase repository;

	_GetValue(step, _P_Linkbase, lwk_c_domain_linkbase, &repository);

	if (repository == (_Linkbase) _NullObject)
	    phase_2 = _True;
	else
	    phase_2 = _False;
    }

    /*
    ** Always clear the Origin and Destination of the Step
    */

    if ((_Boolean) _SetState(step, _StateOriginIsId, _StateClear))
	_Origin_of(step).surrogate = (_Surrogate) _NullObject;
    else {
	old_surrogate = _Origin_of(step).surrogate;
	_Origin_of(step).surrogate = (_Surrogate) _NullObject;
	SetOrigin(step, old_surrogate);
    }

    if ((_Boolean) _SetState(step, _StateDestinationIsId, _StateClear))
	_Destination_of(step).surrogate = (_Surrogate) _NullObject;
    else {
	old_surrogate = _Destination_of(step).surrogate;
	_Destination_of(step).surrogate = (_Surrogate) _NullObject;
	SetDestination(step, old_surrogate);
    }

    /*
    ** On phase 2, remove the Step from its Path and complete the deletion
    ** process.
    */
    
    if (phase_2) {
	old_path = _Path_of(step);
	_Path_of(step) = (_Path) _NullObject;

	MoveStep(step, old_path);

	_Expunge_S(step, MyType);
    }

    return;
    }


void  LwkOpStepFree(step)
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
    _Boolean is_id;

    /*
    **  If this Step was part of a Path, remove it from the _Steps list of
    **  that Path.
    */

    if (_Path_of(step) != (_Path) _NullObject)
	_SetValue(_Path_of(step), _P_Steps, lwk_c_domain_step, &step,
	    lwk_c_remove_property);

    /*
    **  If there was an _Origin, remove this Step from the
    **  _InterConnections list of that Surrogate.
    */

    is_id = (_Boolean) _SetState(step, _StateOriginIsId, _StateGet);

    if (!is_id && _Origin_of(step).surrogate != (_Surrogate) _NullObject)
	_SetValue(_Origin_of(step).surrogate, _P_InterLinks,
	    lwk_c_domain_step, &step, lwk_c_remove_property);

    /*
    **  If there was a _Destination, remove this Step from the
    **  _InterConnections list of that Surrogate.
    */

    is_id = (_Boolean) _SetState(step, _StateDestinationIsId, _StateGet);

    if (!is_id && _Destination_of(step).surrogate != (_Surrogate) _NullObject)
	_SetValue(_Destination_of(step).surrogate, _P_InterLinks,
	    lwk_c_domain_step, &step, lwk_c_remove_property);

    /*
    **  Free the storage used by the properties defined by our Type.
    */

    _DeleteValue(&_Origin_of(step).surrogate, lwk_c_domain_surrogate);
    _DeleteValue(&_Destination_of(step).surrogate, lwk_c_domain_surrogate);
    _DeleteValue(&_PreviousStep_of(step).step, lwk_c_domain_step);
    _DeleteValue(&_NextStep_of(step).step, lwk_c_domain_step);
    _DeleteValue(&_FollowType_of(step), lwk_c_domain_integer);
    _DeleteValue(&_DestinationOperation_of(step), lwk_c_domain_string);
    _DeleteValue(&_Interval_of(step), lwk_c_domain_integer);
    _DeleteValue(&_Path_of(step), lwk_c_domain_path);

    /*
    **  Ask our supertype to free its properties
    */

    _Free_S(step, MyType);

    return;
    }


_Set  LwkOpStepEnumProps(step)
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
    _Set set;

    /*
    **  Ask our supertype to enumerate its properties.
    */

    set = (_Set) _EnumerateProperties_S(step, MyType);

    /*
    **  Add our properties to the set.
    */

    _ListTable(MyProperties, set, _False);

    /*
    **  Return the property name set to the caller
    */

    return set;
    }


_Boolean  LwkOpStepIsMultiValued(step, property)
_Step step;
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
	answer = _IsMultiValued_S(step, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
	    case _DestinationIndex :
	    case _PreviousStepIndex :
	    case _NextStepIndex :
	    case _FollowTypeIndex :
	    case _DestinationOperationIndex :
	    case _IntervalIndex :
	    case _PathIndex :
		answer = _False;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return answer;
    }


_Domain  LwkOpStepGetValueDomain(step, property)
_Step step;
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
	domain = (_Domain) _GetValueDomain_S(step, property, MyType);
    }
    else {
	/*
	**  Domain depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
	    case _DestinationIndex :
		domain = lwk_c_domain_surrogate;
		break;

	    case _PreviousStepIndex :
	    case _NextStepIndex :
		domain = lwk_c_domain_step;
		break;

	    case _FollowTypeIndex :
	    case _IntervalIndex :
		domain = lwk_c_domain_integer;
		break;

	    case _DestinationOperationIndex :
		domain = lwk_c_domain_string;
		break;

	    case _PathIndex :
		domain = lwk_c_domain_path;
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return domain;
    }


void  LwkOpStepGetValue(step, property, domain, value)
_Step step;
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
	_GetValue_S(step, property, domain, value, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Origin_of(step).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_surrogate)) {
		    /*
		    **  Retrieve the Origin if we only know its Id.
		    */

		    if ((_Boolean) _SetState(step, _StateOriginIsId,
			    _StateClear))
			RetrieveOrigin(step);

		    _CopyValue(&_Origin_of(step).surrogate, value,
			lwk_c_domain_surrogate);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _DestinationIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Destination_of(step).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_surrogate)) {
		    /*
		    **  Retrieve the Destination if we only know its Id.
		    */

		    if ((_Boolean) _SetState(step, _StateDestinationIsId,
			    _StateClear))
			RetrieveDestination(step);

		    _CopyValue(&_Destination_of(step).surrogate, value,
			lwk_c_domain_surrogate);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _PreviousStepIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_PreviousStep_of(step).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_step)) {
		    /*
		    **  Retrieve the PreviousStep if we only know its Id.
		    */

		    if ((_Boolean) _SetState(step, _StatePreviousStepIsId,
			    _StateClear))
			RetrievePreviousStep(step);

		    _CopyValue(&_PreviousStep_of(step).step, value,
			lwk_c_domain_step);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _NextStepIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_NextStep_of(step).id, value,
			lwk_c_domain_integer);
		else if (_IsDomain(domain, lwk_c_domain_step)) {
		    /*
		    **  Retrieve the NextStep if we only know its Id.
		    */

		    if ((_Boolean) _SetState(step, _StateNextStepIsId,
			    _StateClear))
			RetrieveNextStep(step);

		    _CopyValue(&_NextStep_of(step).step, value,
			lwk_c_domain_step);
		}
		else
		    _Raise(inv_domain);

		break;

	    case _FollowTypeIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_FollowType_of(step), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _IntervalIndex :
		if (_IsDomain(domain, lwk_c_domain_integer))
		    _CopyValue(&_Interval_of(step), value,
			lwk_c_domain_integer);
		else
		    _Raise(inv_domain);

		break;

	    case _DestinationOperationIndex :
		if (_IsDomain(domain, lwk_c_domain_string))
		    _CopyValue(&_DestinationOperation_of(step), value,
			lwk_c_domain_string);
		else
		    _Raise(inv_domain);

		break;

	    case _PathIndex :
		if (_IsDomain(domain, lwk_c_domain_path))
		    _CopyValue(&_Path_of(step), value, lwk_c_domain_path);
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


_List  LwkOpStepGetValueList(step, property)
_Step step;
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
	return (_List) _GetValueList_S(step, property, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
		/*
		**  Retrieve the Origin if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateOriginIsId,
			_StateClear))
		    RetrieveOrigin(step);

		_ListValue(&_Origin_of(step).surrogate, &value_list,
		    lwk_c_domain_surrogate);

		break;

	    case _DestinationIndex :
		/*
		**  Retrieve the Destination if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateDestinationIsId,
			_StateClear))
		    RetrieveDestination(step);

		_ListValue(&_Destination_of(step).surrogate, &value_list,
		    lwk_c_domain_surrogate);

		break;

	    case _PreviousStepIndex :
		/*
		**  Retrieve the PreviousStep if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StatePreviousStepIsId,
			_StateClear))
		    RetrievePreviousStep(step);

		_ListValue(&_PreviousStep_of(step).step, &value_list,
		    lwk_c_domain_step);

		break;

	    case _NextStepIndex :
		/*
		**  Retrieve the NextStep if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateNextStepIsId,
			_StateClear))
		    RetrieveNextStep(step);

		_ListValue(&_NextStep_of(step).step, &value_list,
		    lwk_c_domain_step);

		break;

	    case _FollowTypeIndex :
		_ListValue(&_FollowType_of(step), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _IntervalIndex :
		_ListValue(&_Interval_of(step), &value_list,
		    lwk_c_domain_integer);
		break;

	    case _DestinationOperationIndex :
		_ListValue(&_DestinationOperation_of(step), &value_list,
		    lwk_c_domain_string);
		break;

	    case _PathIndex :
		_ListValue(&_Path_of(step), &value_list,
		    lwk_c_domain_path);
		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    return value_list;
    }


void  LwkOpStepSetValue(step, property, domain, value, flag)
_Step step;
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
    _Surrogate old_surrogate;
    _Path old_path;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValue_S(step, property, domain, value, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
		/*
		**  Retrieve the Origin if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateOriginIsId,
			_StateClear))
		    RetrieveOrigin(step);

		/*
		**  Save the old Origin and then Set either the new Origin, or
		**  the new Origin Id.
		*/

		old_surrogate = _Origin_of(step).surrogate;

		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_Origin_of(step).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState(step, _StateOriginIsId, _StateSet);
		}
		else
		    _SetSingleValuedProperty(&_Origin_of(step).surrogate,
			lwk_c_domain_surrogate, domain, value, flag, _False);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterConnection property values.
		*/

		SetOrigin(step, old_surrogate);

		break;

	    case _DestinationIndex :
		/*
		**  Retrieve the Destination if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateDestinationIsId,
			_StateClear))
		    RetrieveDestination(step);

		/*
		**  Save the old Destination and then Set either the new
		**  Destination, or the new Destination Id.
		*/

		old_surrogate = _Destination_of(step).surrogate;

		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_Destination_of(step).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState(step, _StateDestinationIsId, _StateSet);
		}
		else
		    _SetSingleValuedProperty(&_Destination_of(step).surrogate,
			lwk_c_domain_surrogate, domain, value, flag, _False);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterConnection property values.
		*/

		SetDestination(step, old_surrogate);

		break;

	    case _PreviousStepIndex :
		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_PreviousStep_of(step).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState(step, _StatePreviousStepIsId, _StateSet);
		}
		else {
		    _SetSingleValuedProperty(&_PreviousStep_of(step).step,
			lwk_c_domain_step, domain, value, flag, _False);

		    _SetState(step, _StatePreviousStepIsId, _StateClear);
		}

		break;

	    case _NextStepIndex :
		if (_IsDomain(lwk_c_domain_integer, domain)) {
		    _SetSingleValuedProperty(&_NextStep_of(step).id,
			lwk_c_domain_integer, domain, value, flag, _False);

		    _SetState(step, _StateNextStepIsId, _StateSet);
		}
		else {
		    _SetSingleValuedProperty(&_NextStep_of(step).step,
			lwk_c_domain_step, domain, value, flag, _False);

		    _SetState(step, _StateNextStepIsId, _StateClear);
		}

		break;

	    case _FollowTypeIndex :
		_SetSingleValuedProperty(&_FollowType_of(step),
		    lwk_c_domain_integer, domain, value, flag, _False);
		break;

	    case _IntervalIndex :
		_SetSingleValuedProperty(&_Interval_of(step),
		    lwk_c_domain_integer, domain, value, flag, _False);
		break;

	    case _DestinationOperationIndex :
		_SetSingleValuedProperty(&_DestinationOperation_of(step),
		    lwk_c_domain_string, domain, value, flag, _False);
		break;

	    case _PathIndex :
		old_path = _Path_of(step);

		_SetSingleValuedProperty(&_Path_of(step),
		    lwk_c_domain_path, domain, value, flag, _False);

		/*
		**  Move this step from its old Path to its new Path.
		*/

		MoveStep(step, old_path);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Step Modified.
    */

    _SetState(step, _StateModified, _StateSet);

    return;
    }


void  LwkOpStepSetValueList(step, property, values, flag)
_Step step;
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
    _Surrogate old_surrogate;
    _Path old_path;

    /*
    **	See if the property is defined by our type.  If not, pass the request
    **	on to our supertype.
    */

    index = _SearchTable(MyProperties, property, _False);

    if (index < 0) {
	_SetValueList_S(step, property, values, flag, MyType);
    }
    else {
	/*
	**  Answer depends on the property
	*/

	switch (index) {
	    case _OriginIndex :
		/*
		**  Retrieve the Origin if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateOriginIsId,
			_StateClear))
		    RetrieveOrigin(step);

		old_surrogate = _Origin_of(step).surrogate;

		_SetSingleValuedProperty(&_Origin_of(step).surrogate,
		    lwk_c_domain_surrogate, lwk_c_domain_list, values,
		    flag, _True);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterConnection property values.
		*/

		SetOrigin(step, old_surrogate);

		break;

	    case _DestinationIndex :
		/*
		**  Retrieve the Destination if we only know its Id.
		*/

		if ((_Boolean) _SetState(step, _StateDestinationIsId,
			_StateClear))
		    RetrieveDestination(step);

		old_surrogate = _Destination_of(step).surrogate;

		_SetSingleValuedProperty(&_Destination_of(step).surrogate,
		    lwk_c_domain_surrogate, lwk_c_domain_list, values,
		    flag, _True);

		/*
		**  Move this Surrogate from its old Container, if necessary,
		**  and set up the proper _InterConnection property values.
		*/

		SetDestination(step, old_surrogate);

		break;

	    case _PreviousStepIndex :
		_SetSingleValuedProperty(&_PreviousStep_of(step).step,
		    lwk_c_domain_step, lwk_c_domain_list, values, flag, _True);

		_SetState(step, _StatePreviousStepIsId, _StateClear);

		break;

	    case _NextStepIndex :
		_SetSingleValuedProperty(&_NextStep_of(step).step,
		    lwk_c_domain_step, lwk_c_domain_list, values, flag, _True);

		_SetState(step, _StateNextStepIsId, _StateClear);

		break;

	    case _FollowTypeIndex :
		_SetSingleValuedProperty(&_FollowType_of(step),
		    lwk_c_domain_integer, lwk_c_domain_list, values,
		    flag, _True);
		break;

	    case _IntervalIndex :
		_SetSingleValuedProperty(&_Interval_of(step),
		    lwk_c_domain_integer, lwk_c_domain_list, values,
		    flag, _True);
		break;

	    case _DestinationOperationIndex :
		_SetSingleValuedProperty(&_DestinationOperation_of(step),
		    lwk_c_domain_string, lwk_c_domain_list, values,
		    flag, _True);
		break;

	    case _PathIndex :
		old_path = _Path_of(step);

		_SetSingleValuedProperty(&_Path_of(step),
		    lwk_c_domain_path, lwk_c_domain_list, values, flag, _True);

		/*
		**  Move this step from its old Path to its new Path.
		*/

		MoveStep(step, old_path);

		break;

	    default :
		_Raise(no_such_property);
		break;
	}
    }

    /*
    **  Mark the Step Modified.
    */

    _SetState(step, _StateModified, _StateSet);

    return;
    }


_ObjectId  LwkOpStepGetObjectId(step)
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
    _Integer path_id;
    _ObjectId volatile oid;

    /*
    ** Let our supertype generate the basic ObjectId
    */
    
    oid = (_ObjectId) _GetObjectId_S(step, MyType);

    _StartExceptionBlock

    /*
    **  Find the Identifier of the Path in which the Step is stored
    */

    _GetValue(_Path_of(step), _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Set the Container Identifier property of the ObjectId.
    */

    _SetValue(oid, _P_ContainerIdentifier, lwk_c_domain_integer,
	&path_id, lwk_c_set_property);

    /*
    **	If any exceptions are raised, delete any new ObjectId then reraise the
    **	exception.
    */

    _Exceptions
	_WhenOthers
	    _Delete(&oid);
	    _Reraise;
    _EndExceptionBlock

    /*
    **  Return the ObjectId.
    */

    return oid;
    }


void  LwkOpStepEncode(step, aggregate, handle)
_Step step;
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
    **  Begin the encoding of Step.
    */

    type = LWK_K_T_PERS_STEP;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Encoding the properties of Step.
    */

    type = LWK_K_T_STEP_FOLLOW_TYPE;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length,
	(_DDISdata) &_FollowType_of(step), (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    type = LWK_K_T_STEP_INTERVAL;
    value_length = sizeof(_Integer);
    status = ddis_put(&handle, &type, &value_length,
	(_DDISdata) &_Interval_of(step), (_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    if (_DestinationOperation_of(step) != (_String) _NullObject) {
	type = LWK_K_T_STEP_OPERATION;
	value_length = _LengthString(_DestinationOperation_of(step));
	status = ddis_put(&handle, &type, &value_length,
	    (_DDISdata) _DestinationOperation_of(step), (_DDISconstantPtr) 0);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    /*
    **  If an aggregate Step is requested, encode the Origin and Destination
    **	Surrogates as well.
    */

    if (aggregate) {
	if ((_Boolean) _SetState(step, _StateOriginIsId, _StateClear))
	    RetrieveOrigin(step);

	EncodeSurrogate(_Origin_of(step).surrogate, LWK_K_T_STEP_ORIGIN,
	    handle);

	if ((_Boolean) _SetState(step, _StateDestinationIsId, _StateClear))
	    RetrieveDestination(step);

	EncodeSurrogate(_Destination_of(step).surrogate,
	    LWK_K_T_STEP_DESTINATION, handle);
    }

    /*
    **  Finish the Step encoding.
    */

    type = DDIS_K_T_EOC;
    status = ddis_put(&handle, &type, (_DDISlengthPtr) 0, (_DDISdata) 0,
	(_DDISconstantPtr) 0);

    if (!_Success(status))
	_Raise(internal_encoding_error);

    /*
    **  Ask our supertype to encode its properties
    */

    _Encode_S(step, aggregate, handle, MyType);

    return;
    }


void  LwkOpStepDecode(step, handle, keys_only, properties)
_Step step;
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
    _VaryingString volatile encoding;
    _DDIStype type;
    _DDISlength value_length,
		get_length;
    _DDISentry entry;
    _DDIStable table;
    _DDISconstant additional_info;

    /*
    **  Begin the Step decoding.
    */

    status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (entry != LWK_K_P_PERS_STEP)
	_Raise(inv_encoding);

    /*
    **	Decode the Step Properties.  If extracting Key Properties, all
    **	properties can be skipped -- none are a Key.
    */

    do {
	status = ddis_get_tag(&handle, &type, &value_length, &entry, &table);

	if (!_Success(status))
	    _Raise(internal_decoding_error);

	switch (entry) {
	    case LWK_K_E_PERS_STEP :
		/*
		**  End of Step Properties.
		*/
		break;

	    case LWK_K_P_STEP_FOLLOW_TYPE :
		if (!keys_only) {
		    if (value_length > sizeof(_Integer))
			_Raise(internal_decoding_error);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length, (_DDISdata) &integer,
			&get_length, (_DDISconstantPtr) &_FollowType_of(step));

		    if (!_Success(status))
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_STEP_OPERATION :
		if (!keys_only) {
		    _DestinationOperation_of(step) =
			_AllocateMem(value_length + 1);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length,
			(_DDISdata) _DestinationOperation_of(step), &get_length,
			&additional_info);

		    if (!_Success(status))
			_Raise(internal_decoding_error);

		    if (value_length != get_length)
			_Raise(internal_decoding_error);

		    _DestinationOperation_of(step)[get_length] = _EndOfString;
		}

		break;

	    case LWK_K_P_STEP_INTERVAL :
		if (!keys_only) {
		    if (value_length > sizeof(_Integer))
			_Raise(internal_decoding_error);

		    status = ddis_get_value(&handle,
			(_DDISsizePtr) &value_length, (_DDISdata) &integer,
			&get_length, (_DDISconstantPtr) &_Interval_of(step));

		    if (!_Success(status))
			_Raise(internal_decoding_error);
		}

		break;

	    case LWK_K_P_STEP_ORIGIN :
		if (!keys_only) {
		    _Origin_of(step).surrogate = DecodeSurrogate(handle,
			(_DDISsize) value_length);

		    SetOrigin(step, (_Surrogate) _NullObject);
		}

		break;

	    case LWK_K_P_STEP_DESTINATION :
		if (!keys_only) {
		    _Destination_of(step).surrogate =
			DecodeSurrogate(handle, (_DDISsize) value_length);

		    SetDestination(step, (_Surrogate) _NullObject);
		}

		break;

	    default :
		_Raise(inv_encoding);
	}

    } while (entry != LWK_K_E_PERS_STEP);

    /*
    **  Ask our supertype to decode its properties
    */

    _Decode_S(step, handle, keys_only, properties, MyType);

    return;
    }


_Boolean  LwkOpStepSelected(step, expression)
_Step step;
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
    _Boolean result;
    _QueryExpression operand_1;
    _QueryExpression operand_2;

    /*
    **  Dispatch based on the operator in root of the Selection Expression.
    */
    
    if (expression == (_QueryExpression) _NullObject)
	_Raise(inv_query_expr);

    operand_1 = (_QueryExpression) expression->lwk_operand_1;
    operand_2 = (_QueryExpression) expression->lwk_operand_2;

    switch (expression->lwk_operator) {
	case lwk_c_and :
	    /*
	    **  True if the both left and right Selection sub-Expressions are
	    **	True.
	    */

	    result = _Selected(step, operand_1)
		&& _Selected(step, operand_2);

	    break;

	case lwk_c_or :
	    /*
	    **  True if the either left and right Selection sub-Expressions is
	    **	True.
	    */

	    result = _Selected(step, operand_1)
		|| _Selected(step, operand_2);

	    break;

	case lwk_c_not :
	    /*
	    **  True if the left Selection sub-Expressions is False.
	    */

	    result = !_Selected(step, operand_1);

	    break;

	case lwk_c_any :
	    /*
	    **  Always True.
	    */

	    result = _True;

	    break;

	case lwk_c_identity :
	    /*
	    **  True if the left operand is this Step.
	    */

	    result = (step == (_Step) expression->lwk_operand_1);

	    break;

	case lwk_c_has_origin :
	    /*
	    **	True if the left Selection sub-Expressions selects the Origin
	    **	of the Step.
	    */

	    if (_Origin_of(step).surrogate == (_Surrogate) _NullObject) {
		if (operand_1 == (_QueryExpression) _NullObject)
		    result = _True;
		else
		    result = _False;
	    }
	    else {
		if (operand_1->lwk_operator == lwk_c_any)
		    result = _True;
		else {
		    if ((_Boolean) _SetState(step, _StateOriginIsId,
			    _StateClear))
			RetrieveOrigin(step);

		    result = _Selected(_Origin_of(step).surrogate, operand_1);
		}
	    }

	    break;

	case lwk_c_has_destination :
	    /*
	    **	True if the left Selection sub-Expressions selects the
	    **	Destination of the Step.
	    */

	    if (_Destination_of(step).surrogate == (_Surrogate) _NullObject) {
		if (operand_1 == (_QueryExpression) _NullObject)
		    result = _True;
		else
		    result = _False;
	    }
	    else {
		if (operand_1->lwk_operator == lwk_c_any)
		    result = _True;
		else {
		    if ((_Boolean) _SetState(step, _StateDestinationIsId,
			    _StateClear))
			RetrieveDestination(step);

		    result = _Selected(_Destination_of(step).surrogate,
			operand_1);
		}
	    }

	    break;
	
	case lwk_c_has_properties :
	    /*
	    **  True if the properties match.
	    */

	    result = _SelectProperties(step, operand_1);

	    break;

	default :
	    _Raise(inv_query_expr);
	    break;

    }

    return result;
    }


static void  MoveStep(step, old)
_Step volatile step;
 _Path volatile old;

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
    **  The _Path property of a Step may be set only in the following
    **	cases:
    **
    **	    - The new _Path is the same as the old _Path (i.e., a NoOp)
    **	    - There was no previous _Path
    **	    - There are no _Origin or _Destination Surrogates binding the
    **	      Step in the old _Path
    **
    **	If these conditions are not met, the _Path is reset to the old
    **	value and an exception is raised.
    */

    _StartExceptionBlock

    if (old != _Path_of(step)) {
	if ((old == (_Path) _NullObject) ||
	    ((_Origin_of(step).surrogate == (_Surrogate) _NullObject) &&
	     (_Destination_of(step).surrogate == (_Surrogate) _NullObject))) {

	    /*
	    **	Make sure that the _Origin and _Destination can be moved, and
	    **	if so, do it.
	    */

	    MoveSurrogate(_Origin_of(step).surrogate, _Path_of(step));
	    MoveSurrogate(_Destination_of(step).surrogate, _Path_of(step));

	    /*
	    ** Remove this Step from the _Steps property of the old Path, if
	    ** any.
	    */

	    if (old != (_Path) _NullObject) {
		_SetValue(old , _P_Steps, lwk_c_domain_step, (_AnyPtr) &step,
		    lwk_c_remove_property);

		/*
		**  Clear Previous/NextStep
		*/

		_SetState(step, _StatePreviousStepIsId, _StateClear);
		_SetState(step, _StateNextStepIsId, _StateClear);

		_PreviousStep_of(step).step = (_Step) _NullObject;
		_NextStep_of(step).step = (_Step) _NullObject;
	    }

	    /*
	    **	Add this Step to the _Steps property of the new Path, if any.
	    */

	    if (_Path_of(step) != (_Path) _NullObject)
		_SetValue(_Path_of(step), _P_Steps, lwk_c_domain_step,
		    (_AnyPtr) &step, lwk_c_add_property);
	}
	else {
	    _Raise(cannot_move_step);
	}
    }

    /*
    **  If there are any problems, reset the old _Path.
    */

    _Exceptions
	_WhenOthers
	    _Path_of(step) = old;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetOrigin(step, old_origin)
_Step volatile step;
 _Surrogate volatile old_origin;

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
    _Boolean is_id;

    /*
    **  If the Origin did not change, return now.
    */

    if (_Origin_of(step).surrogate == old_origin)
	return;

    _StartExceptionBlock

    /*
    **	Remove the Step from the _InterConnections list of the old _Origin if
    **	there was one.
    */

    if (old_origin != (_Surrogate) _NullObject)
	_SetValue(old_origin, _P_InterLinks, lwk_c_domain_step,
	    (_AnyPtr) &step, lwk_c_remove_property);

    /*
    **  If the new _Origin is not an Identifier, do some more housekeeping
    */

    is_id = (_Boolean) _SetState(step, _StateOriginIsId, _StateGet);

    if (!is_id) {
	/*
	**  Move the new _Origin to the _Path of the Step, if there is one.
	*/

	if (_Path_of(step) != (_Path) _NullObject)
	    MoveSurrogate(_Origin_of(step).surrogate, _Path_of(step));

	/*
	**  Add the Step to the _InterConnections list of the _Origin, if there
	**  is one.
	*/

	if (_Origin_of(step).surrogate != (_Surrogate) _NullObject)
	    _SetValue(_Origin_of(step).surrogate, _P_InterLinks,
		lwk_c_domain_step, (_AnyPtr) &step, lwk_c_add_property);
    }

    /*
    **  If there are any problems, reset the old _Origin.
    */

    _Exceptions
	_WhenOthers
	    _Origin_of(step).surrogate = old_origin;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  SetDestination(step, old_destination)
_Step volatile step;

    _Surrogate volatile old_destination;

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
    _Boolean is_id;

    /*
    **  If the Destination did not change, return now.
    */

    if (_Destination_of(step).surrogate == old_destination)
	return;

    _StartExceptionBlock

    /*
    **	Remove the Step from the _InterConnections list of the old _Destination
    **	if there was one.
    */

    if (old_destination != (_Surrogate) _NullObject)
	_SetValue(old_destination, _P_InterLinks, lwk_c_domain_step,
	    (_AnyPtr) &step, lwk_c_remove_property);

    /*
    **  If the new _Destination is not an Identifier, do some more housekeeping
    */

    is_id = (_Boolean) _SetState(step, _StateDestinationIsId, _StateGet);

    if (!is_id) {
	/*
	**  Move the new _Destination to the _Path of the Step, if there is
	**  one.
	*/

	if (_Path_of(step) != (_Path) _NullObject)
	    MoveSurrogate(_Destination_of(step).surrogate, _Path_of(step));

	/*
	**  Add the Step to the _InterConnections list of the _Origin, if there
	**  is one.
	*/

	if (_Destination_of(step).surrogate != (_Surrogate) _NullObject)
	    _SetValue(_Destination_of(step).surrogate, _P_InterLinks,
		lwk_c_domain_step, (_AnyPtr) &step, lwk_c_add_property);
    }

    /*
    **  If there are any problems, reset the old _Destination.
    */

    _Exceptions
	_WhenOthers
	    _Destination_of(step).surrogate = old_destination;
	    _Reraise;
    _EndExceptionBlock

    return;
    }


static void  RetrieveOrigin(step)
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
    _Integer id;
    _Path path;
    _Integer path_id;
    _Boolean step_modified;
    _Boolean surr_modified;
    _Surrogate surrogate;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Origin Id in case we fail to retrieve it
    */

    id = _Origin_of(step).id;
    _Origin_of(step).surrogate = (_Surrogate) _NullObject;

    /*
    **  Find the Repository in which the Step is stored
    */

    _GetValue(step, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Find the Identifier of the Path in which the Step is stored
    */

    _GetValue(_Path_of(step), _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Surrogate
    */

    surrogate = (_Surrogate) LwkLbRetrieve(repository,
	lwk_c_domain_surrogate, id, path_id);

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

    /*
    **  Preserve the Modified state of the Step and the Surrogate
    */

    step_modified = (_Boolean) _SetState(step, _StateModified, _StateGet);
    surr_modified = (_Boolean) _SetState(surrogate, _StateModified, _StateGet);

    /*
    **  Set the Surrogate as the Origin
    */

    _Origin_of(step).surrogate = surrogate;

    SetOrigin(step, (_Surrogate) _NullObject);

    /*
    **  Restore the Modified state of the Step and the Surrogate
    */

    if (!step_modified)
	_SetState(step, _StateModified, _StateClear);
    if (!surr_modified)
	_SetState(surrogate, _StateModified, _StateClear);

    return;
    }


static void  RetrieveDestination(step)
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
    _Integer id;
    _Path path;
    _Integer path_id;
    _Boolean step_modified;
    _Boolean surr_modified;
    _Surrogate surrogate;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Destination Id in case we fail to retrieve it
    */

    id = _Destination_of(step).id;
    _Destination_of(step).surrogate = (_Surrogate) _NullObject;

    /*
    **  Find the Repository in which the Step is stored
    */

    _GetValue(step, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Find the Identifier of the Path in which the Step is stored
    */

    _GetValue(_Path_of(step), _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Surrogate
    */

    surrogate = (_Surrogate) LwkLbRetrieve(repository,
	lwk_c_domain_surrogate, id, path_id);

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

    /*
    **  Preserve the Modified state of the Step and the Surrogate
    */

    step_modified = (_Boolean) _SetState(step, _StateModified, _StateGet);
    surr_modified = (_Boolean) _SetState(surrogate, _StateModified, _StateGet);

    /*
    **  Set the Surrogate as the Destination
    */

    _Destination_of(step).surrogate = surrogate;

    SetDestination(step, (_Surrogate) _NullObject);

    /*
    **  Restore the Modified state of the Step and the Surrogate
    */

    if (!step_modified)
	_SetState(step, _StateModified, _StateClear);
    if (!surr_modified)
	_SetState(surrogate, _StateModified, _StateClear);

    return;
    }


static void  RetrievePreviousStep(step)
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
    _Integer id;
    _Path path;
    _Integer path_id;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Origin Id in case we fail to retrieve it
    */

    id = _PreviousStep_of(step).id;
    _PreviousStep_of(step).step = (_Step) _NullObject;

    /*
    **  Find the Repository in which the Step is stored
    */

    _GetValue(step, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Find the Identifier of the Path in which the Step is stored
    */

    _GetValue(_Path_of(step), _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Previous Step
    */

    _PreviousStep_of(step).step =
	(_Step) LwkLbRetrieve(repository, lwk_c_domain_step, id, path_id);

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


static void  RetrieveNextStep(step)
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
    _Integer id;
    _Path path;
    _Integer path_id;
    _Linkbase repository;
    _Transaction old_state;

    /*
    **  Save and clear the Origin Id in case we fail to retrieve it
    */

    id = _NextStep_of(step).id;
    _NextStep_of(step).step = (_Step) _NullObject;

    /*
    **  Find the Repository in which the Step is stored
    */

    _GetValue(step, _P_Linkbase, lwk_c_domain_linkbase, &repository);

    /*
    **  Find the Identifier of the Path in which the Step is stored
    */

    _GetValue(_Path_of(step), _P_Identifier, lwk_c_domain_integer, &path_id);

    /*
    **  Make sure a read transaction is in progress
    */

    old_state = (_Transaction) _Transact(repository, lwk_c_transact_read);

    _StartExceptionBlock

    /*
    **  Retrieve the Next Step
    */

    _NextStep_of(step).step =
	(_Step) LwkLbRetrieve(repository, lwk_c_domain_step, id, path_id);

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


static void  MoveSurrogate(surrogate, path)
_Surrogate surrogate;
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
    ** Set the _Container of the new Surrogate to the _Path of the
    ** Step.
    */

    if (surrogate != (_Surrogate) _NullObject)
	_SetValue(surrogate, _P_Container, lwk_c_domain_path,
	    &path, lwk_c_set_property);

    return;
    }


static void  EncodeSurrogate(surrogate, type, handle)
_Surrogate surrogate;
 unsigned int type;

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
    _DDISlength size;
    _VaryingString encoding;

    if (surrogate != (_Surrogate) _NullObject) {
	/*
	**  Export the Surrogate.
	*/

	size = _Export(surrogate, _False, &encoding);

	/*
	**  Encode the Origin Surrogate.
	*/

	status = ddis_put(&handle, (_DDIStypePtr) &type, &size,
	    (_DDISdata) encoding, (_DDISconstantPtr) 0);

	_DeleteEncoding(&encoding);

	if (!_Success(status))
	    _Raise(internal_encoding_error);
    }

    return;
    }


static _Surrogate  DecodeSurrogate(handle, value_length)
_DDIShandle handle;
 _DDISsize value_length;

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
    _Surrogate surrogate;
    _VaryingString volatile encoding;
    _DDISlength get_length;
    _DDISconstant additional_info;

    encoding = (_VaryingString) _NullObject;

    _StartExceptionBlock

    encoding = (_VaryingString) _AllocateMem(value_length);

    status = ddis_get_value(&handle, &value_length, (_DDISdata) encoding,
	&get_length, &additional_info);

    if (!_Success(status))
	_Raise(internal_decoding_error);

    if (value_length != get_length)
	_Raise(internal_decoding_error);

    /*
    **  Import the Origin Surrogate from the encoding.
    */

    surrogate = (_Surrogate) _Import(_TypeSurrogate, encoding);

    _DeleteEncoding(&encoding);

    _Exceptions
	_WhenOthers
	    _DeleteEncoding(&encoding);
	    _Reraise;
    _EndExceptionBlock

    return surrogate;
    }
